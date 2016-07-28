/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include <sleep.h>

#include <lua_stack.h>
#include <lua_utf8.h>
#include <io_util.h>

#include "core_option.h"
#include "grit_lua_util.h"
#include "input_filter.h"
#include "keyboard.h"
#include "mouse.h"
#if defined(__linux__) && defined(__JOYSTICK_DEVJS__)
#include "joystick.h"
#endif
#include "background_loader.h"
#include "clipboard.h"
#include <centralised_log.h>
#include "main.h"

#include "gfx/gfx_disk_resource.h"


#include "lua_wrappers_primitives.h"
#include "path_util.h"
#include "lua_wrappers_gritobj.h"
#include "lua_wrappers_disk_resource.h"

#include "gfx/lua_wrappers_gfx.h"
#include "audio/lua_wrappers_audio.h"
#include "physics/lua_wrappers_physics.h"
#include "net/lua_wrappers_net.h"
#include "navigation/lua_wrappers_navigation.h"

#define IFILTER_TAG "Grit/InputFilter"


// INPUT_FILTER  ========================================================== {{{

static int ifilter_make (lua_State *L)
{
TRY_START
    check_args(L, 2);
    double order = luaL_checknumber(L, 1);
    std::string desc = check_string(L, 2);
    InputFilter *self = new InputFilter(order, desc);
    push(L,self,IFILTER_TAG);
    return 1;
TRY_END
}

static int ifilter_tostring (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(InputFilter, self, 1, IFILTER_TAG);
    std::stringstream ss;
    ss << IFILTER_TAG << " " << static_cast<void*>(&self) << " (" << self.order << ") \"" << self.description << "\"";
    lua_pushstring(L, ss.str().c_str());
    return 1;
TRY_END
}

static int ifilter_gc (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(InputFilter, self, 1, IFILTER_TAG);
    if (self.isAlive()) self.destroy(L);
    delete &self;
    return 0;
TRY_END
}

static int ifilter_pressed (lua_State *L)
{
TRY_START
    check_args(L, 2);
    GET_UD_MACRO(InputFilter, self, 1, IFILTER_TAG);
    std::string button = check_string(L, 2);
    lua_pushboolean(L, self.isButtonPressed(button));
    return 1;
TRY_END
}

static int ifilter_bind (lua_State *L)
{
TRY_START
    if (lua_gettop(L) < 3) EXCEPT << "InputFilter.bind takes at least 3 arguments." << ENDL;
    while (lua_gettop(L) < 5) lua_pushnil(L);
    check_args(L, 5);
    GET_UD_MACRO(InputFilter, self, 1, IFILTER_TAG);
    std::string button = check_string(L, 2);
    self.bind(L, button);
    return 0;
TRY_END
}

static int ifilter_unbind (lua_State *L)
{
TRY_START
    check_args(L, 2);
    GET_UD_MACRO(InputFilter, self, 1, IFILTER_TAG);
    std::string button = check_string(L, 2);
    self.unbind(L, button);
    return 0;
TRY_END
}

static int ifilter_destroy (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(InputFilter, self, 1, IFILTER_TAG);
    self.destroy(L);
    return 0;
TRY_END
}

static int ifilter_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(InputFilter,self,1,IFILTER_TAG);
    std::string key  = luaL_checkstring(L,2);
    if (key=="order") {
        lua_pushnumber(L,self.order);
    } else if (key=="description") {
        push_string(L,self.description);
    } else if (key=="enabled") {
        lua_pushboolean(L,self.getEnabled());
    } else if (key=="modal") {
        lua_pushboolean(L,self.getModal());
    } else if (key=="mouseCapture") {
        lua_pushboolean(L,self.getMouseCapture());
    } else if (key=="bind") {
        push_cfunction(L, ifilter_bind);
    } else if (key=="unbind") {
        push_cfunction(L, ifilter_unbind);
    } else if (key=="pressed") {
        push_cfunction(L, ifilter_pressed);
    } else if (key=="destroy") {
        push_cfunction(L, ifilter_destroy);
    } else {
        my_lua_error(L,"Not a readable InputFilter member: "+key);
    }
    return 1;
