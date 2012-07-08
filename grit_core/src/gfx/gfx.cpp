/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

#include "../path_util.h"

#include "gfx_internal.h"
#include "gfx_option.h"
#include "Clutter.h"
#include "GfxMaterial.h"
#include "GfxBody.h"
#include "GfxSkyMaterial.h"
#include "GfxSkyBody.h"

Ogre::Root *ogre_root;
Ogre::OctreeSceneManager *ogre_sm;
Ogre::SceneNode *ogre_root_node;
Ogre::Camera *left_eye;
Ogre::Camera *right_eye;
Ogre::Light *ogre_sun;
Ogre::SceneNode *ogre_sky_node;
Ogre::Viewport *overlay_vp;
Ogre::Viewport *left_vp;
Ogre::Viewport *right_vp;
Ogre::TexturePtr anaglyph_fb;
Ogre::RenderWindow *ogre_win;
#ifdef NO_PLUGINS
    Ogre::GLPlugin *gl;
    Ogre::OctreePlugin *octree;
    Ogre::CgPlugin *cg;
    #ifdef WIN32
        Ogre::D3D9Plugin *d3d9;
    #endif
#endif


GfxMaterialDB material_db;

bool use_hwgamma = false; //getenv("GRIT_NOHWGAMMA")==NULL;


GfxCallback *gfx_cb;
bool shutting_down = false;
float cam_separation = 0;

Vector3 fog_colour;
float fog_density;


// {{{ utilities

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

const GfxStringMap gfx_empty_string_map;

fast_erase_vector<GfxBody*> gfx_all_bodies;

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
void clean_compositors (void)
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
        
// The deferred lights are not just a screen space quad
// so use a custom composition pass to render them
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

void do_reset_compositors (void)
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

void update_coronas (const Vector3 &cam_pos);

void gfx_render (float elapsed, const Vector3 &cam_pos, const Quaternion &cam_dir)
{
    try {
        Ogre::WindowEventUtilities::messagePump();

        update_coronas(cam_pos);

        handle_dirty_materials();
        handle_dirty_sky_materials();

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
                                const std::string &logName,
                                bool& skipThisMessage )

    {
        (void)lml;
        (void)logName;
        (void)skipThisMessage;
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
                d3d9 = OGRE_NEW Ogre::D3D9Plugin();
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

        ogre_sky_node = ogre_sm->getSkyCustomNode();
        
        Ogre::WindowEventUtilities::addWindowEventListener(ogre_win, &wel);

        gfx_option_init();

 

        return winid;
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Initialising graphics subsystem");
    }
}

HUD::RootPtr gfx_init_hud (void)
{
    return HUD::RootPtr(new HUD::Root(ogre_win->getWidth(),ogre_win->getHeight()));
}

GfxMaterialType gfx_material_type (const std::string &name)
{
    GFX_MAT_SYNC;
    if (!gfx_material_has_any(name)) GRIT_EXCEPT("Non-existent material: \""+name+"\"");
    GfxBaseMaterial *mat = material_db[name];
    if (dynamic_cast<GfxMaterial*>(mat) != NULL) return GFX_MATERIAL;
    if (dynamic_cast<GfxSkyMaterial*>(mat) != NULL) return GFX_SKY_MATERIAL;
    GRIT_EXCEPT("Internal error: unrecognised kind of material");
    return GFX_MATERIAL; // never get here
}

bool gfx_material_has_any (const std::string &name)
{
    GFX_MAT_SYNC;
    return material_db.find(name) != material_db.end();
}

void gfx_material_add_dependencies (const std::string &name, GfxDiskResource *into)
{
    if (!gfx_material_has_any(name)) GRIT_EXCEPT("Non-existent material: \""+name+"\"");
    material_db[name]->addDependencies(into);
}

void gfx_shutdown (void)
{
    try {
        if (shutting_down) return;
        shutting_down = true;
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
