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

#include <limits.h>

#include <OgreFont.h>
#include <OgreFontManager.h>
#include <OgreHighLevelGpuProgramManager.h>

#ifdef USE_GOOGLE_PERF_TOOLS
        #include <google/profiler.h>
#endif

#ifndef WIN32
        #include <sys/mman.h>
#endif

#include "Grit.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "BackgroundMeshLoader.h"
#include "sleep.h"
#include "clipboard.h"
#include "HUD.h"
#include "CentralisedLog.h"


#include "lua_wrappers_primitives.h"
#include "lua_wrappers_physics.h"
#include "lua_wrappers_mobj.h"
#include "lua_wrappers_scnmgr.h"
#include "lua_wrappers_material.h"
#include "lua_wrappers_mesh.h"
#include "lua_wrappers_gpuprog.h"
#include "lua_wrappers_tex.h"
#include "lua_wrappers_render.h"
#include "lua_wrappers_gritobj.h"
#include "lua_wrappers_hud.h"
#include "path_util.h"

#include "lua_utf8.h"


// GLOBAL LIBRARY ========================================================== {{{

static int global_have_focus (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,grit->windowHasFocus());
        return 1;
TRY_END
}

static int global_get_keyb_presses (lua_State *L)
{
TRY_START
        check_args(L,0);
        Keyboard::Presses presses = grit->getKeyboard()->getPresses();

        lua_createtable(L, presses.size(), 0);
        for (unsigned int i=0 ; i<presses.size() ; i++) {
                const char *key = presses[i].c_str();
                lua_pushnumber(L,i+LUA_ARRAY_BASE);
                lua_pushstring(L,key);
                lua_settable(L,-3);
        }

        return 1;
TRY_END
}

static int global_keyb_flush (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==1) {
                std::string key = luaL_checkstring(L,1);
                grit->getKeyboard()->flush(key);
        } else {
                check_args(L,0);
                grit->getKeyboard()->flush();
        }
        return 0;
TRY_END
}

static int global_get_keyb_verbose (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,grit->getKeyboard()->getVerbose());
        return 1;
TRY_END
}

static int global_set_keyb_verbose (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        grit->getKeyboard()->setVerbose(b);
        return 0;
TRY_END
}

static int global_get_mouse_events (lua_State *L)
{
TRY_START
        check_args(L,0);

        int rel_x, rel_y, x, y;
        std::vector<int> clicks;
        bool moved = grit->getMouse()->getEvents(&clicks,&x,&y,&rel_x,&rel_y);

        lua_pushboolean(L,moved);

        lua_createtable(L, clicks.size(), 0);
        for (unsigned int i=0 ; i<clicks.size() ; i++) {
                int button = clicks[i];
                lua_pushnumber(L,i+LUA_ARRAY_BASE);
                const char *button_ = "unknown";
                switch (button) {
                        case Mouse::MOUSE_LEFT: button_="+left" ; break;
                        case -Mouse::MOUSE_LEFT: button_="-left" ; break;
                        case Mouse::MOUSE_MIDDLE: button_="+middle" ; break;
                        case -Mouse::MOUSE_MIDDLE: button_="-middle" ; break;
                        case Mouse::MOUSE_RIGHT: button_="+right" ; break;
                        case -Mouse::MOUSE_RIGHT: button_="-right" ; break;
                        case Mouse::MOUSE_SCROLL_UP: button_="+up" ; break;
                        case -Mouse::MOUSE_SCROLL_UP: button_="-up" ;break;
                        case Mouse::MOUSE_SCROLL_DOWN: button_="+down" ; break;
                        case -Mouse::MOUSE_SCROLL_DOWN: button_="-down" ;break;
                }
                lua_pushstring(L,button_);
                lua_settable(L,-3);
        }

        lua_pushnumber(L,x);
        lua_pushnumber(L,y);
        lua_pushnumber(L,rel_x);
        lua_pushnumber(L,rel_y);

        return 6;
TRY_END
}

static int global_set_mouse_pos (lua_State *L)
{
TRY_START
        check_args(L,2);
        int x = check_t<int>(L,1);
        int y = check_t<int>(L,2);
        grit->getMouse()->setPos(x,y);
        return 0;
TRY_END
}

static int global_get_mouse_hide (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,grit->getMouse()->getHide());
        return 1;
TRY_END
}

static int global_get_mouse_grab (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,grit->getMouse()->getGrab());
        return 1;
TRY_END
}

static int global_set_mouse_hide (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        grit->getMouse()->setHide(b);
        return 0;
TRY_END
}

static int global_set_mouse_grab (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        grit->getMouse()->setGrab(b);
        return 0;
TRY_END
}

static int global_fire_frame_started (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,grit->getOgre()->_fireFrameStarted());
        return 1;
TRY_END
}

static int global_fire_frame_rendering_queued (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,grit->getOgre()->_fireFrameRenderingQueued());
        return 1;
TRY_END
}

static int global_fire_frame_ended (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,grit->getOgre()->_fireFrameEnded());
        return 1;
TRY_END
}

static int global_render (lua_State *L)
{
TRY_START
        check_args(L,0);
        grit->render();
        return 0;
TRY_END
}


static int global_update_elapsed_time (lua_State *L)
{
TRY_START
        check_args(L,1);
        Ogre::Real seconds = luaL_checknumber(L,1);
        Ogre::Real t = Ogre::ControllerManager::getSingleton().getElapsedTime();
        t += seconds;
        Ogre::ControllerManager::getSingleton().setElapsedTime(t);
        return 0;
TRY_END
}


static int global_make_scnmgr (lua_State *L)
{
TRY_START
        check_args(L,2);
        const char *type = luaL_checkstring(L,1);
        const char *name = luaL_checkstring(L,2);
        push_scnmgr(L, grit->getOgre()->createSceneManager(type,name));
        return 1;
TRY_END
}

static int global_get_sm (lua_State *L)
{
TRY_START
        check_args(L,0);
        push_scnmgr(L, grit->getSM());
        return 1;
TRY_END
}

