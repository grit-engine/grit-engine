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

//#include <fenv.h>
#include <cerrno>
#include <sstream>

#ifdef WIN32
#  include <windows.h>
#  define DIRECT3D_VERSION 0x0900
//#  include <d3d9.h>
#  include "win32/mouse_direct_input8.h"
#  include "win32/keyboard_direct_input8.h"
#  include "win32/keyboard_win_api.h"
#else
#  include "linux/keyboard_x11.h"
#  include "linux/mouse_x11.h"
  #if defined(__linux__) && defined(__JOYSTICK_DEVJS__)
    #  include "linux/joystick_devjs.h"
  #endif
#endif

#include "clipboard.h"

#include <centralised_log.h>
#include "core_option.h"
#include "grit_lua_util.h"
#include "lua_wrappers_core.h"
#include "main.h"

#include "gfx/gfx.h"
#include "physics/physics_world.h"
#include "audio/audio.h"
#include "net/net.h"
#include"navigation/navigation_system.h"

CentralisedLog clog;
bool clicked_close = false;
Mouse *mouse = NULL;
Keyboard *keyboard = NULL;
#if defined(__linux__) && defined(__JOYSTICK_DEVJS__)
Joystick *joystick = NULL;
#endif

lua_State *core_L = NULL;
BulletDebugDrawer *debug_drawer = NULL;
BackgroundLoader *bgl;

// Receive notifications from the graphics engine.
struct TheGfxCallback : GfxCallback {

    virtual void clickedClose (void)
    {
        clicked_close = true;
    }

    virtual void windowResized (int width, int height)
    {
        (void) width;
        (void) height;
    }
    
    virtual void messageLogged (const std::string &line)
    {
        clog.print(line);
    }

} cb;

int already_fatal = 0;

// To be called in emergencies, upon unrecoverable errors.
void app_fatal()
{
    if (!already_fatal) {
        already_fatal = 1;
        gfx_shutdown();
    }

    abort();
}

int main (int argc, const char **argv)
{
#ifdef WIN32
	{
		// set the console to accept escape sequences (Win10+)
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

		if (console != INVALID_HANDLE_VALUE)
		{
			DWORD consoleMode = 0;
			GetConsoleMode(console, &consoleMode);

			consoleMode |= 4; // ENABLE_VIRTUAL_TERMINAL_PROCESSING; but that isn't available in our current SDK target

			SetConsoleMode(console, consoleMode);
		}
	}
#endif

    int exit_code = EXIT_SUCCESS;

    try {

        // Experiment with floating point masks.
        // #ifdef FE_NOMASK_ENV
        // feenableexcept(FE_INVALID);
        // feenableexcept(FE_DIVBYZERO | FE_INVALID);
        // #endif

        bgl = new BackgroundLoader();

        size_t winid = gfx_init(cb);

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

        #if defined(__linux__) && defined(__JOYSTICK_DEVJS__)
        joystick = new JoystickDevjs(winid);
        #endif

        clipboard_init();

        physics_init();

        net_init();

        core_option_init();
        streamer_init();

        navigation_init();

        // audio_init(getenv("GRIT_AUDIO_DEV"));
        audio_init(NULL);

        std::vector<std::string> args;
        for (int i=0 ; i<argc ; i++) {
            args.push_back(argv[i]);
        }

        try {
            const char *init_file = getenv("GRIT_INIT");
            if (init_file == nullptr) init_file = "/system/init.lua";
            init_lua(init_file, args, core_L);
        } catch (Exception &e) {
            CERR << "Fatal error: " << e << std::endl;
            exit_code = EXIT_FAILURE;
        }

        // Lua returns - we quit.
        
        CVERB << "Shutting down Lua net subsystem..." << std::endl;

        net_shutdown(core_L);

        navigation_shutdown();

        object_all_del(core_L);  // Will remove all demands from background loader.

        CVERB << "Shutting down Lua graphics subsystem..." << std::endl;

        gfx_shutdown_lua(core_L);

        CVERB << "Shutting down Background Loader..." << std::endl;

        bgl->shutdown();

        CVERB << "Shutting down Mouse & Keyboard..." << std::endl;

        if (mouse) delete mouse;
        if (keyboard) delete keyboard;
        #if defined(__linux__) && defined(__JOYSTICK_DEVJS__)
        if (joystick) delete joystick;
        #endif
        CVERB << "Shutting down clipboard..." << std::endl;
        clipboard_shutdown();

        CVERB << "Shutting down Lua VM..." << std::endl;
        if (core_L) shutdown_lua(core_L);

        CVERB << "Shutting down audio subsystem..." << std::endl;
        audio_shutdown(); //close AL device

        CVERB << "Shutting down physics subsystem..." << std::endl;
        physics_shutdown();

        CVERB << "Shutting down the physics debug drawer..." << std::endl;
        if (debug_drawer) delete debug_drawer;

        CVERB << "Shutting down the Graphics subsystem..." << std::endl;
        gfx_shutdown();

        delete bgl;

    } catch (Exception &e) {
        std::cerr << "TOP LEVEL ERROR: " << e << std::endl;
        return EXIT_FAILURE;
    }

    return exit_code;
}

// vim: shiftwidth=8:tabstop=8:expandtab