TRY_END
}

static int ifilter_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(InputFilter,self,1,IFILTER_TAG);
    std::string key  = luaL_checkstring(L,2);
    if (key=="enabled") {
        bool v = check_bool(L, 3);
        self.setEnabled(L, v);
    } else if (key=="modal") {
        bool v = check_bool(L, 3);
        self.setModal(L, v);
    } else if (key=="mouseCapture") {
        bool v = check_bool(L, 3);
        self.setMouseCapture(L, v);
    } else if (key=="mouseMoveCallback") {
        if (lua_type(L, 3) != LUA_TFUNCTION) {
            EXCEPT << "mouseMoveCallback expects a function." << ENDL;
        }
        self.setMouseMoveCallback(L);
    } else {
        EXCEPT << "Not a writeable InputFilter member: " << key << ENDL;
    }
    return 0;
TRY_END
}

EQ_PTR_MACRO(InputFilter,ifilter,IFILTER_TAG)

MT_MACRO_NEWINDEX(ifilter);


//}}}


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
                lua_pushnumber(L,i+1);
                lua_pushstring(L,key);
                lua_settable(L,-3);
        }

        return 1;
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
                lua_pushnumber(L,i+1);
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

#if defined(__linux__) && defined(__JOYSTICK_DEVJS__)
static int global_get_joystick_events (lua_State *L)
{
TRY_START

        check_args(L,0);

        std::vector<signed char> buttons_indexes_and_values;
        std::vector<signed char> axes_indexes;
        std::vector<short int> axes_values; 

        bool moved = joystick->getEvents(&buttons_indexes_and_values,&axes_indexes,&axes_values);

        lua_pushboolean(L,moved);

        lua_createtable(L, buttons_indexes_and_values.size(), 0);
        for (unsigned int i=0 ; i<buttons_indexes_and_values.size() ; i++) {
                int button = buttons_indexes_and_values[i];
                lua_pushnumber(L,i+1);
                const char *button_ = "unknown";
                
                switch (button) {
                        case Joystick::JOYSTICK_BUTTON1: button_="+js01" ; break;
                        case -Joystick::JOYSTICK_BUTTON1: button_="-js01" ; break;
                        case Joystick::JOYSTICK_BUTTON2: button_="+js02" ; break;
                        case -Joystick::JOYSTICK_BUTTON2: button_="-js02" ; break;
                        case Joystick::JOYSTICK_BUTTON3: button_="+js03" ; break;
                        case -Joystick::JOYSTICK_BUTTON3: button_="-js03" ; break;
                        case Joystick::JOYSTICK_BUTTON4: button_="+js04" ; break;
                        case -Joystick::JOYSTICK_BUTTON4: button_="-js04" ;break;
                        case Joystick::JOYSTICK_BUTTON5: button_="+js05" ; break;
                        case -Joystick::JOYSTICK_BUTTON5: button_="-js05" ;break;
                        case Joystick::JOYSTICK_BUTTON6: button_="+js06" ; break;
                        case -Joystick::JOYSTICK_BUTTON6: button_="-js06" ; break;
                        case Joystick::JOYSTICK_BUTTON7: button_="+js07" ; break;
                        case -Joystick::JOYSTICK_BUTTON7: button_="-js07" ; break;
                        case Joystick::JOYSTICK_BUTTON8: button_="+js08" ; break;
                        case -Joystick::JOYSTICK_BUTTON8: button_="-js08" ; break;
                        case Joystick::JOYSTICK_BUTTON9: button_="+js09" ; break;
                        case -Joystick::JOYSTICK_BUTTON9: button_="-js09" ;break;
                        case Joystick::JOYSTICK_BUTTON10: button_="+js10" ; break;
                        case -Joystick::JOYSTICK_BUTTON10: button_="-js10" ;break;
                        case Joystick::JOYSTICK_BUTTON11: button_="+js11" ; break;
                        case -Joystick::JOYSTICK_BUTTON11: button_="-js11" ;break;
                }

                lua_pushstring(L,button_);
                lua_settable(L,-3);
        }

        if( axes_indexes.size() == axes_values.size() )
        {
           lua_createtable(L, axes_indexes.size(), 0);
           for (unsigned int i=0 ; i<axes_indexes.size() ; i++) {
                double axe_index = axes_indexes[i];
                double axe_value = axes_values[i];
                double pushvalue = (axe_value/32767.0);  
                lua_pushnumber(L,i+1);
                lua_pushvector2(L,axe_index,pushvalue);
                lua_settable(L,-3);
           }

           return 3;
        }
        return 2;
TRY_END
}
#endif // defined(__linux__) && defined(__JOYSTICK_DEVJS__)

