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

#include <assert.h>
#include <iostream>

#include "MouseDirectInput8.h"
#include "../CentralisedLog.h"


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

MouseDirectInput8::MouseDirectInput8(size_t window)
{

        win = (HWND)window;

        // create the root interface
        HRESULT r = DirectInput8Create(GetModuleHandle(0),
                                       DIRECTINPUT_VERSION,
                                       IID_IDirectInput8,
                                       (void**)&directInput,
                                       NULL);
        assert(!FAILED(r));

        // create a mouse device
        r = directInput->CreateDevice(GUID_SysMouse,&dev,NULL);
        assert(!FAILED(r));

        // Mouse2 gives us more buttons - 8 instead of 4
        r = dev->SetDataFormat(&c_dfDIMouse2);
        assert(!FAILED(r));

        r = dev->SetCooperativeLevel(win, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
        assert(!FAILED(r));

        // http://msdn2.microsoft.com/en-us/library/bb219669.aspx
        DIPROPDWORD dipdw;
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = BUFFSZ;

        // set buffer size to BUFFSZ
        r = dev->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph );
        assert(!FAILED(r));

        hidden = false;
        grabbed = false;

        scroll = 0;
}

MouseDirectInput8::~MouseDirectInput8()
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

bool MouseDirectInput8::getEvents(std::vector<int> *clicks,
                           int *x, int *y, int *rel_x, int *rel_y)
{
        if (clicks) clicks->clear();

        int rx = 0;
        int ry = 0;
        bool moved = false;

        DWORD num_elements = BUFFSZ;
        DIDEVICEOBJECTDATA buf[BUFFSZ];

        // Get window size
        RECT rect;
        GetClientRect(win,&rect);
        int win_height = rect.bottom - rect.top;

        HRESULT r;
        r = dev->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),buf,&num_elements,0);
        if (r==DIERR_INPUTLOST || r==DIERR_NOTACQUIRED) {
                r = dev->Acquire();
                if (r==DIERR_OTHERAPPHASPRIO) {
                        // let function exit normally but with no data
                } else if (FAILED(r)) {
                        BAD_DI_RESULT(r,"reacquiring mouse");
                }
        } else if (FAILED(r)) {
                BAD_DI_RESULT(r,"getting mouse data");
        } else {
                // finally, process events
                for (DWORD i=0 ; i<num_elements ; i++) {
                        POINT p; GetCursorPos(&p);
                        ScreenToClient(win,&p);

                        if (p.x<0 || p.x>=rect.right) continue;
                        if (p.y<0 || p.y>=rect.bottom) continue;

                        DWORD d = buf[i].dwData;

                        if (buf[i].dwOfs==DIMOFS_BUTTON0 && clicks) {
                                clicks->push_back((d&0x80?1:-1)*MOUSE_LEFT);
                        } else if (buf[i].dwOfs==DIMOFS_BUTTON1 && clicks) {
                                clicks->push_back((d&0x80?1:-1)*MOUSE_RIGHT);
                        } else if (buf[i].dwOfs==DIMOFS_BUTTON2 && clicks) {
                                clicks->push_back((d&0x80?1:-1)*MOUSE_MIDDLE);
                        } else if (buf[i].dwOfs==DIMOFS_X) {
                                moved = true;
                                rx += (int)d;
                                current_x = p.x;
                        } else if (buf[i].dwOfs==DIMOFS_Y) {
                                moved = true;
                                ry += (int)d;
                                current_y = p.y;
                        } else if (buf[i].dwOfs==DIMOFS_Z) {
                                scroll += (int)d;
                                while (scroll>=WHEEL_DELTA) {
                                        clicks->push_back(MOUSE_SCROLL_UP);
                                        scroll -= WHEEL_DELTA;
                                }
                                while (scroll<=-WHEEL_DELTA) {
                                        clicks->push_back(MOUSE_SCROLL_DOWN);
                                        scroll += WHEEL_DELTA;
                                }
                        }
                }
        }

        if (x) *x = current_x;
        if (y) *y = win_height-current_y-1; // invert axis
        if (rel_x) *rel_x = rx;
        if (rel_y) *rel_y = -ry; // invert axis

        return moved;
}

void MouseDirectInput8::setPos(int x, int y)
{
        // Get window size
        RECT rect;
        GetClientRect(win,&rect);
        int win_height = rect.bottom - rect.top;

        y = win_height - y - 1;

        POINT p;
        p.x = x;
        p.y = y;
        ClientToScreen(win,&p);
        SetCursorPos(p.x,p.y);
        current_x = x;
        current_y = y;
}


void MouseDirectInput8::setHide(bool toggle)
{
        if (hidden==toggle) return;
        hidden = toggle;
        ShowCursor(!toggle);
}

bool MouseDirectInput8::getHide()
{
        return hidden;
}


void MouseDirectInput8::setGrab(bool toggle)
{
        if (grabbed==toggle) return;
        grabbed = toggle;
        if (!toggle) {
                ClipCursor(NULL);
        } else {
                RECT rect;
                GetClientRect(win,&rect);
                POINT p;
                p.x = 0;
                p.y = 0;
                ClientToScreen(win,&p);
                rect.left = p.x;
                rect.top = p.y;
                rect.right += p.x;
                rect.bottom += p.y;
                ClipCursor(&rect);
        }
}

bool MouseDirectInput8::getGrab()
{
        return grabbed;
}



// vim: shiftwidth=8:tabstop=8:expandtab
