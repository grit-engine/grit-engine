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

class GfxLeaf;
class GfxNode;
typedef SharedPtr<GfxNode> GfxNodePtr;

#ifndef GfxLeaf_h
#define GfxLeaf_h

#include <OgreSceneNode.h>

#include "../math_util.h"

// this should rarely need to be used by users of this API
class GfxLeaf {
    protected:
    static const std::string className;
    Vector3 pos; Quaternion quat; Vector3 scl;
    GfxNodePtr par;
    public: // HACK
    Ogre::SceneNode *node;
    protected:
    bool dead;

    GfxLeaf (const GfxNodePtr &par_);
    virtual ~GfxLeaf ();

    void notifyParentDead (void);
    void ensureNotChildOf (GfxNode *node) const;

    public:

    const GfxNodePtr &getParent (void) const;
    virtual void setParent (const GfxNodePtr &par_);

    Vector3 transformPositionParent (const Vector3 &v);
    Quaternion transformOrientationParent (const Quaternion &v);
    Vector3 transformScaleParent (const Vector3 &v);
    Vector3 transformPosition (const Vector3 &v);
    Quaternion transformOrientation (const Quaternion &v);
    Vector3 transformScale (const Vector3 &v);

    const Vector3 &getLocalPosition (void);
    void setLocalPosition (const Vector3 &p);
    Vector3 getWorldPosition (void);

    const Quaternion &getLocalOrientation (void);
    void setLocalOrientation (const Quaternion &q);
    Quaternion getWorldOrientation (void);

    const Vector3 &getLocalScale (void);
    void setLocalScale (const Vector3 &s);
    Vector3 getWorldScale (void);

    virtual void destroy (void);
    virtual bool destroyed (void) const { return dead; }

    friend class GfxNode; // otherwise it cannot access our protected stuff
};

#endif
