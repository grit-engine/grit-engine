#include "lua_wrappers_common.h"

#define SCNMGR_TAG "Grit/SceneManager"
MT_MACRO_DECLARE(scnmgr);
void push_scnmgr (lua_State *L, Ogre::SceneManager *scnmgr);


#define NODE_TAG "Grit/SceneNode"
MT_MACRO_DECLARE(node);
void push_node (lua_State *L, Ogre::SceneNode *n);


// vim: shiftwidth=8:tabstop=8:expandtab
