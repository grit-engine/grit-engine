#include <OgrePixelFormat.h>

#include "lua_wrappers_common.h"

#define SCNMGR_TAG "Grit/SceneManager"
MT_MACRO_DECLARE(scnmgr);
void push_scnmgr (lua_State *L, Ogre::SceneManager *scnmgr);


#define NODE_TAG "Grit/SceneNode"
MT_MACRO_DECLARE(node);
void push_node (lua_State *L, Ogre::SceneNode *n);

Ogre::PixelFormat pixel_format_from_string (lua_State *L, const std::string& s);
const char *pixel_format_to_string (lua_State *L, Ogre::PixelFormat pf);

// vim: shiftwidth=8:tabstop=8:expandtab
