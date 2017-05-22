/* Copyright (c) The Grit Game Engine authors 2016 
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
#include "gfx_pipeline.h"
#include "gfx_option.h"
#include "../option.h"
#include <centralised_log.h>
    
GfxBoolOption gfx_bool_options[] = {
    GFX_AUTOUPDATE,
    GFX_SHADOW_RECEIVE,
    GFX_SHADOW_CAST,
    GFX_VSYNC,
    GFX_FULLSCREEN,

    GFX_FOG,
    GFX_WIREFRAME,
    GFX_WIREFRAME_SOLID,
    GFX_ANAGLYPH,
    GFX_CROSS_EYE,

    GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST,
    GFX_SHADOW_AGGRESSIVE_FOCUS_REGION,
    GFX_SHADOW_FILTER_DITHER,
    GFX_SHADOW_FILTER_DITHER_TEXTURE,
    GFX_SHADOW_EMULATE_PCF,

    GFX_POST_PROCESSING,
    GFX_RENDER_PARTICLES,
    GFX_POINT_LIGHTS,
    GFX_RENDER_SKY,
    GFX_RENDER_HUD,

    GFX_RENDER_FIRST_PERSON,
    GFX_UPDATE_MATERIALS,
};  

GfxIntOption gfx_int_options[] = {
    GFX_FULLSCREEN_WIDTH,
    GFX_FULLSCREEN_HEIGHT,
    GFX_SHADOW_RES,
    GFX_SHADOW_FILTER_TAPS,
    GFX_BLOOM_ITERATIONS,
    GFX_RAM,
    GFX_DEBUG_MODE,
};      
        
GfxFloatOption gfx_float_options[] = {
    GFX_FOV,
    GFX_NEAR_CLIP,
    GFX_FAR_CLIP,
    GFX_FIRST_PERSON_NEAR_CLIP,
    GFX_FIRST_PERSON_FAR_CLIP,
    GFX_EYE_SEPARATION,
    GFX_MONITOR_HEIGHT,

    GFX_MONITOR_EYE_DISTANCE,
    GFX_MIN_PERCEIVED_DEPTH,
    GFX_MAX_PERCEIVED_DEPTH,
    GFX_SHADOW_START,
    GFX_SHADOW_END0,

    GFX_SHADOW_END1,
    GFX_SHADOW_END2,
    GFX_SHADOW_FADE_START,
    GFX_SHADOW_FILTER_SIZE,
    GFX_SHADOW_OPTIMAL_ADJUST0,

    GFX_SHADOW_OPTIMAL_ADJUST1,
    GFX_SHADOW_OPTIMAL_ADJUST2,
    GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD,
    GFX_SHADOW_PADDING,
    GFX_SHADOW_SPREAD_FACTOR0,

    GFX_SHADOW_SPREAD_FACTOR1,
    GFX_SHADOW_SPREAD_FACTOR2,
    GFX_ANAGLYPH_LEFT_RED_MASK,
    GFX_ANAGLYPH_LEFT_GREEN_MASK,
    GFX_ANAGLYPH_LEFT_BLUE_MASK,

    GFX_ANAGLYPH_RIGHT_RED_MASK,
    GFX_ANAGLYPH_RIGHT_GREEN_MASK,
    GFX_ANAGLYPH_RIGHT_BLUE_MASK,
    GFX_ANAGLYPH_DESATURATION,
    GFX_BLOOM_THRESHOLD,
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

#define TO_STRING_MACRO(x) case x: return (&#x[4])

std::string gfx_option_to_string (GfxBoolOption o)
{
    switch (o) {
        TO_STRING_MACRO(GFX_AUTOUPDATE);
        TO_STRING_MACRO(GFX_SHADOW_RECEIVE);
        TO_STRING_MACRO(GFX_SHADOW_CAST);
        TO_STRING_MACRO(GFX_VSYNC);
        TO_STRING_MACRO(GFX_FULLSCREEN);

        TO_STRING_MACRO(GFX_FOG);
        TO_STRING_MACRO(GFX_WIREFRAME);
        TO_STRING_MACRO(GFX_WIREFRAME_SOLID);
        TO_STRING_MACRO(GFX_ANAGLYPH);
        TO_STRING_MACRO(GFX_CROSS_EYE);

        TO_STRING_MACRO(GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST);
        TO_STRING_MACRO(GFX_SHADOW_AGGRESSIVE_FOCUS_REGION);
        TO_STRING_MACRO(GFX_SHADOW_FILTER_DITHER);
        TO_STRING_MACRO(GFX_SHADOW_FILTER_DITHER_TEXTURE);
        TO_STRING_MACRO(GFX_SHADOW_EMULATE_PCF);

        TO_STRING_MACRO(GFX_POST_PROCESSING);
        TO_STRING_MACRO(GFX_RENDER_PARTICLES);
        TO_STRING_MACRO(GFX_POINT_LIGHTS);
        TO_STRING_MACRO(GFX_RENDER_SKY);
        TO_STRING_MACRO(GFX_RENDER_HUD);

        TO_STRING_MACRO(GFX_RENDER_FIRST_PERSON);
        TO_STRING_MACRO(GFX_UPDATE_MATERIALS);
    }
    return "UNKNOWN_BOOL_OPTION";
}
std::string gfx_option_to_string (GfxIntOption o)
{
    switch (o) {
        TO_STRING_MACRO(GFX_FULLSCREEN_WIDTH);
        TO_STRING_MACRO(GFX_FULLSCREEN_HEIGHT);
        TO_STRING_MACRO(GFX_SHADOW_RES);
        TO_STRING_MACRO(GFX_SHADOW_FILTER_TAPS);
        TO_STRING_MACRO(GFX_BLOOM_ITERATIONS);
        TO_STRING_MACRO(GFX_RAM);
        TO_STRING_MACRO(GFX_DEBUG_MODE);
    }
    return "UNKNOWN_INT_OPTION";
}
std::string gfx_option_to_string (GfxFloatOption o)
{
    switch (o) {
        TO_STRING_MACRO(GFX_FOV);
        TO_STRING_MACRO(GFX_NEAR_CLIP);
        TO_STRING_MACRO(GFX_FAR_CLIP);
        TO_STRING_MACRO(GFX_FIRST_PERSON_NEAR_CLIP);
        TO_STRING_MACRO(GFX_FIRST_PERSON_FAR_CLIP);
        TO_STRING_MACRO(GFX_EYE_SEPARATION);
        TO_STRING_MACRO(GFX_MONITOR_HEIGHT);

        TO_STRING_MACRO(GFX_MONITOR_EYE_DISTANCE);
        TO_STRING_MACRO(GFX_MIN_PERCEIVED_DEPTH);
        TO_STRING_MACRO(GFX_MAX_PERCEIVED_DEPTH);
        TO_STRING_MACRO(GFX_SHADOW_START);
        TO_STRING_MACRO(GFX_SHADOW_END0);

        TO_STRING_MACRO(GFX_SHADOW_END1);
        TO_STRING_MACRO(GFX_SHADOW_END2);
        TO_STRING_MACRO(GFX_SHADOW_FADE_START);
        TO_STRING_MACRO(GFX_SHADOW_FILTER_SIZE);
        TO_STRING_MACRO(GFX_SHADOW_OPTIMAL_ADJUST0);

        TO_STRING_MACRO(GFX_SHADOW_OPTIMAL_ADJUST1);
        TO_STRING_MACRO(GFX_SHADOW_OPTIMAL_ADJUST2);
        TO_STRING_MACRO(GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD);
        TO_STRING_MACRO(GFX_SHADOW_PADDING);
        TO_STRING_MACRO(GFX_SHADOW_SPREAD_FACTOR0);

        TO_STRING_MACRO(GFX_SHADOW_SPREAD_FACTOR1);
        TO_STRING_MACRO(GFX_SHADOW_SPREAD_FACTOR2);
        TO_STRING_MACRO(GFX_ANAGLYPH_LEFT_RED_MASK);
        TO_STRING_MACRO(GFX_ANAGLYPH_LEFT_GREEN_MASK);
        TO_STRING_MACRO(GFX_ANAGLYPH_LEFT_BLUE_MASK);

        TO_STRING_MACRO(GFX_ANAGLYPH_RIGHT_RED_MASK);
        TO_STRING_MACRO(GFX_ANAGLYPH_RIGHT_GREEN_MASK);
        TO_STRING_MACRO(GFX_ANAGLYPH_RIGHT_BLUE_MASK);
        TO_STRING_MACRO(GFX_ANAGLYPH_DESATURATION);
        TO_STRING_MACRO(GFX_BLOOM_THRESHOLD);
    }
    return "UNKNOWN_FLOAT_OPTION";
}

#define FROM_STRING_MACRO(x,var,type) else if (s==(&#x[4])) { t = type; var = x; }
#define FROM_STRING_BOOL_MACRO(x) FROM_STRING_MACRO(x,o0,0)
#define FROM_STRING_INT_MACRO(x) FROM_STRING_MACRO(x,o1,1)
#define FROM_STRING_FLOAT_MACRO(x) FROM_STRING_MACRO(x,o2,2)

// set's t to either 0,1,2 and fills in the approriate argument
void gfx_option_from_string (const std::string &s,
                             int &t,
                             GfxBoolOption &o0,
                             GfxIntOption &o1,
                             GfxFloatOption &o2)
{
    if (s=="AUTOUPDATE") { t = 0; o0 = GFX_AUTOUPDATE; }
    FROM_STRING_BOOL_MACRO(GFX_SHADOW_RECEIVE)
    FROM_STRING_BOOL_MACRO(GFX_SHADOW_RECEIVE)
    FROM_STRING_BOOL_MACRO(GFX_SHADOW_CAST)
    FROM_STRING_BOOL_MACRO(GFX_VSYNC)
    FROM_STRING_BOOL_MACRO(GFX_FULLSCREEN)

    FROM_STRING_BOOL_MACRO(GFX_FOG)
    FROM_STRING_BOOL_MACRO(GFX_WIREFRAME)
    FROM_STRING_BOOL_MACRO(GFX_WIREFRAME_SOLID)
    FROM_STRING_BOOL_MACRO(GFX_ANAGLYPH)
    FROM_STRING_BOOL_MACRO(GFX_CROSS_EYE)

    FROM_STRING_BOOL_MACRO(GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST)
    FROM_STRING_BOOL_MACRO(GFX_SHADOW_AGGRESSIVE_FOCUS_REGION)
    FROM_STRING_BOOL_MACRO(GFX_SHADOW_FILTER_DITHER)
    FROM_STRING_BOOL_MACRO(GFX_SHADOW_FILTER_DITHER_TEXTURE)
    FROM_STRING_BOOL_MACRO(GFX_SHADOW_EMULATE_PCF)

    FROM_STRING_BOOL_MACRO(GFX_POST_PROCESSING)
    FROM_STRING_BOOL_MACRO(GFX_RENDER_PARTICLES)
    FROM_STRING_BOOL_MACRO(GFX_POINT_LIGHTS)
    FROM_STRING_BOOL_MACRO(GFX_RENDER_SKY)
    FROM_STRING_BOOL_MACRO(GFX_RENDER_HUD)

    FROM_STRING_BOOL_MACRO(GFX_RENDER_FIRST_PERSON)
    FROM_STRING_BOOL_MACRO(GFX_UPDATE_MATERIALS)


    FROM_STRING_INT_MACRO(GFX_FULLSCREEN_WIDTH)
    FROM_STRING_INT_MACRO(GFX_FULLSCREEN_HEIGHT)
    FROM_STRING_INT_MACRO(GFX_SHADOW_RES)
    FROM_STRING_INT_MACRO(GFX_SHADOW_FILTER_TAPS)
    FROM_STRING_INT_MACRO(GFX_BLOOM_ITERATIONS)

    FROM_STRING_INT_MACRO(GFX_RAM)
    FROM_STRING_INT_MACRO(GFX_DEBUG_MODE)


    FROM_STRING_FLOAT_MACRO(GFX_FOV)
    FROM_STRING_FLOAT_MACRO(GFX_NEAR_CLIP)
    FROM_STRING_FLOAT_MACRO(GFX_FAR_CLIP)
    FROM_STRING_FLOAT_MACRO(GFX_FIRST_PERSON_NEAR_CLIP)
    FROM_STRING_FLOAT_MACRO(GFX_FIRST_PERSON_FAR_CLIP)
    FROM_STRING_FLOAT_MACRO(GFX_EYE_SEPARATION)
    FROM_STRING_FLOAT_MACRO(GFX_MONITOR_HEIGHT)

    FROM_STRING_FLOAT_MACRO(GFX_MONITOR_EYE_DISTANCE)
    FROM_STRING_FLOAT_MACRO(GFX_MIN_PERCEIVED_DEPTH)
    FROM_STRING_FLOAT_MACRO(GFX_MAX_PERCEIVED_DEPTH)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_START)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_END0)

    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_END1)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_END2)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_FADE_START)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_FILTER_SIZE)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_OPTIMAL_ADJUST0)

    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_OPTIMAL_ADJUST1)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_OPTIMAL_ADJUST2)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_PADDING)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_SPREAD_FACTOR0)

    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_SPREAD_FACTOR1)
    FROM_STRING_FLOAT_MACRO(GFX_SHADOW_SPREAD_FACTOR2)
    FROM_STRING_FLOAT_MACRO(GFX_ANAGLYPH_LEFT_RED_MASK)
    FROM_STRING_FLOAT_MACRO(GFX_ANAGLYPH_LEFT_GREEN_MASK)
    FROM_STRING_FLOAT_MACRO(GFX_ANAGLYPH_LEFT_BLUE_MASK)

    FROM_STRING_FLOAT_MACRO(GFX_ANAGLYPH_RIGHT_RED_MASK)
    FROM_STRING_FLOAT_MACRO(GFX_ANAGLYPH_RIGHT_GREEN_MASK)
    FROM_STRING_FLOAT_MACRO(GFX_ANAGLYPH_RIGHT_BLUE_MASK)
    FROM_STRING_FLOAT_MACRO(GFX_ANAGLYPH_DESATURATION)
    FROM_STRING_FLOAT_MACRO(GFX_BLOOM_THRESHOLD)

    else t = -1;
}

/* Transfer settings to lower level components.
 *
 * Will regenerate anything if new_options differs to old_options, or the called can force
 * regeneration by setting 'flush'.
 */
