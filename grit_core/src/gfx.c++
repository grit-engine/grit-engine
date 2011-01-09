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
#  include "OgreParticleFXPlugin.h"
#  include "OgreCgPlugin.h"
#  ifdef WIN32
#    include "OgreD3D9Plugin.h"
#  endif
#endif

#include "gfx.h"
#include "HUD.h"
#include "LuaParticleSystem.h"
#include "Clutter.h"
#include "path_util.h"
#include "math_util.h"


bool use_hwgamma = getenv("GRIT_NOHWGAMMA")==NULL;


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
    Ogre::ParticleFXPlugin *pfx;
    Ogre::CgPlugin *cg;
    #ifdef WIN32
        Ogre::D3D9Plugin *d3d9;
    #endif
#endif

GfxCallback *gfx_cb;
bool shutting_down = false;
float cam_separation = 0;


bool stereoscopic (void) 
{ return gfx_option(GFX_CROSS_EYE) || gfx_option(GFX_ANAGLYPH); }

int freshname_counter = 0;
std::string freshname (void)
{
    std::stringstream ss;
    ss << "F:" << freshname_counter;
    freshname_counter++;
    return ss.str();
}


// {{{ FRAMEBUFFER / COMPOSITOR

#define ANAGLYPH_COMPOSITOR "system/AnaglyphCompositor"
#define DEFERRED_COMPOSITOR "system/CoreCompositor"

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

class DeferredAmbientSunListener : public Ogre::CompositorInstance::Listener {
public:
    bool left;
    DeferredAmbientSunListener (bool left_) : left(left_) { }
    virtual ~DeferredAmbientSunListener (void) { }

    virtual void notifyMaterialRender (Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        (void) pass_id;

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

        try_set_named_constant(vp, "far_clip_distance", cam->getFarClipDistance());

        // actually we need only the z and w rows but this is just one renderable per frame so
        // not a big deal
        Ogre::Matrix4 view_proj = cam->getProjectionMatrix() * cam->getViewMatrix();
        try_set_named_constant(fp, "view_proj", view_proj);
    }
};

DeferredAmbientSunListener left_dasl(true);
DeferredAmbientSunListener right_dasl(false);

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

class DeferredLightsRenderOp : public CI::RenderSystemOperation {   

public:

    DeferredLightsRenderOp (CI* ins, const CP* p)
    {
    }
    
    virtual ~DeferredLightsRenderOp (void)
    {
    }

