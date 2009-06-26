#include "ExternalTable.h"
#include "lua_util.h"
#include "lua_wrappers_primitives.h"


const char *ExternalTable::luaGet (lua_State *L)
{
        if (lua_type(L,-1)!=LUA_TSTRING) {
                return "key was not a string";
        }
        Ogre::String key = luaL_checkstring(L,-1);
        return luaGet(L,key);
}

const char *ExternalTable::luaGet (lua_State *L, const Ogre::String &key)
{
        if (!has(key)) {
                lua_pushnil(L);
        } else {
                const Value &v = fields[key];
                switch (v.type) {
                        case 0:
                        lua_pushnumber(L,v.real);
                        break;
                        case 1:
                        lua_pushstring(L,v.str.c_str());
                        break;
                        case 2:
                        push(L,new Ogre::Vector3(v.v3),VECTOR3_TAG);
                        break;
                        case 3:
                        push(L,new Ogre::Quaternion(v.q),QUAT_TAG);
                        break;
                        case 4:
                        lua_pushboolean(L,v.b);
                        break;
                        case 5:
                        lua_createtable(L, v.strs.size(), 0);
                        for (unsigned int i=0 ; i<v.strs.size() ; i++) {
                                lua_pushnumber(L,i+LUA_ARRAY_BASE);
                                lua_pushstring(L,v.strs[i].c_str());
                                lua_settable(L,-3);
                        }
                        break;
                }
        }
        return NULL;
}

const char *ExternalTable::luaSet (lua_State *L)
{
        if (lua_type(L,-2)!=LUA_TSTRING) {
                return "key was not a string";
        }
        Ogre::String key = luaL_checkstring(L,-2);
        return luaSet(L,key);
}

const char *ExternalTable::luaSet (lua_State *L, const Ogre::String &key)
{
        // the name is held in the object anyway
        if (lua_type(L,3)==LUA_TNIL) {
                unset(key);
        } else if (lua_type(L,-1)==LUA_TSTRING) {
                Ogre::String val = luaL_checkstring(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TNUMBER) {
                Ogre::Real val = luaL_checknumber(L,-1);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TBOOLEAN) {
                bool val = 0!=lua_toboolean(L,-1);
                set(key,val);
        } else if (has_tag(L,-1,VECTOR3_TAG)) {
                GET_UD_MACRO(Ogre::Vector3,val,-1,VECTOR3_TAG);
                set(key,val);
        } else if (has_tag(L,-1,QUAT_TAG)) {
                GET_UD_MACRO(Ogre::Quaternion,val,-1,QUAT_TAG);
                set(key,val);
        } else if (lua_type(L,-1)==LUA_TTABLE) {
                std::vector<Ogre::String> self;
                int table = lua_gettop(L);
                for (lua_pushnil(L) ; lua_next(L,table)!=0 ; lua_pop(L,1)) {
                        std::string str = luaL_checkstring(L,-1);
                        self.push_back(str);
                }
                set(key,self);
        } else {
                return "type not supported";
        }
        return NULL;
}

// vim: shiftwidth=8:tabstop=8:expandtab
