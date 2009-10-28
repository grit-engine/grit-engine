#include <iostream>
#include <sstream>

#include "KeyboardX11.h"
#include "../CentralisedLog.h"

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

static std::string key_to_string (XEvent &ev, std::map<KeySym,const char*> &my_key_map)
{
        KeySym key = XLookupKeysym(&ev.xkey, 0);
        if (my_key_map.find(key) != my_key_map.end()) return my_key_map[key];
     
        char buf[1024];
        int r = XLookupString(&ev.xkey, buf, sizeof buf, NULL, NULL);
        const char *keystr = r ? buf : XKeysymToString(key);

        // TODO: handle unicode properly
        for (size_t i=0 ; i<strlen(keystr) ; ++i) {
                unsigned char c = (unsigned char)keystr[i];
                if (c<32 || c>127) return "?";
        }

        return keystr;
}

void KeyboardX11::add_key (Keyboard::Presses &keys, XEvent ev, int kind)
{
        const char *prefix[] = { "-", "=", "+" };
        std::string str = prefix[kind+1];
        ev.xkey.state = 0;
        std::string keystr = key_to_string(ev, myKeyMap);
        ev.xkey.state = ShiftMask;
        std::string keystr2 = key_to_string(ev, myKeyMap);
        ev.xkey.state = Mod5Mask;
        std::string keystr3 = key_to_string(ev, myKeyMap);

        str += keystr;

        bool contains = currentlyPressed.find(keystr)!=currentlyPressed.end();

        switch (kind) {
                case -1:
                if (!contains) return;
                currentlyPressed.erase(keystr);
                break;
                case 0:
                if (!contains) {
                        add_key(keys, ev, 1);
                        return;
                } 
                break;
                case 1:
                if (contains) return;
                currentlyPressed.insert(keystr);
                break;
        }
        if (shiftMap[keystr] == "") {
                shiftMap[keystr] = keystr2;
                if (verbose) {
                        CLOG << "X map: " << keystr
                             << " -> " << keystr2 << std::endl;
                }
        }
        if (altMap[keystr] == "") {
                altMap[keystr] = keystr3;
                if (verbose) {
                        CLOG << "X map: " << keystr
                             << " => " << keystr3 << std::endl;
                }
        }
        if (verbose) {
                CLOG << "X key: " << str << std::endl;
        }
        keys.push_back(str);
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


Keyboard::Press KeyboardX11::getShifted (const Press &press)
{
        return shiftMap[press];
}

Keyboard::Press KeyboardX11::getAlted (const Press &press)
{
        return altMap[press];
}


// vim: shiftwidth=8:tabstop=8:expandtab
