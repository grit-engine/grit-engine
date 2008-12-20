#include "lua_wrappers_common.h"

#define GOM_TAG "Grit/ObjectManager"
MT_MACRO_DECLARE(gom);
void push_gom (lua_State *L, GritObjectManager *self);


#define GRITOBJ_TAG "Grit/Object"
MT_MACRO_DECLARE(gritobj);
void push_gritobj (lua_State *L, const GritObjectPtr &self);



// vim: shiftwidth=8:tabstop=8:expandtab
