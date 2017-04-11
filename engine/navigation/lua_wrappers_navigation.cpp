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

#include "lua_wrappers_navigation.h"
#include"navigation_system.h"
#include"navigation.h"
#include <iostream>
#include "../gfx/lua_wrappers_gfx.h"
#include"../physics/lua_wrappers_physics.h"
#include "../external_table.h"
#include "../lua_ptr.h"
#include "../path_util.h"

#include "input_geom.h"

static int navigation_system_update(lua_State *L)
{
TRY_START
    check_args(L, 1);

    navigation_update(check_float(L, 1));

    return 0;
TRY_END
}

static int navigation_system_update_debug(lua_State *L)
{
TRY_START
    check_args(L, 1);

    navigation_update_debug(check_float(L, 1));

    return 0;
TRY_END
}

static int navigation_system_reset(lua_State *L)
{
TRY_START
    check_args(L, 0);
    nvsys->reset();
    return 0;
TRY_END
}

static int crowd_move_agents(lua_State *L)
{
TRY_START
    check_args(L, 1);

    nvsys->set_dest_pos(to_ogre(check_v3(L, 1)));

    return 0;
TRY_END
}

static int global_navigation_add_obj(lua_State *L)
{
TRY_START
    check_args(L, 1);

    nvsys->addObj(check_string(L, 1));

    return 0;
TRY_END
}

static int global_navigation_add_gfx_body(lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(GfxBodyPtr, body, 1, GFXBODY_TAG);
    nvsys->addGfxBody(body);
    return 0;
TRY_END
}

static int global_navigation_add_gfx_bodies(lua_State *L)
{
TRY_START
    check_args(L, 1);

    std::vector<GfxBodyPtr> bds;

    lua_pushnil(L);
    while (lua_next(L, -2))
    {
        GET_UD_MACRO(GfxBodyPtr, body, -1, GFXBODY_TAG);
        bds.push_back(body);
        lua_pop(L, 1);
    }

    lua_pop(L, lua_gettop(L));

    nvsys->addGfxBodies(bds);
    return 0;
TRY_END
}

static int global_navigation_add_rigid_body(lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(RigidBody, body, 1, "Grit/RigidBody");
    nvsys->addRigidBody(&body);
    return 0;
TRY_END
}

static int global_add_temp_obstacle(lua_State *L)
{
TRY_START
    check_args(L, 1);
    nvsys->addTempObstacle(to_ogre(check_v3(L, 1)));
    return 0;
TRY_END
}

static int global_remove_temp_obstacle(lua_State *L)
{
TRY_START
    check_args(L, 1);
    nvsys->removeTempObstacle(to_ogre(check_v3(L, 1)));
    return 0;
TRY_END
}

static int global_add_offmesh_connection(lua_State *L)
{
TRY_START
    check_args(L, 3);
    nvsys->addOffmeshConection(to_ogre(check_v3(L, 1)), to_ogre(check_v3(L, 2)), check_bool(L, 3));
    return 0;
TRY_END
}

static int global_remove_offmesh_connection(lua_State *L)
{
TRY_START
    check_args(L, 1);
    nvsys->removeOffmeshConection(to_ogre(check_v3(L, 1)));
    return 0;
TRY_END
}

static int global_build_nav_mesh(lua_State *L)
{
TRY_START
    check_args(L, 0);
    nvsys->buildNavMesh();
    return 0;
TRY_END
}

const char* getfield_string(lua_State *L, const char *key) {
    const char* result;
    lua_pushstring(L, key);
    lua_gettable(L, -2);

    result = (const char*)lua_tostring(L, -1);
    lua_pop(L, 1);
    return result;
}

int getfield_int(lua_State *L, const char *key) {
    int result;
    lua_pushstring(L, key);
    lua_gettable(L, -2);

    result = (int)lua_tonumber(L, -1);
    lua_pop(L, 1);
    return result;
}

float getfield_float(lua_State *L, const char *key) {
    float result;
    lua_pushstring(L, key);
    lua_gettable(L, -2);

    result = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);
    return result;
}

bool getfield_bool(lua_State *L, const char *key) {
    bool result;
    lua_pushstring(L, key);
    lua_gettable(L, -2);

    result = (bool)lua_toboolean(L, -1);
    lua_pop(L, 1);
    return result;
}

