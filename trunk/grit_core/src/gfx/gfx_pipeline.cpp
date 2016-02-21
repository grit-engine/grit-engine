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

#include <math_util.h>
#include <sleep.h>

#include "gfx_body.h"
#include "gfx_decal.h"
#include "gfx_particle_system.h"
#include "gfx_pipeline.h"
#include "gfx_sky_body.h"

static inline Ogre::HighLevelGpuProgramPtr get_shader(const char *name)
{
    auto ptr = Ogre::HighLevelGpuProgramManager::getSingleton().getByName(name, RESGRP);
    if (ptr.isNull())
        EXCEPT << "Shader does not exist: " << name << ENDL;
    return ptr;
}

static void render_with_progs (GfxShader *shader, const Ogre::RenderOperation &op)
{
    auto vp = shader->getHackOgreVertexProgram();
    auto fp = shader->getHackOgreFragmentProgram();

    // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
    ogre_rs->bindGpuProgram(vp->_getBindingDelegate());
    ogre_rs->bindGpuProgram(fp->_getBindingDelegate());

    ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, fp->getDefaultParameters(), Ogre::GPV_ALL);
    ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, vp->getDefaultParameters(), Ogre::GPV_ALL);

    ogre_rs->_render(op);
}

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


static GfxShader *deferred_ambient_sun = nullptr;
static GfxShader *get_deferred_ambient_sun (void)
{
    if (deferred_ambient_sun == nullptr) {
        auto vp = get_shader("deferred_ambient_sun_v");
        auto fp = get_shader("deferred_ambient_sun_f");
        deferred_ambient_sun = gfx_shader_make_from_existing("deferred_ambient_sun", vp, fp, {});
    }
    return deferred_ambient_sun;
}

static GfxShader *deferred_lights = nullptr;
static GfxShader *get_deferred_lights (void)
{
    if (deferred_lights == nullptr) {
        auto vp = get_shader("deferred_lights_v");
        auto fp = get_shader("deferred_lights_f");
        deferred_lights = gfx_shader_make_from_existing("deferred_lights", vp, fp, {});
    }
    return deferred_lights;
}

static GfxShader *compositor_tonemap = nullptr;
static GfxShader *get_compositor_tonemap (void)
{
    if (compositor_tonemap == nullptr) {
        auto vp = get_shader("compositor_v");
        auto fp = get_shader("tonemap");
        compositor_tonemap = gfx_shader_make_from_existing("compositor_tonemap", vp, fp, {});
    }
    return compositor_tonemap;
}

static GfxShader *compositor_bloom_filter_then_horz_blur = nullptr;
static GfxShader *get_compositor_bloom_filter_then_horz_blur (void)
{
    if (compositor_bloom_filter_then_horz_blur == nullptr) {
        auto vp = get_shader("compositor_v");
        auto fp = get_shader("bloom_filter_then_horz_blur");
        compositor_bloom_filter_then_horz_blur =
            gfx_shader_make_from_existing("compositor_bloom_filter_then_horz_blur", vp, fp, {});
    }
    return compositor_bloom_filter_then_horz_blur;
}

static GfxShader *compositor_bloom_vert_blur = nullptr;
static GfxShader *get_compositor_bloom_vert_blur (void)
{
    if (compositor_bloom_vert_blur == nullptr) {
        auto vp = get_shader("compositor_v");
        auto fp = get_shader("bloom_vert_blur");
        compositor_bloom_vert_blur = gfx_shader_make_from_existing("compositor_bloom_vert_blur", vp, fp, {});
    }
    return compositor_bloom_vert_blur;
}

static GfxShader *compositor_bloom_horz_blur = nullptr;
static GfxShader *get_compositor_bloom_horz_blur (void)
{
    if (compositor_bloom_horz_blur == nullptr) {
        auto vp = get_shader("compositor_v");
        auto fp = get_shader("bloom_horz_blur");
        compositor_bloom_horz_blur = gfx_shader_make_from_existing("compositor_bloom_horz_blur", vp, fp, {});
    }
    return compositor_bloom_horz_blur;
}

