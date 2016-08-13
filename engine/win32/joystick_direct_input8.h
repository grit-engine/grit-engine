/*
 * Copyright (c) François Dorval fdorval@hotmail.com and the Grit Game Engine project 2016 
 * Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php 
 */

#ifndef JOYSTICK_DIRECT_INPUT8_H
#define JOYSTICK_DIRECT_INPUT8_H

#include "../joystick.h"

// Temporary stub implementation.
class JoystickDirectInput8 : public Joystick {

public:

        JoystickDirectInput8 (size_t) { }
        virtual ~JoystickDirectInput8 () { };

        virtual bool getEvents(std::vector<signed char> *,
                               std::vector<signed char> *,
                               std::vector<short int> *) { return false; }

};

#endif //JOYSTICK_DIRECT_INPUT8_H