static int global_navigation_update_params(lua_State *L)
{
TRY_START
    check_args(L, 0);

    lua_getglobal(L, "nav_builder_params");

    nvsys->getNavigationManager()->setCellSize(getfield_float(L, "cellSize"));
    nvsys->getNavigationManager()->setCellHeight(getfield_float(L, "cellHeight"));

    nvsys->getNavigationManager()->setAgentHeight(getfield_float(L, "agentHeight"));
    nvsys->getNavigationManager()->setAgentRadius(getfield_float(L, "agentRadius"));
    nvsys->getNavigationManager()->setAgentMaxClimb(getfield_float(L, "agentMaxClimb"));
    nvsys->getNavigationManager()->setAgentMaxSlope(getfield_float(L, "agentMaxSlope"));

    nvsys->getNavigationManager()->setRegionMinSize(getfield_float(L, "regionMinSize"));
    nvsys->getNavigationManager()->setRegionMergeSize(getfield_float(L, "regionMergeSize"));

    nvsys->getNavigationManager()->setPartitionType(getfield_int(L, "partitionType"));

    nvsys->getNavigationManager()->setEdgeMaxLen(getfield_float(L, "edgeMaxLen"));
    nvsys->getNavigationManager()->setEdgeMaxError(getfield_float(L, "edgeMaxError"));
    nvsys->getNavigationManager()->setVertsPerPoly(getfield_float(L, "vertsPerPoly"));

    nvsys->getNavigationManager()->setDetailSampleDist(getfield_float(L, "detailSampleDist"));
    nvsys->getNavigationManager()->setDetailSampleMaxError(getfield_float(L, "detailSampleMaxError"));
    nvsys->getNavigationManager()->setKeepInterResults(getfield_bool(L, "keepInterResults"));

    nvsys->getNavigationManager()->setTileSize(getfield_float(L, "tileSize"));

    return 0;
TRY_END
}

static int global_navigation_nearest_point_on_navmesh(lua_State *L)
{
TRY_START
    check_args(L, 1);

    dtPolyRef m_targetRef;

    Ogre::Vector3 resPoint;

    bool found = nvsys->findNearestPointOnNavmesh(swap_yz(to_ogre(check_v3(L, 1))), m_targetRef, resPoint);
    if (found)
    {
        push_v3(L, from_ogre(swap_yz(resPoint)));
        lua_pushinteger(L, m_targetRef);
        return 2;
    }
    else {
        lua_pushnil(L);
        return 1;
    }
TRY_END
}

static int global_navigation_random_navmesh_point(lua_State *L)
{
TRY_START
    check_args(L, 0);

    Ogre::Vector3 resPoint;

    bool found = nvsys->getRandomNavMeshPoint(resPoint);
    if (found)
    {
        push_v3(L, from_ogre(swap_yz(resPoint)));
        return 1;
    }
    else {
        lua_pushnil(L);
        return 1;
    }
TRY_END
}

static int global_navigation_random_navmesh_point_in_circle(lua_State *L)
{
TRY_START
    check_args(L, 2);

    Ogre::Vector3 resPoint;

    bool found = nvsys->getRandomNavMeshPointInCircle(swap_yz(to_ogre(check_v3(L, 1))), check_float(L, 2), resPoint);
    if (found)
    {
        push_v3(L, from_ogre(swap_yz(resPoint)));
        return 1;
    }
    else {
        lua_pushnil(L);
        return 1;
    }
TRY_END
}

