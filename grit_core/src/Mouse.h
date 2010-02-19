/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

#ifndef Mouse_h
#define Mouse_h

#include <vector>

#define NUM_BUTTONS 8

class Mouse {

public:

        enum Button
        {
                MOUSE_LEFT = 1, MOUSE_RIGHT, MOUSE_MIDDLE,
                MOUSE_SCROLL_UP, MOUSE_SCROLL_DOWN
        };

        virtual ~Mouse() = 0;

        virtual bool getEvents(std::vector<int> *clicks,
                               int *x, int *y, int *rel_x, int *rel_y) = 0;

        virtual void setPos(int x, int y) = 0;

        virtual void setHide(bool toggle) = 0;
        virtual bool getHide() = 0;

        virtual void setGrab(bool toggle) = 0;
        virtual bool getGrab() = 0;

};

inline Mouse::~Mouse() { }

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
