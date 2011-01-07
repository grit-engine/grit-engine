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

#include "gfx.h"

#include "lua_wrappers_gfx.h"
#include "lua_wrappers_primitives.h"
#include "lua_wrappers_scnmgr.h"
#include "lua_wrappers_mobj.h"

// GFXBODY ============================================================== {{{

void push_gfxbody (lua_State *L, const GfxBodyPtr &self)
{
        if (self.isNull())
                lua_pushnil(L);
        else
                push(L,new GfxBodyPtr(self),GFXBODY_TAG);
}

GC_MACRO(GfxBodyPtr,gfxbody,GFXBODY_TAG)

static int gfxbody_set_fade (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        float f = luaL_checknumber(L,2);
        if (lua_isnil(L,3)) {
                self->setFade(f, -1);
        } else {
                int t = check_int(L,3,0,2);
                self->setFade(f, t);
        }
        return 0;
TRY_END
}

static int gfxbody_get_paint_colour (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        int i = check_int(L,2,0,3);
        GfxPaintColour col = self->getPaintColour(i);
        lua_pushnumber(L, col.diff.r);
        lua_pushnumber(L, col.diff.g);
        lua_pushnumber(L, col.diff.b);
        lua_pushnumber(L, col.met);
        lua_pushnumber(L, col.spec.r);
        lua_pushnumber(L, col.spec.g);
        lua_pushnumber(L, col.spec.b);
        return 7;
TRY_END
}

static int gfxbody_set_paint_colour (lua_State *L)
{
TRY_START
        check_args(L,9);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        int i = check_int(L,2,0,3);
        GfxPaintColour col;
        col.diff.r = luaL_checknumber(L,3);
        col.diff.g = luaL_checknumber(L,4);
        col.diff.b = luaL_checknumber(L,5);
        col.met    = luaL_checknumber(L,6);
        col.spec.r = luaL_checknumber(L,7);
        col.spec.g = luaL_checknumber(L,8);
        col.spec.b = luaL_checknumber(L,9);
        self->setPaintColour(i, col);
        return 0;
TRY_END
}

static int gfxbody_get_bone_id (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        const char *name = luaL_checkstring(L,2);
        unsigned bone = self->getBoneId(name);
        lua_pushnumber(L,bone);
        return 1;
TRY_END
}

static int gfxbody_get_bone_name (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        const std::string &name = self->getBoneName(i);
        lua_pushstring(L,name.c_str());
        return 1;
TRY_END
}

static int gfxbody_get_bone_manually_controlled (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        bool v = self->getBoneManuallyControlled(i);
        lua_pushboolean(L,v);
        return 0;
TRY_END
}

static int gfxbody_set_bone_manually_controlled (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        bool v = check_bool(L,3);
        self->setBoneManuallyControlled(i, v);
        return 0;
TRY_END
}

static int gfxbody_set_all_bones_manually_controlled (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        bool v = check_bool(L,2);
        self->setAllBonesManuallyControlled(v);
        return 0;
TRY_END
}

static int gfxbody_get_bone_initial_position (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        push_v3(L, self->getBoneInitialPosition(i));
        return 1;
TRY_END
}

static int gfxbody_get_bone_world_position (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        push_v3(L, self->getBoneWorldPosition(i));
        return 1;
TRY_END
}

static int gfxbody_get_bone_local_position (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        push_v3(L, self->getBoneLocalPosition(i));
        return 1;
TRY_END
}

static int gfxbody_get_bone_initial_orientation (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        push_quat(L, self->getBoneInitialOrientation(i));
        return 1;
TRY_END
}

static int gfxbody_get_bone_world_orientation (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        push_quat(L, self->getBoneWorldOrientation(i));
        return 1;
TRY_END
}

static int gfxbody_get_bone_local_orientation (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        push_quat(L, self->getBoneLocalOrientation(i));
        return 1;
TRY_END
}

static int gfxbody_set_bone_local_position (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        Vector3 pos = check_v3(L,3);
        self->setBoneLocalPosition(i, pos);
        return 0;
TRY_END
}

static int gfxbody_set_bone_local_orientation (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        Quaternion quat = check_quat(L,3);
        self->setBoneLocalOrientation(i, quat);
        return 0;
TRY_END
}

