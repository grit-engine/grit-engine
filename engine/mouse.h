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

#ifndef Mouse_h
#define Mouse_h

#include <vector>

/** Maximum number of buttons allowed on a mouse. */
#define NUM_BUTTONS 8

/** A class to be inheritted by a particular platform's implementation for
 * signalling mouse input to the engine. */
class Mouse {

public:

        /** The kind of mouse button being pressed. */
        enum Button
        {
                MOUSE_LEFT = 1, MOUSE_RIGHT, MOUSE_MIDDLE,
                MOUSE_SCROLL_UP, MOUSE_SCROLL_DOWN
        };

        /** Empty destructor. */
        virtual ~Mouse() { }

        /** Return the mouse events since the last call.  Should be called frequently (e.g. every graphics frame).
         * \param clicks A list of mouse buttons pressed since the last call.
         * \param x Returns the current x position of the mouse in window coordinates.
         * \param y Returns the current y position of the mouse in window coordinates.
         * \param rel_x Returns the change in x (can be used to calculate the velocity of the mouse).
         * \param rel_y Returns the change in y (can be used to calculate the velocity of the mouse).
         */
        virtual bool getEvents(std::vector<int> *clicks,
                               int *x, int *y, int *rel_x, int *rel_y) = 0;

        /** Set the mouse position to the given window coordinates. */
        virtual void setPos(int x, int y) = 0;


        /** Set whether or not the mouse cursor is hidden (invisible). */
        virtual void setHide(bool toggle) = 0;

        /** Get whether or not the mouse cursor is hidden (invisible). */
        virtual bool getHide() = 0;


        /** Get whether or not the mouse cursor is hidden (invisible). */
        virtual void setGrab(bool toggle) = 0;

        /** Get whether or not the mouse cursor is hidden (invisible). */
        virtual bool getGrab() = 0;

};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
