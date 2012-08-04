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


static int global_disk_resource_all (lua_State *L)
{
TRY_START
    check_args(L,0);
    DiskResources drs = disk_resource_all();
    lua_newtable(L);
    int counter = 1;
    for (DiskResources::iterator i=drs.begin(),i_=drs.end() ; i!=i_ ; ++i) {
        lua_pushstring(L, (*i)->getName().c_str());
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
        lua_pushstring(L, (*i)->getName().c_str());
        lua_rawseti(L, -2, counter++);
    }
    return 1;
TRY_END
}

static int global_disk_resource_reload (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    DiskResource *dr = disk_resource_get(name);
    if (dr==NULL) my_lua_error(L, "No such resource: \""+std::string(name)+"\"");
    if (!dr->isLoaded()) my_lua_error(L, "Resource not loaded: \""+std::string(name)+"\"");
    dr->reload();
    return 0;
TRY_END
}

static int global_disk_resource_load (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    DiskResource *dr = disk_resource_get(name);
    if (dr==NULL) my_lua_error(L, "No such resource: \""+std::string(name)+"\"");
    if (dr->isLoaded()) my_lua_error(L, "Resource already loaded: \""+std::string(name)+"\"");
    dr->load();
    return 0;
TRY_END
}

static int global_disk_resource_load_indefinitely (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    DiskResource *dr = disk_resource_get_or_make(name);
    dr->increment();
    if (dr->isLoaded()) {
        lua_pushboolean(L, false);
    } else {
        lua_pushboolean(L, true);
        dr->load();
    }
    return 1;
TRY_END
}

static int global_disk_resource_unload (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    DiskResource *dr = disk_resource_get(name);
    if (dr==NULL) my_lua_error(L, "No such resource: \""+std::string(name)+"\"");
    if (!dr->isLoaded()) my_lua_error(L, "Resource not loaded: \""+std::string(name)+"\"");
    dr->unload();
    return 0;
TRY_END
}

static int global_disk_resource_add (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    disk_resource_get_or_make(name);
    return 0;
TRY_END
}

static int global_disk_resource_has (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    DiskResource *dr = disk_resource_get(name);
    lua_pushboolean(L, dr!=NULL);
    return 1;
TRY_END
}

static int global_disk_resource_loaded (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    DiskResource *dr = disk_resource_get(name);
    if (dr==NULL) my_lua_error(L, "No such resource: \""+std::string(name)+"\"");
    lua_pushboolean(L, dr->isLoaded());
    return 1;
TRY_END
}

static int global_disk_resource_users (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    DiskResource *dr = disk_resource_get(name);
    if (dr==NULL) my_lua_error(L, "No such resource: \""+std::string(name)+"\"");
    lua_pushnumber(L, dr->getUsers());
    return 1;
TRY_END
}

static int global_disk_resource_acquire (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    DiskResource *dr = disk_resource_get(name);
    if (dr==NULL) my_lua_error(L, "No such resource: \""+std::string(name)+"\"");
    dr->increment();
    return 0;
TRY_END
}

static int global_disk_resource_release (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    DiskResource *dr = disk_resource_get(name);
    if (dr==NULL) my_lua_error(L, "No such resource: \""+std::string(name)+"\"");
    dr->decrement();
    return 0;
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

    // querying
    {"disk_resource_num",global_disk_resource_num},
    {"disk_resource_num_loaded",global_disk_resource_num_loaded},
    {"disk_resource_all",global_disk_resource_all},
    {"disk_resource_all_loaded",global_disk_resource_all_loaded},

    // the resources themselves
    {"disk_resource_add",global_disk_resource_add},
    {"disk_resource_has",global_disk_resource_has},
    {"disk_resource_users",global_disk_resource_users},
    {"disk_resource_loaded",global_disk_resource_loaded},
    {"disk_resource_load",global_disk_resource_load},
    {"disk_resource_load_indefinitely",global_disk_resource_load_indefinitely},
    {"disk_resource_unload",global_disk_resource_unload},
    {"disk_resource_reload",global_disk_resource_reload},
    {"disk_resource_acquire",global_disk_resource_acquire},
    {"disk_resource_release",global_disk_resource_release},


    {NULL, NULL}
};

void disk_resource_lua_init (lua_State *L)
{
    register_lua_globals(L, global);
}

// vim: ts=4:sw=4:et