static GfxShader *compositor_bloom_vert_blur_combine_and_tonemap = nullptr;
static GfxShader *get_compositor_bloom_vert_blur_combine_and_tonemap (void)
{
    if (compositor_bloom_vert_blur_combine_and_tonemap == nullptr) {
        auto vp = get_shader("compositor_v");
        auto fp = get_shader("bloom_vert_blur_combine_and_tonemap");
        compositor_bloom_vert_blur_combine_and_tonemap =
            gfx_shader_make_from_existing("compositor_bloom_vert_blur_combine_and_tonemap", vp, fp, {});
    }
    return compositor_bloom_vert_blur_combine_and_tonemap;
}

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
        Ogre::Vector4 viewport_size(     viewport->getActualWidth(),      viewport->getActualHeight(),
                                    1.0f/viewport->getActualWidth(), 1.0f/viewport->getActualHeight());

        // various camera and lighting things
        Ogre::Matrix4 view = cam->getViewMatrix();
        Ogre::Matrix4 inv_view = view.inverse();
        Ogre::Matrix4 proj = cam->getProjectionMatrixWithRSDepth();
        Ogre::Matrix4 view_proj = proj*view;
        Ogre::Vector3 sun_pos_ws = ogre_sun->getDirection();

        // corners mapping for worldspace fragment position reconstruction:
        // top-right near, top-left near, bottom-left near, bottom-right near,
        // top-right far, top-left far, bottom-left far, bottom-right far. 
        Ogre::Vector3 top_right_ray = cam->getWorldSpaceCorners()[4] - cam_pos;
        Ogre::Vector3 top_left_ray = cam->getWorldSpaceCorners()[5] - cam_pos;
        Ogre::Vector3 bottom_left_ray = cam->getWorldSpaceCorners()[6] - cam_pos;
        Ogre::Vector3 bottom_right_ray = cam->getWorldSpaceCorners()[7] - cam_pos;

        try {
            get_deferred_ambient_sun()->validate();
            auto das_vp = deferred_ambient_sun->getHackOgreVertexProgram();
            auto das_fp = deferred_ambient_sun->getHackOgreFragmentProgram();

            Ogre::TexturePtr noise_tex = shadow_pcf_noise_map->getOgreTexturePtr();
            noise_tex->load();


            /////////
            // sun //
            /////////
            try_set_named_constant(das_vp, "top_left_ray", top_left_ray);
            try_set_named_constant(das_vp, "top_right_ray", top_right_ray);
            try_set_named_constant(das_vp, "bottom_left_ray", bottom_left_ray);
            try_set_named_constant(das_vp, "bottom_right_ray", bottom_right_ray);
            try_set_named_constant(das_vp, "render_target_flipping", render_target_flipping);
            if (d3d9) {
                try_set_named_constant(das_vp, "viewport_size",viewport_size);
            }

            //try_set_named_constant(f_params, "near_clip_distance", cam->getNearClipDistance());
            try_set_named_constant(das_fp, "far_clip_distance", cam->getFarClipDistance());
            try_set_named_constant(das_fp, "camera_pos_ws", cam_pos);

            // actually we need only the z and w rows but this is just one renderable per frame so
            // not a big deal
            Ogre::Matrix4 special_proj = cam->getProjectionMatrix();
            Ogre::Matrix4 special_view_proj = special_proj*view;
            try_set_named_constant(das_fp, "view_proj", special_view_proj);
            try_set_named_constant(das_fp, "inv_view", inv_view);
            try_set_named_constant(das_fp, "sun_pos_ws", -sun_pos_ws);
            try_set_named_constant(das_fp, "sun_diffuse", to_ogre(sunlight_diffuse));
            try_set_named_constant(das_fp, "sun_specular", to_ogre(sunlight_specular));

            Ogre::Vector3 the_fog_params(fog_density, env_cube_cross_fade, /*unused*/ 0);
            try_set_named_constant(das_fp, "the_fog_params", the_fog_params);
            if (gfx_option(GFX_FOG)) {
                try_set_named_constant(das_fp, "the_fog_colour", to_ogre(fog_colour));
            }

            if (gfx_option(GFX_SHADOW_RECEIVE)) {
                try_set_named_constant(das_fp, "shadow_view_proj1", shadow_view_proj[0]);
                try_set_named_constant(das_fp, "shadow_view_proj2", shadow_view_proj[1]);
                try_set_named_constant(das_fp, "shadow_view_proj3", shadow_view_proj[2]);
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

            if (global_env_cube0 != nullptr && global_env_cube0->isLoaded()) {
                const Ogre::TexturePtr &global_env_cube_tex = global_env_cube0->getOgreTexturePtr();
                ogre_rs->_setTexture(tex_index, true, global_env_cube_tex);
                ogre_rs->_setTextureUnitFiltering(tex_index, Ogre::FT_MIN, Ogre::FO_ANISOTROPIC);
                ogre_rs->_setTextureUnitFiltering(tex_index, Ogre::FT_MAG, Ogre::FO_ANISOTROPIC);
                ogre_rs->_setTextureUnitFiltering(tex_index, Ogre::FT_MIP, Ogre::FO_LINEAR);
            }
            tex_index++;

            if (global_env_cube1 != nullptr && global_env_cube1->isLoaded()) {
                const Ogre::TexturePtr &global_env_cube_tex = global_env_cube1->getOgreTexturePtr();
                ogre_rs->_setTexture(tex_index, true, global_env_cube_tex);
                ogre_rs->_setTextureUnitFiltering(tex_index, Ogre::FT_MIN, Ogre::FO_ANISOTROPIC);
                ogre_rs->_setTextureUnitFiltering(tex_index, Ogre::FT_MAG, Ogre::FO_ANISOTROPIC);
                ogre_rs->_setTextureUnitFiltering(tex_index, Ogre::FT_MIP, Ogre::FO_LINEAR);
            }
            tex_index++;

            ogre_rs->_setCullingMode(Ogre::CULL_NONE);
            ogre_rs->_setDepthBufferParams(true, true, Ogre::CMPF_LESS_EQUAL);
            ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
            ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
            ogre_rs->setStencilCheckEnabled(false);
            ogre_rs->_setDepthBias(0, 0);

            // Sun + ambient render
            Ogre::RenderOperation op;
            op.useIndexes = false;
            op.vertexData = screen_quad_vdata;
            op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
            render_with_progs(deferred_ambient_sun, op);

            for (unsigned i=0 ; i<tex_index ; ++i) {
                ogre_rs->_disableTextureUnit(i);
            }

        } catch (const Exception &e) {
            CERR << "Rendering deferred sun, got: " << e << std::endl;
        } catch (const Ogre::Exception &e) {
            CERR << "Rendering deferred sun, got: " << e.getDescription() << std::endl;
        }


        if (!pipe->getCameraOpts().pointLights) return;

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

                get_deferred_lights()->validate();
                auto dl_vp = deferred_lights->getHackOgreVertexProgram();
                auto dl_fp = deferred_lights->getHackOgreFragmentProgram();

                try_set_named_constant(dl_vp, "view_proj", view_proj);
                try_set_named_constant(dl_vp, "render_target_flipping", render_target_flipping);

                try_set_named_constant(dl_fp, "inv_view", inv_view);
                try_set_named_constant(dl_fp, "top_left_ray", top_left_ray);
                try_set_named_constant(dl_fp, "top_right_ray", top_right_ray);
                try_set_named_constant(dl_fp, "bottom_left_ray", bottom_left_ray);
                try_set_named_constant(dl_fp, "bottom_right_ray", bottom_right_ray);

                Ogre::Vector3 the_fog_params(fog_density, env_cube_cross_fade, /*unused*/ 0);
                try_set_named_constant(dl_fp, "the_fog_params", the_fog_params);
                try_set_named_constant(dl_fp, "far_clip_distance", cam->getFarClipDistance());
                try_set_named_constant(dl_fp, "camera_pos_ws", cam_pos);
                if (d3d9) {
                    try_set_named_constant(dl_fp, "viewport_size",viewport_size);
                }

                unsigned tex_index = 0;
                for (unsigned i=0 ; i<3 ; ++i) {
                    ogre_rs->_setTexture(tex_index++, true, pipe->getGBufferTexture(i));
                }

                ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);
                ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
                ogre_rs->setStencilCheckEnabled(false);
                ogre_rs->_setDepthBias(0, 0);

                if (mdl.indexesUsed() > 0) {
                    ogre_rs->_setCullingMode(Ogre::CULL_CLOCKWISE);
                    ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_LESS_EQUAL);
                    render_with_progs(deferred_lights, mdl.getRenderOperation());
                }

                if (mdlInside.indexesUsed() > 0) {
                    ogre_rs->_setCullingMode(Ogre::CULL_ANTICLOCKWISE);
                    ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_GREATER_EQUAL);
                    render_with_progs(deferred_lights, mdlInside.getRenderOperation());
                }

                for (unsigned i=0 ; i<tex_index ; ++i) {
                    ogre_rs->_disableTextureUnit(i);
                }
            }

        } catch (const Exception &e) {
            CERR << "Rendering deferred point lights, got: " << e << std::endl;
        } catch (const Ogre::Exception &e) {
            CERR << "Rendering deferred point lights, got: " << e.getDescription() << std::endl;
        }

    }
    
};

