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

//#include <fenv.h>
#include <cerrno>
#include <sstream>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#  include <windows.h>
#define DIRECT3D_VERSION 0x0900
//#  include <d3d9.h>
#  include "win32/MouseDirectInput8.h"
#  include "win32/KeyboardDirectInput8.h"
#  include "win32/KeyboardWinAPI.h"
#else
#  include "linux/KeyboardX11.h"
#  include "linux/MouseX11.h"
#endif

#include "main.h"
#include "CentralisedLog.h"


#include "grit_lua_util.h"
#include "lua_wrappers_core.h"

#include "gfx/gfx.h"

#include "physics/PhysicsWorld.h"

#include "audio/audio.h"

#include "net/net.h"


CentralisedLog clog;
bool clicked_close = false;
Mouse *mouse = NULL;
Keyboard *keyboard = NULL;
lua_State *core_L = NULL;
UserDataTables user_data_tables;
BulletDebugDrawer *debug_drawer = NULL;
std::stringstream gfx_msg_buffer;
HUD::RootPtr hud;
BackgroundLoader *bgl;

struct TheGfxCallback : GfxCallback {

        virtual void clickedClose (void)
        {
                clicked_close = true;
        }

        virtual void windowResized (int width, int height)
        {
                hud->parentResized(width, height);
        }
        
        virtual void messageLogged (const std::string &line)
        {
                std::cout << line << RESET << std::endl;
                gfx_msg_buffer << line << RESET << std::endl;
        }

    protected:


} cb;

/*
std::string poll_gfx_msg_buffer (void)
{
        std::string r = gfx_msg_buffer.str();
        gfx_msg_buffer.str("");
        return r;
}
*/


int already_fatal = 0;

void app_fatal()
{
        if (!already_fatal) {
                already_fatal = 1;
                gfx_shutdown();
        }

        abort();
}

int main(int argc, const char **argv)
{

        try {

                #ifdef FE_NOMASK_ENV
                //feenableexcept(FE_INVALID);
                //feenableexcept(FE_DIVBYZERO | FE_INVALID);
                #endif

                bgl = new BackgroundLoader();

                size_t winid = gfx_init(cb);

                hud = gfx_init_hud();

                debug_drawer = new BulletDebugDrawer(ogre_sm); // FIXME: hack

                #ifdef WIN32
                mouse = new MouseDirectInput8(winid);
                bool use_dinput = getenv("GRIT_DINPUT")!=NULL;
                keyboard = use_dinput ? (Keyboard *)new KeyboardDirectInput8(winid)
                                      : (Keyboard *)new KeyboardWinAPI(winid);
                #else
                mouse = new MouseX11(winid);
                keyboard = new KeyboardX11(winid);
                #endif

                physics_init();

				net_init();

                streamer_init();

                //audio_init(getenv("GRIT_AUDIO_DEV"));
                audio_init(NULL);

                core_L = init_lua("/system/init.lua");

                // now call out to lua
                {
                        int error_handler;
                        lua_State *L = core_L;

                        push_cfunction(L, my_lua_error_handler);
                        error_handler = lua_gettop(L);

                        lua_getglobal(L, "main");
                        if (lua_isnil(L,-1)) {
                                lua_pop(L,1); // nil 'main object'
                                lua_pop(L,1); //handler
                                CERR << "invoking main:run(...): object \"main\" not found" << std::endl;
                                app_fatal();
                        }

                        lua_getfield(L, -1, "run");
                        if (lua_isnil(L,-1)) {
                                lua_pop(L,1); // nil 'run function'
                                lua_pop(L,1); //handler
                                CERR << "invoking main:run(...): function \"run\" not found" << std::endl;
                                app_fatal();
                        }

                        // arguments
                        lua_getglobal(L, "main");
                        for (int i=0 ; i<argc ; i++) {
                                check_stack(L,1);
                                lua_pushstring(L,argv[i]);
                        }

                        int status = lua_pcall(L, argc+1, 0, error_handler);
                        if (status) {
                                lua_pop(L,1); // error message
                        }

                        lua_pop(L,1); // error_handler
                }

                // lua returns - we quit
                
                net_shutdown(core_L); //net needs to close LuaPtrs properly

                object_all_del(core_L); // will remove all demands from bgl

                hud->removeAllChildren(); // will avoid any lua callbacks during destruction

                gfx_shutdown_lua(core_L);

                bgl->shutdown();

                if (mouse) delete mouse;
                if (keyboard) delete keyboard;
                if (core_L) shutdown_lua(core_L);
                hud.setNull();
                audio_shutdown(); //close AL device
                physics_shutdown();
                if (debug_drawer) delete debug_drawer;
                gfx_shutdown();

                delete bgl;

        } catch (Exception& e) {
                std::cerr << "TOP LEVEL ERROR: " << e << std::endl;
        }
        return EXIT_SUCCESS;
}

// vim: shiftwidth=8:tabstop=8:expandtab
