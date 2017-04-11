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

#ifndef LuaPtr_h
#define LuaPtr_h

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}

#include <centralised_log.h>

/** A smart pointer that holds a pointer to an object on the lua heap.  The
 * smarts here are to do with ensuring lua GC works correctly, using the Lua
 * registry.
 */
class LuaPtr {

    public:

    /** The pointer initially points to nil */
    LuaPtr (void)
    {
        ptr = LUA_REFNIL;
    }

    bool isNil (void) const { return ptr == LUA_REFNIL; }

    /** Make this pointer point to whatever is at the top of the Lua stack,
     * and pops the stack.
     */
    void set (lua_State *L)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, ptr);
        ptr = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    /** Just like set(L), but does not pop the stack. */
    void setNoPop (lua_State *L, int index=-1)
    {
        lua_pushvalue(L, index);
        set(L);
    }

    /** Leak the Lua object.  It will never be garbage collected.  The one case
     * where this is a good idea is just after lua_close is called.  If something
     * is so fundamental that it can't be removed from the registry before
     * lua_close is called (e.g. because garbage collection metamethods may try to
     * use it during the lua_close call), then one can simply let Lua clean up the
     * metatable.  At this point, we need to not call luaL_ref on the closed state,
     * in fact there is no cleanup to do, so we simply unlink it and walk away.
     * The only real effect of this is disabling the error message in the LuaPtr
     * destructor.
     */
    void leak (void)
    {
        ptr = LUA_REFNIL;
    }

    /** Make this pointer point to nil.
     */
    void setNil (lua_State *L)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, ptr);
        ptr = LUA_REFNIL;
    }

    /** The pointer must be destroyed properly (with a lua_State*) before
     * allowing it do be destructed.  Otherwise the lua memory leaks.
     */
    ~LuaPtr (void)
    {
        if (!isNil())
            CERR << "LuaPtr was not shut down properly (call setNil())." << std::endl;
    }

    /** Push the lua object on the top of the lua stack.
     */
    void push (lua_State *L) const
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ptr);
    }

    protected:

    /** The location of the lua object in the lua registry.
     */
    int ptr;
};

#endif
