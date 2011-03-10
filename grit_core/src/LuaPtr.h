/* Copyright (c) David Cunningham and the Grit Game Engine project 2011
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

class LuaPtr {
  public:
    LuaPtr (void)
    {
        ptr = LUA_REFNIL;
    }

    bool isNil() const { return ptr == LUA_REFNIL; }

    void set (lua_State *L)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, ptr);
        ptr = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    void setNoPop (lua_State *L)
    {
        lua_pushvalue(L, -1);
        set(L);
    }

    void setNil (lua_State *L)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, ptr);
        ptr = LUA_REFNIL;
    }

    ~LuaPtr (void)
    {
        if (!isNil())
            CERR << "LuaPtr was not shut down properly (call setNil())." << std::endl;
    }

    void push (lua_State *L) const
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ptr);
    }

  protected:
    lua_Number ptr;
};

#endif
// vim: shiftwidth=4:tabstop=4:expandtab
