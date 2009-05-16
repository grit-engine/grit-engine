#include <iostream>
#include <sstream>

#include "KeyboardX11.h"
#include "../CentralisedLog.h"

KeyboardX11::KeyboardX11 (size_t window)
      : focussed(true)
{

        #define MAP_KEY(xk,k) myKeyMap.insert(std::make_pair(xk,k))

        // all the keys for which XKeysymToString would not return
        // what we want
        MAP_KEY(XK_minus, "-");
        MAP_KEY(XK_equal, "=");
        MAP_KEY(XK_space, "Space");
        MAP_KEY(XK_comma, ",");
        MAP_KEY(XK_period, ".");
        MAP_KEY(XK_numbersign, "#");

        MAP_KEY(XK_backslash, "\\");
        MAP_KEY(XK_slash, "/");
        MAP_KEY(XK_bracketleft, "[");
        MAP_KEY(XK_bracketright, "]");

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

        MAP_KEY(XK_colon, ":");
        MAP_KEY(XK_semicolon, ";");
        MAP_KEY(XK_apostrophe, "'");
        MAP_KEY(XK_grave, "`");


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



        win = window;

        display = XOpenDisplay(0);
        APP_ASSERT(display);

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
                XCloseDisplay(display);
        }
}

void KeyboardX11::add_key (Keyboard::Presses &keys, KeySym key, int kind)
{
        bool contains = currentlyPressed.find(key)!=currentlyPressed.end();
        const char *string;
        switch (kind) {
                case -1:
                if (!contains) return;
                string = xKeysUp[key];
                if (string) currentlyPressed.erase(key);
                break;
                case 0:
                if (!contains) {
                        add_key(keys, key, 1);
                        return;
                } 
                string = xKeysRep[key];
                break;
                case 1:
                if (contains) return;
                string = xKeysDown[key];
                if (string) currentlyPressed.insert(key);
                break;
        }
        //std::cout << "key: " << string << std::endl;
        if (string)
                keys.push_back(string);
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
        KeySym last = 0;

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
                                        add_key(r, last, -1);
                                }
                                KeySym key = XLookupKeysym(&event.xkey, 0);
                                KeySym key2 = XLookupKeysym(&event.xkey, 1);
                                last = key;
                                last_was_release = true;
                        } break;

                        case KeyPress: {
                                KeySym key = XLookupKeysym(&event.xkey, 0);
                                KeySym key2 = XLookupKeysym(&event.xkey, 1);
                                shiftMap[key] = key2;
                                if (last_was_release) {
                                        if (last!=key) {
                                                // different key, add both
                                                add_key(r, last, -1);
                                                add_key(r, key, 1);
                                        } else {
                                                add_key(r, key, 0);
                                        }
                                } else {
                                        add_key(r, key, 1);
                                }
                                last_was_release = false;
                        } break;

                }
        }

        if (last_was_release) {
                add_key(r, last, -1);
        }

        if (flushRequested) {
                // Any key we currently recognise as being held
                // down is "released" (note a repeating key
                // will still repeat upon refocus)

                typedef std::set<KeySym>::iterator I;
                std::set<KeySym> s = currentlyPressed;
                for (I i=s.begin(),i_=s.end() ; i!=i_ ; ++i) {
                        add_key(r, *i, -1);
                }
                flushRequested = false;
        }
        return r;
}


Keyboard::Press KeyboardX11::getShifted (const Press &press)
{
        Press ret;
        Press postfix = press.substr(1);
        ret = press[0]+shiftMap[postfix];
        return ret;
}


// vim: shiftwidth=8:tabstop=8:expandtab
