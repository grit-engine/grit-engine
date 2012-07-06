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

#ifdef USE_GOOGLE_PERF_TOOLS
        #include <google/profiler.h>
#endif

#ifndef WIN32
        #include <sys/mman.h>
#endif

#include "Keyboard.h"
#include "Mouse.h"
#include "BackgroundLoader.h"
#include "sleep.h"
#include "clipboard.h"
#include "CentralisedLog.h"
#include "main.h"
#include "lua_utf8.h"
#include "gfx/GfxDiskResource.h"


#include "lua_wrappers_primitives.h"
#include "path_util.h"
#include "lua_wrappers_gritobj.h"
#include "lua_wrappers_disk_resource.h"

#include "gfx/lua_wrappers_gfx.h"
#include "audio/lua_wrappers_audio.h"
#include "physics/lua_wrappers_physics.h"


// GLOBAL LIBRARY ========================================================== {{{

static int global_have_focus (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,keyboard->hasFocus());
        return 1;
TRY_END
}

static int global_get_keyb_presses (lua_State *L)
{
TRY_START
        check_args(L,0);
        Keyboard::Presses presses = keyboard->getPresses();

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
                keyboard->flush(key);
        } else {
                check_args(L,0);
                keyboard->flush();
        }
        return 0;
TRY_END
}

static int global_get_keyb_verbose (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,keyboard->getVerbose());
        return 1;
TRY_END
}

static int global_set_keyb_verbose (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        keyboard->setVerbose(b);
        return 0;
TRY_END
}

static int global_get_mouse_events (lua_State *L)
{
TRY_START
        check_args(L,0);

        int rel_x, rel_y, x, y;
        std::vector<int> clicks;
        bool moved = mouse->getEvents(&clicks,&x,&y,&rel_x,&rel_y);

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
        mouse->setPos(x,y);
        return 0;
TRY_END
}

static int global_get_mouse_hide (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,mouse->getHide());
        return 1;
TRY_END
}

static int global_get_mouse_grab (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,mouse->getGrab());
        return 1;
TRY_END
}

static int global_set_mouse_hide (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        mouse->setHide(b);
        return 0;
TRY_END
}

static int global_set_mouse_grab (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        mouse->setGrab(b);
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
        size_t sz = bgl->size();
        lua_pushnumber(L, sz);
        return 1;
TRY_END
}

static int global_get_out_queue_size_gpu (lua_State *L)
{
TRY_START
        check_args(L,0);
        size_t sz = bgl->getLRUQueueSizeGPU();
        lua_pushnumber(L, sz);
        return 1;
TRY_END
}

static int global_get_out_queue_size_host (lua_State *L)
{
TRY_START
        check_args(L,0);
        size_t sz = bgl->getLRUQueueSizeHost();
        lua_pushnumber(L, sz);
        return 1;
TRY_END
}


static int global_give_queue_allowance (lua_State *L)
{
TRY_START
        check_args(L,1);
        float amount = luaL_checknumber(L,1);
        bgl->setAllowance(amount);
        return 0;
TRY_END
}


static int global_handle_bastards (lua_State *L)
{
TRY_START
        check_args(L,0);
        bgl->handleBastards();
        return 0;
TRY_END
}

static int global_check_ram_gpu (lua_State *L)
{
TRY_START
        check_args(L,0);
        bgl->checkRAMGPU();
        return 0;
TRY_END
}

static int global_check_ram_host (lua_State *L)
{
TRY_START
        check_args(L,0);
        bgl->checkRAMHost();
        return 0;
TRY_END
}


static int global_get_streamer (lua_State *L)
{
TRY_START
        check_args(L,0);
        push_streamer(L,streamer);
        return 1;
TRY_END
}

static int global_clicked_close (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,clicked_close);
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


static int global_sleep_micros (lua_State *L)
{
TRY_START
        check_args(L,1);
        lua_Number micros = luaL_checknumber(L,1);
        mysleep((long)micros);
        return 0;
TRY_END
}

static int global_sleep_seconds (lua_State *L)
{
TRY_START
        check_args(L,1);
        lua_Number secs = luaL_checknumber(L,1);
        mysleep((long)(1E6*secs));
        return 0;
TRY_END
}

static int global_micros (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,micros());
        return 1;
TRY_END
}

