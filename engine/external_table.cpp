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

#include "external_table.h"
#include "grit_lua_util.h"
#include "lua_wrappers_primitives.h"

void ExternalTable::destroy (lua_State *L)
{
        clear(L);
}

void ExternalTable::clear (lua_State *L)
{
        for (StringMap::iterator i=fields.begin(),i_=fields.end() ; i!=i_ ; ++i) {
                Value &v = i->second;
                if (v.type == 8) {
                        v.func.setNil(L);
                } else if (v.type == 5) {
                        v.t->destroy(L);
                }
        }
        for (NumberMap::iterator i=elements.begin(),i_=elements.end() ; i!=i_ ; ++i) {
                Value &v = i->second;
                if (v.type == 8) {
                        v.func.setNil(L);
                } else if (v.type == 5) {
                        v.t->destroy(L);
                }
        }
        fields.clear();
        elements.clear();
}
const char *ExternalTable::luaGet (lua_State *L)
{
        if (lua_type(L,-1)==LUA_TSTRING) {
                std::string key = lua_tostring(L,-1);
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
                push_v3(L, v.v3);
                break;
                case 3:
                push_quat(L, v.q);
                break;
                case 4:
                lua_pushboolean(L,v.b);
                break;
                case 5:
                v.t->dump(L);
                break;
                case 6:
                push(L,new Plot(v.plot),PLOT_TAG);
                break;
                case 7:
                push(L,new PlotV3(v.plot_v3),PLOT_V3_TAG);
                break;
                case 8:
                v.func.push(L);
                break;
                case 9:
                push_v2(L, v.v2);
                break;
                case 10:
                push_v4(L, v.v4);
                break;
                default:
                CERR << "Unhandled ExternalTable type: " << v.type << std::endl;
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
                std::string key = lua_tostring(L,-2);
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
                std::string val = lua_tostring(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TNUMBER) {
                lua_Number val = luaL_checknumber(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TBOOLEAN) {
                bool val = check_bool(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TVECTOR3) {
                Vector3 val = check_v3(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TQUAT) {
                Quaternion val = check_quat(L,-1);
                set(key,val);
        } else if (is_userdata(L,-1,PLOT_TAG)) {
                GET_UD_MACRO(Plot,self,-1,PLOT_TAG);
                set(key,self);
        } else if (is_userdata(L,-1,PLOT_V3_TAG)) {
                GET_UD_MACRO(PlotV3,self,-1,PLOT_V3_TAG);
                set(key,self);
        } else if (lua_type(L,-1)==LUA_TTABLE) {
                SharedPtr<ExternalTable> self = SharedPtr<ExternalTable>(new ExternalTable());
                self->takeTableFromLuaStack(L,lua_gettop(L));
                set(key,self);
        } else if (lua_type(L,-1)==LUA_TFUNCTION) {
                Value &v = fields[key];
                v.func.setNoPop(L);
                v.type = 8;
        } else if (lua_type(L,-1)==LUA_TVECTOR2) {
                Vector2 val = check_v2(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TVECTOR4) {
                Vector4 val = check_v4(L,-1);
                set(key,val);
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
                std::string val = lua_tostring(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TNUMBER) {
                lua_Number val = luaL_checknumber(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TBOOLEAN) {
                bool val = check_bool(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TVECTOR3) {
                Vector3 val = check_v3(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TQUAT) {
                Quaternion val = check_quat(L,-1);
                set(key,val);
        } else if (is_userdata(L,-1,PLOT_TAG)) {
                GET_UD_MACRO(Plot,self,-1,PLOT_TAG);
                set(key,self);
        } else if (is_userdata(L,-1,PLOT_V3_TAG)) {
                GET_UD_MACRO(PlotV3,self,-1,PLOT_V3_TAG);
                set(key,self);
        } else if (lua_type(L,-1)==LUA_TTABLE) {
                SharedPtr<ExternalTable> self = SharedPtr<ExternalTable>(new ExternalTable());
                self->takeTableFromLuaStack(L,-1);
                set(key,self);
        } else if (lua_type(L,-1)==LUA_TFUNCTION) {
                Value &v = elements[key];
                v.func.setNoPop(L);
                v.type = 8;
        } else if (lua_type(L,-1)==LUA_TVECTOR2) {
                Vector2 val = check_v2(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TVECTOR4) {
                Vector4 val = check_v4(L,-1);
                set(key,val);
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
