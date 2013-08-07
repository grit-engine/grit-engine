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

#include "../SharedPtr.h"

class GfxNode;
class GfxFertileNode;
typedef SharedPtr<GfxFertileNode> GfxNodePtr;

#ifndef GfxLeaf_h
#define GfxLeaf_h

#include <OgreSceneNode.h>

#include "../math_util.h"
#include "../vect_util.h"

#include "gfx_internal.h"

// TERMINOLOGY:
// A GfxNode is something that can be a leaf.  Everything in the tree is GfxNode.
// A GfxFertileNode is something that can be a parent.  All GfxFertileNode are also GfxNode.
// Objects should typically extend GfxFertileNode so they can have children attached.
// Objects that do not exist in the Ogre scene graph are currenty not GfxFertileNode since they have no internal scene node.

/* OH FUCK FUCK FUCK
 * The parent can change its localTransform, thereby needing to make its children dirty.  Oh, the humanity.
 */

// this should rarely need to be used by users of this API
class GfxNode : public fast_erase_index {
    protected:
    static const std::string className;
    Vector3 localPos, localScale;
    Quaternion localOrientation;
    Transform worldTransform;
    //bool dirtyWorldTransform;
    GfxNodePtr par;
    std::string parentBoneName;
    int parentBoneId;
    Ogre::SceneNode *node;
    bool dead;

    GfxNode (const GfxNodePtr &par_);
    virtual ~GfxNode ();

    void notifyParentDead (void);
    void ensureNotChildOf (GfxFertileNode *node) const;

    void doUpdateWorldTransform (void);
    void updateParentBoneId (void);

    void ensureAlive (void) const { if (dead) THROW_DEAD(className); }

    public:

    void updateWorldTransform (void) {
        /*if (dirtyWorldTransform)*/ doUpdateWorldTransform();
    }

    const GfxNodePtr &getParent (void) const { ensureAlive(); return par; }
    virtual void setParent (const GfxNodePtr &par_);

    const std::string &getParentBoneName (void) const
    {
        ensureAlive();
        return parentBoneName;
    }
    virtual void setParentBoneName (const std::string &s)
    {
        ensureAlive();
        parentBoneName = s;
        updateParentBoneId();
    }

    const Vector3 &getLocalPosition (void) const
    {
        ensureAlive();
        return localPos;
    }
    const Quaternion &getLocalOrientation (void) const
    {
        ensureAlive();
        return localOrientation;
    }
    const Vector3 &getLocalScale (void) const
    {
        ensureAlive();
        return localScale;
    }
    void setLocalPosition (const Vector3 &v)
    {
        ensureAlive();
        localPos = v;
        //dirtyWorldTransform = true;
    }
    void setLocalOrientation (const Quaternion &v)
    {
        ensureAlive();
        localOrientation = v;
        //dirtyWorldTransform = true;
    }
    void setLocalScale (const Vector3 &v)
    {
        ensureAlive();
        localScale = v;
        //dirtyWorldTransform = true;
    }
    Transform getWorldTransform (void)
    {
        ensureAlive();
        updateWorldTransform();
        return worldTransform;
    }

    virtual void destroy (void);
    virtual bool destroyed (void) const { return dead; }

    friend class GfxFertileNode; // otherwise it cannot access our protected stuff
};

#endif