    virtual void execute (Ogre::SceneManager *sm, Ogre::RenderSystem *rs)
    {
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

static void init_compositor (void)
{
    Ogre::CompositorManager::getSingleton()
        .registerCustomCompositionPass("DeferredLights", new DeferredLightsPass());
}


static void add_deferred_compositor (bool left)
{
    Ogre::Viewport *vp = left ? left_vp : right_vp;
    CI *def_comp = Ogre::CompositorManager::getSingleton()
        .addCompositor(vp,DEFERRED_COMPOSITOR);
    def_comp->setEnabled(true);
    def_comp->addListener(left ? &left_dasl : &right_dasl);
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
        Ogre::TextureManager::getSingleton().remove("system/AnaglyphFB");
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
            "system/AnaglyphFB", "GRIT", Ogre::TEX_TYPE_2D,
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
            .getByName("system/AnaglyphCompositorMaterialDesaturated");
        rtt_mat->load();
        rtt_mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)
               ->setTextureName("system/AnaglyphFB");
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
            .getByName("system/AnaglyphCompositorMaterialDesaturated");
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


// {{{ GFX_BODY

#define THROW_DEAD(name) do { GRIT_EXCEPT(std::string(name)+" has been destroyed."); } while (0)

GfxBody::GfxBody (const std::string &mesh_name, const GfxBodyPtr &par_)
{
    dead = false;
    pos = Vector3(0,0,0);
    scl = Vector3(1,1,1);
    quat = Quaternion(1,0,0,0);
    memset(colours, 0, sizeof(colours));
    node = ogre_sm->createSceneNode();
    std::string ogre_name = mesh_name.substr(1);
    Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load(ogre_name,"GRIT");
    materials = std::vector<GfxMaterialPtr>(mesh->getNumSubMeshes());
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        std::string matname = sm->getMaterialName();
        if (!gfx_material_has(matname)) {
            CERR << "Mesh \""<<mesh_name<<"\" references non-existing material "
                 << "\""<<matname<<"\""<<std::endl;
            matname = "/BaseWhite";
            sm->setMaterialName(matname);
        }
        materials[i] = gfx_material_get(matname);
    }
    ent = ogre_sm->createEntity(freshname(), ogre_name);
    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {
        Ogre::SubEntity *se = ent->getSubEntity(i);
        if (materials[i]->getAlphaBlend()) {
            se->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
        }
    }
    node->attachObject(ent);
    par = GfxBodyPtr(NULL); setParent(par_);
    setFade(1.0f, 0);
}

GfxBody::GfxBody (const GfxBodyPtr &par_)
{
    dead = false;
    pos = Vector3(0,0,0);
    scl = Vector3(1,1,1);
    quat = Quaternion(1,0,0,0);
    memset(colours, 0, sizeof(colours));
    node = ogre_sm->createSceneNode();
    par = GfxBodyPtr(NULL); setParent(par_);
    ent = NULL;
}

GfxBody::~GfxBody (void)
{
    if (!dead) destroy();
}

void GfxBody::destroy (void)
{
    if (dead) THROW_DEAD("GfxBody");
    for (unsigned i=0 ; i<children.size() ; ++i) {
        children[i]->notifyParentDead();
    }
    if (!par.isNull()) par->notifyLostChild(this);
    ogre_sm->destroySceneNode(node->getName());
    if (ent) ogre_sm->destroyEntity(ent);
    node = NULL;
    ent = NULL;
    par = GfxBodyPtr(NULL);
    dead = true;
}

void GfxBody::notifyParentDead (void)
{
    setParent(GfxBodyPtr(NULL));
}

void GfxBody::notifyLostChild (GfxBody *child)
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

void GfxBody::notifyGainedChild (GfxBody *child)
{
    children.push_back(child);
}

const GfxBodyPtr &GfxBody::getParent (void) const
{
    if (dead) THROW_DEAD("GfxBody");
    return par;
}

void GfxBody::setParent (const GfxBodyPtr &par_)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!par_.isNull()) par_->scanForCycle(this); // check that we are not a parent of par_
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

void GfxBody::scanForCycle (GfxBody *leaf) const
{
    if (leaf==this) GRIT_EXCEPT("Parenthood must be acyclic.");
    if (par.isNull()) return;
    par->scanForCycle(leaf);
}

unsigned GfxBody::getBatches (void) const
{
    return materials.size();
}

unsigned GfxBody::getBatchesWithChildren (void) const
{
    unsigned total = getBatches();
    for (unsigned i=0 ; i<children.size() ; ++i) {
        total += children[i]->getBatchesWithChildren();
    }
    return total;
}

void GfxBody::setLocalPosition (const Vector3 &p)
{
    if (dead) THROW_DEAD("GfxBody");
    pos = p;
    node->setPosition(to_ogre(p));
}

void GfxBody::setLocalOrientation (const Quaternion &q)
{
    if (dead) THROW_DEAD("GfxBody");
    quat = q;
    node->setOrientation(to_ogre(q));
}

void GfxBody::setLocalScale (const Vector3 &s)
{
    if (dead) THROW_DEAD("GfxBody");
    scl = s;
    node->setScale(to_ogre(s));
}

Vector3 GfxBody::transformPosition (const Vector3 &v)
{
    if (dead) THROW_DEAD("GfxBody");
    if (par.isNull()) return v;
    return getWorldPosition() + getWorldScale()*(getWorldOrientation()*v);
}
Quaternion GfxBody::transformOrientation (const Quaternion &v)
{
    if (dead) THROW_DEAD("GfxBody");
    if (par.isNull()) return v;
    return getWorldOrientation()*v;
}
Vector3 GfxBody::transformScale (const Vector3 &v)
{
    if (dead) THROW_DEAD("GfxBody");
    if (par.isNull()) return v;
    return getWorldScale()*v;
}

