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

#include <algorithm>

#include "../main.h"

#include "gfx_internal.h"
#include "gfx_ranged_instances.h"

const std::string GfxRangedInstances::className = "GfxRangedInstances";

GfxRangedInstancesPtr GfxRangedInstances::make (const std::string &mesh_name, const GfxNodePtr &par_)
{       
    auto gdr = disk_resource_use<GfxMeshDiskResource>(mesh_name);
    
    if (gdr == nullptr) GRIT_EXCEPT("Resource is not a mesh: \"" + mesh_name + "\"");

    return GfxRangedInstancesPtr(new GfxRangedInstances(gdr, par_));
}

GfxRangedInstances::GfxRangedInstances (const DiskResourcePtr<GfxMeshDiskResource> &gdr,
                                        const GfxNodePtr &par_)
  : GfxInstances(gdr, par_),
    mItemRenderingDistance(40),
    mVisibility(1),
    mStepSize(100000)
{   
    registerMe();
}   

GfxRangedInstances::~GfxRangedInstances (void)
{
    unregisterMe();
}

void GfxRangedInstances::registerMe (void)
{
    streamer_callback_register(this);
}

void GfxRangedInstances::unregisterMe (void)
{
    streamer_callback_unregister(this);
}

void GfxRangedInstances::update (const Vector3 &new_pos)
{
    const float vis2 = mVisibility * mVisibility;
    typedef Cargo::iterator I;

    // iterate through all activated guys to see who is too far to stay activated
    Cargo victims = activated;
    for (I i=victims.begin(), i_=victims.end() ; i!=i_ ; ++i) {
        Item *o = *i;
         //note we use vis2 not visibility
        float range2 = o->range2(new_pos) / vis2;

        if (range2 > 1) {
            // now out of range
            del(o->ticket);
            o->activated = false;
            Item *filler = activated[activated.size()-1];
            activated[o->activatedIndex] = filler;
            filler->activatedIndex = o->activatedIndex;
            activated.pop_back();
        } else {
            // still in range, update visibility
            float fade = o->calcFade(range2);
            if (fade!=o->lastFade) {
                update(o->ticket, o->pos, o->quat, fade);
                o->lastFade = fade;
            }
        }
    }

    Cargo cargo;
    mSpace.getPresent(new_pos.x, new_pos.y, new_pos.z, mStepSize, mVisibility, cargo);
    // iterate through the cargo to see who needs to become activated
    for (Cargo::iterator i=cargo.begin(),i_=cargo.end() ; i!=i_ ; ++i) {
        Item *o = *i;

        if (o->activated) continue;

        float range2 = o->range2(new_pos) / vis2;

        // not in range yet
        if (range2 > 1) continue;

        float fade = o->calcFade(range2);
        o->lastFade = fade;

        //activate o
        o->ticket = add(o->pos, o->quat, fade);
        o->activatedIndex = activated.size();
        activated.push_back(o);
        o->activated = true;
    }
}

void GfxRangedInstances::push_back (const SimpleTransform &t)
{
    // it is very important that the number of samples does not increase over time, or
    // the vector will resize and these pointers will become invalid
    items.push_back(Item());
    Item &item = items[items.size()-1];
    item.parent = this;
    item.activated = false;
    item.quat = t.quat;
    item.lastFade = 0;
    mSpace.add(&item);
    item.updateSphere(t.pos, mItemRenderingDistance);
}

float GfxRangedInstances::Item::calcFade (float range2)
{
    const float out = streamer_fade_out_factor;
    float range = ::sqrtf(range2);

    float fade = 1.0;

    if (range > out) {
            fade = (1-range) / (1-out);
    }

    return fade;
}

