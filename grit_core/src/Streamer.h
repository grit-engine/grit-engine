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

class GritClass;

#ifndef Streamer_h
#define Streamer_h

#include <map>

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "GritObject.h"
#include "math_util.h"

extern float streamer_visibility;
extern float streamer_prepare_distance_factor;
extern float streamer_fade_out_factor;
extern float streamer_fade_overlap_factor;

enum CoreBoolOption {
    CORE_AUTOUPDATE
};

enum CoreFloatOption {
    CORE_VISIBILITY,
    CORE_PREPARE_DISTANCE_FACTOR,
    CORE_FADE_OUT_FACTOR,
    CORE_FADE_OVERLAP_FACTOR
};

enum CoreIntOption {
    CORE_STEP_SIZE
};

// set's t to either 0,1,2 and fills in the approriate argument
void core_option_from_string (const std::string &s,
                             int &t,
                             CoreBoolOption &o0,
                             CoreIntOption &o1,
                             CoreFloatOption &o2);

void core_option (CoreBoolOption o, bool v);
bool core_option (CoreBoolOption o);
void core_option (CoreIntOption o, int v);
int core_option (CoreIntOption o);
void core_option (CoreFloatOption o, float v);
float core_option (CoreFloatOption o);

void streamer_init();

void streamer_centre (lua_State *L, const Vector3 &new_pos);

// called by objects when they change position or rendering distance
void streamer_update_sphere (size_t index, const Vector3 &pos, float d);
        
void streamer_list (const GritObjectPtr &o);

void streamer_unlist (const GritObjectPtr &o);

void streamer_list_as_activated (const GritObjectPtr &o);

void streamer_unlist_as_activated (const GritObjectPtr &o);

void streamer_object_activated (GObjPtrs::iterator &begin, GObjPtrs::iterator &end);

int streamer_object_activated_count (void);



struct StreamerCallback {
        virtual void update(const Vector3 &new_pos) = 0;
};

void streamer_callback_register (StreamerCallback *rc);

void streamer_callback_unregister (StreamerCallback *rc);

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
