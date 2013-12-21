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

#include <string>

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}

#include <OgreException.h>

#include "CentralisedLog.h"
#include <lua_util.h>
#include <lua_wrappers_common.h>


#define TRY_START try {
#define TRY_END } catch (Ogre::Exception &e) { \
        std::string msg = e.getFullDescription(); \
        my_lua_error(L,msg); \
        return 0; \
} catch (Exception &e) { \
        my_lua_error(L,e.msg); \
        return 0; \
}

std::string check_path (lua_State *l, int stack_index);

void push_cfunction (lua_State *L, int (*func)(lua_State*));
void func_map_leak_all (void);

int my_lua_error_handler (lua_State *l);

int my_lua_error_handler (lua_State *l, lua_State *coro, int levelhack);
