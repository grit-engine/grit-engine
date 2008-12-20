#include <OgreMesh.h>

#include "lua_wrappers_common.h"


#define MESH_TAG "Grit/Mesh"
MT_MACRO_DECLARE(mesh);

void push_mesh (lua_State *L, const Ogre::MeshPtr &self);


// vim: shiftwidth=8:tabstop=8:expandtab