const Vector3 &GfxBody::getLocalPosition (void)
{
    if (dead) THROW_DEAD("GfxBody");
    return pos;
}
Vector3 GfxBody::getWorldPosition (void)
{
    if (dead) THROW_DEAD("GfxBody");
    return par.isNull() ? pos : par->transformPosition(pos);
}

const Quaternion &GfxBody::getLocalOrientation (void)
{
    if (dead) THROW_DEAD("GfxBody");
    return quat;
}
Quaternion GfxBody::getWorldOrientation (void)
{
    if (dead) THROW_DEAD("GfxBody");
    return par.isNull() ? quat : par->transformOrientation(quat);
}

const Vector3 &GfxBody::getLocalScale (void)
{
    if (dead) THROW_DEAD("GfxBody");
    return scl;
}
Vector3 GfxBody::getWorldScale (void)
{
    if (dead) THROW_DEAD("GfxBody");
    return  par.isNull()? scl : par->transformScale(scl);
}

float GfxBody::getFade (void)
{
    if (dead) THROW_DEAD("GfxBody");
    return fade;
}
void GfxBody::setFade (float f, int transition)
{
    if (dead) THROW_DEAD("GfxBody");
    fade = f;
    if (!ent) return;
    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {
        Ogre::SubEntity *se = ent->getSubEntity(i);
        se->setCustomParameter(0, Ogre::Vector4(fade,0,0,0));
        GfxMaterialPtr true_material = materials[i];
        if (transition == 0) {
            se->setMaterial(true_material->regularMat);
        } else {
            if (!true_material->fadingMat.isNull()) {
                se->setMaterial(true_material->fadingMat);
            }
        }
    }
    ent->setVisible(fade > 0.001);
}

bool GfxBody::getCastShadows (void)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) return false;
    return ent->getCastShadows();
}
void GfxBody::setCastShadows (bool v)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) return;
    ent->setCastShadows(v);
}

GfxPaintColour GfxBody::getPaintColour (int i)
{
    if (dead) THROW_DEAD("GfxBody");
    return colours[i];
}
void GfxBody::setPaintColour (int i, const GfxPaintColour &c)
{
    if (dead) THROW_DEAD("GfxBody");
    colours[i] = c;
    if (!ent) return;
    for (size_t j=0 ; j<ent->getNumSubEntities() ; ++j) {
            Ogre::SubEntity *se = ent->getSubEntity(j);
            se->setCustomParameter(2*i+1,Ogre::Vector4(c.diff.r, c.diff.g, c.diff.b, c.met));
            se->setCustomParameter(2*i+2,  Ogre::Vector4(c.spec.r, c.spec.g, c.spec.b, 0));
    }
}

unsigned GfxBody::getNumBones (void)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) return 0;
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) return 0;
    return skel->getNumBones();
}

unsigned GfxBody::getBoneId (const std::string name)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL)
    if (!skel->hasBone(name)) GRIT_EXCEPT("GfxBody has no bone \""+name+"\"");
    return skel->getBone(name)->getHandle();
}

const std::string &GfxBody::getBoneName (unsigned n)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    return skel->getBone(n)->getName();
}

bool GfxBody::getBoneManuallyControlled (unsigned n)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    return skel->getBone(n)->isManuallyControlled();
}

void GfxBody::setBoneManuallyControlled (unsigned n, bool v)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    skel->getBone(n)->setManuallyControlled(v);
    skel->_notifyManualBonesDirty(); // HACK: ogre should do this itself
}

void GfxBody::setAllBonesManuallyControlled (bool v)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    for (unsigned i=0 ; i<skel->getNumBones() ; ++i) {
        skel->getBone(i)->setManuallyControlled(v);
    }
    skel->_notifyManualBonesDirty(); // HACK: ogre should do this itself
}

Vector3 GfxBody::getBoneInitialPosition (unsigned n)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getInitialPosition());
}

Vector3 GfxBody::getBoneWorldPosition (unsigned n)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->_getDerivedPosition());
}

