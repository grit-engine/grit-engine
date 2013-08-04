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

#include "GfxFertileNode.h"

#include "gfx_internal.h"

const std::string GfxFertileNode::className = "GfxFertileNode";

GfxFertileNode::GfxFertileNode (const GfxNodePtr &par_)
  : GfxNode(par_)
{
}

GfxFertileNode::~GfxFertileNode (void)
{
    if (!dead) destroy();
}

void GfxFertileNode::destroy (void)
{
    if (dead) THROW_DEAD(className);
    for (unsigned i=0 ; i<children.size() ; ++i) {
        children[i]->notifyParentDead();
    }
    children.clear();
    GfxNode::destroy();
}

void GfxFertileNode::notifyLostChild (GfxNode *child)
{
    unsigned ci = 0;
    bool found_child = false;
    for (unsigned i=0 ; i<children.size() ; ++i) {
        if (children[i]==child) {
            ci = i;
            found_child = true;
            break;
        }
    }
    APP_ASSERT(found_child);
    children[ci] = children[children.size()-1];
    children.pop_back();
}

void GfxFertileNode::notifyGainedChild (GfxNode *child)
{
    children.push_back(child);
}

void GfxFertileNode::setParent (const GfxNodePtr &par_)
{
    if (dead) THROW_DEAD(className);
    if (!par_.isNull()) par_->ensureNotChildOf(this); // check that we are not a parent of par_
    GfxNode::setParent(par_);
}

unsigned GfxFertileNode::getBatchesWithChildren (void) const
{
    unsigned total = 0;
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxFertileNode *child = dynamic_cast<GfxFertileNode*>(children[i]);
        if (child) {
            total += child->getBatchesWithChildren();
        }
    }
    return total;
}

unsigned GfxFertileNode::getVertexesWithChildren (void) const
{
    unsigned total = 0;
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxFertileNode *child = dynamic_cast<GfxFertileNode*>(children[i]);
        if (child) {
            total += child->getVertexesWithChildren();
        }
    }
    return total;
}

unsigned GfxFertileNode::getTrianglesWithChildren (void) const
{
    unsigned total = 0;
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxFertileNode *child = dynamic_cast<GfxFertileNode*>(children[i]);
        if (child) {
            total += child->getTrianglesWithChildren();
        }
    }
    return total;
}