static int gfxbody_set_bone_local_position_offset (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        Vector3 pos = check_v3(L,3);
        self->setBoneLocalPosition(i, self->getBoneInitialPosition(i)+pos);
        return 0;
TRY_END
}

static int gfxbody_set_bone_local_orientation_offset (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        unsigned i = check_t<unsigned>(L,2);
        Quaternion quat = check_quat(L,3);
        self->setBoneLocalOrientation(i, self->getBoneInitialOrientation(i)*quat);
        return 0;
TRY_END
}

static int gfxbody_make_child (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==1) {
                GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
                push_gfxbody(L, GfxBody::make(self));
        } else {
                check_args(L,2);
                GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
                const char *meshname = luaL_checkstring(L,2);
                push_gfxbody(L, GfxBody::make(meshname, self));
        }
        return 1;
TRY_END
}

static int gfxbody_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        self->destroy();
        return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxbody,GfxBodyPtr,GFXBODY_TAG)


static int gfxbody_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"localPosition")) {
                push_v3(L, self->getLocalPosition());
        } else if (!::strcmp(key,"worldPosition")) {
                push_v3(L, self->getWorldPosition());
        } else if (!::strcmp(key,"localOrientation")) {
                push_quat(L, self->getLocalOrientation());
        } else if (!::strcmp(key,"worldOrientation")) {
                push_quat(L, self->getWorldOrientation());
        } else if (!::strcmp(key,"localScale")) {
                push_v3(L, self->getLocalScale());
        } else if (!::strcmp(key,"worldScale")) {
                push_v3(L, self->getWorldScale());
        } else if (!::strcmp(key,"parent")) {
                push_gfxbody(L, self->getParent());
        } else if (!::strcmp(key,"batches")) {
                lua_pushnumber(L, self->getBatches());
        } else if (!::strcmp(key,"batchesWithChildren")) {
                lua_pushnumber(L, self->getBatchesWithChildren());
        } else if (!::strcmp(key,"nodeHACK")) {
                push_node(L, self->node);
        } else if (!::strcmp(key,"entHACK")) {
                push_entity(L, self->ent);

        } else if (!::strcmp(key,"fade")) {
                lua_pushnumber(L, self->getFade());
        } else if (!::strcmp(key,"setFade")) {
                push_cfunction(L,gfxbody_set_fade);
        } else if (!::strcmp(key,"castShadows")) {
                lua_pushboolean(L, self->getCastShadows());

        } else if (!::strcmp(key,"getPaintColour")) {
                push_cfunction(L,gfxbody_get_paint_colour);
        } else if (!::strcmp(key,"setPaintColour")) {
                push_cfunction(L,gfxbody_set_paint_colour);

        } else if (!::strcmp(key,"numBones")) {
                lua_pushnumber(L, self->getNumBones());
        } else if (!::strcmp(key,"getBoneId")) {
                push_cfunction(L,gfxbody_get_bone_id);
        } else if (!::strcmp(key,"getBoneName")) {
                push_cfunction(L,gfxbody_get_bone_name);

        } else if (!::strcmp(key,"getBoneManuallyControlled")) {
                push_cfunction(L,gfxbody_get_bone_manually_controlled);
        } else if (!::strcmp(key,"setBoneManuallyControlled")) {
                push_cfunction(L,gfxbody_set_bone_manually_controlled);
        } else if (!::strcmp(key,"setAllBonesManuallyControlled")) {
                push_cfunction(L,gfxbody_set_all_bones_manually_controlled);

        } else if (!::strcmp(key,"getBoneInitialPosition")) {
                push_cfunction(L,gfxbody_get_bone_initial_position);
        } else if (!::strcmp(key,"getBoneWorldPosition")) {
                push_cfunction(L,gfxbody_get_bone_world_position);
        } else if (!::strcmp(key,"getBoneLocalPosition")) {
                push_cfunction(L,gfxbody_get_bone_local_position);
        } else if (!::strcmp(key,"getBoneInitialOrientation")) {
                push_cfunction(L,gfxbody_get_bone_initial_orientation);
        } else if (!::strcmp(key,"getBoneWorldOrientation")) {
                push_cfunction(L,gfxbody_get_bone_world_orientation);
        } else if (!::strcmp(key,"getBoneLocalOrientation")) {
                push_cfunction(L,gfxbody_get_bone_local_orientation);

        } else if (!::strcmp(key,"setBoneLocalPosition")) {
                push_cfunction(L,gfxbody_set_bone_local_position);
        } else if (!::strcmp(key,"setBoneLocalOrientation")) {
                push_cfunction(L,gfxbody_set_bone_local_orientation);

        // 2 convenience functions
        } else if (!::strcmp(key,"setBoneLocalPositionOffset")) {
                push_cfunction(L,gfxbody_set_bone_local_position_offset);
        } else if (!::strcmp(key,"setBoneLocalOrientationOffset")) {
                push_cfunction(L,gfxbody_set_bone_local_orientation_offset);

        } else if (!::strcmp(key,"makeChild")) {
                push_cfunction(L,gfxbody_make_child);
        } else if (!::strcmp(key,"destroy")) {
                push_cfunction(L,gfxbody_destroy);
        } else {
                my_lua_error(L,"Not a readable GfxBody member: "+std::string(key));
        }
        return 1;