Vector3 GfxBody::getBoneLocalPosition (unsigned n)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getPosition());
}

Quaternion GfxBody::getBoneInitialOrientation (unsigned n)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getInitialOrientation());
}

Quaternion GfxBody::getBoneWorldOrientation (unsigned n)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->_getDerivedOrientation());
}

Quaternion GfxBody::getBoneLocalOrientation (unsigned n)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getOrientation());
}


void GfxBody::setBoneLocalPosition (unsigned n, const Vector3 &v)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    bone->setPosition(to_ogre(v));
}

void GfxBody::setBoneLocalOrientation (unsigned n, const Quaternion &v)
{
    if (dead) THROW_DEAD("GfxBody");
    if (!ent) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel==NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    bone->setOrientation(to_ogre(v));
}

// }}}


// {{{ GFX_MATERIAL

GfxMaterial::GfxMaterial (const std::string &name_)
  : fadingMat(NULL),
    alpha(1),
    alphaBlend(false),
    name(name_)
{
}

void GfxMaterial::setAlpha (float v)
{
    alpha = v;
}

void GfxMaterial::setAlphaBlend (bool v)
{
    alphaBlend = v;
}

// }}}


// {{{ GFX_MATERIAL_DB

typedef std::map<std::string,GfxMaterialPtr> GfxMaterialDB;
static GfxMaterialDB material_db;

GfxMaterialPtr gfx_material_add (const std::string &name)
{
    if (gfx_material_has(name)) GRIT_EXCEPT("Material already exists: \""+name+"\"");
    GfxMaterialPtr r = GfxMaterialPtr(new GfxMaterial(name));
    material_db[name] = r;
    return r;
}

GfxMaterialPtr gfx_material_add_or_get (const std::string &name)
{
    if (gfx_material_has(name)) return gfx_material_get(name);
    return gfx_material_add(name);
}

GfxMaterialPtr gfx_material_get (const std::string &name)
{
    if (!gfx_material_has(name)) GRIT_EXCEPT("Material does not exist: \""+name+"\"");
    return material_db[name];
}

bool gfx_material_has (const std::string &name)
{
    return material_db.find(name) != material_db.end();
}

// }}}


GfxRGB gfx_sun_get_diffuse (void)
{
    return from_ogre(ogre_sun->getDiffuseColour());;
}

void gfx_sun_set_diffuse (const GfxRGB &v)
{
    ogre_sun->setDiffuseColour(to_ogre(v));
}

GfxRGB gfx_sun_get_specular (void)
{
    return from_ogre(ogre_sun->getSpecularColour());;
}

void gfx_sun_set_specular (const GfxRGB &v)
{
    ogre_sun->setSpecularColour(to_ogre(v));
}

Vector3 gfx_sun_get_direction (void)
{
    return from_ogre(ogre_sun->getDirection());
}

void gfx_sun_set_direction (const Vector3 &v)
{
    ogre_sun->setDirection(to_ogre(v));
}

GfxRGB gfx_get_ambient (void)
{
    return from_ogre(ogre_sm->getAmbientLight());
}

void gfx_set_ambient (const GfxRGB &v)
{
    ogre_sm->setAmbientLight(to_ogre(v));
}


static GfxRGB fog_colour;
static float fog_density;

GfxRGB gfx_fog_get_colour (void)
{
    return fog_colour;
}

void gfx_fog_set_colour (const GfxRGB &v)
{
    fog_colour = v;
    ogre_sm->setFog(Ogre::FOG_EXP2, to_ogre(fog_colour), fog_density, 0, 0);
}

float gfx_fog_get_density (void)
{
    return fog_density;
}

void gfx_fog_set_density (float v)
{
    fog_density = v;
    ogre_sm->setFog(Ogre::FOG_EXP2, to_ogre(fog_colour), fog_density, 0, 0);
}


Quaternion gfx_get_celestial_orientation (void)
{
    return from_ogre(ogre_celestial->getOrientation());
}

void gfx_set_celestial_orientation (const Quaternion &v)
{
    ogre_celestial->setOrientation(to_ogre(v));
}


