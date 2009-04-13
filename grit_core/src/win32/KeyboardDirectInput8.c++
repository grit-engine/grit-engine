#include <assert.h>
#include <iostream>

#include "KeyboardDirectInput8.h"
#include "../app_error.h"


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

#define BAD_DI_RESULT(r,i_was) app_error(__FILE__,__LINE__,i_was,result_str(r))

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
                              keysUp.insert(std::make_pair(kc,"-"k))

        MAP_KEY(DIK_1, "1");
        MAP_KEY(DIK_2, "2");
        MAP_KEY(DIK_3, "3");
        MAP_KEY(DIK_4, "4");
        MAP_KEY(DIK_5, "5");
        MAP_KEY(DIK_6, "6");
        MAP_KEY(DIK_7, "7");
        MAP_KEY(DIK_8, "8");
        MAP_KEY(DIK_9, "9");
        MAP_KEY(DIK_0, "0");

        MAP_KEY(DIK_BACK, "BackSpace");

        MAP_KEY(DIK_MINUS, "-");
        MAP_KEY(DIK_EQUALS, "=");
        MAP_KEY(DIK_SPACE, "Space");
        MAP_KEY(DIK_COMMA, ",");
        MAP_KEY(DIK_PERIOD, ".");

        MAP_KEY(DIK_BACKSLASH, "\\");
        MAP_KEY(DIK_SLASH, "/");
        MAP_KEY(DIK_LBRACKET, "[");
        MAP_KEY(DIK_RBRACKET, "]");

        MAP_KEY(DIK_ESCAPE,"Escape");
        MAP_KEY(DIK_CAPSLOCK, "CapsLock");

        MAP_KEY(DIK_TAB, "Tab");
        MAP_KEY(DIK_RETURN, "Return");
        MAP_KEY(DIK_LCONTROL, "LCtrl");
        MAP_KEY(DIK_RCONTROL, "RCtrl");

        MAP_KEY(DIK_COLON, ":");
        MAP_KEY(DIK_SEMICOLON, ";");
        MAP_KEY(DIK_APOSTROPHE, "'");
        MAP_KEY(DIK_GRAVE, "`");

        MAP_KEY(DIK_A, "A");
        MAP_KEY(DIK_B, "B");
        MAP_KEY(DIK_C, "C");
        MAP_KEY(DIK_D, "D");
        MAP_KEY(DIK_E, "E");
        MAP_KEY(DIK_F, "F");
        MAP_KEY(DIK_G, "G");
        MAP_KEY(DIK_H, "H");
        MAP_KEY(DIK_I, "I");
        MAP_KEY(DIK_J, "J");
        MAP_KEY(DIK_K, "K");
        MAP_KEY(DIK_L, "L");
        MAP_KEY(DIK_M, "M");
        MAP_KEY(DIK_N, "N");
        MAP_KEY(DIK_O, "O");
        MAP_KEY(DIK_P, "P");
        MAP_KEY(DIK_Q, "Q");
        MAP_KEY(DIK_R, "R");
        MAP_KEY(DIK_S, "S");
        MAP_KEY(DIK_T, "T");
        MAP_KEY(DIK_U, "U");
        MAP_KEY(DIK_V, "V");
        MAP_KEY(DIK_W, "W");
        MAP_KEY(DIK_X, "X");
        MAP_KEY(DIK_Y, "Y");
        MAP_KEY(DIK_Z, "Z");

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

        MAP_KEY(DIK_NUMPAD0, "NUMPAD0");
        MAP_KEY(DIK_NUMPAD1, "NUMPAD1");
        MAP_KEY(DIK_NUMPAD2, "NUMPAD2");
        MAP_KEY(DIK_NUMPAD3, "NUMPAD3");
        MAP_KEY(DIK_NUMPAD4, "NUMPAD4");
        MAP_KEY(DIK_NUMPAD5, "NUMPAD5");
        MAP_KEY(DIK_NUMPAD6, "NUMPAD6");
        MAP_KEY(DIK_NUMPAD7, "NUMPAD7");
        MAP_KEY(DIK_NUMPAD8, "NUMPAD8");
        MAP_KEY(DIK_NUMPAD9, "NUMPAD9");
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

        MAP_KEY(DIK_RSHIFT, "RShift");
        MAP_KEY(DIK_LSHIFT, "LShift");
        MAP_KEY(DIK_RALT, "RAlt");
        MAP_KEY(DIK_LALT, "LAlt");

        MAP_KEY(DIK_INSERT, "Insert");
        MAP_KEY(DIK_DELETE, "Delete");

        MAP_KEY(DIK_LWIN, "LWin");
        MAP_KEY(DIK_RWIN, "RWin");
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

unsigned long repeatRate = 60;
unsigned long repeatDelay = 300;
        

Keyboard::Presses KeyboardDirectInput8::getPresses()
{
        ULONGLONG this_time = millis();

        Keyboard::Presses ret;

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
                        if (down) {
                                pressTime[kc] = this_time;
                                ret.push_back(keysDown[kc]);
                        } else {
                                pressTime.erase(kc);
                                ret.push_back(keysUp[kc]);
                        }
                }
        }

        typedef std::map<DWORD, ULONGLONG>::iterator I;
        for (I i=pressTime.begin(), i_=pressTime.end() ; i!=i_ ; ++i) {
                // repeat
                DWORD key = i->first;
                ULONGLONG &press_time = i->second;
                if (press_time + repeatDelay > this_time) continue; // hasn't been held down long enough yet
                ULONGLONG repeat_period = 1000 / repeatRate;
                for (; press_time + repeatDelay + repeat_period <= this_time ; press_time += repeat_period) {
                        ret.push_back(keysRep[key]);
                }
        }

        lastTime = this_time;
        return ret;
}

// vim: shiftwidth=8:tabstop=8:expandtab