TRY_END
}


static int gfxbody_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"localPosition")) {
                Vector3 v = check_v3(L,3);
                self->setLocalPosition(v);
        } else if (!::strcmp(key,"localOrientation")) {
                Quaternion v = check_quat(L,3);
                self->setLocalOrientation(v);
        } else if (!::strcmp(key,"localScale")) {
                Vector3 v = check_v3(L,3);
                self->setLocalScale(v);
        } else if (!::strcmp(key,"parent")) {
                GET_UD_MACRO(GfxBodyPtr,par,3,GFXBODY_TAG);
                self->setParent(par);
        } else if (!::strcmp(key,"castShadows")) {
                bool v = check_bool(L,3);
                self->setCastShadows(v);
        } else {
               my_lua_error(L,"Not a writeable GfxBody member: "+std::string(key));
        }
        return 0;
TRY_END
}

EQ_MACRO(GfxBodyPtr,gfxbody,GFXBODY_TAG)

MT_MACRO_NEWINDEX(gfxbody);

//}}}


int global_gfx_render (lua_State *L)
{
TRY_START
        check_args(L,3);
        float elapsed = luaL_checknumber(L,1);
        Vector3 cam_pos = check_v3(L,2);
        Quaternion cam_dir = check_quat(L,3);
        gfx_render(elapsed, cam_pos, cam_dir);
        return 0;
TRY_END
}

int global_gfx_screenshot (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *filename = luaL_checkstring(L,1);
        gfx_screenshot(filename);
        return 0;
TRY_END
}

int global_gfx_option (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==2) {
                std::string opt = luaL_checkstring(L,1);
                int t;
                GfxBoolOption o0;
                GfxIntOption o1;
                GfxFloatOption o2;
                gfx_option_from_string(opt, t, o0, o1, o2);
                switch (t) {
                        case -1: my_lua_error(L,"Unrecognised graphics option: \""+opt+"\"");
                        case 0: gfx_option(o0, check_bool(L,2)); break;
                        case 1: gfx_option(o1, check_t<int>(L,2)); break;
                        case 2: gfx_option(o2, luaL_checknumber(L,2)); break;
                        default: my_lua_error(L,"Unrecognised type from gfx_option_from_string");
                }
                return 0;
        } else {
                check_args(L,1);
                std::string opt = luaL_checkstring(L,1);
                int t;
                GfxBoolOption o0;
                GfxIntOption o1;
                GfxFloatOption o2;
                gfx_option_from_string(opt, t, o0, o1, o2);
                switch (t) {
                        case -1: my_lua_error(L,"Unrecognised graphics option: \""+opt+"\"");
                        case 0: lua_pushboolean(L,gfx_option(o0)); break;
                        case 1: lua_pushnumber(L,gfx_option(o1)); break;
                        case 2: lua_pushnumber(L,gfx_option(o2)); break;
                        default: my_lua_error(L,"Unrecognised type from gfx_option_from_string");
                }
                return 1;
        }
TRY_END
}

