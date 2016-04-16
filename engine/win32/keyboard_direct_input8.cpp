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

#include <assert.h>
#include <iostream>

#include "keyboard_direct_input8.h"
#include "../centralised_log.h"


#define BUFFSZ 128

static const char *result_str(HRESULT r)
{{{
        #define Shittymacro(id) if (r==(HRESULT)id) return #id; else
        Shittymacro(DI_BUFFEROVERFLOW)
        Shittymacro(DI_DOWNLOADSKIPPED)
        Shittymacro(DI_EFFECTRESTARTED)
        Shittymacro(DI_NOEFFECT)
        Shittymacro(DI_NOTATTACHED)
        Shittymacro(DI_OK)
        Shittymacro(DI_POLLEDDEVICE)
        Shittymacro(DI_PROPNOEFFECT)
        Shittymacro(DI_SETTINGSNOTSAVED)
        Shittymacro(DI_TRUNCATED)
        Shittymacro(DI_TRUNCATEDANDRESTARTED)
        Shittymacro(DI_WRITEPROTECT)
        Shittymacro(DIERR_ACQUIRED)
        Shittymacro(DIERR_ALREADYINITIALIZED)
        Shittymacro(DIERR_BADDRIVERVER)
        Shittymacro(DIERR_BETADIRECTINPUTVERSION)
        Shittymacro(DIERR_DEVICEFULL)
        Shittymacro(DIERR_DEVICENOTREG)
        Shittymacro(DIERR_EFFECTPLAYING)
        Shittymacro(DIERR_GENERIC)
        Shittymacro(DIERR_HANDLEEXISTS)
        Shittymacro(DIERR_HASEFFECTS)
        Shittymacro(DIERR_INCOMPLETEEFFECT)
        Shittymacro(DIERR_INPUTLOST)
        Shittymacro(DIERR_INVALIDPARAM)
        Shittymacro(DIERR_MAPFILEFAIL)
        Shittymacro(DIERR_MOREDATA)
        Shittymacro(DIERR_NOAGGREGATION)
        Shittymacro(DIERR_NOINTERFACE)
        Shittymacro(DIERR_NOTACQUIRED)
        Shittymacro(DIERR_NOTBUFFERED)
        Shittymacro(DIERR_NOTDOWNLOADED)
        Shittymacro(DIERR_NOTEXCLUSIVEACQUIRED)
        Shittymacro(DIERR_NOTFOUND)
        Shittymacro(DIERR_NOTINITIALIZED)
        Shittymacro(DIERR_OBJECTNOTFOUND)
        Shittymacro(DIERR_OLDDIRECTINPUTVERSION)
        Shittymacro(DIERR_OTHERAPPHASPRIO)
        Shittymacro(DIERR_OUTOFMEMORY)
        Shittymacro(DIERR_READONLY)
        Shittymacro(DIERR_REPORTFULL)
        Shittymacro(DIERR_UNPLUGGED)
        Shittymacro(DIERR_UNSUPPORTED)
        Shittymacro(E_HANDLE)
        Shittymacro(E_PENDING)
        Shittymacro(E_POINTER) {
                return "Unrecognised result (shouldn't happen)";
        }
}}}

#define BAD_DI_RESULT(r,i_was) do { CERR<<i_was<<": "<<result_str(r)<<std::endl; } while (0)

static ULONGLONG millis()
{
        FILETIME time;
        GetSystemTimeAsFileTime(&time);
        ULARGE_INTEGER millis;
        millis.LowPart = time.dwLowDateTime;
        millis.HighPart = time.dwHighDateTime;
        return millis.QuadPart/10000;
}

