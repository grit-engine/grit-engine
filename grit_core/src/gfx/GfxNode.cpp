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

#include "GfxNode.h"

#include "gfx_internal.h"

const std::string GfxNode::className = "GfxNode";

GfxNode::GfxNode (const GfxNodePtr &par_)
  : GfxLeaf(par_)
{
}

GfxNode::~GfxNode (void)
{
    if (!dead) destroy();
}

void GfxNode::destroy (void)
{
    if (dead) THROW_DEAD(className);
    for (unsigned i=0 ; i<children.size() ; ++i) {
        children[i]->notifyParentDead();
    }
    children.clear();
    GfxLeaf::destroy();
}

void GfxNode::notifyLostChild (GfxLeaf *child)
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

void GfxNode::notifyGainedChild (GfxLeaf *child)
{
    children.push_back(child);
}

void GfxNode::setParent (const GfxNodePtr &par_)
{
    if (dead) THROW_DEAD(className);
    if (!par_.isNull()) par_->ensureNotChildOf(this); // check that we are not a parent of par_
    GfxLeaf::setParent(par_);
}

unsigned GfxNode::getBatchesWithChildren (void) const
{
    unsigned total = 0;
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxNode *child = dynamic_cast<GfxNode*>(children[i]);
        if (child) {
            total += child->getBatchesWithChildren();
        }
    }
    return total;
}

unsigned GfxNode::getVertexesWithChildren (void) const
{
    unsigned total = 0;
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxNode *child = dynamic_cast<GfxNode*>(children[i]);
        if (child) {
            total += child->getVertexesWithChildren();
        }
    }
    return total;
}

unsigned GfxNode::getTrianglesWithChildren (void) const
{
    unsigned total = 0;
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxNode *child = dynamic_cast<GfxNode*>(children[i]);
        if (child) {
            total += child->getTrianglesWithChildren();
        }
    }
    return total;
}
