#include "lua_wrappers_common.h"

#include "Streamer.h"

#define STREAMER_TAG "Grit/Streamer"
MT_MACRO_DECLARE(streamer);
void push_streamer (lua_State *L, Streamer *self);


#define GRITCLS_TAG "Grit/Class"
MT_MACRO_DECLARE(gritcls);
void push_gritcls (lua_State *L, GritClass *self);


#define GRITOBJ_TAG "Grit/Object"
MT_MACRO_DECLARE(gritobj);
void push_gritobj (lua_State *L, const GritObjectPtr &self);



// vim: shiftwidth=8:tabstop=8:expandtab
