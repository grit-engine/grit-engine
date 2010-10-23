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
Ogre::Camera *left_eye;
Ogre::Camera *right_eye;
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

void do_reset_compositors (void)
{
        if (Ogre::CompositorManager::getSingleton().hasCompositorChain(left_vp)) {
            Ogre::CompositorManager::getSingleton()
                .removeCompositor(left_vp,"system/AnaglyphCompositor");
            Ogre::CompositorManager::getSingleton().removeCompositorChain(left_vp);
        }

        if (gfx_get_option(GFX_CROSSEYE)) {
        } else if (gfx_get_option(GFX_ANAGLYPH)) {
            Ogre::CompositorManager::getSingleton()
                .addCompositor(left_vp,"system/AnaglyphCompositor");
            Ogre::CompositorManager::getSingleton()
                .setCompositorEnabled(left_vp,"system/AnaglyphCompositor",true);
        }
}

void do_reset_framebuffer (void)
{
        if (gfx_get_option(GFX_CROSSEYE)) {
            left_vp->setDimensions(0,0,0.5,1);
        } else {
            left_vp->setDimensions(0,0,1,1);
        }

        unsigned width = left_vp->getActualWidth();
        unsigned height = left_vp->getActualHeight();

        // get rid of everything that might be set up already
        if (right_vp != NULL) {
            right_vp->getTarget()->removeViewport(right_vp->getZOrder());
            right_vp = NULL;
        }

        if (!anaglyph_fb.isNull()) {
            Ogre::TextureManager::getSingleton().remove("system/AnaglyphFB");
            anaglyph_fb.setNull();
        }


        if (gfx_get_option(GFX_CROSSEYE)) {
            right_vp = ogre_win->addViewport(right_eye, 2, 0.5,0,0.5,1);
            right_vp->setOverlaysEnabled(false);
        } else if (gfx_get_option(GFX_ANAGLYPH)) {
            // ok if we're using anaglyph rendering we'll need a separate rtt for the right eye
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

        do_reset_compositors();
}

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
        do_reset_framebuffer();
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
        if (!maskDebug)
            gfx_cb->messageLogged(message);
    }
} ll;

HUD::TextListOverlayElementFactory tloef;

