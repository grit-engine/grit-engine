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

#include <string>
#include <map>

class GritClass;
typedef std::map<std::string,GritClass*> GritClassMap;

#ifndef GritClass_h
#define GritClass_h

#include <string>

extern "C" {
        #include <lua.h>
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "ExternalTable.h"
#include "lua_util.h"
#include "LuaPtr.h"

class GritClass {

    public:

        GritClass (lua_State *L, const std::string &name_)
              : name(name_), refCount(1)
        {
                int index = lua_gettop(L);
                for (lua_pushnil(L) ; lua_next(L,index)!=0 ; lua_pop(L,1)) {
                        const char *err = table.luaSet(L);
                        if (err) my_lua_error(L, err);
                }
                lua_pop(L,1); // the table we just iterated through
                parentClass.set(L);
        }

        void setParent (lua_State *L)
        {
                parentClass.set(L);
        }

        void pushParent (lua_State *L)
        {
                parentClass.push(L);
        }

        void get (lua_State *L, const std::string &key)
        {
                const char *err = table.luaGet(L, key);
                if (err) my_lua_error(L,err);
                if (lua_isnil(L,-1)) {
                        lua_pop(L,1);
                        pushParent(L);
                        lua_getfield(L, -1, key.c_str());
                        lua_replace(L,-2); // pop the parent
                }
        }

        void set (lua_State *L, const std::string &key)
        {
                const char *err = table.luaSet(L, key);
                if (err) my_lua_error(L,err);
        }

        void set (lua_State *L)
        {
                const char *err = table.luaSet(L);
                if (err) my_lua_error(L,err);
        }

        void dump (lua_State *L)
        {
                table.dump(L);
        }

        void acquire ()
        {
                refCount++;
        }

        void release (lua_State *L)
        {
                refCount--;
                if (refCount>0) return;
                table.destroy(L);
                parentClass.setNil(L);
                delete this;
        }

        const std::string name;

    protected:

        int refCount;

        LuaPtr parentClass;

        ExternalTable table;

};

extern GritClassMap classes;

GritClass *class_add (lua_State *L, const std::string& name);
void class_del (lua_State *L, GritClass *c);

GritClass *class_get (const std::string &name);
bool class_has (const std::string &name);
        
void class_all_del (lua_State *L);

void class_all (GritClassMap::iterator &begin, GritClassMap::iterator &end);

size_t class_count (void);


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
