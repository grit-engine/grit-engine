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
#include "ExternalTable.h"
#include "path_util.h"

// GFXBODY ============================================================== {{{

void push_gfxbody (lua_State *L, const GfxBodyPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxBodyPtr(self),GFXBODY_TAG);
}

GC_MACRO(GfxBodyPtr,gfxbody,GFXBODY_TAG)

static int gfxbody_get_paint_colour (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    int i = check_int(L,2,0,3);
    GfxPaintColour col = self->getPaintColour(i);
    push_v3(L, col.diff);
    lua_pushnumber(L, col.met);
    push_v3(L, col.spec);
    return 3;
TRY_END
}

static int gfxbody_set_paint_colour (lua_State *L)
{
TRY_START
    check_args(L,5);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    int i = check_int(L,2,0,3);
    GfxPaintColour col;
    col.diff = check_v3(L,3);
    col.met    = luaL_checknumber(L,4);
    col.spec = check_v3(L,5);
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
    } else if (!::strcmp(key,"triangles")) {
        lua_pushnumber(L, self->getTriangles());
    } else if (!::strcmp(key,"trianglesWithChildren")) {
        lua_pushnumber(L, self->getTrianglesWithChildren());
    } else if (!::strcmp(key,"nodeHACK")) {
        push_node(L, self->node);
    } else if (!::strcmp(key,"entHACK")) {
        if (self->ent)
            push_entity(L, self->ent);
        else
            lua_pushnil(L);
    } else if (!::strcmp(key,"entEmissiveHACK")) {
        if (self->entEmissive)
            push_entity(L, self->entEmissive);
        else
            lua_pushnil(L);

    } else if (!::strcmp(key,"fade")) {
        lua_pushnumber(L, self->getFade());
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
    } else if (!::strcmp(key,"fade")) {
        float v = check_float(L,3);
        self->setFade(v);
    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self->setParent(GfxBodyPtr(NULL));
        } else {
            GET_UD_MACRO(GfxBodyPtr,par,3,GFXBODY_TAG);
            self->setParent(par);
        }
    } else if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else {
           my_lua_error(L,"Not a writeable GfxBody member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxBodyPtr,gfxbody,GFXBODY_TAG)

MT_MACRO_NEWINDEX(gfxbody);

//}}}


// GFXLIGHT ============================================================== {{{

void push_gfxlight (lua_State *L, const GfxLightPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxLightPtr(self),GFXLIGHT_TAG);
}

GC_MACRO(GfxLightPtr,gfxlight,GFXLIGHT_TAG)

static int gfxlight_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxLightPtr,self,1,GFXLIGHT_TAG);
    self->destroy();
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxlight,GfxLightPtr,GFXLIGHT_TAG)


