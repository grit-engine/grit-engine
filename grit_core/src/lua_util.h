/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef lua_util_h_
#define lua_util_h_

#include <string>
#include <limits>
#include <vector>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "CentralisedLog.h"
#include "math_util.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef __GNUC__
#define NORETURN __attribute__ ((noreturn))
#else
#define NORETURN
#endif

#ifdef NO_LUA_STACK_CHECKS
#define STACK_BASE_(name)
#define STACK_CHECK_N_(name,N)
#else
#define STACK_BASE_(name) int name = lua_gettop(L)
#define STACK_CHECK_N_(name,N) APP_ASSERT(lua_gettop(L)==name+N)
#endif

#define STACK_BASE STACK_BASE_(____stack)
#define STACK_CHECK_N(N) STACK_CHECK_N_(____stack,N)
#define STACK_CHECK_(name) STACK_CHECK_N_(name,0)
#define STACK_CHECK STACK_CHECK_N(0)

#define ADD_MT_MACRO(name,tag) do {\
luaL_newmetatable(L, tag); \
luaL_register(L, NULL, name##_meta_table); \
lua_pop(L,1); } while(0)

static inline Vector2 check_v2 (lua_State *L, int idx)
{ Vector2 v; lua_checkvector2(L, idx, &v.x, &v.y); return v; } 
static inline void push_v2 (lua_State *L, const Vector2 &v)
{ lua_pushvector2(L, v.x, v.y); }

static inline Vector3 check_v3 (lua_State *L, int idx)
{ Vector3 v; lua_checkvector3(L, idx, &v.x, &v.y, &v.z); return v; } 
static inline void push_v3 (lua_State *L, const Vector3 &v)
{ lua_pushvector3(L, v.x, v.y, v.z); }

static inline Quaternion check_quat (lua_State *L, int idx)
{ Quaternion v; lua_checkquat(L, idx, &v.w, &v.x, &v.y, &v.z); return v; } 
static inline void push_quat (lua_State *L, const Quaternion &v)
{ lua_pushquat(L, v.w, v.x, v.y, v.z); }

static inline void push_string (lua_State *L, const std::string &v)
{ lua_pushstring(L, v.c_str()); }

void my_lua_error(lua_State *l, const std::string &msg) NORETURN;
void my_lua_error(lua_State *l, const std::string &msg, unsigned long level) NORETURN;
void my_lua_error(lua_State *l, const char *) NORETURN;
void my_lua_error(lua_State *l, const char *, unsigned long level) NORETURN;

void check_args(lua_State *l, int expected);
void check_args_min(lua_State *l, int expected);
void check_args_max(lua_State *l, int expected);

bool check_bool (lua_State *l, int stack_index);

std::string check_path (lua_State *l, int stack_index);

bool has_tag(lua_State *l, int index, const char* tag);

lua_Number check_int (lua_State *l, int stack_index, lua_Number min, lua_Number max);

float check_float (lua_State *l, int stack_index);
const char* check_string (lua_State *l, int stack_index);

template <typename T>
T check_t (lua_State *l, int stack_index,
           T min = std::numeric_limits<T>::min(),
           T max = std::numeric_limits<T>::max())
{
    return (T) check_int(l, stack_index, min, max);
}

inline bool table_fetch_bool (lua_State *L, const char *f, bool def)
{
    bool r;
    lua_getfield(L,-1,f);
    if (lua_isnil(L,-1)) {
        r = def;
    } else if (lua_type(L,-1)==LUA_TBOOLEAN) {
        r = 0!=lua_toboolean(L,-1);
    } else {
        my_lua_error(L, std::string(f)+" should be a boolean.");
    }
    lua_pop(L,1);
    return r;
}

template<class T> T table_fetch_num (lua_State *L, const char *f, const T &def)
{
    T r;
    lua_getfield(L,-1,f);
    if (lua_isnil(L,-1)) {
        r = def;
    } else if (lua_type(L,-1)==LUA_TNUMBER) {
        r = check_t<T>(L, -1);
    } else {
        my_lua_error(L, std::string(f)+" should be a number.");
    }
    lua_pop(L,1);
    return r;
}

inline float table_fetch_real (lua_State *L, const char *f, float def)
{
    float r;
    lua_getfield(L,-1,f);
    if (lua_isnil(L,-1)) {
        r = def;
    } else if (lua_type(L,-1)==LUA_TNUMBER) {
        r = (float)lua_tonumber(L,-1);
    } else {
        my_lua_error(L, std::string(f)+" should be a number.");
    }
    lua_pop(L,1);
    return r;
}

int my_lua_error_handler(lua_State *l, lua_State *coro, int levelhack);
int my_lua_error_handler(lua_State *l);
int my_do_nothing_lua_error_handler(lua_State *l);

void lua_alloc_stats_get (size_t &counter, size_t &mallocs,
                          size_t &reallocs, size_t &frees);
    
void lua_alloc_stats_set (size_t mallocs, size_t reallocs, size_t frees);

void *lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize);

void check_stack (lua_State *l, int size);

bool is_userdata (lua_State *L, int ud, const char *tname);

void push_cfunction (lua_State *L, int (*func)(lua_State*));

void func_map_shutdown (lua_State *L);

void register_lua_globals (lua_State *L, const luaL_reg *globals);

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
