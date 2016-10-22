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

#include "../grit_lua_util.h"

#include "gfx.h"

#include "gfx_body.h"
#include "gfx_decal.h"
#include "gfx_fertile_node.h"
#include "gfx_hud.h"
#include "gfx_instances.h"
#include "gfx_light.h"
#include "gfx_material.h"
#include "gfx_ranged_instances.h"
#include "gfx_sky_body.h"
#include "gfx_sky_material.h"


void gfx_lua_init (lua_State *L);


#define GFXBODY_TAG "Grit/GfxBody"
void push_gfxbody (lua_State *L, const GfxBodyPtr &self);

#define GFXDECAL_TAG "Grit/GfxDecal"
void push_gfxdecal (lua_State *L, const GfxDecalPtr &self);

#define GFXINSTANCES_TAG "Grit/GfxInstances"
void push_gfxinstances (lua_State *L, const GfxInstancesPtr &self);

#define GFXLIGHT_TAG "Grit/GfxLight"
void push_gfxlight (lua_State *L, const GfxLightPtr &self);

#define GFXNODE_TAG "Grit/GfxFertileNode"
void push_gfxnode (lua_State *L, const GfxNodePtr &self);

#define GFXRANGEDINSTANCES_TAG "Grit/GfxRangedInstances"
void push_gfxrangedinstances (lua_State *L, const GfxRangedInstancesPtr &self);

#define GFXSKYBODY_TAG "Grit/GfxSkyBody"
void push_gfxskybody (lua_State *L, const GfxSkyBodyPtr &self);


#define GFXHUDCLASS_TAG "Grit/GfxHudClass"
void push_gfxhudclass (lua_State *L, GfxHudClass *self);

#define GFXHUDOBJECT_TAG "Grit/GfxHudObject"
void push_gfxhudobj (lua_State *L, GfxHudObject *self);

#define GFXHUDTEXT_TAG "Grit/GfxHudText"
void push_gfxhudtext (lua_State *L, GfxHudText *self);

// vim: shiftwidth=8:tabstop=8:expandtab
