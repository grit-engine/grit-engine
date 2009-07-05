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
void my_lua_error(lua_State *l, const std::string &msg, unsigned long level)
NORETURN;

void check_args(lua_State *l, int expected);
void check_args_min(lua_State *l, int expected);
void check_args_max(lua_State *l, int expected);

bool has_tag(lua_State *l, int index, const char* tag);

lua_Number check_int (lua_State *l, int stack_index,
                      lua_Number min, lua_Number max);

template <typename T>
T check_t (lua_State *l, int stack_index)
{
        return (T) check_int(l, stack_index,
                             std::numeric_limits<T>::min(),
                             std::numeric_limits<T>::max());
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
