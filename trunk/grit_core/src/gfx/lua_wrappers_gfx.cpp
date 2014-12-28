/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include <colour_conversion.h>
#include <unicode_util.h>

#include "../grit_lua_util.h"

#include "../lua_wrappers_primitives.h"
#include "../main.h"
#include "../external_table.h"
#include "../lua_ptr.h"
#include "../path_util.h"

#include "lua_wrappers_gfx.h"
#include "gfx_option.h"
#include "gfx_hud.h"
#include "gfx_font.h"

GfxNodePtr check_gfx_node (lua_State *L, int idx)
{
    if (has_tag(L, idx, GFXNODE_TAG)) {
        GET_UD_MACRO(GfxNodePtr,self,idx,GFXNODE_TAG);
        return self;
    } else if (has_tag(L, idx, GFXBODY_TAG)) {
        GET_UD_MACRO(GfxBodyPtr,self,idx,GFXBODY_TAG);
        return self.staticCast<GfxFertileNode>();
    } else {
        std::stringstream ss;
        ss << "Expected a GfxFertileNode or GfxBody at position " << idx;
        my_lua_error(L, ss.str());
    }
}

void push_gfx_node_concrete (lua_State *L, const GfxNodePtr &np)
{
    if (np.isNull()) {
        lua_pushnil(L);
    } else if (np->hasGraphics()) {
        push_gfxbody(L, np.staticCast<GfxBody>());
    } else {
        push_gfxnode(L, np);
    }
}

// GFXNODE ============================================================== {{{

void push_gfxnode (lua_State *L, const GfxNodePtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxNodePtr(self),GFXNODE_TAG);
}

GC_MACRO(GfxNodePtr,gfxnode,GFXNODE_TAG)

static int gfxnode_make_child (lua_State *L)
{
TRY_START
    if (lua_gettop(L)==1) {
        GET_UD_MACRO(GfxNodePtr,self,1,GFXNODE_TAG);
        push_gfxnode(L, GfxFertileNode::make(self));
    } else {
        check_args(L,2);
        GET_UD_MACRO(GfxNodePtr,self,1,GFXNODE_TAG);
        std::string mesh_name = check_path(L,2);
        push_gfxbody(L, GfxBody::make(mesh_name, gfx_empty_string_map, self));
    }
    return 1;
TRY_END
}

static int gfxnode_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxNodePtr,self,1,GFXNODE_TAG);
    self->destroy();
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxnode,GfxNodePtr,GFXNODE_TAG)


static int gfxnode_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxNodePtr,self,1,GFXNODE_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"localPosition")) {
        push_v3(L, self->getLocalPosition());
    } else if (!::strcmp(key,"localOrientation")) {
        push_quat(L, self->getLocalOrientation());
    } else if (!::strcmp(key,"localScale")) {
        push_v3(L, self->getLocalScale());
    } else if (!::strcmp(key,"parent")) {
        push_gfx_node_concrete(L, self->getParent());
    } else if (!::strcmp(key,"parentBone")) {
        if (self->getParentBoneName().length() == 0) {
            lua_pushnil(L);
        } else {
            push_string(L, self->getParentBoneName());
        }
    } else if (!::strcmp(key,"batchesWithChildren")) {
        lua_pushnumber(L, self->getBatchesWithChildren());
    } else if (!::strcmp(key,"trianglesWithChildren")) {
        lua_pushnumber(L, self->getTrianglesWithChildren());
    } else if (!::strcmp(key,"vertexesWithChildren")) {
        lua_pushnumber(L, self->getVertexesWithChildren());
    } else if (!::strcmp(key,"fade")) {
        lua_pushnumber(L, 1);
    } else if (!::strcmp(key,"makeChild")) {
        push_cfunction(L,gfxnode_make_child);
    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxnode_destroy);
    } else {
        my_lua_error(L,"Not a readable GfxFertileNode member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxnode_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxNodePtr,self,1,GFXNODE_TAG);
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
        if (lua_isnil(L,3)) {
            self->setParent(GfxNodePtr(NULL));
        } else {
            GfxNodePtr par = check_gfx_node(L, 3);
            self->setParent(par);
        }
    } else if (!::strcmp(key,"parentBone")) {
        if (lua_isnil(L,3)) {
            self->setParentBoneName("");
        } else {
            std::string s = check_string(L, 3);
            self->setParentBoneName(s);
        }
    } else if (!::strcmp(key,"fade")) {
        float v = check_float(L,3);
        (void) v;
    } else {
       my_lua_error(L,"Not a writeable GfxFertileNode member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxNodePtr,gfxnode,GFXNODE_TAG)

MT_MACRO_NEWINDEX(gfxnode);

//}}}


// GFXBODY ============================================================== {{{

void push_gfxbody (lua_State *L, const GfxBodyPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxBodyPtr(self),GFXBODY_TAG);
}

GC_MACRO(GfxBodyPtr,gfxbody,GFXBODY_TAG)

static int gfxbody_reinitialise (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    self->reinitialise();
    return 0;
TRY_END
}

static int gfxbody_get_emissive_enabled (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const std::string &n = luaL_checkstring(L,2);
    unsigned n2 = self->getSubMeshByOriginalMaterialName(n);
    lua_pushboolean(L, self->getEmissiveEnabled(n2));
    return 1;
TRY_END
}

static int gfxbody_set_emissive_enabled (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const std::string &n = luaL_checkstring(L,2);
    unsigned n2 = self->getSubMeshByOriginalMaterialName(n);
    bool v = check_bool(L,3);
    self->setEmissiveEnabled(n2,v);
    return 0;
TRY_END
}

static int gfxbody_get_materials (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    for (unsigned i=0 ; i<self->getNumSubMeshes() ; ++i) {
        GfxMaterial *m = self->getMaterial(i);
        lua_pushstring(L, m->name.c_str());
    }
    return self->getNumSubMeshes() ;
TRY_END
}

static int gfxbody_set_material (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const std::string &n = luaL_checkstring(L,2);
    unsigned n2 = self->getSubMeshByOriginalMaterialName(n);
    const char *mname = luaL_checkstring(L,3);
    GfxMaterial *m = gfx_material_get(mname);
    self->setMaterial(n2,m);
    return 0;
TRY_END
}

static int gfxbody_set_all_materials (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const char *mname = luaL_checkstring(L,2);
    GfxMaterial *m = gfx_material_get(mname);
    for (unsigned i=0 ; i<self->getBatches() ; ++i) {
        self->setMaterial(i,m);
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
    col.met    = check_float(L,4);
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
    return 1;
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

static int gfxbody_get_all_animations(lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::vector<std::string> names = self->getAnimationNames();
    for (unsigned i=0 ; i<names.size() ; ++i) {
        lua_pushstring(L, names[i].c_str());
    }
    return names.size();
TRY_END
}

static int gfxbody_get_animation_length(lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    lua_pushnumber(L, self->getAnimationLength(anim));
    return 1;
TRY_END
}

static int gfxbody_get_animation_pos(lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    lua_pushnumber(L, self->getAnimationPos(anim));
    return 1;
TRY_END
}

static int gfxbody_set_animation_pos(lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    float const t = check_float(L,3);
    self->setAnimationPos(anim, t);
    return 0;
TRY_END
}

static int gfxbody_get_animation_pos_normalised(lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    lua_pushnumber(L, self->getAnimationPos(anim) / self->getAnimationLength(anim));
    return 1;
TRY_END
}

static int gfxbody_set_animation_pos_normalised(lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    float const t = check_float(L,3);
    self->setAnimationPos(anim, t * self->getAnimationLength(anim));
    return 0;
TRY_END
}

static int gfxbody_get_animation_mask(lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    lua_pushnumber(L, self->getAnimationMask(anim));
    return 1;
TRY_END
}

static int gfxbody_set_animation_mask(lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    float const t = check_float(L,3);
    self->setAnimationMask(anim, t);
    return 0;
TRY_END
}

static int gfxbody_make_child (lua_State *L)
{
TRY_START
    if (lua_gettop(L)==1) {
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        push_gfxnode(L, GfxFertileNode::make(self.staticCast<GfxFertileNode>()));
    } else {
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        std::string mesh_name = check_path(L,2);
        push_gfxbody(L, GfxBody::make(mesh_name, gfx_empty_string_map, self.staticCast<GfxFertileNode>()));
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
    } else if (!::strcmp(key,"localOrientation")) {
        push_quat(L, self->getLocalOrientation());
    } else if (!::strcmp(key,"localScale")) {
        push_v3(L, self->getLocalScale());
    } else if (!::strcmp(key,"parent")) {
        push_gfx_node_concrete(L, self->getParent());
    } else if (!::strcmp(key,"parentBone")) {
        if (self->getParentBoneName().length() == 0) {
            lua_pushnil(L);
        } else {
            push_string(L, self->getParentBoneName());
        }
    } else if (!::strcmp(key,"batches")) {
        lua_pushnumber(L, self->getBatches());
    } else if (!::strcmp(key,"batchesWithChildren")) {
        lua_pushnumber(L, self->getBatchesWithChildren());
    } else if (!::strcmp(key,"triangles")) {
        lua_pushnumber(L, self->getTriangles());
    } else if (!::strcmp(key,"trianglesWithChildren")) {
        lua_pushnumber(L, self->getTrianglesWithChildren());
    } else if (!::strcmp(key,"vertexes")) {
        lua_pushnumber(L, self->getVertexes());
    } else if (!::strcmp(key,"vertexesWithChildren")) {
        lua_pushnumber(L, self->getVertexesWithChildren());
    } else if (!::strcmp(key,"getMaterials")) {
        push_cfunction(L,gfxbody_get_materials);
    } else if (!::strcmp(key,"setMaterial")) {
        push_cfunction(L,gfxbody_set_material);
    } else if (!::strcmp(key,"setAllMaterials")) {
        push_cfunction(L,gfxbody_set_all_materials);
    } else if (!::strcmp(key,"getEmissiveEnabled")) {
        push_cfunction(L,gfxbody_get_emissive_enabled);
    } else if (!::strcmp(key,"setEmissiveEnabled")) {
        push_cfunction(L,gfxbody_set_emissive_enabled);
    } else if (!::strcmp(key,"reinitialise")) {
        push_cfunction(L, gfxbody_reinitialise);

    } else if (!::strcmp(key,"fade")) {
        lua_pushnumber(L, self->getFade());
    } else if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"wireframe")) {
        lua_pushboolean(L, self->getWireframe());
    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());

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
/*
    } else if (!::strcmp(key,"setAnimation")) {
        push_cfunction(L,gfxbody_set_animation);
    } else if (!::strcmp(key,"findAnimation")) {
        push_cfunction(L,gfxbody_find_animation);
    } else if (!::strcmp(key,"updateAnimation")) {
        push_cfunction(L,gfxbody_update_animation);
*/

    } else if (!::strcmp(key,"getAllAnimations")) {
        push_cfunction(L,gfxbody_get_all_animations);

    } else if (!::strcmp(key,"getAnimationLength")) {
        push_cfunction(L,gfxbody_get_animation_length);
    } else if (!::strcmp(key,"getAnimationPos")) {
        push_cfunction(L,gfxbody_get_animation_pos);
    } else if (!::strcmp(key,"getAnimationPosNormalised")) {
        push_cfunction(L,gfxbody_get_animation_pos_normalised);
    } else if (!::strcmp(key,"setAnimationPos")) {
        push_cfunction(L,gfxbody_set_animation_pos);
    } else if (!::strcmp(key,"setAnimationPosNormalised")) {
        push_cfunction(L,gfxbody_set_animation_pos_normalised);
    } else if (!::strcmp(key,"getAnimationMask")) {
        push_cfunction(L,gfxbody_get_animation_mask);
    } else if (!::strcmp(key,"setAnimationMask")) {
        push_cfunction(L,gfxbody_set_animation_mask);

    } else if (!::strcmp(key,"meshName")) {
        push_string(L,self->getMeshName());
    } else if (!::strcmp(key,"makeChild")) {
        push_cfunction(L,gfxbody_make_child);
    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
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
            self->setParent(GfxNodePtr(NULL));
        } else {
            GfxNodePtr par = check_gfx_node(L, 3);
            self->setParent(par);
        }
    } else if (!::strcmp(key,"parentBone")) {
        if (lua_isnil(L,3)) {
            self->setParentBoneName("");
        } else {
            std::string s = check_string(L, 3);
            self->setParentBoneName(s);
        }
    } else if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else if (!::strcmp(key,"wireframe")) {
        bool v = check_bool(L,3);
        self->setWireframe(v);
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


// GFXRANGEDINSTANCES ============================================================== {{{

void push_gfxrangedinstances (lua_State *L, const GfxRangedInstancesPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxRangedInstancesPtr(self),GFXRANGEDINSTANCES_TAG);
}

GC_MACRO(GfxRangedInstancesPtr,gfxrangedinstances,GFXRANGEDINSTANCES_TAG)

static int gfxrangedinstances_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    self->destroy();
    return 0;
TRY_END
}