static int gfxlight_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxLightPtr,self,1,GFXLIGHT_TAG);
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
    } else if (!::strcmp(key,"diffuseColour")) {
        push_v3(L, self->getDiffuseColour());
    } else if (!::strcmp(key,"specularColour")) {
        push_v3(L, self->getSpecularColour());
    } else if (!::strcmp(key,"coronaSize")) {
        lua_pushnumber(L, self->getCoronaSize());
    } else if (!::strcmp(key,"coronaLocalPosition")) {
        push_v3(L, self->getCoronaLocalPosition());
    } else if (!::strcmp(key,"coronaColour")) {
        push_v3(L, self->getCoronaColour());
    } else if (!::strcmp(key,"aim")) {
        push_quat(L, self->getAim());
    } else if (!::strcmp(key,"range")) {
        lua_pushnumber(L, self->getRange());
    } else if (!::strcmp(key,"fade")) {
        lua_pushnumber(L, self->getFade());
    } else if (!::strcmp(key,"innerAngle")) {
        lua_pushnumber(L, self->getInnerAngle().inDegrees());
    } else if (!::strcmp(key,"outerAngle")) {
        lua_pushnumber(L, self->getOuterAngle().inDegrees());
    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());
    } else if (!::strcmp(key,"parent")) {
        push_gfxbody(L, self->getParent());
    } else if (!::strcmp(key,"nodeHACK")) {
        push_node(L, self->node);
    } else if (!::strcmp(key,"lightHACK")) {
        push_light(L, self->light);

    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxlight_destroy);
    } else {
        my_lua_error(L,"Not a readable GfxLight member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxlight_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxLightPtr,self,1,GFXLIGHT_TAG);
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
    } else if (!::strcmp(key,"diffuseColour")) {
        Vector3 v = check_v3(L,3);
        self->setDiffuseColour(v);
    } else if (!::strcmp(key,"specularColour")) {
        Vector3 v = check_v3(L,3);
        self->setSpecularColour(v);
    } else if (!::strcmp(key,"coronaSize")) {
        float v = luaL_checknumber(L,3);
        self->setCoronaSize(v);
    } else if (!::strcmp(key,"coronaLocalPosition")) {
        Vector3 v = check_v3(L,3);
        self->setCoronaLocalPosition(v);
    } else if (!::strcmp(key,"coronaColour")) {
        Vector3 v = check_v3(L,3);
        self->setCoronaColour(v);
    } else if (!::strcmp(key,"aim")) {
        Quaternion v = check_quat(L,3);
        self->setAim(v);
    } else if (!::strcmp(key,"fade")) {
        float v = check_float(L,3);
        self->setFade(v);
    } else if (!::strcmp(key,"range")) {
        float v = check_float(L,3);
        self->setRange(v);
    } else if (!::strcmp(key,"innerAngle")) {
        float v = check_float(L,3);
        self->setInnerAngle(Degree(v));
    } else if (!::strcmp(key,"outerAngle")) {
        float v = check_float(L,3);
        self->setOuterAngle(Degree(v));
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self->setParent(GfxBodyPtr(NULL));
        } else {
            GET_UD_MACRO(GfxBodyPtr,par,3,GFXBODY_TAG);
            self->setParent(par);
        }
    } else {
           my_lua_error(L,"Not a writeable GfxLight member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxLightPtr,gfxlight,GFXLIGHT_TAG)

MT_MACRO_NEWINDEX(gfxlight);

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

int global_gfx_light_make (lua_State *L)
{
TRY_START
    check_args(L,0);
    push_gfxlight(L, GfxLight::make());
    return 1;
TRY_END
}

int global_gfx_sun_get_diffuse (lua_State *L)
{
TRY_START
    check_args(L,0);
    push_v3(L, gfx_sun_get_diffuse());
    return 1;
TRY_END
}

int global_gfx_sun_set_diffuse (lua_State *L)
{
TRY_START
    check_args(L,1);
    Vector3 c = check_v3(L,1);
    gfx_sun_set_diffuse(c);
    return 0;
TRY_END
}

int global_gfx_sun_get_specular (lua_State *L)
{
TRY_START
    check_args(L,0);
    push_v3(L, gfx_sun_get_specular());
    return 1;
TRY_END
}

int global_gfx_sun_set_specular (lua_State *L)
{
TRY_START
    check_args(L,1);
    Vector3 c = check_v3(L,1);
    gfx_sun_set_specular(c);
    return 0;
TRY_END
}

int global_gfx_sun_get_direction (lua_State *L)
{
TRY_START
    check_args(L,0);
    push_v3(L, gfx_sun_get_direction());
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


int global_gfx_get_scene_ambient (lua_State *L)
{
TRY_START
    check_args(L,0);
    push_v3(L, gfx_get_scene_ambient());
    return 1;
TRY_END
}

int global_gfx_set_scene_ambient (lua_State *L)
{
TRY_START
    check_args(L,1);
    gfx_set_scene_ambient(check_v3(L,1));
    return 0;
TRY_END
}


int global_gfx_fog_get_colour (lua_State *L)
{
TRY_START
    check_args(L,0);
    push_v3(L, gfx_fog_get_colour());
    return 1;
TRY_END
}

int global_gfx_fog_set_colour (lua_State *L)
{
TRY_START
    check_args(L,1);
    gfx_fog_set_colour(check_v3(L,1));
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


template<class T> void vect_remove_fast (std::vector<T> &vect, size_t index)
{
        std::swap<T>(vect[index],vect[vect.size()-1]);
        vect.pop_back();
}

namespace {
    struct UVRect { float u1,v1, u2,v2; };
    struct ParticleDefinition;

    struct ParticleDefinition {
        ParticleDefinition (const std::string &m, const std::vector<UVRect> fs,
                    const ExternalTable &t, int b)
              : material(m), frames(fs), table(t), behaviour(b)
        {
        }
        void destroy (lua_State *L);
                    
        std::string material;
        std::vector<UVRect> frames;
        ExternalTable table;
        int behaviour; // pointer to lua function
    };

    std::map<std::string, ParticleDefinition*> particle_defs;

    struct LuaParticle {

        int state; // pointer to lua table
        GfxParticle p;
        ParticleDefinition *pd;

        LuaParticle (lua_State *L, const GfxParticle &p_, ParticleDefinition *pd_)
             : p(p_), pd(pd_)
        {
            state = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        ~LuaParticle (void)
        {
            APP_ASSERT(state==LUA_NOREF);
        }

        void destroy (lua_State *L)
        {
            luaL_unref(L,LUA_REGISTRYINDEX,state);
            state = LUA_NOREF;
            p.release();
        }

        bool updateGraphics (lua_State *L, float elapsed, int error_handler)
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, state);
            // stack: err tab

            bool destroy = false;

            lua_getfield(L, -1, "behaviour");
            // stack: err tab func
            lua_pushvalue(L,-2);
            // stack: err tab func tab
            lua_pushnumber(L,elapsed);
            // stack: err tab func tab elapsed
            int status = lua_pcall(L,2,1,error_handler);
            if (status) {
                // stack: err,tab,msg
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                //stack: err,tab
                destroy = true;
            } else {
                // stack: err, tab, bool
                if (lua_isboolean(L,-1) && !lua_toboolean(L,-1)) destroy = true;
                lua_pop(L,1); // destroy code
                // stack: err, tab
            }
            // stack: err, tab

            if (!destroy) {
                // handle stuff from tab
                lua_getfield(L, -1, "position");
                if (!lua_isvector3(L,-1)) {
                    CERR << "Particle position was not a vector3." << std::endl;
                    destroy = true;
                } else {
                    p.setPosition(check_v3(L,-1));
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "width");
                if (lua_isnil(L,-1)) {
                    p.setWidth(1);
                } else if (!lua_isnumber(L,-1)) {
                    CERR << "Particle width was not a number." << std::endl;
                    destroy = true;
                } else {
                    p.setWidth(luaL_checknumber(L,-1));
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "height");
                if (lua_isnil(L,-1)) {
                    p.setHeight(1);
                } else if (!lua_isnumber(L,-1)) {
                    CERR << "Particle height was not a number." << std::endl;
                    destroy = true;
                } else {
                    p.setHeight(luaL_checknumber(L,-1));
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "depth");
                if (lua_isnil(L,-1)) {
                    p.setDepth(1);
                } else if (!lua_isnumber(L,-1)) {
                    CERR << "Particle depth was not a number." << std::endl;
                    destroy = true;
                } else {
                    p.setDepth(luaL_checknumber(L,-1));
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "colour");
                if (lua_isnil(L,-1)) {
                    p.setAmbient(Vector3(1,1,1));
                } else if (!lua_isvector3(L,-1)) {
                    CERR << "Particle colour was not a vector3." << std::endl;
                    destroy = true;
                } else {
                    p.setAmbient(check_v3(L,-1));
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "alpha");
                if (lua_isnil(L,-1)) {
                    p.setAlpha(1);
                } else if (!lua_isnumber(L,-1)) {
                    CERR << "Particle alpha was not a number." << std::endl;
                    destroy = true;
                } else {
                    p.setAlpha(luaL_checknumber(L,-1));
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "angle");
                if (lua_isnil(L,-1)) {
                    p.setAngle(0);
                } else if (!lua_isnumber(L,-1)) {
                    CERR << "Particle angle was not a number." << std::endl;
                    destroy = true;
                } else {
                    p.setAngle(luaL_checknumber(L,-1));
                }
                lua_pop(L,1);

                bool has_frame = false;
                lua_getfield(L, -1, "frame");
                if (lua_isnil(L,-1) || pd->frames.size()==0) {
                } else if (!lua_isnumber(L,-1)) {
                    has_frame = true;
                    CERR << "Particle frame was not a number." << std::endl;
                    destroy = true;
                } else {
                    has_frame = true;
                    float frame_ = lua_tonumber(L,-1);
                    unsigned frame = unsigned(frame_);
                    UVRect &uvr = pd->frames[frame % pd->frames.size()];
                    p.setUV(uvr.u1, uvr.v1, uvr.u2, uvr.v2);
                }
                lua_pop(L,1);

                bool has_uvs = false;
                lua_getfield(L, -1, "uvs");
                if (has_frame || lua_isnil(L,-1)) {
                } else if (!lua_istable(L,-1)) {
                    CERR << "Particle uvs was not a table." << std::endl;
                    destroy = true;
                } else if (!lua_objlen(L,-1) != 4) {
                    CERR << "Particle uvs did not have 4 elements." << std::endl;
                    destroy = true;
                } else {
                    has_uvs = true;
                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1))
                        CERR << "Texture ordinate was not a number." << std::endl;
                    float u1 = lua_tonumber(L,-1);
                    lua_pop(L,1);

                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1))
                        CERR << "Texture ordinate was not a number." << std::endl;
                    float v1 = lua_tonumber(L,-1);
                    lua_pop(L,1);


                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1))
                        CERR << "Texture ordinate was not a number." << std::endl;
                    float u2 = lua_tonumber(L,-1);
                    lua_pop(L,1);

                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1))
                        CERR << "Texture ordinate was not a number." << std::endl;
                    float v2 = lua_tonumber(L,-1);
                    lua_pop(L,1);

                    p.setUV(u1, v1, u2, v2);
                }
                lua_pop(L,1);
                if (!has_frame && !has_uvs) {
                    p.setDefaultUV();
                }
                // stack: err, tab
            }
            lua_pop(L,1);

            // stack: err
            return destroy;
        }
    };

    std::vector<LuaParticle*> particles;

    void ParticleDefinition::destroy (lua_State *L)
    {
        luaL_unref(L,LUA_REGISTRYINDEX,behaviour);
        // remove any particles that used to belong to this definition
        for (unsigned i=0 ; i<particles.size() ; ++i) {
            LuaParticle *p = particles[i];
            if (p->pd == this) {
                p->destroy(L);
                delete p;
                vect_remove_fast(particles, i);
                --i;
            }
        }
    }

}

