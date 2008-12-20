#include "lua_wrappers_common.h"

#define STRINGDB_TAG "Grit/StringDB"
MT_MACRO_DECLARE(stringdb);
int stringdb_make(lua_State *L);

#define SPLINE_TAG "Grit/Spline"
MT_MACRO_DECLARE(spline);
int spline_make(lua_State *L);

#define VECTOR3_TAG "Grit/Vector3"
MT_MACRO_DECLARE(vector3);
int vector3_make(lua_State *L);

#define QUAT_TAG "Grit/Quaternion"
MT_MACRO_DECLARE(quat);
int quat_make(lua_State *L);

#define TIMER_TAG "Grit/Timer"
MT_MACRO_DECLARE(timer);
int timer_make(lua_State *L);


// vim: shiftwidth=8:tabstop=8:expandtab
