/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
 *
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

#include <sstream>

#include <Ogre.h>
#include <OgreFontManager.h>
#include <OgreMeshManager.h>
#include <OgreOverlayElementFactory.h>
#include <OgreOctreeSceneManager.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreCustomCompositionPass.h>
#include <OgreCompositor.h>
#ifdef NO_PLUGINS
#  include "OgreOctreePlugin.h"
#  include "OgreGLPlugin.h"
#  include "OgreCgPlugin.h"
#  ifdef WIN32
#    include "OgreD3D9Plugin.h"
#  endif
#endif

#include "gfx.h"
#include "HUD.h"
#include "Clutter.h"
#include "../path_util.h"
#include "../math_util.h"


bool use_hwgamma = false; //getenv("GRIT_NOHWGAMMA")==NULL;

#define ANAGLYPH_COMPOSITOR "system/AnaglyphCompositor"
#define ANAGLYPH_FB_TEXTURE "system/AnaglyphFB"
#define ANAGLYPH_COMPOSITOR_MATERIAL "system/AnaglyphCompositorMaterialDesaturated"

#define SKY_MESH "system/SkyCube.mesh"

#define DEFERRED_COMPOSITOR "/system/CoreCompositor"
#define DEFERRED_AMBIENT_SUN_MATERIAL "/system/DeferredAmbientSun"
#define DEFERRED_POINT_LIGHT_MATERIAL "/system/DeferredLights"
#define DEFERRED_POINT_LIGHT_INSIDE_MATERIAL "/system/DeferredLightsInside"
#define DEFERRED_LIGHTS_CUSTOM_PASS "DeferredLights"
#define CORONAS_CUSTOM_PASS "Coronas"

#define RESGRP "GRIT"

// render queues
#define RQ_GBUFFER_OPAQUE 50
#define RQ_FORWARD_OPAQUE 51
#define RQ_SKY 60
#define RQ_FORWARD_EMISSIVE 61
#define RQ_FORWARD_ALPHA_DEPTH 94
#define RQ_FORWARD_ALPHA 95

Ogre::Root *ogre_root;
Ogre::OctreeSceneManager *ogre_sm;
Ogre::SceneNode *ogre_root_node;
Ogre::Camera *left_eye;
Ogre::Camera *right_eye;
Ogre::Light *ogre_sun;
Ogre::SceneNode *ogre_celestial;
Ogre::SceneNode *ogre_sky_node;
Ogre::Entity *ogre_sky_ent;
Ogre::Viewport *overlay_vp;
Ogre::Viewport *left_vp;
Ogre::Viewport *right_vp;
Ogre::TexturePtr anaglyph_fb;
Ogre::RenderWindow *ogre_win;
#ifdef NO_PLUGINS
    Ogre::GLPlugin *gl;
    Ogre::OctreePlugin *octree;
    //Ogre::ParticleFXPlugin *pfx;
    Ogre::CgPlugin *cg;
    #ifdef WIN32
        Ogre::D3D9Plugin *d3d9;
    #endif
#endif


GfxCallback *gfx_cb;
bool shutting_down = false;
float cam_separation = 0;

Vector3 fog_colour;
float fog_density;


// {{{ utilities

bool stereoscopic (void) 
{ return gfx_option(GFX_CROSS_EYE) || gfx_option(GFX_ANAGLYPH); }

int freshname_counter = 0;
std::string freshname (const std::string &prefix)
{
    std::stringstream ss;
    ss << prefix << freshname_counter;
    freshname_counter++;
    return ss.str();
}
std::string freshname (void)
{
    return freshname("F:");
}

template<class T> bool between (T x, T m, T M) { return std::less<T>()(m,x)&&std::less<T>()(x,M); }

// }}}



// {{{ FRAMEBUFFER / COMPOSITOR

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

#define AMBIENT_SUN_PASS_ID 42

// {{{ Ambient / sun light

class DeferredAmbientSunListener : public Ogre::CompositorInstance::Listener {
public:
    bool left;
    DeferredAmbientSunListener (bool left_) : left(left_) { }
    virtual ~DeferredAmbientSunListener (void) { }

    virtual void notifyMaterialRender (Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        if (pass_id != AMBIENT_SUN_PASS_ID) return;

        //CVERB << "Setting ambient sun params for material " << mat->getName() << std::endl;

        // set params on material
        Ogre::Pass *p = mat->getTechnique(0)->getPass(0);
        Ogre::Camera *cam = left ? left_eye : right_eye;
        const Ogre::Vector3 &cam_pos = cam->getDerivedPosition();

        // corners mapping:
        // top-right near, top-left near, bottom-left near, bottom-right near,
        // top-right far, top-left far, bottom-left far, bottom-right far. 
        Ogre::Vector3 top_right_ray = cam->getWorldSpaceCorners()[4] - cam_pos;
        Ogre::Vector3 top_left_ray = cam->getWorldSpaceCorners()[5] - cam_pos;
        Ogre::Vector3 bottom_left_ray = cam->getWorldSpaceCorners()[6] - cam_pos;
        Ogre::Vector3 bottom_right_ray = cam->getWorldSpaceCorners()[7] - cam_pos;

        const Ogre::GpuProgramParametersSharedPtr &vp = p->getVertexProgramParameters();
        const Ogre::GpuProgramParametersSharedPtr &fp = p->getFragmentProgramParameters();

        try_set_named_constant(vp, "top_left_ray", top_left_ray);
        try_set_named_constant(vp, "top_right_ray", top_right_ray);
        try_set_named_constant(vp, "bottom_left_ray", bottom_left_ray);
        try_set_named_constant(vp, "bottom_right_ray", bottom_right_ray);

        //try_set_named_constant(fp, "near_clip_distance", cam->getNearClipDistance());
        try_set_named_constant(fp, "far_clip_distance", cam->getFarClipDistance());
        try_set_named_constant(fp, "camera_pos_ws", cam_pos);

        // actually we need only the z and w rows but this is just one renderable per frame so
        // not a big deal
        Ogre::Matrix4 view = cam->getViewMatrix();
        //try_set_named_constant(fp, "view", view);
        Ogre::Matrix4 proj = cam->getProjectionMatrix();
        //try_set_named_constant(fp, "proj", proj);
        try_set_named_constant(fp, "view_proj", proj*view);
        Ogre::Vector3 sun_pos_ws = ogre_sun->getDirection();
        try_set_named_constant(fp, "sun_pos_ws", -sun_pos_ws);
        Ogre::ColourValue sun_diffuse = ogre_sun->getDiffuseColour();
        try_set_named_constant(fp, "sun_diffuse", sun_diffuse);
        Ogre::ColourValue sun_specular = ogre_sun->getSpecularColour();
        try_set_named_constant(fp, "sun_specular", sun_specular);
    }
};

DeferredAmbientSunListener left_dasl(true);
DeferredAmbientSunListener right_dasl(false);

// }}}

// anything that might need removing will get removed
static void clean_compositors (void)
{
        if (left_vp != NULL) {
            if (Ogre::CompositorManager::getSingleton().hasCompositorChain(left_vp)) {
                Ogre::CompositorManager::getSingleton()
                    .removeCompositor(left_vp,ANAGLYPH_COMPOSITOR);
                Ogre::CompositorManager::getSingleton()
                    .removeCompositor(left_vp,DEFERRED_COMPOSITOR);
                Ogre::CompositorManager::getSingleton().removeCompositorChain(left_vp);
            }
        }

        if (right_vp != NULL) {
            if (Ogre::CompositorManager::getSingleton().hasCompositorChain(right_vp)) {
                Ogre::CompositorManager::getSingleton()
                    .removeCompositor(right_vp,DEFERRED_COMPOSITOR);
                Ogre::CompositorManager::getSingleton().removeCompositorChain(right_vp);
            }
        }

}

typedef Ogre::CompositorInstance CI;
typedef Ogre::CompositionPass CP;

// {{{ Multiple small lights

#ifdef WIN32
typedef unsigned short uint16_t;
#endif

class MultiDeferredLight : public Ogre::Renderable
{
protected:

    Ogre::MaterialPtr mMaterial;
    Ogre::RenderOperation mRenderOperation;
    Ogre::VertexData mVertexData;
    Ogre::IndexData mIndexData;

    Ogre::LightList emptyLightList;

    unsigned mDeclSize;
    unsigned mMaxVertexesGPU;
    unsigned mMaxIndexesGPU;

    float *mVertexPtr0; // hold the base pointer
    uint16_t *mIndexPtr0; // hold the base pointer
    

public:

    MultiDeferredLight (const Ogre::MaterialPtr &m)
      :
        mMaterial(m),
        mDeclSize(0),
        mMaxVertexesGPU(0), // increase later
        mMaxIndexesGPU(0), // increase later
        mVertexPtr0(NULL),
        mIndexPtr0(NULL)
    {
        APP_ASSERT(!mMaterial.isNull());

        mRenderOperation.useIndexes = false;
        mRenderOperation.vertexData = &mVertexData;
        mRenderOperation.indexData = &mIndexData;
        mRenderOperation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        mRenderOperation.useIndexes = true;

        // position on screen (just distinguishes the corners frome each other, -1 or 1)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_POSITION);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // direction the light is pointing (same for all 4 corners of the quad)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_NORMAL);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // diffuse colour
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_DIFFUSE);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // specular colour
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_SPECULAR);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // light centre
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_TEXTURE_COORDINATES, 0);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // inner/outer cone dot product and range
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_TEXTURE_COORDINATES, 1);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

    }

    ~MultiDeferredLight (void)
    {
        // everything else cleaned up by destructors and Ogre::SharedPtr for the vbuf
    }

    Ogre::RenderOperation *getRenderOperation (void) { return &mRenderOperation; }
    
    // Renderable overrides
    const Ogre::MaterialPtr& getMaterial (void) const { return mMaterial; }
    void getRenderOperation (Ogre::RenderOperation& op) { op = mRenderOperation; }

    void getWorldTransforms (Ogre::Matrix4* xform) const
    { xform[0] = Ogre::Matrix4::IDENTITY; }
    Ogre::Real getSquaredViewDepth (const Ogre::Camera *) const { return 0; }
    const Ogre::LightList &getLights (void) const { return emptyLightList; }

    unsigned indexesUsed (uint16_t *&iptr) { return iptr - mIndexPtr0; }
    unsigned vertexesUsed (float *&vptr) { return ((vptr-mVertexPtr0)*sizeof(float)) / mDeclSize; }

    void beginLights (unsigned lights, float *&vptr, uint16_t *&iptr)
    {
        unsigned vertexes = lights * 8; // assume worst case -- all cubes
        if (vertexes > mMaxVertexesGPU) {
            mMaxVertexesGPU = vertexes;
            Ogre::HardwareVertexBufferSharedPtr vbuf =
                Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                    mDeclSize, vertexes,
                    Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
            mVertexData.vertexBufferBinding->setBinding(0, vbuf);
        }

        unsigned indexes = lights * 36; // assume worst case -- all cubes
        if (indexes > mMaxIndexesGPU) {
            mMaxIndexesGPU = indexes;
            mIndexData.indexBuffer = Ogre::HardwareBufferManager::getSingleton().
                createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, indexes,
                                  Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
        }

        mVertexPtr0 = vptr = static_cast<float*>(mVertexData.vertexBufferBinding->getBuffer(0)
                                                 ->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        mIndexPtr0 = iptr = static_cast<uint16_t*>(mIndexData.indexBuffer
                                                   ->lock(Ogre::HardwareBuffer::HBL_DISCARD));
    }

    #define quad_vertexes(a,b,c,d) a, b, d, a, d, c

    void pyramidLight (float *&vptr, uint16_t *&iptr, const Ogre::Vector3 &wpos,
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

        unsigned offset = vertexesUsed(vptr);
        for (int j=0 ; j<5 ; ++j) {
            // position
            *(vptr++) = vertexes[j].x;
            *(vptr++) = vertexes[j].y;
            *(vptr++) = vertexes[j].z;
            // direction
            *(vptr++) = dir_ws.x;
            *(vptr++) = dir_ws.y;
            *(vptr++) = dir_ws.z;
            // diffuse colour
            *(vptr++) = diff.r;
            *(vptr++) = diff.g;
            *(vptr++) = diff.b;
            // specular colour
            *(vptr++) = spec.r;
            *(vptr++) = spec.g;
            *(vptr++) = spec.b;
            // centre position
            *(vptr++) = wpos.x;
            *(vptr++) = wpos.y;
            *(vptr++) = wpos.z;
            // inner/outer cone dot product, range
            *(vptr++) = inner;
            *(vptr++) = outer;
            *(vptr++) = range;
        }
        for (int j=0 ; j<18 ; ++j) {
            *(iptr++) = offset+indexes[j];
        }
    }

    void cubeLight (float *&vptr, uint16_t *&iptr, const Ogre::Vector3 &wpos,
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

        unsigned offset = vertexesUsed(vptr);
        for (int j=0 ; j<8 ; ++j) {
            // position
            *(vptr++) = vertexes[j].x;
            *(vptr++) = vertexes[j].y;
            *(vptr++) = vertexes[j].z;
            // direction
            *(vptr++) = dir_ws.x;
            *(vptr++) = dir_ws.y;
            *(vptr++) = dir_ws.z;
            // diffuse colour
            *(vptr++) = diff.r;
            *(vptr++) = diff.g;
            *(vptr++) = diff.b;
            // specular colour
            *(vptr++) = spec.r;
            *(vptr++) = spec.g;
            *(vptr++) = spec.b;
            // centre position
            *(vptr++) = wpos.x;
            *(vptr++) = wpos.y;
            *(vptr++) = wpos.z;
            // inner/outer cone dot product, range
            *(vptr++) = inner;
            *(vptr++) = outer;
            *(vptr++) = range;
        }
        for (int j=0 ; j<36 ; ++j) {
            *(iptr++) = offset+indexes[j];
        }
    }

    void endLights (float *&vptr, uint16_t *&iptr, Ogre::SceneManager *sm, Ogre::Viewport *viewport)
    {
        mVertexData.vertexBufferBinding->getBuffer(0)->unlock();
        mIndexData.indexBuffer->unlock();

        mVertexData.vertexCount = vertexesUsed(vptr);
        mIndexData.indexCount = indexesUsed(iptr);

        mMaterial->load();
        Ogre::Pass *p = mMaterial->getTechnique(0)->getPass(0);

        // set params on material
        Ogre::Camera *cam = viewport->getCamera();
        const Ogre::Vector3 &cam_pos = cam->getDerivedPosition();

        // corners mapping:
        // top-right near, top-left near, bottom-left near, bottom-right near,
        // top-right far, top-left far, bottom-left far, bottom-right far. 
        Ogre::Vector3 top_right_ray = cam->getWorldSpaceCorners()[4] - cam_pos;
        Ogre::Vector3 top_left_ray = cam->getWorldSpaceCorners()[5] - cam_pos;
        Ogre::Vector3 bottom_left_ray = cam->getWorldSpaceCorners()[6] - cam_pos;
        Ogre::Vector3 bottom_right_ray = cam->getWorldSpaceCorners()[7] - cam_pos;

        const Ogre::GpuProgramParametersSharedPtr &vp = p->getVertexProgramParameters();
        const Ogre::GpuProgramParametersSharedPtr &fp = p->getFragmentProgramParameters();

        try_set_named_constant(fp, "top_left_ray", top_left_ray);
        try_set_named_constant(fp, "top_right_ray", top_right_ray);
        try_set_named_constant(fp, "bottom_left_ray", bottom_left_ray);
        try_set_named_constant(fp, "bottom_right_ray", bottom_right_ray);

        try_set_named_constant(fp, "far_clip_distance", cam->getFarClipDistance());
        try_set_named_constant(fp, "camera_pos_ws", cam_pos);

        Ogre::Matrix4 view = cam->getViewMatrix();
        //try_set_named_constant(vp, "view", view);
        Ogre::Matrix4 proj = cam->getProjectionMatrixWithRSDepth();
        //try_set_named_constant(vp, "proj", proj);
        try_set_named_constant(vp, "view_proj", proj*view);

        sm->_injectRenderWithPass(p, this, false);
    }

};

