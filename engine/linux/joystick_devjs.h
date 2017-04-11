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

#ifndef __JOYSTICK_DEVJS_H__
#define __JOYSTICK_DEVJS_H__

#include <X11/Xlib.h>

#include "../joystick.h"

#define JOYSTICK_DEVJS_EVENT_TYPE_BUTTON         0x01    // boolean: button pressed(1=on),released
#define JOYSTICK_DEVJS_EVENT_TYPE_AXE            0x02    // analog:  axe -32767 .. +32767
#define JOYSTICK_DEVJS_EVENT_TYPE_INIT           0x80    // initial state

struct JoystickDevjsEvent {
   uint32_t timestamp;  // timestamp in ms
   int16_t value;   // value bool or analog
   uint8_t type;  // event type: axe or button
   uint8_t index;  // index of axe or button
};

struct JoystickDevjsValue {
    int button[JOYSTICK_MAX_NUM_BUTTONS];
    int axe[JOYSTICK_MAX_NUM_AXES];
};


class JoystickDevjs : public Joystick {

    public:

    JoystickDevjs (size_t window);
    virtual ~JoystickDevjs ();

    virtual bool getEvents(std::vector<signed char> *buttons_indexes_and_values,
                           std::vector<signed char> *axes_indexes,
                           std::vector<short int> *axes_values);

    protected:

    Window win;

    int fd=-1;
    struct JoystickDevjsEvent event;
    struct JoystickDevjsValue value;
};

#endif //__JOYSTICK_DEVJS_H__
