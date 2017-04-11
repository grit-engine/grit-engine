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

// based on https://www.kernel.org/doc/Documentation/input/joystick-api.txt
//
// REQUIREMENTS: a physical joystick/gamepad and libsdl2-dev
//
// TESTED WITH: Gamepad/Joystick USB Rock Candy for Xbox 360
//
// LINUX DRIVER INSTALLATION:
// ubuntu$ sudo apt-get install libsdl2-dev 
//
// LINUX TEST: (plug joystick/gamepad)
// The device shall not generate any events during idle state (ie: when not moving sticks or
// pressing buttons) otherwise grit engine will be flooded with events and may decrease performance
// of a game engine.
// ubuntu$ jstest --normal /dev/input/js0
//
// LINUX CALIBRATION: (plug joystick/gamepad)
// During calibration when asked to press a button or move a stick hold action until specified to
// release otherwise wrong calibration values will be computed.
// ubuntu$ jscal /dev/input/js0
// 
// DISPLAY CALIBRATION: 
// ubuntu$ jscal -p /dev/input/js0
//
// CREATE CALIBRATION SCRIPT: 

/*
Created once after you have done the calibration, execute this script to restore your calibration
after reboot if necessary

# create
echo "#!/bin/bash" >calibrate.sh
jscal -p /dev/input/js0 >>calibrate.sh
chmod +x calibrate.sh

# restore
./calibrate.sh

*/

/*
Calibrate reset script for Rock Candy.  (Warning! this script erase the current linux joystick
calibration to default neutral value.  You may need to adapt this script to the number of
buttons/axes of your device.)

#!/bin/bash
jscal -s 8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 /dev/input/js0
jscal -p /dev/input/js0
jscal -c /dev/input/js0

*/

/*

In the event that jscal did not compute the correct values, here is manually calibrated values for
Rock Candy.  (Warning! this command overwrite the current linux joystick calibration and reduce
precision/range of your device.  Reuse calibaration procedure or reset procedure to recover full
precision/range of your device.  You may need to adapt this command to the number of buttons/axes of
your device.)

jscal -s 8,1,8000,-4000,4000,18663,18663,1,8000,-4000,4000,18663,18663,0,0,1,8000,-4000,4000,\
18663,18663,1,8000,-4000,4000,18663,18663,0,0,0,0,0,0 /dev/input/js0

Details:
8         number of axes
# First axis:
1         1 = correction and 4 coefficients, otherwise 0 no correction and no 4 coefficients after
8000      precision range, -4000..4000 = 8000
-4000     minimum value for center, ignore 0..-4000  
4000      maximum value for center, ignore 0..4000
18663     round((32767 / (abs(-32767) - abs(-4000))) * 16384)
          = (normal_range / reduced_range) * 16384 = 18662.15 >> 18663
18663     round((32767 / (abs(32767) - abs(4000))) * 16384)
# Second axis:
...
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

#include <centralised_log.h>

#include "joystick_devjs.h"

JoystickDevjs::JoystickDevjs(size_t window)
{
    win = window;

    // If this fails, fd will be < 0 and JoystickDevjs will be a stub.
    fd = open ("/dev/input/js0", O_RDONLY | O_NONBLOCK);
}

JoystickDevjs::~JoystickDevjs()
{
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

bool JoystickDevjs::getEvents(std::vector<signed char> *buttons_indexes_and_values,
                              std::vector<signed char> *axes_indexes,
                              std::vector<short int> *axes_values) 
{
    struct JoystickDevjsEvent event;
    if (buttons_indexes_and_values) buttons_indexes_and_values->clear();
    if (axes_indexes) axes_indexes->clear();
    if (axes_values) axes_values->clear();

    if (fd < 0) return false;

    bool moved = false;

    while (true) {
        int n = read(fd, &event, sizeof(struct JoystickDevjsEvent));
        if (n != sizeof(struct JoystickDevjsEvent)) break;

        // event.type &= ~ JOYSTICK_DEVJS_EVENT_TYPE_INIT; /* discard init events */

        switch (event.type) {
            case JOYSTICK_DEVJS_EVENT_TYPE_BUTTON: {
                signed char pushed_button_index_and_value;
                signed char button_index = event.index+1;
                
                if ( button_index <= JOYSTICK_MAX_NUM_BUTTONS ) {
                    if ( buttons_indexes_and_values ) {
                        signed char mod=-1;
                        if( event.value ) mod=1;
                        pushed_button_index_and_value = button_index;
                        pushed_button_index_and_value = pushed_button_index_and_value * mod;
                        buttons_indexes_and_values->push_back(pushed_button_index_and_value);
                        moved = true;
                        // printf("B%02d:%02d:%02d\n",
                        //        button_index, event.value, pushed_button_index_and_value);
                    }
                }
            }
            break;

            case JOYSTICK_DEVJS_EVENT_TYPE_AXE: {
                signed char pushed_axe_index = event.index + 1;
                short int pushed_axe_value = event.value;

                if (pushed_axe_index <= JOYSTICK_MAX_NUM_AXES) {
                    if (axes_indexes && axes_values) {
                       axes_indexes->push_back(pushed_axe_index);
                       axes_values->push_back(pushed_axe_value);
                       moved = true;
                       // printf("J%02d:%06d:%d\n",
                       //        pushed_axe_index, event.value, pushed_axe_value);
                    }
                }
            }
            break;
        }
    }

    return moved;
}
