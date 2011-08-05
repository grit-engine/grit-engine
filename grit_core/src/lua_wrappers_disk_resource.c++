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

#include "gfx/GfxDiskResource.h"
#include "physics/CollisionMesh.h"

#include "lua_wrappers_common.h"

#include "lua_wrappers_disk_resource.h"
#include "gfx/lua_wrappers_gfx.h"
#include "physics/lua_wrappers_physics.h"





// {{{ FLAGS

static int global_disk_resource_get_gfx_verbose_loads (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L, gfx_disk_resource_verbose_loads);
        return 1;
TRY_END
}

static int global_disk_resource_set_gfx_verbose_loads (lua_State *L)
{
TRY_START
        check_args(L,1);
        gfx_disk_resource_verbose_loads = check_bool(L,1);
        return 0;
TRY_END
}

static int global_disk_resource_get_verbose_loads (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L, disk_resource_verbose_loads);
        return 1;
TRY_END
}

static int global_disk_resource_set_verbose_loads (lua_State *L)
{
TRY_START
        check_args(L,1);
        disk_resource_verbose_loads = check_bool(L,1);
        return 0;
TRY_END
}

static int global_disk_resource_get_verbose_incs (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L, disk_resource_verbose_incs);
        return 1;
TRY_END
}

static int global_disk_resource_set_verbose_incs (lua_State *L)
{
TRY_START
        check_args(L,1);
        disk_resource_verbose_incs = check_bool(L,1);
        return 0;
TRY_END
}

// }}}


// {{{ COUNTERS

static int global_disk_resource_num (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,disk_resource_num());
        return 1;
TRY_END
}

static int global_disk_resource_num_loaded (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,disk_resource_num_loaded());
        return 1;
TRY_END
}

// }}}


// need some way to increment / decrement, probably

bool disk_resource_index (lua_State *L, DiskResource &self, const char *key)
{
TRY_START
    if (!::strcmp(key,"users")) {
        lua_pushnumber(L,self.getUsers());
    } else if (!::strcmp(key,"loaded")) {
        lua_pushboolean(L,self.isLoaded());
    } else if (!::strcmp(key,"gpuResource")) {
        lua_pushnumber(L,self.isGPUResource());
    } else if (!::strcmp(key,"name")) {
        lua_pushstring(L,self.getName().c_str());
/*
        } else if (!::strcmp(key,"reload")) {
                lua_pushnumber(L,self.getLinearSleepThreshold());
*/
    } else {
        return false;
    }
    return true;
TRY_END
}


static void push_disk_resource (lua_State *L, DiskResource *dr)
{
    GfxDiskResource *gdr = dynamic_cast<GfxDiskResource*>(dr);
    if (gdr!=NULL) {
        lua_pushnil(L);
        //push_gfx_disk_resource(L,gdr);
        return;
    }
    CollisionMesh *cm = dynamic_cast<CollisionMesh*>(dr);
    if (cm!=NULL) {
        push_colmesh(L,cm);
        return;
    }
}



static int global_disk_resource_all (lua_State *L)
{
TRY_START
    check_args(L,0);
    DiskResources drs = disk_resource_all();
    lua_newtable(L);
    int counter = 1;
    for (DiskResources::iterator i=drs.begin(),i_=drs.end() ; i!=i_ ; ++i) {
        push_disk_resource(L, *i);
        lua_rawseti(L, -2, counter++);
    }
    return 1;
TRY_END
}

static int global_disk_resource_all_loaded (lua_State *L)
{
TRY_START
    check_args(L,0);
    DiskResources drs = disk_resource_all_loaded();
    lua_newtable(L);
    int counter = 1;
    for (DiskResources::iterator i=drs.begin(),i_=drs.end() ; i!=i_ ; ++i) {
        push_disk_resource(L, *i);
        lua_rawseti(L, -2, counter++);
    }
    return 1;
TRY_END
}

static int global_disk_resource_get (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    DiskResource *dr = disk_resource_get(name);
    push_disk_resource(L, dr);
    return 1;
TRY_END
}

static int global_disk_resource_get_or_make (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    DiskResource *dr = disk_resource_get_or_make(name);
    push_disk_resource(L, dr);
    return 1;
TRY_END
}




static const luaL_reg global[] = {
    // global flags
    {"disk_resource_get_gfx_verbose_loads",global_disk_resource_get_gfx_verbose_loads},
    {"disk_resource_set_gfx_verbose_loads",global_disk_resource_set_gfx_verbose_loads},
    {"disk_resource_get_verbose_loads",global_disk_resource_get_verbose_loads},
    {"disk_resource_set_verbose_loads",global_disk_resource_set_verbose_loads},
    {"disk_resource_get_verbose_incs",global_disk_resource_get_verbose_incs},
    {"disk_resource_set_verbose_incs",global_disk_resource_set_verbose_incs},

    // counters
    {"disk_resource_num",global_disk_resource_num},
    {"disk_resource_num_loaded",global_disk_resource_num_loaded},

    // the resources themselves
    {"disk_resource_get_or_make",global_disk_resource_get_or_make},
    {"disk_resource_get",global_disk_resource_get},
    {"disk_resource_all",global_disk_resource_all},
    {"disk_resource_all_loaded",global_disk_resource_all_loaded},

    {NULL, NULL}
};

void disk_resource_lua_init (lua_State *L)
{
    luaL_register(L, "_G", global);
}

// vim: ts=4:sw=4:et
