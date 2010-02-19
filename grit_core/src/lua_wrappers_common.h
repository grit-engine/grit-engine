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

#ifndef lua_wrappers_common_h
#define lua_wrappers_common_h

#include <OgreString.h>
#include <OgreException.h>

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "lua_util.h"



static inline void *ensure_valid (lua_State *L, void *ptr, const char *tag)
{
        void *_ptr = *static_cast<void**>(ptr);
        if (_ptr==NULL)
                my_lua_error(L,"Object disabled: "+std::string(tag));
        return _ptr;
}


#define GET_UD_MACRO(type,ud,index,tag) type & ud = *static_cast<type*> ( \
                ensure_valid(L,luaL_checkudata(L,index,tag),tag));

#define GET_UD_MACRO_OFFSET(type,ud,index,tag,offset) \
        type*&ud = static_cast<type**>(luaL_checkudata(L,index,tag))[offset]; \
        if (offset>0) APP_ASSERT(ud!=NULL);


#define TOSTRING_MACRO(name,type,tag) \
static int name##_tostring (lua_State *L) \
{ \
TRY_START \
        check_args(L,1); \
        GET_UD_MACRO(type,ud,1,tag); \
        std::stringstream ss; \
        ss << ud; \
        lua_pushstring(L, ss.str().c_str()); \
        return 1; \
TRY_END \
}

#define TOSTRING_SMART_PTR_MACRO(name,type,tag) \
static int name##_tostring (lua_State *L) \
{ \
        check_args(L,1); \
        GET_UD_MACRO(type,ud,1,tag); \
        std::stringstream ss; \
        ss << tag << " " << static_cast<void*>(&*ud); \
        lua_pushstring(L, ss.str().c_str()); \
        return 1; \
}

#define TOSTRING_ADDR_MACRO(name,type,tag) \
static int name##_tostring (lua_State *L) \
{ \
        check_args(L,1); \
        GET_UD_MACRO(type,ud,1,tag); \
        std::stringstream ss; \
        ss << tag << " " << static_cast<void*>(&ud); \
        lua_pushstring(L, ss.str().c_str()); \
        return 1; \
}

#define TOSTRING_GETNAME_MACRO(name,Type,method,TAG) \
static int name##_tostring (lua_State *L) \
{ \
        check_args(L,1); \
        GET_UD_MACRO(Type,ud,1,TAG); \
        std::stringstream ss; \
        ss << TAG << " \"" << ud method << "\""; \
        lua_pushstring(L, ss.str().c_str()); \
        return 1; \
}



#define GC_MACRO(type, name, tag) \
static int name##_gc (lua_State *L) \
{ \
TRY_START \
        check_args(L,1); \
        GET_UD_MACRO(type,ud,1,tag); \
        delete &ud; \
        return 0; \
TRY_END \
}

#define EQ_MACRO(type, name, tag) \
static int name##_eq (lua_State *L) \
{ \
TRY_START \
        check_args(L,2); \
        GET_UD_MACRO(type,self,1,tag); \
        GET_UD_MACRO(type,other,2,tag); \
        lua_pushboolean(L,self==other); \
        return 1; \
TRY_END \
}


#define EQ_PTR_MACRO(type, name, tag) \
static int name##_eq (lua_State *L) \
{ \
TRY_START \
        check_args(L,2); \
        GET_UD_MACRO(type,self,1,tag); \
        GET_UD_MACRO(type,other,2,tag); \
        lua_pushboolean(L,&self==&other); \
        return 1; \
TRY_END \
}


#define MT_MACRO_DECLARE(name) \
extern const luaL_reg name##_meta_table[]

#define MT_MACRO(name) \
const luaL_reg name##_meta_table[] = { \
        {"__tostring", name##_tostring}, \
        {"__gc", name##_gc}, \
        {"__index", name##_index}, \
        {"__eq", name##_eq}, \
        {NULL, NULL} \
}

#define MT_MACRO_NEWINDEX(name) \
const luaL_reg name##_meta_table[] = { \
        {"__tostring", name##_tostring}, \
        {"__gc", name##_gc}, \
        {"__newindex", name##_newindex}, \
        {"__index", name##_index}, \
        {"__eq", name##_eq}, \
        {NULL, NULL} \
}

#define MT_MACRO_LEN_NEWINDEX(name) \
const luaL_reg name##_meta_table[] = { \
        {"__tostring", name##_tostring}, \
        {"__gc", name##_gc}, \
        {"__newindex", name##_newindex}, \
        {"__index", name##_index}, \
        {"__len", name##_len}, \
        {"__eq", name##_eq}, \
        {NULL, NULL} \
}

#define MT_MACRO_ARITH_NEWINDEX(name) \
const luaL_reg name##_meta_table[] = { \
        {"__tostring", name##_tostring}, \
        {"__gc", name##_gc}, \
        {"__index", name##_index}, \
        {"__newindex", name##_newindex}, \
        {"__mul", name##_mul}, \
        {"__unm", name##_unm}, \
        {"__add", name##_add}, \
        {"__sub", name##_sub}, \
        {"__len", name##_len}, \
        {"__div", name##_div}, \
        {"__eq", name##_eq}, \
        {NULL, NULL} \
}



#define TRY_START try {
#define TRY_END } catch (Ogre::Exception &e) { \
        std::string msg = e.getFullDescription(); \
        my_lua_error(L,msg); \
        return 0; \
}


#define LUA_ARRAY_BASE 1

static inline void push (lua_State *L, void *v, const char *tag)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        *ud = static_cast<void*>(v);
        luaL_getmetatable(L, tag);
        lua_setmetatable(L, -2);
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
