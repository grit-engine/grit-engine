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

#include "grit_lua_util.h"
#include "lua_wrappers_disk_resource.h"
#include "main.h"

#include "gfx/gfx_disk_resource.h"

#define DISKRESHOLD_TAG "Grit/DiskResourceHold"



// DISK RESOURCE HOLD ============================================================= {{{

static int diskreshold_gc (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO_OFFSET(DiskResourcePtr<DiskResource>, self, 1, DISKRESHOLD_TAG, 0);
    delete self;
    return 0;
TRY_END
}

TOSTRING_ADDR_MACRO(diskreshold, DiskResourcePtr<DiskResource>, DISKRESHOLD_TAG)

static int diskreshold_index (lua_State *L)
{
TRY_START
    check_args(L, 2);
    GET_UD_MACRO(DiskResourcePtr<DiskResource>, self, 1, DISKRESHOLD_TAG);
    std::string key = check_string(L, 2);
    if (key == "name") {
        push_string(L, self->getName());
    } else {
        EXCEPT << "DiskResourceHold no such field: " << key << ENDL;
    }
    return 1;
TRY_END
}

static int diskreshold_newindex (lua_State *L)
{
TRY_START
    check_args(L, 3);
    GET_UD_MACRO(DiskResourcePtr<DiskResource>, self, 1, DISKRESHOLD_TAG);
    std::string key = check_string(L, 2);
    (void) self;
    if (false) {
    } else {
        EXCEPT << "DiskResourceHold no such field: " << key << ENDL;
    }
    return 0;
TRY_END
}

EQ_PTR_MACRO(DiskResourcePtr<DiskResource>, diskreshold, DISKRESHOLD_TAG)

MT_MACRO_NEWINDEX(diskreshold);

//}}}


// {{{ FLAGS

static int global_disk_resource_get_gfx_verbose_loads (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushboolean(L, gfx_disk_resource_verbose_loads);
    return 1;
TRY_END
}

static int global_disk_resource_set_gfx_verbose_loads (lua_State *L)
{
TRY_START
    check_args(L, 1);
    gfx_disk_resource_verbose_loads = check_bool(L, 1);
    return 0;
TRY_END
}

static int global_disk_resource_get_verbose_loads (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushboolean(L, disk_resource_verbose_loads);
    return 1;
TRY_END
}

static int global_disk_resource_set_verbose_loads (lua_State *L)
{
TRY_START
    check_args(L, 1);
    disk_resource_verbose_loads = check_bool(L, 1);
    return 0;
TRY_END
}

static int global_disk_resource_get_verbose_incs (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushboolean(L, disk_resource_verbose_incs);
    return 1;
TRY_END
}

static int global_disk_resource_set_verbose_incs (lua_State *L)
{
TRY_START
    check_args(L, 1);
    disk_resource_verbose_incs = check_bool(L, 1);
    return 0;
TRY_END
}

// }}}


static int global_disk_resource_num (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushnumber(L, disk_resource_num());
    return 1;
TRY_END
}

static int global_disk_resource_num_loaded (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushnumber(L, disk_resource_num_loaded());
    return 1;
TRY_END
}

static int global_disk_resource_all (lua_State *L)
{
TRY_START
    check_args(L, 0);
    DiskResources drs = disk_resource_all();
    lua_newtable(L);
    int counter = 1;
    for (DiskResources::iterator i=drs.begin(), i_=drs.end() ; i != i_ ; ++i) {
        lua_pushstring(L, (*i)->getName().c_str());
        lua_rawseti(L, -2, counter++);
    }
    return 1;
TRY_END
}

static int global_disk_resource_all_loaded (lua_State *L)
{
TRY_START
    check_args(L, 0);
    DiskResources drs = disk_resource_all_loaded();
    lua_newtable(L);
    int counter = 1;
    for (DiskResources::iterator i=drs.begin(), i_=drs.end() ; i != i_ ; ++i) {
        lua_pushstring(L, (*i)->getName().c_str());
        lua_rawseti(L, -2, counter++);
    }
    return 1;
TRY_END
}