BulletDebugDrawer::DebugDrawModes ddm_from_string (lua_State *L, const std::string &str)
{
        if (str=="DrawWireframe") return BulletDebugDrawer::DBG_DrawWireframe;
        else if (str=="DrawAabb") return BulletDebugDrawer::DBG_DrawAabb;
        else if (str=="DrawFeaturesText") return BulletDebugDrawer::DBG_DrawFeaturesText;
        else if (str=="DrawContactPoints") return BulletDebugDrawer::DBG_DrawContactPoints;
        else if (str=="NoDeactivation") return BulletDebugDrawer::DBG_NoDeactivation;
        else if (str=="NoHelpText") return BulletDebugDrawer::DBG_NoHelpText;
        else if (str=="DrawText") return BulletDebugDrawer::DBG_DrawText;
        else if (str=="ProfileTimings") return BulletDebugDrawer::DBG_ProfileTimings;
        else if (str=="EnableSatComparison") return BulletDebugDrawer::DBG_EnableSatComparison;
        else if (str=="DisableBulletLCP") return BulletDebugDrawer::DBG_DisableBulletLCP;
        else if (str=="EnableCCD") return BulletDebugDrawer::DBG_EnableCCD;
        else if (str=="DrawConstraints") return BulletDebugDrawer::DBG_DrawConstraints;
        else if (str=="DrawConstraintLimits") return BulletDebugDrawer::DBG_DrawConstraintLimits;
        else if (str=="FastWireframe") return BulletDebugDrawer::DBG_FastWireframe;
        else {
                my_lua_error(L, "Did not recognise Bullet DebugDrawMode: "+str);
                return BulletDebugDrawer::DBG_NoDebug; // avoid compiler warning
        }
}


static int global_set_physics_debug (lua_State *L)
{
TRY_START
        check_args(L,2);
        push_scnmgr(L, grit->getSM());
        const char *str = luaL_checkstring(L,1);
        bool b = check_bool(L,2);
        BulletDebugDrawer::DebugDrawModes modes = (BulletDebugDrawer::DebugDrawModes)grit->getDebugDrawer()->getDebugMode();
        BulletDebugDrawer::DebugDrawModes mode = ddm_from_string(L, str);
        int new_modes = (modes & ~mode) | (b ? mode : BulletDebugDrawer::DBG_NoDebug);
        grit->getDebugDrawer()->setDebugMode(new_modes);
        return 0;
TRY_END
}

static int global_get_physics_debug (lua_State *L)
{
TRY_START
        check_args(L,1);
        push_scnmgr(L, grit->getSM());
        const char *str = luaL_checkstring(L,1);
        BulletDebugDrawer::DebugDrawModes modes = (BulletDebugDrawer::DebugDrawModes)grit->getDebugDrawer()->getDebugMode();
        BulletDebugDrawer::DebugDrawModes mode = ddm_from_string(L, str);
        lua_pushboolean(L, modes & mode);
        return 1;
TRY_END
}



////////////////////////////////////////////////////////////////////////////////

static int global_make_tex (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) lua_pushnumber(L,Ogre::MIP_DEFAULT);
        if (lua_gettop(L)==5) lua_pushnumber(L,Ogre::TU_DEFAULT);
        check_args(L,7);
        const char *name = luaL_checkstring(L,1);
        const char *texType = luaL_checkstring(L,2);
        unsigned width = check_t<unsigned>(L,3,1);
        unsigned height = check_t<unsigned>(L,4,1);
        unsigned depth = check_t<unsigned>(L,5,1);
        unsigned mipmaps = check_t<unsigned>(L,6);
        unsigned usage = check_t<unsigned>(L,7);

        Ogre::TexturePtr t = Ogre::TextureManager::getSingleton().createManual(
                name,
                "GRIT",
                texture_type_from_string(L,texType),
                width,
                height,
                depth,
                mipmaps,
                Ogre::PF_R8G8B8,
                usage);
        push(L, new Ogre::TexturePtr(t),TEX_TAG);
        return 1;
TRY_END
}

static int global_load_tex (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::TexturePtr t = Ogre::TextureManager::getSingleton()
                                .load(name,"GRIT");
        push(L, new Ogre::TexturePtr(t),TEX_TAG);
        return 1;
TRY_END
}


static int global_get_all_texs (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::TextureManager::ResourceMapIterator rmi =
                Ogre::TextureManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::TexturePtr &r = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::TexturePtr(r), TEX_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_tex (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::TexturePtr t =
                Ogre::TextureManager::getSingleton().getByName(name);
        if (t.isNull()) {
                lua_pushnil(L);
                return 1;
        }
        push(L, new Ogre::TexturePtr(t),TEX_TAG);
        return 1;
TRY_END
}

static int global_get_texture_verbose (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,Ogre::TextureManager::getSingleton().getVerbose());
        return 1;
TRY_END
}

static int global_set_texture_verbose (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        Ogre::TextureManager::getSingleton().setVerbose(b);
        return 0;
TRY_END
}

static int global_get_texture_budget (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,
                       Ogre::TextureManager::getSingleton().getMemoryBudget());
        return 1;
TRY_END
}

static int global_set_texture_budget (lua_State *L)
{
TRY_START
        check_args(L,1);
        size_t n = check_t<size_t>(L,1);
        Ogre::TextureManager::getSingleton().setMemoryBudget(n);
        return 0;
TRY_END
}

