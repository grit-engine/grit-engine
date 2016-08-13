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

/*
 * Derivative work based David Cunninghamon mouse_x11.h, mouse_x11.cpp and mouse.h 
 * Copyright (c) François Dorval fdorval@hotmail.com and the Grit Game Engine project 2016 
 * Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php 
 */

#ifndef Joystick_h
#define Joystick_h

#include <vector>

/** Maximum number of buttons and axes allowed on a joystick. */
#define JOYSTICK_MAX_NUM_BUTTONS 11  // 1 to 11
#define JOYSTICK_MAX_NUM_AXES 8      // 1 to 8

/** A class to be inheritted by a particular platform's implementation for
 * signalling joystick input to the engine. */
class Joystick {

public:

        /** The kind of joystick boolean button being pressed(1=on) or unpressed(0=off). */
        enum Button
        {
                JOYSTICK_BUTTON1 = 1, JOYSTICK_BUTTON2, JOYSTICK_BUTTON3,
                JOYSTICK_BUTTON4, JOYSTICK_BUTTON5, JOYSTICK_BUTTON6,
                JOYSTICK_BUTTON7, JOYSTICK_BUTTON8, JOYSTICK_BUTTON9,
                JOYSTICK_BUTTON10,JOYSTICK_BUTTON11
        };

        /** The kind of joystick analog axes being moved */
        enum Axe
        {
                JOYSTICK_AXE1 = 1, JOYSTICK_AXE2, JOYSTICK_AXE3,
                JOYSTICK_AXE4, JOYSTICK_AXE5, JOYSTICK_AXE6,
                JOYSTICK_AXE7, JOYSTICK_AXE8
        };

        /** Empty destructor. */
        virtual ~Joystick() { }

        /** Return the joystick events since the last call.  Should be called frequently (e.g. every graphics frame).
         * \param buttons A list of joystick buttons pressed since the last call.
         * \param axes    A list of joystick buttons pressed since the last call.
         * \param axes    A list of joystick buttons pressed since the last call.
         */
        virtual bool getEvents(std::vector<signed char> *buttons, std::vector<signed char> *axes, std::vector<short int> *values) = 0;

        /** Set the joystick position to the given window coordinates. */
        virtual void setPos(int x, int y) = 0;

        /** Set whether or not the joystick cursor is hidden (invisible). */
        virtual void setHide(bool toggle) = 0;

        /** Get whether or not the joystick cursor is hidden (invisible). */
        virtual bool getHide() = 0;

        /** Set whether or not the joystick cursor is hidden (invisible). */
        virtual void setGrab(bool toggle) = 0;

        /** Get whether or not the joystick cursor is hidden (invisible). */
        virtual bool getGrab() = 0;

};

#endif //Joystick_h


