#include <cstdlib>
#include <cmath>

#include <map>
#include <sstream>

#include "lua_util.h"


// code nicked from ldblib.c
struct stack_frame {
        std::string file;
        int line;
        std::string func_name;
        int gap;
};

static std::vector<struct stack_frame> traceback(lua_State *L1, int level)
{
        std::vector<struct stack_frame> r;
        lua_Debug ar;

        int top = 7;
        int bottom = 3;

        int i=0;
        while (lua_getstack(L1, level+(i++), &ar)) {
                if (i==top+1) {
                        if (lua_getstack(L1, level+i+bottom, &ar)) {
                                struct stack_frame sf;
                                sf.gap = 1;
                                r.push_back(sf);
                                // skip a load of frames because the stack
                                // is too big
                                while (lua_getstack(L1, level+i+bottom, &ar))
                                        i++;
                                continue;
                        }
                }

                lua_getinfo(L1, "Snl", &ar);

                struct stack_frame sf;

                sf.file = ar.short_src;

                sf.line = ar.currentline;

                if (*ar.namewhat != '\0') {       /* is there a name? */
                        sf.func_name = ar.name;
                } else {
                        if (*ar.what == 'm')
                                sf.func_name = "global scope";
                        else if (*ar.what == 'C')
                                sf.func_name = "C function";
                        else if (*ar.what == 't')
                                sf.func_name = "Tail call";
                        else {
                                std::stringstream ss;
                                ss << "func <"<<ar.short_src
                                   << ":"<<ar.linedefined << ">";
                                sf.func_name = ss.str();
                        }
                }
                sf.gap = 0;
                r.push_back(sf);
        }

        return r;
}

void my_lua_error(lua_State *l, const std::string &msg)
{
        // default value of 1 because this function is called from within c
        // code implementing lua functions that just need to strip themselves
        // from the traceback
        my_lua_error(l,msg,1);
}

void my_lua_error(lua_State *l, const std::string &msg, unsigned long level)
{
        luaL_where(l,level);
        std::string str = luaL_checkstring(l,-1);
        lua_pop(l,1);
        str += msg;
        lua_newtable(l);
        lua_pushnumber(l,level);
        lua_rawseti(l,-2,1);
        lua_pushstring(l,str.c_str());
        lua_rawseti(l,-2,2);
        lua_error(l);
        abort(); // never happens, keeps compiler happy
}

void check_args_max(lua_State *l, int expected)
{
        int got = lua_gettop(l);
        if (got>expected) {
                std::stringstream msg;
                msg << "Wrong number of arguments: " << got
                    << " should be at most " << expected;
                my_lua_error(l,msg.str());
        }
}

void check_args_min(lua_State *l, int expected)
{
        int got = lua_gettop(l);
        if (got<expected) {
                std::stringstream msg;
                msg << "Wrong number of arguments: " << got
                    << " should be at least " << expected;
                my_lua_error(l,msg.str());
        }
}

void check_args(lua_State *l, int expected)
{
        int got = lua_gettop(l);
        if (got!=expected) {
                std::stringstream msg;
                msg << "Wrong number of arguments: " << got
                    << " should be " << expected;
                my_lua_error(l,msg.str());
        }
}

lua_Number check_int (lua_State *l, int stack_index,
                      lua_Number min, lua_Number max)
{
        lua_Number n = luaL_checknumber(l, stack_index);
        if (n<min || n>max || n!=floor(n)) {
                std::stringstream msg;
                msg << "Not an integer ["<<min<<","<<max<<"]: " << n;
                my_lua_error(l,msg.str());
        }
        return n;
}

int my_lua_error_handler(lua_State *l)
{
        return my_lua_error_handler(l,l,1);
}