// }}}



// {{{ Decals passes 

class DecalPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    public:
    DecalPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0, Ogre::StringUtil::BLANK), pipe(pipe)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        if (pipe->getCameraOpts().decals) gfx_decal_render(pipe);
    }
    
};

// }}}



// {{{ Particles passes 

class ParticlesPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    public:
    ParticlesPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0, Ogre::StringUtil::BLANK), pipe(pipe)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        if (pipe->getCameraOpts().particles) gfx_particle_render(pipe);
    }
    
};

// }}}



// {{{ Sky passes 

class SkyPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    public:
    SkyPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0), pipe(pipe)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        try {
            if (pipe->getCameraOpts().sky) gfx_sky_render(pipe);
        } catch (Ogre::Exception &e) {
            CERR << "While rendering skies: " << e.getDescription() << std::endl;
        } catch (const Exception &e) {
            CERR << "While rendering skies: " << e << std::endl;
        }
    }
    
};

// }}}



// {{{ Reset pass

class ResetPass : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;
    unsigned int buffers;
    Vector3 colour;
    float depth;
    unsigned short stencil;

    public:
    ResetPass (GfxPipeline *pipe, unsigned int buffers,
               const Vector3 &colour=Vector3(1,1,1), float depth=1.0f, unsigned short stencil=0)
      : Ogre::RenderQueueInvocation(0), pipe(pipe), buffers(buffers),
        colour(colour), depth(depth), stencil(stencil)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        try {
            pipe->getCamera()->getViewport()->clear(buffers, to_ogre_cv(colour), depth, stencil);
        } catch (Ogre::Exception &e) {
            CERR << "While clearing buffer: " << e.getDescription() << std::endl;
        }
    }
    
};

