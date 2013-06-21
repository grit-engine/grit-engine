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

#include <iostream>
#include <sstream>

#include "KeyboardX11.h"
#include "../CentralisedLog.h"
#include "../unicode_util.h"

KeyboardX11::KeyboardX11 (size_t window)
      : focussed(true)
{

        #define MAP_KEY(xk,k) myKeyMap.insert(std::make_pair(xk,k))

        // all the keys for which XLookupString followed by XKeySymToString would not return
        // what we want

        // these guys return the ascii control code
        MAP_KEY(XK_space, "Space");
        MAP_KEY(XK_BackSpace, "BackSpace");
        MAP_KEY(XK_Delete, "Delete");
        MAP_KEY(XK_Tab, "Tab");
        MAP_KEY(XK_Return, "Return");
        MAP_KEY(XK_Escape, "Escape");

        MAP_KEY(XK_Caps_Lock, "CapsLock");

        MAP_KEY(XK_Page_Up, "PageUp");
        MAP_KEY(XK_Page_Down, "PageDown");

        MAP_KEY(XK_Num_Lock, "NumLock");
        MAP_KEY(XK_Print, "SysRq");
        MAP_KEY(XK_Scroll_Lock, "Scroll");

        MAP_KEY(XK_Control_L, "Ctrl");
        MAP_KEY(XK_Control_R, "Ctrl");
        MAP_KEY(XK_Shift_R, "Shift");
        MAP_KEY(XK_Shift_L, "Shift");
        MAP_KEY(XK_Alt_R, "Alt");
        MAP_KEY(XK_Alt_L, "Alt");
        MAP_KEY(XK_Super_L, "LWin");
        MAP_KEY(XK_Super_R, "RWin");


        //Keypad
        MAP_KEY(XK_KP_0, "NUMPAD0");
        MAP_KEY(XK_KP_1, "NUMPAD1");
        MAP_KEY(XK_KP_2, "NUMPAD2");
        MAP_KEY(XK_KP_3, "NUMPAD3");
        MAP_KEY(XK_KP_4, "NUMPAD4");
        MAP_KEY(XK_KP_5, "NUMPAD5");
        MAP_KEY(XK_KP_6, "NUMPAD6");
        MAP_KEY(XK_KP_7, "NUMPAD7");
        MAP_KEY(XK_KP_8, "NUMPAD8");
        MAP_KEY(XK_KP_9, "NUMPAD9");
        MAP_KEY(XK_KP_Add, "NUMPAD+");
        MAP_KEY(XK_KP_Subtract, "NUMPAD-");
        MAP_KEY(XK_KP_Decimal, "NUMPAD.");
        MAP_KEY(XK_KP_Equal, "NUMPAD=");
        MAP_KEY(XK_KP_Divide, "NUMPAD/");
        MAP_KEY(XK_KP_Multiply, "NUMPAD*");
        MAP_KEY(XK_KP_Enter, "NUMPADReturn");

        //Keypad with numlock off
        MAP_KEY(XK_KP_Home, "NUMPAD7");
        MAP_KEY(XK_KP_Up, "NUMPAD8");
        MAP_KEY(XK_KP_Page_Up, "NUMPAD9");
        MAP_KEY(XK_KP_Left, "NUMPAD4");
        MAP_KEY(XK_KP_Begin, "NUMPAD5");
        MAP_KEY(XK_KP_Right, "NUMPAD6");
        MAP_KEY(XK_KP_End, "NUMPAD1");
        MAP_KEY(XK_KP_Down, "NUMPAD2");
        MAP_KEY(XK_KP_Page_Down, "NUMPAD3");
        MAP_KEY(XK_KP_Insert, "NUMPAD0");
        MAP_KEY(XK_KP_Delete, "NUMPAD.");

        //These guys give full names instead of the symbols
        //but we want to behave the same on windows and linux, so...
        MAP_KEY(XK_comma, ",");
        MAP_KEY(XK_period, ".");
        MAP_KEY(XK_semicolon, ";");
        MAP_KEY(XK_slash, "/");
        MAP_KEY(XK_backslash, "\\");
        MAP_KEY(XK_apostrophe, "'");
        MAP_KEY(XK_bracketleft, "[");
        MAP_KEY(XK_bracketright, "]");
        MAP_KEY(XK_minus, "-");
        MAP_KEY(XK_equal, "=");
        MAP_KEY(XK_grave, "`");

        win = window;

        display = XOpenDisplay(0);
        APP_ASSERT(display);

/*
        im = XOpenIM(display, NULL, NULL, NULL);
        APP_ASSERT(im);

        ic = XCreateIC(im);
        APP_ASSERT(im);
*/


        long event_mask = KeyPressMask | KeyReleaseMask;
        event_mask |= FocusChangeMask;
        if (XSelectInput(display, win, event_mask) == BadWindow) {
                CERR << "calling XSelectInput: BadWindow" << std::endl;
                app_fatal();
        }

}

