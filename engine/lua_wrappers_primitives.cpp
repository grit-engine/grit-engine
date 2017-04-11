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

#include <algorithm>

#include "lua_wrappers_primitives.h"
#include "math_util.h"


// PLOT ==================================================================== {{{

int plot_make (lua_State *L)
{
TRY_START
    Plot *self = new Plot();
    check_args(L, 1);
    int table = lua_gettop(L);
    if (!lua_istable(L, table))
        my_lua_error(L, "Parameter should be a table");
    for (lua_pushnil(L) ; lua_next(L, table) != 0 ; lua_pop(L, 1)) {
        // the name is held in the object anyway
        float k = check_float(L, -2);
        float v = check_float(L, -1);
        self->addPoint(k, v);
    }
    self->commit();
    push(L, self, PLOT_TAG);
    return 1;
TRY_END
}

TOSTRING_ADDR_MACRO(plot, Plot, PLOT_TAG)

GC_MACRO(Plot, plot, PLOT_TAG)

static int plot_index(lua_State *L)
{
TRY_START
    typedef Plot::Map Map;
    typedef Plot::MI MI;
    check_args(L, 2);
    GET_UD_MACRO(Plot, self, 1, PLOT_TAG);
    if (lua_type(L, 2) == LUA_TNUMBER) {
        float k = check_float(L, 2);
        lua_pushnumber(L, self[k]);
    } else {
        std::string key  = luaL_checkstring(L, 2);
        if (key == "minX") {
            lua_pushnumber(L, self.minX());
        } else if (key == "maxX") {
            lua_pushnumber(L, self.maxX());
        } else if (key == "points") {
            Map data = self.getPoints();
            lua_createtable(L, data.size(), 0);
            for (MI i=data.begin(), i_=data.end() ; i != i_ ; ++i) {
                lua_pushnumber(L, i->first);
                lua_pushnumber(L, i->second);
                lua_settable(L, -3);
            }
        } else if (key == "tangents") {
            Map data = self.getTangents();
            lua_createtable(L, data.size(), 0);
            for (MI i=data.begin(), i_=data.end() ; i != i_ ; ++i) {
                lua_pushnumber(L, i->first);
                lua_pushnumber(L, i->second);
                lua_settable(L, -3);
            }
        } else {
            my_lua_error(L, "Not a readable Plot member: " + key);
        }
    }
    return 1;
TRY_END
}

static int plot_newindex(lua_State *L)
{
TRY_START
    check_args(L, 3);
    GET_UD_MACRO(Plot, self, 1, PLOT_TAG);
    (void) self;
    std::string key  = luaL_checkstring(L, 2);
    if (false) {
    } else {
        my_lua_error(L, "Not a writeable Plot member: " + key);
    }
    return 0;
TRY_END
}

EQ_PTR_MACRO(Plot, plot, PLOT_TAG)

MT_MACRO_NEWINDEX(plot);

// }}}


// PLOT_V3 ==================================================================== {{{

int plot_v3_make (lua_State *L)
{
TRY_START
    PlotV3 *self = new PlotV3();
    check_args(L, 1);
    int table = lua_gettop(L);
    if (!lua_istable(L, table))
        my_lua_error(L, "Parameter should be a table");
    for (lua_pushnil(L) ; lua_next(L, table) != 0 ; lua_pop(L, 1)) {
        // the name is held in the object anyway
        float k = check_float(L, -2);
        Vector3 v = check_v3(L, -1);
        self->addPoint(k, v);
    }
    self->commit();
    push(L, self, PLOT_V3_TAG);
    return 1;
TRY_END
}

TOSTRING_ADDR_MACRO(plot_v3, PlotV3, PLOT_V3_TAG)

GC_MACRO(PlotV3, plot_v3, PLOT_V3_TAG)

static int plot_v3_index(lua_State *L)
{
TRY_START
    typedef PlotV3::Map Map;
    typedef PlotV3::MI MI;
    check_args(L, 2);
    GET_UD_MACRO(PlotV3, self, 1, PLOT_V3_TAG);
    if (lua_type(L, 2) == LUA_TNUMBER) {
        float k = check_float(L, 2);
        push_v3(L, self[k]);
    } else {
        std::string key  = luaL_checkstring(L, 2);
        if (key == "minX") {
            lua_pushnumber(L, self.minX());
        } else if (key == "maxX") {
            lua_pushnumber(L, self.maxX());
        } else if (key == "points") {
            Map data = self.getPoints();
            lua_createtable(L, data.size(), 0);
            for (MI i=data.begin(), i_=data.end() ; i != i_ ; ++i) {
                lua_pushnumber(L, i->first);
                push_v3(L, i->second);
                lua_settable(L, -3);
            }
        } else if (key == "tangents") {
            Map data = self.getTangents();
            lua_createtable(L, data.size(), 0);
            for (MI i=data.begin(), i_=data.end() ; i != i_ ; ++i) {
                lua_pushnumber(L, i->first);
                push_v3(L, i->second);
                lua_settable(L, -3);
            }
        } else {
            my_lua_error(L, "Not a readable PlotV3 member: " + key);
        }
    }
    return 1;
TRY_END
}