// }}}



// {{{ First Person

class FirstPersonPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;
    bool alphaBlend;

    public:
    FirstPersonPasses (GfxPipeline *pipe, bool alpha_blend)
      : Ogre::RenderQueueInvocation(0), pipe(pipe), alphaBlend(alpha_blend)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        try {
            if (pipe->getCameraOpts().firstPerson) gfx_body_render_first_person(pipe, alphaBlend);
        } catch (Ogre::Exception &e) {
            CERR << "While rendering first person: " << e.getDescription() << std::endl;
        } catch (const Exception &e) {
            CERR << "While rendering first person: " << e << std::endl;
        }
    }
    
};

// }}}



GfxPipeline::GfxPipeline (const std::string &name, Ogre::Viewport *target_viewport)
  : targetViewport(target_viewport)
{

    unsigned width = targetViewport->getActualWidth();
    unsigned height = targetViewport->getActualHeight();

    gBuffer = ogre_rs->createMultiRenderTarget(name+":G");
    // make textures for it
    const char *gbuffer_el_names[] = { ":G0", ":G1", ":G2", ":G3" };
    for (unsigned i=0 ; i<3 ; ++i) {
        gBufferElements[i] =
            Ogre::TextureManager::getSingleton().createManual(
                name+gbuffer_el_names[i], RESGRP, Ogre::TEX_TYPE_2D,
                width, height, 1,
                0,
                Ogre::PF_A8R8G8B8,
                Ogre::TU_RENDERTARGET,
                NULL,
                use_hwgamma);
        gBuffer->bindSurface(i, gBufferElements[i]->getBuffer()->getRenderTarget());
    }


    for (unsigned i=0 ; i<sizeof(hdrFb)/sizeof(*hdrFb) ; ++i) {
        std::stringstream ss; ss << i;
        hdrFb[i] = Ogre::TextureManager::getSingleton().createManual(
                    name+":hdrFb"+ss.str(), RESGRP, Ogre::TEX_TYPE_2D,
                    width, height, 1,
                    0,
                    Ogre::PF_FLOAT16_RGB,
                    Ogre::TU_RENDERTARGET,
                    NULL,
                    false);
    }

    cam = ogre_sm->createCamera(name+":cam");
    cam->setAutoAspectRatio(true);

    rqisGbuffer = ogre_root->createRenderQueueInvocationSequence(name+":gbuffer");
    rqisGbuffer->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_GBUFFER_OPAQUE));

    // Opaque passes
    rqisDeferred = ogre_root->createRenderQueueInvocationSequence(name+":deferred");
    rqisDeferred->add(new DeferredLightingPasses(this));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_OPAQUE));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_OPAQUE_EMISSIVE));
    rqisDeferred->add(new SkyPasses(this));
    // Alpha passes
    rqisDeferred->add(new DecalPasses(this));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA_DEPTH));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA_DEPTH_EMISSIVE));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA_EMISSIVE));
    rqisDeferred->add(new ParticlesPasses(this));
    // Debug passes
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_BULLET_DEBUG_DRAWER));
    // First person passes
    rqisDeferred->add(new ResetPass(this, Ogre::FBT_DEPTH));
    rqisDeferred->add(new FirstPersonPasses(this, false));
    rqisDeferred->add(new FirstPersonPasses(this, true));
}

