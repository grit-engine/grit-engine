#include <OgreTexture.h>

#include "lua_wrappers_common.h"

#define TEX_TAG "Grit/Texture"
MT_MACRO_DECLARE(tex);

Ogre::TextureType texture_type_from_string (lua_State *L, const std::string& s);


// vim: shiftwidth=8:tabstop=8:expandtab
