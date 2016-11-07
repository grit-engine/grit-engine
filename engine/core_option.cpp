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

#include "streamer.h"
#include "core_option.h"

static CoreBoolOption option_keys_bool[] = {
    CORE_AUTOUPDATE,
    CORE_FOREGROUND_WARNINGS
};

static CoreFloatOption option_keys_float[] = {
    CORE_VISIBILITY,
    CORE_PREPARE_DISTANCE_FACTOR,
    CORE_FADE_OUT_FACTOR,
    CORE_FADE_OVERLAP_FACTOR
};

static CoreIntOption option_keys_int[] = {
    CORE_STEP_SIZE,
    CORE_RAM
};


static std::map<CoreBoolOption,bool> options_bool;
static std::map<CoreIntOption,int> options_int;
static std::map<CoreFloatOption,float> options_float;
static std::map<CoreBoolOption,bool> new_options_bool;
static std::map<CoreIntOption,int> new_options_int;
static std::map<CoreFloatOption,float> new_options_float;

static std::map<CoreBoolOption,ValidOption<bool>*> valid_option_bool;
static std::map<CoreIntOption,ValidOption<int>*> valid_option_int;
static std::map<CoreFloatOption,ValidOption<float>*> valid_option_float;

static void valid_option (CoreBoolOption o, ValidOption<bool> *v) { valid_option_bool[o] = v; }
static void valid_option (CoreIntOption o, ValidOption<int> *v) { valid_option_int[o] = v; }
static void valid_option (CoreFloatOption o, ValidOption<float> *v) { valid_option_float[o] = v; }
    
static bool truefalse_[] = { false, true };
static ValidOptionList<bool,bool[2]> *truefalse = new ValidOptionList<bool,bool[2]>(truefalse_);

        
std::string core_option_to_string (CoreBoolOption o)
{       
    switch (o) {
        case CORE_AUTOUPDATE: return "AUTOUPDATE";
        case CORE_FOREGROUND_WARNINGS: return "FOREGROUND_WARNINGS";
    }
    return "UNKNOWN_BOOL_OPTION";
}       
std::string core_option_to_string (CoreIntOption o)
{   
    switch (o) {
        case CORE_STEP_SIZE: return "STEP_SIZE";
        case CORE_RAM: return "RAM";
    }   
    return "UNKNOWN_INT_OPTION";
}
std::string core_option_to_string (CoreFloatOption o)
{       
    switch (o) {
        case CORE_VISIBILITY: return "VISIBILITY";
        case CORE_PREPARE_DISTANCE_FACTOR: return "PREPARE_DISTANCE_FACTOR";
        case CORE_FADE_OUT_FACTOR: return "FADE_OUT_FACTOR";
        case CORE_FADE_OVERLAP_FACTOR: return "FADE_OVERLAP_FACTOR";
    }   
    return "UNKNOWN_FLOAT_OPTION";
}

void core_option_from_string (const std::string &s,
                               int &t,
                               CoreBoolOption &o0,
                               CoreIntOption &o1,
                               CoreFloatOption &o2)
{
    if (s=="AUTOUPDATE") { t = 0; o0 = CORE_AUTOUPDATE; }
    else if (s=="FOREGROUND_WARNINGS") { t = 0; o0 = CORE_FOREGROUND_WARNINGS; }

    else if (s=="STEP_SIZE") { t = 1 ; o1 = CORE_STEP_SIZE; }
    else if (s=="RAM") { t = 1 ; o1 = CORE_RAM; }

    else if (s=="VISIBILITY") { t = 2 ; o2 = CORE_VISIBILITY; }
    else if (s=="PREPARE_DISTANCE_FACTOR") { t = 2 ; o2 = CORE_PREPARE_DISTANCE_FACTOR; }
    else if (s=="FADE_OUT_FACTOR") { t = 2 ; o2 = CORE_FADE_OUT_FACTOR; }
    else if (s=="FADE_OVERLAP_FACTOR") { t = 2 ; o2 = CORE_FADE_OVERLAP_FACTOR; }

    else t = -1;
}

