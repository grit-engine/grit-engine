/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

#include "ExternalTable.h"
#include "lua_util.h"
#include "lua_wrappers_primitives.h"


const char *ExternalTable::luaGet (lua_State *L)
{
        if (lua_type(L,-1)==LUA_TSTRING) {
                std::string key = luaL_checkstring(L,-1);
                return luaGet(L,key);
        } else if (lua_type(L,-1)==LUA_TNUMBER) {
                lua_Number key = luaL_checknumber(L,-1);
                return luaGet(L,key);
        }
        return "key was not a string or number";
}

static void push (lua_State *L, const ExternalTable::Value &v)
{
        switch (v.type) {
                case 0:
                lua_pushnumber(L,v.real);
                break;
                case 1:
                lua_pushstring(L,v.str.c_str());
                break;
                case 2:
                push(L,new Vector3(v.v3),VECTOR3_TAG);
                break;
                case 3:
                push(L,new Quaternion(v.q),QUAT_TAG);
                break;
                case 4:
                lua_pushboolean(L,v.b);
                break;
                case 5:
                v.t->dump(L);
                break;
        }
}

const char *ExternalTable::luaGet (lua_State *L, const std::string &key)
{
        if (!has(key)) {
                lua_pushnil(L);
        } else {
                push(L, fields[key]);
        }
        return NULL;
}

const char *ExternalTable::luaGet (lua_State *L, lua_Number key)
{
        if (!has(key)) {
                lua_pushnil(L);
        } else {
                push(L, elements[key]);
        }
        return NULL;
}

const char *ExternalTable::luaSet (lua_State *L)
{
        if (lua_type(L,-2)==LUA_TSTRING) {
                std::string key = luaL_checkstring(L,-2);
                return luaSet(L,key);
        } else if (lua_type(L,-2)==LUA_TNUMBER) {
                lua_Number key = luaL_checknumber(L,-2);
                return luaSet(L,key);
        }
        return "key was not a string or number";
}

const char *ExternalTable::luaSet (lua_State *L, const std::string &key)
{
        if (lua_type(L,-1)==LUA_TNIL) {
                unset(key);
        } else if (lua_type(L,-1)==LUA_TSTRING) {
                std::string val = luaL_checkstring(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TNUMBER) {
                lua_Number val = luaL_checknumber(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TBOOLEAN) {
                bool val = check_bool(L,-1);
                set(key,val);
        } else if (has_tag(L,-1,VECTOR3_TAG)) {
                GET_UD_MACRO(Vector3,val,-1,VECTOR3_TAG);
                set(key,val);
        } else if (has_tag(L,-1,QUAT_TAG)) {
                GET_UD_MACRO(Quaternion,val,-1,QUAT_TAG);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TTABLE) {
                ExternalTable *self = new ExternalTable();
                self->takeTableFromLuaStack(L,lua_gettop(L));
                set(key,self);
        } else {
                return "type not supported";
        }
        return NULL;
}

const char *ExternalTable::luaSet (lua_State *L, lua_Number key)
{
        if (lua_type(L,-1)==LUA_TNIL) {
                unset(key);
        } else if (lua_type(L,-1)==LUA_TSTRING) {
                std::string val = luaL_checkstring(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TNUMBER) {
                lua_Number val = luaL_checknumber(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TBOOLEAN) {
                bool val = check_bool(L,-1);
                set(key,val);
        } else if (has_tag(L,-1,VECTOR3_TAG)) {
                GET_UD_MACRO(Vector3,val,-1,VECTOR3_TAG);
                set(key,val);
        } else if (has_tag(L,-1,QUAT_TAG)) {
                GET_UD_MACRO(Quaternion,val,-1,QUAT_TAG);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TTABLE) {
                ExternalTable *self = new ExternalTable();
                self->takeTableFromLuaStack(L,-1);
                set(key,self);
        } else {
                return "type not supported";
        }
        return NULL;
}

void ExternalTable::dump (lua_State *L)
{
        lua_createtable(L, elements.size(), fields.size());
        for (StringMap::const_iterator i=fields.begin(),i_=fields.end() ; i!=i_ ; ++i) {
                const std::string &key = i->first;
                const Value &v = i->second;
                lua_pushstring(L, key.c_str());
                push(L, v);
                lua_rawset(L,-3);
        }
        for (NumberMap::const_iterator i=elements.begin(),i_=elements.end() ; i!=i_ ; ++i) {
                lua_Number key = i->first;
                const Value &v = i->second;
                lua_pushnumber(L, key);
                push(L, v);
                lua_rawset(L,-3);
        }
}

void ExternalTable::takeTableFromLuaStack (lua_State *L, int tab)
{
        if (tab<0) tab += 1 + lua_gettop(L);
        lua_checkstack(L,2);
        for (lua_pushnil(L) ; lua_next(L,tab)!=0 ; lua_pop(L,1)) {
                const char *err = luaSet(L);
                if (err) my_lua_error(L, err);
        }
}


// vim: shiftwidth=8:tabstop=8:expandtab
