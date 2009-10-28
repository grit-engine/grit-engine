#include <assert.h>
#include <iostream>

#include "KeyboardWinAPI.h"
#include "../CentralisedLog.h"

typedef std::map<HWND, KeyboardWinAPI*> WindowMap;
static WindowMap wmap;

static std::string errorstr (void)
{
        char sys_msg[256];
        DWORD err = GetLastError();

        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), sys_msg, sizeof sys_msg, NULL);

        // Trim the end of the line and terminate it with a null
        char *p = sys_msg;
        while (*p > 31 || *p == 9)
                p++;
        do {
                *p = 0;
                p--;
        } while (p >= sys_msg && (*p == '.' || *p < 33));

        return std::string(sys_msg);
}

static LRESULT WINAPI wndproc (HWND msgwin, UINT msg, WPARAM wParam, LPARAM lParam)
{
        KeyboardWinAPI *kb = wmap[msgwin];
        if (!kb) {
            CERR << "Got a message from an unknown window: 0x" << (void*)msgwin << std::endl;
            return 0;
        }
        return kb->wndproc(msgwin, msg, wParam, lParam);
}

KeyboardWinAPI::KeyboardWinAPI (size_t window)
{
        win = (HWND)window;


        SetLastError(0);
        old_wndproc = (WNDPROC) SetWindowLong(win, GWL_WNDPROC,
                                              (LONG)::wndproc);
        if (!old_wndproc) {
            if (GetLastError()) CERR << "Could not trap keyboard events:  " << errorstr() << std::endl;
        }

        wmap[win] = this;
}

KeyboardWinAPI::~KeyboardWinAPI (void)
{
        SetWindowLong(win, GWL_WNDPROC, (LONG) old_wndproc);
}

bool KeyboardWinAPI::hasFocus (void)
{
        return GetActiveWindow() == win;
}

static std::string break_horribly (TCHAR *buf, size_t sz)
{
        std::string r(sz,' ');
        for (size_t i=0 ; i<sz ; ++i) {
            if (buf[i]<32 || buf[i]>=127)
                buf[i] = '?';
            r[i] = (char) buf[i];
        }
        return r;
}

static Keyboard::Press get_string_press (WPARAM wParam, LPARAM scan_code,
                                         bool shift, bool alt)
{
        Keyboard::Press r;
        BYTE state[256] = {0};
        if (shift) {
                state[VK_SHIFT] = 0xFF;
                state[VK_LSHIFT] = 0xFF;
                state[VK_RSHIFT] = 0xFF;
        }
        if (alt) {
                state[VK_MENU] = 0xFF;
                state[VK_LMENU] = 0xFF;
                state[VK_RMENU] = 0xFF;
                state[VK_CONTROL] = 0xFF;
                state[VK_CONTROL] = 0xFF;
                state[VK_CONTROL] = 0xFF;
        }
        TCHAR buf[1024] = {0};
        int ret = ToUnicodeEx(wParam, scan_code, state,
                              buf, sizeof(buf)/sizeof(*buf),
                              0, GetKeyboardLayout(0));
        // TODO: support unicode properly
        if (ret==-1) {
            return "?";
        } else if (ret<0 || ret>=sizeof(buf)/sizeof(*buf)) {
            CERR << "ToUnicodeEx returned out of range: " << ret << std::endl;
        } else if (ret != 1) {
            return "?";
        } else {
            return break_horribly(buf,ret);
        }
}

Keyboard::Press KeyboardWinAPI::getPress (WPARAM wParam, LPARAM scan_code)
{
        Press unshifted = get_string_press(wParam,scan_code,false,false);
        Press shifted = get_string_press(wParam,scan_code,true,false);
        Press alted = get_string_press(wParam,scan_code,false,true);
        if (shiftMap[unshifted] == "") {
            if (verbose) CLOG << unshifted << " -> " << shifted << std::endl;
            shiftMap[unshifted] = shifted;
        }
        if (altMap[unshifted] == "") {
            if (verbose) CLOG << unshifted << " => " << alted << std::endl;
            altMap[unshifted] = alted;
        }
        return unshifted;
}