static int gfxrangedinstances_add (lua_State *L)
{
TRY_START
    check_args(L,4);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    Vector3 v = check_v3(L,2);
    Quaternion q = check_quat(L,3);
    float f = check_float(L,4);
    unsigned index = self->add(v, q, f);
    lua_pushnumber(L, index);
    return 1;
TRY_END
}

static int gfxrangedinstances_update (lua_State *L)
{
TRY_START
    check_args(L,5);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    unsigned index = check_t<unsigned>(L, 2);
    Vector3 v = check_v3(L,3);
    Quaternion q = check_quat(L,4);
    float f = check_float(L,5);
    self->update(index, v, q, f);
    return 0;
TRY_END
}

static int gfxrangedinstances_del (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    unsigned index = check_t<unsigned>(L, 2);
    self->del(index);
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxrangedinstances,GfxRangedInstancesPtr,GFXRANGEDINSTANCES_TAG)


static int gfxrangedinstances_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxrangedinstances_destroy);
    } else if (!::strcmp(key,"parent")) {
        push_gfx_node_concrete(L, self->getParent());
    } else if (!::strcmp(key,"add")) {
        push_cfunction(L,gfxrangedinstances_add);
    } else if (!::strcmp(key,"update")) {
        push_cfunction(L,gfxrangedinstances_update);
    } else if (!::strcmp(key,"del")) {
        push_cfunction(L,gfxrangedinstances_del);
    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());
    } else if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"instances")) {
        lua_pushnumber(L, self->getInstances());
    } else if (!::strcmp(key,"trianglesPerInstance")) {
        lua_pushnumber(L, self->getTrianglesPerInstance());
    } else if (!::strcmp(key,"triangles")) {
        lua_pushnumber(L, self->getTrianglesPerInstance() * self->getInstances());
    } else if (!::strcmp(key,"batches")) {
        lua_pushnumber(L, self->getBatches());
    } else if (!::strcmp(key,"meshName")) {
        push_string(L,self->getMeshName());
    } else {
        my_lua_error(L,"Not a readable GfxRangedInstance member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxrangedinstances_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self->setParent(GfxNodePtr(NULL));
        } else {
            GfxNodePtr par = check_gfx_node(L, 3);
            self->setParent(par);
        }
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else {
           my_lua_error(L,"Not a writeable GfxRangedInstance member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxRangedInstancesPtr,gfxrangedinstances,GFXRANGEDINSTANCES_TAG)

MT_MACRO_NEWINDEX(gfxrangedinstances);

//}}}


// GFXINSTANCES ============================================================== {{{

void push_gfxinstances (lua_State *L, const GfxInstancesPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxInstancesPtr(self),GFXINSTANCES_TAG);
}

GC_MACRO(GfxInstancesPtr,gfxinstances,GFXINSTANCES_TAG)

static int gfxinstances_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    self->destroy();
    return 0;
TRY_END
}

static int gfxinstances_add (lua_State *L)
{
TRY_START
    check_args(L,4);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    Vector3 v = check_v3(L,2);
    Quaternion q = check_quat(L,3);
    float f = check_float(L,4);
    unsigned index = self->add(v, q, f);
    lua_pushnumber(L, index);
    return 1;
TRY_END
}

static int gfxinstances_update (lua_State *L)
{
TRY_START
    check_args(L,5);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    unsigned index = check_t<unsigned>(L, 2);
    Vector3 v = check_v3(L,3);
    Quaternion q = check_quat(L,4);
    float f = check_float(L,5);
    self->update(index, v, q, f);
    return 0;
TRY_END
}

static int gfxinstances_del (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    unsigned index = check_t<unsigned>(L, 2);
    self->del(index);
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxinstances,GfxInstancesPtr,GFXINSTANCES_TAG)


static int gfxinstances_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxinstances_destroy);
    } else if (!::strcmp(key,"parent")) {
        push_gfx_node_concrete(L, self->getParent());
    } else if (!::strcmp(key,"add")) {
        push_cfunction(L,gfxinstances_add);
    } else if (!::strcmp(key,"update")) {
        push_cfunction(L,gfxinstances_update);
    } else if (!::strcmp(key,"del")) {
        push_cfunction(L,gfxinstances_del);
    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());
    } else if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"instances")) {
        lua_pushnumber(L, self->getInstances());
    } else if (!::strcmp(key,"trianglesPerInstance")) {
        lua_pushnumber(L, self->getTrianglesPerInstance());
    } else if (!::strcmp(key,"triangles")) {
        lua_pushnumber(L, self->getTrianglesPerInstance() * self->getInstances());
    } else if (!::strcmp(key,"batches")) {
        lua_pushnumber(L, self->getBatches());
    } else if (!::strcmp(key,"meshName")) {
        push_string(L,self->getMeshName());

    } else {
        my_lua_error(L,"Not a readable GfxInstance member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxinstances_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self->setParent(GfxNodePtr(NULL));
        } else {
            GfxNodePtr par = check_gfx_node(L, 3);
            self->setParent(par);
        }
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else {
           my_lua_error(L,"Not a writeable GfxInstance member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxInstancesPtr,gfxinstances,GFXINSTANCES_TAG)

MT_MACRO_NEWINDEX(gfxinstances);

//}}}


// GFXSKYBODY ============================================================== {{{

void push_gfxskybody (lua_State *L, const GfxSkyBodyPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxSkyBodyPtr(self),GFXSKYBODY_TAG);
}

GC_MACRO(GfxSkyBodyPtr,gfxskybody,GFXSKYBODY_TAG)

static int gfxskybody_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxSkyBodyPtr,self,1,GFXSKYBODY_TAG);
    self->destroy();
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxskybody,GfxSkyBodyPtr,GFXSKYBODY_TAG)


static int gfxskybody_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxSkyBodyPtr,self,1,GFXSKYBODY_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"orientation")) {
        push_quat(L, self->getOrientation());
    } else if (!::strcmp(key,"zOrder")) {
        lua_pushnumber(L, self->getZOrder());

    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());

    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxskybody_destroy);
    } else {
        my_lua_error(L,"Not a readable GfxSkyBody member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxskybody_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxSkyBodyPtr,self,1,GFXSKYBODY_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"orientation")) {
        Quaternion v = check_quat(L,3);
        self->setOrientation(v);
    } else if (!::strcmp(key,"zOrder")) {
        unsigned char v = check_int(L,3,0,255);
        self->setZOrder(v);
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else {
           my_lua_error(L,"Not a writeable GfxSkyBody member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxBodyPtr,gfxskybody,GFXSKYBODY_TAG)

MT_MACRO_NEWINDEX(gfxskybody);

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
    } else if (!::strcmp(key,"localOrientation")) {
        push_quat(L, self->getLocalOrientation());
    } else if (!::strcmp(key,"localScale")) {
        push_v3(L, self->getLocalScale());
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
    } else if (!::strcmp(key,"range")) {
        lua_pushnumber(L, self->getRange());
    } else if (!::strcmp(key,"fade")) {
        lua_pushnumber(L, self->getFade());
    } else if (!::strcmp(key,"innerAngle")) {
        lua_pushnumber(L, self->getInnerAngle().inDegrees());
    } else if (!::strcmp(key,"outerAngle")) {
        lua_pushnumber(L, self->getOuterAngle().inDegrees());
    } else if (!::strcmp(key,"coronaInnerAngle")) {
        lua_pushnumber(L, self->getCoronaInnerAngle().inDegrees());
    } else if (!::strcmp(key,"coronaOuterAngle")) {
        lua_pushnumber(L, self->getCoronaOuterAngle().inDegrees());
    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());
    } else if (!::strcmp(key,"parent")) {
        push_gfx_node_concrete(L, self->getParent());

    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
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
        float v = check_float(L,3);
        self->setCoronaSize(v);
    } else if (!::strcmp(key,"coronaLocalPosition")) {
        Vector3 v = check_v3(L,3);
        self->setCoronaLocalPosition(v);
    } else if (!::strcmp(key,"coronaColour")) {
        Vector3 v = check_v3(L,3);
        self->setCoronaColour(v);
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
    } else if (!::strcmp(key,"coronaInnerAngle")) {
        float v = check_float(L,3);
        self->setCoronaInnerAngle(Degree(v));
    } else if (!::strcmp(key,"coronaOuterAngle")) {
        float v = check_float(L,3);
        self->setCoronaOuterAngle(Degree(v));
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self->setParent(GfxNodePtr(NULL));
        } else {
            GfxNodePtr par = check_gfx_node(L, 3);
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



// GFXHUDCLASS ============================================================= {{{

void push_gfxhudclass (lua_State *L, GfxHudClass *self)
{
        if (self == NULL) {
                lua_pushnil(L);
        } else {
                push(L,self,GFXHUDCLASS_TAG);
        }
}

static int gfxhudclass_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(GfxHudClass,self,1,GFXHUDCLASS_TAG,0);
        return 0;
TRY_END
}

TOSTRING_GETNAME_MACRO(gfxhudclass,GfxHudClass,.name,GFXHUDCLASS_TAG)



static int gfxhudclass_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxHudClass,self,1,GFXHUDCLASS_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="name") {
                lua_pushstring(L,self.name.c_str());
        } else if (key=="dump") {
                self.dump(L);
        } else {
                self.get(L,key);
        }
        return 1;
TRY_END
}

static int gfxhudclass_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxHudClass,self,1,GFXHUDCLASS_TAG);
        std::string key = luaL_checkstring(L,2);

        if (key=="name") {
                my_lua_error(L,"Not a writeable GfxHudClass member: "+key);
        } else if (key=="dump") {
                my_lua_error(L,"Not a writeable GfxHudClass member: "+key);
        } else {
                self.set(L,key);
        }

        return 0;
TRY_END
}

EQ_PTR_MACRO(GfxHudClass,gfxhudclass,GFXHUDCLASS_TAG)

MT_MACRO_NEWINDEX(gfxhudclass);

//}}}


// GFXHUDOBJECT ============================================================== {{{

void push_gfxhudobj (lua_State *L, GfxHudObject *self)
{
    if (self==NULL)
        lua_pushnil(L);
    else {
        self->incRefCount();
        push(L,self,GFXHUDOBJECT_TAG);
    }
}

static int gfxhudobj_gc (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxHudObject,self,1,GFXHUDOBJECT_TAG);
    self.decRefCount(L);
    return 0;
TRY_END
}

static int gfxhudobj_set_rect (lua_State *L)
{
TRY_START
    check_args(L,5);
    GET_UD_MACRO(GfxHudObject,self,1,GFXHUDOBJECT_TAG);
    float left = check_float(L,2);
    float bottom = check_float(L,3);
    float right = check_float(L,4);
    float top = check_float(L,5);
    left = floorf(left + 0.5);
    bottom = floorf(bottom + 0.5);
    right = floorf(right + 0.5);
    top = floorf(top + 0.5);
    self.setPosition(Vector2((right+left)/2, (top+bottom)/2));
    self.setSize(L, Vector2(right-left, top-bottom));
    return 0;
TRY_END
}

static int gfxhudobj_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxHudObject,self,1,GFXHUDOBJECT_TAG);
    self.destroy(L);
    return 0;
TRY_END
}



TOSTRING_ADDR_MACRO (gfxhudobj,GfxHudObject,GFXHUDOBJECT_TAG)


