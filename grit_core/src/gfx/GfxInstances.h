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

class GfxInstances;
class GfxInstance;
typedef SharedPtr<GfxInstances> GfxInstancesPtr;


#ifndef GFX_INSTANCES_H
#define GFX_INSTANCES_H

#include "../Streamer.h"

#include "OgreMovableObject.h"

#include "GfxBody.h"

//class GfxInstances : public GfxNode, public fast_erase_index, public StreamerCallback, public Ogre::MovableObject {
class GfxInstances : public GfxNode, public fast_erase_index, public Ogre::MovableObject {

    protected:

    unsigned capacity;
    fast_erase_vector<GfxInstance*> instances;
    static const std::string className;

    class Section;

    unsigned numSections;
    Section **sections;

    Ogre::MeshPtr mesh;
    Ogre::VertexData *sharedVertexData;
    Ogre::HardwareVertexBufferSharedPtr instBuf;
    std::vector<float> instBufRaw;
    bool dirty;

    GfxInstances (GfxDiskResource *mesh, const GfxBodyPtr &par_);
    ~GfxInstances ();

    //void registerMe (void);
    //void unregisterMe (void);
    //void update (const Vector3 &new_pos);

    public:
    static GfxInstancesPtr make (const std::string &mesh, const GfxBodyPtr &par_=GfxBodyPtr(NULL));

    GfxInstance *add (const Vector3 &pos, const Quaternion &q, float fade);
    // in future, perhaps 3d scale, skew, or general 3x3 matrix?
    void update (GfxInstance *inst, const Vector3 &pos, const Quaternion &q, float fade);
    void remove (GfxInstance *inst);

    void reserve (unsigned new_capacity);

    void destroy (void);


    // Stuff for Ogre::MovableObject

    protected:

    Ogre::AxisAlignedBox mBoundingBox;
    Ogre::Real mBoundingRadius;

    void copyToGPU ();
    void copyToGPU (unsigned from, unsigned to, bool discard);

    public:

    virtual const Ogre::String& getMovableType (void) const
    {
        static const Ogre::String type = "GfxInstances";
        return type;
    }

    virtual const Ogre::AxisAlignedBox& getBoundingBox (void) const
    { return mBoundingBox; }

    virtual Ogre::Real getBoundingRadius (void) const
    { return mBoundingRadius; }

    virtual void setBoundingBox (const Ogre::AxisAlignedBox &b)
    { mBoundingBox = b; }

    virtual void setBoundingRadius (Ogre::Real v)
    { mBoundingRadius = v; }

    virtual void visitRenderables(Ogre::Renderable::Visitor *v, bool b);

    virtual void _updateRenderQueue(Ogre::RenderQueue *q);



    friend class SharedPtr<GfxInstances>;
};

#endif
