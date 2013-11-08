/* Copyright (c) David Cunningham and the Grit Game Engine project 2013
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

#include "LuaPtr.h"
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
    return pwd_full(L, check_string(L, stack_index));
}