static int global_seconds (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,micros()/1000000.0);
        return 1;
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
                // call error function manually, lua will not do this for us in lua_load
                my_lua_error(L,str);
        } else {
                pwd_push_file(filename);
                status = lua_pcall(L,0,0,error_handler);
                pwd_pop();
                if (status) {
                        //my_lua_error(L, "Failed to include file.");
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
                // call error function manually, lua will not do this for us in lua_load
                // (but we want to fail silently if it's a file not found)
                if (status!=LUA_ERRFILE) {
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

static lua_Number game_time = 0;
typedef std::map<lua_Number, std::vector<LuaPtr*> > EventMap;
static EventMap event_map;

static void add_event (lua_State *L, lua_Number countdown)
{
        LuaPtr *lp = new LuaPtr();
        lp->setNoPop(L);
        event_map[countdown + game_time].push_back(lp);
}

static int global_future_event (lua_State *L)
{
TRY_START
        check_args(L,2);
        lua_Number countdown = luaL_checknumber(L,1);
        if (!lua_isfunction(L,2)) my_lua_error(L, "Argument 2 must be a function.");
        add_event(L, countdown);
        return 0;
TRY_END
}

static int global_clear_events (lua_State *L)
{
TRY_START
        check_args(L,0);
        for (EventMap::iterator i=event_map.begin(),i_=event_map.end() ; i!=i_ ; ++i) {
                for (unsigned j=0 ; j<i->second.size() ; ++j) {
                        i->second[j]->setNil(L);
                        delete i->second[j];
                }
        }
        event_map.clear();
        return 0;
TRY_END
}

static int global_dump_events (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_newtable(L);
        int counter = 1;
        for (EventMap::iterator i=event_map.begin(),i_=event_map.end() ; i!=i_ ; ++i) {
                for (unsigned j=0 ; j<i->second.size() ; ++j) {
                        i->second[j]->push(L);
                        lua_rawseti(L, -2, counter++);
                        lua_pushnumber(L, i->first);
                        lua_rawseti(L, -2, counter++);
                }
        }
        return 1;
TRY_END
}

static int global_do_events (lua_State *L)
{
TRY_START
        check_args(L,1);
        lua_Number elapsed = luaL_checknumber(L,1);
        
        lua_pushcfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        game_time += elapsed;
        do {
                // stack: eh
                EventMap::iterator i = event_map.begin();
                if (i==event_map.end()) break;
                lua_Number time = i->first;
                std::vector<LuaPtr*> &events = i->second;
                if (time > game_time) break;
                for (unsigned j=0 ; j<events.size() ; ++j) {
                        // stack: eh
                        LuaPtr *func = events[j];
                        func->push(L);
                        // stack: eh, func
                        int status = lua_pcall(L, 0, 1, error_handler);
                        if (status) {
                                lua_pop(L,1); // error msg
                        } else {
                                // stack: eh, r
                                if (!lua_isnil(L,-1)) {
                                        if (!lua_isnumber(L,-1)) {
                                                CERR << "Return type of event must be number or nil." << std::endl;
                                        } else {
                                                lua_Number r = lua_tonumber(L,-1);
                                                func->push(L);
                                                add_event(L, r);
                                                lua_pop(L,1);
                                        }
                                        // stack: eh, r;
                                }
                                lua_pop(L, 1);
                                // stack: eh
                        }
                        func->setNil(L);
                        delete func;
                        // stack: eh
                }
                event_map.erase(i);
        } while (true);
        return 0;
TRY_END
}



static const luaL_reg global[] = {

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

        {"micros" ,global_micros},
        {"seconds" ,global_seconds},
        {"sleep_seconds",global_sleep_seconds},
        {"sleep_micros",global_sleep_micros},
        {"sleep",global_sleep_micros},
        {"get_clipboard",global_get_clipboard},
        {"set_clipboard",global_set_clipboard},

        {"get_alloc_stats" ,global_get_alloc_stats},
        {"set_alloc_stats" ,global_set_alloc_stats},
        {"reset_alloc_stats" ,global_reset_alloc_stats},

        {"get_in_queue_size" ,global_get_in_queue_size},
        {"get_out_queue_size_gpu" ,global_get_out_queue_size_gpu},
        {"get_out_queue_size_host" ,global_get_out_queue_size_host},
        {"give_queue_allowance" ,global_give_queue_allowance},
        {"handle_bastards" ,global_handle_bastards},
        {"check_ram_gpu" ,global_check_ram_gpu},
        {"check_ram_host" ,global_check_ram_host},

        {"mlockall" ,global_mlockall},
        {"munlockall" ,global_munlockall},

        {"profiler_start" ,global_profiler_start},
        {"profiler_stop" ,global_profiler_stop},

        {"get_streamer",global_get_streamer},

        {"PlotV3",plot_v3_make},
        {"Plot",plot_make},
        {"StringDB",stringdb_make},

        {"future_event",global_future_event},
        {"clear_events",global_clear_events},
        {"dump_events",global_dump_events},
        {"do_events",global_do_events},

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

        ADD_MT_MACRO(streamer,STREAMER_TAG);
        ADD_MT_MACRO(gritcls,GRITCLS_TAG);
        ADD_MT_MACRO(gritobj,GRITOBJ_TAG);
        ADD_MT_MACRO(plot,PLOT_TAG);
        ADD_MT_MACRO(plot_v3,PLOT_V3_TAG);
        ADD_MT_MACRO(stringdb,STRINGDB_TAG);

        luaL_register(L, "_G", global);

        lua_pushthread(L); lua_setglobal(L,"MAIN_STATE");
        push_streamer(L, streamer); lua_setglobal(L,"streamer");

        lua_utf8_init(L);

        gfx_lua_init(L);
        physics_lua_init(L);
		audio_lua_init(L);
        disk_resource_lua_init(L);

        status = aux_include(L,filename);
        if (status) {
                const char *str = lua_tostring(L,-1);
                CERR << "loading lua file: " << str << std::endl;
                lua_pop(L,1); // message
                app_fatal();
        } else {
                pwd_push_file(filename);
                // error handler should print stacktrace and stuff
                status = lua_pcall(L,0,0,error_handler);
                pwd_pop();
                if (status) {
                        lua_pop(L,1); //message
                        app_fatal();
                }
        }
        lua_pop(L,1); //error handler

        return L;

}

void shutdown_lua (lua_State *L)
{
        func_map_shutdown(L);
        lua_close(L);
}

//}}}



// vim: shiftwidth=8:tabstop=8:expandtab