class DeferredLightsRenderOp : public CI::RenderSystemOperation {   

    Ogre::Viewport *viewport;
    MultiDeferredLight mdl;
    MultiDeferredLight mdl_inside;

public:

    DeferredLightsRenderOp (CI* ins, const CP* c_pass)
      : viewport(ins->getChain()->getViewport()),
        mdl(Ogre::MaterialManager::getSingleton().getByName(DEFERRED_POINT_LIGHT_MATERIAL, RESGRP)),
        mdl_inside(Ogre::MaterialManager::getSingleton()
                   .getByName(DEFERRED_POINT_LIGHT_INSIDE_MATERIAL, RESGRP))
    {
        (void) c_pass;
    }
    
    virtual ~DeferredLightsRenderOp (void)
    {
        // everything cleaned up by destructors
    }

    virtual void execute (Ogre::SceneManager *sm, Ogre::RenderSystem *rs)
    {
        (void) rs;
        const Ogre::LightList &ll = sm->_getLightsAffectingFrustum();

        float *mdl_vptr;
        uint16_t *mdl_iptr;
        mdl.beginLights(ll.size(), mdl_vptr, mdl_iptr);
        float *mdl_inside_vptr;
        uint16_t *mdl_inside_iptr;
        mdl_inside.beginLights(ll.size(), mdl_inside_vptr, mdl_inside_iptr);

        int light_counter = 0;
        Ogre::Camera *cam = viewport->getCamera();
        const Ogre::Vector3 &cam_pos = cam->getDerivedPosition();
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

            float *&vp = inside ? mdl_inside_vptr : mdl_vptr;
            uint16_t *&ip = inside ? mdl_inside_iptr : mdl_iptr;
            MultiDeferredLight &mdl_ = inside ? mdl_inside : mdl;

            if (use_pyramid) {
                mdl_.pyramidLight(vp,ip, wpos, cuboid_planes, dir_ws,diff,spec,inner,outer,range);
            } else {
                mdl_.cubeLight(vp,ip, wpos, cuboid_planes, dir_ws,diff,spec,inner,outer,range);
            }
        }
        mdl.endLights(mdl_vptr, mdl_iptr, sm, viewport);
        mdl_inside.endLights(mdl_inside_vptr, mdl_inside_iptr, sm, viewport);
        //CVERB << "Total lights: " << light_counter << std::endl;
    }
};
        
class DeferredLightsPass : public Ogre::CustomCompositionPass {   
    virtual ~DeferredLightsPass() { }

public:

    virtual CI::RenderSystemOperation* createOperation(CI* ins, const CP* p)
    {
        return OGRE_NEW DeferredLightsRenderOp(ins, p);
    }
};

// }}}

static void init_compositor (void)
{
    Ogre::CompositorManager::getSingleton()
        .registerCustomCompositionPass(DEFERRED_LIGHTS_CUSTOM_PASS, new DeferredLightsPass());
}


static void add_deferred_compositor (bool left)
{
    Ogre::Viewport *vp = left ? left_vp : right_vp;
    CI *def_comp = Ogre::CompositorManager::getSingleton()
        .addCompositor(vp,DEFERRED_COMPOSITOR);
    APP_ASSERT(def_comp!=NULL);
    def_comp->setEnabled(true);
    def_comp->addListener(left ? &left_dasl : &right_dasl);
    Ogre::CompositionTechnique *t = def_comp->getTechnique();
    size_t np = t->getNumTargetPasses();
    Ogre::CompositionTargetPass *tp = np<=1 ? t->getOutputTargetPass() : t->getTargetPass(1);
    for (unsigned i=0 ; i<tp->getNumPasses() ; ++i) {
        Ogre::CompositionPass *pass = tp->getPass(i);
        // clear passes don't hvae a material?
        if (!pass->getMaterial().isNull()) {
            if (pass->getMaterial()->getName() == DEFERRED_AMBIENT_SUN_MATERIAL) {
                pass->setIdentifier(AMBIENT_SUN_PASS_ID);
                break;
            }
        }
    }
}

static void do_reset_compositors (void)
{
    if (gfx_option(GFX_DEFERRED)) {
        add_deferred_compositor(true);
        if (gfx_option(GFX_CROSS_EYE)) {
            add_deferred_compositor(false);
        } else if (gfx_option(GFX_ANAGLYPH)) {
            add_deferred_compositor(false);
        }
    }

    if (gfx_option(GFX_CROSS_EYE)) {
    } else if (gfx_option(GFX_ANAGLYPH)) {
        Ogre::CompositorManager::getSingleton()
            .addCompositor(left_vp,ANAGLYPH_COMPOSITOR);
        Ogre::CompositorManager::getSingleton()
            .setCompositorEnabled(left_vp,ANAGLYPH_COMPOSITOR,true);
    }
}

void do_reset_framebuffer (void)
{
    // get rid of everything that might be set up already

    if (right_vp != NULL) {
        right_vp->getTarget()->removeViewport(right_vp->getZOrder());
        right_vp = NULL;
    }

    if (!anaglyph_fb.isNull()) {
        Ogre::TextureManager::getSingleton().remove(ANAGLYPH_FB_TEXTURE);
        anaglyph_fb.setNull();
    }

    // set things up again

    if (gfx_option(GFX_CROSS_EYE)) {
        left_vp->setDimensions(0,0,0.5,1);
    } else {
        left_vp->setDimensions(0,0,1,1);
    }

    if (gfx_option(GFX_CROSS_EYE)) {
        right_vp = ogre_win->addViewport(right_eye, 2, 0.5,0,0.5,1);
        right_vp->setOverlaysEnabled(false);
    } else if (gfx_option(GFX_ANAGLYPH)) {
        // ok if we're using anaglyph rendering we'll need a separate rtt for the right eye

        unsigned width = left_vp->getActualWidth();
        unsigned height = left_vp->getActualHeight();
        anaglyph_fb = Ogre::TextureManager::getSingleton().createManual(
            ANAGLYPH_FB_TEXTURE, RESGRP, Ogre::TEX_TYPE_2D,
            width, height, 1,
            0,
            Ogre::PF_R8G8B8,
            Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE | Ogre::TU_RENDERTARGET ,
            NULL,
            use_hwgamma);

        // and a viewport for it, linked to the right eye camera
        right_vp = anaglyph_fb->getBuffer()->getRenderTarget()
            ->addViewport(right_eye,2,0,0,1,1);
        right_vp->setOverlaysEnabled(false);

        // since we've recreated the texture, need to update the material
        Ogre::MaterialPtr rtt_mat = Ogre::MaterialManager::getSingleton()
            .getByName(ANAGLYPH_COMPOSITOR_MATERIAL);
        rtt_mat->load();
        rtt_mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)
               ->setTextureName(ANAGLYPH_FB_TEXTURE);
    }
}

// }}}



// {{{ GFX_OPTION

GfxBoolOption gfx_bool_options[] = {
    GFX_AUTOUPDATE,
    GFX_SHADOW_RECEIVE,
    GFX_SHADOW_CAST,
    GFX_VSYNC,
    GFX_FULLSCREEN,
    GFX_FOG,
    GFX_DEFERRED,
    GFX_WIREFRAME,
    GFX_ANAGLYPH,
    GFX_CROSS_EYE,
    GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST,
    GFX_SHADOW_AGGRESSIVE_FOCUS_REGION,
    GFX_SKY,
};

GfxIntOption gfx_int_options[] = {
    GFX_FULLSCREEN_WIDTH,
    GFX_FULLSCREEN_HEIGHT,
    GFX_SHADOW_RES,
    GFX_RAM,
};

GfxFloatOption gfx_float_options[] = {
    GFX_FOV,
    GFX_NEAR_CLIP,
    GFX_FAR_CLIP,
    GFX_EYE_SEPARATION,
    GFX_MONITOR_HEIGHT,
    GFX_MONITOR_EYE_DISTANCE,
    GFX_MIN_PERCEIVED_DEPTH,
    GFX_MAX_PERCEIVED_DEPTH,
    GFX_SHADOW_START,
    GFX_SHADOW_END0,
    GFX_SHADOW_END1,
    GFX_SHADOW_END2,
    GFX_SHADOW_OPTIMAL_ADJUST0,
    GFX_SHADOW_OPTIMAL_ADJUST1,
    GFX_SHADOW_OPTIMAL_ADJUST2,
    GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD,
    GFX_SHADOW_PADDING,
    GFX_ANAGLYPH_LEFT_RED_MASK,
    GFX_ANAGLYPH_LEFT_GREEN_MASK,
    GFX_ANAGLYPH_LEFT_BLUE_MASK,
    GFX_ANAGLYPH_RIGHT_RED_MASK,
    GFX_ANAGLYPH_RIGHT_GREEN_MASK,
    GFX_ANAGLYPH_RIGHT_BLUE_MASK,
    GFX_ANAGLYPH_DESATURATION,
};

