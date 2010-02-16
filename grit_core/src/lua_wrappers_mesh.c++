#include <OgreSubMesh.h>
#include <OgreMaterialManager.h>

#include "lua_wrappers_mesh.h"
#include "lua_wrappers_material.h"

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
        return 0;
TRY_END
}

static int mesh_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        delete &self;
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




TOSTRING_GETNAME_MACRO(mesh,Ogre::MeshPtr,->getName(),MESH_TAG)



static int mesh_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::MeshPtr,self,1,MESH_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="destroy") {
                push_cfunction(L,mesh_destroy);
        } else if (key=="load") {
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
        } else if (key == "getMaterialName") {
                push_cfunction(L,mesh_get_material_name);
        } else if (key == "setMaterialName") {
                push_cfunction(L,mesh_set_material_name);
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
