#include "lua_wrappers_gpuprog.h"

// GPUPROG ================================================================= {{{

void push_gpuprog (lua_State *L, const Ogre::GpuProgramPtr &self)
{
        if (self.isNull())
                lua_pushnil(L);
        else
                push(L,new Ogre::GpuProgramPtr(self),GPUPROG_TAG);
}

static int gpuprog_load (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::GpuProgramPtr,self,1,GPUPROG_TAG);
        self->load();
        return 0;
TRY_END
}

static int gpuprog_unload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::GpuProgramPtr,self,1,GPUPROG_TAG);
        self->unload();
        return 0;
TRY_END
}

static int gpuprog_reload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::GpuProgramPtr,self,1,GPUPROG_TAG);
        self->reload();
        return 0;
TRY_END
}

static int gpuprog_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::GpuProgramPtr,self,1,GPUPROG_TAG,0);
        if (self==NULL) return 0;
        return 0;
TRY_END
}

static int gpuprog_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::GpuProgramPtr,self,1,GPUPROG_TAG);
        delete &self;
        return 0;
TRY_END
}



TOSTRING_GETNAME_MACRO(gpuprog,Ogre::GpuProgramPtr,->getName(),GPUPROG_TAG)



static int gpuprog_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::GpuProgramPtr,self,1,GPUPROG_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="destroy") {
                push_cfunction(L,gpuprog_destroy);
        } else if (key=="load") {
                push_cfunction(L,gpuprog_load);
        } else if (key=="unload") {
                push_cfunction(L,gpuprog_unload);
        } else if (key=="reload") {
                push_cfunction(L,gpuprog_reload);
        } else if (key=="isLoaded") {
                lua_pushboolean(L,self->isLoaded());
        } else if (key=="isPrepared") {
                lua_pushboolean(L,self->isPrepared());
        } else if (key == "useCount") {
                lua_pushnumber(L,self.useCount());
        } else {
                my_lua_error(L,"Not a readable GPUProg member: "+key);
        }
        return 1;
TRY_END
}

static int gpuprog_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::GpuProgramPtr,self,1,GPUPROG_TAG);
        std::string key = luaL_checkstring(L,2);
        (void)self;
        my_lua_error(L,"Not a writeable GPUProg member: "+key);
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::GpuProgram,gpuprog,GPUPROG_TAG)

MT_MACRO_NEWINDEX(gpuprog);

// }}}

// vim: shiftwidth=8:tabstop=8:expandtab