KeyboardDirectInput8::KeyboardDirectInput8(size_t window)
{

        #define MAP_KEY(kc,k) keysDown.insert(std::make_pair(kc,"+"k)); \
                              keysRep.insert(std::make_pair(kc,"="k)); \
                              keysUp.insert(std::make_pair(kc,"-"k)); \
                              keyCode.insert(std::make_pair(k,kc))

        #define MAP_KEY_TEXT2(kc,k,kt,ks) keysDown.insert(std::make_pair(kc,"+"k)); \
                                          keysRep.insert(std::make_pair(kc,"="k)); \
                                          keysUp.insert(std::make_pair(kc,"-"k)); \
                                          keysText.insert(std::make_pair(kc,":"kt)); \
                                          keysTextShifted.insert(std::make_pair(kc,":"ks)); \
                                          keyCode.insert(std::make_pair(k,kc))

        #define MAP_KEY_TEXT(kc,k,ks) MAP_KEY_TEXT2(kc,k,k,ks)

        MAP_KEY_TEXT(DIK_1, "1", "!");
        MAP_KEY_TEXT(DIK_2, "2", "@");
        MAP_KEY_TEXT(DIK_3, "3", "#");
        MAP_KEY_TEXT(DIK_4, "4", "$");
        MAP_KEY_TEXT(DIK_5, "5", "%");
        MAP_KEY_TEXT(DIK_6, "6", "^");
        MAP_KEY_TEXT(DIK_7, "7", "&");
        MAP_KEY_TEXT(DIK_8, "8", "*");
        MAP_KEY_TEXT(DIK_9, "9", "(");
        MAP_KEY_TEXT(DIK_0, "0", ")");

        MAP_KEY(DIK_BACK, "BackSpace");

        MAP_KEY_TEXT(DIK_MINUS, "-", "_");
        MAP_KEY_TEXT(DIK_EQUALS, "=", "+");
        MAP_KEY_TEXT2(DIK_SPACE, "Space", " ", " ");
        MAP_KEY_TEXT(DIK_COMMA, ",", "<");
        MAP_KEY_TEXT(DIK_PERIOD, ".", ">");

        MAP_KEY_TEXT(DIK_BACKSLASH, "\\", "|");
        MAP_KEY_TEXT(DIK_SLASH, "/", "?");
        MAP_KEY_TEXT(DIK_LBRACKET, "[", "{");
        MAP_KEY_TEXT(DIK_RBRACKET, "]", "}");

        MAP_KEY(DIK_ESCAPE,"Escape");
        MAP_KEY(DIK_CAPSLOCK, "CapsLock");

        MAP_KEY(DIK_TAB, "Tab");
        MAP_KEY(DIK_RETURN, "Return");
        MAP_KEY(DIK_LCONTROL, "Ctrl");
        MAP_KEY(DIK_RCONTROL, "Ctrl");

        MAP_KEY(DIK_COLON, ":");
        MAP_KEY_TEXT(DIK_SEMICOLON, ";", ":");
        MAP_KEY_TEXT(DIK_APOSTROPHE, "'", "\"");
        MAP_KEY_TEXT(DIK_GRAVE, "`", "~");

        MAP_KEY_TEXT(DIK_A, "a", "A");
        MAP_KEY_TEXT(DIK_B, "b", "B");
        MAP_KEY_TEXT(DIK_C, "c", "C");
        MAP_KEY_TEXT(DIK_D, "d", "D");
        MAP_KEY_TEXT(DIK_E, "e", "E");
        MAP_KEY_TEXT(DIK_F, "f", "F");
        MAP_KEY_TEXT(DIK_G, "g", "G");
        MAP_KEY_TEXT(DIK_H, "h", "H");
        MAP_KEY_TEXT(DIK_I, "i", "I");
        MAP_KEY_TEXT(DIK_J, "j", "J");
        MAP_KEY_TEXT(DIK_K, "k", "K");
        MAP_KEY_TEXT(DIK_L, "l", "L");
        MAP_KEY_TEXT(DIK_M, "m", "M");
        MAP_KEY_TEXT(DIK_N, "n", "N");
        MAP_KEY_TEXT(DIK_O, "o", "O");
        MAP_KEY_TEXT(DIK_P, "p", "P");
        MAP_KEY_TEXT(DIK_Q, "q", "Q");
        MAP_KEY_TEXT(DIK_R, "r", "R");
        MAP_KEY_TEXT(DIK_S, "s", "S");
        MAP_KEY_TEXT(DIK_T, "t", "T");
        MAP_KEY_TEXT(DIK_U, "u", "U");
        MAP_KEY_TEXT(DIK_V, "v", "V");
        MAP_KEY_TEXT(DIK_W, "w", "W");
        MAP_KEY_TEXT(DIK_X, "x", "X");
        MAP_KEY_TEXT(DIK_Y, "y", "Y");
        MAP_KEY_TEXT(DIK_Z, "z", "Z");

        MAP_KEY(DIK_F1, "F1");
        MAP_KEY(DIK_F2, "F2");
        MAP_KEY(DIK_F3, "F3");
        MAP_KEY(DIK_F4, "F4");
        MAP_KEY(DIK_F5, "F5");
        MAP_KEY(DIK_F6, "F6");
        MAP_KEY(DIK_F7, "F7");
        MAP_KEY(DIK_F8, "F8");
        MAP_KEY(DIK_F9, "F9");
        MAP_KEY(DIK_F10, "F10");
        MAP_KEY(DIK_F11, "F11");
        MAP_KEY(DIK_F12, "F12");
        MAP_KEY(DIK_F13, "F13");
        MAP_KEY(DIK_F14, "F14");
        MAP_KEY(DIK_F15, "F15");

        MAP_KEY_TEXT2(DIK_NUMPAD0, "NUMPAD0", "0", "0");
        MAP_KEY_TEXT2(DIK_NUMPAD1, "NUMPAD1", "1", "1");
        MAP_KEY_TEXT2(DIK_NUMPAD2, "NUMPAD2", "2", "2");
        MAP_KEY_TEXT2(DIK_NUMPAD3, "NUMPAD3", "3", "3");
        MAP_KEY_TEXT2(DIK_NUMPAD4, "NUMPAD4", "4", "4");
        MAP_KEY_TEXT2(DIK_NUMPAD5, "NUMPAD5", "5", "5");
        MAP_KEY_TEXT2(DIK_NUMPAD6, "NUMPAD6", "6", "6");
        MAP_KEY_TEXT2(DIK_NUMPAD7, "NUMPAD7", "7", "7");
        MAP_KEY_TEXT2(DIK_NUMPAD8, "NUMPAD8", "8", "8");
        MAP_KEY_TEXT2(DIK_NUMPAD9, "NUMPAD9", "9", "9");
        MAP_KEY(DIK_NUMPADENTER, "NUMPADReturn");

        MAP_KEY(DIK_UP, "Up");
        MAP_KEY(DIK_DOWN, "Down");
        MAP_KEY(DIK_LEFT, "Left");
        MAP_KEY(DIK_RIGHT, "Right");

        MAP_KEY(DIK_PGUP, "PageUp");
        MAP_KEY(DIK_PGDN, "PageDown");
        MAP_KEY(DIK_HOME, "Home");
        MAP_KEY(DIK_END, "End");

        MAP_KEY(DIK_NUMLOCK, "NumLock");
        MAP_KEY(DIK_SYSRQ, "SysRq");
        MAP_KEY(DIK_SCROLL, "Scroll");
        MAP_KEY(DIK_PAUSE, "Pause");

        MAP_KEY(DIK_RSHIFT, "Shift");
        MAP_KEY(DIK_LSHIFT, "Shift");
        MAP_KEY(DIK_RALT, "Alt");
        MAP_KEY(DIK_LALT, "Alt");

        MAP_KEY(DIK_INSERT, "Insert");
        MAP_KEY(DIK_DELETE, "Delete");

        MAP_KEY(DIK_LWIN, "Win");
        MAP_KEY(DIK_RWIN, "Win");
        MAP_KEY(DIK_APPS, "Menu");





        win = (HWND)window;

        // create the root interface
        HRESULT r = DirectInput8Create(GetModuleHandle(0),
                                       DIRECTINPUT_VERSION,
                                       IID_IDirectInput8,
                                       (void**)&directInput,
                                       NULL);
        if (FAILED(r)) BAD_DI_RESULT(r,"creating di8 root object");

        // create a keyboard device
        r = directInput->CreateDevice(GUID_SysKeyboard,&dev,NULL);
        if (FAILED(r)) BAD_DI_RESULT(r,"creating di8 device");

        // Keyboard2 gives us more buttons - 8 instead of 4
        r = dev->SetDataFormat(&c_dfDIKeyboard);
        if (FAILED(r)) BAD_DI_RESULT(r,"calling SetDataFormat");

        r = dev->SetCooperativeLevel(win, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
        if (FAILED(r)) BAD_DI_RESULT(r,"calling SetCooperativeLevel");

        // http://msdn2.microsoft.com/en-us/library/bb219669.aspx
        DIPROPDWORD dipdw;
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = BUFFSZ;

        // set buffer size to BUFFSZ
        r = dev->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph );
        if (FAILED(r)) BAD_DI_RESULT(r,"setting buffer size");

        lastTime = millis();
}

