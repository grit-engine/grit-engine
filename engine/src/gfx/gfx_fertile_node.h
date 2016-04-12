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

#include "../shared_ptr.h"

class GfxFertileNode;
typedef SharedPtr<GfxFertileNode> GfxNodePtr;

#ifndef GfxNode_h
#define GfxNode_h

#include "gfx_node.h"

/** A node can be a leaf, or a parent. */
class GfxFertileNode : public GfxNode {
    protected:
    static const std::string className;
    std::vector<GfxNode*> children; // caution!

    GfxFertileNode (const GfxNodePtr &par_);
    ~GfxFertileNode ();

    public:
    static GfxNodePtr make (const GfxNodePtr &par_=GfxNodePtr(NULL))
    { return GfxNodePtr(new GfxFertileNode(par_)); }

    void notifyLostChild (GfxNode *child);
    void notifyGainedChild (GfxNode *child);
    virtual void setParent (const GfxNodePtr &par_);

    virtual unsigned getBatchesWithChildren (void) const;

    virtual unsigned getTrianglesWithChildren (void) const;

    virtual unsigned getVertexesWithChildren (void) const;

    virtual void destroy (void);

    virtual bool hasGraphics (void) const { return false; }

    friend class SharedPtr<GfxFertileNode>;
};

#endif
