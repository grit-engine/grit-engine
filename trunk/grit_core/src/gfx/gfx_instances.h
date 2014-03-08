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

#include "../shared_ptr.h"

class GfxInstances;
typedef SharedPtr<GfxInstances> GfxInstancesPtr;


#ifndef GFX_INSTANCES_H
#define GFX_INSTANCES_H

#include <OgreMovableObject.h>

#include "../dense_index_map.h"

#include "gfx_disk_resource.h"
#include "gfx_node.h"
#include "gfx_fertile_node.h"

class GfxInstances : public GfxNode, public Ogre::MovableObject {

    protected:

    static const std::string className;

    DenseIndexMap indexes;

    class Section;

    unsigned numSections;
    Section **sections;

    Ogre::MeshPtr mesh;
    Ogre::VertexData *sharedVertexData;
    Ogre::HardwareVertexBufferSharedPtr instBuf;
    std::vector<float> instBufRaw;
    bool dirty;
    bool enabled;

    GfxInstances (GfxMeshDiskResource *mesh, const GfxNodePtr &par_);
    ~GfxInstances (void);

    public:
    static GfxInstancesPtr make (const std::string &mesh, const GfxNodePtr &par_=GfxNodePtr(NULL));

    unsigned int add (const Vector3 &pos, const Quaternion &q, float fade);
    // in future, perhaps 3d scale, skew, or general 3x3 matrix?
    void update (unsigned int inst, const Vector3 &pos, const Quaternion &q, float fade);
    void del (unsigned int inst);

    // don't call this reserve because a subclass wants to call its member function reserve
    void reserveSpace (unsigned new_capacity);

    void destroy (void);

    void setEnabled (bool v) { enabled = v; }
    bool isEnabled (void) { return enabled; }

    unsigned getCapacity (void) { return indexes.capacity(); }
    unsigned getInstances (void) { return indexes.size(); }
    unsigned getTrianglesPerInstance (void);
    unsigned getBatches (void);

    protected:

    void updateSections (void);
    void updateProperties (void);
    void reinitialise (void);

    void copyToGPU ();
    void copyToGPU (unsigned from, unsigned to, bool discard);


    // Stuff for Ogre::MovableObject

    Ogre::AxisAlignedBox mBoundingBox;
    Ogre::Real mBoundingRadius;

    public:

    virtual const Ogre::String& getMovableType (void) const
    {
        return className;
    }

    virtual const Ogre::AxisAlignedBox &getBoundingBox (void) const
    { return mBoundingBox; }

    virtual Ogre::Real getBoundingRadius (void) const
    { return mBoundingRadius; }

    virtual void setBoundingBox (const Ogre::AxisAlignedBox &b)
    { mBoundingBox = b; }

    virtual void setBoundingRadius (Ogre::Real v)
    { mBoundingRadius = v; }

    virtual void visitRenderables(Ogre::Renderable::Visitor *v, bool b);

    virtual void _updateRenderQueue(Ogre::RenderQueue *q);


    const std::string &getMeshName (void);

    friend class SharedPtr<GfxInstances>;
};

#endif