static int gfxhudobj_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxHudObject,self,1,GFXHUDOBJECT_TAG);
    if (lua_type(L,2) == LUA_TSTRING) {
        const char *key = lua_tostring(L,2);
        if (!::strcmp(key,"orientation")) {
            lua_pushnumber(L, self.getOrientation().inDegrees());
        } else if (!::strcmp(key,"position")) {
            push_v2(L, self.getPosition());
        } else if (!::strcmp(key,"derivedPosition")) {
            push_v2(L, self.getDerivedPosition());
        } else if (!::strcmp(key,"derivedOrientation")) {
            lua_pushnumber(L, self.getDerivedOrientation().inDegrees());
        } else if (!::strcmp(key,"size")) {
            push_v2(L, self.GfxHudObject::getSize());
        } else if (!::strcmp(key,"sizeSet")) {
            lua_pushboolean(L, self.getSizeSet());
        } else if (!::strcmp(key,"bounds")) {
            push_v2(L, self.getBounds());
        } else if (!::strcmp(key,"derivedBounds")) {
            push_v2(L, self.getDerivedBounds());
        } else if (!::strcmp(key,"setRect")) {
            push_cfunction(L, gfxhudobj_set_rect);
        } else if (!::strcmp(key,"zOrder")) {
            lua_pushnumber(L, self.getZOrder());
        } else if (!::strcmp(key,"inheritOrientation")) {
            lua_pushboolean(L, self.getInheritOrientation());
        } else if (!::strcmp(key,"snapPixels")) {
            lua_pushboolean(L, self.snapPixels);

        } else if (!::strcmp(key,"colour")) {
            push_v3(L, self.getColour());
        } else if (!::strcmp(key,"alpha")) {
            lua_pushnumber(L, self.getAlpha());
        } else if (!::strcmp(key,"texture")) {
            GfxTextureDiskResource *d = self.getTexture();
            if (d==NULL) {
                lua_pushnil(L);
            } else {
                push_string(L, d->getName());
            }

        } else if (!::strcmp(key,"needsInputCallbacks")) {
            lua_pushboolean(L, self.getNeedsInputCallbacks());
        } else if (!::strcmp(key,"needsFrameCallbacks")) {
            lua_pushboolean(L, self.getNeedsFrameCallbacks());
        } else if (!::strcmp(key,"needsParentResizedCallbacks")) {
            lua_pushboolean(L, self.getNeedsParentResizedCallbacks());

        } else if (!::strcmp(key,"cornered")) {
            lua_pushboolean(L, self.isCornered());

        } else if (!::strcmp(key,"enabled")) {
            lua_pushboolean(L, self.isEnabled());

        } else if (!::strcmp(key,"parent")) {
            push_gfxhudobj(L, self.getParent());
        } else if (!::strcmp(key,"class")) {
            push_gfxhudclass(L, self.hudClass);
        } else if (!::strcmp(key,"className")) {
            push_string(L, self.hudClass->name);
        } else if (!::strcmp(key,"table")) {
            self.table.push(L);

        } else if (!::strcmp(key,"destroyed")) {
            lua_pushboolean(L,self.destroyed());
        } else if (!::strcmp(key,"destroy")) {
            push_cfunction(L,gfxhudobj_destroy);
        } else {
            if (self.destroyed()) my_lua_error(L,"GfxHudObject destroyed");
            self.table.push(L);
            lua_pushstring(L, key);
            lua_rawget(L, -2);

            if (!lua_isnil(L,-1)) return 1;
            lua_pop(L,1);

            // try class instead
            self.hudClass->get(L,key);
        }
    } else {
        if (self.destroyed()) my_lua_error(L,"GfxHudObject destroyed");
        self.table.push(L);
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);

        if (!lua_isnil(L,-1)) return 1;
        lua_pop(L,1);

        // try class instead
        lua_pushvalue(L,2);
        self.hudClass->get(L);
    }
    return 1;
TRY_END
}


static int gfxhudobj_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxHudObject,self,1,GFXHUDOBJECT_TAG);
    if (lua_type(L,2) == LUA_TSTRING) {
        const char *key = lua_tostring(L,2);
        if (!::strcmp(key,"orientation")) {
            float v = check_float(L,3);
            self.setOrientation(Degree(v));
        } else if (!::strcmp(key,"position")) {
            Vector2 v = check_v2(L,3);
            self.setPosition(v);
        } else if (!::strcmp(key,"size")) {
            Vector2 v = check_v2(L,3);
            self.setSize(L, v);
        } else if (!::strcmp(key,"inheritOrientation")) {
            bool v = check_bool(L, 3);
            self.setInheritOrientation(v);
        } else if (!::strcmp(key,"snapPixels")) {
            bool v = check_bool(L, 3);
            self.snapPixels = v;

        } else if (!::strcmp(key,"colour")) {
            Vector3 v = check_v3(L,3);
            self.setColour(v);
        } else if (!::strcmp(key,"alpha")) {
            float v = check_float(L,3);
            self.setAlpha(v);
        } else if (!::strcmp(key,"texture")) {
            if (lua_isnil(L,3)) {
                self.setTexture(NULL);
            } else {
                std::string v = check_path(L,3);
                DiskResource *d = disk_resource_get_or_make(v);
                GfxTextureDiskResource *d2 = dynamic_cast<GfxTextureDiskResource*>(d);
                if (d2==NULL) my_lua_error(L, "Resource not a texture: \""+v+"\"");
                self.setTexture(d2);
            }

        } else if (!::strcmp(key,"parent")) {
            if (lua_isnil(L,3)) {
                self.setParent(L, NULL);
            } else {
                GET_UD_MACRO(GfxHudObject,v,3,GFXHUDOBJECT_TAG);
                self.setParent(L, &v);
            }
        } else if (!::strcmp(key,"zOrder")) {
            unsigned char v = check_int(L,3,0,7);
            self.setZOrder(v);

        } else if (!::strcmp(key,"needsInputCallbacks")) {
            bool v = check_bool(L,3);
            self.setNeedsInputCallbacks(v);
        } else if (!::strcmp(key,"needsFrameCallbacks")) {
            bool v = check_bool(L,3);
            self.setNeedsFrameCallbacks(v);
        } else if (!::strcmp(key,"needsParentResizedCallbacks")) {
            bool v = check_bool(L,3);
            self.setNeedsParentResizedCallbacks(v);

        } else if (!::strcmp(key,"cornered")) {
            bool v = check_bool(L,3);
            self.setCornered(v);

        } else if (!::strcmp(key,"enabled")) {
            bool v = check_bool(L,3);
            self.setEnabled(v);
        } else if (!::strcmp(key,"table")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"class")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"className")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"destroy")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"destroyed")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else {
            if (self.destroyed()) my_lua_error(L,"GfxHudObject destroyed");

            self.table.push(L);
            lua_pushvalue(L, 2);
            lua_pushvalue(L, 3);
            lua_rawset(L, -3);
        }
    } else {
        if (self.destroyed()) my_lua_error(L,"GfxHudObject destroyed");

        self.table.push(L);
        lua_pushvalue(L, 2);
        lua_pushvalue(L, 3);
        lua_rawset(L, -3);
    }
    return 0;
TRY_END
}

EQ_PTR_MACRO(GfxBodyPtr,gfxhudobj,GFXHUDOBJECT_TAG)

MT_MACRO_NEWINDEX(gfxhudobj);

//}}}


// GFXHUDTEXT ============================================================== {{{

void push_gfxhudtext (lua_State *L, GfxHudText *self)
{
    if (self==NULL)
        lua_pushnil(L);
    else {
        self->incRefCount();
        push(L,self,GFXHUDTEXT_TAG);
    }
}

static int gfxhudtext_gc (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxHudText,self,1,GFXHUDTEXT_TAG);
    self.decRefCount();
    return 0;
TRY_END
}

static int gfxhudtext_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxHudText,self,1,GFXHUDTEXT_TAG);
    self.destroy();
    return 0;
TRY_END
}

static int gfxhudtext_append (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxHudText,self,1,GFXHUDTEXT_TAG);
    std::string str = check_string(L,2);
    self.append(str);
    return 0;
TRY_END
}

static int gfxhudtext_clear (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxHudText,self,1,GFXHUDTEXT_TAG);
    self.clear();
    return 0;
TRY_END
}



TOSTRING_ADDR_MACRO (gfxhudtext,GfxHudText,GFXHUDTEXT_TAG)