int global_gfx_particle_define (lua_State *L)
{
TRY_START
    check_args(L,2);
    std::string name = luaL_checkstring(L,1);
    name = pwd_full(L, name);
    if (!lua_istable(L,2)) my_lua_error(L,"Parameter 2 must be a table.");

    lua_getfield(L, 2, "map");
    if (!lua_isstring(L,-1)) my_lua_error(L,"Particle map must be a string.");
    std::string texture = lua_tostring(L,-1);
    texture = pwd_full(L, texture);
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "map");

    lua_getfield(L, 2, "blending");
    GfxParticleBlend blend;
    if (lua_isnil(L,-1)) {
        blend = GFX_PARTICLE_OPAQUE;
    } else if (!lua_isstring(L,-1)) {
        my_lua_error(L,"Particle blending must be a string.");
    } else {
        std::string blending = lua_tostring(L,-1);
        if (blending=="OPAQUE") {
            blend = GFX_PARTICLE_OPAQUE;
        } else if (blending=="ALPHA") {
            blend = GFX_PARTICLE_ALPHA;
        } else if (blending=="ADD") {
            blend = GFX_PARTICLE_ADD;
        } else {
            my_lua_error(L, "Particle blending must be OPAQUE / ALPHA / ADD.");
            blend = GFX_PARTICLE_OPAQUE; // to shut up the compiler
        }
    }
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "blending");

    float alpha_rej;
    lua_getfield(L, 2, "alphaReject");
    if (lua_isnil(L,-1)) {
        alpha_rej = blend==GFX_PARTICLE_OPAQUE ? 127 : 0;
    } else if (!lua_isnumber(L,-1)) {
        my_lua_error(L,"Particle alphaReject must be a number.");
    } else {
        alpha_rej = (float)lua_tonumber(L,-1);
    }
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "alphaReject");

    bool emissive;
    lua_getfield(L, 2, "emissive");
    if (lua_isnil(L,-1)) {
        emissive = false;
    } else if (!lua_isboolean(L,-1)) {
        my_lua_error(L,"Particle emissive must be a boolean.");
    } else {
        emissive = (bool)lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "emissive");

    lua_getfield(L, 2, "frames");
    std::vector<UVRect> frames;
    if (lua_isnil(L,-1)) {
    } else if (!lua_istable(L,-1)) {
        my_lua_error(L,"Particle frames must be an array.");
    } else {
        unsigned nums = lua_objlen(L,-1);
        if (nums%4 != 0) my_lua_error(L,"Number of texcoords should be a multiple of 4.");
        frames.resize(nums/4);
        for (unsigned i=0 ; i<nums/4 ; ++i) {
            UVRect &uvrect = frames[i];

            lua_rawgeti(L,-1,4*i+1);
            if (!lua_isnumber(L,-1)) my_lua_error(L, "Texcoord must be a number");
            uvrect.u1 = 0.5+lua_tonumber(L,-1);
            lua_pop(L,1);

            lua_rawgeti(L,-1,4*i+2);
            if (!lua_isnumber(L,-1)) my_lua_error(L, "Texcoord must be a number");
            uvrect.v1 = 0.5+lua_tonumber(L,-1);
            lua_pop(L,1);

            lua_rawgeti(L,-1,4*i+3);
            if (!lua_isnumber(L,-1)) my_lua_error(L, "Texcoord must be a number");
            uvrect.u2 = 0.5+lua_tonumber(L,-1);
            lua_pop(L,1);

            lua_rawgeti(L,-1,4*i+4);
            if (!lua_isnumber(L,-1)) my_lua_error(L, "Texcoord must be a number");
            uvrect.v2 = 0.5+lua_tonumber(L,-1);
            lua_pop(L,1);

            uvrect.u2 += uvrect.u1 - 1;
            uvrect.v2 += uvrect.v1 - 1;

        }
    }
    lua_pop(L,1); // pop frames array
    lua_pushnil(L);
    lua_setfield(L, 2, "frames");

    lua_getfield(L, 2, "behaviour");
    if (!lua_isfunction(L,-1)) my_lua_error(L,"Particle behaviour must be a function.");
    int behaviour = luaL_ref(L,LUA_REGISTRYINDEX);
    lua_pushnil(L);
    lua_setfield(L, 2, "behaviour");
    
    ExternalTable table;
    table.takeTableFromLuaStack(L,2);

    ParticleDefinition *&pd = particle_defs[name];
    if (pd != NULL) {
        pd->destroy(L);
        delete pd;
    }
    gfx_particle_define(name, texture, blend, alpha_rej, emissive); // will invalidate 
    ParticleDefinition *newpd = new ParticleDefinition(name, frames, table, behaviour);
    pd = newpd;
    return 0;