static int global_input_filter_trickle_mouse_move (lua_State *L)
{
TRY_START
    check_args(L,2);
    Vector2 rel = check_v2(L, 1);
    Vector2 abs = check_v2(L, 2);
    input_filter_trickle_mouse_move(L, rel, abs);
    return 0;
TRY_END
}

static int global_input_filter_trickle_button (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *str = luaL_checkstring(L, 1);
    input_filter_trickle_button(L, str);
    return 0;
TRY_END
}

static int global_input_filter_pressed (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *str = luaL_checkstring(L, 1);
    lua_pushboolean(L, input_filter_pressed(str));
    return 1;
TRY_END
}

static int global_input_filter_flush (lua_State *L)
{
TRY_START
    check_args(L,0);
    input_filter_flush(L);
    return 0;
TRY_END
}

static int global_input_filter_map (lua_State *L)
{
TRY_START
    check_args(L,0);
    std::vector<std::pair<double,std::string>> filters = input_filter_list();
    lua_createtable(L, filters.size(), 0);
    int top_table = lua_gettop(L);
    for (unsigned int i=0 ; i<filters.size() ; i++) {
        lua_pushnumber(L, filters[i].first);
        push_string(L, filters[i].second);
        lua_settable(L, top_table);
    }
    return 1;
TRY_END
}

static int global_input_filter_get_cursor_hidden (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_pushboolean(L, input_filter_get_cursor_hidden());
    return 1;
TRY_END
}

