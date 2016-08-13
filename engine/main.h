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

/** \mainpage The Grit Game Engine
 *
 * \section intro Introduction
 * 
 * This is the intro
 *
 * \section toplevel Top Level Design
 *
 * Placeholder text.  Another sentence.
 * 
 * Another paragraph.
 */

extern "C" {
        #include "lua.h"
}

#include "mouse.h"
#include "keyboard.h"
#if defined(__linux__) && defined(__JOYSTICK_DEVJS__)
#include "joystick.h"
#endif
#include "background_loader.h"
#include <centralised_log.h>
#include "bullet_debug_drawer.h"


/** A singleton log object through which all internal debug log output is passed. */
extern CentralisedLog clog;

/** A boolean that starts off false and becomes true when the user closes the game window. */
extern bool clicked_close;

/** A singleton for receiving mouse input. */
extern Mouse *mouse;

/** A singleton for receiving keyboard input. */
extern Keyboard *keyboard;

/** A singleton for receiving joystick input. */
#if defined(__linux__) && defined(__JOYSTICK_DEVJS__)
extern Joystick *joystick;
#endif

/** The core Lua state object, created when lua is initialised. */
extern lua_State *core_L;

/** Used for garbage collecting Ogre objects properly in Lua. */
//extern UserDataTables user_data_tables;

/** Used by the bullet backend for drawing debug information.  This is a
 * conduit between the physics and graphics subsystems. */
extern BulletDebugDrawer *debug_drawer;

/** The singleton that manages loading resources from disk in a background thread. */
extern BackgroundLoader *bgl;
