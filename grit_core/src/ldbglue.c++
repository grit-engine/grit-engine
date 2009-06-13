extern "C" {
#include "lua.h"
}

/* Call ldb as carefully as possible. Returns true if it managed
 * to call ldb and ldb does not return 'false' (nil is ok). An
 * example call would be:
 *
 *    assert(ldb(L, "some message"));
 *
 * ldb() will work as a LUA_CUSTOM_APICHECK reporter function if
 * you use the `apicheck.diff` patch in the ldb distribution. (In
 * fact, that's why I wrote that patch.)
 * 
 * Even if you don't use the custom apicheck feature, I strongly
 * recommend the use of the patch. It will put meaningful error
 * messages into the assert() calls, and also ensures that the
 * lua_State is consistent when the assert() goes off, which
 * allows you to call Lua API functions from a debugger *provided*
 * that you're careful to call lua_checkstack() first.
 *
 * In order to integrate (better) with gdb, we also define the
 * interface
 *     void ldbdo(lua_State *L, const char *cmd);
 * which submits its second argument to ldb with the trigger 'ldb_cmd'
 * unless its second argument is NULL, in which case it just does
 * the same as ldb() without returning a success indication (this
 * avoids adding values to gdb's expression history after a call
 * command.) Obviously other possibilities exist.
 *
 * A sample gdb canned command is in ldb.gdb in the distribution.
 * It lets you use 'ldb' as a command from a gdb session, making
 * use of the ldbdo() interface, such as:
 *
 * ldb print local0
 *
 * You'll have to modify the script if you use something other than `L` as
 * your lua_State variable; then put it in your .gdbinit file.
 */

int ldb (lua_State *L, const char *msg) {
  int rc;
  if (!lua_checkstack(L, 3)) /* 2 of ours + possibly 1 in api_incr_top */
    return 0;
  lua_getfield(L, LUA_REGISTRYINDEX, "ldb");
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return 0;
  }
  if (msg != NULL && msg[0] != 0) {
    lua_pushstring(L, msg);
    lua_call(L, 1, 1);
  }
  else
    lua_call(L, 0, 1);
  rc = lua_isnil(L, -1) || lua_toboolean(L, -1);
  lua_pop(L, 1);
  return rc;
}

void ldbdo (lua_State *L, const char *cmd) {
  if (lua_checkstack(L, 4)) {
    lua_getfield(L, LUA_REGISTRYINDEX, "ldb");
    if (lua_isnil(L, 1)) 
      lua_pop(L, 1);
    else if (cmd == NULL || cmd[0] == 0)
      lua_call(L, 0, 0);
    else {
      lua_pushliteral(L, "ldb_cmd");
      lua_pushstring(L, cmd);
      lua_call(L, 2, 0);
    }
  }
}

int luaopen_ldbglue (lua_State *L) {
  lua_getglobal(L, "require");
  lua_pushstring(L, "ldb");
  lua_call(L, 1, 1);  /* If this fails, we can't go on */
  lua_setfield(L, LUA_REGISTRYINDEX, "ldb");
  return 0;
}