static int global_input_filter_set_cursor_hidden (lua_State *L)
{
TRY_START
    check_args(L,1);
    bool v = check_bool(L, 1);
    input_filter_set_cursor_hidden(v);
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
        float amount = check_float(L,1);
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

static int global_get_clipboard_selection (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushstring(L,clipboard_selection_get().c_str());
        return 1;
TRY_END
}

static int global_set_clipboard_selection (lua_State *L)
{
TRY_START
        check_args(L,1);
        std::string s = luaL_checkstring(L,1);
        clipboard_selection_set(s);
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


static int global_print (lua_State *L)
{
TRY_START
    std::stringstream ss;
    int args = lua_gettop(L);
    for (int i=1 ; i<=args ; ++i) {
        if (i > 1) ss << "\t";
        if (lua_isstring(L, i)) {
            ss << lua_tostring(L, i);
        } else {
            lua_pushglobaltable(L);
            lua_getfield(L, -1, "dump");
            lua_remove(L, -2); //global table
            lua_pushvalue(L,i);
            lua_call(L, 1, 1);
            ss << lua_tostring(L, -1);
            lua_pop(L, 1);
        }
    }
    clog.print(ss.str());
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
        return lua_load(L, aux_aux_include, &lis, ("@"+filename).c_str(), "bt"); //last arg means it will accept either binary or text files
}


static int global_import_str (lua_State *L)
{
TRY_START
        check_args(L, 1);
        std::string filename = luaL_checkstring(L, 1);
        std::string fname(filename, 1); // strip leading /

        if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("GRIT", fname))
                my_lua_error(L, "File not found: \"" + filename + "\"");

        Ogre::DataStreamPtr ds
                = Ogre::ResourceGroupManager::getSingleton().openResource(fname, "GRIT");
        push_string(L, ds->getAsString());

        return 1;
TRY_END
}

static int global_include (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string filename = check_path(L, 1);

    // stack: []
    int status = aux_include(L,filename);
    if (status) {
        // stack: [error_string]
        const char *str = lua_tostring(L, -1);
        // call error function manually, lua will not do this for us in lua_load
        my_lua_error(L, str);
    } else {
        // stack: [function]
        lua_call(L, 0, 0);
        // stack: []
    }
    // stack: []

    return 0;
TRY_END
}

static int global_safe_include (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string filename = check_path(L, 1);

    bool successful;

    // stack: []
    int status = aux_include(L,filename);
    if (status) {
        successful = false;
        // stack: [error_string]
        // call error function manually, lua will not do this for us in lua_load
        // (but we want to fail silently if it's a file not found)
        if (status!=LUA_ERRFILE) {
            const char *str = lua_tostring(L,-1);
            my_lua_error(L,str);
        }
        // stack: [error_string]
        lua_pop(L,1);
        // stack: []
    } else {
        successful = true;
        // stack: [function]
        lua_call(L, 0, 0);
        // stack: []
    }
    // stack: []

    lua_pushboolean(L, successful);
    return 1;
TRY_END
}

static int global_current_dir (lua_State *L)
{
    int level = 1;
    if (lua_gettop(L) == 1) {
        level = check_t<int>(L, 1);
    } else {
        check_args(L, 0);
    }
    std::string dir = lua_current_dir(L, level);
    push_string(L, dir);
    return 1;
}

int global_core_option (lua_State *L)
{
TRY_START
    if (lua_gettop(L)==2) {
        std::string opt = check_string(L,1);
        int t;
        CoreBoolOption o0;
        CoreIntOption o1;
        CoreFloatOption o2;
        core_option_from_string(opt, t, o0, o1, o2);
        switch (t) {
            case -1: my_lua_error(L,"Unrecognised core option: \""+opt+"\"");
            case 0: core_option(o0, check_bool(L,2)); break;
            case 1: core_option(o1, check_t<int>(L,2)); break;
            case 2: core_option(o2, check_float(L,2)); break;
            default: my_lua_error(L,"Unrecognised type from core_option_from_string");
        }
        return 0;
    } else {
        check_args(L,1);
        std::string opt = check_string(L,1);
        int t;
        CoreBoolOption o0;
        CoreIntOption o1;
        CoreFloatOption o2;
        core_option_from_string(opt, t, o0, o1, o2);
        switch (t) {
            case -1: my_lua_error(L,"Unrecognised core option: \""+opt+"\"");
            case 0: lua_pushboolean(L,core_option(o0)); break;
            case 1: lua_pushnumber(L,core_option(o1)); break;
            case 2: lua_pushnumber(L,core_option(o2)); break;
            default: my_lua_error(L,"Unrecognised type from core_option_from_string");
        }
        return 1;
    }
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

    {"core_option", global_core_option},
    {"import_str", global_import_str},
    {"include", global_include},
    {"safe_include", global_safe_include},
    {"current_dir", global_current_dir},
    {"error", global_error},
    {"error_handler", global_error_handler},
    {"print", global_print},
    {"console_poll", global_console_poll},

    {"clicked_close", global_clicked_close},
    {"have_focus", global_have_focus},

    {"get_keyb_presses", global_get_keyb_presses},
    {"set_keyb_verbose", global_set_keyb_verbose},
    {"get_keyb_verbose", global_get_keyb_verbose},
    {"get_mouse_events", global_get_mouse_events},
#if defined(__linux__) && defined(__JOYSTICK_DEVJS__)
    {"get_joystick_events", global_get_joystick_events},
#endif
    {"micros", global_micros},
    {"seconds", global_seconds},
    {"sleep_seconds", global_sleep_seconds},
    {"sleep_micros", global_sleep_micros},
    {"sleep", global_sleep_micros},
    {"get_clipboard", global_get_clipboard},
    {"set_clipboard", global_set_clipboard},
    {"get_clipboard_selection", global_get_clipboard_selection},
    {"set_clipboard_selection", global_set_clipboard_selection},

    {"get_alloc_stats", global_get_alloc_stats},
    {"set_alloc_stats", global_set_alloc_stats},
    {"reset_alloc_stats", global_reset_alloc_stats},

    {"get_in_queue_size", global_get_in_queue_size},
    {"get_out_queue_size_gpu", global_get_out_queue_size_gpu},
    {"get_out_queue_size_host", global_get_out_queue_size_host},
    {"give_queue_allowance", global_give_queue_allowance},
    {"handle_bastards", global_handle_bastards},
    {"check_ram_gpu", global_check_ram_gpu},
    {"check_ram_host", global_check_ram_host},

    {"mlockall", global_mlockall},
    {"munlockall", global_munlockall},

    {"profiler_start", global_profiler_start},
    {"profiler_stop", global_profiler_stop},

    {"input_filter_trickle_button", global_input_filter_trickle_button},
    {"input_filter_trickle_mouse_move", global_input_filter_trickle_mouse_move},
    {"input_filter_pressed", global_input_filter_pressed},
    {"input_filter_flush", global_input_filter_flush},
    {"input_filter_map", global_input_filter_map},
    {"input_filter_get_cursor_hidden", global_input_filter_get_cursor_hidden},
    {"input_filter_set_cursor_hidden", global_input_filter_set_cursor_hidden},

    {"InputFilter", ifilter_make},
    {"PlotV3", plot_v3_make},
    {"Plot", plot_make},
    {"StringDB", stringdb_make},

    {"future_event", global_future_event},
    {"clear_events", global_clear_events},
    {"dump_events", global_dump_events},
    {"do_events", global_do_events},

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

void init_lua (const char *filename, const std::vector<std::string> &args, lua_State *&L)
{
    L = lua_newstate(lua_alloc, NULL);
    lua_atpanic(L,lua_panic);

    luaL_openlibs(L);

    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    ADD_MT_MACRO(ifilter,IFILTER_TAG);
    ADD_MT_MACRO(plot,PLOT_TAG);
    ADD_MT_MACRO(plot_v3,PLOT_V3_TAG);
    ADD_MT_MACRO(stringdb,STRINGDB_TAG);

    lua_getglobal(L, "print");
    lua_setglobal(L, "print_stdout");

    register_lua_globals(L, global);

    utf8_lua_init(L);
    gritobj_lua_init(L);
    gfx_lua_init(L);
    physics_lua_init(L);
    audio_lua_init(L);
    disk_resource_lua_init(L);
    net_lua_init(L);
	navigation_lua_init(L);

    int status = aux_include(L, filename);
    if (status) {
        std::string str = lua_tostring(L,-1);
        lua_pop(L,1); // message
        EXCEPT << "Loading Lua file: " << str << std::endl;
    } else {
        check_stack(L, args.size());
        for (const auto &arg : args) {
            push_string(L, arg);
        }
        // error handler should print stacktrace and stuff
        status = lua_pcall(L, args.size(), 0, error_handler);
        if (status) {
            lua_pop(L,1); //message
            EXCEPT << "Error running init script." << ENDL;
        }
    }
    lua_pop(L,1); //error handler
}

void shutdown_lua (lua_State *L)
{
    lua_close(L);
    func_map_leak_all();
}

//}}}
