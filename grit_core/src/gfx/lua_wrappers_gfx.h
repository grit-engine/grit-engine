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

#include "../lua_wrappers_common.h"

#include "gfx.h"

#include "GfxBody.h"
#include "GfxSkyBody.h"
#include "GfxMaterial.h"
#include "GfxSkyMaterial.h"
#include "GfxLight.h"
#include "GfxInstances.h"
#include "GfxRangedInstances.h"
#include "gfx_hud.h"

#define GFXBODY_TAG "Grit/GfxBody"
#define GFXSKYBODY_TAG "Grit/GfxSkyBody"
#define GFXINSTANCES_TAG "Grit/GfxInstances"
#define GFXRANGEDINSTANCES_TAG "Grit/GfxRangedInstances"
#define GFXLIGHT_TAG "Grit/GfxLight"

#define GFXHUDOBJECT_TAG "Grit/GfxHudObject"
#define GFXHUDCLASS_TAG "Grit/GfxHudClass"

void gfx_lua_init (lua_State *L);

void push_gfxbody (lua_State *L, const GfxBodyPtr &self);
void push_gfxrangedinstances (lua_State *L, const GfxRangedInstancesPtr &self);
void push_gfxinstances (lua_State *L, const GfxInstancesPtr &self);
void push_gfxskybody (lua_State *L, const GfxSkyBodyPtr &self);
void push_gfxlight (lua_State *L, const GfxLightPtr &self);

void push_gfxhudclass (lua_State *L, GfxHudClass *self);
void push_gfxhudobj (lua_State *L, GfxHudObject *self);

// vim: shiftwidth=8:tabstop=8:expandtab
