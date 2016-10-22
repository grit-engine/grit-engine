/* Copyright (c) The Grit Game Engine authors 2016
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

#include <X11/Xlib.h>


#include "../mouse.h"


class MouseX11 : public Mouse {

public:

        MouseX11 (size_t window);
        virtual ~MouseX11 ();

        virtual bool getEvents(std::vector<int> *clicks,
                               int *x, int *y, int *rel_x, int *rel_y);

        virtual void setPos(int x, int y);

        virtual void setHide(bool toggle);
        virtual bool getHide();

        virtual void setGrab(bool toggle);
        virtual bool getGrab();

protected:

        Display *display;
        Window win;
        Cursor blankCursor;


        // this is where the game or whatever thinks the mouse is
        int current_x;
        int current_y;

        // record whether the user asked for mouse grab / pointer hide.
        // when we regain focus we need to re-establish the grab
        bool requested_grab;
        bool requested_hide;

        // this is where X11 thinks the mouse is
        int real_last_x;
        int real_last_y;

        // used to signal the event listener that there will be a motion notify
        // event that needs to be ignored
        int warped_x;
        int warped_y;
        bool just_warped;
};

// vim: shiftwidth=8:tabstop=8:expandtab