// {{{ RENDER

static void position_camera (bool left, const Vector3 &cam_pos, const Quaternion &cam_dir)
{
    Ogre::Camera *cam = left ? left_eye : right_eye;
    float sep = cam_separation/2;
    Vector3 p = cam_pos + cam_dir * Vector3((left?-1:1) * sep,0,0);
    cam->setPosition(to_ogre(p));
    cam->setOrientation(to_ogre(cam_dir*Quaternion(Degree(90),Vector3(1,0,0))));
}

void gfx_render (float elapsed, const Vector3 &cam_pos, const Quaternion &cam_dir)
{
    try {
        Ogre::WindowEventUtilities::messagePump();

        position_camera(true, cam_pos, cam_dir);
        if (stereoscopic())
            position_camera(false, cam_pos, cam_dir);
        ogre_root->renderOneFrame(elapsed);

    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Rendering a frame");
    }
}

// }}}


void gfx_screenshot (const std::string &filename)
{
    ogre_win->writeContentsToFile(filename);
}

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

void gfx_reload_resources (void)
{
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

            Ogre::GLPlugin *gl = OGRE_NEW Ogre::GLPlugin();
            ogre_root->installPlugin(gl);

            Ogre::OctreePlugin *octree =
                    OGRE_NEW Ogre::OctreePlugin();
            ogre_root->installPlugin(octree);

            Ogre::ParticleFXPlugin *pfx =
                    OGRE_NEW Ogre::ParticleFXPlugin();
            ogre_root->installPlugin(pfx);

            Ogre::CgPlugin *cg =
                    OGRE_NEW Ogre::CgPlugin();
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
        HICON icon_big = (HICON)LoadImage(mod,MAKEINTRESOURCE(118),IMAGE_ICON,0,0,LR_DEFAULTSIZE|LR_SHARED);
        HICON icon_small = (HICON)LoadImage(mod,MAKEINTRESOURCE(118),IMAGE_ICON,16,16,LR_DEFAULTSIZE|LR_SHARED);
        SendMessage((HWND)winid, (UINT)WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) icon_big);
        SendMessage((HWND)winid, (UINT)WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM) icon_small);
        #endif

        ogre_win->setDeactivateOnFocusChange(false);
        Ogre::TextureManager::getSingleton().setVerbose(false);
        Ogre::MeshManager::getSingleton().setVerbose(false);


        Ogre::ParticleSystemManager::getSingleton()
                .addAffectorFactory(new LuaParticleAffectorFactory());
        Ogre::ParticleSystemManager::getSingleton()
                .addRendererFactory(new LuaParticleRendererFactory());
        Ogre::OverlayManager::getSingleton()
                .addOverlayElementFactory(new HUD::TextListOverlayElementFactory());
        ogre_root->addMovableObjectFactory(new MovableClutterFactory());
        ogre_root->addMovableObjectFactory(new RangedClutterFactory());

        ogre_sm = static_cast<Ogre::OctreeSceneManager*>(ogre_root->createSceneManager("OctreeSceneManager"));
        ogre_root_node = ogre_sm->getRootSceneNode();
        ogre_sm->setShadowCasterRenderBackFaces(false);
        Ogre::MeshManager::getSingleton().setListener(&msl);

        init_compositor();
        
        Ogre::ResourceGroupManager::getSingleton()
                .addResourceLocation(".","FileSystem","GRIT",true);

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

        Ogre::MeshManager::getSingleton().load("system/SkyCube.mesh","GRIT")
            ->_setBounds(Ogre::AxisAlignedBox::BOX_INFINITE);
        ogre_sky_node = ogre_sm->getSkyCustomNode();
        ogre_celestial = ogre_sky_node->createChildSceneNode();
        ogre_sky_ent = ogre_sm->createEntity("SkyEntity", "system/SkyCube.mesh");
        ogre_celestial->attachObject(ogre_sky_ent);
        ogre_sky_ent->setCastShadows(false);
        ogre_sky_ent->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
        
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
            OGRE_DELETE pfx;
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
