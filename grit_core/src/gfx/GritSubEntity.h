/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef GRIT_SUB_ENTITY_H
#define GRIT_SUB_ENTITY_H

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreRenderable.h"
#include "OgreHardwareBufferManager.h"
#include "OgreResourceGroupManager.h"

class GritEntity;

class GritSubEntity: public Ogre::Renderable, public Ogre::SubEntityAlloc
{
    friend class GritEntity;
    friend class SceneManager;
protected:
    GritSubEntity(GritEntity* parent, Ogre::SubMesh* subMeshBasis);

    virtual ~GritSubEntity();

    GritEntity* mParentEntity;

    Ogre::MaterialPtr mMaterial;

    Ogre::SubMesh* mSubMesh;

    bool mVisible;

    Ogre::uint8 mRenderQueueID;
    bool mRenderQueueIDSet;
    Ogre::ushort mRenderQueuePriority;
    bool mRenderQueuePrioritySet;

    Ogre::VertexData* mSkelAnimVertexData;
    Ogre::TempBlendedBufferInfo mTempSkelAnimInfo;
    bool mVertexAnimationAppliedThisFrame;
    mutable Ogre::Real mCachedCameraDist;
    mutable const Ogre::Camera *mCachedCamera;

    void prepareTempBlendBuffers(void);

public:
    const Ogre::String& getMaterialName() const;

    void setMaterial( const Ogre::MaterialPtr& material );

    virtual void setVisible(bool visible);

    virtual bool isVisible(void) const;

    virtual void setRenderQueueGroup(Ogre::uint8 queueID);

    virtual void setRenderQueueGroupAndPriority(Ogre::uint8 queueID, Ogre::ushort priority);

    virtual Ogre::uint8 getRenderQueueGroup(void) const;

    virtual Ogre::ushort getRenderQueuePriority(void) const;

    virtual bool isRenderQueueGroupSet(void) const;

    virtual bool isRenderQueuePrioritySet(void) const;

    Ogre::SubMesh* getSubMesh(void);

    GritEntity* getParent(void) const { return mParentEntity; }

    const Ogre::MaterialPtr& getMaterial(void) const;

    Ogre::Technique* getTechnique(void) const;

    void getRenderOperation(Ogre::RenderOperation& op);

    void getWorldTransforms(Ogre::Matrix4* xform) const;
    unsigned short getNumWorldTransforms(void) const;
    Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
    const Ogre::LightList& getLights(void) const;
    bool getCastsShadows(void) const;
    Ogre::VertexData* _getSkelAnimVertexData(void);
    Ogre::TempBlendedBufferInfo* _getSkelAnimTempBufferInfo(void);
    Ogre::VertexData* getVertexDataForBinding(void);

    void _markBuffersUnusedForAnimation(void);
    void _markBuffersUsedForAnimation(void);
    bool _getBuffersMarkedForAnimation(void) const { return mVertexAnimationAppliedThisFrame; }

    void _invalidateCameraCache ()
    { mCachedCamera = 0; }
};

#endif