LRESULT KeyboardWinAPI::wndproc (HWND msgwin, UINT msg, WPARAM wParam, LPARAM lParam)
{
        LPARAM repeat_count = lParam & 0x0000FFFF;
        LPARAM scan_code = lParam & 0x00FF0000;
        LPARAM extended = lParam & 0x01000000;
        LPARAM context_code = lParam & 0x20000000;
        LPARAM was_down = lParam & 0x40000000;
        LPARAM key_up = lParam & 0x80000000;
        std::string key;
        if (key_up) {
            key = "-";
        } else {
            key = was_down ? "=" : "+";
        }
        bool have_key = false;
        switch (msg) {
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                if (verbose) {
                    CLOG << (void*)msg << ": " << key << wParam << ": " << "x" << repeat_count << std::endl;
                }
                switch (wParam) {
                        #define MAP_KEY(vk, name) case vk: { \
                            key += name; \
                            have_key = true; \
                        }; break
                        MAP_KEY(VK_SPACE, "Space");
                        MAP_KEY(VK_BACK, "BackSpace");
                        MAP_KEY(VK_ESCAPE,"Escape");
                        MAP_KEY(VK_CAPITAL, "CapsLock");
                        MAP_KEY(VK_TAB, "Tab");
                        MAP_KEY(VK_RETURN, "Return");
                        MAP_KEY(VK_CONTROL, "Ctrl");
                        //MAP_KEY(VK_LCONTROL, "Ctrl");
                        //MAP_KEY(VK_RCONTROL, "Ctrl");
                        MAP_KEY(VK_MENU, "Alt");
                        //MAP_KEY(VK_LMENU, "Alt");
                        //MAP_KEY(VK_RMENU, "Alt");

                        MAP_KEY(VK_F1, "F1");
                        MAP_KEY(VK_F2, "F2");
                        MAP_KEY(VK_F3, "F3");
                        MAP_KEY(VK_F4, "F4");
                        MAP_KEY(VK_F5, "F5");
                        MAP_KEY(VK_F6, "F6");
                        MAP_KEY(VK_F7, "F7");
                        MAP_KEY(VK_F8, "F8");
                        MAP_KEY(VK_F9, "F9");
                        MAP_KEY(VK_F10, "F10");
                        MAP_KEY(VK_F11, "F11");
                        MAP_KEY(VK_F12, "F12");
                        MAP_KEY(VK_F13, "F13");
                        MAP_KEY(VK_F14, "F14");
                        MAP_KEY(VK_F15, "F15");

                        MAP_KEY(VK_NUMPAD0, "NUMPAD0");
                        MAP_KEY(VK_NUMPAD1, "NUMPAD1");
                        MAP_KEY(VK_NUMPAD2, "NUMPAD2");
                        MAP_KEY(VK_NUMPAD3, "NUMPAD3");
                        MAP_KEY(VK_NUMPAD4, "NUMPAD4");
                        MAP_KEY(VK_NUMPAD5, "NUMPAD5");
                        MAP_KEY(VK_NUMPAD6, "NUMPAD6");
                        MAP_KEY(VK_NUMPAD7, "NUMPAD7");
                        MAP_KEY(VK_NUMPAD8, "NUMPAD8");
                        MAP_KEY(VK_NUMPAD9, "NUMPAD9");

                        MAP_KEY(VK_UP, "Up");
                        MAP_KEY(VK_DOWN, "Down");
                        MAP_KEY(VK_LEFT, "Left");
                        MAP_KEY(VK_RIGHT, "Right");

                        MAP_KEY(VK_PRIOR, "PageUp");
                        MAP_KEY(VK_NEXT, "PageDown");
                        MAP_KEY(VK_HOME, "Home");
                        MAP_KEY(VK_END, "End");

                        MAP_KEY(VK_NUMLOCK, "NumLock");
                        MAP_KEY(VK_SCROLL, "Scroll");
                        MAP_KEY(VK_PAUSE, "Pause");

                        MAP_KEY(VK_SHIFT, "Shift");

                        MAP_KEY(VK_INSERT, "Insert");
                        MAP_KEY(VK_DELETE, "Delete");

                        MAP_KEY(VK_LWIN, "Win");
                        MAP_KEY(VK_RWIN, "Win");
                        MAP_KEY(VK_APPS, "Menu");

                        MAP_KEY(VK_SNAPSHOT, "PrintScreen");

                        MAP_KEY(VK_CONVERT, "Convert");
                        MAP_KEY(VK_NONCONVERT, "Nonconvert");
                        MAP_KEY(VK_ACCEPT, "Accept");
                        MAP_KEY(VK_MODECHANGE, "Modechange");
                        MAP_KEY(VK_SELECT, "Select");
                        MAP_KEY(VK_PRINT, "Print");
                        MAP_KEY(VK_EXECUTE, "Execute");
                        MAP_KEY(VK_HELP, "Help");

                        default: {
                                key += getPress(wParam, scan_code);
                                have_key = true;
                        }
                }
                break;
        }

        if (have_key) {
                for (int i=0 ; i<repeat_count ; ++i)
                    presses.push_back(key);

                return 0;
        }

        return CallWindowProc(old_wndproc, msgwin, msg, wParam, lParam);
}

Keyboard::Press KeyboardWinAPI::getShifted (const Press &press)
{
        return shiftMap[press];
}

Keyboard::Press KeyboardWinAPI::getAlted (const Press &press)
{
        return altMap[press];
}

Keyboard::Presses KeyboardWinAPI::getPresses (void)
{
        Keyboard::Presses ret;

        for (Presses::iterator i=presses.begin(),i_=presses.end() ; i!=i_ ; ++i) {
                Press p = *i;
                if (p.substr(0,1)=="+") {
                    down.insert(p.substr(1));
                } else if (p.substr(0,1)=="-") {
                    down.erase(p.substr(1));
                }
                ret.push_back(p);
        }
        presses.clear();
        
        for (Presses::iterator i=keysToFlush.begin(),
                               i_=keysToFlush.end() ; i!=i_ ; ++i) {
                if (down.find(*i)!=down.end()) {
                        ret.push_back("-"+*i);
                        down.erase(*i);
                        if (verbose) {
                            CLOG << "winapi: key flushed: " << *i << std::endl;
                        }
                }
        }
        keysToFlush.clear();

        if (flushRequested) {
                for (DownSet::iterator i=down.begin(),i_=down.end() ; i!=i_ ; ++i) {
                        ret.push_back("-"+*i);
                }
                down.clear();
                flushRequested = false;
                if (verbose) {
                    CLOG << "winapi: keyboard flushed" << std::endl;
                }
        }
        return ret;
}

// vim: shiftwidth=8:tabstop=8:expandtab