static void options_update (bool flush)
{
    (void) flush;

    for (unsigned i=0 ; i<sizeof(option_keys_bool)/sizeof(*option_keys_bool) ; ++i) {
        CoreBoolOption o = option_keys_bool[i];
        bool v_old = options_bool[o];
        bool v_new = new_options_bool[o];
        if (v_old == v_new) continue;
        switch (o) {
            case CORE_AUTOUPDATE: break;
            case CORE_FOREGROUND_WARNINGS:
            disk_resource_foreground_warnings = v_new;
            break;
        }
    }
    for (unsigned i=0 ; i<sizeof(option_keys_int)/sizeof(*option_keys_int) ; ++i) {
        CoreIntOption o = option_keys_int[i];
        int v_old = options_int[o];
        int v_new = new_options_int[o];
        if (v_old == v_new) continue;
        switch (o) {
            case CORE_STEP_SIZE:
            case CORE_RAM:
            break;
        }
    }
    for (unsigned i=0 ; i<sizeof(option_keys_float)/sizeof(*option_keys_float) ; ++i) {
        CoreFloatOption o = option_keys_float[i];
        float v_old = options_float[o];
        float v_new = new_options_float[o];
        if (v_old == v_new) continue;
        switch (o) {
            case CORE_VISIBILITY:
            streamer_visibility = v_new;
            break;
            case CORE_PREPARE_DISTANCE_FACTOR:
            streamer_prepare_distance_factor = v_new;
            break;
            case CORE_FADE_OUT_FACTOR:
            streamer_fade_out_factor = v_new;
            break;
            case CORE_FADE_OVERLAP_FACTOR:
            streamer_fade_overlap_factor = v_new;
            break;
        }
    }

    options_bool = new_options_bool;
    options_int = new_options_int;
    options_float = new_options_float;
}


void core_option_reset (void)
{
    core_option(CORE_FOREGROUND_WARNINGS, true);

    core_option(CORE_STEP_SIZE, 20000);
    core_option(CORE_RAM, 1024); // 1GB

    core_option(CORE_VISIBILITY, 1.0f);
    core_option(CORE_PREPARE_DISTANCE_FACTOR, 1.3f);
    core_option(CORE_FADE_OUT_FACTOR, 0.7f);
    core_option(CORE_FADE_OVERLAP_FACTOR, 0.7f);
}


void core_option_init (void)
{

    for (unsigned i=0 ; i < sizeof(option_keys_bool) / sizeof(*option_keys_bool) ; ++i) {
        valid_option(option_keys_bool[i], truefalse);

    }

    valid_option(CORE_STEP_SIZE, new ValidOptionRange<int>(0,20000));
    valid_option(CORE_RAM, new ValidOptionRange<int>(0,1024*1024)); // 1TB

    valid_option(CORE_VISIBILITY, new ValidOptionRange<float>(0, 10));
    valid_option(CORE_PREPARE_DISTANCE_FACTOR, new ValidOptionRange<float>(1, 3));
    valid_option(CORE_FADE_OUT_FACTOR, new ValidOptionRange<float>(0, 1));
    valid_option(CORE_FADE_OVERLAP_FACTOR, new ValidOptionRange<float>(0, 1));


    core_option(CORE_AUTOUPDATE, false);
    core_option_reset();
    options_update(true);
    // This will call options_update(false) but it will be a no-op this time.
    core_option(CORE_AUTOUPDATE, true);

}


void core_option (CoreBoolOption o, bool v)
{
    valid_option_bool[o]->maybeThrow("Core", v);
    new_options_bool[o] = v;
    if (new_options_bool[CORE_AUTOUPDATE]) options_update(false);
}
bool core_option (CoreBoolOption o)
{
    return options_bool[o];
}

void core_option (CoreIntOption o, int v)
{
    valid_option_int[o]->maybeThrow("Core", v);
    new_options_int[o] = v;
    if (new_options_bool[CORE_AUTOUPDATE]) options_update(false);
}
int core_option (CoreIntOption o)
{
    return options_int[o];
}

void core_option (CoreFloatOption o, float v)
{
    valid_option_float[o]->maybeThrow("Core", v);
    new_options_float[o] = v;
    if (new_options_bool[CORE_AUTOUPDATE]) options_update(false);
}
float core_option (CoreFloatOption o)
{
    return options_float[o];
}

