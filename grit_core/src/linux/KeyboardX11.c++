#include <iostream>
#include <sstream>

#include "KeyboardX11.h"
#include "../CentralisedLog.h"

KeyboardX11::KeyboardX11(size_t window)
{

        #define MAP_KEY(xk,k) xKeysDown.insert(std::make_pair(xk,"+"k)); \
                              xKeysRep.insert(std::make_pair(xk,"="k)); \
                              xKeysUp.insert(std::make_pair(xk,"-"k))

        MAP_KEY(XK_1, "1");
        MAP_KEY(XK_2, "2");
        MAP_KEY(XK_3, "3");
        MAP_KEY(XK_4, "4");
        MAP_KEY(XK_5, "5");
        MAP_KEY(XK_6, "6");
        MAP_KEY(XK_7, "7");
        MAP_KEY(XK_8, "8");
        MAP_KEY(XK_9, "9");
        MAP_KEY(XK_0, "0");

        MAP_KEY(XK_BackSpace, "BackSpace");

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

        MAP_KEY(XK_Escape,"Escape");
        MAP_KEY(XK_Caps_Lock, "CapsLock");

        MAP_KEY(XK_Tab, "Tab");
        MAP_KEY(XK_Return, "Return");
        MAP_KEY(XK_Control_L, "LCtrl");
        MAP_KEY(XK_Control_R, "RCtrl");

        MAP_KEY(XK_colon, ":");
        MAP_KEY(XK_semicolon, ";");
        MAP_KEY(XK_apostrophe, "'");
        MAP_KEY(XK_grave, "`");

        MAP_KEY(XK_a, "A");
        MAP_KEY(XK_b, "B");
        MAP_KEY(XK_c, "C");
        MAP_KEY(XK_d, "D");
        MAP_KEY(XK_e, "E");
        MAP_KEY(XK_f, "F");
        MAP_KEY(XK_g, "G");
        MAP_KEY(XK_h, "H");
        MAP_KEY(XK_i, "I");
        MAP_KEY(XK_j, "J");
        MAP_KEY(XK_k, "K");
        MAP_KEY(XK_l, "L");
        MAP_KEY(XK_m, "M");
        MAP_KEY(XK_n, "N");
        MAP_KEY(XK_o, "O");
        MAP_KEY(XK_p, "P");
        MAP_KEY(XK_q, "Q");
        MAP_KEY(XK_r, "R");
        MAP_KEY(XK_s, "S");
        MAP_KEY(XK_t, "T");
        MAP_KEY(XK_u, "U");
        MAP_KEY(XK_v, "V");
        MAP_KEY(XK_w, "W");
        MAP_KEY(XK_x, "X");
        MAP_KEY(XK_y, "Y");
        MAP_KEY(XK_z, "Z");

        MAP_KEY(XK_A, "A");
        MAP_KEY(XK_B, "B");
        MAP_KEY(XK_C, "C");
        MAP_KEY(XK_D, "D");
        MAP_KEY(XK_E, "E");
        MAP_KEY(XK_F, "F");
        MAP_KEY(XK_G, "G");
        MAP_KEY(XK_H, "H");
        MAP_KEY(XK_I, "I");
        MAP_KEY(XK_J, "J");
        MAP_KEY(XK_K, "K");
        MAP_KEY(XK_L, "L");
        MAP_KEY(XK_M, "M");
        MAP_KEY(XK_N, "N");
        MAP_KEY(XK_O, "O");
        MAP_KEY(XK_P, "P");
        MAP_KEY(XK_Q, "Q");
        MAP_KEY(XK_R, "R");
        MAP_KEY(XK_S, "S");
        MAP_KEY(XK_T, "T");
        MAP_KEY(XK_U, "U");
        MAP_KEY(XK_V, "V");
        MAP_KEY(XK_W, "W");
        MAP_KEY(XK_X, "X");
        MAP_KEY(XK_Y, "Y");
        MAP_KEY(XK_Z, "Z");

        MAP_KEY(XK_F1, "F1");
        MAP_KEY(XK_F2, "F2");
        MAP_KEY(XK_F3, "F3");
        MAP_KEY(XK_F4, "F4");
        MAP_KEY(XK_F5, "F5");
        MAP_KEY(XK_F6, "F6");
        MAP_KEY(XK_F7, "F7");
        MAP_KEY(XK_F8, "F8");
        MAP_KEY(XK_F9, "F9");
        MAP_KEY(XK_F10, "F10");
        MAP_KEY(XK_F11, "F11");
        MAP_KEY(XK_F12, "F12");
        MAP_KEY(XK_F13, "F13");
        MAP_KEY(XK_F14, "F14");
        MAP_KEY(XK_F15, "F15");

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

        MAP_KEY(XK_Up, "Up");
        MAP_KEY(XK_Down, "Down");
        MAP_KEY(XK_Left, "Left");
        MAP_KEY(XK_Right, "Right");

        MAP_KEY(XK_Page_Up, "PageUp");
        MAP_KEY(XK_Page_Down, "PageDown");
        MAP_KEY(XK_Home, "Home");
        MAP_KEY(XK_End, "End");

        MAP_KEY(XK_Num_Lock, "NumLock");
        MAP_KEY(XK_Print, "SysRq");
        MAP_KEY(XK_Scroll_Lock, "Scroll");
        MAP_KEY(XK_Pause, "Pause");

        MAP_KEY(XK_Shift_R, "RShift");
        MAP_KEY(XK_Shift_L, "LShift");
        MAP_KEY(XK_Alt_R, "RAlt");
        MAP_KEY(XK_Alt_L, "LAlt");

        MAP_KEY(XK_Insert, "Insert");
        MAP_KEY(XK_Delete, "Delete");

        MAP_KEY(XK_Super_L, "LWin");
        MAP_KEY(XK_Super_R, "RWin");
        MAP_KEY(XK_Menu, "Menu");


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

KeyboardX11::~KeyboardX11()
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

Keyboard::Presses KeyboardX11::getPresses()
{

        Keyboard::Presses r; // collect presses here
        XEvent event;

        //Poll x11 for events

        bool last_was_release = false;
        KeySym last = 0;

        while(XPending(display) > 0) {

                XNextEvent(display, &event);

                switch (event.type) {

                        case FocusOut: {
                                // Any key we currently recognise as being held
                                // down is "released" (note a repeating key
                                // will still repeat upon refocus)

                                typedef std::set<KeySym>::iterator I;
                                std::set<KeySym> s = currentlyPressed;
                                for (I i=s.begin(),i_=s.end() ; i!=i_ ; ++i) {
                                        add_key(r, *i, -1);
                                }
                                last_was_release = false;
                        } break;

                        case KeyRelease: {
                                if (last_was_release) {
                                        add_key(r, last, -1);
                                }
                                KeySym key = XLookupKeysym(&event.xkey, 0);
                                last = key;
                                last_was_release = true;
                        } break;

                        case KeyPress: {
                                KeySym key = XLookupKeysym(&event.xkey, 0);
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

        return r;
}

// vim: shiftwidth=8:tabstop=8:expandtab