static int gfxhudtext_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxHudText,self,1,GFXHUDTEXT_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"orientation")) {
        lua_pushnumber(L, self.getOrientation().inDegrees());
    } else if (!::strcmp(key,"position")) {
        push_v2(L, self.getPosition());
    } else if (!::strcmp(key,"derivedPosition")) {
        push_v2(L, self.getDerivedPosition());
    } else if (!::strcmp(key,"derivedOrientation")) {
        lua_pushnumber(L, self.getDerivedOrientation().inDegrees());
    } else if (!::strcmp(key,"zOrder")) {
        lua_pushnumber(L, self.getZOrder());
    } else if (!::strcmp(key,"size")) {
        push_v2(L, self.GfxHudText::getSize());
    } else if (!::strcmp(key,"textWrap")) {
        if (self.getTextWrap() == Vector2(0,0)) {
            lua_pushnil(L);
        } else {
            push_v2(L, self.getTextWrap());
        }
    } else if (!::strcmp(key,"scroll")) {
        lua_pushnumber(L, self.getScroll());
    } else if (!::strcmp(key,"shadow")) {
        if (self.getShadow() == Vector2(0,0)) {
            lua_pushnil(L);
        } else {
            push_v2(L, self.getShadow());
        }
    } else if (!::strcmp(key,"shadowColour")) {
        push_v3(L, self.getShadowColour());
    } else if (!::strcmp(key,"shadowAlpha")) {
        lua_pushnumber(L, self.getShadowAlpha());
    } else if (!::strcmp(key,"bufferHeight")) {
        lua_pushnumber(L, self.getBufferHeight());
    } else if (!::strcmp(key,"bounds")) {
        push_v2(L, self.getBounds());
    } else if (!::strcmp(key,"derivedBounds")) {
        push_v2(L, self.getDerivedBounds());
    } else if (!::strcmp(key,"inheritOrientation")) {
        lua_pushboolean(L, self.getInheritOrientation());
    } else if (!::strcmp(key,"snapPixels")) {
        lua_pushboolean(L, self.snapPixels);

    } else if (!::strcmp(key,"colour")) {
        push_v3(L, self.getColour());
    } else if (!::strcmp(key,"alpha")) {
        lua_pushnumber(L, self.getAlpha());
    } else if (!::strcmp(key,"letterTopColour")) {
        push_v3(L, self.getLetterTopColour());
    } else if (!::strcmp(key,"letterTopAlpha")) {
        lua_pushnumber(L, self.getLetterTopAlpha());
    } else if (!::strcmp(key,"letterBottomColour")) {
        push_v3(L, self.getLetterBottomColour());
    } else if (!::strcmp(key,"letterBottomAlpha")) {
        lua_pushnumber(L, self.getLetterBottomAlpha());
    } else if (!::strcmp(key,"font")) {
        GfxFont *font = self.getFont();
        push_string(L, font->name);

    } else if (!::strcmp(key,"text")) {
        push_string(L, self.getText());
    } else if (!::strcmp(key,"clear")) {
        push_cfunction(L,gfxhudtext_clear);
    } else if (!::strcmp(key,"append")) {
        push_cfunction(L,gfxhudtext_append);

    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self.isEnabled());

    } else if (!::strcmp(key,"parent")) {
        push_gfxhudobj(L, self.getParent());

    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self.destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxhudtext_destroy);
    } else {
        my_lua_error(L,"Not a readable GfxHudText member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxhudtext_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxHudText,self,1,GFXHUDTEXT_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"orientation")) {
        float v = check_float(L,3);
        self.setOrientation(Degree(v));
    } else if (!::strcmp(key,"position")) {
        Vector2 v = check_v2(L,3);
        self.setPosition(v);
    } else if (!::strcmp(key,"textWrap")) {
        if (lua_isnil(L,3)) {
            self.setTextWrap(Vector2(0,0));
        } else {
            Vector2 v = check_v2(L,3);
            self.setTextWrap(v);
        }
    } else if (!::strcmp(key,"scroll")) {
        long v = check_t<long>(L, 3);
        self.setScroll(v);
    } else if (!::strcmp(key,"shadow")) {
        if (lua_isnil(L,3)) {
            self.setShadow(Vector2(0,0));
        } else {
            Vector2 v = check_v2(L,3);
            self.setShadow(v);
        }
    } else if (!::strcmp(key,"shadowColour")) {
        Vector3 v = check_v3(L,3);
        self.setShadowColour(v);
    } else if (!::strcmp(key,"shadowAlpha")) {
        float v = check_float(L,3);
        self.setShadowAlpha(v);
    } else if (!::strcmp(key,"inheritOrientation")) {
        bool v = check_bool(L, 3);
        self.setInheritOrientation(v);
    } else if (!::strcmp(key,"snapPixels")) {
        bool v = check_bool(L, 3);
        self.snapPixels = v;

    } else if (!::strcmp(key,"colour")) {
        Vector3 v = check_v3(L,3);
        self.setColour(v);
    } else if (!::strcmp(key,"alpha")) {
        float v = check_float(L,3);
        self.setAlpha(v);
    } else if (!::strcmp(key,"letterTopColour")) {
        Vector3 v = check_v3(L,3);
        self.setLetterTopColour(v);
    } else if (!::strcmp(key,"letterTopAlpha")) {
        float v = check_float(L,3);
        self.setLetterTopAlpha(v);
    } else if (!::strcmp(key,"letterBottomColour")) {
        Vector3 v = check_v3(L,3);
        self.setLetterBottomColour(v);
    } else if (!::strcmp(key,"letterBottomAlpha")) {
        float v = check_float(L,3);
        self.setLetterBottomAlpha(v);
    } else if (!::strcmp(key,"font")) {
        std::string v = check_string(L,3);
        GfxFont *font = gfx_font_get(v);
        if (font == NULL) my_lua_error(L, "Font does not exist \""+v+"\"");
        self.setFont(font);

    } else if (!::strcmp(key,"text")) {
        std::string v = check_string(L,3);
        self.clear();
        self.setLetterTopColour(Vector3(1,1,1));
        self.setLetterBottomColour(Vector3(1,1,1));
        self.setLetterTopAlpha(1);
        self.setLetterBottomAlpha(1);
        self.append(v);

    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self.setParent(NULL);
        } else {
            GET_UD_MACRO(GfxHudObject,v,3,GFXHUDOBJECT_TAG);
            self.setParent(&v);
        }
    } else if (!::strcmp(key,"zOrder")) {
        unsigned char v = check_int(L,3,0,7);
        self.setZOrder(v);

    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self.setEnabled(v);
    } else {
        my_lua_error(L,"Not a writeable GfxHudText member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_PTR_MACRO(GfxBodyPtr,gfxhudtext,GFXHUDTEXT_TAG)

MT_MACRO_NEWINDEX(gfxhudtext);

//}}}



static int global_gfx_render (lua_State *L)
{
TRY_START
    check_args(L,3);
    float elapsed = check_float(L,1);
    Vector3 cam_pos = check_v3(L,2);
    Quaternion cam_dir = check_quat(L,3);
    gfx_hud_call_per_frame_callbacks(L, elapsed);
    gfx_render(elapsed, cam_pos, cam_dir);
    return 0;
TRY_END
}

static int global_gfx_bake_env_cube (lua_State *L)
{
TRY_START
    check_args(L,5);
    std::string name = check_string(L,1);
    unsigned size = check_t<unsigned>(L,2);
    Vector3 cam_pos = check_v3(L,3);
    float saturation = check_float(L,4);
    Vector3 ambient = check_v3(L,5);
    gfx_bake_env_cube(name, size, cam_pos, saturation, ambient);
    return 0;
TRY_END
}

static int global_gfx_screenshot (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *filename = luaL_checkstring(L,1);
    gfx_screenshot(filename);
    return 0;
TRY_END
}

static int global_gfx_option (lua_State *L)
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
            case 2: gfx_option(o2, check_float(L,2)); break;
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

static int global_gfx_instances_make (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string meshname = check_path(L,1);
    push_gfxinstances(L, GfxInstances::make(meshname));
    return 1;
TRY_END
}

static int global_gfx_ranged_instances_make (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string meshname = check_path(L,1);
    push_gfxrangedinstances(L, GfxRangedInstances::make(meshname));
    return 1;
TRY_END
}

static int global_gfx_body_make (lua_State *L)
{
TRY_START
    if (lua_gettop(L)==0) {
        push_gfxnode(L, GfxFertileNode::make());
    } else if (lua_gettop(L)==1) {
        std::string meshname = check_path(L,1);
        push_gfxbody(L, GfxBody::make(meshname));
    } else {
        // map the materials?
        check_args(L,2);
        std::string meshname = check_path(L,1);
        if (lua_isnil(L,2)) {
            push_gfxbody(L, GfxBody::make(meshname));
        } else {
            if (lua_type(L,2)!=LUA_TTABLE) {
                my_lua_error(L, "Second parameter should be a table (string map)");
            }
            GfxStringMap sm;
            for (lua_pushnil(L) ; lua_next(L,2)!=0 ; lua_pop(L,1)) {
                // stack: sm, key, val
                if (lua_type(L,-2)!=LUA_TSTRING) {
                    my_lua_error(L, "Table keys must be strings");
                }
                if (lua_type(L,-1)!=LUA_TSTRING) {
                    my_lua_error(L, "Table values must be strings");
                }
                sm[lua_tostring(L,-2)] = lua_tostring(L,-1);
            }
            push_gfxbody(L, GfxBody::make(meshname, sm));
        }
    }
    return 1;
TRY_END
}

static int global_gfx_sky_body_make (lua_State *L)
{
TRY_START
    std::string meshname = check_path(L,1);
    short z_order = check_t<short>(L,2);
    push_gfxskybody(L, GfxSkyBody::make(meshname, z_order));
    return 1;
TRY_END
}

static int global_gfx_font_define (lua_State *L)
{
TRY_START
    check_args(L,4);
    std::string name = check_path(L,1);
    std::string tex_name = check_path(L,2);
    lua_Number line_height = luaL_checknumber(L, 3);
    luaL_checktype(L,4,LUA_TTABLE);

    DiskResource *d = disk_resource_get_or_make(tex_name);
    if (d==NULL) my_lua_error(L, "Resource does not exist: \""+tex_name+"\"");
    GfxTextureDiskResource *d2 = dynamic_cast<GfxTextureDiskResource*>(d);
    if (d2==NULL) my_lua_error(L, "Resource not a texture: \""+tex_name+"\"");

    // get or create font of the right name
    GfxFont *font = gfx_font_make(name, d2, line_height);

    // iterate through codepoints
    for (lua_pushnil(L) ; lua_next(L,4) ; lua_pop(L,1)) {
        lua_Number codepoint = check_t<unsigned>(L,-2);
        lua_rawgeti(L, -1, 1);
        lua_Number x = check_t<unsigned>(L,-1);
        lua_pop(L,1);
        lua_rawgeti(L, -1, 2);
        lua_Number y = check_t<unsigned>(L,-1);
        lua_pop(L,1);
        lua_rawgeti(L, -1, 3);
        lua_Number w = check_t<unsigned>(L,-1);
        lua_pop(L,1);
        lua_rawgeti(L, -1, 4);
        lua_Number h = check_t<unsigned>(L,-1);
        lua_pop(L,1);

        GfxFont::CharRect cr;
        cr.u1 = x;
        cr.v1 = y;
        cr.u2 = cr.u1 + w;
        cr.v2 = cr.v1 + h;
        font->setCodePoint(codepoint, cr);
    }

    return 0;
TRY_END
}

static int global_gfx_font_line_height (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    GfxFont *font = gfx_font_get(name);
    if (font == NULL) EXCEPT << "Font does not exist: \"" << name << "\"" << ENDL;
    lua_pushnumber(L, font->getHeight());
    return 1;
TRY_END
}

static int global_gfx_font_list (lua_State *L)
{
TRY_START
    check_args(L, 0);
    std::vector<GfxFont*> list = gfx_font_list();
    lua_newtable(L);
    int counter = 1;
    for (unsigned i=0 ; i<list.size() ; ++i) {
        lua_pushstring(L, list[i]->name.c_str());
        lua_rawseti(L,-2,counter);
        counter++;
    }
    return 1;
TRY_END
}

static int global_gfx_font_text_width (lua_State *L)
{
TRY_START
    check_args(L, 2);
    std::string name = check_path(L,1);
    std::string text = check_string(L, 2);
    GfxFont *font = gfx_font_get(name);
    if (font == NULL) EXCEPT << "Font does not exist: \"" << name << "\"" << ENDL;
    unsigned long width = 0;
    for (size_t i=0 ; i<text.length() ; ++i) {
        GfxFont::codepoint_t cp = decode_utf8(text, i);
        GfxFont::CharRect uvs;
        bool r = font->getCodePointOrFail(cp, uvs);
        if (!r) continue;
        width += uvs.u2 - uvs.u1;
    }
    lua_pushnumber(L, width);
    return 1;
TRY_END
}

static int global_gfx_hud_text_add (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string font_name = check_path(L,1);
    GfxFont *font = gfx_font_get(font_name);
    if (font == NULL) my_lua_error(L, "Font does not exist: \""+font_name+"\"");
    
    GfxHudText *self = new GfxHudText(font);
    push_gfxhudtext(L,self);

    return 1;
TRY_END
}


static int global_gfx_hud_object_add (lua_State *L)
{
TRY_START
    if (lua_gettop(L) == 1) lua_newtable(L);
    check_args(L,2);
    std::string class_name = check_path(L,1);
    GfxHudClass *hud_class = gfx_hud_class_get(class_name);
    int table_index = lua_gettop(L);
    if (!lua_istable(L,table_index)) my_lua_error(L,"Last parameter should be a table");
    
    GfxHudObject *parent = NULL;
    GfxHudObject *self = new GfxHudObject(hud_class);

    // push this now so that it will inc its internal ref counter and not be deleted if aliases are gc'd
    push_gfxhudobj(L,self);

    bool have_orientation = false;
    bool have_position = false;
    bool have_size = false;
    bool have_colour = false;
    bool have_alpha = false;
    bool have_texture = false;
    bool have_zorder = false;
    bool have_cornered = false;
    bool have_enabled = false;

    lua_newtable(L);
    int new_table_index = lua_gettop(L);

    self->table.setNoPop(L);

    // scan through table adding lua data to self
    for (lua_pushnil(L) ; lua_next(L,table_index)!=0 ; lua_pop(L,1)) {
        const char *key = "";
        if (lua_type(L,-2)==LUA_TSTRING) {
            key = luaL_checkstring(L,-2);
        }
        
        if (!::strcmp(key,"orientation")) {
            if (lua_isnumber(L,-1)) {
                float v = check_float(L,-1);
                self->setOrientation(Degree(v));
                have_orientation = true;
            } else {
                my_lua_error(L, "orientation must be a number.");
            }
        } else if (!::strcmp(key,"position")) {
            if (lua_isvector2(L,-1)) {
                Vector2 v = check_v2(L,-1);
                self->setPosition(v);
                have_position = true;
            } else {
                my_lua_error(L, "position must be a vector2.");
            }
        } else if (!::strcmp(key,"size")) {
            if (lua_isvector2(L,-1)) {
                Vector2 v = check_v2(L,-1);
                self->setSize(L, v);
                have_size = true;
            } else {
                my_lua_error(L, "size must be a vector2.");
            }
        } else if (!::strcmp(key,"parent")) {
            if (lua_isnil(L,-1)) {
                parent = NULL;
            } else {
                GET_UD_MACRO(GfxHudObject,v,-1,GFXHUDOBJECT_TAG);
                parent = &v;
            }
        } else if (!::strcmp(key,"colour")) {
            if (lua_isvector3(L,-1)) {
                Vector3 v = check_v3(L,-1);
                self->setColour(v);
                have_colour = true;
            } else {
                my_lua_error(L, "colour must be a vector3.");
            }
        } else if (!::strcmp(key,"alpha")) {
            if (lua_isnumber(L,-1)) {
                float v = check_float(L,-1);
                self->setAlpha(v);
                have_alpha = true;
            } else {
                my_lua_error(L, "alpha must be a number.");
            }
        } else if (!::strcmp(key,"texture")) {
            if (lua_type(L,-1) == LUA_TSTRING) {
                std::string v = check_path(L,-1);
                DiskResource *d = disk_resource_get_or_make(v);
                GfxTextureDiskResource *d2 = dynamic_cast<GfxTextureDiskResource*>(d);
                if (d2==NULL) my_lua_error(L, "Resource not a texture: \""+v+"\"");
                self->setTexture(d2);
                have_texture = true;
            } else {
                my_lua_error(L, "texture must be a string.");
            }
        } else if (!::strcmp(key,"zOrder")) {
            if (lua_isnumber(L,-1)) {
                lua_Number v = lua_tonumber(L,-1);
                if (v!=(unsigned char)(v)) {
                    my_lua_error(L, "zOrder must be an integer between 0 and 255 inclusive.");
                }
                self->setZOrder((unsigned char)v);
                have_zorder = true;
            } else {
                my_lua_error(L, "zOrder must be a number.");
            }
        } else if (!::strcmp(key,"cornered")) {
            if (lua_isboolean(L,-1)) {
                bool v = check_bool(L,-1);
                self->setCornered(v);
                have_cornered = true;
            } else {
                my_lua_error(L, "cornered must be a boolean.");
            }
        } else if (!::strcmp(key,"enabled")) {
            if (lua_isboolean(L,-1)) {
                bool v = check_bool(L,-1);
                self->setEnabled(v);
                have_enabled = true;
            } else {
                my_lua_error(L, "enabled must be a boolean.");
            }
        } else if (!::strcmp(key,"class")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"className")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"destroy")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"destroyed")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else {
            lua_pushvalue(L, -2); // push key
            lua_pushvalue(L, -2); // push value
            lua_rawset(L, new_table_index);
        }
    }
    ExternalTable &tab = hud_class->getTable();
    if (!have_orientation) {
        if (tab.has("orientation")) {
            lua_Number v;
            bool success = tab.get("orientation",v);
            if (!success) my_lua_error(L, "Wrong type for orientation field in hud class \""+hud_class->name+"\".");
            self->setOrientation(Degree(v));
        }
    }
    if (!have_position) {
        if (tab.has("position")) {
            Vector2 v;
            bool success = tab.get("position",v);
            if (!success) my_lua_error(L, "Wrong type for position field in hud class \""+hud_class->name+"\".");
            self->setPosition(v);
        }
    }
    if (!have_size) {
        if (tab.has("size")) {
            Vector2 v;
            bool success = tab.get("size",v);
            if (!success) my_lua_error(L, "Wrong type for size field in hud class \""+hud_class->name+"\".");
            self->setSize(L, v);
            have_size = true;
        }
    }
    if (!have_colour) {
        if (tab.has("colour")) {
            Vector3 v;
            bool success = tab.get("colour",v);
            if (!success) my_lua_error(L, "Wrong type for colour field in hud class \""+hud_class->name+"\".");
            self->setColour(v);
        }
    }
    if (!have_alpha) {
        if (tab.has("alpha")) {
            lua_Number v;
            bool success = tab.get("alpha",v);
            if (!success) my_lua_error(L, "Wrong type for alpha field in hud class \""+hud_class->name+"\".");
            self->setAlpha(v);
        }
    }
    if (!have_texture) {
        if (tab.has("texture")) {
            std::string v;
            bool success = tab.get("texture",v);
            if (!success) my_lua_error(L, "Wrong type for texture field in hud class \""+hud_class->name+"\".");
            DiskResource *d = disk_resource_get_or_make(v);
            if (d==NULL) my_lua_error(L, "Resource does not exist: \""+std::string(v)+"\"");
            GfxTextureDiskResource *d2 = dynamic_cast<GfxTextureDiskResource*>(d);
            if (d2==NULL) my_lua_error(L, "Resource not a texture: \""+std::string(v)+"\"");
            self->setTexture(d2);
        }
    }
    if (!have_zorder) {
        if (tab.has("zOrder")) {
            lua_Number v;
            bool success = tab.get("zOrder",v);
            if (!success) my_lua_error(L, "Wrong type for zOrder field in hud class \""+hud_class->name+"\".");
            if ((unsigned char)(v) != v) my_lua_error(L, "zOrder must be an integer in range 0 to 255 in class \""+hud_class->name+"\".");
            self->setZOrder((unsigned char)v);
        }
    }
    if (!have_cornered) {
        if (tab.has("cornered")) {
            bool v;
            bool success = tab.get("cornered",v);
            if (!success) my_lua_error(L, "Wrong type for cornered field in hud class \""+hud_class->name+"\".");
            self->setCornered(v);
        }
    }
    if (!have_enabled) {
        if (tab.has("enabled")) {
            bool v;
            bool success = tab.get("enabled",v);
            if (!success) my_lua_error(L, "Wrong type for enabled field in hud class \""+hud_class->name+"\".");
            self->setEnabled(v);
        }
    }

    self->triggerInit(L);
    if (!have_size && !self->getSizeSet() && self->getTexture()!=NULL) {
        // set size from texture
        GfxTextureDiskResource *dr = self->getTexture();
        if (dr->isLoaded()) {
            Ogre::TexturePtr tex = dr->getOgreTexturePtr();
            tex->load(); // otherwise width and height are 512!?
            self->setSize(L, Vector2(tex->getWidth(),tex->getHeight()));
        }
    }
    self->setParent(L, parent);

    push_gfxhudobj(L,self);

    return 1;
TRY_END
}

