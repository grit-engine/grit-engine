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

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "../Mouse.h"


class MouseDirectInput8 : public Mouse {

public:

        MouseDirectInput8 (size_t);
        virtual ~MouseDirectInput8 ();

        virtual bool getEvents(std::vector<int> *clicks,
                               int *x, int *y, int *rel_x, int *rel_y);

        virtual void setPos(int x, int y);

        virtual void setHide(bool toggle);
        virtual bool getHide();

        virtual void setGrab(bool toggle);
        virtual bool getGrab();

protected:

        HWND win;

        IDirectInput8 *directInput;

        IDirectInputDevice8 *dev;

        int current_x, current_y;

        int scroll;

        bool hidden;
        bool grabbed;
};

// vim: shiftwidth=8:tabstop=8:expandtab