static int plot_v3_newindex(lua_State *L)
{
TRY_START
    check_args(L, 3);
    GET_UD_MACRO(PlotV3, self, 1, PLOT_V3_TAG);
    (void) self;
    std::string key  = luaL_checkstring(L, 2);
    if (false) {
    } else {
        my_lua_error(L, "Not a writeable PlotV3 member: " + key);
    }
    return 0;
TRY_END
}

EQ_PTR_MACRO(PlotV3, plot_v3, PLOT_V3_TAG)

MT_MACRO_NEWINDEX(plot_v3);

// }}}


// STRINGDB ================================================================ {{{

typedef std::vector<std::string> StringDB;


int stringdb_make (lua_State *L)
{
TRY_START
    StringDB self;
    if (lua_gettop(L) == 1) {
        int table = lua_gettop(L);
        if (!lua_istable(L, table))
            my_lua_error(L, "Parameter should be a table");
        for (lua_pushnil(L) ; lua_next(L, table) != 0 ; lua_pop(L, 1)) {
            // the name is held in the object anyway
            std::string str = luaL_checkstring(L, -1);
            self.push_back(str);
        }
    } else {
        check_args(L, 0);
    }
    push(L, new StringDB(self), STRINGDB_TAG);
    return 1;
TRY_END
}

std::ostream &operator<<(std::ostream &o, StringDB &db)
{
    o << "{";
    for (size_t i=0 ; i<db.size() ; ++i) {
        if (i>0) o << ", ";
        o << db[i];
    }
    o << "}";
    return o;
}
    


static int stringdb_add (lua_State *L)
{
TRY_START
    check_args(L, 2);
    GET_UD_MACRO(StringDB, self, 1, STRINGDB_TAG);
    self.push_back(luaL_checkstring(L, 2));
    return 0;
TRY_END
}


static int stringdb_clear (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(StringDB, self, 1, STRINGDB_TAG);
    self.clear();
    return 0;
TRY_END
}


TOSTRING_MACRO(stringdb, StringDB, STRINGDB_TAG)

GC_MACRO(StringDB, stringdb, STRINGDB_TAG)

static int stringdb_index(lua_State *L)
{
TRY_START
    check_args(L, 2);
    GET_UD_MACRO(StringDB, self, 1, STRINGDB_TAG);
    if (lua_type(L, 2) == LUA_TNUMBER) {
        if (self.size() == 0) {
            my_lua_error(L, "Empty stringdb");
        }
        unsigned short index=check_t<unsigned short>(L, 2, 1, self.size());
        lua_pushstring(L, self[index-1].c_str());
    } else {
        std::string key  = luaL_checkstring(L, 2);
        if (key == "add") {
            push_cfunction(L, stringdb_add);
        } else if (key == "clear") {
            push_cfunction(L, stringdb_clear);
        } else if (key == "table") {
            lua_createtable(L, self.size(), 0);
            for (unsigned int i=0 ; i<self.size() ; i++) {
                lua_pushnumber(L, i + 1);
                lua_pushstring(L, self[i].c_str());
                lua_settable(L, -3);
            }
        } else {
            my_lua_error(L, "Not a readable StringDB member: " + key);
        }
    }
    return 1;
TRY_END
}

static int stringdb_newindex(lua_State *L)
{
TRY_START
    check_args(L, 3);
    GET_UD_MACRO(StringDB, self, 1, STRINGDB_TAG);
    std::string key  = luaL_checkstring(L, 2);
    if (lua_type(L, 2) == LUA_TNUMBER) {
        if (self.size() == 0) {
            my_lua_error(L, "Empty stringdb");
        }
        unsigned short index=check_t<unsigned short>(L, 2, 1, self.size());
        std::string v = luaL_checkstring(L, 3);
        self[index-1] = v;
    } else {
        if (key == "value") {
            GET_UD_MACRO(StringDB, v, 3, STRINGDB_TAG);
            self = v;
        } else {
            my_lua_error(L, "Not a writeable StringDB member: " + key);
        }
    }
    return 0;
TRY_END
}

static int stringdb_len(lua_State *L)
{
TRY_START
    check_args(L, 2); // a
    GET_UD_MACRO(StringDB, self, 1, STRINGDB_TAG);
    lua_pushnumber(L, self.size());
    return 1;
TRY_END
}

EQ_PTR_MACRO(StringDB, stringdb, STRINGDB_TAG)

MT_MACRO_LEN_NEWINDEX(stringdb);

// }}}
