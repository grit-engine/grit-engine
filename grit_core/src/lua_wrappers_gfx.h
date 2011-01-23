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

#include "lua_wrappers_common.h"

#include "gfx.h"

#define GFXBODY_TAG "Grit/GfxBody"
MT_MACRO_DECLARE(gfxbody);
void push_gfxbody (lua_State *L, const GfxBodyPtr &self);

#define GFXLIGHT_TAG "Grit/GfxLight"
MT_MACRO_DECLARE(gfxlight);
void push_gfxlight (lua_State *L, const GfxLightPtr &self);

int global_gfx_render (lua_State *L);

int global_gfx_screenshot (lua_State *L);

int global_gfx_option (lua_State *L);

int global_gfx_body_make (lua_State *L);

int global_gfx_light_make (lua_State *L);


int global_gfx_sun_get_diffuse (lua_State *L);
int global_gfx_sun_set_diffuse (lua_State *L);

int global_gfx_sun_get_specular (lua_State *L);
int global_gfx_sun_set_specular (lua_State *L);

int global_gfx_sun_get_direction (lua_State *L);
int global_gfx_sun_set_direction (lua_State *L);

int global_gfx_get_scene_ambient (lua_State *L);
int global_gfx_set_scene_ambient (lua_State *L);

int global_gfx_fog_get_colour (lua_State *L);
int global_gfx_fog_set_colour (lua_State *L);

int global_gfx_fog_get_density (lua_State *L);
int global_gfx_fog_set_density (lua_State *L);

int global_gfx_get_celestial_orientation (lua_State *L);
int global_gfx_set_celestial_orientation (lua_State *L);

// vim: shiftwidth=8:tabstop=8:expandtab
