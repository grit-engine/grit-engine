/* Copyright (c) David Cunningham and the Grit Game Engine project 2014
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

#ifndef INPUT_FILTER_H
#define INPUT_FILTER_H

#include <map>
#include <set>
#include <string>

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "lua_ptr.h"

// You can set a callback set for specific button events.  This
// emulates UI:bind().  If a callback is provided for a button event,
// it will not propagate to lower priority InputFilters.

// You can disable and enable InputFilters, doing so prevents the
// button events being masked so lower priorities will receive them.

// You can also query what buttons are down (for a given level).  If
// you are disabled, this is always the empty set.

// You can set modal, which means that all lower priorities are effectively
// disabled.

// You can set mouse capture, which hides the cursor.  The highest priority
// input filter with mouse capture receives mouse move events via a callback.


// The HUD will get mouse move events and remaining button events if no-one has
// captured the mouse.  The hud system keeps track of button down status
// itself.


// Lowever priority filters (including the HUD) will get button up events to
// cleanly empty their button down sets, when they are no-longer able to
// receive those events naturally (e.g. disable of themselves, enable of higher
// priority stuff, modal, mouse lock changes, etc).


// Menu (binds escape)
//   * goes into modal to display menu (disable player_ctrl and tab stuff)
//   * never captures mouse
//   * changes hud to display menu screen (highest z order)
//   * menu screen hud bindings for up/down, enter, etc
//   * menu screen hud support for text entry "player name" fields, etc
//   * other hud elements not focussed, or obscured by screen rectangle
// Debug (binds tab)
//   * goes into modal when tab pressed
//   * changes enable status of console, etc
//   * never captures mouse
//   * can handle screenshot and simple things also, e.g. physics pause, any global bindings that work in debug mode
//   * can be disabled to disallow cheating
// Playing General
//   * just game binds, set up by configuration.lua
//   * captures mouse, dispatches through controlObj stuff
//   * understands 'f' key, controls foot/driving/flying/ghosting enablement
// Playing on foot
//   * movement, firing, etc
// Playing driving
//   * movement
// Playing flying
// Playing ghosting
// Implicit HUD

class InputFilter {

    public:

    struct Callback {
        LuaPtr down;
        LuaPtr up;
        LuaPtr repeat;
        std::string path;
    };

    typedef std::map<std::string, std::string> ButtonStatus;
    typedef std::map<std::string, Callback> CallbackMap;
    typedef std::set<std::string> BindingSet;

    private:

    /** Keys that we sent a down signal for, but not an up yet.  These must be
     * "flushed" in certain circumstances, such as when as when we are disabled, or
     * if a higher priority input watcher goes modal.  Each down button is mapped
     * to the prefix of modifiers that were down at the time of it being pressed.
     * These allow the button up events to be matched to button down events
     * properly, even if the modifier was released between the down and up events.
     */
    ButtonStatus down;

    bool modal;
    bool enabled;
    bool mouseCapture;

    /** The keys of this map can be things like C-A-S-Space meaning
     * ctrl+alt+shift+space.
     */
    CallbackMap buttonCallbacks;

    LuaPtr mouseMoveCallback;
    std::string mouseMoveCallbackPath;

    double priority;

    bool destroyed;

    void ensureAlive() {
        if (!destroyed) return;
        EXCEPT << "InputFilter \""<<description<<"\" destroyed." << ENDL;
    }

    public:

    const std::string description;

    /** Give the initial priority, and a string description (useful for debugging). */
    InputFilter (double priority, const std::string &desc);

    ~InputFilter (void);

    /** Has anyone called destroy yet? */
    bool isAlive (void) { return !destroyed; };
    
    /** Unlink all the Lua callbacks. */
    void destroy (lua_State *L);
    
    /** Bind callbacks to a specific key.  Expects the stack to contain 3
     * things.  First thing must be a function and is the 'down' callback.  Second
     * thing can be nil or a function and is the 'up' callback.  Third thing can be
     * nil, true, or a function and is the 'repeat' callback (true means to use the
     * 'down' callback for this as well).  The button name is given without +-:
     * prefix.  It can have a C-A-S- prefix, e.g. C-A-T for ctrl+alt, or S-V for
     * shift+V.
     */
    void bind (lua_State *L, const std::string &button);

    /** The given key no-longer triggers callbacks and falls to the next filter.  The
     * button is given in the same form as in bind.
     */
    void unbind (lua_State *L, const std::string &button);

    /** Receive mouse move events.  This callback is called if this InputFilter
     * has locked the mouse and no higher priority ones have done so.
     */
    void setMouseMoveCallback (lua_State *L);

    bool getEnabled (void) { return enabled; }

    /** Will trigger events at lower levels or this level. */
    void setEnabled (lua_State *L, bool v);

    bool getModal (void) { return modal; }

    /** Will effectively disable all lower levels. */
    void setModal (lua_State *L, bool v);

    bool getMouseCapture (void) { return mouseCapture; }

    /** Will hide the mouse cursor and we will get mouse move events instead of
     * the HUD.
     */
    void setMouseCapture (lua_State *L, bool v);

    /** Test for whether a given button is down at this time from the
     * perspective of this filter.  The button is given without +-:= prefix.
     */
    bool isButtonPressed (const std::string &b);

    double getPriority (void) { return priority; }

    /** For internal use. */
    bool acceptButton (lua_State *L, const std::string &b);
    void triggerFunc (lua_State *L, const std::string &button, const LuaPtr &func, const std::string &path);
    void triggerMouseMove (lua_State *L, const Vector2 &abs);
    void flushAll (lua_State *L);
    void flushSet (lua_State *L, const BindingSet &s);
    bool isBound (const std::string &b);

};

/** Does there exist an InputFilter at this priority? */
bool input_filter_has (double priority);

/** Get the description at this priority, or throw exception. */
std::string input_filter_get_description (double priority);

/** Destroy a given input filter.  This can be useful if you've lost the pointer to it. */
std::string input_filter_destroy (double priority);

/** Issue button events to the appropriate level of the stack or the HUD. */
void input_filter_trickle_button (lua_State *L, const std::string &ev);

/** Issue mouse move events to the appropriate level of the stack or the HUD. */
void input_filter_trickle_mouse_move (lua_State *L, const Vector2 &rel, const Vector2 &abs);

/** Is the given button down (globally). */
bool input_filter_pressed (const std::string &button);

/** Issue button up events to anyone who believes a key is down.  This is used
 * when losing focus from the game window and similar situations when we can
 * expect to fundamentally lose input events for a period of time. */
void input_filter_flush (lua_State *L);

/** Suppress system mouse cursor.  The system mouse cursor is usually more
 * responsive than anything you can render in game.  However, if the style is
 * disagreeable it can be hidden, and you can draw your own instead.
*/
void input_filter_set_cursor_hidden (bool v);

/** Is the system mouse cursor suppressed? */
bool input_filter_get_cursor_hidden (void);

/** Shutdown Lua state. */
void input_filter_shutdown (lua_State *L);

#endif