GfxPipeline::~GfxPipeline (void) {
    ogre_sm->destroyCamera(cam);

    // destroy MRT
    ogre_rs->destroyRenderTarget(gBuffer->getName());
    for (unsigned i=0 ; i<3 ; ++i) {
         Ogre::TextureManager::getSingleton().remove(gBufferElements[i]->getName());
    }

    for (unsigned i=0 ; i<sizeof(hdrFb)/sizeof(*hdrFb) ; ++i) {
        Ogre::TextureManager::getSingleton().remove(hdrFb[i]->getName());
    }

    ogre_root->destroyRenderQueueInvocationSequence(rqisGbuffer->getName());
    ogre_root->destroyRenderQueueInvocationSequence(rqisDeferred->getName());
}

template<unsigned n> struct RenderQuadParams {
    Ogre::TexturePtr texs[n];
    unsigned next;
    GfxShader *shader;
    Ogre::SceneBlendFactor targetBlend;
    RenderQuadParams(GfxShader *shader, Ogre::SceneBlendFactor target_blend=Ogre::SBF_ZERO)
      : next(0), shader(shader), targetBlend(target_blend)
    { }
    RenderQuadParams<n> tex (const Ogre::TexturePtr &t) {
        APP_ASSERT(next < n);
        texs[next] = t;
        next++;
        return *this;
    };
};