static void options_update (bool flush)
{
    bool reset_fullscreen = flush;
    bool reset_shadowmaps = flush;
    bool reset_shadows = flush;
    bool reset_pcss = flush;
    bool reset_framebuffer = flush;
    bool reset_shadow_spread = flush;
    bool reset_shadow_dither_mode = flush;

    for (unsigned i=0 ; i<sizeof(gfx_bool_options)/sizeof(*gfx_bool_options) ; ++i) {
        GfxBoolOption o = gfx_bool_options[i];
        bool v_old = options_bool[o];
        bool v_new = new_options_bool[o];
        if (v_old == v_new) continue;
        switch (o) {
            case GFX_AUTOUPDATE: break;
            case GFX_CROSS_EYE:
            case GFX_ANAGLYPH:
            reset_framebuffer = true;
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
            case GFX_WIREFRAME_SOLID:
            break;
            case GFX_WIREFRAME:
            break;
            case GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST:
            reset_pcss = true;
            break;
            case GFX_SHADOW_AGGRESSIVE_FOCUS_REGION:
            reset_pcss = true;
            break;
            case GFX_SHADOW_FILTER_DITHER:
            case GFX_SHADOW_FILTER_DITHER_TEXTURE:
            reset_shadow_dither_mode = true;
            break;
            case GFX_SHADOW_EMULATE_PCF: break;
            case GFX_POST_PROCESSING: break;
            case GFX_RENDER_PARTICLES: break;
            case GFX_POINT_LIGHTS: break;
            case GFX_RENDER_SKY: break;
            case GFX_RENDER_HUD: break;
            case GFX_RENDER_FIRST_PERSON: break;
            case GFX_UPDATE_MATERIALS: break;
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
            case GFX_DEBUG_MODE:
            break;
            case GFX_SHADOW_RES:
            shader_scene_env.shadowRes = v_new;
            reset_shadowmaps = true;
            break;
            case GFX_SHADOW_FILTER_TAPS:
            shader_scene_env.shadowFilterTaps = v_new;
            break;
            case GFX_BLOOM_ITERATIONS: break;
        }
    }
    for (unsigned i=0 ; i<sizeof(gfx_float_options)/sizeof(*gfx_float_options) ; ++i) {
        GfxFloatOption o = gfx_float_options[i];
        float v_old = options_float[o];
        float v_new = new_options_float[o];
        if (v_old == v_new) continue;
        switch (o) {
            // the following read every frame
            case GFX_FOV:
            case GFX_NEAR_CLIP:
            case GFX_FAR_CLIP:
            case GFX_FIRST_PERSON_NEAR_CLIP:
            case GFX_FIRST_PERSON_FAR_CLIP:
            case GFX_EYE_SEPARATION:
            case GFX_MONITOR_HEIGHT:
            case GFX_MONITOR_EYE_DISTANCE:
            case GFX_MIN_PERCEIVED_DEPTH:
            case GFX_MAX_PERCEIVED_DEPTH:
            break;
            case GFX_SHADOW_START:
            shader_scene_env.shadowFadeStart = v_new;
            reset_pcss = true;
            break;
            case GFX_SHADOW_END0:
            shader_scene_env.shadowDist[0] = v_new;
            reset_pcss = true;
            break;
            break;
            case GFX_SHADOW_END1:
            shader_scene_env.shadowDist[1] = v_new;
            reset_pcss = true;
            break;
            case GFX_SHADOW_END2:
            shader_scene_env.shadowDist[2] = v_new;
            shader_scene_env.shadowFadeEnd = v_new;
            reset_pcss = true;
            break;
            case GFX_SHADOW_OPTIMAL_ADJUST0:
            case GFX_SHADOW_OPTIMAL_ADJUST1:
            case GFX_SHADOW_OPTIMAL_ADJUST2:
            case GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD:
            case GFX_SHADOW_PADDING:
            reset_pcss = true;
            break;
            case GFX_SHADOW_SPREAD_FACTOR0:
            reset_shadow_spread = true;
            break;
            case GFX_SHADOW_SPREAD_FACTOR1:
            reset_shadow_spread = true;
            break;
            case GFX_SHADOW_SPREAD_FACTOR2:
            reset_shadow_spread = true;
            break;
            case GFX_SHADOW_FADE_START:
            shader_scene_env.shadowFadeStart = v_new;
            break;
            case GFX_SHADOW_FILTER_SIZE:
            reset_shadow_spread = true;
            break;
            case GFX_ANAGLYPH_LEFT_RED_MASK:
            case GFX_ANAGLYPH_LEFT_GREEN_MASK:
            case GFX_ANAGLYPH_LEFT_BLUE_MASK:
            case GFX_ANAGLYPH_RIGHT_RED_MASK:
            case GFX_ANAGLYPH_RIGHT_GREEN_MASK:
            case GFX_ANAGLYPH_RIGHT_BLUE_MASK:
            case GFX_ANAGLYPH_DESATURATION:
            break;
            case GFX_BLOOM_THRESHOLD:
            break;
        }
    }

    options_bool = new_options_bool;
    options_int = new_options_int;
    options_float = new_options_float;

    if (reset_shadow_dither_mode) {
        if (gfx_option(GFX_SHADOW_FILTER_DITHER_TEXTURE)) {
            shader_scene_env.shadowDitherMode = GfxGslConfigEnvironment::SHADOW_DITHER_NOISE;
        } else if (gfx_option(GFX_SHADOW_FILTER_DITHER)) {
            shader_scene_env.shadowDitherMode = GfxGslConfigEnvironment::SHADOW_DITHER_PLAIN;
        } else {
            shader_scene_env.shadowDitherMode = GfxGslConfigEnvironment::SHADOW_DITHER_NONE;
        }
    }

    if (reset_shadow_spread) {
        shader_scene_env.shadowSpread[0] = gfx_option(GFX_SHADOW_SPREAD_FACTOR0)
                                         * gfx_option(GFX_SHADOW_FILTER_SIZE);
        shader_scene_env.shadowSpread[1] = gfx_option(GFX_SHADOW_SPREAD_FACTOR1)
                                         * gfx_option(GFX_SHADOW_FILTER_SIZE);
        shader_scene_env.shadowSpread[2] = gfx_option(GFX_SHADOW_SPREAD_FACTOR2)
                                         * gfx_option(GFX_SHADOW_FILTER_SIZE);
        shader_scene_env.shadowFilterSize = gfx_option(GFX_SHADOW_FILTER_SIZE);
    }

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

    if (reset_framebuffer) {
        do_reset_framebuffer();
    }

}