static int global_disk_resource_reload (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    DiskResource *dr = disk_resource_get_or_make(name);
    if (!dr->isLoaded()) my_lua_error(L, "Resource not loaded: \"" + std::string(name) + "\"");
    dr->reload();
    return 0;
TRY_END
}

static int global_disk_resource_load (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    DiskResource *dr = disk_resource_get_or_make(name);
    if (dr->isLoaded()) my_lua_error(L, "Resource already loaded: \"" + std::string(name) + "\"");
    dr->loadForeground();
    return 0;
TRY_END
}

static int global_disk_resource_ensure_loaded (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    DiskResource *dr = disk_resource_get_or_make(name);
    if (!dr->isLoaded())
        dr->loadForeground();
    return 0;
TRY_END
}

static int global_disk_resource_unload (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    DiskResource *dr = disk_resource_get_or_make(name);
    if (!dr->isLoaded()) my_lua_error(L, "Resource not loaded: \"" + std::string(name) + "\"");
    dr->unload();
    return 0;
TRY_END
}

static int global_disk_resource_add (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    disk_resource_get_or_make(name);
    return 0;
TRY_END
}

static int global_disk_resource_has (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    bool b = disk_resource_has(name);
    lua_pushboolean(L, b);
    return 1;
TRY_END
}

static int global_disk_resource_loaded (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    DiskResource *dr = disk_resource_get_or_make(name);
    lua_pushboolean(L, dr->isLoaded());
    return 1;
TRY_END
}

static int global_disk_resource_users (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    DiskResource *dr = disk_resource_get_or_make(name);
    lua_pushnumber(L, dr->getUsers());
    return 1;
TRY_END
}


static int global_disk_resource_check (lua_State *L)
{
TRY_START
    check_args(L, 0);
    bgl->checkRAMHost();
    bgl->checkRAMGPU();
    return 0;
TRY_END
}

static int global_host_ram_available (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushnumber(L, host_ram_available());
    return 1;
TRY_END
}

static int global_host_ram_used (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushnumber(L, host_ram_used());
    return 1;
TRY_END
}


static int global_disk_resource_hold_make (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    DiskResource *dr = disk_resource_get_or_make(name);
    auto *self = new DiskResourcePtr<DiskResource>(dr);
    push(L, self, DISKRESHOLD_TAG);
    return 1;
TRY_END
}



static const luaL_reg global[] = {

    // global flags
    {"disk_resource_get_gfx_verbose_loads", global_disk_resource_get_gfx_verbose_loads},
    {"disk_resource_set_gfx_verbose_loads", global_disk_resource_set_gfx_verbose_loads},
    {"disk_resource_get_verbose_loads", global_disk_resource_get_verbose_loads},
    {"disk_resource_set_verbose_loads", global_disk_resource_set_verbose_loads},
    {"disk_resource_get_verbose_incs", global_disk_resource_get_verbose_incs},
    {"disk_resource_set_verbose_incs", global_disk_resource_set_verbose_incs},

    // querying
    {"disk_resource_num", global_disk_resource_num},
    {"disk_resource_num_loaded", global_disk_resource_num_loaded},
    {"disk_resource_all", global_disk_resource_all},
    {"disk_resource_all_loaded", global_disk_resource_all_loaded},

    // the resources themselves
    {"disk_resource_add", global_disk_resource_add},
    {"disk_resource_has", global_disk_resource_has},
    {"disk_resource_users", global_disk_resource_users},
    {"disk_resource_ensure_loaded", global_disk_resource_ensure_loaded},
    {"disk_resource_loaded", global_disk_resource_loaded},
    {"disk_resource_load", global_disk_resource_load},
    {"disk_resource_unload", global_disk_resource_unload},
    {"disk_resource_reload", global_disk_resource_reload},
    {"disk_resource_hold_make", global_disk_resource_hold_make},

    {"disk_resource_check", global_disk_resource_check},

    {"host_ram_available", global_host_ram_available},
    {"host_ram_used", global_host_ram_used},

    {NULL, NULL}
};

void disk_resource_lua_init (lua_State *L)
{
    register_lua_globals(L, global);

    ADD_MT_MACRO(diskreshold, DISKRESHOLD_TAG);
}
