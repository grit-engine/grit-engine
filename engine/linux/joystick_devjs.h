/*
 * Copyright (c) François Dorval fdorval@hotmail.com and the Grit Game Engine project 2016 
 * Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php 
 */

#ifndef __JOYSTICK_DEVJS_H__
#define __JOYSTICK_DEVJS_H__

#include <X11/Xlib.h>

#include "../joystick.h"

#define JOYSTICK_DEVJS_EVENT_TYPE_BUTTON         0x01    /* boolean: button pressed(1=on),released */
#define JOYSTICK_DEVJS_EVENT_TYPE_AXE            0x02    /* analog:  axe -32767 .. +32767 */
#define JOYSTICK_DEVJS_EVENT_TYPE_INIT           0x80    /* initial state */

struct JoystickDevjsEvent {
       uint32_t timestamp;     /* timestamp in ms */
       int16_t value;     /* value bool or analog */
       uint8_t type;      /* event type: axe or button */
       uint8_t index;    /* index of axe or button */
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

        virtual void setPos(int x, int y);

        virtual void setHide(bool toggle);
        virtual bool getHide();

        virtual void setGrab(bool toggle);
        virtual bool getGrab();

protected:

        Window win;

        int fd=-1;
        struct JoystickDevjsEvent event;
        struct JoystickDevjsValue value;
     
};

#endif //__JOYSTICK_DEVJS_H__