static int global_gfx_hud_class_add (lua_State *L)
{
TRY_START
    check_args(L,2);
    std::string name = check_path(L,1);
    if (!lua_istable(L,2))
        my_lua_error(L,"Second parameter should be a table");
    gfx_hud_class_add(L, name);
    return 1;
TRY_END
}

static int global_gfx_hud_class_get (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    push_gfxhudclass(L,gfx_hud_class_get(name));
    return 1;
TRY_END
}

static int global_gfx_hud_class_has (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    lua_pushboolean(L,gfx_hud_class_has(name));
    return 1;
TRY_END
}

static int global_gfx_hud_class_count (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_pushnumber(L, gfx_hud_class_count());
    return 1;
TRY_END
}

static int global_gfx_hud_class_all (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_newtable(L);
    unsigned int c = 0;
    GfxHudClassMap::iterator i, i_;
    for (gfx_hud_class_all(i,i_) ; i!=i_ ; ++i) {
        push_gfxhudclass(L,i->second);
        lua_rawseti(L,-2,c+1);
        c++;
    }
    return 1;
TRY_END
}

static int global_gfx_hud_ray (lua_State *L)
{
TRY_START
    check_args(L,1);
    Vector2 pos = check_v2(L,1);
    // negative values occur when dragging out of the game window
    // casting to unsigned makes them go away...
    push_gfxhudobj(L, gfx_hud_ray((unsigned)pos.x, (unsigned)pos.y));
    return 1;
TRY_END
}





static int global_gfx_light_make (lua_State *L)
{
TRY_START
    check_args(L,0);
    push_gfxlight(L, GfxLight::make());
    return 1;
TRY_END
}

