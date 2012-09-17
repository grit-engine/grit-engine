/* Copyright (c) David Cunningham and the Grit Game Engine project 2012 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <OgreString.h>

#include "../math_util.h"
#include "../sleep.h"

#include "GfxPipeline.h"
#include "GfxParticleSystem.h"

// {{{ Utilities

template<class T> void try_set_named_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                                               const char *name, const T &v)
{
    try {
        p->setNamedConstant(name,v);
    } catch (const Ogre::Exception &e) {
        if (e.getNumber() == Ogre::Exception::ERR_INVALIDPARAMS) {
            CLOG << "WARNING: " << e.getDescription() << std::endl;
        } else {
            throw e;
        }
    }
}

// }}}

Ogre::VertexData *screen_quad_vdata;
Ogre::HardwareVertexBufferSharedPtr screen_quad_vbuf;

void gfx_pipeline_init (void)
{
    // Prepare vertex buffer
    screen_quad_vdata = new Ogre::VertexData();
    screen_quad_vdata->vertexStart = 0;
    screen_quad_vdata->vertexCount = 6;

    // Non-instanced data
    unsigned vdecl_size = 0;
    vdecl_size += screen_quad_vdata->vertexDeclaration->addElement(0, vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_POSITION).getSize();
    screen_quad_vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            vdecl_size, 6, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
    screen_quad_vdata->vertexBufferBinding->setBinding(0, screen_quad_vbuf);

    float vdata_raw[] = {
        -1, -1, // bottom left
         1, -1, // bottom right
        -1,  1, // top left
         1,  1, // top right
         1, -1, // bottom right
        -1,  1 // top left
    };
    screen_quad_vbuf->writeData(screen_quad_vdata->vertexStart, screen_quad_vdata->vertexCount*vdecl_size, vdata_raw);
}


// {{{ (Deferred rendering) PointLightsGeometry

// Buffers constructed every frame to render the point lights with deferred shading
// (Cube and pyramid shaped bounding boxes for each light)
class PointLightsGeometry {

protected:

    Ogre::RenderOperation mRenderOperation;
    Ogre::VertexData mVertexData;
    Ogre::IndexData mIndexData;

    unsigned mDeclSize;
    unsigned mMaxVertexesGPU;
    unsigned mMaxIndexesGPU;

    float *mVertexPtr0; // hold the base pointer
    uint16_t *mIndexPtr0; // hold the base pointer
    float *mVertexPtr; // hold the marching pointer
    uint16_t *mIndexPtr; // hold the marching pointer

public:

    PointLightsGeometry (void)
      :
        mDeclSize(0),
        mMaxVertexesGPU(0), // increase later
        mMaxIndexesGPU(0), // increase later
        mVertexPtr0(NULL),
        mIndexPtr0(NULL)
    {
        mRenderOperation.vertexData = &mVertexData;
        mRenderOperation.indexData = &mIndexData;
        mRenderOperation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        mRenderOperation.useIndexes = true;

        // position on screen (just distinguishes the corners frome each other, -1 or 1)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_POSITION);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // direction the light is pointing (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_NORMAL);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // diffuse colour (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_DIFFUSE);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // specular colou (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_SPECULAR);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // light centre (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_TEXTURE_COORDINATES, 0);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // inner/outer cone dot product and range (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_TEXTURE_COORDINATES, 1);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

    }

    const Ogre::RenderOperation &getRenderOperation (void) { return mRenderOperation; }

    unsigned indexesUsed (void) { return mIndexPtr - mIndexPtr0; }
    unsigned vertexesUsed (void) { return ((mVertexPtr-mVertexPtr0)*sizeof(float)) / mDeclSize; }

    void beginLights (unsigned lights)
    {
        unsigned vertexes = lights * 8; // assume worst case -- all cubes
        if (vertexes > mMaxVertexesGPU) {
            mMaxVertexesGPU = vertexes;
            Ogre::HardwareVertexBufferSharedPtr vbuf =
                Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(mDeclSize, vertexes, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
            mVertexData.vertexBufferBinding->setBinding(0, vbuf);
        }

        unsigned indexes = lights * 36; // assume worst case -- all cubes
        if (indexes > mMaxIndexesGPU) {
            mMaxIndexesGPU = indexes;
            mIndexData.indexBuffer = Ogre::HardwareBufferManager::getSingleton().
                createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, indexes, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
        }

        mVertexPtr0 = mVertexPtr = static_cast<float*>(mVertexData.vertexBufferBinding->getBuffer(0)->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        mIndexPtr0 = mIndexPtr = static_cast<uint16_t*>(mIndexData.indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
    }

    #define quad_vertexes(a,b,c,d) a, b, d, a, d, c

    void pyramidLight (const Ogre::Vector3 &wpos,
                       const Ogre::Vector3 *cuboid_planes, const Ogre::Vector3 &dir_ws,
                       const Ogre::ColourValue &diff, const Ogre::ColourValue &spec,
                       float inner, float outer, float range)
    {

        Ogre::Vector3 vertexes[] = {
            wpos + cuboid_planes[0] + cuboid_planes[2] + cuboid_planes[5],
            wpos + cuboid_planes[0] + cuboid_planes[3] + cuboid_planes[5],
            wpos + cuboid_planes[1] + cuboid_planes[2] + cuboid_planes[5],
            wpos + cuboid_planes[1] + cuboid_planes[3] + cuboid_planes[5],
            wpos,
        };

        static unsigned indexes[] = {
            quad_vertexes(3,1,2,0),
            3, 4, 1,
            1, 4, 0,
            0, 4, 2,
            2, 4, 3,
        };


        unsigned offset = vertexesUsed();
        for (int j=0 ; j<5 ; ++j) {
            // position
            *(mVertexPtr++) = vertexes[j].x;
            *(mVertexPtr++) = vertexes[j].y;
            *(mVertexPtr++) = vertexes[j].z;
            // direction
            *(mVertexPtr++) = dir_ws.x;
            *(mVertexPtr++) = dir_ws.y;
            *(mVertexPtr++) = dir_ws.z;
            // diffuse colour
            *(mVertexPtr++) = diff.r;
            *(mVertexPtr++) = diff.g;
            *(mVertexPtr++) = diff.b;
            // specular colour
            *(mVertexPtr++) = spec.r;
            *(mVertexPtr++) = spec.g;
            *(mVertexPtr++) = spec.b;
            // centre position
            *(mVertexPtr++) = wpos.x;
            *(mVertexPtr++) = wpos.y;
            *(mVertexPtr++) = wpos.z;
            // inner/outer cone dot product, range
            *(mVertexPtr++) = inner;
            *(mVertexPtr++) = outer;
            *(mVertexPtr++) = range;
        }
        for (int j=0 ; j<18 ; ++j) {
            *(mIndexPtr++) = offset+indexes[j];
        }
    }

    void cubeLight (const Ogre::Vector3 &wpos,
                    const Ogre::Vector3 *cuboid_planes, const Ogre::Vector3 &dir_ws,
                    const Ogre::ColourValue &diff, const Ogre::ColourValue &spec,
                    float inner, float outer, float range)
    {
        Ogre::Vector3 vertexes[] = {
            wpos + cuboid_planes[0] + cuboid_planes[2] + cuboid_planes[4],
            wpos + cuboid_planes[0] + cuboid_planes[2] + cuboid_planes[5],
            wpos + cuboid_planes[0] + cuboid_planes[3] + cuboid_planes[4],
            wpos + cuboid_planes[0] + cuboid_planes[3] + cuboid_planes[5],
            wpos + cuboid_planes[1] + cuboid_planes[2] + cuboid_planes[4],
            wpos + cuboid_planes[1] + cuboid_planes[2] + cuboid_planes[5],
            wpos + cuboid_planes[1] + cuboid_planes[3] + cuboid_planes[4],
            wpos + cuboid_planes[1] + cuboid_planes[3] + cuboid_planes[5],
        };

        static unsigned indexes[] = {
            quad_vertexes(2,6,0,4),
            quad_vertexes(7,3,5,1),
            quad_vertexes(3,2,1,0),
            quad_vertexes(6,7,4,5),
            quad_vertexes(0,4,1,5),
            quad_vertexes(3,7,2,6)
        };

        unsigned offset = vertexesUsed();
        for (int j=0 ; j<8 ; ++j) {
            // position
            *(mVertexPtr++) = vertexes[j].x;
            *(mVertexPtr++) = vertexes[j].y;
            *(mVertexPtr++) = vertexes[j].z;
            // direction
            *(mVertexPtr++) = dir_ws.x;
            *(mVertexPtr++) = dir_ws.y;
            *(mVertexPtr++) = dir_ws.z;
            // diffuse colour
            *(mVertexPtr++) = diff.r;
            *(mVertexPtr++) = diff.g;
            *(mVertexPtr++) = diff.b;
            // specular colour
            *(mVertexPtr++) = spec.r;
            *(mVertexPtr++) = spec.g;
            *(mVertexPtr++) = spec.b;
            // centre position
            *(mVertexPtr++) = wpos.x;
            *(mVertexPtr++) = wpos.y;
            *(mVertexPtr++) = wpos.z;
            // inner/outer cone dot product, range
            *(mVertexPtr++) = inner;
            *(mVertexPtr++) = outer;
            *(mVertexPtr++) = range;
        }
        for (int j=0 ; j<36 ; ++j) {
            *(mIndexPtr++) = offset+indexes[j];
        }
    }


    void endLights ()
    {
        mVertexData.vertexBufferBinding->getBuffer(0)->unlock();
        mIndexData.indexBuffer->unlock();

        mVertexData.vertexCount = vertexesUsed();
        mIndexData.indexCount = indexesUsed();
    }


};

// }}}


// {{{ (Deferred rendering) DeferredLightingPasses

class DeferredLightingPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    PointLightsGeometry mdl;
    PointLightsGeometry mdlInside;

    public:
    DeferredLightingPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0, Ogre::StringUtil::BLANK), pipe(pipe)
    {

        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        Ogre::Camera *cam = pipe->getCamera();
        Ogre::Vector3 cam_pos = cam->getPosition();
        Ogre::Viewport *viewport = ogre_sm->getCurrentViewport();
        Ogre::RenderTarget *render_target = viewport->getTarget();
        float render_target_flipping = render_target->requiresTextureFlipping() ? -1 : 1;

        // various camera and lighting things
        Ogre::Matrix4 view = cam->getViewMatrix();
        Ogre::Matrix4 proj = cam->getProjectionMatrixRS();
        Ogre::Matrix4 view_proj = proj*view;
        Ogre::Vector3 sun_pos_ws = ogre_sun->getDirection();
        Ogre::ColourValue sun_diffuse = ogre_sun->getDiffuseColour();
        Ogre::ColourValue sun_specular = ogre_sun->getSpecularColour();

        // corners mapping for worldspace fragment position reconstruction:
        // top-right near, top-left near, bottom-left near, bottom-right near,
        // top-right far, top-left far, bottom-left far, bottom-right far. 
        Ogre::Vector3 top_right_ray = cam->getWorldSpaceCorners()[4] - cam_pos;
        Ogre::Vector3 top_left_ray = cam->getWorldSpaceCorners()[5] - cam_pos;
        Ogre::Vector3 bottom_left_ray = cam->getWorldSpaceCorners()[6] - cam_pos;
        Ogre::Vector3 bottom_right_ray = cam->getWorldSpaceCorners()[7] - cam_pos;

        try {
            Ogre::HighLevelGpuProgramPtr das_vp = Ogre::HighLevelGpuProgramManager::getSingleton().getByName("deferred_ambient_sun_v", RESGRP);
            load_and_validate_shader("Environment/Sun/Shadows","deferred_ambient_sun_v",das_vp);
            Ogre::HighLevelGpuProgramPtr das_fp = Ogre::HighLevelGpuProgramManager::getSingleton().getByName("deferred_ambient_sun_f", RESGRP);
            load_and_validate_shader("Environment/Sun/Shadows","deferred_ambient_sun_f",das_fp);

            Ogre::TexturePtr noise_tex = Ogre::TextureManager::getSingleton().load("system/HiFreqNoiseGauss.64.bmp", RESGRP);


            const Ogre::GpuProgramParametersSharedPtr &das_vp_params = das_vp->getDefaultParameters();
            const Ogre::GpuProgramParametersSharedPtr &das_fp_params = das_fp->getDefaultParameters();


            /////////
            // sun //
            /////////
            try_set_named_constant(das_vp_params, "top_left_ray", top_left_ray);
            try_set_named_constant(das_vp_params, "top_right_ray", top_right_ray);
            try_set_named_constant(das_vp_params, "bottom_left_ray", bottom_left_ray);
            try_set_named_constant(das_vp_params, "bottom_right_ray", bottom_right_ray);
            try_set_named_constant(das_vp_params, "render_target_flipping", render_target_flipping);

            //try_set_named_constant(f_params, "near_clip_distance", cam->getNearClipDistance());
            try_set_named_constant(das_fp_params, "far_clip_distance", cam->getFarClipDistance());
            try_set_named_constant(das_fp_params, "camera_pos_ws", cam_pos);

            // actually we need only the z and w rows but this is just one renderable per frame so
            // not a big deal
            Ogre::Matrix4 special_proj = cam->getProjectionMatrix();
            Ogre::Matrix4 special_view_proj = special_proj*view;
            try_set_named_constant(das_fp_params, "view_proj", special_view_proj);
            try_set_named_constant(das_fp_params, "sun_pos_ws", -sun_pos_ws);
            try_set_named_constant(das_fp_params, "sun_diffuse", sun_diffuse);
            try_set_named_constant(das_fp_params, "sun_specular", sun_specular);

            Ogre::Vector3 the_fog_params(fog_density, env_brightness, global_exposure);
            try_set_named_constant(das_fp_params, "the_fog_params", the_fog_params);
            try_set_named_constant(das_fp_params, "the_fog_colour", ogre_sm->getFogColour());

            if (gfx_option(GFX_SHADOW_RECEIVE)) {
                try_set_named_constant(das_fp_params, "shadow_view_proj1", shadow_view_proj[0]);
                try_set_named_constant(das_fp_params, "shadow_view_proj2", shadow_view_proj[1]);
                try_set_named_constant(das_fp_params, "shadow_view_proj3", shadow_view_proj[2]);
            }


            unsigned tex_index = 0;
            for (unsigned i=0 ; i<3 ; ++i) {
                ogre_rs->_setTexture(tex_index++, true, pipe->getGBufferTexture(i));
            }

            if (gfx_option(GFX_SHADOW_RECEIVE)) {
                for (unsigned i=0 ; i<3 ; ++i) {
                    ogre_rs->_setTexture(tex_index++, true, ogre_sm->getShadowTexture(i));
                }
                if (gfx_option(GFX_SHADOW_FILTER_DITHER_TEXTURE)) {
                    ogre_rs->_setTexture(tex_index++, true, noise_tex);
                }
            }

            ogre_rs->_setTexture(tex_index, true, env_cube_tex);
            ogre_rs->_setTextureUnitFiltering(tex_index, Ogre::FT_MIN, Ogre::FO_ANISOTROPIC);
            ogre_rs->_setTextureUnitFiltering(tex_index, Ogre::FT_MAG, Ogre::FO_ANISOTROPIC);
            ogre_rs->_setTextureUnitFiltering(tex_index, Ogre::FT_MIP, Ogre::FO_LINEAR);
            tex_index++;

            ogre_rs->_setCullingMode(Ogre::CULL_NONE);
            ogre_rs->_setDepthBufferParams(true, true, Ogre::CMPF_LESS_EQUAL);
            ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
            ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
            ogre_rs->setStencilCheckEnabled(false);

            // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
            ogre_rs->bindGpuProgram(das_vp->_getBindingDelegate());
            ogre_rs->bindGpuProgram(das_fp->_getBindingDelegate());

            ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, das_fp_params, Ogre::GPV_ALL);
            ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, das_vp_params, Ogre::GPV_ALL);

            // render the instances
            Ogre::RenderOperation op;
            op.useIndexes = false;
            op.vertexData = screen_quad_vdata;
            op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
            ogre_rs->_render(op);

            for (unsigned i=0 ; i<tex_index ; ++i) {
                ogre_rs->_disableTextureUnit(i);
            }

        } catch (const Ogre::Exception &e) {
            CERR << "Rendering deferred sun, got: " << e.getDescription() << std::endl;
        }


        try {

            /////////////////////
            // deferred lights //
            /////////////////////

            const Ogre::LightList &ll = ogre_sm->_getLightsAffectingFrustum();

            mdl.beginLights(ll.size());
            mdlInside.beginLights(ll.size());

            int light_counter = 0;
            for (Ogre::LightList::const_iterator i=ll.begin(),i_=ll.end(); i!=i_ ; ++i) {
                Ogre::Light *l = *i;
                if (l == ogre_sun) continue;
                light_counter++;
                const Ogre::Vector3 &dir_ws = l->getDerivedDirection();
                const Ogre::ColourValue &diff = l->getDiffuseColour();
                const Ogre::ColourValue &spec = l->getSpecularColour();
                float inner = Ogre::Math::Cos(l->getSpotlightInnerAngle());
                float outer = Ogre::Math::Cos(l->getSpotlightOuterAngle());
                float range = l->getAttenuationRange();
                Ogre::Vector3 wpos = l->getDerivedPosition();
                // define cone space in terms of world space
                Ogre::Vector3 dir_z = dir_ws;
                Ogre::Vector3 dir_x = dir_z.perpendicular();
                Ogre::Vector3 dir_y = dir_z.crossProduct(dir_x);
                bool use_pyramid = outer >= 0.4;
                // define a bounding cuboid in cone space
                float outer2 = std::max(0.0f, outer); // stop it at the 90 degree point
                float cs_x_max = range * sqrtf(1 - outer2*outer2);
                if (use_pyramid) {
                        cs_x_max /= outer;
                };
                float cs_x_min = -cs_x_max;
                float cs_y_max = cs_x_max;
                float cs_y_min = cs_x_min;
                float cs_z_max = range;
                float cs_z_min = range * std::min(0.0f, outer);
                Ogre::Vector3 light_to_cam = cam_pos - wpos;

                // the 'inside' method always works but is less efficient as it
                // does not allow us to avoid lighting when there is an object
                // between the lit surface and the camera occluding the fragments
                // to be shaded.

                // on the other hand, the 'inside' method does have an optimisation
                // in the case where the light is between the camera and a fragment
                // but the fragment is out of range of the light

                // I am choosing to prefer the occlusion optimisation as lights that are
                // too far from surfaces should be avoided by good level design.

                // however 'inside' is the only method that can be used if the
                // camera is inside the light volume.

                bool inside = between<float>(light_to_cam.dotProduct(dir_x), cs_x_min-1, cs_x_max+1)
                           && between<float>(light_to_cam.dotProduct(dir_y), cs_y_min-1, cs_y_max+1)
                           && between<float>(light_to_cam.dotProduct(dir_z), cs_z_min-1, cs_z_max+1);

                //inside = true;

                Ogre::Vector3 cuboid_planes[] = {
                    cs_x_min * dir_x, cs_x_max * dir_x,
                    cs_y_min * dir_y, cs_y_max * dir_y,
                    cs_z_min * dir_z, cs_z_max * dir_z,
                };

                PointLightsGeometry &mdl_ = inside ? mdlInside : mdl;

                if (use_pyramid) {
                    mdl_.pyramidLight(wpos, cuboid_planes, dir_ws,diff,spec,inner,outer,range);
                } else {
                    mdl_.cubeLight(wpos, cuboid_planes, dir_ws,diff,spec,inner,outer,range);
                }
            }
            mdl.endLights();
            mdlInside.endLights();
            //CVERB << "Total lights: " << light_counter << std::endl;

            if (light_counter > 0) {

                Ogre::HighLevelGpuProgramPtr dl_vp = Ogre::HighLevelGpuProgramManager::getSingleton().getByName("deferred_lights_v", RESGRP);
                dl_vp->load();
                Ogre::HighLevelGpuProgramPtr dl_fp = Ogre::HighLevelGpuProgramManager::getSingleton().getByName("deferred_lights_f", RESGRP);
                dl_fp->load();

                const Ogre::GpuProgramParametersSharedPtr &dl_vp_params = dl_vp->getDefaultParameters();
                const Ogre::GpuProgramParametersSharedPtr &dl_fp_params = dl_fp->getDefaultParameters();
                try_set_named_constant(dl_vp_params, "view_proj", view_proj);
                try_set_named_constant(dl_vp_params, "render_target_flipping", render_target_flipping);

                try_set_named_constant(dl_fp_params, "top_left_ray", top_left_ray);
                try_set_named_constant(dl_fp_params, "top_right_ray", top_right_ray);
                try_set_named_constant(dl_fp_params, "bottom_left_ray", bottom_left_ray);
                try_set_named_constant(dl_fp_params, "bottom_right_ray", bottom_right_ray);

                Ogre::Vector3 the_fog_params(fog_density, env_brightness, global_exposure);
                try_set_named_constant(dl_fp_params, "the_fog_params", the_fog_params);
                try_set_named_constant(dl_fp_params, "far_clip_distance", cam->getFarClipDistance());
                try_set_named_constant(dl_fp_params, "camera_pos_ws", cam_pos);
                if (d3d9) {
                    Ogre::Vector4 viewport_size(     viewport->getActualWidth(),      viewport->getActualHeight(),
                                                1.0f/viewport->getActualWidth(), 1.0f/viewport->getActualHeight());
                    try_set_named_constant(dl_fp_params, "viewport_size",viewport_size);
                }

                unsigned tex_index = 0;
                for (unsigned i=0 ; i<3 ; ++i) {
                    ogre_rs->_setTexture(tex_index++, true, pipe->getGBufferTexture(i));
                }


                // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
                ogre_rs->bindGpuProgram(dl_vp->_getBindingDelegate());
                ogre_rs->bindGpuProgram(dl_fp->_getBindingDelegate());

                ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, dl_fp_params, Ogre::GPV_ALL);
                ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, dl_vp_params, Ogre::GPV_ALL);


                if (mdl.indexesUsed() > 0) {
                    ogre_rs->_setCullingMode(Ogre::CULL_CLOCKWISE);
                    ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_LESS_EQUAL);
                    ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);
                    ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
                    ogre_rs->setStencilCheckEnabled(false);
                    ogre_rs->_render(mdl.getRenderOperation());
                }

                if (mdlInside.indexesUsed() > 0) {
                    ogre_rs->_setCullingMode(Ogre::CULL_ANTICLOCKWISE);
                    ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_GREATER_EQUAL);
                    ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);
                    ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
                    ogre_rs->setStencilCheckEnabled(false);
                    ogre_rs->_render(mdlInside.getRenderOperation());
                }

                for (unsigned i=0 ; i<tex_index ; ++i) {
                    ogre_rs->_disableTextureUnit(i);
                }
            }

        } catch (const Ogre::Exception &e) {
            CERR << "Rendering deferred point lights, got: " << e.getDescription() << std::endl;
        }

    }
    
};

// }}}



// {{{ Particles passes 

class ParticlesPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;
    bool alphaBlend;

    public:
    ParticlesPasses (GfxPipeline *pipe, bool alpha_blend)
      : Ogre::RenderQueueInvocation(0, Ogre::StringUtil::BLANK), pipe(pipe), alphaBlend(alpha_blend)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        gfx_particle_render(pipe, alphaBlend);
    }
    
};

// }}}



float eye_separation = 0.0f;
GfxPipeline *eye_left = NULL, *eye_right = NULL;

void do_reset_eyes (void)
{
        eye_left->setFOVy(gfx_option(GFX_FOV));
        eye_left->setNearClipDistance(gfx_option(GFX_NEAR_CLIP));
        eye_left->setFarClipDistance(gfx_option(GFX_FAR_CLIP));
    
        if (stereoscopic()) {

            eye_right->setFOVy(gfx_option(GFX_FOV));
            eye_right->setNearClipDistance(gfx_option(GFX_NEAR_CLIP));
            eye_right->setFarClipDistance(gfx_option(GFX_FAR_CLIP));

            float FOV = gfx_option(GFX_FOV);
            float monitor_height = gfx_option(GFX_MONITOR_HEIGHT);
            float distance = gfx_option(GFX_MONITOR_EYE_DISTANCE);
            float eye_separation = gfx_option(GFX_EYE_SEPARATION);
            float min = gfx_option(GFX_MIN_PERCEIVED_DEPTH);
            float max = gfx_option(GFX_MAX_PERCEIVED_DEPTH);

            float s = 2*tan((FOV/2)/180*M_PI)/monitor_height * (eye_separation * (1-distance/max));
            eye_left->setFrustumOffset(s/2);
            eye_right->setFrustumOffset(-s/2);
            eye_left->setFocalLength(1);
            eye_right->setFocalLength(1);
    
            float c = 2*tan((FOV/2)/180*M_PI)/monitor_height * (eye_separation * (1-distance/min));
            c = gfx_option(GFX_NEAR_CLIP) * (s - c);
            eye_separation = c;
        } else {
            eye_left->setFrustumOffset(0);
            eye_left->setFocalLength(1);
            eye_separation = 0;
        }

}


GfxPipeline::GfxPipeline (const std::string &name, Ogre::RenderTarget *target, bool left)
  : left(left), target(target)
{

    unsigned width = target->getWidth();
    if (gfx_option(GFX_CROSS_EYE)) width /= 2;
    unsigned height = target->getHeight();

    {
        gBuffer = ogre_rs->createMultiRenderTarget(name+":G");
        // make textures for it
        const char *el_names[] = { ":G0", ":G1", ":G2", ":G3" };
        for (unsigned i=0 ; i<3 ; ++i) {
            gBufferElements[i] =
                Ogre::TextureManager::getSingleton().createManual(
                    name+el_names[i], RESGRP, Ogre::TEX_TYPE_2D,
                    width, height, 1,
                    0,
                    Ogre::PF_A8R8G8B8,
                    Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE | Ogre::TU_RENDERTARGET,
                    NULL,
                    use_hwgamma);
            gBuffer->bindSurface(i, gBufferElements[i]->getBuffer()->getRenderTarget());
        }

        gBufferViewport = gBuffer->addViewport(NULL);
        gBufferViewport->setAutoUpdated(false);
        gBufferViewport->setOverlaysEnabled(false);
        gBufferViewport->setShadowsEnabled(true);
        // white here makes sure that the depth (remember that it is 3 bytes) is maximal
        gBufferViewport->setBackgroundColour(Ogre::ColourValue::White);

        rqisGbuffer = ogre_root->createRenderQueueInvocationSequence(name+":gbuffer");
        rqisGbuffer->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_GBUFFER_OPAQUE));
        gBufferViewport->setRenderQueueInvocationSequenceName(name+":gbuffer");
    }

    {
        hdrFb = Ogre::TextureManager::getSingleton().createManual(
                    name+":hdrFb", RESGRP, Ogre::TEX_TYPE_2D,
                    width, height, 1,
                    0,
                    Ogre::PF_FLOAT16_RGB,
                    Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE | Ogre::TU_RENDERTARGET,
                    NULL,
                    false);
        hdrFbTarget = hdrFb->getBuffer()->getRenderTarget();

        hdrFbViewport = hdrFbTarget->addViewport(NULL);
        hdrFbViewport->setAutoUpdated(false);
        hdrFbViewport->setOverlaysEnabled(left); // FIXME: should be on the target, but never mind
        hdrFbViewport->setShadowsEnabled(false);

        rqisDeferred = ogre_root->createRenderQueueInvocationSequence(name+":deferred");
        rqisDeferred->add(new DeferredLightingPasses(this));
        rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_OPAQUE));
        rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_SKY));
        rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_EMISSIVE));
        rqisDeferred->add(new ParticlesPasses(this, false)); // opaque
        rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_SKY_ALPHA));
        rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA_DEPTH));
        rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA));
        rqisDeferred->add(new ParticlesPasses(this, true)); // alpha
        rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(Ogre::RENDER_QUEUE_OVERLAY));
        hdrFbViewport->setRenderQueueInvocationSequenceName(name+":deferred");
    }

    {
        float norm_left_side = gfx_option(GFX_CROSS_EYE) && !left ? 0.5 : 0;
        float norm_width = gfx_option(GFX_CROSS_EYE) ? 0.5 : 1;
        targetViewport = target->addViewport(NULL, 0,  norm_left_side, 0, norm_width, 1);
        targetViewport->setAutoUpdated(false);
        targetViewport->setOverlaysEnabled(false);
        targetViewport->setShadowsEnabled(false);

        // we just render a quad into this guy, so no need for a render queue invocation sequence
    }

    cam = ogre_sm->createCamera(name+":cam");
    cam->setAutoAspectRatio(true);

}

GfxPipeline::~GfxPipeline (void) {
    ogre_sm->destroyCamera(cam);

    gBuffer->removeViewport(gBufferViewport->getZOrder());
    ogre_rs->destroyRenderTarget(gBuffer->getName());
    ogre_root->destroyRenderQueueInvocationSequence(rqisGbuffer->getName());
    for (unsigned i=0 ; i<3 ; ++i) {
         Ogre::TextureManager::getSingleton().remove(gBufferElements[i]->getName());
    }

    target->removeViewport(targetViewport->getZOrder());

    hdrFbTarget->removeViewport(hdrFbViewport->getZOrder());
    ogre_root->destroyRenderQueueInvocationSequence(rqisDeferred->getName());
    Ogre::TextureManager::getSingleton().remove(hdrFb->getName());
}


void GfxPipeline::render (const Vector3 &cam_pos, const Quaternion &cam_dir)
{

    viewPos = cam_pos + cam_dir * Vector3((left?-1:1) * eye_separation/2,0,0);
    viewDir = cam_dir;
 
    cam->setPosition(to_ogre(viewPos));
    // Ogre cameras point towards Z whereas in Grit the convention is that 'unrotated' means pointing towards y (north)
    cam->setOrientation(to_ogre(viewDir*Quaternion(Degree(90),Vector3(1,0,0))));

    // populate gbuffer
    
    unsigned long long micros_before = micros();
    gBufferViewport->setCamera(cam);
    gBufferViewport->update();
    unsigned long long micros_after_gbuffer = micros();
    gBufferStats.batches = ogre_rs->_getBatchCount();
    gBufferStats.triangles = ogre_rs->_getFaceCount();;
    gBufferStats.micros = micros_after_gbuffer - micros_before;

    // render gbuffer and alpha, sky, etc into hdr viewport
    hdrFbViewport->setCamera(cam);
    hdrFbViewport->update();
    unsigned long long micros_after_deferred = micros();
    deferredStats.batches = ogre_rs->_getBatchCount();
    deferredStats.triangles = ogre_rs->_getFaceCount();
    deferredStats.micros = micros_after_deferred - micros_after_gbuffer;

    // tonemap onto screen
    try {

        
        ogre_rs->_setViewport(targetViewport);
        ogre_rs->_beginFrame();

        Ogre::HighLevelGpuProgramPtr vp = Ogre::HighLevelGpuProgramManager::getSingleton().getByName("tonemap_v", RESGRP);
        load_and_validate_shader("Tonemap","tonemap_v",vp);
        Ogre::HighLevelGpuProgramPtr fp = Ogre::HighLevelGpuProgramManager::getSingleton().getByName("tonemap_f", RESGRP);
        load_and_validate_shader("Tonemap","tonemap_f",fp);

        const Ogre::GpuProgramParametersSharedPtr &vp_params = vp->getDefaultParameters();
        const Ogre::GpuProgramParametersSharedPtr &fp_params = fp->getDefaultParameters();

        float render_target_flipping = target->requiresTextureFlipping() ? -1.0f : 1.0f;
        try_set_named_constant(vp_params, "render_target_flipping", render_target_flipping);
        if (d3d9) {
            Ogre::Vector4 viewport_size(     targetViewport->getActualWidth(),      targetViewport->getActualHeight(),
                                        1.0f/targetViewport->getActualWidth(), 1.0f/targetViewport->getActualHeight());
            try_set_named_constant(vp_params, "viewport_size",viewport_size);
        }

        try_set_named_constant(fp_params, "global_exposure", global_exposure);
        try_set_named_constant(fp_params, "global_contrast", global_contrast);

        ogre_rs->_setCullingMode(Ogre::CULL_NONE);
        ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);

        ogre_rs->_setTexture(0, true, hdrFb);

        // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
        ogre_rs->bindGpuProgram(vp->_getBindingDelegate());
        ogre_rs->bindGpuProgram(fp->_getBindingDelegate());

        ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, fp_params, Ogre::GPV_ALL);
        ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, vp_params, Ogre::GPV_ALL);

        ogre_rs->_setCullingMode(Ogre::CULL_NONE);
        ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);
        ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
        ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
        ogre_rs->setStencilCheckEnabled(false);

        // render the instances
        Ogre::RenderOperation op;
        op.useIndexes = false;
        op.vertexData = screen_quad_vdata;
        op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        ogre_rs->_render(op);

        ogre_rs->_disableTextureUnit(0);

        ogre_rs->_endFrame();

    } catch (const Ogre::Exception &e) {

        CERR << "Rendering tone map compositor, got: " << e.getDescription() << std::endl;
    }
}

