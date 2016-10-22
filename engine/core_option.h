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

#ifndef CORE_OPTION_h
#define CORE_OPTION_h

#include <map>

#include "option.h"

enum CoreBoolOption {

    /** Whether or not setting the next option will cause fresh option values to be
     * processed (default true).  To update more than one option concurrently, set
     * autoupdate to false, set the options, then set it to true.  */
    CORE_AUTOUPDATE,

    /** Whether to issue warnings if disk resources are loaded in the rendering thread.  */
    CORE_FOREGROUND_WARNINGS
};

enum CoreFloatOption {
    /** A factor that is globally applied to object rendering distances. */
    CORE_VISIBILITY,

    /** The proportion of grit object rendering distance at which background
     * loading of disk resources is triggered. */
    CORE_PREPARE_DISTANCE_FACTOR,

    /** The proportion of rendering distance at which fading out begins. */
    CORE_FADE_OUT_FACTOR,

    /** The proportion of rendering distance at which fading to the next lod level begins. */
    CORE_FADE_OVERLAP_FACTOR
};

enum CoreIntOption {
    /** The number of objects per frame considered for streaming in. */
    CORE_STEP_SIZE,
    /** The number of megabytes of host RAM to use for cached disk resources. */
    CORE_RAM
};

/** Returns the enum value of the option described by s.  Only one of o0, o1,
 * o2 is modified, and t is used to tell the caller which one. */
void core_option_from_string (const std::string &s,
                             int &t,
                             CoreBoolOption &o0,
                             CoreIntOption &o1,
                             CoreFloatOption &o2);

/** Set the option to a particular value. */
void core_option (CoreBoolOption o, bool v);
/** Return the current value of the option. */
bool core_option (CoreBoolOption o);
/** Set the option to a particular value. */
void core_option (CoreIntOption o, int v);
/** Return the current value of the option. */
int core_option (CoreIntOption o);
/** Set the option to a particular value. */
void core_option (CoreFloatOption o, float v);
/** Return the current value of the option. */
float core_option (CoreFloatOption o);

/** Initialise these options. */
void core_option_init (void);

#endif