static std::map<GfxBoolOption,bool> options_bool;
static std::map<GfxIntOption,int> options_int;
static std::map<GfxFloatOption,float> options_float;
static std::map<GfxBoolOption,bool> new_options_bool;
static std::map<GfxIntOption,int> new_options_int;
static std::map<GfxFloatOption,float> new_options_float;

template<class T> struct ValidOption {
    virtual ~ValidOption (void) { }
    virtual bool isValid (T v) = 0;
    virtual void err (std::ostream &o) = 0;
    virtual void maybeThrow (T v)
    {
        if (!isValid(v)) {
            std::stringstream ss;
            ss << "Graphics option: new value " << v << ": ";
            err(ss);
            GRIT_EXCEPT(ss.str());
        }
    }
};

template<class T> struct ValidOptionRange : ValidOption<T> {
    T min, max;
    ValidOptionRange (T min_, T max_) : min(min_), max(max_) { }
    virtual bool isValid (T v) { return v>=min && v<=max; }
    virtual void err (std::ostream &o) { o << "must be between "<<min<<" and "<<max; }
};

template<class T, class A> struct ValidOptionList : ValidOption<T> {
    A list;
    ValidOptionList (A &list_)
    {
        for (unsigned i=0 ; i<sizeof(A)/sizeof(T) ; ++i)
            list[i] = list_[i];
    }
    virtual bool isValid (T v)
    {
        for (unsigned i=0 ; i<sizeof(A)/sizeof(T) ; ++i)
            if (v==list[i]) return true;
        return false;
    }
    virtual void err (std::ostream &o)
    {
        o << "must be one of [";
        for (unsigned i=0 ; i<sizeof(A)/sizeof(T) ; ++i)
            o << (i==0?"":", ") << list[i];
        o << "]";
    }
};

static std::map<GfxBoolOption,ValidOption<bool>*> valid_option_bool;
static std::map<GfxIntOption,ValidOption<int>*> valid_option_int;
static std::map<GfxFloatOption,ValidOption<float>*> valid_option_float;

static void valid_option (GfxBoolOption o, ValidOption<bool> *v) { valid_option_bool[o] = v; }
static void valid_option (GfxIntOption o, ValidOption<int> *v) { valid_option_int[o] = v; }
static void valid_option (GfxFloatOption o, ValidOption<float> *v) { valid_option_float[o] = v; }

static bool truefalse_[] = { false, true };
static ValidOptionList<bool,bool[2]> *truefalse = new ValidOptionList<bool,bool[2]>(truefalse_);

std::string gfx_option_to_string (GfxBoolOption o)
{
    switch (o) {
        case GFX_AUTOUPDATE: return "GFX_AUTOUPDATE";
        case GFX_SHADOW_RECEIVE: return "GFX_SHADOW_RECEIVE";
        case GFX_SHADOW_CAST: return "GFX_SHADOW_CAST";
        case GFX_VSYNC: return "GFX_VSYNC";
        case GFX_FULLSCREEN: return "GFX_FULLSCREEN";
        case GFX_FOG: return "GFX_FOG";
        case GFX_DEFERRED: return "GFX_DEFERRED";
        case GFX_WIREFRAME: return "GFX_WIREFRAME";
        case GFX_ANAGLYPH: return "GFX_ANAGLYPH";
        case GFX_CROSS_EYE: return "GFX_CROSS_EYE";
        case GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST: return "SHADOW_SIMPLE_OPTIMAL_ADJUST";
        case GFX_SHADOW_AGGRESSIVE_FOCUS_REGION: return "SHADOW_AGGRESSIVE_FOCUS_REGION";
        case GFX_SKY: return "SKY";
    }
    return "UNKNOWN_BOOL_OPTION";
}
std::string gfx_option_to_string (GfxIntOption o)
{
    switch (o) {
        case GFX_FULLSCREEN_WIDTH: return "GFX_FULLSCREEN_WIDTH";
        case GFX_FULLSCREEN_HEIGHT: return "GFX_FULLSCREEN_HEIGHT";
        case GFX_SHADOW_RES: return "GFX_SHADOW_RES";
        case GFX_RAM: return "GFX_RAM";
    }
    return "UNKNOWN_INT_OPTION";
}
std::string gfx_option_to_string (GfxFloatOption o)
{
    switch (o) {
        case GFX_FOV: return "FOV";
        case GFX_NEAR_CLIP: return "NEAR_CLIP";
        case GFX_FAR_CLIP: return "FAR_CLIP";
        case GFX_EYE_SEPARATION: return "EYE_SEPARATION";
        case GFX_MONITOR_HEIGHT: return "MONITOR_HEIGHT";
        case GFX_MONITOR_EYE_DISTANCE: return "MONITOR_EYE_DISTANCE";
        case GFX_MIN_PERCEIVED_DEPTH: return "MIN_PERCEIVED_DEPTH";
        case GFX_MAX_PERCEIVED_DEPTH: return "MAX_PERCEIVED_DEPTH";
        case GFX_SHADOW_START: return "SHADOW_START";
        case GFX_SHADOW_END0: return "SHADOW_END0";
        case GFX_SHADOW_END1: return "SHADOW_END1";
        case GFX_SHADOW_END2: return "SHADOW_END2";
        case GFX_SHADOW_OPTIMAL_ADJUST0: return "SHADOW_OPTIMAL_ADJUST0";
        case GFX_SHADOW_OPTIMAL_ADJUST1: return "SHADOW_OPTIMAL_ADJUST1";
        case GFX_SHADOW_OPTIMAL_ADJUST2: return "SHADOW_OPTIMAL_ADJUST2";
        case GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD: return "SHADOW_LIGHT_DIRECTION_THRESHOLD";
        case GFX_SHADOW_PADDING: return "SHADOW_PADDING";
        case GFX_ANAGLYPH_LEFT_RED_MASK: return "ANAGLYPH_LEFT_RED_MASK";
        case GFX_ANAGLYPH_LEFT_GREEN_MASK: return "ANAGLYPH_LEFT_GREEN_MASK";
        case GFX_ANAGLYPH_LEFT_BLUE_MASK: return "ANAGLYPH_LEFT_BLUE_MASK";
        case GFX_ANAGLYPH_RIGHT_RED_MASK: return "ANAGLYPH_RIGHT_RED_MASK";
        case GFX_ANAGLYPH_RIGHT_GREEN_MASK: return "ANAGLYPH_RIGHT_GREEN_MASK";
        case GFX_ANAGLYPH_RIGHT_BLUE_MASK: return "ANAGLYPH_RIGHT_BLUE_MASK";
        case GFX_ANAGLYPH_DESATURATION: return "ANAGLYPH_DESATURATION";
    }
    return "UNKNOWN_FLOAT_OPTION";
}

// set's t to either 0,1,2 and fills in the approriate argument
void gfx_option_from_string (const std::string &s,
                             int &t,
                             GfxBoolOption &o0,
                             GfxIntOption &o1,
                             GfxFloatOption &o2)
{
    if (s=="AUTOUPDATE") { t = 0; o0 = GFX_AUTOUPDATE; }
    else if (s=="SHADOW_RECEIVE") { t = 0; o0 = GFX_SHADOW_RECEIVE; }
    else if (s=="SHADOW_CAST") { t = 0; o0 = GFX_SHADOW_CAST; }
    else if (s=="VSYNC") { t = 0; o0 = GFX_VSYNC; }
    else if (s=="FULLSCREEN") { t = 0; o0 = GFX_FULLSCREEN; }
    else if (s=="FOG") { t = 0; o0 = GFX_FOG; }
    else if (s=="DEFERRED") { t = 0; o0 = GFX_DEFERRED; }
    else if (s=="WIREFRAME") { t = 0; o0 = GFX_WIREFRAME; }
    else if (s=="ANAGLYPH") { t = 0; o0 = GFX_ANAGLYPH; }
    else if (s=="CROSS_EYE") { t = 0; o0 = GFX_CROSS_EYE; }
    else if (s=="SHADOW_SIMPLE_OPTIMAL_ADJUST") { t = 0; o0 = GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST; }
    else if (s=="SHADOW_AGGRESSIVE_FOCUS_REGION") { t = 0; o0 = GFX_SHADOW_AGGRESSIVE_FOCUS_REGION; }
    else if (s=="SKY") { t = 0; o0 = GFX_SKY; }
    else if (s=="FULLSCREEN_WIDTH") { t = 1; o1 = GFX_FULLSCREEN_WIDTH; }
    else if (s=="FULLSCREEN_HEIGHT") { t = 1; o1 = GFX_FULLSCREEN_HEIGHT; }
    else if (s=="SHADOW_RES") { t = 1; o1 = GFX_SHADOW_RES; }
    else if (s=="RAM") { t = 1; o1 = GFX_RAM; }
    else if (s=="FOV") { t = 2; o2 = GFX_FOV; }
    else if (s=="NEAR_CLIP") { t = 2; o2 = GFX_NEAR_CLIP; }
    else if (s=="FAR_CLIP") { t = 2; o2 = GFX_FAR_CLIP; }
    else if (s=="EYE_SEPARATION") { t = 2; o2 = GFX_EYE_SEPARATION; }
    else if (s=="MONITOR_HEIGHT") { t = 2; o2 = GFX_MONITOR_HEIGHT; }
    else if (s=="MONITOR_EYE_DISTANCE") { t = 2; o2 = GFX_MONITOR_EYE_DISTANCE; }
    else if (s=="MIN_PERCEIVED_DEPTH") { t = 2; o2 = GFX_MIN_PERCEIVED_DEPTH; }
    else if (s=="MAX_PERCEIVED_DEPTH") { t = 2; o2 = GFX_MAX_PERCEIVED_DEPTH; }
    else if (s=="SHADOW_START") { t = 2; o2 = GFX_SHADOW_START; }
    else if (s=="SHADOW_END0") { t = 2; o2 = GFX_SHADOW_END0; }
    else if (s=="SHADOW_END1") { t = 2; o2 = GFX_SHADOW_END1; }
    else if (s=="SHADOW_END2") { t = 2; o2 = GFX_SHADOW_END2; }
    else if (s=="SHADOW_OPTIMAL_ADJUST0") { t = 2; o2 = GFX_SHADOW_OPTIMAL_ADJUST0; }
    else if (s=="SHADOW_OPTIMAL_ADJUST1") { t = 2; o2 = GFX_SHADOW_OPTIMAL_ADJUST1; }
    else if (s=="SHADOW_OPTIMAL_ADJUST2") { t = 2; o2 = GFX_SHADOW_OPTIMAL_ADJUST2; }
    else if (s=="SHADOW_LIGHT_DIRECTION_THRESHOLD") { t = 2; o2 = GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD; }
    else if (s=="SHADOW_PADDING") { t = 2; o2 = GFX_SHADOW_PADDING; }
    else if (s=="ANAGLYPH_LEFT_RED_MASK") { t = 2; o2 = GFX_ANAGLYPH_LEFT_RED_MASK; }
    else if (s=="ANAGLYPH_LEFT_GREEN_MASK") { t = 2; o2 = GFX_ANAGLYPH_LEFT_GREEN_MASK; }
    else if (s=="ANAGLYPH_LEFT_BLUE_MASK") { t = 2; o2 = GFX_ANAGLYPH_LEFT_BLUE_MASK; }
    else if (s=="ANAGLYPH_RIGHT_RED_MASK") { t = 2; o2 = GFX_ANAGLYPH_RIGHT_RED_MASK; }
    else if (s=="ANAGLYPH_RIGHT_GREEN_MASK") { t = 2; o2 = GFX_ANAGLYPH_RIGHT_GREEN_MASK; }
    else if (s=="ANAGLYPH_RIGHT_BLUE_MASK") { t = 2; o2 = GFX_ANAGLYPH_RIGHT_BLUE_MASK; }
    else if (s=="ANAGLYPH_DESATURATION") { t = 2; o2 = GFX_ANAGLYPH_DESATURATION; }
    else t = -1;
}