int global_gfx_body_make (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==0) {
                push_gfxbody(L, GfxBody::make());
        } else {
                check_args(L,1);
                const char *meshname = luaL_checkstring(L,1);
                push_gfxbody(L, GfxBody::make(meshname));
        }
        return 1;
TRY_END
}

int global_gfx_sun_get_diffuse (lua_State *L)
{
TRY_START
        check_args(L,0);
        GfxRGB c = gfx_sun_get_diffuse();
        lua_pushnumber(L, c.r);
        lua_pushnumber(L, c.g);
        lua_pushnumber(L, c.b);
        return 3;
TRY_END
}

int global_gfx_sun_set_diffuse (lua_State *L)
{
TRY_START
        check_args(L,3);
        GfxRGB c;
        c.r = luaL_checknumber(L,1);
        c.g = luaL_checknumber(L,2);
        c.b = luaL_checknumber(L,3);
        gfx_sun_set_diffuse(c);
        return 0;
TRY_END
}

int global_gfx_sun_get_specular (lua_State *L)
{
TRY_START
        check_args(L,0);
        GfxRGB c = gfx_sun_get_specular();
        lua_pushnumber(L, c.r);
        lua_pushnumber(L, c.g);
        lua_pushnumber(L, c.b);
        return 3;
TRY_END
}

int global_gfx_sun_set_specular (lua_State *L)
{
TRY_START
        check_args(L,3);
        GfxRGB c;
        c.r = luaL_checknumber(L,1);
        c.g = luaL_checknumber(L,2);
        c.b = luaL_checknumber(L,3);
        gfx_sun_set_specular(c);
        return 0;
TRY_END
}

int global_gfx_sun_get_direction (lua_State *L)
{
TRY_START
        check_args(L,0);
        Vector3 d = gfx_sun_get_direction();
        push_v3(L, d);
        return 1;
TRY_END
}

int global_gfx_sun_set_direction (lua_State *L)
{
TRY_START
        check_args(L,1);
        Vector3 d = check_v3(L,1);
        gfx_sun_set_direction(d);
        return 0;
TRY_END
}


int global_gfx_get_ambient (lua_State *L)
{
TRY_START
        check_args(L,0);
        GfxRGB c = gfx_get_ambient();
        lua_pushnumber(L, c.r);
        lua_pushnumber(L, c.g);
        lua_pushnumber(L, c.b);
        return 3;
TRY_END
}

int global_gfx_set_ambient (lua_State *L)
{
TRY_START
        check_args(L,3);
        GfxRGB c;
        c.r = luaL_checknumber(L,1);
        c.g = luaL_checknumber(L,2);
        c.b = luaL_checknumber(L,3);
        gfx_set_ambient(c);
        return 0;
TRY_END
}


int global_gfx_fog_get_colour (lua_State *L)
{
TRY_START
        check_args(L,0);
        GfxRGB c = gfx_fog_get_colour();
        lua_pushnumber(L, c.r);
        lua_pushnumber(L, c.g);
        lua_pushnumber(L, c.b);
        return 3;
TRY_END
}

int global_gfx_fog_set_colour (lua_State *L)
{
TRY_START
        check_args(L,3);
        GfxRGB c;
        c.r = luaL_checknumber(L,1);
        c.g = luaL_checknumber(L,2);
        c.b = luaL_checknumber(L,3);
        gfx_fog_set_colour(c);
        return 0;
TRY_END
}

int global_gfx_fog_get_density (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L, gfx_fog_get_density());
        return 3;
TRY_END
}

int global_gfx_fog_set_density (lua_State *L)
{
TRY_START
        check_args(L,1);
        float d = luaL_checknumber(L,1);
        gfx_fog_set_density(d);
        return 0;
TRY_END
}

int global_gfx_get_celestial_orientation (lua_State *L)
{
TRY_START
        check_args(L,0);
        Quaternion d = gfx_get_celestial_orientation();
        push_quat(L, d);
        return 1;
TRY_END
}

int global_gfx_set_celestial_orientation (lua_State *L)
{
TRY_START
        check_args(L,1);
        Quaternion d = check_quat(L,1);
        gfx_set_celestial_orientation(d);
        return 0;
TRY_END
}




// vim: shiftwidth=8:tabstop=8:expandtab