static int global_navigation_debug_option(lua_State *L)
{
TRY_START
    if (lua_gettop(L) == 1)
    {
        const char *key = luaL_checkstring(L, 1);
        if (!::strcmp(key, "enabled")) {
            lua_pushboolean(L, NavSysDebug::Enabled);
        }
        else if (!::strcmp(key, "navmesh")) {
            lua_pushboolean(L, NavSysDebug::ShowNavmesh);
        }
        else if (!::strcmp(key, "navmesh_use_tile_colours")) {
            lua_pushboolean(L, NavSysDebug::NavmeshUseTileColours);
        }
        else if (!::strcmp(key, "bounds")) {
            lua_pushboolean(L, NavSysDebug::ShowBounds);
        }
        else if (!::strcmp(key, "tiling_grid")) {
            lua_pushboolean(L, NavSysDebug::ShowTilingGrid);
        }
        else if (!::strcmp(key, "agent")) {
            lua_pushboolean(L, NavSysDebug::ShowAgents);
        }
        else if (!::strcmp(key, "agent_arrows")) {
            lua_pushboolean(L, NavSysDebug::ShowAgentArrows);
        }
        else if (!::strcmp(key, "convex_volumes")) {
            lua_pushboolean(L, NavSysDebug::ShowConvexVolumes);
        }
        else if (!::strcmp(key, "obstacles")) {
            lua_pushboolean(L, NavSysDebug::ShowObstacles);
        }
        else if (!::strcmp(key, "offmesh_connections")) {
            lua_pushboolean(L, NavSysDebug::ShowOffmeshConnections);
        }
        else {
            my_lua_error(L, "Invalid Attribute: " + std::string(key));
        }
        return 1;
    }
    else if (lua_gettop(L) == 2)
    {
        const char *key = luaL_checkstring(L, 1);
        if (!::strcmp(key, "enabled")) {
            bool b = check_bool(L, 2);
            NavSysDebug::Enabled = b;
        }
        else if (!::strcmp(key, "navmesh")) {
            bool b = check_bool(L, 2);
            NavSysDebug::ShowNavmesh = b;
        }
        else if (!::strcmp(key, "navmesh_use_tile_colours")) {
            bool b = check_bool(L, 2);
            NavSysDebug::NavmeshUseTileColours = b;
        }
        else if (!::strcmp(key, "bounds")) {
            bool b = check_bool(L, 2);
            NavSysDebug::ShowBounds = b;
        }
        else if (!::strcmp(key, "tiling_grid")) {
            bool b = check_bool(L, 2);
            NavSysDebug::ShowTilingGrid = b;
        }
        else if (!::strcmp(key, "agent")) {
            bool b = check_bool(L, 2);
            NavSysDebug::ShowAgents = b;
        }
        else if(!::strcmp(key, "agent_arrows")) {
            bool b = check_bool(L, 2);
            NavSysDebug::ShowAgentArrows = b;
        }
        else if (!::strcmp(key, "convex_volumes")) {
            bool b = check_bool(L, 2);
            NavSysDebug::ShowConvexVolumes = b;
        }
        else if (!::strcmp(key, "obstacles")) {
            bool b = check_bool(L, 2);
            NavSysDebug::ShowObstacles = b;
        }
        else if (!::strcmp(key, "offmesh_connections")) {
            bool b = check_bool(L, 2);
            NavSysDebug::ShowOffmeshConnections = b;
        }
        else {
            my_lua_error(L, "Invalid Option: " + std::string(key));
        }
    }
    return 0;
TRY_END
}

static int global_navigation_save_navmesh(lua_State *L)
{
TRY_START
    check_args(L, 1);
    lua_pushboolean(L, nvsys->saveNavmesh(check_string(L, 1)));
    return 1;
TRY_END
}

static int global_navigation_load_navmesh(lua_State *L)
{
TRY_START
    check_args(L, 1);
    lua_pushboolean(L, nvsys->loadNavmesh(check_string(L, 1)));
    return 1;
TRY_END
}

static int global_navigation_navmesh_loaded(lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushboolean(L, nvsys->anyNavmeshLoaded());
    return 1;
TRY_END
}

static int global_add_convex_volume_point(lua_State *L)
{
TRY_START
    check_args(L, 1);
    nvsys->createConvexVolume(to_ogre(check_v3(L, 1)));
    return 0;
TRY_END
}

static int global_remove_convex_volume(lua_State *L)
{
TRY_START
    check_args(L, 1);
    nvsys->removeConvexVolume(to_ogre(check_v3(L, 1)));
    return 0;
TRY_END
}