static void options_update (bool flush)
{
    bool reset_fullscreen = flush;
    bool reset_shadowmaps = flush;
    bool reset_shadows = flush;
    bool reset_pcss = flush;
    bool reset_framebuffer = flush;
    bool reset_compositors = flush;
    bool reset_vr_cams = flush;
    bool reset_anaglyph_params = flush;

    for (unsigned i=0 ; i<sizeof(gfx_bool_options)/sizeof(*gfx_bool_options) ; ++i) {
        GfxBoolOption o = gfx_bool_options[i];
        bool v_old = options_bool[o];
        bool v_new = new_options_bool[o];
        if (v_old == v_new) continue;
        switch (o) {
            case GFX_AUTOUPDATE: break;
            case GFX_CROSS_EYE:
            case GFX_ANAGLYPH:
            reset_vr_cams = true;
            case GFX_DEFERRED:
            reset_framebuffer = true;
            reset_compositors = true;
            break;
            case GFX_SHADOW_RECEIVE:
            break;
            case GFX_SHADOW_CAST:
            reset_shadows = true;
            break;
            case GFX_VSYNC:
            ogre_win->setVSyncEnabled(v_new);
            break;
            case GFX_FULLSCREEN:
            reset_fullscreen = true;
            break;
            case GFX_FOG:
            break;
            case GFX_WIREFRAME:
            break;
            case GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST:
            reset_pcss = true;
            break;
            case GFX_SHADOW_AGGRESSIVE_FOCUS_REGION:
            reset_pcss = true;
            break;
            case GFX_SKY:
            ogre_sky_ent->setVisible(v_new);
            break;
        }
    }
    for (unsigned i=0 ; i<sizeof(gfx_int_options)/sizeof(*gfx_int_options) ; ++i) {
        GfxIntOption o = gfx_int_options[i];
        int v_old = options_int[o];
        int v_new = new_options_int[o];
        if (v_old == v_new) continue;
        switch (o) {
            case GFX_FULLSCREEN_WIDTH:
            reset_fullscreen = true;
            break;
            case GFX_FULLSCREEN_HEIGHT:
            reset_fullscreen = true;
            break;
            case GFX_RAM:
            break;
            case GFX_SHADOW_RES:
            reset_shadowmaps = true;
            break;
        }
    }
    for (unsigned i=0 ; i<sizeof(gfx_float_options)/sizeof(*gfx_float_options) ; ++i) {
        GfxFloatOption o = gfx_float_options[i];
        float v_old = options_float[o];
        float v_new = new_options_float[o];
        if (v_old == v_new) continue;
        switch (o) {
            case GFX_FOV:
            left_eye->setFOVy(Ogre::Degree(v_new));
            right_eye->setFOVy(Ogre::Degree(v_new));
            reset_vr_cams = true;
            break;
            case GFX_NEAR_CLIP:
            left_eye->setNearClipDistance(v_new);
            right_eye->setNearClipDistance(v_new);
            reset_vr_cams = true;
            break;
            case GFX_FAR_CLIP:
            left_eye->setFarClipDistance(v_new);
            right_eye->setFarClipDistance(v_new);
            reset_vr_cams = true;
            break;
            case GFX_EYE_SEPARATION:
            case GFX_MONITOR_HEIGHT:
            case GFX_MONITOR_EYE_DISTANCE:
            case GFX_MIN_PERCEIVED_DEPTH:
            case GFX_MAX_PERCEIVED_DEPTH:
            reset_vr_cams = true;
            break;
            case GFX_SHADOW_START:
            case GFX_SHADOW_END0:
            case GFX_SHADOW_END1:
            case GFX_SHADOW_END2:
            case GFX_SHADOW_OPTIMAL_ADJUST0:
            case GFX_SHADOW_OPTIMAL_ADJUST1:
            case GFX_SHADOW_OPTIMAL_ADJUST2:
            case GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD:
            case GFX_SHADOW_PADDING:
            reset_pcss = true;
            break;
            case GFX_ANAGLYPH_LEFT_RED_MASK:
            case GFX_ANAGLYPH_LEFT_GREEN_MASK:
            case GFX_ANAGLYPH_LEFT_BLUE_MASK:
            case GFX_ANAGLYPH_RIGHT_RED_MASK:
            case GFX_ANAGLYPH_RIGHT_GREEN_MASK:
            case GFX_ANAGLYPH_RIGHT_BLUE_MASK:
            case GFX_ANAGLYPH_DESATURATION:
            reset_anaglyph_params = true;
            reset_compositors = true;
            break;
        }
    }

    options_bool = new_options_bool;
    options_int = new_options_int;
    options_float = new_options_float;

    if (reset_shadowmaps) {
        ogre_sm->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, 3);
        ogre_sm->setShadowTextureSettings(options_int[GFX_SHADOW_RES], 3, Ogre::PF_FLOAT32_R);
    }

    if (reset_pcss) {

        Ogre::PSSMShadowCameraSetup *p = new Ogre::PSSMShadowCameraSetup();

        p->setOptimalAdjustFactor(0, gfx_option(GFX_SHADOW_OPTIMAL_ADJUST0));
        p->setOptimalAdjustFactor(1, gfx_option(GFX_SHADOW_OPTIMAL_ADJUST1));
        p->setOptimalAdjustFactor(2, gfx_option(GFX_SHADOW_OPTIMAL_ADJUST2));

        p->setUseAggressiveFocusRegion(gfx_option(GFX_SHADOW_AGGRESSIVE_FOCUS_REGION));

        p->setCameraLightDirectionThreshold(
            Ogre::Degree(gfx_option(GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD)));
        p->setUseSimpleOptimalAdjust(gfx_option(GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST));

        Ogre::PSSMShadowCameraSetup::SplitPointList boundaries;
        boundaries.push_back(gfx_option(GFX_SHADOW_START));
        boundaries.push_back(gfx_option(GFX_SHADOW_END0));
        boundaries.push_back(gfx_option(GFX_SHADOW_END1));
        boundaries.push_back(gfx_option(GFX_SHADOW_END2));
        p->setSplitPoints(boundaries);
        p->setSplitPadding(gfx_option(GFX_SHADOW_PADDING));

        ogre_sm->setShadowCameraSetup(Ogre::ShadowCameraSetupPtr(p));

    }

    if (reset_shadows) {
        if (gfx_option(GFX_SHADOW_CAST)) {
            ogre_sm->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED);
        } else {
            ogre_sm->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
        }
    }

    if (reset_fullscreen) {
        unsigned width = gfx_option(GFX_FULLSCREEN_WIDTH);
        unsigned height = gfx_option(GFX_FULLSCREEN_HEIGHT);

        if (gfx_option(GFX_FULLSCREEN)) {
            if (ogre_win->isFullScreen()) {
                if (ogre_win->getWidth()!=width || ogre_win->getHeight()!=height) {
                    ogre_win->setFullscreen(true, width, height);
                }
            } else {
                ogre_win->setFullscreen(true, width, height);
            }
        } else {
            if (ogre_win->isFullScreen()) {
                ogre_win->setFullscreen(false, 640, 480);
            }
        }
    }

    if (reset_vr_cams) {
        if (stereoscopic()) {
            float FOV = gfx_option(GFX_FOV);
            float monitor_height = gfx_option(GFX_MONITOR_HEIGHT);
            float distance = gfx_option(GFX_MONITOR_EYE_DISTANCE);
            float eye_separation = gfx_option(GFX_EYE_SEPARATION);
            float min = gfx_option(GFX_MIN_PERCEIVED_DEPTH);
            float max = gfx_option(GFX_MAX_PERCEIVED_DEPTH);

            float s = 2*tan((FOV/2)/180*M_PI)/monitor_height * (eye_separation * (1-distance/max));
            left_eye->setFrustumOffset(s/2);
            right_eye->setFrustumOffset(-s/2);
            left_eye->setFocalLength(1);
            right_eye->setFocalLength(1);

            float c = 2*tan((FOV/2)/180*M_PI)/monitor_height * (eye_separation * (1-distance/min));
            c = gfx_option(GFX_NEAR_CLIP) * (s - c);
            cam_separation = c;
        } else {
            left_eye->setFrustumOffset(0);
            right_eye->setFrustumOffset(0);
            left_eye->setFocalLength(1);
            right_eye->setFocalLength(1);
            cam_separation = 0;
        }

    }

    if (reset_anaglyph_params) {
        Ogre::MaterialPtr rtt_mat = Ogre::MaterialManager::getSingleton()
            .getByName(ANAGLYPH_COMPOSITOR_MATERIAL);
        rtt_mat->load();
        Ogre::Pass *p = rtt_mat->getTechnique(0)->getPass(0);

        Ogre::Vector3 left_mask(gfx_option(GFX_ANAGLYPH_LEFT_RED_MASK),
                                gfx_option(GFX_ANAGLYPH_LEFT_GREEN_MASK),
                                gfx_option(GFX_ANAGLYPH_LEFT_BLUE_MASK));
        Ogre::Vector3 right_mask(gfx_option(GFX_ANAGLYPH_RIGHT_RED_MASK),
                                 gfx_option(GFX_ANAGLYPH_RIGHT_GREEN_MASK),
                                 gfx_option(GFX_ANAGLYPH_RIGHT_BLUE_MASK));
        p->getFragmentProgramParameters()
            ->setNamedConstant("desaturation",gfx_option(GFX_ANAGLYPH_DESATURATION));
        p->getFragmentProgramParameters()->setNamedConstant("left_mask",left_mask);
        p->getFragmentProgramParameters()->setNamedConstant("right_mask",right_mask);

    }

    if (reset_compositors) clean_compositors();

    if (reset_framebuffer) {
        APP_ASSERT(reset_compositors);
        do_reset_framebuffer();
    }

    if (reset_compositors) do_reset_compositors();
}

