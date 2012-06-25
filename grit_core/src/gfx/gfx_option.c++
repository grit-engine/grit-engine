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

#include <string>
#include <map>

#include "gfx_internal.h"
#include "gfx_option.h"
#include "../option.h"
#include "../CentralisedLog.h"
    
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

void gfx_option_init (void)
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
    valid_option(GFX_RAM, new ValidOptionRange<int>(0,16384));

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
    valid_option_bool[o]->maybeThrow("Graphics", v);
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
    valid_option_int[o]->maybeThrow("Graphics", v);
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
    valid_option_float[o]->maybeThrow("Graphics", v);
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

