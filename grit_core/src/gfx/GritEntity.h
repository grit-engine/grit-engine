/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef GRIT_ENTITY_H
#define GRIT_ENTITY_H

#include "OgrePrerequisites.h"
#include "OgreCommon.h"

#include "OgreString.h"
#include "OgreMovableObject.h"
#include "OgreQuaternion.h"
#include "OgreVector3.h"
#include "OgreHardwareBufferManager.h"
#include "OgreMesh.h"
#include "OgreRenderable.h"
#include "OgreResourceGroupManager.h"
#include "OgreShadowCaster.h"
#include "OgreMatrix4.h"

class GritSubEntity;

class GritEntity: public Ogre::MovableObject
{
    friend class GritSubEntity;

public:
    
    typedef std::set<GritEntity*> EntitySet;
    GritEntity( const std::string& name, const Ogre::MeshPtr& mesh);

protected:

    GritEntity();

    Ogre::MeshPtr mMesh;

    typedef std::vector<GritSubEntity*> SubEntityList;
    SubEntityList mSubEntityList;


    Ogre::AnimationStateSet* mAnimationState;

    bool mHardwareAnimationDirty;
    bool mHardwareAnimation;

    Ogre::TempBlendedBufferInfo mTempSkelAnimInfo;
    Ogre::VertexData* mSkelAnimVertexData;

    void extractTempBufferInfo(Ogre::VertexData* sourceData, Ogre::TempBlendedBufferInfo* info);
    Ogre::VertexData* cloneVertexDataRemoveBlendInfo(const Ogre::VertexData* source);
    void prepareTempBlendBuffers(void);

    Ogre::Matrix4 *mBoneWorldMatrices;
    Ogre::Matrix4 *mBoneMatrices;
    unsigned short mNumBoneMatrices;
    unsigned long mFrameAnimationLastUpdated;

    void updateAnimation(void);

    unsigned long *mFrameBonesLastUpdated;

    bool cacheBoneMatrices(void);

    bool mCurrentHWAnimationState;

    int mSoftwareAnimationNormalsRequests;
    bool mSkipAnimStateUpdates;


    Ogre::SkeletonInstance* mSkeletonInstance;

    bool mInitialised;

    Ogre::Matrix4 mLastParentXform;

    size_t mMeshStateCount;

    void buildSubEntityList(Ogre::MeshPtr& mesh, SubEntityList* sublist);

    void reevaluateVertexProcessing(void);

    Ogre::ushort initHardwareAnimationElements(Ogre::VertexData* vdata, Ogre::ushort numberOfElements, bool animateNormals);
    bool tempSkelAnimBuffersBound(bool requestNormals) const;


    mutable Ogre::AxisAlignedBox mFullBoundingBox;

public:
    ~GritEntity();

    const Ogre::MeshPtr& getMesh(void) const;

    GritSubEntity* getSubEntity(unsigned int index) const;

    GritSubEntity* getSubEntity( const std::string& name ) const;

    unsigned int getNumSubEntities(void) const;

    const Ogre::AxisAlignedBox& getBoundingBox(void) const;

    Ogre::AxisAlignedBox getChildObjectsBoundingBox(void) const;

    void _updateRenderQueue(Ogre::RenderQueue* queue);

    const std::string& getMovableType(void) const;

    Ogre::AnimationStateSet* getAllAnimationStates(void) const;

    float getBoundingRadius(void) const;

    const Ogre::Matrix4* _getBoneMatrices(void) const { return mBoneMatrices;}
    unsigned short _getNumBoneMatrices(void) const { return mNumBoneMatrices; }
    bool hasSkeleton(void) const { return mSkeletonInstance != 0; }
    Ogre::SkeletonInstance* getSkeleton(void) const { return mSkeletonInstance; }
    bool isHardwareAnimationEnabled(void);

    void refreshAvailableAnimationState(void);
    void _updateAnimation(void);
    bool _isAnimated(void) const;
    bool _isSkeletonAnimated(void) const;
    Ogre::VertexData* _getSkelAnimVertexData(void) const;
    Ogre::TempBlendedBufferInfo* _getSkelAnimTempBufferInfo(void);
    Ogre::uint32 getTypeFlags(void) const;
    Ogre::VertexData* getVertexDataForBinding(void);

    enum VertexDataBindChoice
    {
        BIND_ORIGINAL,
        BIND_SOFTWARE_SKELETAL
    };
    VertexDataBindChoice chooseVertexDataForBinding();

    bool isInitialised(void) const { return mInitialised; }

    void _initialise(bool forceReinitialise = false);
    void _deinitialise(void);

    void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

    void setSkipAnimationStateUpdate(bool skip) {
        mSkipAnimStateUpdates = skip;
    }
    
    bool getSkipAnimationStateUpdate() const {
        return mSkipAnimStateUpdates;
    }


    
};

#endif