static void init_options (void)
{
    valid_option(GFX_AUTOUPDATE, truefalse);

    valid_option(GFX_SHADOW_RECEIVE, truefalse);
    valid_option(GFX_SHADOW_CAST, truefalse);
    valid_option(GFX_VSYNC, truefalse);
    valid_option(GFX_FULLSCREEN, truefalse);
    valid_option(GFX_FOG, truefalse);
    valid_option(GFX_DEFERRED, truefalse);
    valid_option(GFX_WIREFRAME, truefalse);
    valid_option(GFX_ANAGLYPH, truefalse);
    valid_option(GFX_CROSS_EYE, truefalse);
    valid_option(GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST, truefalse);
    valid_option(GFX_SHADOW_AGGRESSIVE_FOCUS_REGION, truefalse);
    valid_option(GFX_SKY, truefalse);

    valid_option(GFX_FULLSCREEN_WIDTH, new ValidOptionRange<int>(1,10000));
    valid_option(GFX_FULLSCREEN_HEIGHT, new ValidOptionRange<int>(1,10000));
    int res_list[] = {512,1024,2048,4096};
    valid_option(GFX_SHADOW_RES, new ValidOptionList<int,int[4]>(res_list));
    valid_option(GFX_RAM, new ValidOptionRange<int>(1,16384));

    valid_option(GFX_FOV, new ValidOptionRange<float>(0.0000001,179));
    valid_option(GFX_NEAR_CLIP, new ValidOptionRange<float>(0.0000001,10000));
    valid_option(GFX_FAR_CLIP, new ValidOptionRange<float>(0.0000001,10000));
    valid_option(GFX_EYE_SEPARATION, new ValidOptionRange<float>(0,0.5));
    valid_option(GFX_MONITOR_HEIGHT, new ValidOptionRange<float>(0.01,1000));
    valid_option(GFX_MONITOR_EYE_DISTANCE, new ValidOptionRange<float>(0.01,1000));
    valid_option(GFX_MIN_PERCEIVED_DEPTH, new ValidOptionRange<float>(0.01,1000));
    valid_option(GFX_MAX_PERCEIVED_DEPTH, new ValidOptionRange<float>(0.01,1000));

    valid_option(GFX_SHADOW_START, new ValidOptionRange<float>(0,10000));
    valid_option(GFX_SHADOW_END0, new ValidOptionRange<float>(0,10000));
    valid_option(GFX_SHADOW_END1, new ValidOptionRange<float>(0,10000));
    valid_option(GFX_SHADOW_END2, new ValidOptionRange<float>(0,10000));
    valid_option(GFX_SHADOW_OPTIMAL_ADJUST0, new ValidOptionRange<float>(0,10000));
    valid_option(GFX_SHADOW_OPTIMAL_ADJUST1, new ValidOptionRange<float>(0,10000));
    valid_option(GFX_SHADOW_OPTIMAL_ADJUST2, new ValidOptionRange<float>(0,10000));
    valid_option(GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD, new ValidOptionRange<float>(0,10000));
    valid_option(GFX_SHADOW_PADDING, new ValidOptionRange<float>(0,100));
    valid_option(GFX_ANAGLYPH_DESATURATION, new ValidOptionRange<float>(0,1));
    valid_option(GFX_ANAGLYPH_LEFT_RED_MASK, new ValidOptionRange<float>(0,1));
    valid_option(GFX_ANAGLYPH_LEFT_GREEN_MASK, new ValidOptionRange<float>(0,1));
    valid_option(GFX_ANAGLYPH_LEFT_BLUE_MASK, new ValidOptionRange<float>(0,1));
    valid_option(GFX_ANAGLYPH_RIGHT_RED_MASK, new ValidOptionRange<float>(0,1));
    valid_option(GFX_ANAGLYPH_RIGHT_GREEN_MASK, new ValidOptionRange<float>(0,1));
    valid_option(GFX_ANAGLYPH_RIGHT_BLUE_MASK, new ValidOptionRange<float>(0,1));

    valid_option(GFX_AUTOUPDATE, truefalse);


    gfx_option(GFX_AUTOUPDATE, false);

    gfx_option(GFX_SHADOW_RECEIVE, true);
    gfx_option(GFX_SHADOW_CAST, true);
    gfx_option(GFX_VSYNC, true);
    gfx_option(GFX_FULLSCREEN, false);
    gfx_option(GFX_FOG, true);
    gfx_option(GFX_DEFERRED, false);
    gfx_option(GFX_WIREFRAME, false);
    gfx_option(GFX_ANAGLYPH, false);
    gfx_option(GFX_CROSS_EYE, false);

    gfx_option(GFX_FULLSCREEN_WIDTH, 800);
    gfx_option(GFX_FULLSCREEN_HEIGHT, 600);
    gfx_option(GFX_SHADOW_RES, 1024);
    gfx_option(GFX_RAM, 128);

    gfx_option(GFX_FOV, 55);
    gfx_option(GFX_NEAR_CLIP, 0.355);
    gfx_option(GFX_FAR_CLIP, 1000);
    gfx_option(GFX_EYE_SEPARATION, 0.06);
    gfx_option(GFX_MONITOR_HEIGHT, 0.27);
    gfx_option(GFX_MONITOR_EYE_DISTANCE, 0.6);
    gfx_option(GFX_MIN_PERCEIVED_DEPTH, 0.3);
    gfx_option(GFX_MAX_PERCEIVED_DEPTH, 2);
    gfx_option(GFX_SHADOW_START, 0.2);
    gfx_option(GFX_SHADOW_END0, 20);
    gfx_option(GFX_SHADOW_END1, 50);
    gfx_option(GFX_SHADOW_END2, 200);
    gfx_option(GFX_SHADOW_OPTIMAL_ADJUST0, 3);
    gfx_option(GFX_SHADOW_OPTIMAL_ADJUST1, 1);
    gfx_option(GFX_SHADOW_OPTIMAL_ADJUST2, 1);
    gfx_option(GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD, 35);
    gfx_option(GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST, true);
    gfx_option(GFX_SHADOW_PADDING, 0.8);
    gfx_option(GFX_SHADOW_AGGRESSIVE_FOCUS_REGION, true);
    gfx_option(GFX_SKY, true);
    gfx_option(GFX_ANAGLYPH_DESATURATION, 0.5);
    gfx_option(GFX_ANAGLYPH_LEFT_RED_MASK, 1);
    gfx_option(GFX_ANAGLYPH_LEFT_GREEN_MASK, 0);
    gfx_option(GFX_ANAGLYPH_LEFT_BLUE_MASK, 0);
    gfx_option(GFX_ANAGLYPH_RIGHT_RED_MASK, 0);
    gfx_option(GFX_ANAGLYPH_RIGHT_GREEN_MASK, 1);
    gfx_option(GFX_ANAGLYPH_RIGHT_BLUE_MASK, 1);

    options_update(true);

    gfx_option(GFX_AUTOUPDATE, true);
}

void gfx_option (GfxBoolOption o, bool v)
{
    valid_option_bool[o]->maybeThrow(v);
    try {
        new_options_bool[o] = v;
        if (new_options_bool[GFX_AUTOUPDATE]) options_update(false);
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Setting graphics option");
    }
}
bool gfx_option (GfxBoolOption o)
{
    return options_bool[o];
}

void gfx_option (GfxIntOption o, int v)
{
    valid_option_int[o]->maybeThrow(v);
    try {
        new_options_int[o] = v;
        if (new_options_bool[GFX_AUTOUPDATE]) options_update(false);
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Setting graphics option");
    }
}
int gfx_option (GfxIntOption o)
{
    return options_int[o];
}

void gfx_option (GfxFloatOption o, float v)
{
    valid_option_float[o]->maybeThrow(v);
    try {
        new_options_float[o] = v;
        if (new_options_bool[GFX_AUTOUPDATE]) options_update(false);
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Setting graphics option");
    }
}
float gfx_option (GfxFloatOption o)
{
    return options_float[o];
}

// }}}


static fast_erase_vector<GfxBody*> all_bodies;
static fast_erase_vector<GfxLight*> all_lights;

// {{{ GFX_NODE

#define THROW_DEAD(name) do { GRIT_EXCEPT(name+" has been destroyed."); } while (0)

const std::string GfxNode::className = "GfxNode";

GfxNode::GfxNode (const GfxBodyPtr &par_)
{
    dead = false;
    pos = Vector3(0,0,0);
    scl = Vector3(1,1,1);
    quat = Quaternion(1,0,0,0);
    node = ogre_sm->createSceneNode();
    par = GfxBodyPtr(NULL); setParent(par_);
}

GfxNode::~GfxNode (void)
{
    if (!dead) CERR << className+" has not been destroyed properly." << std::endl;
}

void GfxNode::destroy (void)
{
    if (!par.isNull()) par->notifyLostChild(this);
    ogre_sm->destroySceneNode(node->getName());
    node = NULL;
    par = GfxBodyPtr(NULL);
    dead = true;
}

void GfxNode::notifyParentDead (void)
{
    if (dead) THROW_DEAD(className);
    APP_ASSERT(!par.isNull());
    par->node->removeChild(node);
    par = GfxBodyPtr(NULL);
    ogre_root_node->addChild(node);
}

const GfxBodyPtr &GfxNode::getParent (void) const
{
    if (dead) THROW_DEAD(className);
    return par;
}

void GfxNode::setParent (const GfxBodyPtr &par_)
{
    if (dead) THROW_DEAD(className);
    if (!par.isNull()) {
        par->node->removeChild(node);
        par->notifyLostChild(this);
    } else {
        ogre_root_node->removeChild(node);
    }
    par = par_;
    if (!par.isNull()) {
        par->notifyGainedChild(this);
        par->node->addChild(node);
    } else {
        ogre_root_node->addChild(node);
    }
}

void GfxNode::ensureNotChildOf (GfxBody *leaf) const
{
    if (leaf==this) GRIT_EXCEPT("Parenthood must be acyclic.");
    if (par.isNull()) return;
    par->ensureNotChildOf(leaf);
}

void GfxNode::setLocalPosition (const Vector3 &p)
{
    if (dead) THROW_DEAD(className);
    pos = p;
    node->setPosition(to_ogre(p));
}

void GfxNode::setLocalOrientation (const Quaternion &q)
{
    if (dead) THROW_DEAD(className);
    quat = q;
    node->setOrientation(to_ogre(q));
}

void GfxNode::setLocalScale (const Vector3 &s)
{
    if (dead) THROW_DEAD(className);
    scl = s;
    node->setScale(to_ogre(s));
}

Vector3 GfxNode::transformPosition (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return getWorldPosition() + getWorldScale()*(getWorldOrientation()*v);
}
Quaternion GfxNode::transformOrientation (const Quaternion &v)
{
    if (dead) THROW_DEAD(className);
    return getWorldOrientation()*v;
}
Vector3 GfxNode::transformScale (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return getWorldScale()*v;
}

Vector3 GfxNode::transformPositionParent (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return par.isNull() ? v : par->transformPosition(v);
}
Quaternion GfxNode::transformOrientationParent (const Quaternion &v)
{
    if (dead) THROW_DEAD(className);
    return par.isNull() ? v : par->transformOrientation(v);
}
Vector3 GfxNode::transformScaleParent (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return par.isNull() ? v : par->transformScale(v);
}

const Vector3 &GfxNode::getLocalPosition (void)
{
    if (dead) THROW_DEAD(className);
    return pos;
}
Vector3 GfxNode::getWorldPosition (void)
{
    if (dead) THROW_DEAD(className);
    return transformPositionParent(pos);
}

const Quaternion &GfxNode::getLocalOrientation (void)
{
    if (dead) THROW_DEAD(className);
    return quat;
}
Quaternion GfxNode::getWorldOrientation (void)
{
    if (dead) THROW_DEAD(className);
    return transformOrientationParent(quat);
}

const Vector3 &GfxNode::getLocalScale (void)
{
    if (dead) THROW_DEAD(className);
    return scl;
}
Vector3 GfxNode::getWorldScale (void)
{
    if (dead) THROW_DEAD(className);
    return transformScaleParent(scl);
}

// }}}


// {{{ GFX_BODY

const std::string GfxBody::className = "GfxBody";

static const std::string &apply_map (const GfxStringMap &sm, const std::string &s)
{
    GfxStringMap::const_iterator i = sm.find(s);
    return i==sm.end() ? s : i->second;
}

static void validate_mesh (const Ogre::MeshPtr &mesh, const GfxStringMap &gsm)
{
/*
    for (GfxStringMap::const_iterator i=gsm.begin(),i_=gsm.end() ; i!=i_ ; ++i) {
        CVERB << i->first << " " << i->second << std::endl;
    }
*/
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        std::string matname = apply_map(gsm, sm->getMaterialName());
        if (!gfx_material_has(matname)) {
            CERR << "Mesh \"/"<<mesh->getName()<<"\" references non-existing material "
                 << "\""<<matname<<"\""<<std::endl;
            matname = "/BaseWhite";
            sm->setMaterialName(matname);
        }
    }
}

GfxBody::GfxBody (const std::string &mesh_name, const GfxStringMap &sm, const GfxBodyPtr &par_)
  : GfxNode(par_), initialMaterialMap(sm)
{
    memset(colours, 0, sizeof(colours));
    std::string ogre_name = mesh_name.substr(1);

    mesh = Ogre::MeshManager::getSingleton().load(ogre_name,RESGRP);
    // validate mesh in case it was just loaded for the first time

    ent = NULL;
    entEmissive = NULL;
    fade = 1;
    enabled = true;

    reinitialise();

    all_bodies.push_back(this);
}

void GfxBody::updateMaterials (void)
{
    if (mesh.isNull()) return;
    validate_mesh(mesh, initialMaterialMap);
    materials = std::vector<GfxMaterial*>(mesh->getNumSubMeshes());
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        std::string matname = apply_map(initialMaterialMap, sm->getMaterialName());
        materials[i] = gfx_material_get(matname);
    }
    unsigned old_size = emissiveEnabled.size();
    emissiveEnabled.resize(mesh->getNumSubMeshes());
    for (unsigned i=old_size ; i<emissiveEnabled.size() ; ++i) {
        emissiveEnabled[i] = true;
    }
}

unsigned GfxBody::getSubMeshByOriginalMaterialName (const std::string &n)
{
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        if (sm->getMaterialName() == n) {
            return i;
        }
    }
    CERR << "Mesh did not contain material \""<<n<<"\"" <<std::endl;
    return 0;
}

void GfxBody::updateBones (void)
{
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) {
        manualBones.clear();
        return;
    }
    manualBones.resize(skel->getNumBones());
    for (unsigned i=0 ; i<skel->getNumBones() ; ++i) {
        skel->getBone(i)->setManuallyControlled(manualBones[i]);
    }
    skel->_notifyManualBonesDirty(); // HACK: ogre should do this itself
}

void GfxBody::reinitialise (void)
{
    updateMaterials();
    if (entEmissive) ogre_sm->destroyEntity(entEmissive);
    entEmissive = NULL;

    if (ent!=NULL) ogre_sm->destroyEntity(ent);
    ent = ogre_sm->createEntity(freshname(), mesh->getName());
    node->attachObject(ent);
    updateProperties();
    updateBones();
}