static const luaL_reg global_navigation[] = {
    { "navigation_add_obj", global_navigation_add_obj },
    { "navigation_add_gfx_body", global_navigation_add_gfx_body },
    { "navigation_add_gfx_bodies", global_navigation_add_gfx_bodies },
    { "navigation_add_rigid_body", global_navigation_add_rigid_body },

    { "navigation_update", navigation_system_update },
    { "navigation_update_debug", navigation_system_update_debug },
    { "navigation_reset", navigation_system_reset },

    { "navigation_navmesh_loaded", global_navigation_navmesh_loaded },

    { "crowd_move_to", crowd_move_agents },
    { "navigation_add_obstacle", global_add_temp_obstacle },
    { "navigation_remove_obstacle", global_remove_temp_obstacle },
    { "navigation_add_offmesh_connection", global_add_offmesh_connection },

    { "navigation_add_convex_volume_point", global_add_convex_volume_point },
    { "navigation_remove_convex_volume", global_remove_convex_volume },

    { "navigation_remove_offmesh_connection", global_remove_offmesh_connection },
    { "navigation_build_nav_mesh", global_build_nav_mesh },
    { "navigation_update_params", global_navigation_update_params },
    { "navigation_nearest_point_on_navmesh", global_navigation_nearest_point_on_navmesh },

    { "navigation_random_navmesh_point", global_navigation_random_navmesh_point },
    { "navigation_random_navmesh_point_in_circle", global_navigation_random_navmesh_point_in_circle },

    { "navigation_debug_option", global_navigation_debug_option },
    { "navigation_save_navmesh", global_navigation_save_navmesh },
    { "navigation_load_navmesh", global_navigation_load_navmesh },

    { NULL, NULL }
};

static int agent_add(lua_State *L)
{
TRY_START
    check_args(L, 1);
    lua_pushinteger(L, nvsys->addAgent(to_ogre(check_v3(L, 1))));
    return 1;
TRY_END
}

static int agent_remove(lua_State *L)
{
TRY_START
    check_args(L, 1);
    nvsys->removeAgent(check_int(L, 1, 0, 256));
    return 0;
TRY_END
}

static int agent_active(lua_State *L)
{
TRY_START
    check_args(L, 1);
    lua_pushboolean(L, nvsys->isAgentActive(check_int(L, 1, 0, 256)));
    return 1;
TRY_END
}

static int agent_stop(lua_State *L)
{
TRY_START
    check_args(L, 1);
    nvsys->agentStop(check_int(L, 1, 0, 256));
    return 0;
TRY_END
}

static int agent_position(lua_State *L)
{
TRY_START
    check_args(L, 1);
    push_v3(L, from_ogre(nvsys->getAgentPosition(check_int(L, 1, 0, 256))));
    return 1;
TRY_END
}

static int agent_velocity(lua_State *L)
{
TRY_START
    check_args(L, 1);
    push_v3(L, from_ogre(nvsys->getAgentVelocity(check_int(L, 1, 0, 256))));
    return 1;
TRY_END
}

static int agent_request_velocity(lua_State *L)
{
TRY_START
    check_args(L, 2);
    nvsys->agentRequestVelocity(check_int(L, 1, 0, 256), to_ogre(check_v3(L, 2)));
    return 0;
TRY_END
}

static int agent_move_target(lua_State *L)
{
TRY_START
    check_args(L, 3);
    nvsys->setAgentMoveTarget(check_int(L, 1, 0, 256), to_ogre(check_v3(L, 2)), check_bool(L, 3));
    return 0;
TRY_END
}

static int agent_distance_to_goal(lua_State *L)
{
TRY_START
    check_args(L, 2);
    lua_pushnumber(L, nvsys->getDistanceToGoal(check_int(L, 1, 0, 256), check_float(L, 2)));
    return 1;
TRY_END
}

static int agent_height(lua_State *L)
{
TRY_START
    check_args(L, 1);
    lua_pushnumber(L, nvsys->getAgentHeight(check_int(L, 1, 0, 256)));
    return 1;
TRY_END
}

static int agent_radius(lua_State *L)
{
TRY_START
    check_args(L, 1);
    lua_pushnumber(L, nvsys->getAgentRadius(check_int(L, 1, 0, 256)));
    return 1;
TRY_END
}

static const luaL_reg global_agent[] = {
    { "agent_make", agent_add },
    { "agent_destroy", agent_remove },
    { "agent_active", agent_active },
    { "agent_stop", agent_stop },
    { "agent_position", agent_position },
    { "agent_velocity", agent_velocity },
    { "agent_request_velocity", agent_request_velocity },
    { "agent_move_target", agent_move_target },
    { "agent_distance_to_goal", agent_distance_to_goal },
    { "agent_height", agent_height },
    { "agent_radius", agent_radius },
    { NULL, NULL }
};

void navigation_lua_init(lua_State *L)
{
    register_lua_globals(L, global_navigation);
    register_lua_globals(L, global_agent);
}
