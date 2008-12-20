#include "lua_wrappers_common.h"

#include "PhysicsWorld.h"

#define PWORLD_TAG "Grit/PhysicsWorld"
MT_MACRO_DECLARE(pworld);
int pworld_make(lua_State *L);
void push_pworld (lua_State *L, const PhysicsWorldPtr &self);


#define COLMESH_TAG "Grit/CollisionMesh"
MT_MACRO_DECLARE(colmesh);

#define RBODY_TAG "Grit/RigidBody"
MT_MACRO_DECLARE(rbody);
void push_rbody (lua_State *L, const RigidBodyPtr &self);



// vim: shiftwidth=8:tabstop=8:expandtab
