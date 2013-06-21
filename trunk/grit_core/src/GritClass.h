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
#include "path_util.h"

/** A game object class.  The 'DNA' from which multiple similar game objects
 * can be instantiated.  For example, there could be a class representing a
 * street lamp, complete with data and code to define its appearance and
 * behaviour, and this class would be instantiated everywhere around the map
 * where a streetlamp is needed.  Notionally, the class is like a Lua table,
 * providing a mapping from keys to values.
 *
 * The class has a parent class, which is just a Lua table.  The parent class
 * is used whenever the class itself does not provide a mapping for a given
 * key.  For its own mappings, the class uses an ExternalTable to store data
 * without pressurising the Lua garbage collector.
 */
class GritClass {

    public:

        /** Create a class using class_add, this function is internal. */
        GritClass (lua_State *L, const std::string &name_)
              : name(name_), dir(grit_dirname(name_)), refCount(1)
        {
                int index = lua_gettop(L);
                for (lua_pushnil(L) ; lua_next(L,index)!=0 ; lua_pop(L,1)) {
                        const char *err = table.luaSet(L);
                        if (err) my_lua_error(L, err);
                }
                lua_pop(L,1); // the table we just iterated through
                parentClass.set(L);
        }

        /** The parent class is a table at the top of the Lua stack. */
        void setParent (lua_State *L)
        {
                parentClass.set(L);
        }

        /** Push the parent to the top of the Lua stack. */
        void pushParent (lua_State *L)
        {
                parentClass.push(L);
        }

        /** Look up the given key, pushing the value on the stack, or nil if it
         * is unbound. */
        void get (lua_State *L, const std::string &key)
        {
                const char *err = table.luaGet(L, key);
                if (err) my_lua_error(L,err);
                // check the parent
                if (lua_isnil(L,-1)) {
                        lua_pop(L,1);
                        pushParent(L);
                        lua_getfield(L, -1, key.c_str());
                        lua_replace(L,-2); // pop the parent
                }
        }

        /** Set the given key to the value at the top of the Lua stack. */
        void set (lua_State *L, const std::string &key)
        {
                const char *err = table.luaSet(L, key);
                if (err) my_lua_error(L,err);
        }

        /** Set the key at Lua stack position -2 to the value at Lua stack position -1. */
        void set (lua_State *L)
        {
                const char *err = table.luaSet(L);
                if (err) my_lua_error(L,err);
        }

        /** Push a table to the stack containing the key/value mappings in this class. */
        void dump (lua_State *L)
        {
                table.dump(L);
        }

        /** Indicate that you are using this class.  A class will not be completely destroyed until noone is using it. */
        void acquire ()
        {
                refCount++;
        }

        /** Indicate that you are no-longer using this class (e.g. you are an
         * object that is in the process of being destroyed).  The class may be
         * destroyed if it is no-longer being used by anyone and has been removed via
         * class_del. */
        void release (lua_State *L)
        {
                refCount--;
                if (refCount>0) return;
                table.destroy(L);
                parentClass.setNil(L);
                delete this;
        }

        /** The name of the class, as a Grit path. */
        const std::string name;

        /** The directory part of the class name. */
        const std::string dir;

    protected:

        /** The number of objects using this class, +1 if it has not yet been 'deleted' and thus the system is using it. */
        int refCount;

        /** A reference to the parent class, which is a Lua table and therefore exists on the Lua heap. */
        LuaPtr parentClass;

        /** The class's key/value mappings. */
        ExternalTable table;

};

/** Create a new class or replace the existing class with the given name.  Uses
 * the table at the top of the stack for the class's contents and the next
 * table in the stack as its parent. */
GritClass *class_add (lua_State *L, const std::string& name);

/** Remove the given class from the system.  This meas it can no-longer be
 * found with class_get or class_has and it can no-longer be instantiated into
 * new objects.  Existing objects using it will keep working until they are all
 * dead, at which point any last traces of the class will be deleted. */
void class_del (lua_State *L, GritClass *c);

/** Fetch the class of the given name, or throw an exception. */
GritClass *class_get (const std::string &name);

/** Does the class exist? */
bool class_has (const std::string &name);
        
/** Remove all classes from the system. */
void class_all_del (lua_State *L);

/** Return a list of all classes, via the two iterator 'byref' parameters. */
void class_all (GritClassMap::iterator &begin, GritClassMap::iterator &end);

/** Return the number of classes currently in the system. */
size_t class_count (void);


#endif

// vim: shiftwidth=4:tabstop=4:expandtab