int my_lua_error_handler(lua_State *l, lua_State *coro, int levelhack)
{
        //check_args(l,1);
        int level = 0;
        if (lua_type(l,-1)==LUA_TTABLE) {
                lua_rawgeti(l,-1,1);
                level = luaL_checkinteger(l,-1);
                lua_pop(l,1);
                lua_rawgeti(l,-1,2);
        }
        level+=levelhack; // to remove the current function as well

        std::string str = luaL_checkstring(l,-1);

        std::vector<struct stack_frame> tb = traceback(coro,level);

        if (tb.size()==0) {
                APP_ERROR("getting traceback","ERROR LEVEL TOO HIGH!");
                level=0;
                tb = traceback(coro,level);
        }

        if (tb.size()==0) {
                APP_ERROR("getting traceback","EVEN ZERO TOO HIGH!");
                return 1;
        }

        std::stringstream ss; ss<<tb[0].file<<":"<<tb[0].line<<": ";
        std::string str_prefix1 = ss.str();
        std::string str_prefix2 = str.substr(0,str_prefix1.size());
        if (str_prefix1==str_prefix2)
                str = str.substr(str_prefix1.size());

        app_error(tb[0].file.c_str(), tb[0].line,
                  tb[0].func_name.c_str(),
                  str.c_str());
        for (size_t i=1 ; i<tb.size() ; i++) {
                if (tb[i].gap) {
                        app_line("...");
                } else {
                        std::stringstream ss;
                        ss << tb[i].file << ":" << tb[i].line << ": "
                           << tb[i].func_name;
                        app_line(ss.str().c_str());
                }
        }
        return 1;
}

size_t lua_alloc_stats_mallocs = 0;
size_t lua_alloc_stats_reallocs = 0;
size_t lua_alloc_stats_frees = 0;
size_t lua_alloc_stats_counter = 0;

void lua_alloc_stats_get (size_t &counter, size_t &mallocs,
                          size_t &reallocs, size_t &frees)
{
        counter = lua_alloc_stats_counter;
        mallocs = lua_alloc_stats_mallocs;
        reallocs = lua_alloc_stats_reallocs;
        frees = lua_alloc_stats_frees;
}

void lua_alloc_stats_reset ()
{
        lua_alloc_stats_mallocs = 0;
        lua_alloc_stats_reallocs = 0;
        lua_alloc_stats_frees = 0;
}

void *lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize)
{
        (void) ud;
        (void) osize;
        if (nsize==0) {
                if (ptr!=NULL) {
                        lua_alloc_stats_frees++;
                        lua_alloc_stats_counter--;
                        free(ptr);
                }
                return NULL;
        } else {
                if (ptr==NULL) {
                        lua_alloc_stats_mallocs++;
                        lua_alloc_stats_counter++;
                        return malloc(nsize);
                } else {
                        lua_alloc_stats_reallocs++;
                        return realloc(ptr,nsize);
                }
        }
}

void check_stack (lua_State *l, int size)
{
        if (!lua_checkstack(l,size)) {
                std::stringstream ss;
                ss << "Failed to guarantee " << size <<", current top is "
                   << lua_gettop(l);
                APP_ERROR("lua_checkstack",ss.str());
        }
}

bool has_tag(lua_State *l, int index, const char* tag)
{
        if (!lua_getmetatable(l,index)) return false;
        lua_getfield(l,LUA_REGISTRYINDEX,tag);
        bool ret = lua_equal(l,-1,-2)!=0;
        lua_pop(l,2);
        return ret;
}

// this version silently ignores nullified userdata
bool is_userdata (lua_State *L, int ud, const char *tname)
{ 
        void *p = lua_touserdata(L, ud); 
        if (p==NULL) return false;
        return has_tag(L,ud,tname);
} 


typedef std::map<int (*)(lua_State*),int> FuncMap;
FuncMap func_map;
void push_cfunction (lua_State *L, int (*func)(lua_State*))
{
        if (func_map.find(func)==func_map.end()) {
                lua_pushcfunction(L, func);
                lua_pushvalue(L,-1);
                func_map[func] = luaL_ref(L, LUA_REGISTRYINDEX);
        } else {
                lua_rawgeti(L,LUA_REGISTRYINDEX,func_map[func]);
        }
}


// vim: shiftwidth=8:tabstop=8:expandtab
