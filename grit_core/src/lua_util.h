#ifndef lua_util_h_
#define lua_util_h_

#include <string>
#include <limits>
#include <vector>

extern "C" {
#include "lua.h"
#include <lauxlib.h>
#include <lualib.h>
}

#include "CentralisedLog.h"


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

void my_lua_error(lua_State *l, const std::string &msg) NORETURN;
void my_lua_error(lua_State *l, const std::string &msg, unsigned long level) NORETURN;
void my_lua_error(lua_State *l, const char *) NORETURN;
void my_lua_error(lua_State *l, const char *, unsigned long level) NORETURN;

void check_args(lua_State *l, int expected);
void check_args_min(lua_State *l, int expected);
void check_args_max(lua_State *l, int expected);

bool check_bool (lua_State *l, int stack_index);

bool has_tag(lua_State *l, int index, const char* tag);

lua_Number check_int (lua_State *l, int stack_index,
                      lua_Number min, lua_Number max);

template <typename T>
T check_t (lua_State *l, int stack_index,
           T min=std::numeric_limits<T>::min(),
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

inline Ogre::Real table_fetch_real (lua_State *L, const char *f, Ogre::Real def)
{
        Ogre::Real r;
        lua_getfield(L,-1,f);
        if (lua_isnil(L,-1)) {
                r = def;
        } else if (lua_type(L,-1)==LUA_TNUMBER) {
                r = lua_tonumber(L,-1);
        } else {
                my_lua_error(L, std::string(f)+" should be a number.");
        }
        lua_pop(L,1);
        return r;
}

int my_lua_error_handler(lua_State *l, lua_State *coro, int levelhack);
int my_lua_error_handler(lua_State *l);

void lua_alloc_stats_get (size_t &counter, size_t &mallocs,
                          size_t &reallocs, size_t &frees);
        
void lua_alloc_stats_set (size_t mallocs, size_t reallocs, size_t frees);

void *lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize);

void check_stack (lua_State *l, int size);

bool is_userdata (lua_State *L, int ud, const char *tname);

void push_cfunction (lua_State *L, int (*func)(lua_State*));


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