void GfxBody::updateVisibility (void)
{
    if (ent==NULL) return;
    // avoid the draw entirely if faded out
    ent->setVisible(enabled && fade > 0.001);
    if (entEmissive) entEmissive->setVisible(enabled && fade > 0.001);

    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {
        Ogre::SubEntity *se = ent->getSubEntity(i);
        // fading in/out (either stipple or alpha factor)
        se->setCustomParameter(0, Ogre::Vector4(fade,0,0,0));
    }

    if (entEmissive != NULL) {
        for (unsigned i=0 ; i<entEmissive->getNumSubEntities() ; ++i) {
            Ogre::SubEntity *se = entEmissive->getSubEntity(i);
            // fading in/out (either stipple or alpha factor)
            se->setCustomParameter(0, Ogre::Vector4(fade,0,0,0));
        }
    }

}

void GfxBody::updateEntEmissive (void)
{
    if (ent==NULL) return;
    if (entEmissive != NULL) {
        // destroy it if we've already got one
        ogre_sm->destroyEntity(entEmissive);
        entEmissive = NULL;
    }
    bool needs_emissive = false;
    for (unsigned i=0 ; i<materials.size() ; ++i) {
        GfxMaterial *gfx_material = materials[i];
        if (!gfx_material->emissiveMat.isNull() && emissiveEnabled[i]) {
            needs_emissive = true;
        }
    }
    if (!needs_emissive) return;
    entEmissive = ogre_sm->createEntity(freshname(), mesh->getName());
    node->attachObject(entEmissive);
    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {
        GfxMaterial *gfx_material = materials[i];
        Ogre::SubEntity *se = entEmissive->getSubEntity(i);
        if (!gfx_material->emissiveMat.isNull() && emissiveEnabled[i]) {
            se->setMaterial(gfx_material->emissiveMat);
            se->setRenderQueueGroup(RQ_FORWARD_EMISSIVE);
        } else {
            se->setVisible(false);
        }
    }
    updateVisibility();
}

void GfxBody::updateProperties (void)
{
    if (ent==NULL) return;

    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {

        Ogre::SubEntity *se = ent->getSubEntity(i);

        GfxMaterial *gfx_material = materials[i];

        switch (gfx_material->getBlend()) {
            case GFX_MATERIAL_OPAQUE:
            se->setRenderQueueGroup(RQ_GBUFFER_OPAQUE);
            break;
            case GFX_MATERIAL_ALPHA:
            se->setRenderQueueGroup(RQ_FORWARD_ALPHA);
            break;
            case GFX_MATERIAL_ALPHA_DEPTH:
            se->setRenderQueueGroup(RQ_FORWARD_ALPHA_DEPTH);
            break;
        }

        // materials
        /* TODO: include other criteria to pick a specific Ogre::Material:
         * bones: 1 2 3 4
         * vertex colours: false/true
         * vertex alpha: false/true
         * Fading: false/true
         * World: false/true
         */
        if (fade < 1 && !gfx_material->getStipple()) {
            se->setMaterial(gfx_material->fadingMat);
        } else {
            se->setMaterial(gfx_material->regularMat);
        }

        // car paint
        for (int k=0 ; k<4 ; ++k) {
            const GfxPaintColour &c = colours[k];
            se->setCustomParameter(2*k+1,Ogre::Vector4(c.diff.x, c.diff.y, c.diff.z, c.met));
            se->setCustomParameter(2*k+2,  Ogre::Vector4(c.spec.x, c.spec.y, c.spec.z, 0));
        }
    }

    updateEntEmissive();

    updateVisibility();
}

GfxBody::GfxBody (const GfxBodyPtr &par_)
  : GfxNode(par_)
{
    memset(colours, 0, sizeof(colours));

    mesh.setNull();

    ent = NULL;
    entEmissive = NULL;
    fade = 1;
    enabled = true;

    all_bodies.push_back(this);
}

GfxBody::~GfxBody (void)
{
    if (!dead) destroy();
}

void GfxBody::destroy (void)
{
    if (dead) THROW_DEAD(className);
    for (unsigned i=0 ; i<children.size() ; ++i) {
        children[i]->notifyParentDead();
    }
    children.clear();
    if (ent) ogre_sm->destroyEntity(ent);
    ent = NULL;
    if (entEmissive) ogre_sm->destroyEntity(entEmissive);
    entEmissive = NULL;
    all_bodies.erase(this);
    GfxNode::destroy();
}

GfxMaterial *GfxBody::getMaterial (unsigned i)
{
    if (i >= materials.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return materials[i];
}

const std::string &GfxBody::getOriginalMaterialName (unsigned i)
{
    if (i >= materials.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return mesh->getSubMesh(i)->getMaterialName();
}

void GfxBody::setMaterial (unsigned i, GfxMaterial *m)
{
    if (i >= materials.size()) GRIT_EXCEPT("Submesh id out of range. ");
    if (materials[i] == m) return;
    materials[i] = m;
    updateProperties();
}

bool GfxBody::getEmissiveEnabled (unsigned i)
{
    if (i >= emissiveEnabled.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return emissiveEnabled[i];
}

void GfxBody::setEmissiveEnabled (unsigned i, bool v)
{
    if (i >= emissiveEnabled.size()) GRIT_EXCEPT("Submesh id out of range. ");
    emissiveEnabled[i] = v;
    updateEntEmissive();
}

void GfxBody::notifyLostChild (GfxNode *child)
{
    unsigned ci = 0;
    bool found_child = false;
    for (unsigned i=0 ; i<children.size() ; ++i) {
        if (children[i]==child) {
            ci = i;
            found_child = true;
            break;
        }
    }
    APP_ASSERT(found_child);
    children[ci] = children[children.size()-1];
    children.pop_back();
}

void GfxBody::notifyGainedChild (GfxNode *child)
{
    children.push_back(child);
}

void GfxBody::setParent (const GfxBodyPtr &par_)
{
    if (dead) THROW_DEAD(className);
    if (!par_.isNull()) par_->ensureNotChildOf(this); // check that we are not a parent of par_
    GfxNode::setParent(par_);
}

unsigned GfxBody::getBatches (void) const
{
    if (!hasGraphics()) return 0;
    return materials.size();
}

unsigned GfxBody::getBatchesWithChildren (void) const
{
    unsigned total = getBatches();
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxBody *child = dynamic_cast<GfxBody*>(children[i]);
        if (child) {
            total += child->getBatchesWithChildren();
        }
    }
    return total;
}

unsigned GfxBody::getTriangles (void) const
{
    if (!hasGraphics()) return 0;
    unsigned total = 0;
    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {
        total += ent->getSubEntity(i)->getSubMesh()->indexData->indexCount/3;
    }
    return total;
}

unsigned GfxBody::getTrianglesWithChildren (void) const
{
    unsigned total = getTriangles();
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxBody *child = dynamic_cast<GfxBody*>(children[i]);
        if (child) {
            total += child->getTrianglesWithChildren();
        }
    }
    return total;
}

float GfxBody::getFade (void)
{
    if (dead) THROW_DEAD(className);
    // It's convenient to just set fade for all nodes in the hierarchy
    //if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    return fade;
}
void GfxBody::setFade (float f)
{
    if (dead) THROW_DEAD(className);
    //if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    fade = f;
    updateVisibility();
}

bool GfxBody::getCastShadows (void)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    return ent->getCastShadows();
}
void GfxBody::setCastShadows (bool v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    ent->setCastShadows(v);
}

GfxPaintColour GfxBody::getPaintColour (int i)
{
    if (dead) THROW_DEAD(className);
    return colours[i];
}
void GfxBody::setPaintColour (int i, const GfxPaintColour &c)
{
    if (dead) THROW_DEAD(className);
    colours[i] = c;
    updateProperties();
}

unsigned GfxBody::getNumBones (void)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    return manualBones.size();
}

unsigned GfxBody::getBoneId (const std::string name)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (!skel->hasBone(name)) GRIT_EXCEPT("GfxBody has no bone \""+name+"\"");
    return skel->getBone(name)->getHandle();
}

void GfxBody::checkBone (unsigned n)
{
    if (manualBones.size()==0) GRIT_EXCEPT("GfxBody has no skeleton");
    if (n >= manualBones.size()) {
        std::stringstream ss;
        ss << "Bone " << n << " out of range [0," << manualBones.size() << ")";
        GRIT_EXCEPT(ss.str());
    }
}

const std::string &GfxBody::getBoneName (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    return skel->getBone(n)->getName();
}

bool GfxBody::getBoneManuallyControlled (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    return manualBones[n];
}

void GfxBody::setBoneManuallyControlled (unsigned n, bool v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    manualBones[n] = v;
    updateBones();
}

void GfxBody::setAllBonesManuallyControlled (bool v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    for (unsigned i=0 ; i<manualBones.size() ; ++i) {
        manualBones[i] = v;
    }
    updateBones();
}

Vector3 GfxBody::getBoneInitialPosition (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getInitialPosition());
}

Vector3 GfxBody::getBoneWorldPosition (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->_getDerivedPosition());
}

Vector3 GfxBody::getBoneLocalPosition (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getPosition());
}

Quaternion GfxBody::getBoneInitialOrientation (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getInitialOrientation());
}

Quaternion GfxBody::getBoneWorldOrientation (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->_getDerivedOrientation());
}

Quaternion GfxBody::getBoneLocalOrientation (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getOrientation());
}


void GfxBody::setBoneLocalPosition (unsigned n, const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    Ogre::Bone *bone = skel->getBone(n);
    bone->setPosition(to_ogre(v));
}

void GfxBody::setBoneLocalOrientation (unsigned n, const Quaternion &v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    Ogre::Bone *bone = skel->getBone(n);
    bone->setOrientation(to_ogre(v));
}

bool GfxBody::isEnabled (void)
{
    if (dead) THROW_DEAD(className);
    return enabled;
}

void GfxBody::setEnabled (bool v)
{
    if (dead) THROW_DEAD(className);
    enabled = v;
    updateVisibility();
}

// }}}


// {{{ PARTICLES

static void reset_particle_material (const Ogre::MaterialPtr &mat, std::string texname,
                                     GfxParticleBlend blend, float alphaRej, bool emissive)
{
    mat->removeAllTechniques();
    Ogre::Pass *pass = mat->createTechnique()->createPass();
    {
        Ogre::TextureUnitState *tus = pass->createTextureUnitState();
        tus->setTextureFiltering(Ogre::FO_POINT, Ogre::FO_POINT, Ogre::FO_NONE);
        tus->setContentType(Ogre::TextureUnitState::CONTENT_COMPOSITOR);
        tus->setCompositorReference(DEFERRED_COMPOSITOR,"fat_fb",0);
    }
    {
        Ogre::TextureUnitState *tus = pass->createTextureUnitState();
        tus->setTextureName(texname);
        tus->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
    }
    std::string gpuprog_name;
    switch (blend) {
        case GFX_PARTICLE_OPAQUE:
        pass->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
        pass->setDepthWriteEnabled(true);
        gpuprog_name = "O";
        break;
        case GFX_PARTICLE_ALPHA:
        pass->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
        pass->setDepthWriteEnabled(false);
        gpuprog_name = "A";
        break;
        case GFX_PARTICLE_ADD:
        pass->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);
        pass->setDepthWriteEnabled(false);
        gpuprog_name = "L";
        break;
    }
    gpuprog_name += "_";
    gpuprog_name += emissive ? "E" : "e";
    pass->setAlphaRejectFunction(Ogre::CMPF_GREATER_EQUAL);
    pass->setAlphaRejectValue(alphaRej);
    pass->setFragmentProgram("particle_f:"+gpuprog_name);
    pass->setVertexProgram("particle_v:"+gpuprog_name);
}

