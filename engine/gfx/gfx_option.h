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

#ifndef GFX_OPTION_H
#define GFX_OPTION_H

enum GfxBoolOption {
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
};

enum GfxIntOption {
    GFX_FULLSCREEN_WIDTH,
    GFX_FULLSCREEN_HEIGHT,
    GFX_SHADOW_RES,
    GFX_SHADOW_FILTER_TAPS,
    GFX_BLOOM_ITERATIONS,

    GFX_RAM,
    GFX_DEBUG_MODE,
};

enum GfxFloatOption {
    GFX_FOV,
    GFX_NEAR_CLIP,
    GFX_FAR_CLIP,
    GFX_FIRST_PERSON_NEAR_CLIP,
    GFX_FIRST_PERSON_FAR_CLIP,
    GFX_EYE_SEPARATION, // FOR 'real' 3d
    GFX_MONITOR_HEIGHT, // FOR 'real' 3d

    GFX_MONITOR_EYE_DISTANCE, // FOR 'real' 3d
    GFX_MIN_PERCEIVED_DEPTH, // FOR 'real' 3d -- distance from eyes of the front clip plane
    GFX_MAX_PERCEIVED_DEPTH, // FOR 'real' 3d -- distance from eyes of the rear clip plane
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

    GFX_BLOOM_THRESHOLD
};

std::string gfx_option_to_string (GfxBoolOption o);
std::string gfx_option_to_string (GfxIntOption o);
std::string gfx_option_to_string (GfxFloatOption o);

// set's t to either 0,1,2 and fills in the approriate argument
void gfx_option_from_string (const std::string &s,
                             int &t,
                             GfxBoolOption &o0,
                             GfxIntOption &o1,
                             GfxFloatOption &o2);

void gfx_option (GfxBoolOption o, bool v);
bool gfx_option (GfxBoolOption o);
void gfx_option (GfxIntOption o, int v);
int gfx_option (GfxIntOption o);
void gfx_option (GfxFloatOption o, float v);
float gfx_option (GfxFloatOption o);

void gfx_option_init (void);
void gfx_option_reset (void);


#endif