KeyboardX11::~KeyboardX11 (void)
{
        if(display) {
                getPresses();
                //if (ic) XDestroyIC(ic);
                //if (im) XCloseIM(im);
                XCloseDisplay(display);
        }
}

void KeyboardX11::add_key (Keyboard::Presses &keys, XEvent ev, int kind)
{
        const char *prefix[] = { "-", "=", "+" };
        std::string str = prefix[kind+1];

        std::string keystr;

        // There is a list of specific keysyms that I want to map to strings myself
        KeySym key = XLookupKeysym(&ev.xkey, 0);
        if (key == NoSymbol) return; // key is not bound in X

        if (myKeyMap.find(key) != myKeyMap.end()) {
                keystr = myKeyMap[key];
        } else {
                keystr = XKeysymToString(key);
        }
     
        str += keystr;

        bool contains = currentlyPressed.find(keystr)!=currentlyPressed.end();

        switch (kind) {
                case -1: // release
                if (!contains) return;
                currentlyPressed.erase(keystr);
                break;
                case 0: // repeat
                if (!contains) {
                        add_key(keys, ev, 1);
                        return;
                } 
                break;
                case 1: // press
                if (contains) return;
                currentlyPressed.insert(keystr);
                break;
        }
        if (verbose) {
                CLOG << "X key: " << str << std::endl;
        }
        keys.push_back(str);

        // TODO: advanced input method / context for non-european languages
        // use Xutf8LookupString
        // or use an existing system like gtk
        char buf[1024];
        int r = XLookupString(&ev.xkey, buf, sizeof buf, NULL, NULL);

        // don't want text events unless the key is being pressed or repeated
        if (r && kind >= 0) {
                APP_ASSERT(r==1);
                // returns latin1 which is a subset of unicode
                unsigned long codepoint = (unsigned char)buf[0];
                // do not want non-printable text coming from keyboard, we have the
                // above system for those keys
                if ((codepoint>=0x20 && codepoint<0x7f) || codepoint>=0xa0) {
                        str = ":";
                        // just encode into utf8 and we're done
                        encode_utf8(codepoint, str);
                        if (verbose) {
                                CLOG << "X key text: \"" << str << "\"" << std::endl;
                        }
                        keys.push_back(str);
                }
        }
}

bool KeyboardX11::hasFocus (void)
{
        return focussed;
}

Keyboard::Presses KeyboardX11::getPresses (void)
{

        Keyboard::Presses r; // collect presses here
        XEvent event;

        //Poll x11 for events

        bool last_was_release = false;
        KeySym last_key = 0;
        XEvent last_event;

        while(XPending(display) > 0) {

                XNextEvent(display, &event);

                switch (event.type) {

                        case FocusIn: {
                                focussed = true;
                                last_was_release = false;
                        } break;

                        case FocusOut: {
                                focussed = false;
                                last_was_release = false;
                        } break;

                        case KeyRelease: {
                                if (last_was_release) {
                                        add_key(r, last_event, -1);
                                }
                                KeySym key = XLookupKeysym(&event.xkey, 0);
                                last_key = key;
                                last_event = event;
                                last_was_release = true;
                        } break;

                        case KeyPress: {
                                KeySym key = XLookupKeysym(&event.xkey, 0);
                                if (last_was_release) {
                                        if (last_key!=key) {
                                                // different key, add both
                                                add_key(r, last_event, -1);
                                                add_key(r, event, 1);
                                        } else {
                                                add_key(r, event, 0);
                                        }
                                } else {
                                        add_key(r, event, 1);
                                }
                                last_was_release = false;
                        } break;

                }
        }

        if (last_was_release) {
                add_key(r, last_event, -1);
        }

        for (Presses::iterator i=keysToFlush.begin(),
                               i_=keysToFlush.end() ; i!=i_ ; ++i) {
                if (currentlyPressed.find(*i)!=currentlyPressed.end()) {
                        r.push_back("-"+*i);
                        currentlyPressed.erase(*i);
                        if (verbose) {
                                CLOG << "X keyboard: flushed: " << *i << std::endl;
                        }
                }
        }
        keysToFlush.clear();
        if (fullFlushRequested) {
                // Any key we currently recognise as being held
                // down is "released" (note a repeating key
                // will still repeat upon refocus)

                typedef std::set<Press>::iterator I;
                for (I i=currentlyPressed.begin(),i_=currentlyPressed.end() ;
                     i!=i_ ; ++i) {
                        r.push_back("-"+*i);
                }
                currentlyPressed.clear();
                fullFlushRequested = false;
                if (verbose) {
                        CLOG << "X keyboard: all flushed" << std::endl;
                }
        }
        return r;
}


// vim: shiftwidth=8:tabstop=8:expandtab
