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

#include <OgreSubMesh.h>
#include <OgreMaterialManager.h>

#include "lua_wrappers_mesh.h"
#include "lua_wrappers_material.h"

// SKELETON ================================================================ {{{

void push_skel (lua_State *L, const Ogre::SkeletonPtr &self)
{
        if (self.isNull())
                lua_pushnil(L);
        else
                push(L,new Ogre::SkeletonPtr(self),SKEL_TAG);
}

static int skel_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::SkeletonPtr,self,1,SKEL_TAG,0);
        if (self==NULL) return 0;
        delete self;
        return 0;
TRY_END
}

static int skel_load (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SkeletonPtr,self,1,SKEL_TAG);
        self->load();
        return 0;
TRY_END
}

static int skel_unload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SkeletonPtr,self,1,SKEL_TAG);
        self->unload();
        return 0;
TRY_END
}

static int skel_reload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SkeletonPtr,self,1,SKEL_TAG);
        self->reload();
        return 0;
TRY_END
}

TOSTRING_GETNAME_MACRO(skel,Ogre::SkeletonPtr,->getName(),SKEL_TAG)


static int skel_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SkeletonPtr,self,1,SKEL_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="load") {
                push_cfunction(L,skel_load);
        } else if (key=="unload") {
                push_cfunction(L,skel_unload);
        } else if (key=="reload") {
                push_cfunction(L,skel_reload);
        } else if (key=="isLoaded") {
                lua_pushboolean(L,self->isLoaded());
        } else if (key=="isPrepared") {
                lua_pushboolean(L,self->isPrepared());
        } else if (key == "useCount") {
                lua_pushnumber(L,self.useCount());
        } else {
                my_lua_error(L,"Not a valid Skeleton member: "+key);
        }
        return 1;
TRY_END
}

static int skel_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::SkeletonPtr,self,1,SKEL_TAG);
        std::string key = luaL_checkstring(L,2);
        if (false) {
                (void) self;
        } else {
                my_lua_error(L,"Not a valid Skeleton member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::SkeletonPtr,skel,SKEL_TAG)

MT_MACRO_NEWINDEX(skel);

// }}}



// MESH ==================================================================== {{{

void push_mesh (lua_State *L, const Ogre::MeshPtr &self)
{
        if (self.isNull())
                lua_pushnil(L);
        else
                push(L,new Ogre::MeshPtr(self),MESH_TAG);
}

static int mesh_load (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        self->load();
        return 0;
TRY_END
}

static int mesh_unload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        self->unload();
        return 0;
TRY_END
}

static int mesh_reload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        self->reload();
        return 0;
TRY_END
}

static int mesh_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::MeshPtr,self,1,MESH_TAG,0);
        if (self==NULL) return 0;
        delete self;
        return 0;
TRY_END
}

static int mesh_get_material (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        unsigned int n = check_t<unsigned int>(L,2);
        Ogre::SubMesh *sm = self->getSubMesh(n);
        std::string name = sm->getMaterialName();
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

static int mesh_get_material_name (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        unsigned int n = check_t<unsigned int>(L,2);
        Ogre::SubMesh *sm = self->getSubMesh(n);
        std::string name = sm->getMaterialName();
        lua_pushstring(L, name.c_str());
        return 1;
TRY_END
}

static int mesh_set_material_name (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        unsigned int n = check_t<unsigned int>(L,2);
        std::string name = luaL_checkstring(L,3);
        Ogre::SubMesh *sm = self->getSubMesh(n);
        sm->setMaterialName(name);
        return 0;
TRY_END
}

static int mesh_set_bounds (lua_State *L)
{
TRY_START
        check_args(L,7);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        GET_V3(min,2);
        GET_V3(max,5);
        Ogre::AxisAlignedBox box(min.x, min.y, min.z, max.x, max.y, max.z);
        self->_setBounds(box);
        return 0;
TRY_END
}




TOSTRING_GETNAME_MACRO(mesh,Ogre::MeshPtr,->getName(),MESH_TAG)



static int mesh_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="load") {
                push_cfunction(L,mesh_load);
        } else if (key=="unload") {
                push_cfunction(L,mesh_unload);
        } else if (key=="reload") {
                push_cfunction(L,mesh_reload);
        } else if (key=="isLoaded") {
                lua_pushboolean(L,self->isLoaded());
        } else if (key=="isPrepared") {
                lua_pushboolean(L,self->isPrepared());
        } else if (key == "useCount") {
                lua_pushnumber(L,self.useCount());
        } else if (key == "numSubMeshes") {
                lua_pushnumber(L,self->getNumSubMeshes());
        } else if (key == "hasSkeleton") {
                lua_pushboolean(L,self->hasSkeleton());
        } else if (key == "skeleton") {
                push_skel(L,self->getSkeleton());
        } else if (key == "skeletonName") {
                lua_pushstring(L,self->getSkeletonName().c_str());
        } else if (key == "getMaterialName") {
                push_cfunction(L,mesh_get_material_name);
        } else if (key == "setMaterialName") {
                push_cfunction(L,mesh_set_material_name);
        } else if (key == "setBounds") {
                push_cfunction(L,mesh_set_bounds);
        } else if (key == "getMaterial") {
                push_cfunction(L,mesh_get_material);
        } else {
                my_lua_error(L,"Not a valid Mesh member: "+key);
        }
        return 1;
TRY_END
}

static int mesh_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="skeletonName") {
                std::string name = luaL_checkstring(L,3);
                self->setSkeletonName(name);
        } else {
                my_lua_error(L,"Not a valid Mesh member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::Mesh,mesh,MESH_TAG)

MT_MACRO_NEWINDEX(mesh);

// }}}

// vim: shiftwidth=8:tabstop=8:expandtab