void gfx_option_reset (void)
{
    gfx_option(GFX_SHADOW_RECEIVE, true);
    gfx_option(GFX_SHADOW_CAST, true);
    gfx_option(GFX_VSYNC, true);
    gfx_option(GFX_FULLSCREEN, false);

    gfx_option(GFX_FOG, true);
    gfx_option(GFX_WIREFRAME, false);
    gfx_option(GFX_WIREFRAME_SOLID, true);
    gfx_option(GFX_ANAGLYPH, false);
    gfx_option(GFX_CROSS_EYE, false);

    gfx_option(GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST, true);
    gfx_option(GFX_SHADOW_AGGRESSIVE_FOCUS_REGION, true);
    gfx_option(GFX_SHADOW_FILTER_DITHER, false);
    gfx_option(GFX_SHADOW_FILTER_DITHER_TEXTURE, true);
    gfx_option(GFX_SHADOW_EMULATE_PCF, false);

    gfx_option(GFX_POST_PROCESSING, true);
    gfx_option(GFX_RENDER_PARTICLES, true);
    gfx_option(GFX_POINT_LIGHTS, true);
    gfx_option(GFX_RENDER_SKY, true);
    gfx_option(GFX_RENDER_HUD, true);

    gfx_option(GFX_RENDER_FIRST_PERSON, true);
    gfx_option(GFX_UPDATE_MATERIALS, true);


    gfx_option(GFX_FULLSCREEN_WIDTH, 800);
    gfx_option(GFX_FULLSCREEN_HEIGHT, 600);
    gfx_option(GFX_SHADOW_RES, 1024);
    gfx_option(GFX_SHADOW_FILTER_TAPS, 4);
    gfx_option(GFX_BLOOM_ITERATIONS, 0);
    gfx_option(GFX_RAM, 128);
    gfx_option(GFX_DEBUG_MODE, 0);


    gfx_option(GFX_FOV, 55.0f);
    gfx_option(GFX_NEAR_CLIP, 0.355f);
    gfx_option(GFX_FAR_CLIP, 1000.0f);
    gfx_option(GFX_FIRST_PERSON_NEAR_CLIP, 0.01f);
    gfx_option(GFX_FIRST_PERSON_FAR_CLIP, 10.0f);

    gfx_option(GFX_EYE_SEPARATION, 0.06f);
    gfx_option(GFX_MONITOR_HEIGHT, 0.27f);
    gfx_option(GFX_MONITOR_EYE_DISTANCE, 0.6f);
    gfx_option(GFX_MIN_PERCEIVED_DEPTH, 0.3f);
    gfx_option(GFX_MAX_PERCEIVED_DEPTH, 2.0f);

    gfx_option(GFX_SHADOW_START, 0.2f);
    gfx_option(GFX_SHADOW_END0, 20.0f);
    gfx_option(GFX_SHADOW_END1, 50.0f);
    gfx_option(GFX_SHADOW_END2, 200.0f);
    gfx_option(GFX_SHADOW_FADE_START, 150.0f);

    gfx_option(GFX_SHADOW_FILTER_SIZE, 4.0f);
    gfx_option(GFX_SHADOW_OPTIMAL_ADJUST0, 3.0f);
    gfx_option(GFX_SHADOW_OPTIMAL_ADJUST1, 1.0f);
    gfx_option(GFX_SHADOW_OPTIMAL_ADJUST2, 1.0f);
    gfx_option(GFX_SHADOW_SPREAD_FACTOR0, 1.0f);

    gfx_option(GFX_SHADOW_SPREAD_FACTOR1, 1.0f);
    gfx_option(GFX_SHADOW_SPREAD_FACTOR2, 0.28f);
    gfx_option(GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD, 35.0f);
    gfx_option(GFX_SHADOW_PADDING, 0.8f);
    gfx_option(GFX_ANAGLYPH_LEFT_RED_MASK, 1.0f);

    gfx_option(GFX_ANAGLYPH_LEFT_GREEN_MASK, 0.0f);
    gfx_option(GFX_ANAGLYPH_LEFT_BLUE_MASK, 0.0f);
    gfx_option(GFX_ANAGLYPH_RIGHT_RED_MASK, 0.0f);
    gfx_option(GFX_ANAGLYPH_RIGHT_GREEN_MASK, 1.0f);
    gfx_option(GFX_ANAGLYPH_RIGHT_BLUE_MASK, 1.0f);

    gfx_option(GFX_ANAGLYPH_DESATURATION, 0.5f);
    gfx_option(GFX_BLOOM_THRESHOLD, 1.0f);

}

