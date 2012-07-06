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

#include "../main.h"

#include "gfx_internal.h"
#include "GfxInstances.h"

const std::string GfxInstances::className = "GfxInstances";

class GfxInstances::Instance {
    int actual_index;
};


// handle emissive?
// make GfxInstances a movable object?
// make it issue two render ops?

// perhaps GfxBody should similarly be a MovableObject
// not use Entity internally
// reimplement Entity but handle emissive / fade

GfxInstances::GfxInstances (const std::string &mesh, const GfxBodyPtr &par_)
    : GfxNode(par_), meshName(mesh)
{
    registerMe();
}

GfxInstances::~GfxInstances (void)
{
    if (!dead) destroy();
    unregisterMe();
}

void GfxInstances::destroy (void)
{
    if (dead) THROW_DEAD(className);
    if (mobj) ogre_sm->destroyMovableObject(mobj);
    mobj = NULL;
    GfxNode::destroy();
}

unsigned int GfxInstances::add (void)
{
    unsigned int index = instances.size();
    instances.push_back(Instance());
    return index;
}

void GfxInstances::update (unsigned int index, const Vector3 &pos, const Quaternion &q, float fade)
{
    //instances[index].update(
}

void GfxInstances::remove (unsigned int index)
{
}


void GfxInstances::registerMe (void)
{
    streamer->registerUpdateHook(this);
}

void GfxInstances::unregisterMe (void)
{
    streamer->unregisterUpdateHook(this);
}

void GfxInstances::update (const Vector3 &new_pos)
{
}