GfxBoolOption gfx_bool_options[] = {
    GFX_AUTOUPDATE,
    GFX_SHADOW_RECEIVE,
    GFX_SHADOW_CAST,
    GFX_VSYNC,
    GFX_FULLSCREEN,
    GFX_FOG,
    GFX_WIREFRAME,
    GFX_ANAGLYPH,
    GFX_CROSSEYE,
    GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST,
    GFX_SHADOW_AGGRESSIVE_FOCUS_REGION,
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
        case GFX_WIREFRAME: return "GFX_WIREFRAME";
        case GFX_ANAGLYPH: return "GFX_ANAGLYPH";
        case GFX_CROSSEYE: return "GFX_CROSSEYE";
        case GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST: return "SHADOW_SIMPLE_OPTIMAL_ADJUST";
        case GFX_SHADOW_AGGRESSIVE_FOCUS_REGION: return "SHADOW_AGGRESSIVE_FOCUS_REGION";
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
    else if (s=="WIREFRAME") { t = 0; o0 = GFX_WIREFRAME; }
    else if (s=="ANAGLYPH") { t = 0; o0 = GFX_ANAGLYPH; }
    else if (s=="CROSSEYE") { t = 0; o0 = GFX_CROSSEYE; }
    else if (s=="SHADOW_SIMPLE_OPTIMAL_ADJUST") { t = 0; o0 = GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST; }
    else if (s=="SHADOW_AGGRESSIVE_FOCUS_REGION") { t = 0; o0 = GFX_SHADOW_AGGRESSIVE_FOCUS_REGION; }
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
    bool reset_vr = flush;
    bool reset_anaglyph_params = flush;

    for (unsigned i=0 ; i<sizeof(gfx_bool_options)/sizeof(*gfx_bool_options) ; ++i) {
        GfxBoolOption o = gfx_bool_options[i];
        bool v_old = options_bool[o];
        bool v_new = new_options_bool[o];
        if (v_old == v_new) continue;
        switch (o) {
            case GFX_AUTOUPDATE: break;
            case GFX_CROSSEYE:
            case GFX_ANAGLYPH:
            reset_framebuffer = true;
            reset_vr = true;
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
            reset_vr = true;
            break;
            case GFX_NEAR_CLIP:
            left_eye->setNearClipDistance(v_new);
            right_eye->setNearClipDistance(v_new);
            reset_vr = true;
            break;
            case GFX_FAR_CLIP:
            left_eye->setFarClipDistance(v_new);
            right_eye->setFarClipDistance(v_new);
            reset_vr = true;
            break;
            case GFX_EYE_SEPARATION:
            case GFX_MONITOR_HEIGHT:
            case GFX_MONITOR_EYE_DISTANCE:
            case GFX_MIN_PERCEIVED_DEPTH:
            case GFX_MAX_PERCEIVED_DEPTH:
            reset_vr = true;
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

        p->setOptimalAdjustFactor(0, gfx_get_option(GFX_SHADOW_OPTIMAL_ADJUST0));
        p->setOptimalAdjustFactor(1, gfx_get_option(GFX_SHADOW_OPTIMAL_ADJUST1));
        p->setOptimalAdjustFactor(2, gfx_get_option(GFX_SHADOW_OPTIMAL_ADJUST2));

        p->setUseAggressiveFocusRegion(gfx_get_option(GFX_SHADOW_AGGRESSIVE_FOCUS_REGION));

        p->setCameraLightDirectionThreshold(
            Ogre::Degree(gfx_get_option(GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD)));
        p->setUseSimpleOptimalAdjust(gfx_get_option(GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST));

        Ogre::PSSMShadowCameraSetup::SplitPointList boundaries;
        boundaries.push_back(gfx_get_option(GFX_SHADOW_START));
        boundaries.push_back(gfx_get_option(GFX_SHADOW_END0));
        boundaries.push_back(gfx_get_option(GFX_SHADOW_END1));
        boundaries.push_back(gfx_get_option(GFX_SHADOW_END2));
        p->setSplitPoints(boundaries);
        p->setSplitPadding(gfx_get_option(GFX_SHADOW_PADDING));

        ogre_sm->setShadowCameraSetup(Ogre::ShadowCameraSetupPtr(p));

    }

    if (reset_shadows) {
        if (gfx_get_option(GFX_SHADOW_CAST)) {
            ogre_sm->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED);
        } else {
            ogre_sm->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
        }
    }

    if (reset_fullscreen) {
        unsigned width = gfx_get_option(GFX_FULLSCREEN_WIDTH);
        unsigned height = gfx_get_option(GFX_FULLSCREEN_HEIGHT);

        if (gfx_get_option(GFX_FULLSCREEN)) {
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

    if (reset_framebuffer) {
        do_reset_framebuffer();

    }

    if (reset_vr) {
        if (gfx_get_option(GFX_CROSSEYE) || gfx_get_option(GFX_ANAGLYPH)) {
            float FOV = gfx_get_option(GFX_FOV);
            float monitor_height = gfx_get_option(GFX_MONITOR_HEIGHT);
            float distance = gfx_get_option(GFX_MONITOR_EYE_DISTANCE);
            float eye_separation = gfx_get_option(GFX_EYE_SEPARATION);
            float min = gfx_get_option(GFX_MIN_PERCEIVED_DEPTH);
            float max = gfx_get_option(GFX_MAX_PERCEIVED_DEPTH);

            float s = 2*tan((FOV/2)/180*M_PI)/monitor_height * (eye_separation * (1-distance/max));
            left_eye->setFrustumOffset(s/2);
            right_eye->setFrustumOffset(-s/2);
            left_eye->setFocalLength(1);
            right_eye->setFocalLength(1);

            float c = 2*tan((FOV/2)/180*M_PI)/monitor_height * (eye_separation * (1-distance/min));
            c = gfx_get_option(GFX_NEAR_CLIP) * (s - c);
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

        Ogre::Vector3 left_mask(gfx_get_option(GFX_ANAGLYPH_LEFT_RED_MASK),
                                gfx_get_option(GFX_ANAGLYPH_LEFT_GREEN_MASK),
                                gfx_get_option(GFX_ANAGLYPH_LEFT_BLUE_MASK));
        Ogre::Vector3 right_mask(gfx_get_option(GFX_ANAGLYPH_RIGHT_RED_MASK),
                                 gfx_get_option(GFX_ANAGLYPH_RIGHT_GREEN_MASK),
                                 gfx_get_option(GFX_ANAGLYPH_RIGHT_BLUE_MASK));
        p->getFragmentProgramParameters()
            ->setNamedConstant("desaturation",gfx_get_option(GFX_ANAGLYPH_DESATURATION));
        p->getFragmentProgramParameters()->setNamedConstant("left_mask",left_mask);
        p->getFragmentProgramParameters()->setNamedConstant("right_mask",right_mask);

        do_reset_compositors();
    }

}

static void init_options (void)
{
    valid_option(GFX_AUTOUPDATE, truefalse);

    valid_option(GFX_SHADOW_RECEIVE, truefalse);
    valid_option(GFX_SHADOW_CAST, truefalse);
    valid_option(GFX_VSYNC, truefalse);
    valid_option(GFX_FULLSCREEN, truefalse);
    valid_option(GFX_FOG, truefalse);
    valid_option(GFX_WIREFRAME, truefalse);
    valid_option(GFX_ANAGLYPH, truefalse);
    valid_option(GFX_CROSSEYE, truefalse);
    valid_option(GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST, truefalse);
    valid_option(GFX_SHADOW_AGGRESSIVE_FOCUS_REGION, truefalse);

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


    gfx_set_option(GFX_AUTOUPDATE, false);

    gfx_set_option(GFX_SHADOW_RECEIVE, true);
    gfx_set_option(GFX_SHADOW_CAST, true);
    gfx_set_option(GFX_VSYNC, true);
    gfx_set_option(GFX_FULLSCREEN, false);
    gfx_set_option(GFX_FOG, true);
    gfx_set_option(GFX_WIREFRAME, false);
    gfx_set_option(GFX_ANAGLYPH, false);
    gfx_set_option(GFX_CROSSEYE, false);

    gfx_set_option(GFX_FULLSCREEN_WIDTH, 800);
    gfx_set_option(GFX_FULLSCREEN_HEIGHT, 600);
    gfx_set_option(GFX_SHADOW_RES, 1024);
    gfx_set_option(GFX_RAM, 128);

    gfx_set_option(GFX_FOV, 55);
    gfx_set_option(GFX_NEAR_CLIP, 0.355);
    gfx_set_option(GFX_FAR_CLIP, 1000);
    gfx_set_option(GFX_EYE_SEPARATION, 0.06);
    gfx_set_option(GFX_MONITOR_HEIGHT, 0.27);
    gfx_set_option(GFX_MONITOR_EYE_DISTANCE, 0.6);
    gfx_set_option(GFX_MIN_PERCEIVED_DEPTH, 0.3);
    gfx_set_option(GFX_MAX_PERCEIVED_DEPTH, 2);
    gfx_set_option(GFX_SHADOW_START, 0.2);
    gfx_set_option(GFX_SHADOW_END0, 20);
    gfx_set_option(GFX_SHADOW_END1, 50);
    gfx_set_option(GFX_SHADOW_END2, 200);
    gfx_set_option(GFX_SHADOW_OPTIMAL_ADJUST0, 3);
    gfx_set_option(GFX_SHADOW_OPTIMAL_ADJUST1, 1);
    gfx_set_option(GFX_SHADOW_OPTIMAL_ADJUST2, 1);
    gfx_set_option(GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD, 35);
    gfx_set_option(GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST, true);
    gfx_set_option(GFX_SHADOW_PADDING, 0.8);
    gfx_set_option(GFX_SHADOW_AGGRESSIVE_FOCUS_REGION, true);
    gfx_set_option(GFX_ANAGLYPH_DESATURATION, 0.7);
    gfx_set_option(GFX_ANAGLYPH_LEFT_RED_MASK, 1);
    gfx_set_option(GFX_ANAGLYPH_LEFT_GREEN_MASK, 0);
    gfx_set_option(GFX_ANAGLYPH_LEFT_BLUE_MASK, 0);
    gfx_set_option(GFX_ANAGLYPH_RIGHT_RED_MASK, 0);
    gfx_set_option(GFX_ANAGLYPH_RIGHT_GREEN_MASK, 1);
    gfx_set_option(GFX_ANAGLYPH_RIGHT_BLUE_MASK, 1);

    options_update(true);

    gfx_set_option(GFX_AUTOUPDATE, true);
}

void gfx_set_option (GfxBoolOption o, bool v)
{
    valid_option_bool[o]->maybeThrow(v);
    try {
        new_options_bool[o] = v;
        if (new_options_bool[GFX_AUTOUPDATE]) options_update(false);
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Setting graphics option");
    }
}
bool gfx_get_option (GfxBoolOption o)
{
    return options_bool[o];
}

void gfx_set_option (GfxIntOption o, int v)
{
    valid_option_int[o]->maybeThrow(v);
    try {
        new_options_int[o] = v;
        if (new_options_bool[GFX_AUTOUPDATE]) options_update(false);
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Setting graphics option");
    }
}
int gfx_get_option (GfxIntOption o)
{
    return options_int[o];
}

void gfx_set_option (GfxFloatOption o, float v)
{
    valid_option_float[o]->maybeThrow(v);
    try {
        new_options_float[o] = v;
        if (new_options_bool[GFX_AUTOUPDATE]) options_update(false);
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Setting graphics option");
    }
}
float gfx_get_option (GfxFloatOption o)
{
    return options_float[o];
}

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
        ogre_sm->setShadowCasterRenderBackFaces(false);
        Ogre::MeshManager::getSingleton().setListener(&msl);
        
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
        
        Ogre::WindowEventUtilities::addWindowEventListener(ogre_win, &wel);

        init_options();

        return winid;
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Initialising graphics subsystem");
    }
}

static Ogre::Vector3 to_ogre_ (const Vector3 &v) { return Ogre::Vector3(v.x,v.y,v.z); }
static Ogre::Quaternion to_ogre_ (const Quaternion &v) { return Ogre::Quaternion(v.w,v.x,v.y,v.z); }

static void position_camera (bool left, const Vector3 &cam_pos, const Quaternion &cam_dir,
                             float cam_chase)
{
    Ogre::Camera *cam = left ? left_eye : right_eye;
    float sep = cam_separation/2;
    Vector3 p = cam_pos + cam_dir * Vector3((left?-1:1) * sep,-cam_chase,0);
    cam->setPosition(to_ogre_(p));
    cam->setOrientation(to_ogre_(cam_dir*Quaternion(Degree(90),Vector3(1,0,0))));
}

void gfx_render (float elapsed, const Vector3 &cam_pos, const Quaternion &cam_dir, float cam_chase)
{
    try {
        position_camera(true, cam_pos, cam_dir, cam_chase);
        if (gfx_get_option(GFX_ANAGLYPH) || gfx_get_option(GFX_CROSSEYE))
            position_camera(false, cam_pos, cam_dir, cam_chase);
        ogre_root->renderOneFrame(elapsed);

    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Rendering a frame");
    }
}

void gfx_pump (void)
{
    try {
        Ogre::WindowEventUtilities::messagePump();
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Rendering a frame");
    }
}

void gfx_reload_resources (void)
{
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

