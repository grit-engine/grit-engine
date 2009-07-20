#include <OgreTexture.h>
#include <OgreHardwarePixelBuffer.h>

#include "lua_wrappers_tex.h"
#include "lua_wrappers_render.h"

static int tex_load (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::TexturePtr,self,1,TEX_TAG);
        self->load();
        return 0;
TRY_END
}

static int tex_unload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::TexturePtr,self,1,TEX_TAG);
        self->unload();
        return 0;
TRY_END
}

static int tex_reload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::TexturePtr,self,1,TEX_TAG);
        self->reload();
        return 0;
TRY_END
}


TOSTRING_GETNAME_MACRO(tex,Ogre::TexturePtr,->getName(),TEX_TAG)

Ogre::TextureType texture_type_from_string (lua_State *L, const std::string& s)
{
        if (s=="1D") {
                return Ogre::TEX_TYPE_1D;
        } else if (s=="2D") {
                return Ogre::TEX_TYPE_2D;
        } else if (s=="3D") {
                return Ogre::TEX_TYPE_3D;
        } else if (s=="CUBE_MAP") {
                return Ogre::TEX_TYPE_CUBE_MAP;
        } else {
                my_lua_error(L,"Unrecognised texture type: "+s);
                return Ogre::TEX_TYPE_CUBE_MAP; // never happens
        }
}

static const char *texture_type_to_string (lua_State *L, Ogre::TextureType t)
{
        switch (t) {
                case Ogre::TEX_TYPE_1D: return "1D";
                case Ogre::TEX_TYPE_2D: return "2D";
                case Ogre::TEX_TYPE_3D: return "3D";
                case Ogre::TEX_TYPE_CUBE_MAP: return "CUBE_MAP";
                default: my_lua_error(L,"Unknown texture type");
        }
        return "never happens";
}

static int tex_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::TexturePtr,self,1,TEX_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="load") {
                push_cfunction(L,tex_load);
        } else if (key=="unload") {
                push_cfunction(L,tex_unload);
        } else if (key=="reload") {
                push_cfunction(L,tex_reload);
        } else if (key=="isLoaded") {
                lua_pushboolean(L,self->isLoaded());
        } else if (key=="isPrepared") {
                lua_pushboolean(L,self->isPrepared());
        } else if (key=="renderTexture") {
                Ogre::RenderTexture *rt = self->getBuffer()->getRenderTarget();
                push_rtex(L,rt);
        } else if (key=="name") {
                lua_pushstring(L, self->getName().c_str());
        } else if (key=="type") {
                lua_pushstring(L,
                              texture_type_to_string(L,self->getTextureType()));
        } else if (key=="mipmapsHardwareGenerated") {
                lua_pushboolean(L,self->getMipmapsHardwareGenerated());
        } else if (key=="numMipmaps") {
                lua_pushnumber(L,self->getNumMipmaps());
        } else if (key=="height") {
                lua_pushnumber(L,self->getHeight());
        } else if (key=="width") {
                lua_pushnumber(L,self->getWidth());
        } else if (key=="depth") {
                lua_pushnumber(L,self->getDepth());
        } else if (key=="hasAlpha") {
                lua_pushboolean(L,self->hasAlpha());
        } else if (key=="size") {
                lua_pushnumber(L,self->getSize());
        } else {
                my_lua_error(L,"Not a valid Texture member: "+key);
        }
        return 1;
TRY_END
}

static int tex_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::TexturePtr,self,1,TEX_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="numMipmaps") {
                size_t n = check_t<size_t>(L,3);
                self->setNumMipmaps(n);
        } else {
                my_lua_error(L,"Not a valid Texture member: "+key);
        }
        return 0;
TRY_END
}

GC_MACRO(Ogre::TexturePtr,tex,TEX_TAG)

EQ_MACRO(Ogre::TexturePtr,tex,TEX_TAG)

MT_MACRO_NEWINDEX(tex);

// vim: shiftwidth=8:tabstop=8:expandtab