template<unsigned n>
static void render_quad (const Ogre::TexturePtr &rtt, const Ogre::Vector4 &win, bool vpsz, const RenderQuadParams<n> &opts)
{
    render_quad(rtt->getBuffer()->getRenderTarget(), win, vpsz, opts);
}

template<unsigned n>
static void render_quad (Ogre::RenderTarget *rt, const Ogre::Vector4 &win, bool vpsz, const RenderQuadParams<n> &opts)
{
    Ogre::Viewport *vp = rt->addViewport(NULL, 0, win.x, win.y, win.z, win.w);
    if (vpsz) {
        Ogre::Vector4 viewport_size(     vp->getActualWidth(),      vp->getActualHeight(),
                                    1.0f/vp->getActualWidth(), 1.0f/vp->getActualHeight());
        try_set_named_constant(opts.shader->getHackOgreFragmentProgram(), "viewport_size", viewport_size);
    }
    render_quad(vp, opts);
    rt->removeViewport(vp->getZOrder());
}

template<unsigned n>
static void render_quad (Ogre::Viewport *viewport, const RenderQuadParams<n> &opts)
{
        Ogre::HighLevelGpuProgramPtr vp = opts.shader->getHackOgreVertexProgram();
        float render_target_flipping = viewport->getTarget()->requiresTextureFlipping() ? -1.0f : 1.0f;
        try_set_named_constant(vp, "render_target_flipping", render_target_flipping);
        if (d3d9) {
            Ogre::Vector4 viewport_size(     viewport->getActualWidth(),      viewport->getActualHeight(),
                                        1.0f/viewport->getActualWidth(), 1.0f/viewport->getActualHeight());
            try_set_named_constant(vp, "viewport_size", viewport_size);
        }

        ogre_rs->_setViewport(viewport);
        ogre_rs->_beginFrame();

        ogre_rs->_setCullingMode(Ogre::CULL_NONE);
        ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);

        for (unsigned i=0 ; i<n ; ++i) {
            ogre_rs->_setTexture(i, true, opts.texs[i]);
            ogre_rs->_setTextureUnitFiltering(i, Ogre::FT_MIN, Ogre::FO_LINEAR);
            ogre_rs->_setTextureUnitFiltering(i, Ogre::FT_MAG, Ogre::FO_LINEAR);
            ogre_rs->_setTextureUnitFiltering(i, Ogre::FT_MIP, Ogre::FO_NONE);
            Ogre::TextureUnitState::UVWAddressingMode am;
            am.u = Ogre::TextureUnitState::TAM_CLAMP;
            am.v = Ogre::TextureUnitState::TAM_CLAMP;
            am.w = Ogre::TextureUnitState::TAM_CLAMP;
            ogre_rs->_setTextureAddressingMode(i, am);
        }

        ogre_rs->_setCullingMode(Ogre::CULL_NONE);
        ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);
        ogre_rs->_setSceneBlending(Ogre::SBF_ONE, opts.targetBlend);
        ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
        ogre_rs->setStencilCheckEnabled(false);
        ogre_rs->_setDepthBias(0, 0);


        // render the quad
        Ogre::RenderOperation op;
        op.useIndexes = false;
        op.vertexData = screen_quad_vdata;
        op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        render_with_progs(opts.shader, op);

        for (unsigned i=0 ; i<n ; ++i) ogre_rs->_disableTextureUnit(i);

        ogre_rs->_endFrame();

}

