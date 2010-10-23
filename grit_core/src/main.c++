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

//#include <fenv.h>
#include <cerrno>
#include <sstream>

#ifdef WIN32
#  include "windows.h"
#  include "win32/MouseDirectInput8.h"
#  include "win32/KeyboardDirectInput8.h"
#  include "win32/KeyboardWinAPI.h"
#else
#  include "linux/KeyboardX11.h"
#  include "linux/MouseX11.h"
#endif

#include "main.h"
#include "gfx.h"


#include "console_colour.h"

#include "lua_util.h"
#include "lua_wrappers_core.h"

CentralisedLog clog;
bool clicked_close = false;
Mouse *mouse = NULL;
Keyboard *keyboard = NULL;
lua_State *core_L = NULL;
UserDataTables user_data_tables;
Streamer *streamer = NULL;
BulletDebugDrawer *debug_drawer = NULL;
std::stringstream gfx_msg_buffer;
HUD::RootPtr hud;

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

std::string poll_gfx_msg_buffer (void)
{
        std::string r = gfx_msg_buffer.str();
        gfx_msg_buffer.str("");
        return r;
}


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

                BackgroundMeshLoader *bml = new BackgroundMeshLoader();

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

                streamer = new Streamer();

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
                                CERR << "invoking main:run(...): object \"main\" not found";
                                app_fatal();
                        }

                        lua_getfield(L, -1, "run");
                        if (lua_isnil(L,-1)) {
                                lua_pop(L,1); // nil 'run function'
                                lua_pop(L,1); //handler
                                CERR << "invoking main:run(...): function \"run\" not found";
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

                bml->shutdown();

                hud->removeAllChildren(); // will avoid any lua callbacks during destruction

                streamer->doShutdown(core_L);
                delete streamer;
                if (debug_drawer) delete debug_drawer;
                if (mouse) delete mouse;
                if (keyboard) delete keyboard;
                if (core_L) lua_close(core_L);
                hud.setNull();
                gfx_shutdown();

                delete bml;

        } catch( GritException& e ) {
                std::cerr << "An exception has occured: "
                          << e.longMessage() << std::endl;
        }
        return EXIT_SUCCESS;
}

// vim: shiftwidth=8:tabstop=8:expandtab