void gfx_option_init (void)
{
    for (unsigned i=0 ; i<sizeof(gfx_bool_options)/sizeof(gfx_bool_options[0]) ; ++i)
        valid_option(gfx_bool_options[i], truefalse);

    valid_option(GFX_FULLSCREEN_WIDTH, new ValidOptionRange<int>(1,10000));
    valid_option(GFX_FULLSCREEN_HEIGHT, new ValidOptionRange<int>(1,10000));
    int res_list[] = {512,1024,2048,4096};
    valid_option(GFX_SHADOW_RES, new ValidOptionList<int,int[4]>(res_list));
    int filter_taps_list[] = {1,4,9,16,36};
    valid_option(GFX_SHADOW_FILTER_TAPS, new ValidOptionList<int,int[5]>(filter_taps_list));
    valid_option(GFX_BLOOM_ITERATIONS, new ValidOptionRange<int>(0,255));
    valid_option(GFX_RAM, new ValidOptionRange<int>(0,16384));
    valid_option(GFX_DEBUG_MODE, new ValidOptionRange<int>(0,8));


    valid_option(GFX_FOV, new ValidOptionRange<float>(0.0000001f,179.0f));
    valid_option(GFX_NEAR_CLIP, new ValidOptionRange<float>(0.0000001f,10000.0f));
    valid_option(GFX_FAR_CLIP, new ValidOptionRange<float>(0.0000001f,10000.0f));
    valid_option(GFX_FIRST_PERSON_NEAR_CLIP, new ValidOptionRange<float>(0.0000001f,10000.0f));
    valid_option(GFX_FIRST_PERSON_FAR_CLIP, new ValidOptionRange<float>(0.0000001f,10000.0f));

    valid_option(GFX_EYE_SEPARATION, new ValidOptionRange<float>(0.0f,0.5f));
    valid_option(GFX_MONITOR_HEIGHT, new ValidOptionRange<float>(0.01f,1000.0f));
    valid_option(GFX_MONITOR_EYE_DISTANCE, new ValidOptionRange<float>(0.01f,1000.0f));
    valid_option(GFX_MIN_PERCEIVED_DEPTH, new ValidOptionRange<float>(0.01f,1000.0f));
    valid_option(GFX_MAX_PERCEIVED_DEPTH, new ValidOptionRange<float>(0.01f,1000.0f));

    valid_option(GFX_SHADOW_START, new ValidOptionRange<float>(0.0f,10000.0f));
    valid_option(GFX_SHADOW_END0, new ValidOptionRange<float>(0.0f,10000.0f));
    valid_option(GFX_SHADOW_END1, new ValidOptionRange<float>(0.0f,10000.0f));
    valid_option(GFX_SHADOW_END2, new ValidOptionRange<float>(0.0f,10000.0f));
    valid_option(GFX_SHADOW_FADE_START, new ValidOptionRange<float>(0.0f,10000.0f));

    valid_option(GFX_SHADOW_FILTER_SIZE, new ValidOptionRange<float>(0.0f,40.0f));
    valid_option(GFX_SHADOW_OPTIMAL_ADJUST0, new ValidOptionRange<float>(0.0f,10000.0f));
    valid_option(GFX_SHADOW_OPTIMAL_ADJUST1, new ValidOptionRange<float>(0.0f,10000.0f));
    valid_option(GFX_SHADOW_OPTIMAL_ADJUST2, new ValidOptionRange<float>(0.0f,10000.0f));
    valid_option(GFX_SHADOW_SPREAD_FACTOR0, new ValidOptionRange<float>(0.0f,20.0f));

    valid_option(GFX_SHADOW_SPREAD_FACTOR1, new ValidOptionRange<float>(0.0f,20.0f));
    valid_option(GFX_SHADOW_SPREAD_FACTOR2, new ValidOptionRange<float>(0.0f,20.0f));
    valid_option(GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD, new ValidOptionRange<float>(0.0f,10000.0f));
    valid_option(GFX_SHADOW_PADDING, new ValidOptionRange<float>(0.0f,100.0f));
    valid_option(GFX_ANAGLYPH_LEFT_RED_MASK, new ValidOptionRange<float>(0.0f,1.0f));

    valid_option(GFX_ANAGLYPH_LEFT_GREEN_MASK, new ValidOptionRange<float>(0.0f,1.0f));
    valid_option(GFX_ANAGLYPH_LEFT_BLUE_MASK, new ValidOptionRange<float>(0.0f,1.0f));
    valid_option(GFX_ANAGLYPH_RIGHT_RED_MASK, new ValidOptionRange<float>(0.0f,1.0f));
    valid_option(GFX_ANAGLYPH_RIGHT_GREEN_MASK, new ValidOptionRange<float>(0.0f,1.0f));
    valid_option(GFX_ANAGLYPH_RIGHT_BLUE_MASK, new ValidOptionRange<float>(0.0f,1.0f));

    valid_option(GFX_ANAGLYPH_DESATURATION, new ValidOptionRange<float>(0.0f,1.0f));
    valid_option(GFX_BLOOM_THRESHOLD, new ValidOptionRange<float>(0.0f,255.0f));

    gfx_option(GFX_AUTOUPDATE, false);
    gfx_option_reset();
    options_update(true);
    // This will trigger options_update(false) but it will be a no-op.
    gfx_option(GFX_AUTOUPDATE, true);
}

void gfx_option (GfxBoolOption o, bool v)
{
    valid_option_bool[o]->maybeThrow("Graphics", v);
    try {
        new_options_bool[o] = v;
        if (new_options_bool[GFX_AUTOUPDATE]) options_update(false);
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT("Couldn't set graphics option: "+e.getFullDescription());
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
        GRIT_EXCEPT("Couldn't set graphics option: "+e.getFullDescription());
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
        GRIT_EXCEPT("Couldn't set graphics option: "+e.getFullDescription());
    }
}
float gfx_option (GfxFloatOption o)
{
    return options_float[o];
}