static int global_gfx_sunlight_diffuse (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sunlight_diffuse());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sunlight_diffuse(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sunlight_specular (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sunlight_specular());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sunlight_specular(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sunlight_direction (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sunlight_direction());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sunlight_direction(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_direction (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sun_direction());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sun_direction(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_hell_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_hell_colour());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_hell_colour(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sun_colour());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sun_colour(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_alpha (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sun_alpha());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sun_alpha(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_size (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sun_size());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sun_size(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_falloff_distance (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sun_falloff_distance());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sun_falloff_distance(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_glare_sun_distance (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sky_glare_sun_distance());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sky_glare_sun_distance(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_glare_horizon_elevation (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sky_glare_horizon_elevation());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sky_glare_horizon_elevation(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_divider (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 4);
            lua_pushnumber(L, gfx_sky_divider(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 4);
            float v = check_float(L,2);
            gfx_sky_divider(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 5);
            push_v3(L, gfx_sky_colour(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 5);
            Vector3 v = check_v3(L,2);
            gfx_sky_colour(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_sun_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 5);
            push_v3(L, gfx_sky_sun_colour(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 5);
            Vector3 v = check_v3(L,2);
            gfx_sky_sun_colour(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_alpha (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 5);
            lua_pushnumber(L, gfx_sky_alpha(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 5);
            float v = check_float(L,2);
            gfx_sky_alpha(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_sun_alpha (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 5);
            lua_pushnumber(L, gfx_sky_alpha(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 5);
            float v = check_float(L,2);
            gfx_sky_alpha(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_cloud_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sky_cloud_colour());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sky_cloud_colour(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_cloud_coverage (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sky_cloud_coverage());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sky_cloud_coverage(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}


static int global_gfx_env_cube (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            if (gfx_env_cube() != NULL) {
                push_string(L, gfx_env_cube()->getName());
            } else {
                lua_pushnil(L);
            }
            return 1;
        }
        case 1: {
            if (lua_isnil(L, 1)) {
                gfx_env_cube(NULL);
            } else {
                std::string v = check_path(L,1);
                DiskResource *dr = disk_resource_get_or_make(v);
                GfxEnvCubeDiskResource *ec = dynamic_cast<GfxEnvCubeDiskResource*>(dr);
                if (ec == NULL) my_lua_error(L, "Not an env cube: \""+v+"\"");
                gfx_env_cube(ec);
            }
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_colour_grade (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            if (gfx_env_cube() != NULL) {
                push_string(L, gfx_colour_grade()->getName());
            } else {
                lua_pushnil(L);
            }
            return 1;
        }
        case 1: {
            if (lua_isnil(L, 1)) {
                gfx_colour_grade(NULL);
            } else {
                std::string v = check_string(L,1);
                DiskResource *dr = disk_resource_get_or_make(v);
                GfxColourGradeLUTDiskResource *cg = dynamic_cast<GfxColourGradeLUTDiskResource*>(dr);
                if (cg == NULL) my_lua_error(L, "Not a colour grade LUT texture: \""+v+"\"");
                gfx_colour_grade(cg);
            }
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_global_saturation (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_global_saturation());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_global_saturation(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_global_exposure (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_global_exposure());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_global_exposure(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_particle_ambient (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_particle_ambient());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_particle_ambient(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_fog_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_fog_colour());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_fog_colour(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_fog_density (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_fog_density());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_fog_density(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static void push_stat (lua_State *L, GfxLastRenderStats &s)
{
    lua_pushnumber(L, s.batches);
    lua_pushnumber(L, s.triangles);
    lua_pushnumber(L, s.micros);
}

static int global_gfx_last_frame_stats (lua_State *L)
{
TRY_START
    check_args(L,0);
    GfxLastFrameStats s = gfx_last_frame_stats();
    lua_checkstack(L,30); // we're going to push a lot of stuff...
    push_stat(L, s.shadow[0]);
    push_stat(L, s.shadow[1]);
    push_stat(L, s.shadow[2]);
    push_stat(L, s.left_gbuffer);
    push_stat(L, s.left_deferred);
    push_stat(L, s.right_gbuffer);
    push_stat(L, s.right_deferred);
    return 3*7;
TRY_END
}

static int global_gfx_colour_grade_look_up (lua_State *L)
{
    check_args(L,1);
    Vector3 v = check_v3(L, 1);
    push_v3(L, gfx_colour_grade_look_up(v));
    return 1;
}

static int global_gfx_window_active (lua_State *L)
{
    check_args(L,0);
    lua_pushboolean(L, gfx_window_active());
    return 1;
}

static int global_gfx_window_size (lua_State *L)
{
    check_args(L,0);
    push_v2(L, gfx_window_size());
    return 1;
}

static int global_gfx_window_size_in_scene (lua_State *L)
{
    check_args(L,0);
    push_v2(L, gfx_window_size_in_scene());
    return 1;
}

static int global_gfx_world_to_screen(lua_State *L)
{
TRY_START
    check_args(L,3);
    Vector3 cp = check_v3(L, 1);
    Quaternion cq = check_quat(L, 2);
    Vector3 p = check_v3(L, 3);
    push_v3(L, gfx_world_to_screen(cp, cq, p));
    return 1;
TRY_END
}

static int global_gfx_gpu_ram_available (lua_State *L)
{
    check_args(L,0);
    lua_pushnumber(L, gfx_gpu_ram_available());
    return 1;
}

static int global_gfx_gpu_ram_used (lua_State *L)
{
    check_args(L,0);
    lua_pushnumber(L, gfx_gpu_ram_used());
    return 1;
}

static int global_rgb_to_hsl (lua_State *L)
{
    check_args(L,1);
    float r,g,b;
    lua_checkvector3(L, 1, &r, &g, &b);
    float h,s,l;
    RGBtoHSL(r,g,b, h,s,l);
    lua_pushvector3(L, h,s,l);
    return 1;
}

static int global_hsl_to_rgb (lua_State *L)
{
    check_args(L,1);
    float h,s,l;
    lua_checkvector3(L, 1, &h, &s, &l);
    float r,g,b;
    HSLtoRGB(h,s,l, r,g,b);
    lua_pushvector3(L, r,g,b);
    return 1;
}

static int global_hsv_to_hsl (lua_State *L)
{
    check_args(L,1);
    float hh,ss,ll;
    lua_checkvector3(L, 1, &hh, &ss, &ll);
    float h,s,l;
    HSVtoHSL(hh,ss,ll, h,s,l);
    lua_pushvector3(L, h,s,l);
    return 1;
}

static int global_hsl_to_hsv (lua_State *L)
{
    check_args(L,1);
    float h,s,l;
    lua_checkvector3(L, 1, &h, &s, &l);
    float hh,ss,ll;
    HSLtoHSV(h,s,l, hh,ss,ll);
    lua_pushvector3(L, hh,ss,ll);
    return 1;
}

static int global_rgb_to_hsv (lua_State *L)
{
    check_args(L,1);
    float r,g,b;
    lua_checkvector3(L, 1, &r, &g, &b);
    float h,s,v;
    RGBtoHSV(r,g,b, h,s,v);
    lua_pushvector3(L, h,s,v);
    return 1;
}

static int global_hsv_to_rgb (lua_State *L)
{
    check_args(L,1);
    float h,s,v;
    lua_checkvector3(L, 1, &h, &s, &v);
    float r,g,b;
    HSVtoRGB(h,s,v, r,g,b);
    lua_pushvector3(L, r,g,b);
    return 1;
}



// {{{ Particles

namespace {
    struct UVRect { float u1,v1, u2,v2; };
    struct ParticleDefinition;

    struct ParticleDefinition {
        ParticleDefinition (const std::string &m, const std::vector<UVRect> fs, lua_State *L, int t)
          : material(m), frames(fs)
        {
            table.takeTableFromLuaStack(L, t);
        }
        void destroy (lua_State *L);
                    
        std::string material;
        std::vector<UVRect> frames;
        ExternalTable table;
    };

    std::map<std::string, ParticleDefinition*> particle_defs;

    struct LuaParticle {

        LuaPtr state;
        GfxParticle *p;
        ParticleDefinition *pd;

        LuaParticle (lua_State *L, GfxParticle *p, ParticleDefinition *pd)
          : p(p), pd(pd)
        {
            state.set(L);
        }

        void destroy (lua_State *L)
        {
            state.setNil(L);
            p->release();
        }

        bool updateGraphics (lua_State *L, float elapsed, int error_handler)
        {
            state.push(L);
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
                    p->pos = check_v3(L,-1);
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "dimensions");
                if (lua_isnil(L,-1)) {
                    p->dimensions = Vector3(1,1,1);
                } else if (!lua_isvector3(L,-1)) {
                    CERR << "Particle dimensions was not a number." << std::endl;
                    destroy = true;
                } else {
                    p->dimensions = check_v3(L,-1);
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "colour");
                if (lua_isnil(L,-1)) {
                    p->colour = Vector3(1,1,1);
                } else if (!lua_isvector3(L,-1)) {
                    CERR << "Particle colour was not a vector3." << std::endl;
                    destroy = true;
                } else {
                    p->colour = check_v3(L,-1);
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "alpha");
                if (lua_isnil(L,-1)) {
                    p->alpha = 1;
                } else if (!lua_isnumber(L,-1)) {
                    CERR << "Particle alpha was not a number." << std::endl;
                    destroy = true;
                } else {
                    p->alpha = check_float(L,-1);
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "angle");
                if (lua_isnil(L,-1)) {
                    p->angle = 0;
                } else if (!lua_isnumber(L,-1)) {
                    CERR << "Particle angle was not a number." << std::endl;
                    destroy = true;
                } else {
                    p->angle = check_float(L,-1);
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
                    p->u1 = uvr.u1;
                    p->v1 = uvr.v1;
                    p->u2 = uvr.u2;
                    p->v2 = uvr.v2;
                }
                lua_pop(L,1);

                bool has_uvs = false;
                lua_getfield(L, -1, "uvs");
                if (has_frame || lua_isnil(L,-1)) {
                } else if (!lua_istable(L,-1)) {
                    CERR << "Particle uvs was not a table." << std::endl;
                    destroy = true;
                } else if (lua_objlen(L,-1) != 4) {
                    CERR << "Particle uvs did not have 4 elements." << std::endl;
                    destroy = true;
                } else {
                    has_uvs = true;
                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1)) {
                        CERR << "Texture ordinate u1 was not a number." << std::endl;
                        destroy = true;
                    }
                    p->u1 = lua_tonumber(L,-1);
                    lua_pop(L,1);

                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1)) {
                        CERR << "Texture ordinate v1 was not a number." << std::endl;
                        destroy = true;
                    }
                    p->v1 = lua_tonumber(L,-1);
                    lua_pop(L,1);


                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1)) {
                        CERR << "Texture ordinate u2 was not a number." << std::endl;
                        destroy = true;
                    }
                    p->u2 = lua_tonumber(L,-1);
                    lua_pop(L,1);

                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1)) {
                        CERR << "Texture ordinate v2 was not a number." << std::endl;
                        destroy = true;
                    }
                    p->v2 = lua_tonumber(L,-1);
                    lua_pop(L,1);

                }
                lua_pop(L,1);
                if (!has_frame && !has_uvs) {
                    p->setDefaultUV();
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
        table.destroy(L);
    }

}

int global_gfx_particle_define (lua_State *L)
{
TRY_START
    check_args(L,2);
    std::string name = check_path(L,1);
    if (!lua_istable(L,2)) my_lua_error(L,"Parameter 2 must be a table.");

    lua_getfield(L, 2, "map");
    if (lua_type(L,-1) != LUA_TSTRING) my_lua_error(L,"Particle map must be a string.");
    std::string texture = lua_tostring(L,-1);
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "map");

    lua_getfield(L, 2, "alphaBlend");
    bool alpha_blend;
    if (lua_isnil(L,-1)) {
        alpha_blend = false;
    } else if (!lua_isboolean(L,-1)) {
        my_lua_error(L,"Particle alphaBlend must be a boolean.");
    } else {
        alpha_blend = lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "alphaBlend");

    float alpha_rej;
    lua_getfield(L, 2, "alphaReject");
    if (lua_isnil(L,-1)) {
        alpha_rej = alpha_blend ? 0 : 127;
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
    
    ParticleDefinition *&pd = particle_defs[name];
    if (pd != NULL) {
        pd->destroy(L);
        delete pd;
    }

    ParticleDefinition *newpd = new ParticleDefinition(name, frames, L, 2);
    pd = newpd;
    gfx_particle_define(name, texture, alpha_blend, alpha_rej, emissive); // will invalidate 
    return 0;
TRY_END
}

namespace {
}

int global_gfx_particle_emit (lua_State *L)
{
TRY_START
    check_args(L,3);
    std::string name = check_path(L,1);
    Vector3 pos = check_v3(L,2);
    if (!lua_istable(L,3)) my_lua_error(L,"Parameter 3 must be a table.");

    ParticleDefinition *pd = particle_defs[name];

    if (pd==NULL) {
        my_lua_error(L, "No such particle \""+name+"\"");
    }

    pd->table.dump(L);
    // stack: particle

    push_v3(L, pos);
    lua_setfield(L,-2,"position");
    lua_pushstring(L, name.c_str());
    lua_setfield(L,-2,"name");

    // stack: particle
    for (lua_pushnil(L) ; lua_next(L,3)!=0 ; lua_pop(L,1)) {
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

float particle_step_size = 0.01f;
float particle_step_remainder= 0.0f;

int global_gfx_particle_step_size (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, particle_step_size);
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            if (v<=0) my_lua_error(L, "Step size must not be 0 or below");
            particle_step_size = v;
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: Expected 0 or 1 arguments.");
        return 0; // silence compiler
    }
TRY_END
}

int global_gfx_particle_pump (lua_State *L)
{
TRY_START
    check_args(L,1);
    float elapsed = check_float(L,1);

    elapsed += particle_step_remainder;

    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);
    // stack: err

    while (elapsed > particle_step_size) {
        elapsed -= particle_step_size;
        for (size_t i=0 ; i<particles.size() ; ++i) {
            LuaParticle *lp = particles[i];
            bool destroy = lp->updateGraphics(L, particle_step_size, error_handler);

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
    }
    particle_step_remainder = elapsed;

    lua_pop(L,1);

    return 0;
TRY_END
}

int global_gfx_particle_count (lua_State *L)
{
    check_args(L,0);
        lua_pushnumber(L, particles.size());
        return 1;
}

// }}}




#include <OgreHighLevelGpuProgramManager.h>
#include <OgreTextureManager.h>
#include <OgreSkeletonManager.h>
#include <OgreMeshManager.h>
#include <OgreGpuProgramManager.h>

#include "lua_wrappers_material.h"
#include "lua_wrappers_gpuprog.h"
#include "lua_wrappers_gfx.h"


////////////////////////////////////////////////////////////////////////////////

static int global_make_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);

        Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().create(
                name,
                "GRIT",
                false,
                NULL);
        push(L, new Ogre::MaterialPtr(m),MAT_TAG);
        return 1;
TRY_END
}

static int global_load_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MaterialPtr t = Ogre::MaterialManager::getSingleton()
                                .load(name,"GRIT");
        push(L, new Ogre::MaterialPtr(t),MAT_TAG);
        return 1;
TRY_END
}

static int global_get_all_materials (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MaterialManager::ResourceMapIterator rmi =
                Ogre::MaterialManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::MaterialPtr &mat = rmi.getNext();
                lua_pushnumber(L,counter+1);
                push(L, new Ogre::MaterialPtr(mat), MAT_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MaterialPtr m =
                Ogre::MaterialManager::getSingleton().getByName(name);
        if (m.isNull()) {
                lua_pushnil(L);
                return 1;
        }
        push(L,new Ogre::MaterialPtr(m),MAT_TAG);
        return 1;
TRY_END
}

static int global_get_material_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
       lua_pushnumber(L,Ogre::MaterialManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_materials (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MaterialManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_materials (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MaterialManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MaterialManager::getSingleton().remove(name);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// new material interface

static void process_tex_blend (GfxMaterial *gfxmat, ExternalTable &src, GfxMaterialTextureBlendUnit &dst)
{

        std::string diffuse_map;
        src.get("diffuseMap", diffuse_map, std::string(""));
        dst.setDiffuseMap(diffuse_map);

        std::string normal_map;
        src.get("normalMap", normal_map, std::string(""));
        dst.setNormalMap(normal_map);

        std::string specular_map;
        src.get("glossMap", specular_map, std::string(""));
        dst.setSpecularMap(specular_map);
        if (specular_map.length() > 0)
                gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_MAP);

        lua_Number specular_diffuse_brightness = 0;
        lua_Number specular_diffuse_contrast = 0;
        SharedPtr<ExternalTable> specular_diffuse;
        if (src.get("specularFromDiffuse",specular_diffuse)) {
                specular_diffuse->get(lua_Number(1), specular_diffuse_brightness);
                specular_diffuse->get(lua_Number(2), specular_diffuse_contrast);
                gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_ADJUSTED_DIFFUSE_COLOUR);
        }
        dst.setSpecularDiffuseBrightness(specular_diffuse_brightness);
        dst.setSpecularDiffuseContrast(specular_diffuse_contrast);

        lua_Number texture_animation_x = 0;
        lua_Number texture_animation_y = 0;
        SharedPtr<ExternalTable> texture_animation;
        if (src.get("textureAnimation",texture_animation)) {
                texture_animation->get(lua_Number(1), texture_animation_x);
                texture_animation->get(lua_Number(2), texture_animation_y);
        }
        dst.setTextureAnimationX(texture_animation_x);
        dst.setTextureAnimationY(texture_animation_y);
        
        lua_Number texture_scale_x = 0;
        lua_Number texture_scale_y = 0;
        SharedPtr<ExternalTable> texture_scale;
        if (src.get("textureScale",texture_scale)) {
                texture_scale->get(lua_Number(1), texture_scale_x);
                texture_scale->get(lua_Number(2), texture_scale_y);
        }
        dst.setTextureScaleX(texture_scale_x);
        dst.setTextureScaleY(texture_scale_y);
}

typedef std::map<std::string, ExternalTable> MatMap;
static MatMap mat_map;

static int global_register_material (lua_State *L)
{
TRY_START

        check_args(L,2);
        std::string name = check_path(L,1);
        if (!lua_istable(L,2))
                my_lua_error(L,"Second parameter should be a table");

        //CVERB << name << std::endl;

        ExternalTable &t = mat_map[name];
        t.clear(L);
        t.takeTableFromLuaStack(L,2);

        GFX_MAT_SYNC;
        GfxMaterial *gfxmat = gfx_material_add_or_get(name);

        bool has_alpha;
        lua_Number alpha;
        if (t.has("alpha")) {
                t.get("alpha", has_alpha, true);
                t.get("alpha", alpha, 1.0);
        } else {
                has_alpha = false;
                alpha = 1.0;
        }

        bool depth_write;
        t.get("depthWrite", depth_write, true);
       
        bool cast_shadows;
        t.get("castShadows", cast_shadows, true);
       
        bool stipple;
        t.get("stipple", stipple, true);

        gfxmat->setCastShadows(cast_shadows);
        gfxmat->setStipple(stipple);
       
        gfxmat->setSceneBlend(has_alpha ? depth_write ? GFX_MATERIAL_ALPHA_DEPTH : GFX_MATERIAL_ALPHA : GFX_MATERIAL_OPAQUE);

        gfxmat->regularMat = Ogre::MaterialManager::getSingleton().getByName(name,"GRIT");
        gfxmat->fadingMat = gfxmat->regularMat;
        gfxmat->worldMat = Ogre::MaterialManager::getSingleton().getByName(name+"&","GRIT");
        gfxmat->wireframeMat = Ogre::MaterialManager::getSingleton().getByName(name+"|","GRIT");

        Vector3 emissive_colour;
        t.get("emissiveColour",emissive_colour,Vector3(0,0,0));
        gfxmat->setEmissiveColour(emissive_colour);

        std::string emissive_map;
        t.get("emissiveMap", emissive_map, std::string(""));
        gfxmat->setEmissiveMap(emissive_map);

        std::string paint_map;
        lua_Number paint_colour = 0; // silence compiler
        if (t.get("paintColour", paint_colour)) {
                if (paint_colour == 1) {
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_1);
                } else if (paint_colour == 2) {
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_2);
                } else if (paint_colour == 3) {
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_3);
                } else if (paint_colour == 4) {
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_4);
                } else {
                        CERR << "Unexpected paint_colour: " << paint_colour << std::endl;
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_NONE);
                }
                gfxmat->setPaintMap("");
        } else if (t.get("paintMap", paint_map)) {
                gfxmat->setPaintMap(paint_map);
                gfxmat->setPaintMode(GFX_MATERIAL_PAINT_MAP);
        } else {
                gfxmat->setPaintMode(GFX_MATERIAL_PAINT_NONE);
        }
        bool paint_by_diffuse_alpha;
        t.get("paintByDiffuseAlpha", paint_by_diffuse_alpha, false);
        gfxmat->setPaintByDiffuseAlpha(paint_by_diffuse_alpha);

        gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_NONE);

        SharedPtr<ExternalTable> blend;
        if(t.get("blend", blend)) {
                for (lua_Number i=1 ; i<=4 ; ++i) {
                        unsigned i_ = unsigned(i)-1;
                        SharedPtr<ExternalTable> texBlends;
                        if (!blend->get(i, texBlends)) {
                                gfxmat->setNumTextureBlends(i_);
                                break;
                        }
                        process_tex_blend(gfxmat, *texBlends, gfxmat->texBlends[i_]);
                }
        } else {
                process_tex_blend(gfxmat, t, gfxmat->texBlends[0]);
                gfxmat->setNumTextureBlends(0);
        }

        bool gloss_from_specular_alpha;
        t.get("glossFromSpecularAlpha", gloss_from_specular_alpha, false);
        if (gloss_from_specular_alpha) {
                gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_MAP_WITH_GLOSS);
        }

        bool specular_from_diffuse_alpha;
        t.get("specularFromDiffuseAlpha", specular_from_diffuse_alpha, false);
        if (specular_from_diffuse_alpha) {
                gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_DIFFUSE_ALPHA);
        }

        return 0;
TRY_END
}

static int global_dump_registered_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        std::string name = check_path(L,1);
        if (mat_map.find(name) == mat_map.end())
                my_lua_error(L, "Material does not exist: \""+std::string(name)+"\"");
        ExternalTable &t = mat_map[name];
        t.dump(L);
        return 1;
TRY_END
}

static int global_registered_material_get (lua_State *L)
{
TRY_START
        check_args(L,2);
        std::string name = check_path(L,1);
        const char *key = luaL_checkstring(L,2);
        if (mat_map.find(name) == mat_map.end())
                my_lua_error(L, "Material does not exist: \""+std::string(name)+"\"");
        ExternalTable &t = mat_map[name];
        t.luaGet(L, key);
        return 1;
TRY_END
}

static int global_reprocess_all_registered_materials (lua_State *L)
{
TRY_START
        check_args(L,1);
        if (!lua_isfunction(L,1))
                my_lua_error(L,"Parameter should be a function");
        for (MatMap::iterator i=mat_map.begin(),i_=mat_map.end() ; i!=i_ ; ++i) {
                lua_pushvalue(L,1);
                lua_pushstring(L,i->first.c_str());
                i->second.dump(L);
                lua_call(L,2,0);
        }
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// sky material interface

static GfxSkyMaterialSceneBlend skymat_scene_blend_from_string (lua_State *L, const std::string& factor)
{
        if (factor=="OPAQUE") {
                return GFX_SKY_MATERIAL_OPAQUE;
        } else if (factor=="ALPHA") {
                return GFX_SKY_MATERIAL_ALPHA;
        } else if (factor=="ADD") {
                return GFX_SKY_MATERIAL_ADD;
        } else {
                std::string msg = "Unknown sky material scene blend: ";
                msg += factor;
                my_lua_error(L,msg);
                return GFX_SKY_MATERIAL_OPAQUE; // never happens
        }
}

/*
static void skymat_push_string_from_scene_blend (lua_State *L, GfxSkyMaterialSceneBlend factor)
{
        switch (factor) {
                case GFX_SKY_MATERIAL_OPAQUE:
                lua_pushstring(L,"OPAQUE");
                break;
                case GFX_SKY_MATERIAL_ALPHA:
                lua_pushstring(L,"ALPHA");
                break;
                case GFX_SKY_MATERIAL_ADD:
                lua_pushstring(L,"ADD");
                break;
                default:
                my_lua_error(L,"Unknown sky material blend");
        }
}
*/

static int global_gfx_register_sky_material (lua_State *L)
{
TRY_START

    check_args(L,2);
    std::string name = check_path(L,1);
    if (!lua_istable(L,2))
            my_lua_error(L,"Second parameter should be a table");

    ExternalTable t;
    t.takeTableFromLuaStack(L,2);

    GFX_MAT_SYNC;
    GfxSkyMaterial *gfxskymat = gfx_sky_material_add_or_get(name);


    std::string shader_name;
    t.get("shader", shader_name, std::string("/system/SkyDefault"));
    GfxSkyShader *shader = gfx_sky_shader_get(shader_name);

    std::string scene_blend;
    t.get("sceneBlend", scene_blend, std::string("OPAQUE"));

    GfxSkyMaterialTextureMap textures;
    GfxShaderBindingsPtr bindings = shader->getShader()->makeBindings();

    typedef ExternalTable::KeyIterator KI;
    for (KI i=t.begin(), i_=t.end() ; i!=i_ ; ++i) {
        const std::string &key = i->first; 
        if (key == "shader") continue;
        if (key == "sceneBlend") continue;

        // must be a texture or param

        SharedPtr<ExternalTable> tab;
        if (!t.get(key, tab)) {
            my_lua_error(L, "Uniform \""+key+"\" incorrectly defined");
        }

        std::string uniform_kind;
        if (!tab->get("uniformKind", uniform_kind)) {
            my_lua_error(L, "Uniform \""+key+"\" expected string 'uniformKind' field.");
        }

        if (uniform_kind == "PARAM") {

            std::string value_kind; // type of the actual data (e.g. FLOAT)
            if (!tab->get("valueKind", value_kind)) {
                my_lua_error(L, "Uniform \""+key+"\" expected string 'valueKind' field.");
            }

            if (value_kind=="FLOAT") {
                GfxShaderParam bind;
                std::vector<float> vs;
                for (unsigned i=1 ; true ; ++i) {
                    lua_Number def;
                    if (tab->get(i, def)) {
                        vs.push_back(def);
                    } else break;
                }
                switch (vs.size()) {
                    case 0: my_lua_error(L, "Uniform \"" + key + "\" must have a default value");
                    case 1: bind = GfxShaderParam(vs[0]); break;
                    case 2: bind = GfxShaderParam(Vector2(vs[0], vs[1])); break;
                    case 3: bind = GfxShaderParam(Vector3(vs[0], vs[1], vs[2])); break;
                    case 4: bind = GfxShaderParam(Vector4(vs[0], vs[1], vs[2], vs[3])); break;
                    default: my_lua_error(L, "Uniform \"" + key + "\" unsupported number of default values");
                }
                bindings->setBinding(key, bind);
            } else {
                my_lua_error(L, "Uniform \""+key+"\" unrecognised 'valueKind' field: \""+value_kind+"\"");
            }

        } else if (uniform_kind == "TEXTURE2D") {

            std::string tex_name;
            bool has_tex = tab->get("name", tex_name);
            APP_ASSERT(has_tex);
            auto *tex = dynamic_cast<GfxTextureDiskResource*>(disk_resource_get_or_make(tex_name));
            if (tex == NULL) my_lua_error(L, "Resource is not a texture \""+tex_name+"\"");
            bool clamp = false;
            int anisotropy = 16;
            textures[key] = { tex, clamp, anisotropy };
            CVERB << key << " " << tex_name << std::endl;

        } else {
            my_lua_error(L, "Did not understand 'uniformKind' value \""+uniform_kind+"\"");
        }
    }

    gfxskymat->setShader(shader);
    gfxskymat->setSceneBlend(skymat_scene_blend_from_string(L, scene_blend));
    gfxskymat->setTextures(textures);
    gfxskymat->setBindings(bindings);

    return 0;
TRY_END
}

static std::string default_vertex_code =
"frag.position = mul(global.worldViewProj, Float4(vert.position.xyz, 1));\n"
"frag.position.z = frag.position.w * (1 - 1.0/65536);\n";

static int global_gfx_register_sky_shader (lua_State *L)
{
TRY_START

    check_args(L,2);
    std::string name = check_path(L,1);
    if (!lua_istable(L,2))
        my_lua_error(L,"Second parameter should be a table");

    ExternalTable t;
    t.takeTableFromLuaStack(L,2);

    std::string fragment_code;
    std::string vertex_code;

    t.get("vertexCode", vertex_code, default_vertex_code);
    if (!t.get("fragmentCode", fragment_code))
        my_lua_error(L, "Shader \"" + name + "\" did not specify fragmentCode.");

    GfxShaderParamMap uniforms;

    typedef ExternalTable::KeyIterator KI;
    for (KI i=t.begin(), i_=t.end() ; i!=i_ ; ++i) {
        const std::string &key = i->first; 
        if (key == "variants") continue;
        if (key == "fragmentCode") continue;
        if (key == "vertexCode") continue;

        // must be a texture or param
        GfxShaderParam uniform;

        SharedPtr<ExternalTable> tab;
        if (!t.get(key, tab)) {
            my_lua_error(L, "Uniform \""+key+"\" incorrectly defined");
        }

        std::string uniform_kind;
        if (!tab->get("uniformKind", uniform_kind)) {
            my_lua_error(L, "Uniform \""+key+"\" expected string 'uniformKind' field.");
        }

        if (uniform_kind == "PARAM") {
            std::string value_kind; // type of the actual data (e.g. FLOAT)
            if (!tab->get("valueKind", value_kind)) {
                my_lua_error(L, "Uniform \""+key+"\" expected string 'valueKind' field.");
            }

            if (value_kind=="FLOAT") {
                std::vector<float> vs;
                for (unsigned i=1 ; true ; ++i) {
                    lua_Number def;
                    if (tab->get(i, def)) {
                        vs.push_back(def);
                    } else break;
                }
                switch (vs.size()) {
                    case 0: my_lua_error(L, "Uniform \"" + key + "\" must have a default value");
                    case 1: uniform = GfxShaderParam(vs[0]); break;
                    case 2: uniform = GfxShaderParam(Vector2(vs[0], vs[1])); break;
                    case 3: uniform = GfxShaderParam(Vector3(vs[0], vs[1], vs[2])); break;
                    case 4: uniform = GfxShaderParam(Vector4(vs[0], vs[1], vs[2], vs[3])); break;
                    default: my_lua_error(L, "Uniform \"" + key + "\" unsupported number of default values");
                }
            } else {
                my_lua_error(L, "Uniform \""+key+"\" unrecognised 'valueKind' field: \""+value_kind+"\"");
            }

        } else if (uniform_kind == "TEXTURE2D") {
            Vector3 c;
            tab->get("defaultColour", c, Vector3(1,1,1));

            lua_Number a;
            tab->get("defaultAlpha", a, 1.0);

            uniform = GfxShaderParam(GFX_GSL_FLOAT_TEXTURE2, Vector4(c.x, c.y, c.z, a));

        } else {
            my_lua_error(L, "Did not understand 'uniformKind' value \"" + uniform_kind + "\"");
        }

        uniforms[key] = uniform;
    }


    GfxSkyShader *shader = gfx_sky_shader_add_or_get(name);

    shader->reset(vertex_code, fragment_code, uniforms);

    return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// make

static int global_make_gpuprog (lua_State *L)
{
TRY_START
    check_args(L,3);
    const char *name = luaL_checkstring(L,1);
    const char *language = luaL_checkstring(L,2);
    std::string typestr = luaL_checkstring(L,3);
    Ogre::GpuProgramType type;
    if (typestr=="FRAGMENT") {
        type = Ogre::GPT_FRAGMENT_PROGRAM;
    } else if (typestr=="VERTEX") {
        type = Ogre::GPT_VERTEX_PROGRAM;
    } else if (typestr=="GEOMETRY") {
        type = Ogre::GPT_GEOMETRY_PROGRAM;
    } else {
        my_lua_error(L,"Unrecognised GPU Program type: "+typestr);
    }
    Ogre::HighLevelGpuProgramPtr t = Ogre::HighLevelGpuProgramManager::getSingleton()
                            .createProgram(name,"GRIT", language, type);
    push(L, new Ogre::HighLevelGpuProgramPtr(t),GPUPROG_TAG);
    return 1;
TRY_END
}

static int global_get_all_gpuprogs (lua_State *L)
{
TRY_START
    check_args(L,0);
    Ogre::HighLevelGpuProgramManager::ResourceMapIterator rmi =
            Ogre::HighLevelGpuProgramManager::getSingleton().getResourceIterator();

    // doesn't seem to be possible to find out how many there are in advance
    lua_createtable(L, 0, 0);
    int counter = 0;
    while (rmi.hasMoreElements()) {
        const Ogre::HighLevelGpuProgramPtr &self = rmi.getNext();
        lua_pushnumber(L,counter+1);
        push(L, new Ogre::HighLevelGpuProgramPtr(self), GPUPROG_TAG);
        lua_settable(L,-3);
        counter++;
    }

    return 1;
TRY_END
}


static int global_get_gpuprog (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    Ogre::HighLevelGpuProgramPtr m = Ogre::HighLevelGpuProgramManager::getSingleton().getByName(name);
    if (m.isNull()) {
        lua_pushnil(L);
        return 1;
    }
    push(L,new Ogre::HighLevelGpuProgramPtr(m),GPUPROG_TAG);
    return 1;
TRY_END
}

static int global_get_gpuprog_usage (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_pushnumber(L,Ogre::GpuProgramManager::getSingleton().getMemoryUsage());
    return 1;
TRY_END
}

static int global_unload_all_gpuprogs (lua_State *L)
{
TRY_START
    check_args(L,0);
    Ogre::GpuProgramManager::getSingleton().unloadAll();
    return 0;
TRY_END
}

static int global_unload_unused_gpuprogs (lua_State *L)
{
TRY_START
    check_args(L,0);
    Ogre::GpuProgramManager::getSingleton().unloadUnreferencedResources();
    return 0;
TRY_END
}

static int global_remove_gpuprog (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    Ogre::GpuProgramManager::getSingleton().remove(name);
    return 0;
TRY_END
}

static int global_resource_exists (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    bool b = Ogre::ResourceGroupManager::getSingleton().
            resourceExists("GRIT",name.substr(1));
    lua_pushboolean(L,b);
    return 1;
TRY_END
}

static int global_add_resource_location (lua_State *L)
{
TRY_START
    check_args(L,3);
    const char *resource = luaL_checkstring(L,1);
    const char *ext = luaL_checkstring(L,2);
    bool recursive = check_bool(L,3);
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(resource,ext,"GRIT",recursive);
    return 0;
TRY_END
}

static int global_init_all_resource_groups (lua_State *L)
{
TRY_START
    check_args(L,0);
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    return 0;
TRY_END
}



static int global_gfx_d3d9 (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_pushboolean(L, gfx_d3d9());
    return 1;
TRY_END
}


static const luaL_reg global[] = {

    {"gfx_d3d9",global_gfx_d3d9},

    {"gfx_render",global_gfx_render},
    {"gfx_bake_env_cube",global_gfx_bake_env_cube},
    {"gfx_screenshot",global_gfx_screenshot},
    {"gfx_option",global_gfx_option},
    {"gfx_body_make",global_gfx_body_make},
    {"gfx_instances_make",global_gfx_instances_make},
    {"gfx_ranged_instances_make",global_gfx_ranged_instances_make},
    {"gfx_sky_body_make",global_gfx_sky_body_make},
    {"gfx_light_make",global_gfx_light_make},

    {"gfx_register_sky_material", global_gfx_register_sky_material},
    {"gfx_register_sky_shader", global_gfx_register_sky_shader},

    {"gfx_font_define",global_gfx_font_define},
    {"gfx_font_line_height",global_gfx_font_line_height},
    {"gfx_font_list",global_gfx_font_list},
    {"gfx_font_text_width",global_gfx_font_text_width},

    {"gfx_hud_object_add",global_gfx_hud_object_add},
    {"gfx_hud_text_add",global_gfx_hud_text_add},
    {"gfx_hud_class_add",global_gfx_hud_class_add},
    {"gfx_hud_class_get",global_gfx_hud_class_get},
    {"gfx_hud_class_has",global_gfx_hud_class_has},
    {"gfx_hud_class_all",global_gfx_hud_class_all},
    {"gfx_hud_class_count",global_gfx_hud_class_count},
    {"gfx_hud_ray",global_gfx_hud_ray},

    {"gfx_env_cube",global_gfx_env_cube},
    {"gfx_colour_grade",global_gfx_colour_grade},
    {"gfx_particle_ambient",global_gfx_particle_ambient},
    {"gfx_sunlight_diffuse",global_gfx_sunlight_diffuse},
    {"gfx_sunlight_specular",global_gfx_sunlight_specular},
    {"gfx_sunlight_direction",global_gfx_sunlight_direction},

    {"gfx_fog_colour",global_gfx_fog_colour},
    {"gfx_fog_density",global_gfx_fog_density},
    {"gfx_hell_colour",global_gfx_hell_colour},

    {"gfx_sun_direction",global_gfx_sun_direction},
    {"gfx_sun_colour",global_gfx_sun_colour},
    {"gfx_sun_alpha",global_gfx_sun_alpha},
    {"gfx_sun_size",global_gfx_sun_size},
    {"gfx_sun_falloff_distance",global_gfx_sun_falloff_distance},

    {"gfx_sky_glare_sun_distance",global_gfx_sky_glare_sun_distance},
    {"gfx_sky_glare_horizon_elevation",global_gfx_sky_glare_horizon_elevation},
    {"gfx_sky_divider",global_gfx_sky_divider},
    {"gfx_sky_colour",global_gfx_sky_colour},
    {"gfx_sky_alpha",global_gfx_sky_alpha},
    {"gfx_sky_sun_colour",global_gfx_sky_sun_colour},
    {"gfx_sky_sun_alpha",global_gfx_sky_sun_alpha},
    {"gfx_sky_cloud_colour",global_gfx_sky_cloud_colour},
    {"gfx_sky_cloud_coverage",global_gfx_sky_cloud_coverage},

    {"gfx_global_saturation",global_gfx_global_saturation},
    {"gfx_global_exposure",global_gfx_global_exposure},

    {"gfx_particle_define",global_gfx_particle_define},
    {"gfx_particle_emit",global_gfx_particle_emit},
    {"gfx_particle_pump",global_gfx_particle_pump},
    {"gfx_particle_count",global_gfx_particle_count},
    {"gfx_particle_step_size",global_gfx_particle_step_size},

    {"gfx_last_frame_stats",global_gfx_last_frame_stats},

    {"gfx_colour_grade_look_up", global_gfx_colour_grade_look_up},

    {"gfx_window_active", global_gfx_window_active},
    {"gfx_window_size", global_gfx_window_size},
    {"gfx_window_size_in_scene", global_gfx_window_size_in_scene},
    {"gfx_world_to_screen", global_gfx_world_to_screen},

    {"gfx_gpu_ram_available", global_gfx_gpu_ram_available},
    {"gfx_gpu_ram_used", global_gfx_gpu_ram_used},

    {"RGBtoHSL", global_rgb_to_hsl},
    {"HSLtoRGB", global_hsl_to_rgb},
    {"HSBtoHSL", global_hsv_to_hsl},
    {"HSLtoHSB", global_hsl_to_hsv},
    {"RGBtoHSV", global_rgb_to_hsv},
    {"HSVtoRGB", global_hsv_to_rgb},

    {NULL, NULL}
};

static const luaL_reg global_ogre_debug[] = {

    {"load_material", global_load_material},
    {"get_all_materials",global_get_all_materials},
    {"get_material",global_get_material},
    {"get_material_usage", global_get_material_usage},
    {"unload_all_materials", global_unload_all_materials},
    {"unload_unused_materials", global_unload_unused_materials},
    {"remove_material",global_remove_material},

    {"make_gpuprog", global_make_gpuprog},
    {"get_all_gpuprogs", global_get_all_gpuprogs},
    {"get_gpuprog", global_get_gpuprog},
    {"get_gpuprog_usage", global_get_gpuprog_usage},
    {"unload_all_gpuprogs", global_unload_all_gpuprogs},
    {"unload_unused_gpuprogs", global_unload_unused_gpuprogs},
    {"remove_gpuprog", global_remove_gpuprog},

    {"register_material", global_register_material},
    {"dump_registered_material", global_dump_registered_material},
    {"registered_material_get", global_registered_material_get},
    {"reprocess_all_registered_materials", global_reprocess_all_registered_materials},

    {"Material", global_make_material},

    {"add_resource_location", global_add_resource_location},
    {"initialise_all_resource_groups", global_init_all_resource_groups},
    {"resource_exists", global_resource_exists},

    {NULL, NULL}
};



void gfx_lua_init (lua_State *L)
{
    ADD_MT_MACRO(gfxnode,GFXNODE_TAG);
    ADD_MT_MACRO(gfxbody,GFXBODY_TAG);
    ADD_MT_MACRO(gfxskybody,GFXSKYBODY_TAG);
    ADD_MT_MACRO(gfxlight,GFXLIGHT_TAG);
    ADD_MT_MACRO(gfxinstances,GFXINSTANCES_TAG);
    ADD_MT_MACRO(gfxrangedinstances,GFXRANGEDINSTANCES_TAG);

    ADD_MT_MACRO(gfxhudobj,GFXHUDOBJECT_TAG);
    ADD_MT_MACRO(gfxhudtext,GFXHUDTEXT_TAG);
    ADD_MT_MACRO(gfxhudclass,GFXHUDCLASS_TAG);

    ADD_MT_MACRO(mat,MAT_TAG);
    ADD_MT_MACRO(gpuprog,GPUPROG_TAG);

    register_lua_globals(L, global);
    register_lua_globals(L, global_ogre_debug);

}



// vim: shiftwidth=4:tabstop=4:expandtab
