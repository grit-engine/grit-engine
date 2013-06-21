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

#include "Streamer.h"
#include "main.h"
#include "option.h"
#include "GritClass.h"

float streamer_visibility;
float streamer_prepare_distance_factor;
float streamer_fade_out_factor;
float streamer_fade_overlap_factor;

static CoreBoolOption option_keys_bool[] = {
    CORE_AUTOUPDATE
};

static CoreFloatOption option_keys_float[] = {
    CORE_VISIBILITY,
    CORE_PREPARE_DISTANCE_FACTOR,
    CORE_FADE_OUT_FACTOR,
    CORE_FADE_OVERLAP_FACTOR
};

static CoreIntOption option_keys_int[] = {
    CORE_STEP_SIZE
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
    }
    return "UNKNOWN_BOOL_OPTION";
}       
std::string core_option_to_string (CoreIntOption o)
{   
    switch (o) {
        case CORE_STEP_SIZE: return "STEP_SIZE";
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

    else if (s=="STEP_SIZE") { t = 1 ; o1 = CORE_STEP_SIZE; }

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


static void init_options (void)
{

    for (unsigned i=0 ; i < sizeof(option_keys_bool) / sizeof(*option_keys_bool) ; ++i) {
        valid_option(option_keys_bool[i], truefalse);

    }

    valid_option(CORE_STEP_SIZE, new ValidOptionRange<int>(0,20000));

    valid_option(CORE_VISIBILITY, new ValidOptionRange<float>(0, 10));
    valid_option(CORE_PREPARE_DISTANCE_FACTOR, new ValidOptionRange<float>(1, 3));
    valid_option(CORE_FADE_OUT_FACTOR, new ValidOptionRange<float>(0, 1));
    valid_option(CORE_FADE_OVERLAP_FACTOR, new ValidOptionRange<float>(0, 1));


    core_option(CORE_AUTOUPDATE, false);

    core_option(CORE_STEP_SIZE, 20000);

    core_option(CORE_VISIBILITY, 1.0f);
    core_option(CORE_PREPARE_DISTANCE_FACTOR, 1.3f);
    core_option(CORE_FADE_OUT_FACTOR, 0.7f);
    core_option(CORE_FADE_OVERLAP_FACTOR, 0.7f);

    options_update(true);

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


typedef CacheFriendlyRangeSpace<GritObjectPtr> Space;
static Space rs;

static GObjPtrs activated;
static GObjPtrs loaded;
static GObjPtrs fresh; // just been added - skip the queue for activation

typedef std::vector<StreamerCallback*> StreamerCallbacks;
StreamerCallbacks streamer_callbacks;

static void remove_if_exists (GObjPtrs &list, const GritObjectPtr &o)
{
    GObjPtrs::iterator iter = find(list.begin(),list.end(),o);
    if (iter!=list.end()) {
        size_t offset = iter - list.begin();
        list[offset] = list[list.size()-1];
        list.pop_back();
    }
}

void streamer_init (void)
{
    init_options();
}


void streamer_centre (lua_State *L, const Vector3 &new_pos)
{
        Space::Cargo fnd = fresh;
        fresh.clear();

        const float visibility = streamer_visibility;

        const float pF = streamer_prepare_distance_factor;
        const float tpF = pF * visibility; // prepare and visibility factors
        const float vis2 = visibility * visibility;

        typedef GObjPtrs::iterator I;

        // iterate through, deactivating things


        ////////////////////////////////////////////////////////////////////////
        // DEACTIVATE DISTANT GRIT OBJECTS /////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        // use victims because deactivate() changes the 'activated' list
        // and so does notifyRange2 if the callback raises an error
        GObjPtrs victims = activated;
        for (I i=victims.begin(), i_=victims.end() ; i!=i_ ; ++i) {
                const GritObjectPtr &o = *i;
                 //note we use vis2 not visibility
                float range2 = o->range2(new_pos) / vis2;
                // sometimes deactivation of an object can cause the deletion of other objects
                // if those objects are also in the victims list, this can become a problem
                // so just skip them
                if (o->getClass()==NULL) continue;
                o->notifyRange2(L,o,range2);
                const GritObjectPtr &the_far = o->getFarObj();
                if (!the_far.isNull()) {
                        // update the far (perhaps for a second time this frame)
                        // to make sure it has picked up the fade imposed by o
                        float range2 = the_far->range2(new_pos) / vis2;
                        the_far->notifyRange2(L,the_far,range2);
                }
                if (range2 > 1) {
                        // now out of range
                        const GritObjectPtr &o = *i;
                        const GritObjectPtr &the_far = o->getFarObj();
                        bool killme = o->deactivate(L,o);
                        if (!the_far.isNull()) {
                                // we're deactivating and we have a far,
                                // so make sure it gets considered this frame
                                fnd.push_back(the_far);
                        }
                        if (killme) {
                                object_del(L,o);
                        }
                }
        }

        ////////////////////////////////////////////////////////////////////////
        // UNLOAD RESOURCES FOR VERY DISTANT GRIT OBJECTS //////////////////////
        ////////////////////////////////////////////////////////////////////////
        for (size_t i=0, i_=loaded.size(); i<i_ ;) {
                // Iteration should be fast for removal of significant number of
                // elements.  Don't do this if it ever stops being a vector.
                const GritObjectPtr &o = loaded[i];
                if (!o->withinRange(new_pos,tpF)) {
                        // unregister demand...
                        // we deactivated first so this should
                        // unload any resources we were using
                        o->tryUnloadResources();
                        loaded[i] = loaded[i_-1];
                        loaded.pop_back();
                        --i_;
                } else {
                        ++i;
                }
        }


        GObjPtrs must_kill;

        ////////////////////////////////////////////////////////////////////////
        // LOAD RESOURCES FOR APPROACHING GRIT OBJECTS /////////////////////////
        // AND... ACTIVATE ARRIVING GRIT OBJECTS ///////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        // note: since fnd is prepopulated by new objects and the lods of deactivated objects
        // it may have duplicates after the rangespace has gone through
        rs.getPresent(new_pos.x,new_pos.y,new_pos.z,core_option(CORE_STEP_SIZE),tpF,fnd);
        for (Space::Cargo::iterator i=fnd.begin(),i_=fnd.end() ; i!=i_ ; ++i) {
                const GritObjectPtr &o = *i;

                // this can happen if there is a duplicate in the list and it gets destroyed
                // the first time due to an error in the activation function
                // but is eventually processed a second time after being destroyed
                if (o->getClass()==NULL) continue;
                if (o->isActivated()) continue;

                float range2 = o->range2(new_pos) / vis2;
                // not in range yet
                if (range2 > 1) continue;

                //CVERB << "Preparing for activation: " << o->name << std::endl;

                // consider background loading
                if (o->backgroundLoadingCausedError()) {
                        must_kill.push_back(o);
                        continue;
                }
                if (o->requestLoad(new_pos)) {
                        // this means we weren't already in the queue.
                        // we may still not be in the queue, if all resources are loaded

                        // note 'loaded' includes things which have started
                        // but not finished loading...
                        loaded.push_back(o);
                }
                if (o->isInBackgroundQueue()) {
                        // if we're in the queue we probably have to wait longer
                        // before all the resources are loaded
                        continue;
                }
                if (o->backgroundLoadingCausedError()) {
                        must_kill.push_back(o);
                        continue;
                }

                // if we get this far, we should be displayed but there might be
                // a near object in the way
                GritObjectPtr the_near = o->getNearObj();
                while (!the_near.isNull()) {
                        if (the_near->withinRange(new_pos,visibility * streamer_fade_overlap_factor)) {
                                if (the_near->isActivated()) {
                                        // why deactivate?
                                        // we already ensured it is not activated above...
                                        o->deactivate(L,o);
                                        // don't activate, near gobj is
                                        // in the way
                                        goto skip;
                                }
                        }
                        the_near = the_near->getNearObj();
                }
      

                // ok there wasn't so activate
                o->activate(L,o);

                // activation can result in a lua error which triggers the destruction of the
                // object 'o' so we test for that here before doing more stuff
                if (o->getClass()==NULL) continue;

                o->notifyRange2(L,o,range2);

                skip:;
        }

        for (GObjPtrs::iterator i=must_kill.begin(),i_=must_kill.end() ; i!=i_ ; ++i) {
                CERR << "Object: \"" << (*i)->name << "\" raised an error while background loading resources, so destroying it." << std::endl;
                object_del(L, *i);
        }

        bgl->handleBastards();
        bgl->checkRAMHost();
        bgl->checkRAMGPU();

        for (StreamerCallbacks::iterator i=streamer_callbacks.begin(),i_=streamer_callbacks.end() ; i!=i_ ; ++i) {
                (*i)->update(new_pos);
        }

}

void streamer_update_sphere (size_t index, const Vector3 &pos, float d)
{
        rs.updateSphere(index,pos.x,pos.y,pos.z,d);
}

void streamer_object_activated (GObjPtrs::iterator &begin, GObjPtrs::iterator &end)
{
    begin = activated.begin();
    end = activated.end();
}

int streamer_object_activated_count (void)
{
    return activated.size();
}

void streamer_list(const GritObjectPtr &o)
{
    rs.add(o);
    fresh.push_back(o);
}

void streamer_unlist(const GritObjectPtr &o)
{
    rs.remove(o);
    remove_if_exists(fresh, o);
}

void streamer_list_as_activated (const GritObjectPtr &o)
{
    GObjPtrs::iterator begin = activated.begin(), end = activated.end();
    GObjPtrs::iterator iter  = find(begin,end,o);
    if (iter!=end) return;
    activated.push_back(o);
}

void streamer_unlist_as_activated (const GritObjectPtr &o)
{
    GObjPtrs::iterator begin = activated.begin(), end = activated.end();
    GObjPtrs::iterator iter  = find(begin,end,o);
    if (iter==end) return;
    size_t index = iter - begin;
    activated[index] = activated[activated.size()-1];
    activated.pop_back();
}


void streamer_callback_register (StreamerCallback *rc)
{
    StreamerCallbacks::iterator begin = streamer_callbacks.begin(), end = streamer_callbacks.end();
    StreamerCallbacks::iterator iter  = find(begin,end,rc);
    if (iter!=end) return;
    streamer_callbacks.push_back(rc);
}

void streamer_callback_unregister (StreamerCallback *rc)
{
    StreamerCallbacks::iterator begin = streamer_callbacks.begin(), end = streamer_callbacks.end();
    StreamerCallbacks::iterator iter  = find(begin,end,rc);
    if (iter==end) return;
    size_t index = iter - begin;
    streamer_callbacks[index] = streamer_callbacks[streamer_callbacks.size()-1];
    streamer_callbacks.pop_back();
}

// vim: shiftwidth=4:tabstop=4:expandtab