// a particle system holds the buffer for particles of a particular material
class GfxParticleSystem {
    Ogre::BillboardSet *bbset;
    float texHeight;
    float texWidth;
    public:
    GfxParticleSystem (const std::string &pname, std::string texname,
                       GfxParticleBlend blend, float alphaRej, bool emissive)
    {
        bbset = ogre_sm->createBillboardSet(pname, 100);
        ogre_root_node->attachObject(bbset);
        bbset->setUseAccurateFacing(true);

        std::string mname = "P:"+pname;
        texname = texname.substr(1);
        Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().load(texname, RESGRP);
        texHeight = tex->getHeight();
        texWidth = tex->getWidth();
        Ogre::MaterialPtr mat =
            Ogre::MaterialManager::getSingleton().createOrRetrieve(mname, RESGRP).first;
        reset_particle_material(mat, texname, blend, alphaRej, emissive);
        bbset->setMaterialName(mname);

        bbset->setBillboardRotationType(Ogre::BBR_VERTEX);
        if (blend==GFX_PARTICLE_OPAQUE) {
            bbset->setRenderQueueGroup(RQ_FORWARD_OPAQUE);
            bbset->setSortingEnabled(false);
        } else {
            bbset->setRenderQueueGroup(RQ_FORWARD_ALPHA);
            bbset->setSortingEnabled(true);
        }
        //bbset->setBillboardsInWorldSpace(true);
        bbset->setBounds(Ogre::AxisAlignedBox::BOX_INFINITE, 100000);
    }
    ~GfxParticleSystem (void)
    {
        ogre_sm->destroyBillboardSet(bbset);
    }

    GfxParticle emit (void)
    {
        return GfxParticle (this);
    }

    void *acquire (void)
    {
        return bbset->createBillboard(Ogre::Vector3(0,0,0));
    }

    void release (void *bb_)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bbset->removeBillboard(bb);
    }

    void setPosition (void *bb_, const Vector3 &v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setPosition(to_ogre(v));
    }

    void setAmbient (void *bb_, const Vector3 &v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        Ogre::ColourValue cv = bb->getColour();
        cv.r = v.x;
        cv.g = v.y;
        cv.b = v.z;
        bb->setColour(cv);
    }

    void setAlpha (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        Ogre::ColourValue cv = bb->getColour();
        cv.a = v;
        bb->setColour(cv);
    }

    void setAngle (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setRotation(Ogre::Degree(v));
    }

    void setUV (void *bb_, float u1, float v1, float u2, float v2)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setTexcoordRect(u1/texWidth, v1/texHeight, u2/texWidth, v2/texHeight);
    }

    void setDefaultUV (void *bb_)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setTexcoordRect(0.5/texWidth, 0.5/texHeight,
                            (texWidth-0.5)/texWidth, (texHeight-0.5)/texHeight);
    }

    void setWidth (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setDimensions(bb->getOwnWidth(), v);
    }

    void setHeight (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setDimensions(v, bb->getOwnHeight());
    }

    void setDepth (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setBias(-v/2);
    }

};

static std::map<std::string, GfxParticleSystem*> psystems;

GfxParticle::GfxParticle (GfxParticleSystem *sys_)
  : sys(sys_)
{
    internal = sys->acquire();
}

GfxParticle &GfxParticle::operator= (const GfxParticle &other)
{
    sys = other.sys;
    internal = other.internal;
    return *this;
}

void GfxParticle::setPosition (const Vector3 &v)
{ sys->setPosition(internal, v); }

void GfxParticle::setAmbient (const Vector3 &v)
{ sys->setAmbient(internal, v); }

void GfxParticle::setAlpha (float v)
{ sys->setAlpha(internal, v); }

void GfxParticle::setAngle (float v)
{ sys->setAngle(internal, v); }

void GfxParticle::setUV (float u1, float v1, float u2, float v2)
{ sys->setUV(internal, u1,v1,u2,v2); }

void GfxParticle::setDefaultUV (void)
{ sys->setDefaultUV(internal); }

void GfxParticle::setWidth (float v)
{ sys->setWidth(internal, v); }

void GfxParticle::setHeight (float v)
{ sys->setHeight(internal, v); }

void GfxParticle::setDepth (float v)
{ sys->setDepth(internal, v); }

void GfxParticle::release (void)
{ sys->release(internal); }

void gfx_particle_define (const std::string &pname, const std::string &tex_name,
                          GfxParticleBlend blend, float alpha_rej, bool emissive)
{
    GfxParticleSystem *&psys = psystems[pname];
    if (psys != NULL) delete psys;
    psys = new GfxParticleSystem(pname, tex_name, blend, alpha_rej, emissive);
}

GfxParticle gfx_particle_emit (const std::string &pname)
{
    GfxParticleSystem *&psys = psystems[pname];
    if (psys == NULL) GRIT_EXCEPT("No such particle: \""+pname+"\"");
    return psys->emit();
}

// }}}


// {{{ GFX_LIGHT

GfxParticleSystem *coronas = NULL;

static void ensure_coronas_init (void)
{
    if (coronas != NULL) return;
    // this only happens once
    coronas = new GfxParticleSystem("/system/Coronas", "/system/Corona.bmp",
                                    GFX_PARTICLE_ADD, 0, true);
}

const std::string GfxLight::className = "GfxLight";

GfxLight::GfxLight (const GfxBodyPtr &par_)
  : GfxNode(par_),
    enabled(true),
    fade(1),
    coronaLocalPos(0,0,0),
    coronaSize(1),
    coronaColour(1,1,1),
    diffuse(1,1,1),
    specular(1,1,1),
    aim(1,0,0,0)
{
    light = ogre_sm->createLight();
    node->attachObject(light);
    light->setDirection(Ogre::Vector3(0,1,0));
    light->setAttenuation(10, 0, 0, 0);
    light->setSpotlightInnerAngle(Ogre::Degree(180));
    light->setSpotlightOuterAngle(Ogre::Degree(180));
    updateVisibility();
    all_lights.push_back(this);
    ensure_coronas_init();
    corona = coronas->emit();
    corona.setDefaultUV();
    corona.setAlpha(1);
    corona.setAngle(0);
    updateCorona(Vector3(0,0,0));
}

GfxLight::~GfxLight (void)
{
    if (!dead) destroy();
}

void GfxLight::destroy (void)
{
    if (dead) THROW_DEAD(className);
    if (light) ogre_sm->destroyLight(light);
    light = NULL;
    all_lights.erase(this);
    corona.release();
    GfxNode::destroy();
}

void GfxLight::updateCorona (const Vector3 &cam_pos)
{
    if (dead) THROW_DEAD(className);
    coronaPos = transformPositionParent(coronaLocalPos);
    corona.setPosition(coronaPos);
    Vector3 col = enabled ? fade * coronaColour : Vector3(0,0,0);
    corona.setWidth(coronaSize);
    corona.setHeight(coronaSize);
    corona.setDepth(coronaSize);

    Vector3 light_dir_ws = (cam_pos - getWorldPosition()).normalisedCopy();
    Vector3 light_aim_ws_ = getWorldOrientation() * Vector3(0,1,0);

    float angle = light_aim_ws_.dot(light_dir_ws);
    float inner = Ogre::Math::Cos(light->getSpotlightInnerAngle());
    float outer = Ogre::Math::Cos(light->getSpotlightOuterAngle());
    if (outer != inner) {
            float occlusion = std::min(std::max((angle-inner)/(outer-inner), 0.0f), 1.0f);
            col *= (1-occlusion);
    }
    corona.setAmbient(col);
}

float GfxLight::getCoronaSize (void)
{
    if (dead) THROW_DEAD(className);
    return coronaSize;
}
void GfxLight::setCoronaSize (float v)
{
    if (dead) THROW_DEAD(className);
    coronaSize = v;
}

Vector3 GfxLight::getCoronaLocalPosition (void)
{
    if (dead) THROW_DEAD(className);
    return coronaLocalPos;
}

void GfxLight::setCoronaLocalPosition (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    coronaLocalPos = v;
}

Vector3 GfxLight::getCoronaColour (void)
{
    if (dead) THROW_DEAD(className);
    return coronaColour;
}

void GfxLight::setCoronaColour (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    coronaColour = v;
}

Vector3 GfxLight::getDiffuseColour (void)
{
    if (dead) THROW_DEAD(className);
    return diffuse;
}

Vector3 GfxLight::getSpecularColour (void)
{
    if (dead) THROW_DEAD(className);
    return specular;
}

void GfxLight::setDiffuseColour (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    diffuse = v;
    updateVisibility();
}

void GfxLight::setSpecularColour (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    specular = v;
    updateVisibility();
}

float GfxLight::getRange (void)
{
    if (dead) THROW_DEAD(className);
    return light->getAttenuationRange();
}

void GfxLight::setRange (float v)
{
    if (dead) THROW_DEAD(className);
    light->setAttenuation(v, 0, 0, 0);
}

Degree GfxLight::getInnerAngle (void)
{
    if (dead) THROW_DEAD(className);
    return from_ogre(light->getSpotlightInnerAngle());
}

void GfxLight::setInnerAngle (Degree v)
{
    if (dead) THROW_DEAD(className);
    light->setSpotlightInnerAngle(to_ogre(v));
}

Degree GfxLight::getOuterAngle (void)
{
    if (dead) THROW_DEAD(className);
    return from_ogre(light->getSpotlightOuterAngle());
}

void GfxLight::setOuterAngle (Degree v)
{
    if (dead) THROW_DEAD(className);
    light->setSpotlightOuterAngle(to_ogre(v));
}

bool GfxLight::isEnabled (void)
{
    if (dead) THROW_DEAD(className);
    return enabled;
}

void GfxLight::setEnabled (bool v)
{
    if (dead) THROW_DEAD(className);
    enabled = v;
    updateVisibility();
}

float GfxLight::getFade (void)
{
    if (dead) THROW_DEAD(className);
    return fade;
}
void GfxLight::setFade (float f)
{
    if (dead) THROW_DEAD(className);
    fade = f;
    updateVisibility();
}

void GfxLight::updateVisibility (void)
{
    light->setVisible(enabled && fade > 0.001);
    light->setDiffuseColour(to_ogre_cv(fade * diffuse));
    light->setSpecularColour(to_ogre_cv(fade * specular));
}
// }}}



// {{{ GFX_MATERIAL

static std::set<GfxMaterial*> dirty_mats;
static void handle_dirty_materials (void)
{
    if (dirty_mats.empty()) return;

    for (unsigned long i=0 ; i<all_bodies.size() ; ++i) {
        GfxBody *b = all_bodies[i];
        bool needs_update = false;
        for (unsigned j=0 ; j<b->getNumSubMeshes() ; ++j) {
            GfxMaterial *m = b->getMaterial(j);
            if (dirty_mats.find(m)!=dirty_mats.end()) {
                needs_update = true;
            }
        }
        if (needs_update) {
            b->updateProperties();
        }
    }

    dirty_mats.clear();
}

GfxMaterial::GfxMaterial (const std::string &name_)
  : fadingMat(NULL),
    alpha(1),
    blend(GFX_MATERIAL_OPAQUE),
    stipple(true),
    name(name_)
{
}

// FIXME: any updates to material properties need to be propagated to the GfxBodies that use them...

void GfxMaterial::setAlpha (float v)
{
    alpha = v;
    dirty_mats.insert(this);
}

void GfxMaterial::setBlend (GfxMaterialBlend v)
{
    blend = v;
    dirty_mats.insert(this);
}

void GfxMaterial::setStipple (bool v)
{
    stipple = v;
    dirty_mats.insert(this);
}

// }}}


// {{{ GFX_MATERIAL_DB

typedef std::map<std::string,GfxMaterial*> GfxMaterialDB;
static GfxMaterialDB material_db;

GfxMaterial *gfx_material_add (const std::string &name)
{
    if (gfx_material_has(name)) GRIT_EXCEPT("Material already exists: \""+name+"\"");
    GfxMaterial *r = new GfxMaterial(name);
    material_db[name] = r;
    return r;
}

GfxMaterial *gfx_material_add_or_get (const std::string &name)
{
    if (gfx_material_has(name)) return gfx_material_get(name);
    return gfx_material_add(name);
}

GfxMaterial *gfx_material_get (const std::string &name)
{
    if (!gfx_material_has(name)) GRIT_EXCEPT("Material does not exist: \""+name+"\"");
    return material_db[name];
}

bool gfx_material_has (const std::string &name)
{
    return material_db.find(name) != material_db.end();
}

// }}}


// {{{ SCENE PROPERTIES

Vector3 gfx_sun_get_diffuse (void)
{
    return from_ogre_cv(ogre_sun->getDiffuseColour());;
}

void gfx_sun_set_diffuse (const Vector3 &v)
{
    ogre_sun->setDiffuseColour(to_ogre_cv(v));
}

Vector3 gfx_sun_get_specular (void)
{
    return from_ogre_cv(ogre_sun->getSpecularColour());;
}