TRY_END
}

namespace {
}

int global_gfx_particle_emit (lua_State *L)
{
TRY_START
    check_args(L,4);
    std::string name = pwd_full(L, luaL_checkstring(L,1));
    Vector3 pos = check_v3(L,2);
    Vector3 vel = check_v3(L,3);
    if (!lua_istable(L,4)) my_lua_error(L,"Parameter 4 must be a table.");

    ParticleDefinition *pd = particle_defs[name];

    if (pd==NULL) {
        my_lua_error(L, "No such particle \""+name+"\"");
    }

    pd->table.dump(L);
    // stack: particle

    push_v3(L, pos);
    lua_setfield(L,-2,"position");
    push_v3(L, vel);
    lua_setfield(L,-2,"velocity");
    lua_rawgeti(L,LUA_REGISTRYINDEX,pd->behaviour);
    lua_setfield(L,-2,"behaviour");

    // stack: particle
    for (lua_pushnil(L) ; lua_next(L,4)!=0 ; lua_pop(L,1)) {
        // stack: particle, key, val
        lua_pushvalue(L,-2);
        // stack: particle, key, val, key
        lua_pushvalue(L,-2);
        // stack: particle, key, val, key, val
        lua_settable(L,-5);
        // stack: particle, key, val
    }
    // stack: particle

    
    LuaParticle *lp = new LuaParticle(L, gfx_particle_emit(pd->material), pd);
    particles.push_back(lp);

    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);
    // stack: eh

    bool destroy = lp->updateGraphics(L, 0, error_handler);
    if (destroy) {
        // stack: eh,
        particles.pop_back();
        lp->destroy(L);
        delete lp;
    }
    // stack: eh

    lua_pop(L,1);
    // stack:

    return 0;
TRY_END
}

int global_gfx_particle_pump (lua_State *L)
{
TRY_START
    check_args(L,1);
    float elapsed = check_float(L,1);

    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);
    // stack: err

    for (size_t i=0 ; i<particles.size() ; ++i) {
        LuaParticle *lp = particles[i];
        bool destroy = lp->updateGraphics(L, elapsed, error_handler);

        if (destroy) {
            // stack: err
            APP_ASSERT(particles[i]==lp);
            vect_remove_fast(particles, i);
            lp->destroy(L);
            delete lp;
            --i;
            continue;
        }
        // stack: err
    }
    lua_pop(L,1);

    return 0;
TRY_END
}

int global_gfx_reload_mesh (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    gfx_reload_mesh(name);
    return 0;
TRY_END
}

int global_gfx_reload_texture (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    gfx_reload_texture(name);
    return 0;
TRY_END
}
// vim: shiftwidth=4:tabstop=4:expandtab
