#include <OgreCommon.h>

#include "lua_wrappers_common.h"

#define MAT_TAG "Grit/Material"
MT_MACRO_DECLARE(mat);

const char* fog_mode_to_string (lua_State *L, Ogre::FogMode fm);

Ogre::FogMode fog_mode_from_string (lua_State *L, const std::string& fm);

const char * polygon_mode_to_string (lua_State *L, Ogre::PolygonMode pm);

Ogre::PolygonMode polygon_mode_from_string (lua_State *L, const std::string& t);


// vim: shiftwidth=8:tabstop=8:expandtab