void gfx_sun_set_specular (const Vector3 &v)
{
    ogre_sun->setSpecularColour(to_ogre_cv(v));
}

Vector3 gfx_sun_get_direction (void)
{
    return from_ogre(ogre_sun->getDirection());
}

void gfx_sun_set_direction (const Vector3 &v)
{
    ogre_sun->setDirection(to_ogre(v));
}

Vector3 gfx_get_scene_ambient (void)
{
    return from_ogre_cv(ogre_sm->getAmbientLight());
}

void gfx_set_scene_ambient (const Vector3 &v)
{
    ogre_sm->setAmbientLight(to_ogre_cv(v));
}

Vector3 gfx_fog_get_colour (void)
{
    return fog_colour;
}

void gfx_fog_set_colour (const Vector3 &v)
{
    fog_colour = v;
    ogre_sm->setFog(Ogre::FOG_EXP2, to_ogre_cv(fog_colour), fog_density, 0, 0);
}

float gfx_fog_get_density (void)
{
    return fog_density;
}

void gfx_fog_set_density (float v)
{
    fog_density = v;
    ogre_sm->setFog(Ogre::FOG_EXP2, to_ogre_cv(fog_colour), fog_density, 0, 0);
}


Quaternion gfx_get_celestial_orientation (void)
{
    return from_ogre(ogre_celestial->getOrientation());
}

void gfx_set_celestial_orientation (const Quaternion &v)
{
    ogre_celestial->setOrientation(to_ogre(v));
}

// }}}


// {{{ RENDER

static void position_camera (bool left, const Vector3 &cam_pos, const Quaternion &cam_dir)
{
    Ogre::Camera *cam = left ? left_eye : right_eye;
    float sep = cam_separation/2;
    Vector3 p = cam_pos + cam_dir * Vector3((left?-1:1) * sep,0,0);
    cam->setPosition(to_ogre(p));
    cam->setOrientation(to_ogre(cam_dir*Quaternion(Degree(90),Vector3(1,0,0))));
}

static void update_coronas (const Vector3 &cam_pos)
{
    //const Ogre::LightList &ll = sm->_getLightsAffectingFrustum();
    for (unsigned long i=0 ; i<all_lights.size() ; ++i) {
        all_lights[i]->updateCorona(cam_pos);
    }
    
}

void gfx_render (float elapsed, const Vector3 &cam_pos, const Quaternion &cam_dir)
{
    try {
        Ogre::WindowEventUtilities::messagePump();

        update_coronas(cam_pos);

        handle_dirty_materials();

        position_camera(true, cam_pos, cam_dir);
        if (stereoscopic())
            position_camera(false, cam_pos, cam_dir);

        ogre_root->renderOneFrame(elapsed);

    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Rendering a frame");
    }
}

// }}}


void gfx_screenshot (const std::string &filename) { ogre_win->writeContentsToFile(filename); }

static GfxLastRenderStats stats_from_rt (Ogre::RenderTarget *rt)
{
    GfxLastRenderStats r;
    r.batches = rt->getBatchCount();
    r.triangles = rt->getTriangleCount();
    return r;
}

GfxLastFrameStats gfx_last_frame_stats (void)
{
    GfxLastFrameStats r;
    if (stereoscopic()) {
        r.left = stats_from_rt(ogre_win);
        r.right = stats_from_rt(anaglyph_fb->getBuffer()->getRenderTarget());
    } else {
        r.left = stats_from_rt(ogre_win);
    }
    for (int i=0 ; i<3 ; ++i) {
        r.shadow[i] = stats_from_rt(ogre_sm->getShadowTexture(i)->getBuffer()->getRenderTarget());
    }
    return r;
}

GfxRunningFrameStats gfx_running_frame_stats (void)
{
    GfxRunningFrameStats r;
    return r;
}

void gfx_reload_mesh (const std::string &name)
{
    Ogre::MeshPtr ptr = Ogre::MeshManager::getSingleton().getByName(name.substr(1), RESGRP);
    if (ptr.isNull()) GRIT_EXCEPT("Could not find mesh \""+name+"\"");
    // old skeleton may become unlinked now, may be important for resource management in future
    ptr->reload();
    if (ptr->hasSkeleton()) ptr->getSkeleton()->reload();
    for (unsigned long i=0 ; i<all_bodies.size() ; ++i) {
        GfxBody *b = all_bodies[i];
        if (b->mesh == ptr) b->reinitialise();
    }
}

void gfx_reload_texture (const std::string &name)
{
    Ogre::TexturePtr ptr = Ogre::TextureManager::getSingleton().getByName(name.substr(1), RESGRP);
    if (ptr.isNull()) GRIT_EXCEPT("Could not find texture \""+name+"\"");
    ptr->reload();
}


// {{{ LISTENERS 

struct MeshSerializerListener : Ogre::MeshSerializerListener {
    void processMaterialName (Ogre::Mesh *mesh, std::string *name)
    {
        if (shutting_down) return;
        std::string filename = mesh->getName();
        std::string dir(filename, 0, filename.rfind('/')+1);
        *name = pwd_full_ex(*name, "/"+dir, "BaseWhite");
    }

    void processSkeletonName (Ogre::Mesh *mesh, std::string *name)
    {
        if (shutting_down) return;
        std::string filename = mesh->getName();
        std::string dir(filename, 0, filename.rfind('/')+1);
        *name = pwd_full_ex(*name, "/"+dir, *name).substr(1); // strip leading '/' from this one
    }
} msl;

struct WindowEventListener : Ogre::WindowEventListener {

    void windowResized(Ogre::RenderWindow *rw)
    {
        if (shutting_down) return;
        gfx_cb->windowResized(rw->getWidth(),rw->getHeight());
        clean_compositors();
        do_reset_framebuffer();
        do_reset_compositors();
    }

    void windowClosed (Ogre::RenderWindow *rw)
    {
        (void) rw;
        if (shutting_down) return;
        gfx_cb->clickedClose();
    }

} wel;

struct LogListener : Ogre::LogListener {
    virtual void messageLogged (const std::string &message,
                                Ogre::LogMessageLevel lml,
                                bool maskDebug,
                                const std::string &logName)
    {
        (void)lml;
        (void)logName;
        if (!maskDebug) gfx_cb->messageLogged(message);
    }
} ll;

// }}}


// {{{ INIT / SHUTDOWN

size_t gfx_init (GfxCallback &cb_)
{
    try {
        gfx_cb = &cb_;

        Ogre::LogManager *lmgr = OGRE_NEW Ogre::LogManager();
        Ogre::Log *ogre_log = OGRE_NEW Ogre::Log("",false,true);
        ogre_log->addListener(&ll);
        lmgr->setDefaultLog(ogre_log);
        lmgr->setLogDetail(Ogre::LL_NORMAL);

        #ifdef WIN32
        bool use_d3d9 = getenv("GRIT_GL")==NULL;
        #else
        bool use_d3d9 = false;
        #endif


        #ifdef NO_PLUGINS
            ogre_root = OGRE_NEW Ogre::Root("","","");

            gl = OGRE_NEW Ogre::GLPlugin();
            ogre_root->installPlugin(gl);

            octree = OGRE_NEW Ogre::OctreePlugin();
            ogre_root->installPlugin(octree);

            cg = OGRE_NEW Ogre::CgPlugin();
            ogre_root->installPlugin(cg);

            #ifdef WIN32
            if (use_d3d9) {
                Ogre::D3D9Plugin *d3d9 = OGRE_NEW Ogre::D3D9Plugin();
                ogre_root->installPlugin(d3d9);
            }
            #endif
        #else
            ogre_root = OGRE_NEW Ogre::Root("plugins.cfg","","");
        #endif


        Ogre::RenderSystem *rs;
        if (use_d3d9) {
            rs = ogre_root->getRenderSystemByName("Direct3D9 Rendering Subsystem");
            rs->setConfigOption("Allow NVPerfHUD","Yes");
            rs->setConfigOption("Floating-point mode","Consistent");
            rs->setConfigOption("Video Mode","800 x 600 @ 32-bit colour");
        } else {
            rs = ogre_root->getRenderSystemByName("OpenGL Rendering Subsystem");
            rs->setConfigOption("RTT Preferred Mode","FBO");
            rs->setConfigOption("Video Mode","800 x 600");
        }
        rs->setConfigOption("sRGB Gamma Conversion",use_hwgamma?"Yes":"No");
        rs->setConfigOption("Full Screen","No");
        rs->setConfigOption("VSync","Yes");
        ogre_root->setRenderSystem(rs);

        ogre_root->initialise(true,"Grit Game Window");

        ogre_win = ogre_root->getAutoCreatedWindow();

        size_t winid;
        ogre_win->getCustomAttribute("WINDOW", &winid);
        #ifdef WIN32
        HMODULE mod = GetModuleHandle(NULL);
        HICON icon_big = (HICON)LoadImage(mod, MAKEINTRESOURCE(118), IMAGE_ICON,
                                          0, 0, LR_DEFAULTSIZE|LR_SHARED);
        HICON icon_small = (HICON)LoadImage(mod,MAKEINTRESOURCE(118), IMAGE_ICON,
                                          16, 16, LR_DEFAULTSIZE|LR_SHARED);
        SendMessage((HWND)winid, (UINT)WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) icon_big);
        SendMessage((HWND)winid, (UINT)WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM) icon_small);
        #endif

        ogre_win->setDeactivateOnFocusChange(false);
        Ogre::TextureManager::getSingleton().setVerbose(false);
        Ogre::MeshManager::getSingleton().setVerbose(false);


        Ogre::OverlayManager::getSingleton()
                .addOverlayElementFactory(new HUD::TextListOverlayElementFactory());
        ogre_root->addMovableObjectFactory(new MovableClutterFactory());
        ogre_root->addMovableObjectFactory(new RangedClutterFactory());

        ogre_sm =static_cast<Ogre::OctreeSceneManager*>(
                                               ogre_root->createSceneManager("OctreeSceneManager"));
        ogre_root_node = ogre_sm->getRootSceneNode();
        ogre_sm->setShadowCasterRenderBackFaces(false);
        Ogre::MeshManager::getSingleton().setListener(&msl);

        init_compositor();
        
        Ogre::ResourceGroupManager::getSingleton()
                .addResourceLocation(".","FileSystem",RESGRP,true);

        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        
        left_eye = ogre_sm->createCamera("LeftEye");
        left_eye->setAutoAspectRatio(true);
        right_eye = ogre_sm->createCamera("RightEye");
        right_eye->setAutoAspectRatio(true);
        anaglyph_fb.setNull();

        left_vp = ogre_win->addViewport(left_eye, 1, 0,0,1,1);
        right_vp = NULL;

        ogre_sun = ogre_sm->createLight("Sun");
        ogre_sun->setType(Ogre::Light::LT_DIRECTIONAL);

        Ogre::MeshManager::getSingleton().load(SKY_MESH,RESGRP)
            ->_setBounds(Ogre::AxisAlignedBox::BOX_INFINITE);
        ogre_sky_node = ogre_sm->getSkyCustomNode();
        ogre_celestial = ogre_sky_node->createChildSceneNode();
        ogre_sky_ent = ogre_sm->createEntity("SkyEntity", SKY_MESH);
        ogre_celestial->attachObject(ogre_sky_ent);
        ogre_sky_ent->setCastShadows(false);
        ogre_sky_ent->setRenderQueueGroup(RQ_SKY);
        
        Ogre::WindowEventUtilities::addWindowEventListener(ogre_win, &wel);

        init_options();

 

        return winid;
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Initialising graphics subsystem");
    }
}

HUD::RootPtr gfx_init_hud (void)
{
    return HUD::RootPtr(new HUD::Root(ogre_win->getWidth(),ogre_win->getHeight()));
}

void gfx_shutdown (void)
{
    try {
        if (shutting_down) return;
        shutting_down = true;
        material_db.clear();
        clean_compositors();
        anaglyph_fb.setNull();
        if (ogre_sm && ogre_root) ogre_root->destroySceneManager(ogre_sm);
        if (ogre_root) OGRE_DELETE ogre_root;
        #ifdef NO_PLUGINS
            OGRE_DELETE gl;
            OGRE_DELETE octree;
            OGRE_DELETE cg;
            #ifdef WIN32
                OGRE_DELETE d3d9;
            #endif
        #endif
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Shutting down graphics subsystem");
    }

}

// }}}