static int global_get_texture_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::TextureManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_textures (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::TextureManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_textures (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::TextureManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_tex (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::TextureManager::getSingleton().remove(name);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

static int global_load_skel (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::SkeletonPtr m = Ogre::SkeletonManager::getSingleton()
                                .load(name,"GRIT");
        push_skel(L,m);
        return 1;
TRY_END
}

static int global_get_all_skels (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::SkeletonManager::ResourceMapIterator rmi =
                Ogre::SkeletonManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::SkeletonPtr &r = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::SkeletonPtr(r), SKEL_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_skel (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::SkeletonPtr m =
                Ogre::SkeletonManager::getSingleton().getByName(name);
        push_skel(L,m);
        return 1;
TRY_END
}

static int global_get_skel_verbose (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,Ogre::SkeletonManager::getSingleton().getVerbose());
        return 1;
TRY_END
}

static int global_set_skel_verbose (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        Ogre::SkeletonManager::getSingleton().setVerbose(b);
        return 0;
TRY_END
}

static int global_get_skel_budget (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::SkeletonManager::getSingleton().getMemoryBudget());
        return 1;
TRY_END
}

static int global_set_skel_budget (lua_State *L)
{
TRY_START
        check_args(L,1);
        size_t n = check_t<size_t>(L,1);
        Ogre::SkeletonManager::getSingleton().setMemoryBudget(n);
        return 0;
TRY_END
}

static int global_get_skel_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::SkeletonManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_skels (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::SkeletonManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_skels (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::SkeletonManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_skel (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::SkeletonManager::getSingleton().remove(name);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// make

static int global_load_mesh (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MeshPtr m = Ogre::MeshManager::getSingleton()
                                .load(name,"GRIT");
        push_mesh(L,m);
        return 1;
TRY_END
}

static int global_get_all_meshes (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MeshManager::ResourceMapIterator rmi =
                Ogre::MeshManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::MeshPtr &r = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::MeshPtr(r), MESH_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_mesh (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MeshPtr m =
                Ogre::MeshManager::getSingleton().getByName(name);
        push_mesh(L,m);
        return 1;
TRY_END
}

static int global_get_mesh_verbose (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,Ogre::MeshManager::getSingleton().getVerbose());
        return 1;
TRY_END
}

static int global_set_mesh_verbose (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        Ogre::MeshManager::getSingleton().setVerbose(b);
        return 0;
TRY_END
}

static int global_get_mesh_budget (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::MeshManager::getSingleton().getMemoryBudget());
        return 1;
TRY_END
}

static int global_set_mesh_budget (lua_State *L)
{
TRY_START
        check_args(L,1);
        size_t n = check_t<size_t>(L,1);
        Ogre::MeshManager::getSingleton().setMemoryBudget(n);
        return 0;
TRY_END
}

static int global_get_mesh_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::MeshManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_meshes (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MeshManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_meshes (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MeshManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_mesh (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MeshManager::getSingleton().remove(name);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

static int global_make_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);

        Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().create(
                name,
                "GRIT",
                false,
                NULL);
        push(L, new Ogre::MaterialPtr(m),MAT_TAG);
        return 1;
TRY_END
}

static int global_load_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MaterialPtr t = Ogre::MaterialManager::getSingleton()
                                .load(name,"GRIT");
        push(L, new Ogre::MaterialPtr(t),MAT_TAG);
        return 1;
TRY_END
}

static int global_get_all_materials (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MaterialManager::ResourceMapIterator rmi =
                Ogre::MaterialManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::MaterialPtr &mat = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::MaterialPtr(mat), MAT_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MaterialPtr m =
                Ogre::MaterialManager::getSingleton().getByName(name);
        if (m.isNull()) {
                lua_pushnil(L);
                return 1;
        }
        push(L,new Ogre::MaterialPtr(m),MAT_TAG);
        return 1;
TRY_END
}

static int global_get_material_budget (lua_State *L)
{
TRY_START
        check_args(L,0);
      lua_pushnumber(L,Ogre::MaterialManager::getSingleton().getMemoryBudget());
        return 1;
TRY_END
}

static int global_set_material_budget (lua_State *L)
{
TRY_START
        check_args(L,1);
        size_t n = check_t<size_t>(L,1);
        Ogre::MaterialManager::getSingleton().setMemoryBudget(n);
        return 0;
TRY_END
}

static int global_get_material_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
       lua_pushnumber(L,Ogre::MaterialManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_materials (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MaterialManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_materials (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MaterialManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MaterialManager::getSingleton().remove(name);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// new material interface

typedef std::map<std::string, ExternalTable> MatMap;
static MatMap mat_map;

static int global_register_material (lua_State *L)
{
TRY_START
        check_args(L,2);
        const char *name = luaL_checkstring(L,1);
        if (!lua_istable(L,2))
                my_lua_error(L,"Second parameter should be a table");

        ExternalTable &t = mat_map[name];
        t.clear();
        t.takeTableFromLuaStack(L,2);
        return 0;
TRY_END
}

static int global_dump_registered_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        if (mat_map.find(name) == mat_map.end())
                my_lua_error(L, "Material does not exist: \""+std::string(name)+"\"");
        ExternalTable &t = mat_map[name];
        t.dump(L);
        return 1;
TRY_END
}

static int global_registered_material_get (lua_State *L)
{
TRY_START
        check_args(L,2);
        const char *name = luaL_checkstring(L,1);
        const char *key = luaL_checkstring(L,2);
        if (mat_map.find(name) == mat_map.end())
                my_lua_error(L, "Material does not exist: \""+std::string(name)+"\"");
        ExternalTable &t = mat_map[name];
        t.luaGet(L, key);
        return 1;
TRY_END
}

static int global_registered_material_set (lua_State *L)
{
TRY_START
        check_args(L,3);
        const char *name = luaL_checkstring(L,1);
        const char *key = luaL_checkstring(L,2);
        if (mat_map.find(name) == mat_map.end())
                my_lua_error(L, "Material does not exist: \""+std::string(name)+"\"");
        ExternalTable &t = mat_map[name];
        t.luaSet(L, key);
        return 1;
TRY_END
}

static int global_reprocess_all_registered_materials (lua_State *L)
{
TRY_START
        check_args(L,1);
        if (!lua_isfunction(L,1))
                my_lua_error(L,"Parameter should be a function");
        for (MatMap::iterator i=mat_map.begin(),i_=mat_map.end() ; i!=i_ ; ++i) {
                lua_pushvalue(L,1);
                lua_pushstring(L,i->first.c_str());
                i->second.dump(L);
                lua_call(L,2,0);
        }
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// make

static int global_make_gpuprog (lua_State *L)
{
TRY_START
        check_args(L,3);
        const char *name = luaL_checkstring(L,1);
        const char *language = luaL_checkstring(L,2);
        std::string typestr = luaL_checkstring(L,3);
        Ogre::GpuProgramType type;
        if (typestr=="FRAGMENT") {
                type = Ogre::GPT_FRAGMENT_PROGRAM;
        } else if (typestr=="VERTEX") {
                type = Ogre::GPT_VERTEX_PROGRAM;
        } else if (typestr=="GEOMETRY") {
                type = Ogre::GPT_GEOMETRY_PROGRAM;
        } else {
                my_lua_error(L,"Unrecognised GPU Program type: "+typestr);
        }
        Ogre::HighLevelGpuProgramPtr t = Ogre::HighLevelGpuProgramManager::getSingleton()
                                .createProgram(name,"GRIT", language, type);
        push(L, new Ogre::HighLevelGpuProgramPtr(t),GPUPROG_TAG);
        return 1;
TRY_END
}

static int global_get_all_gpuprogs (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::HighLevelGpuProgramManager::ResourceMapIterator rmi =
                Ogre::HighLevelGpuProgramManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::HighLevelGpuProgramPtr &self = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::HighLevelGpuProgramPtr(self), GPUPROG_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_gpuprog (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::HighLevelGpuProgramPtr m =
                Ogre::HighLevelGpuProgramManager::getSingleton().getByName(name);
        if (m.isNull()) {
                lua_pushnil(L);
                return 1;
        }
        push(L,new Ogre::HighLevelGpuProgramPtr(m),GPUPROG_TAG);
        return 1;
TRY_END
}

static int global_get_gpuprog_budget (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::GpuProgramManager::getSingleton().getMemoryBudget());
        return 1;
TRY_END
}

static int global_set_gpuprog_budget (lua_State *L)
{
TRY_START
        check_args(L,1);
        size_t n = check_t<size_t>(L,1);
        Ogre::GpuProgramManager::getSingleton().setMemoryBudget(n);
        return 0;
TRY_END
}

static int global_get_gpuprog_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::GpuProgramManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_gpuprogs (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::GpuProgramManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_gpuprogs (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::GpuProgramManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_gpuprog (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::GpuProgramManager::getSingleton().remove(name);
        return 0;
TRY_END
}

static int global_get_gpuprog_verbose (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,Ogre::GpuProgramManager::getSingleton().getVerbose());
        return 1;
TRY_END
}

static int global_set_gpuprog_verbose (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        Ogre::GpuProgramManager::getSingleton().setVerbose(b);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

static int global_get_alloc_stats (lua_State *L)
{
TRY_START
        check_args(L,0);
        size_t counter, mallocs, reallocs, frees;
        lua_alloc_stats_get(counter,mallocs,reallocs,frees);
        lua_pushnumber(L,counter);
        lua_pushnumber(L,mallocs);
        lua_pushnumber(L,reallocs);
        lua_pushnumber(L,frees);
        return 4;
TRY_END
}

static int global_set_alloc_stats (lua_State *L)
{
TRY_START
        check_args(L,3);
        size_t mallocs = check_t<size_t>(L,1);
        size_t reallocs = check_t<size_t>(L,2); 
        size_t frees = check_t<size_t>(L,3);
        lua_alloc_stats_set(mallocs,reallocs,frees);
        return 0;
TRY_END
}

static int global_reset_alloc_stats (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_alloc_stats_set(0,0,0);
        return 0;
TRY_END
}

static int global_get_in_queue_size (lua_State *L)
{
TRY_START
        check_args(L,0);
        size_t sz = BackgroundMeshLoader::getSingleton().size();
        lua_pushnumber(L, sz);
        return 1;
TRY_END
}

static int global_get_out_queue_size (lua_State *L)
{
TRY_START
        check_args(L,0);
        size_t sz = BackgroundMeshLoader::getSingleton().getLRUQueueSize();
        lua_pushnumber(L, sz);
        return 1;
TRY_END
}


static int global_give_queue_allowance (lua_State *L)
{
TRY_START
        check_args(L,1);
        Ogre::Real amount = luaL_checknumber(L,1);
        BackgroundMeshLoader::getSingleton().setAllowance(amount);
        return 0;
TRY_END
}


static int global_handle_bastards (lua_State *L)
{
TRY_START
        check_args(L,0);
        BackgroundMeshLoader::getSingleton().handleBastards();
        return 0;
TRY_END
}


static int global_get_rt (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::RenderTarget *rt =
                grit->getOgre()->getRenderSystem()->getRenderTarget(name);
        if (rt==NULL) {
                lua_pushnil(L);
                return 1;
        }
        if (dynamic_cast<Ogre::RenderWindow*>(rt)) {
                push_rwin(L, static_cast<Ogre::RenderWindow*>(rt));
        } else if (dynamic_cast<Ogre::RenderTexture*>(rt)) {
                push_rtex(L, static_cast<Ogre::RenderTexture*>(rt));
        } else {
                my_lua_error(L,"Unrecognised Render Target");
        }
        return 1;
TRY_END
}


static int global_get_scnmgr (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        push_scnmgr(L, grit->getOgre()->getSceneManager(name));
        return 1;
TRY_END
}


static int global_rendersystem_get_vsync (lua_State *L)
{
TRY_START
        check_args(L,0);
        bool b = grit->getOgre()->getRenderSystem()->getWaitForVerticalBlank();
        lua_pushboolean(L,b);
        return 1;
TRY_END
}


static int global_rendersystem_set_vsync (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        grit->getOgre()->getRenderSystem()->setWaitForVerticalBlank(b);
        return 0;
TRY_END
}


static int global_rendersystem_reinitialise (lua_State *L)
{
TRY_START
        check_args(L,0);
        grit->getOgre()->getRenderSystem()->reinitialise();
        return 0;
TRY_END
}


static int global_get_streamer (lua_State *L)
{
TRY_START
        check_args(L,0);
        push_streamer(L,&grit->getStreamer());
        return 1;
TRY_END
}

static int global_get_main_win (lua_State *L)
{
TRY_START
        check_args(L,0);
        push_rwin(L,grit->getWin());
        return 1;
TRY_END
}

static int global_get_hud_root (lua_State *L)
{
TRY_START
        check_args(L,0);
        push(L,new HUD::RootPtr(grit->getHUD()),PANE_TAG);
        return 1;
TRY_END
}

static int global_clicked_close (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,grit->hasClickedClose());
        return 1;
TRY_END
}


static int global_get_clipboard (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushstring(L,clipboard_get().c_str());
        return 1;
TRY_END
}

static int global_set_clipboard (lua_State *L)
{
TRY_START
        check_args(L,1);
        std::string s = luaL_checkstring(L,1);
        clipboard_set(s);
        return 0;
TRY_END
}


static int global_text_width (lua_State *L)
{
TRY_START
        check_args(L,3);
        const char *text = luaL_checkstring(L,1);
        std::string font_name = luaL_checkstring(L,2);
        lua_Number height = luaL_checknumber(L,3);
        Ogre::FontPtr fp =
                Ogre::FontManager::getSingleton().getByName(font_name);
        if (fp.isNull()) {
                my_lua_error(L,"No such font: "+font_name);
        }

        Ogre::DisplayString str = text;
        lua_pushnumber(L,HUD::text_width(str,fp,height));
        return 1;
TRY_END
}

static int global_text_pixel_substr (lua_State *L)
{
TRY_START
        check_args(L,5);
        const char *text = luaL_checkstring(L,1);
        std::string font_name = luaL_checkstring(L,2);
        lua_Number height = luaL_checknumber(L,3);
        lua_Number width = luaL_checknumber(L,4);
        bool wordwrap = check_bool(L,5);
        Ogre::FontPtr fp =
                Ogre::FontManager::getSingleton().getByName(font_name);
        if (fp.isNull()) {
                my_lua_error(L,"No such font: "+font_name);
        }
        Ogre::DisplayString rest;
        Ogre::DisplayString str =
                HUD::pixel_substr(text,fp,height,width,&rest,wordwrap);
        lua_pushstring(L,str.asUTF8_c_str());
        if (rest=="")
                lua_pushnil(L);
        else
                lua_pushstring(L,rest.asUTF8_c_str());
        return 2;
TRY_END
}

static int global_text_wrap (lua_State *L)
{
TRY_START
        check_args(L,9);
        const char *input = luaL_checkstring(L,1);
        lua_Number width = luaL_checknumber(L,2);
        unsigned lines = check_t<unsigned>(L,3);
        bool word_wrap = check_bool(L,4);
        bool chop_top = check_bool(L,5);
        unsigned tabs = check_t<unsigned>(L,6);
        bool codes = check_bool(L,7);
        std::string font_name = luaL_checkstring(L,8);
        lua_Number char_height = luaL_checknumber(L,9);
        Ogre::FontPtr fp =
                Ogre::FontManager::getSingleton().getByName(font_name);
        if (fp.isNull())
                my_lua_error(L,"No such font: "+font_name);
        HUD::DStr after_exp;
        if (tabs>0)
                HUD::expand_tabs(input,tabs,codes,after_exp);
        else
                after_exp.append(input);
        HUD::DStr offcut, output;
        HUD::wrap(after_exp,width,lines,word_wrap,chop_top,codes,fp,char_height,
                  output, &offcut);
        lua_pushstring(L,output.asUTF8_c_str());
        lua_pushstring(L,offcut.asUTF8_c_str());
        return 2;
TRY_END
}

static int global_pump (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::WindowEventUtilities::messagePump();
        return 0;
TRY_END
}

static int global_sleep (lua_State *L)
{
TRY_START
        check_args(L,1);
        long n = check_t<long>(L, 1);
        mysleep(n);
        return 0;
TRY_END
}

static int global_resource_exists (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        bool b = Ogre::ResourceGroupManager::getSingleton().
                resourceExists("GRIT",name);
        lua_pushboolean(L,b);
        return 1;
TRY_END
}

static int global_add_resource_location (lua_State *L)
{
TRY_START
        check_args(L,3);
        const char *resource = luaL_checkstring(L,1);
        const char *ext = luaL_checkstring(L,2);
        bool recursive = check_bool(L,3);
        Ogre::ResourceGroupManager::getSingleton().
                addResourceLocation(resource,ext,"GRIT",recursive);
        return 0;
TRY_END
}

static int global_init_all_resource_groups (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::ResourceGroupManager::getSingleton().
                initialiseAllResourceGroups();
        return 0;
TRY_END
}


static int global_error (lua_State *L)
{
TRY_START
        if (lua_gettop(L)!=1 && lua_gettop(L)!=2) {
                check_args(L,2);
        }

        const char *msg = luaL_checkstring(L,1);
        unsigned long level = 1;
        if (lua_gettop(L)==2) {
                level = check_t<unsigned long>(L,2);
        }
        my_lua_error(L,msg,level);

        return 0;
TRY_END
}


static int global_error_handler (lua_State *L)
{
TRY_START
        my_lua_error_handler(L);
        return 0;
TRY_END
}


static int global_echo (lua_State *L)
{
TRY_START
        std::stringstream ss;
        int args = lua_gettop(L);
        for (int i=1 ; i<=args ; ++i) {
                if (i>1) ss << "\t";
                lua_getfield(L, LUA_GLOBALSINDEX, "console_tostring");
                lua_pushvalue(L,i);
                lua_call(L,1,1);
                ss << lua_tostring(L,-1);
                lua_pop(L,1);
        }
        clog.echo(ss.str());
        return 0;
TRY_END
}


static int global_console_poll (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushstring(L,clog.consolePoll().c_str());
        return 1;
TRY_END
}


static int global_add_font (lua_State *L)
{
TRY_START
        check_args(L,5);
        const char *name = luaL_checkstring(L,1);
        const char *file = luaL_checkstring(L,2);
        lua_Number tex_width = check_t<unsigned>(L,3);
        lua_Number tex_height = check_t<unsigned>(L,4);
        luaL_checktype(L,5,LUA_TTABLE);


        // get or create font of the right name
        Ogre::FontPtr fntptr =Ogre::FontManager::getSingleton().getByName(name);
        if (fntptr.isNull()) {
                fntptr = Ogre::FontManager::getSingleton().create(name,"GRIT");
                fntptr->setType(Ogre::FT_IMAGE);
                fntptr->setSource(file);
        }

        // iterate through codepoints
        for (lua_pushnil(L) ; lua_next(L,5) ; lua_pop(L,1)) {
                lua_Number codepoint = check_t<unsigned>(L,-2);
                lua_rawgeti(L, -1, 1);
                lua_Number x = check_t<unsigned>(L,-1);
                lua_pop(L,1);
                lua_rawgeti(L, -1, 2);
                lua_Number y = check_t<unsigned>(L,-1);
                lua_pop(L,1);
                lua_rawgeti(L, -1, 3);
                lua_Number w = check_t<unsigned>(L,-1);
                lua_pop(L,1);
                lua_rawgeti(L, -1, 4);
                lua_Number h = check_t<unsigned>(L,-1);
                lua_pop(L,1);

                double u1 = x/tex_width;
                double v1 = y/tex_height;
                double u2 = u1 + w/tex_width;
                double v2 = v1 + h/tex_height;
                fntptr->setGlyphTexCoords ((Ogre::Font::CodePoint)codepoint,
                                           u1,v1,u2,v2,tex_width/tex_height);
        }


        return 0;
TRY_END
}

static int global_profiler_start (lua_State *L)
{
TRY_START
        check_args(L,1);
        #ifdef USE_GOOGLE_PERF_TOOLS
                std::string filename = luaL_checkstring(L,1);
                ProfilerStart(filename.c_str());
        #else
                my_lua_error(L,"Not compiled with USE_GOOGLE_PERF_TOOLS");
        #endif
        return 0;
TRY_END
}


static int global_profiler_stop (lua_State *L)
{
TRY_START
        check_args(L,0);
        #ifdef USE_GOOGLE_PERF_TOOLS
                ProfilerStop();
        #else
                my_lua_error(L,"Not compiled with USE_GOOGLE_PERF_TOOLS");
        #endif
        return 0;
TRY_END
}


static int global_mlockall (lua_State *L)
{
TRY_START
        check_args(L,0);
        #ifdef WIN32
                // just fail silently
                //my_lua_error(L,"mlockall not supported on Windows.");
        #else
                mlockall(MCL_CURRENT | MCL_FUTURE);
        #endif
        return 0;
TRY_END
}


static int global_munlockall (lua_State *L)
{
TRY_START
        check_args(L,0);
        #ifdef WIN32
                // just fail silently
                //my_lua_error(L,"mlockall not supported on Windows.");
        #else
                munlockall();
        #endif
        return 0;
TRY_END
}


static int global_get_rendersystem (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushstring(L,grit->getOgre()->getRenderSystem()->getName().c_str());
        return 1;
TRY_END
}

static int global_fqn (lua_State *L)
{
TRY_START
        check_args(L,1);
        std::string fqn = pwd_full(L, luaL_checkstring(L,1));
        lua_pushstring(L, fqn.c_str());
        return 1;
TRY_END
}

static int global_fqn_ex (lua_State *L)
{
TRY_START
        check_args(L,2);
        std::string rel = luaL_checkstring(L,1);
        std::string filename = luaL_checkstring(L,2);
        std::string dir(filename, 0, filename.rfind('/')+1);
        lua_pushstring(L, pwd_full_ex(L, rel, dir).c_str());
        return 1;
TRY_END
}

static int global_path_stack (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_createtable(L, pwd.size(), 0);
        for (unsigned int i=0 ; i<pwd.size() ; i++) {
                lua_pushnumber(L, i+LUA_ARRAY_BASE);
                lua_pushstring(L, pwd[i].c_str());
                lua_settable(L,-3);
        }
        return 1;
TRY_END
}

static int global_path_stack_push_file (lua_State *L)
{
TRY_START
        check_args(L,1);
        pwd_push_file(luaL_checkstring(L,1));
        return 0;
TRY_END
}

static int global_path_stack_push_dir (lua_State *L)
{
TRY_START
        check_args(L,1);
        pwd_push_dir(luaL_checkstring(L,1));
        return 0;
TRY_END
}

static int global_path_stack_pop (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushstring(L, pwd_pop().c_str());
        return 1;
TRY_END
}

static int global_path_stack_top (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushstring(L, pwd_get().c_str());
        return 1;
TRY_END
}

struct LuaIncludeState {
        Ogre::DataStreamPtr ds;
        char buf[16384];
};

static const char *aux_aux_include (lua_State *L, void *ud, size_t *size)
{
        (void) L;
        LuaIncludeState &lis = *static_cast<LuaIncludeState*>(ud);
        *size = lis.ds->read(lis.buf, sizeof(lis.buf));
        return lis.buf;
}


static int aux_include (lua_State *L, const std::string &filename)
{
        std::string fname(filename,1); // strip leading /
        if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("GRIT",fname)) {
                lua_pushfstring(L, "File not found: \"%s\"", filename.c_str());
                return LUA_ERRFILE;
        }
        LuaIncludeState lis;
        lis.ds = Ogre::ResourceGroupManager::getSingleton().openResource(fname,"GRIT");
        return lua_load(L, aux_aux_include, &lis, ("@"+filename).c_str());
}


static int global_include (lua_State *L)
{
TRY_START
        check_args(L,1);
        std::string filename = pwd_full(L, luaL_checkstring(L,1));

        lua_pushcfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        int status = aux_include(L,filename);
        if (status) {
                const char *str = lua_tostring(L,-1);
                my_lua_error(L,str);
        } else {
                pwd_push_file(filename);
                status = lua_pcall(L,0,0,error_handler);
                pwd_pop();
                if (status) {
                        lua_pop(L,1); //message
                }
        }

        lua_pop(L,1); // error handler
        return 0;
TRY_END
}

static int global_safe_include (lua_State *L)
{
TRY_START
        check_args(L,1);
        std::string filename = pwd_full(L, luaL_checkstring(L,1));

        lua_pushcfunction(L, my_lua_error_handler);
        // stack: [error_handler]
        int error_handler = lua_gettop(L);

        int status = aux_include(L,filename);
        if (status) {
                // stack: [error_handler, error_string]
                if (status==LUA_ERRFILE) {
                        const char *str = lua_tostring(L,-1);
                        my_lua_error(L,str);
                }
                // stack: [error_handler, error_string]
                lua_pop(L,1);
                // stack: [error_handler]
        } else {
                // stack: [error_handler, function]
                pwd_push_file(filename);
                // stack: [error_handler, function]
                status = lua_pcall(L,0,0,error_handler);
                pwd_pop();
                if (status) {
                        // stack: [error_handler, error string]
                        lua_pop(L,1); //message
                }
        }

        lua_pop(L,1); // error handler
        return 0;
TRY_END
}


static int global_alive (lua_State *L)
{
TRY_START
        check_args(L,1);
        void *ud = lua_touserdata(L,-1);
        if (lua_type(L,1)!=LUA_TUSERDATA) 
                luaL_typerror(L,1,"userdata");
        lua_pushboolean(L,*(void**)ud!=NULL);
        return 1;
TRY_END
}

static int global_mulqq (lua_State *L)
{
TRY_START
        check_args(L,8);
        GET_QUAT(a,1);
        GET_QUAT(b,5);
        PUT_QUAT(a*b);
        return 4;
TRY_END
}

static int global_mulqv (lua_State *L)
{
TRY_START
        check_args(L,7);
        GET_QUAT(a,1);
        GET_V3(b,5);
        PUT_V3(a*b);
        return 3;
TRY_END
}

static int global_mulvv (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_V3(a,1);
        GET_V3(b,4);
        PUT_V3(a*b);
        return 3;
TRY_END
}

static int global_mulnv (lua_State *L)
{
TRY_START
        check_args(L,4);
        lua_Number a = luaL_checknumber(L,1);
        GET_V3(b,2);
        PUT_V3(a*b);
        return 3;
TRY_END
}

static int global_mulnq (lua_State *L)
{
TRY_START
        check_args(L,5);
        lua_Number a = luaL_checknumber(L,1);
        GET_QUAT(b,2);
        PUT_QUAT(a*b);
        return 4;
TRY_END
}

static int global_addvv (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_V3(a,1);
        GET_V3(b,4);
        PUT_V3(a+b);
        return 3;
TRY_END
}

static int global_dot (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_V3(a,1);
        GET_V3(b,4);
        lua_pushnumber(L,a.dotProduct(b));
        return 1;
TRY_END
}

static int global_cross (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_V3(a,1);
        GET_V3(b,4);
        PUT_V3(a.crossProduct(b));
        return 3;
TRY_END
}

static int global_project (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_V3(a,1);
        GET_V3(b,4);
        PUT_V3(a - a.dotProduct(b) * b);
        return 3;
TRY_END
}

static int global_lenv (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_V3(a,1);
        lua_pushnumber(L,a.length());
        return 1;
TRY_END
}

static int global_lenq (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_QUAT(a,1);
        lua_pushnumber(L,a.w*a.w + a.x*a.x + a.y*a.y + a.z*a.z);
        return 1;
TRY_END
}

static int global_normv (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_V3(a,1);
        a.normalise();
        PUT_V3(a);
        return 3;
TRY_END
}

static int global_normq (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_QUAT(a,1);
        a.normalise();
        PUT_QUAT(a);
        return 4;
TRY_END
}

static int global_quat_angle (lua_State *L)
{
TRY_START
        check_args(L,4);
        lua_Number a = luaL_checknumber(L,1);
        GET_V3(b,2);
        PUT_QUAT(Ogre::Quaternion(Ogre::Degree(a), b));
        return 4;
TRY_END
}

static int global_quat_between (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_V3(a,1);
        GET_V3(b,4);
        PUT_QUAT(a.getRotationTo(b));
        return 4;
TRY_END
}


static const luaL_reg global[] = {
        {"mulqv",global_mulqv},
        {"mulvv",global_mulvv},
        {"mulqq",global_mulqq},
        {"mulnv",global_mulnv},
        {"mulnq",global_mulnq},
        {"addvv",global_addvv},
        {"dot",global_dot},
        {"cross",global_cross},
        {"project",global_project},
        {"lenv",global_lenv},
        {"lenq",global_lenq},
        {"normv",global_normv},
        {"normq",global_normq},
        {"quat_angle",global_quat_angle},
        {"quat_between",global_quat_between},

        {"fqn",global_fqn},
        {"fqn_ex",global_fqn_ex},
        {"path_stack",global_path_stack},
        {"path_stack_top",global_path_stack_top},
        {"path_stack_push_file",global_path_stack_push_file},
        {"path_stack_push_dir",global_path_stack_push_dir},
        {"path_stack_pop",global_path_stack_pop},
        {"include",global_include},
        {"safe_include",global_safe_include},
        {"error",global_error},
        {"error_handler",global_error_handler},
        {"alive",global_alive},
        {"echo",global_echo},
        {"console_poll",global_console_poll},

        {"pump",global_pump},
        {"clicked_close",global_clicked_close},
        {"have_focus",global_have_focus},

        {"get_keyb_presses",global_get_keyb_presses},
        {"keyb_flush",global_keyb_flush},
        {"set_keyb_verbose",global_set_keyb_verbose},
        {"get_keyb_verbose",global_get_keyb_verbose},
        {"get_mouse_events",global_get_mouse_events},
        {"set_mouse_pos",global_set_mouse_pos},
        {"get_mouse_hide",global_get_mouse_hide},
        {"set_mouse_hide",global_set_mouse_hide},
        {"get_mouse_grab",global_get_mouse_grab},
        {"set_mouse_grab",global_set_mouse_grab},

        {"render",global_render},
        {"fire_frame_started",global_fire_frame_started},
        {"fire_frame_rendering_queued",global_fire_frame_rendering_queued},
        {"fire_frame_ended",global_fire_frame_ended},
        {"update_elapsed_time",global_update_elapsed_time},
        {"get_rendersystem",global_get_rendersystem},
        {"set_vsync",global_rendersystem_set_vsync},
        {"get_vsync",global_rendersystem_get_vsync},
        {"reinitialise",global_rendersystem_reinitialise},

        {"get_hud_root",global_get_hud_root},
        {"get_main_win" ,global_get_main_win},

        {"sleep",global_sleep},
        {"get_clipboard",global_get_clipboard},
        {"set_clipboard",global_set_clipboard},

        {"load_material" ,global_load_material},
        {"get_all_materials",global_get_all_materials},
        {"get_material",global_get_material},
        {"get_material_budget" ,global_get_material_budget},
        {"set_material_budget" ,global_set_material_budget},
        {"get_material_usage" ,global_get_material_usage},
        {"unload_all_materials" ,global_unload_all_materials},
        {"unload_unused_materials" ,global_unload_unused_materials},
        {"remove_material",global_remove_material},

        {"load_texture" ,global_load_tex},
        {"get_all_textures" ,global_get_all_texs},
        {"get_texture" ,global_get_tex},
        {"get_texture_verbose" ,global_get_texture_verbose},
        {"set_texture_verbose" ,global_set_texture_verbose},
        {"get_texture_budget" ,global_get_texture_budget},
        {"set_texture_budget" ,global_set_texture_budget},
        {"get_texture_usage" ,global_get_texture_usage},
        {"unload_all_textures" ,global_unload_all_textures},
        {"unload_unused_textures" ,global_unload_unused_textures},
        {"remove_texture" ,global_remove_tex},

        {"load_mesh" ,global_load_mesh},
        {"get_all_meshes" ,global_get_all_meshes},
        {"get_mesh" ,global_get_mesh},
        {"get_mesh_verbose" ,global_get_mesh_verbose},
        {"set_mesh_verbose" ,global_set_mesh_verbose},
        {"get_mesh_budget" ,global_get_mesh_budget},
        {"set_mesh_budget" ,global_set_mesh_budget},
        {"get_mesh_usage" ,global_get_mesh_usage},
        {"unload_all_meshes" ,global_unload_all_meshes},
        {"unload_unused_meshes" ,global_unload_unused_meshes},
        {"remove_mesh" ,global_remove_mesh},

        {"load_skeleton" ,global_load_skel},
        {"get_all_skeletons" ,global_get_all_skels},
        {"get_skeleton" ,global_get_skel},
        {"get_skeleton_verbose" ,global_get_skel_verbose},
        {"set_skeleton_verbose" ,global_set_skel_verbose},
        {"get_skeleton_budget" ,global_get_skel_budget},
        {"set_skeleton_budget" ,global_set_skel_budget},
        {"get_skeleton_usage" ,global_get_skel_usage},
        {"unload_all_skeletons" ,global_unload_all_skels},
        {"unload_unused_skeletones" ,global_unload_unused_skels},
        {"remove_skeleton" ,global_remove_skel},

        {"make_gpuprog" ,global_make_gpuprog},
        {"get_all_gpuprogs" ,global_get_all_gpuprogs},
        {"get_gpuprog" ,global_get_gpuprog},
        {"get_gpuprog_verbose" ,global_get_gpuprog_verbose},
        {"set_gpuprog_verbose" ,global_set_gpuprog_verbose},
        {"get_gpuprog_budget" ,global_get_gpuprog_budget},
        {"set_gpuprog_budget" ,global_set_gpuprog_budget},
        {"get_gpuprog_usage" ,global_get_gpuprog_usage},
        {"unload_all_gpuprogs" ,global_unload_all_gpuprogs},
        {"unload_unused_gpuprogs" ,global_unload_unused_gpuprogs},
        {"remove_gpuprog" ,global_remove_gpuprog},

        {"register_material" ,global_register_material},
        {"dump_registered_material" ,global_dump_registered_material},
        {"registered_material_get" ,global_registered_material_get},
        {"registered_material_set" ,global_registered_material_set},
        {"reprocess_all_registered_materials" ,global_reprocess_all_registered_materials},

        {"get_alloc_stats" ,global_get_alloc_stats},
        {"set_alloc_stats" ,global_set_alloc_stats},
        {"reset_alloc_stats" ,global_reset_alloc_stats},

        {"get_in_queue_size" ,global_get_in_queue_size},
        {"get_out_queue_size" ,global_get_out_queue_size},
        {"give_queue_allowance" ,global_give_queue_allowance},
        {"handle_bastards" ,global_handle_bastards},

        {"mlockall" ,global_mlockall},
        {"munlockall" ,global_munlockall},

        {"profiler_start" ,global_profiler_start},
        {"profiler_stop" ,global_profiler_stop},

        {"add_font",global_add_font},
        {"text_width",global_text_width},
        {"text_pixel_substr",global_text_pixel_substr},
        {"text_wrap",global_text_wrap},

        {"PhysicsWorld",pworld_make},

        {"get_streamer",global_get_streamer},
        {"get_sm" ,global_get_sm},
        {"set_physics_debug" ,global_set_physics_debug},
        {"get_physics_debug" ,global_get_physics_debug},

        {"Timer",timer_make},
        {"Vector3",vector3_make},
        {"Quat",quat_make},
        {"Plot",plot_make},
        {"Spline",spline_make},
        {"StringDB",stringdb_make},
        {"SceneManager" ,global_make_scnmgr},
        {"Material" ,global_make_material},
        {"Texture" ,global_make_tex},

        {"get_scene_manager" ,global_get_scnmgr},
        {"get_render_target" ,global_get_rt},

        {"add_resource_location",global_add_resource_location},
        {"initialise_all_resource_groups",global_init_all_resource_groups},
        {"resource_exists",global_resource_exists},

        {NULL, NULL}
};

//}}}


// GENERAL CODE ============================================================ {{{

static int lua_panic(lua_State *L)
{
TRY_START
        lua_checkstack(L,2);
        if (lua_type(L,-1)==LUA_TTABLE) {
                lua_rawgeti(L,-1,2);
        }

        std::string err = luaL_checkstring(L,-1);
        CERR<<"PANIC! "<<err<<std::endl;
        app_fatal();
        return 0;
TRY_END
}

lua_State *init_lua(const char *filename)
{
        lua_State *L = lua_newstate(lua_alloc, NULL);
        lua_atpanic(L,lua_panic);

        luaL_openlibs(L);

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);
        int status;

/*
        lua_getglobal(L,"package");
        lua_pushstring(L,"./system/?.lua");
        lua_setfield(L,-2,"path");
        lua_pushstring(L,"");
        lua_setfield(L,-2,"cpath");
        lua_pop(L,1);

        lua_getglobal(L, "require");
        lua_pushstring(L, "ldb");
        // error handler should print stacktrace and stuff
        status = lua_pcall(L,1,1,error_handler);
        if (status) {
                lua_pop(L,1); //message
                CLOG<<"The most common cause of this is running the executable "
                      "from the wrong directory."<<std::endl;
                app_fatal();
        }
        lua_setfield(L, LUA_REGISTRYINDEX, "ldb");
*/


        #define ADD_MT_MACRO(name,tag) do {\
        luaL_newmetatable(L, tag); \
        luaL_register(L, NULL, name##_meta_table); \
        lua_pop(L,1); } while(0)

        ADD_MT_MACRO(pworld,PWORLD_TAG);
        ADD_MT_MACRO(rbody,RBODY_TAG);
        ADD_MT_MACRO(colmesh,COLMESH_TAG);
        ADD_MT_MACRO(streamer,STREAMER_TAG);
        ADD_MT_MACRO(gritcls,GRITCLS_TAG);
        ADD_MT_MACRO(gritobj,GRITOBJ_TAG);
        ADD_MT_MACRO(timer,TIMER_TAG);
        ADD_MT_MACRO(vector3,VECTOR3_TAG);
        ADD_MT_MACRO(quat,QUAT_TAG);
        ADD_MT_MACRO(spline,SPLINE_TAG);
        ADD_MT_MACRO(plot,PLOT_TAG);
        ADD_MT_MACRO(stringdb,STRINGDB_TAG);
        ADD_MT_MACRO(scnmgr,SCNMGR_TAG);
        ADD_MT_MACRO(node,NODE_TAG);
        ADD_MT_MACRO(cam,CAM_TAG);
        ADD_MT_MACRO(entity,ENTITY_TAG);
        ADD_MT_MACRO(manobj,MANOBJ_TAG);
        ADD_MT_MACRO(light,LIGHT_TAG);
        ADD_MT_MACRO(psys,PSYS_TAG);
        ADD_MT_MACRO(viewport,VIEWPORT_TAG);
        ADD_MT_MACRO(statgeo,STATGEO_TAG);
        ADD_MT_MACRO(instgeo,INSTGEO_TAG);
        ADD_MT_MACRO(pane,PANE_TAG);
        ADD_MT_MACRO(text,TEXT_TAG);
        ADD_MT_MACRO(mat,MAT_TAG);
        ADD_MT_MACRO(tex,TEX_TAG);
        ADD_MT_MACRO(mesh,MESH_TAG);
        ADD_MT_MACRO(skel,SKEL_TAG);
        ADD_MT_MACRO(gpuprog,GPUPROG_TAG);
        ADD_MT_MACRO(rtex,RTEX_TAG);
        ADD_MT_MACRO(rwin,RWIN_TAG);

        luaL_register(L, "_G", global);

        lua_pushthread(L); lua_setglobal(L,"MAIN_STATE");
        push_streamer(L, &grit->getStreamer()); lua_setglobal(L,"streamer");

        lua_utf8_init(L);

        status = aux_include(L,filename);
        if (status) {
                const char *str = lua_tostring(L,-1);
                CERR << "loading lua file: " << str << std::endl;
                lua_pop(L,1); // message
        } else {
                pwd_push_file(filename);
                // error handler should print stacktrace and stuff
                status = lua_pcall(L,0,0,error_handler);
                if (status) {
                        lua_pop(L,1); //message
                }
                pwd_pop();
        }
        lua_pop(L,1); //error handler

        return L;

}

//}}}



// vim: shiftwidth=8:tabstop=8:expandtab
