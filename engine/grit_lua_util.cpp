/* Copyright (c) The Grit Game Engine authors 2016
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

#include <cstdlib>

#include <map>

#include <io_util.h>
#include <lua_stack.h>

#include "lua_ptr.h"
#include "grit_lua_util.h"
#include "path_util.h"

typedef std::map<int (*)(lua_State*),LuaPtr> FuncMap;
static FuncMap func_map;
void push_cfunction (lua_State *L, int (*func)(lua_State*))
{
    if (func_map.find(func)==func_map.end()) {
        lua_pushcfunction(L, func);
        func_map[func].setNoPop(L);
    } else {
        func_map[func].push(L);
    }
}

void func_map_leak_all (void)
{
    for (FuncMap::iterator i=func_map.begin(), i_=func_map.end() ; i!=i_ ; ++i) {
        i->second.leak();
    }
}


std::string check_path (lua_State *L, int stack_index)
{
    std::string abs = check_string(L, stack_index);
    if (abs.length() == 0) {
        EXCEPT << "Path was a zero length string." << ENDL;
    }
    if (abs[0] != '/') {
        EXCEPT << "Absolute path expected: \"" << abs << "\"" << ENDL;
    }
    return abs;
}


int my_lua_error_handler (lua_State *l)
{
    return my_lua_error_handler(l,l,1);
}

int my_lua_error_handler (lua_State *l, lua_State *coro, int levelhack)
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
    
    std::string str = check_string(l,-1);

    std::vector<struct stack_frame> tb = traceback(coro,level);

    if (tb.size()==0) {
        CERR<<"getting traceback: ERROR LEVEL TOO HIGH!"<<std::endl;
        level=0;
        tb = traceback(coro,level);
    }   
        
    if (tb.size()==0) {
        CERR<<"getting traceback: EVEN ZERO TOO HIGH!"<<std::endl;
        return 1;
    }   
        
    // strip file:line from message if it is there
    std::stringstream ss; ss<<tb[0].file<<":"<<tb[0].line<<": ";
    std::string str_prefix1 = ss.str();
    std::string str_prefix2 = str.substr(0,str_prefix1.size());
    if (str_prefix1==str_prefix2)
        str = str.substr(str_prefix1.size());
    
    CLOG << BOLD << RED << tb[0].file;
    int line = tb[0].line;
    if (line > 0) { 
        CLOG << ":" << line;
    }
    CLOG << ": " << str << RESET << std::endl;
    for (size_t i=1 ; i<tb.size() ; i++) {
        if (tb[i].gap) {
            CLOG << "\t..." << RESET << std::endl;
        } else {
            CLOG << RED << "\t" << tb[i].file;
            int line = tb[i].line;
            if (line > 0) {
                CLOG << ":" << line;
            }
            CLOG << ": " << tb[i].func_name << RESET << std::endl;
        }
    }
    return 1;
}

