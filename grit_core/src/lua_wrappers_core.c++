#include <OgreFont.h>
#include <OgreFontManager.h>

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
#include "matbin.h"
#include "sleep.h"
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



// GLOBAL LIBRARY ========================================================== {{{

static int global_get_keyb_presses (lua_State *L)
{
TRY_START
        check_args(L,0);
        Keyboard::Presses presses = grit->getKeyboard()->getPresses();

        lua_createtable(L, presses.size(), 0);
        for (unsigned int i=0 ; i<presses.size() ; i++) {
                const char *key = presses[i];
                lua_pushnumber(L,i+LUA_ARRAY_BASE);
                lua_pushstring(L,key);
                lua_settable(L,-3);
        }

        return 1;
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
        int x = (int)check_int(L,1,INT_MIN,INT_MAX);
        int y = (int)check_int(L,2,INT_MIN,INT_MAX);
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
        bool b = 0!=lua_toboolean(L,1);
        grit->getMouse()->setHide(b);
        return 0;
TRY_END
}

static int global_set_mouse_grab (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = 0!=lua_toboolean(L,1);
        grit->getMouse()->setGrab(b);
        return 0;
TRY_END
}

static int global_render (lua_State *L)
{
TRY_START
        grit->render();
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

////////////////////////////////////////////////////////////////////////////////

static int global_make_tex (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) lua_pushnumber(L,Ogre::MIP_DEFAULT);
        if (lua_gettop(L)==5) lua_pushnumber(L,Ogre::TU_DEFAULT);
        check_args(L,7);
        const char *name = luaL_checkstring(L,1);
        const char *texType = luaL_checkstring(L,2);
        unsigned int width = (unsigned int)check_int(L,3,1,UINT_MAX);
        unsigned int height = (unsigned int)check_int(L,4,1,UINT_MAX);
        unsigned int depth = (unsigned int)check_int(L,5,1,UINT_MAX);
        unsigned int mipmaps = (unsigned int)check_int(L,6,0,UINT_MAX);
        unsigned int usage = (unsigned int)check_int(L,7,0,UINT_MAX);

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
        bool b = 0!=lua_toboolean(L,1);
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
        size_t n = (size_t) check_int(L,1,0,
                                      std::numeric_limits<std::size_t>::max());
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
        bool b = 0!=lua_toboolean(L,1);
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
        size_t n = (size_t) check_int(L,1,0,
                                      std::numeric_limits<std::size_t>::max());
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
        size_t n = (size_t) check_int(L,1,0,
                                      std::numeric_limits<std::size_t>::max());
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

// make

static int global_load_gpuprog (lua_State *L)
{
TRY_START
        check_args(L,4);
        const char *name = luaL_checkstring(L,1);
        const char *filename = luaL_checkstring(L,2);
        std::string typestr = luaL_checkstring(L,3);
        const char *syntax = luaL_checkstring(L,4);
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
        Ogre::GpuProgramPtr t = Ogre::GpuProgramManager::getSingleton()
                                .load(name,"GRIT", filename, type, syntax);
        push(L, new Ogre::GpuProgramPtr(t),GPUPROG_TAG);
        return 1;
TRY_END
}

static int global_get_all_gpuprogs (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::GpuProgramManager::ResourceMapIterator rmi =
                Ogre::GpuProgramManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::GpuProgramPtr &self = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::GpuProgramPtr(self), GPUPROG_TAG);
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
        Ogre::GpuProgramPtr m =
                Ogre::GpuProgramManager::getSingleton().getByName(name);
        if (m.isNull()) {
                lua_pushnil(L);
                return 1;
        }
        push(L,new Ogre::GpuProgramPtr(m),GPUPROG_TAG);
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
        size_t n = (size_t) check_int(L,1,0,
                                      std::numeric_limits<std::size_t>::max());
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
        bool b = 0!=lua_toboolean(L,1);
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

static int global_reset_alloc_stats (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_alloc_stats_reset();
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
        bool b = 0!=lua_toboolean(L,1);
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


static int global_get_gom (lua_State *L)
{
TRY_START
        check_args(L,0);
        push_gom(L,&grit->getGOM());
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

static int global_read_matbin (lua_State *L)
{
TRY_START
        check_args(L,2);
        const char *fname = luaL_checkstring(L,1);
        Ogre::DataStreamPtr f = Ogre::ResourceGroupManager::getSingleton()
                .openResource(fname);
        std::vector<struct mat_bin> ms = read_matbins(f);

        for (size_t i=0 ; i<ms.size() ; i++) {
                struct mat_bin& m = ms[i];

                lua_pushvalue(L,2);
                lua_pushstring(L,m.name.c_str());
                lua_pushnumber(L,((m.colour >> 0) & 0xFF) / 255.0);
                lua_pushnumber(L,((m.colour >> 8) & 0xFF) / 255.0);
                lua_pushnumber(L,((m.colour >> 16) & 0xFF) / 255.0);
                lua_pushnumber(L,((m.colour >> 24) & 0xFF) / 255.0);
                if (m.tname=="")
                        lua_pushnil(L);
                else
                        lua_pushstring(L,m.tname.c_str());
                lua_pushboolean(L,m.flags & 0x1);
                lua_pushboolean(L,m.flags & 0x2);
                lua_pushboolean(L,m.flags & 0x4);
                lua_pushboolean(L,m.flags & 0x8);
                lua_pushboolean(L,m.flags & 0x10);
                lua_pushboolean(L,m.flags & 0x20);
                lua_pushboolean(L,m.flags & 0x40);
                lua_pushboolean(L,m.flags & 0x80);
                lua_call(L,14,0);
        }
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
        bool wordwrap = 0!=lua_toboolean(L,5);
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
        lua_Number lines_ = check_int(L,3,0,UINT_MAX);
        unsigned int lines = (unsigned int)lines_;
        bool word_wrap = 0!=lua_toboolean(L,4);
        bool chop_top = 0!=lua_toboolean(L,5);
        lua_Number tabs_ = check_int(L,6,0,UINT_MAX);
        unsigned int tabs = (unsigned int)tabs_;
        bool codes = 0!=lua_toboolean(L,7);
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
        long n = (long)check_int(L, 1, LONG_MIN, LONG_MAX);
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
        bool recursive = 0!=lua_toboolean(L,3);
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
                level = (unsigned long)check_int(L,2,0,ULONG_MAX);
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
        lua_Number tex_width = check_int(L,3,0,UINT_MAX);
        lua_Number tex_height = check_int(L,4,0,UINT_MAX);
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
                lua_Number codepoint = check_int(L,-2,0,4294967295.0);
                lua_rawgeti(L, -1, 1);
                lua_Number x = check_int(L,-1,0,UINT_MAX);
                lua_pop(L,1);
                lua_rawgeti(L, -1, 2);
                lua_Number y = check_int(L,-1,0,UINT_MAX);
                lua_pop(L,1);
                lua_rawgeti(L, -1, 3);
                lua_Number w = check_int(L,-1,0,UINT_MAX);
                lua_pop(L,1);
                lua_rawgeti(L, -1, 4);
                lua_Number h = check_int(L,-1,0,UINT_MAX);
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
                my_lua_error(L,"mlockall not supported on Windows.");
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
                my_lua_error(L,"mlockall not supported on Windows.");
        #else
                munlockall();
        #endif
        return 0;
TRY_END
}


static int global_include (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char* filename = luaL_checkstring(L,1);

        int status = luaL_loadfile(L,filename);
        if (status) {
                const char *str = lua_tostring(L,-1);
                my_lua_error(L,str);
        } else {
                // existing error handler should print stacktrace and stuff
                lua_call(L,0,0);
        }

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


static const luaL_reg global[] = {
        {"include",global_include},
        {"error",global_error},
        {"error_handler",global_error_handler},
        {"alive",global_alive},
        {"echo",global_echo},
        {"console_poll",global_console_poll},

        {"pump",global_pump},
        {"clicked_close",global_clicked_close},

        {"get_keyb_presses",global_get_keyb_presses},
        {"get_mouse_events",global_get_mouse_events},
        {"set_mouse_pos",global_set_mouse_pos},
        {"get_mouse_hide",global_get_mouse_hide},
        {"set_mouse_hide",global_set_mouse_hide},
        {"get_mouse_grab",global_get_mouse_grab},
        {"set_mouse_grab",global_set_mouse_grab},

        {"render",global_render},
        {"set_vsync",global_rendersystem_set_vsync},
        {"get_vsync",global_rendersystem_get_vsync},
        {"reinitialise",global_rendersystem_reinitialise},

        {"get_hud_root",global_get_hud_root},
        {"get_main_win" ,global_get_main_win},

        {"sleep",global_sleep},

        {"read_matbin",global_read_matbin},

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

        {"load_gpuprog" ,global_load_gpuprog},
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

        {"get_alloc_stats" ,global_get_alloc_stats},
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

        {"get_gom",global_get_gom},

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

        lua_getglobal(L,"package");
        lua_pushstring(L,"./lua_base/?.lua");
        lua_setfield(L,-2,"path");
        lua_pushstring(L,"");
        lua_setfield(L,-2,"cpath");
        lua_pop(L,1);

        lua_getglobal(L, "require");
        lua_pushstring(L, "ldb");
        lua_call(L, 1, 1);  /* If this fails, we can't go on */
        lua_setfield(L, LUA_REGISTRYINDEX, "ldb");


        #define ADD_MT_MACRO(name,tag) do {\
        luaL_newmetatable(L, tag); \
        luaL_register(L, NULL, name##_meta_table); \
        lua_pop(L,1); } while(0)

        ADD_MT_MACRO(pworld,PWORLD_TAG);
        ADD_MT_MACRO(rbody,RBODY_TAG);
        ADD_MT_MACRO(colmesh,COLMESH_TAG);
        ADD_MT_MACRO(gom,GOM_TAG);
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
        ADD_MT_MACRO(viewport,VIEWPORT_TAG);
        ADD_MT_MACRO(statgeo,STATGEO_TAG);
        ADD_MT_MACRO(instgeo,INSTGEO_TAG);
        ADD_MT_MACRO(pane,PANE_TAG);
        ADD_MT_MACRO(text,TEXT_TAG);
        ADD_MT_MACRO(mat,MAT_TAG);
        ADD_MT_MACRO(tex,TEX_TAG);
        ADD_MT_MACRO(mesh,MESH_TAG);
        ADD_MT_MACRO(gpuprog,GPUPROG_TAG);
        ADD_MT_MACRO(rtex,RTEX_TAG);
        ADD_MT_MACRO(rwin,RWIN_TAG);

        luaL_register(L, "_G", global);

        lua_pushthread(L);
        lua_setglobal(L,"MAIN_STATE");

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        int status = luaL_loadfile(L,filename);
        if (status) {
                const char *str = lua_tostring(L,-1);
                CERR << "loading lua file: " << str << std::endl;
                lua_pop(L,1); // message
        } else {
                // error handler should print stacktrace and stuff
                status = lua_pcall(L,0,0,error_handler);
                if (status) {
                        lua_pop(L,1); //message
                }
        }
        lua_pop(L,1); //error handler

        return L;

}

//}}}



// vim: shiftwidth=8:tabstop=8:expandtab
