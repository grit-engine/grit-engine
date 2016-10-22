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

class GritClass;

#ifndef Streamer_h
#define Streamer_h

#include <map>

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "grit_object.h"
#include "math_util.h"

/** Single var cache of CORE_PREPARE_DISTANCE_FACTOR. */
extern float streamer_prepare_distance_factor;

/** Single var cache of CORE_VISIBILITY. */
extern float streamer_visibility;

/** Single var cache of CORE_FADE_OUT_FACTOR. */
extern float streamer_fade_out_factor;

/** Single var cache of CORE_FADE_OVERLAP_FACTOR. */
extern float streamer_fade_overlap_factor;

/** Call before anything else.  Sets up internal state of the subsystem. */
void streamer_init();

/** Called frequently to action streaming.
 * \param L Lua state for calling object activation callbacks.
 * \param new_pos The player's position.
 */
void streamer_centre (lua_State *L, const Vector3 &new_pos, bool everything);

/** Called by objects when they change position or rendering distance.
 * \param index The index of the object within the streamer.
 * \param pos The new position of the object.
 * \param d The new rendering distance of the object.
 */
void streamer_update_sphere (size_t index, const Vector3 &pos, float d);
        
/** Add a new object to the 'map', i.e. set of objects considered for streaming in. */
void streamer_list (const GritObjectPtr &o);

/** Remove an object from the map. */
void streamer_unlist (const GritObjectPtr &o);

/** Add to the list of activated objects.  These are not streamed in when they
 * come into range, and are streamed out when they go out of range. */
void streamer_list_as_activated (const GritObjectPtr &o);

/** Remove an object from the list of activated objects. */
void streamer_unlist_as_activated (const GritObjectPtr &o);

/** Fetch a list of currently activated objects (via a pair of iterators). */
void streamer_object_activated (GObjPtrs::iterator &begin, GObjPtrs::iterator &end);

/** Return the number of currently activated objects. */
int streamer_object_activated_count (void);


/** Called whenever someone calls streamer_centre. */
struct StreamerCallback {
        virtual void update(const Vector3 &new_pos) = 0;
};

/** Register a StreamerCallback callback. */
void streamer_callback_register (StreamerCallback *rc);

/** Unregister a StreamerCallback callback. */
void streamer_callback_unregister (StreamerCallback *rc);

#endif