KeyboardDirectInput8::~KeyboardDirectInput8()
{
        if (dev) {
                dev->Unacquire();
                dev->Release();
                dev = NULL;
        }
        if (directInput) {
                directInput->Release();
        }
}

bool KeyboardDirectInput8::hasFocus (void)
{
        return GetActiveWindow() == win;
}

unsigned long repeatRate = 60;
unsigned long repeatDelay = 300;

Keyboard::Presses KeyboardDirectInput8::getPresses()
{
        typedef std::map<DWORD, ULONGLONG>::iterator I;

        Keyboard::Presses ret;

        ULONGLONG this_time = millis();

        DWORD num_elements = BUFFSZ;
        DIDEVICEOBJECTDATA buf[BUFFSZ];

        HRESULT r;
        r = dev->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),buf,&num_elements,0);
        if (r==DIERR_INPUTLOST || r==DIERR_NOTACQUIRED) {
                r = dev->Acquire();
                if (r==DIERR_OTHERAPPHASPRIO) {
                        // let function exit normally but with no data
                } else if (FAILED(r)) {
                        BAD_DI_RESULT(r,"reacquiring keyboard");
                }
        } else if (FAILED(r)) {
                BAD_DI_RESULT(r,"getting keyboard data");
        } else {
                // finally, process events
                for (DWORD i=0 ; i<num_elements ; i++) {
                        DWORD kc = buf[i].dwOfs;
                        bool down = 0!=(buf[i].dwData & 0x80);
                        if (verbose) {
                                CLOG << "dinput: " << kc
                                     << " = " << down << std::endl;
                        }
                        const char *keystr;
                        const char *keystr_text = NULL;
                        if (down) {
                                pressTime[kc] = this_time;
                                keystr = keysDown[kc];
                                if (pressTime.find(DIK_LCONTROL) == pressTime.end() &&
                                    pressTime.find(DIK_RCONTROL) == pressTime.end() &&
                                    pressTime.find(DIK_LALT) == pressTime.end() &&
                                    pressTime.find(DIK_RALT) == pressTime.end()) {
                                        if (pressTime.find(DIK_LSHIFT) != pressTime.end() || pressTime.find(DIK_RSHIFT) != pressTime.end()) {
                                                keystr_text = keysTextShifted[kc];
                                        } else {
                                                keystr_text = keysText[kc];
                                        }
                                }
                        } else {
                                pressTime.erase(kc);
                                keystr = keysUp[kc];
                        }
                        if (keystr!=NULL) {
                                ret.push_back(keystr);
                        } else {
                                CERR << "dinput: unrecognised key: " << kc
                                     << " = " << down << std::endl;
                        }
                        if (keystr_text!=NULL) {
                                ret.push_back(keystr_text);
                        }
                }
        }

        for (I i=pressTime.begin(), i_=pressTime.end() ; i!=i_ ; ++i) {
                // repeat
                DWORD key = i->first;
                ULONGLONG &press_time = i->second;
                if (press_time + repeatDelay > this_time) continue; // hasn't been held down long enough yet
                ULONGLONG repeat_period = 1000 / repeatRate;
                for (; press_time + repeatDelay + repeat_period <= this_time ;
                       press_time += repeat_period) {
                        ret.push_back(keysRep[key]);
                        if (pressTime.find(DIK_LCONTROL) == pressTime.end() &&
                            pressTime.find(DIK_RCONTROL) == pressTime.end() &&
                            pressTime.find(DIK_LALT) == pressTime.end() &&
                            pressTime.find(DIK_RALT) == pressTime.end()) {
                                if (pressTime.find(DIK_LSHIFT) != pressTime.end() || pressTime.find(DIK_RSHIFT) != pressTime.end()) {
                                        if (keysTextShifted[key]) ret.push_back(keysTextShifted[key]);
                                } else {
                                        if (keysText[key]) ret.push_back(keysText[key]);
                                }
                        }
                }
        }

        lastTime = this_time;
        return ret;
}

// vim: shiftwidth=8:tabstop=8:expandtab