static void clear_rt (Ogre::RenderTarget *rt)
{
    Ogre::Viewport *vp = rt->addViewport(NULL);
    ogre_rs->_setViewport(vp);
    ogre_rs->clearFrameBuffer(Ogre::FBT_COLOUR);
    rt->removeViewport(vp->getZOrder());
}


void GfxPipeline::render (const CameraOpts &cam_opts, bool additive)
{
    opts = cam_opts;

    cam->setPosition(to_ogre(opts.pos));
    // Ogre cameras point towards Z whereas in Grit the convention is that 'unrotated' means pointing towards y (north)
    cam->setOrientation(to_ogre(opts.dir*Quaternion(Degree(90),Vector3(1,0,0))));
    cam->setFOVy(Ogre::Degree(opts.fovY));
    cam->setNearClipDistance(opts.nearClip);
    cam->setFarClipDistance(opts.farClip);
    cam->setFrustumOffset(opts.frustumOffset);
    cam->setFocalLength(1);

    Ogre::Viewport *vp;

    // populate gbuffer
    vp = gBuffer->addViewport(cam);
    vp->setShadowsEnabled(true);
    // white here makes sure that the depth (remember that it is 3 bytes) is maximal
    vp->setBackgroundColour(Ogre::ColourValue::White);
    vp->setRenderQueueInvocationSequenceName(rqisGbuffer->getName());
    unsigned long long micros_before = micros();
    vp->update();
    unsigned long long micros_after_gbuffer = micros();
    gBuffer->removeViewport(vp->getZOrder());
    gBufferStats.batches = ogre_rs->_getBatchCount();
    gBufferStats.triangles = ogre_rs->_getFaceCount();;
    gBufferStats.micros = micros_after_gbuffer - micros_before;

    if (!opts.bloomAndToneMap) {

        // render gbuffer and alpha, sky, etc into ldr window
        vp = targetViewport;
        vp->setCamera(cam);
        vp->setShadowsEnabled(false);
        vp->setRenderQueueInvocationSequenceName(rqisDeferred->getName());
        vp->update();
        unsigned long long micros_after_deferred = micros();
        deferredStats.batches = ogre_rs->_getBatchCount();
        deferredStats.triangles = ogre_rs->_getFaceCount();
        deferredStats.micros = micros_after_deferred - micros_after_gbuffer;

    } else {

        // render from gbuffer, the alpha passes, sky, etc into hdr viewport
        vp = hdrFb[0]->getBuffer()->getRenderTarget()->addViewport(cam);
        vp->setShadowsEnabled(false);
        vp->setRenderQueueInvocationSequenceName(rqisDeferred->getName());
        vp->update();
        unsigned long long micros_after_deferred = micros();
        deferredStats.batches = ogre_rs->_getBatchCount();
        deferredStats.triangles = ogre_rs->_getFaceCount();
        deferredStats.micros = micros_after_deferred - micros_after_gbuffer;
        hdrFb[0]->getBuffer()->getRenderTarget()->removeViewport(vp->getZOrder());


        Ogre::SceneBlendFactor target_blend = additive ? Ogre::SBF_ONE : Ogre::SBF_ZERO;

        unsigned bloom_iterations = gfx_option(GFX_BLOOM_ITERATIONS);
        if (bloom_iterations == 0) {

            get_compositor_tonemap()->validate();
            auto fp = compositor_tonemap->getHackOgreFragmentProgram();
            try_set_named_constant(fp, "global_exposure", to_ogre(global_exposure*opts.mask));
            try_set_named_constant(fp, "global_saturation", global_saturation*opts.saturationMask);
            if (colour_grade_lut == NULL) {
                render_quad(targetViewport, RenderQuadParams<1>(compositor_tonemap, target_blend).tex(hdrFb[0]));
            } else {
                const Ogre::TexturePtr &lut = colour_grade_lut->getOgreTexturePtr();
                render_quad(targetViewport, RenderQuadParams<2>(compositor_tonemap, target_blend).tex(hdrFb[0]).tex(lut));
            }

        } else {

            float bloom_threshold = gfx_option(GFX_BLOOM_THRESHOLD);

            get_compositor_bloom_filter_then_horz_blur()->validate();
            auto bloom_filter_then_horz_blur = compositor_bloom_filter_then_horz_blur->getHackOgreFragmentProgram();
            try_set_named_constant(bloom_filter_then_horz_blur, "bloom_threshold", bloom_threshold/global_exposure);

            get_compositor_bloom_vert_blur()->validate();
            auto bloom_vert_blur = compositor_bloom_vert_blur->getHackOgreFragmentProgram();

            get_compositor_bloom_horz_blur()->validate();
            auto bloom_horz_blur = compositor_bloom_horz_blur->getHackOgreFragmentProgram();

            get_compositor_bloom_vert_blur_combine_and_tonemap()->validate();
            auto bloom_vert_blur_combine_and_tonemap = compositor_bloom_vert_blur_combine_and_tonemap->getHackOgreFragmentProgram();
            try_set_named_constant(bloom_vert_blur_combine_and_tonemap, "global_exposure", to_ogre(global_exposure*opts.mask));
            try_set_named_constant(bloom_vert_blur_combine_and_tonemap, "global_saturation", global_saturation*opts.saturationMask);

            // half_bloom = filter_then_horz_blur(raw)
            // loop 1..n-1 {
            //     bloom = vert_blur(half_bloom);
            //     half_bloom = scale_then_horz_blur(bloom);
            // }
            // fb = vert_blur_combine_and_tonemap(raw, half_bloom) 

            Ogre::Vector4 vp_dim(0,0,1,1);

            try_set_named_constant(bloom_filter_then_horz_blur, "bloom_tex_scale", Ogre::Vector4(vp_dim.z, 0, 0, 0));
            render_quad(hdrFb[1], vp_dim, true, RenderQuadParams<1>(compositor_bloom_filter_then_horz_blur).tex(hdrFb[0]));
            for (unsigned i=1 ; i<bloom_iterations ; ++i) {
                try_set_named_constant(bloom_vert_blur, "bloom_tex_scale", Ogre::Vector4(vp_dim.z, 0, 0, 0));
                clear_rt(hdrFb[2]->getBuffer()->getRenderTarget());
                render_quad(hdrFb[2], vp_dim, true, RenderQuadParams<1>(compositor_bloom_vert_blur).tex(hdrFb[1]));
                try_set_named_constant(bloom_horz_blur, "bloom_tex_scale", Ogre::Vector4(vp_dim.z, 0, 0, 0));
                vp_dim /= 2;
                clear_rt(hdrFb[1]->getBuffer()->getRenderTarget());
                render_quad(hdrFb[1], vp_dim, true, RenderQuadParams<1>(compositor_bloom_horz_blur).tex(hdrFb[2]));
            }
            try_set_named_constant(bloom_vert_blur_combine_and_tonemap, "bloom_tex_scale", Ogre::Vector4(vp_dim.z, 0, 0, 0));
            Ogre::Vector4 viewport_size(     targetViewport->getActualWidth(),      targetViewport->getActualHeight(),
                                        1.0f/targetViewport->getActualWidth(), 1.0f/targetViewport->getActualHeight());
            try_set_named_constant(bloom_vert_blur_combine_and_tonemap, "viewport_size", viewport_size);
            if (colour_grade_lut == NULL) {
                render_quad(targetViewport, RenderQuadParams<2>(compositor_bloom_vert_blur_combine_and_tonemap, target_blend).tex(hdrFb[1]).tex(hdrFb[0]));
            } else {
                const Ogre::TexturePtr &lut = colour_grade_lut->getOgreTexturePtr();
                render_quad(targetViewport, RenderQuadParams<3>(compositor_bloom_vert_blur_combine_and_tonemap, target_blend).tex(hdrFb[1]).tex(hdrFb[0]).tex(lut));
            }
        }
    }

}
