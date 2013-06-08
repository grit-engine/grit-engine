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
#include "OgreStableHeaders.h"
#include "OgreSkeletonInstance.h"
#include "OgreSceneManager.h"
#include "OgreMaterialManager.h"
#include "OgreSubMesh.h"
#include "OgreTagPoint.h"
#include "OgreLogManager.h"
#include "OgreMesh.h"
#include "OgreException.h"
#include "OgreCamera.h"

#include "GritSubEntity.h"
#include "GritEntity.h"

//-----------------------------------------------------------------------
GritSubEntity::GritSubEntity (GritEntity* parent, Ogre::SubMesh* subMeshBasis)
    : Renderable(), mParentEntity(parent),
    mSubMesh(subMeshBasis), mCachedCamera(0)
{
    mMaterial = Ogre::MaterialManager::getSingleton().getByName(mSubMesh->getMaterialName(), mSubMesh->parent->getGroup());

    mVisible = true;
    mRenderQueueIDSet = false;
    mRenderQueuePrioritySet = false;
    mSkelAnimVertexData = 0;



}
//-----------------------------------------------------------------------
GritSubEntity::~GritSubEntity()
{
    if (mSkelAnimVertexData)
        OGRE_DELETE mSkelAnimVertexData;
}
//-----------------------------------------------------------------------
Ogre::SubMesh* GritSubEntity::getSubMesh(void)
{
    return mSubMesh;
}

void GritSubEntity::setMaterial( const Ogre::MaterialPtr& material )
{
    mMaterial = material;
    
    if (mMaterial.isNull())
    {
        Ogre::LogManager::getSingleton().logMessage("Can't assign material "  
            " to GritSubEntity of " + mParentEntity->getName() + " because this "
            "Material does not exist. Have you forgotten to define it in a "
            ".material script?");
        
        mMaterial = Ogre::MaterialManager::getSingleton().getByName("BaseWhite");
        
        if (mMaterial.isNull())
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Can't assign default material "
                "to GritSubEntity of " + mParentEntity->getName() + ". Did "
                "you forget to call MaterialManager::initialise()?",
                "GritSubEntity::setMaterial");
        }
    }
    
    // tell parent to reconsider material vertex processing options
    mParentEntity->reevaluateVertexProcessing();

}

//-----------------------------------------------------------------------
const Ogre::MaterialPtr& GritSubEntity::getMaterial(void) const
{
    return mMaterial;
}
//-----------------------------------------------------------------------
Ogre::Technique* GritSubEntity::getTechnique(void) const
{
    return mMaterial->getBestTechnique(0, this);
}
//-----------------------------------------------------------------------
void GritSubEntity::getRenderOperation(Ogre::RenderOperation& op)
{
    // Use LOD
    mSubMesh->_getRenderOperation(op, 0);
    // Deal with any vertex data overrides
    op.vertexData = getVertexDataForBinding();

}
//-----------------------------------------------------------------------
Ogre::VertexData* GritSubEntity::getVertexDataForBinding(void)
{
    if (mSubMesh->useSharedVertices)
    {
        return mParentEntity->getVertexDataForBinding();
    }
    else
    {
        GritEntity::VertexDataBindChoice c = mParentEntity->chooseVertexDataForBinding();
        switch(c)
        {
        case GritEntity::BIND_ORIGINAL:
            return mSubMesh->vertexData;
        case GritEntity::BIND_SOFTWARE_SKELETAL:
            return mSkelAnimVertexData;
        };
        // keep compiler happy
        return mSubMesh->vertexData;

    }
}
//-----------------------------------------------------------------------
void GritSubEntity::getWorldTransforms(Ogre::Matrix4* xform) const
{
    if (!mParentEntity->mNumBoneMatrices ||
        !mParentEntity->isHardwareAnimationEnabled())
    {
        // No skeletal animation, or software skinning
        *xform = mParentEntity->_getParentNodeFullTransform();
    }
    else
    {
        // Hardware skinning, pass all actually used matrices
        const Ogre::Mesh::IndexMap& indexMap = mSubMesh->useSharedVertices ?
            mSubMesh->parent->sharedBlendIndexToBoneIndexMap : mSubMesh->blendIndexToBoneIndexMap;
        assert(indexMap.size() <= mParentEntity->mNumBoneMatrices);

        if (mParentEntity->_isSkeletonAnimated())
        {
            // Bones, use cached matrices built when Entity::_updateRenderQueue was called
            assert(mParentEntity->mBoneWorldMatrices);

            Ogre::Mesh::IndexMap::const_iterator it, itend;
            itend = indexMap.end();
            for (it = indexMap.begin(); it != itend; ++it, ++xform)
            {
                *xform = mParentEntity->mBoneWorldMatrices[*it];
            }
        }
        else
        {
            // All animations disabled, use parent entity world transform only
            std::fill_n(xform, indexMap.size(), mParentEntity->_getParentNodeFullTransform());
        }
    }
}
//-----------------------------------------------------------------------
unsigned short GritSubEntity::getNumWorldTransforms(void) const
{
    if (!mParentEntity->mNumBoneMatrices ||
        !mParentEntity->isHardwareAnimationEnabled())
    {
        // No skeletal animation, or software skinning
        return 1;
    }
    else
    {
        // Hardware skinning, pass all actually used matrices
        const Ogre::Mesh::IndexMap& indexMap = mSubMesh->useSharedVertices ?
            mSubMesh->parent->sharedBlendIndexToBoneIndexMap : mSubMesh->blendIndexToBoneIndexMap;
        assert(indexMap.size() <= mParentEntity->mNumBoneMatrices);

        return static_cast<unsigned short>(indexMap.size());
    }
}
//-----------------------------------------------------------------------
Ogre::Real GritSubEntity::getSquaredViewDepth(const Ogre::Camera* cam) const
{
    // First of all, check the cached value
    // NB this is manually invalidated by parent each _notifyCurrentCamera call
    // Done this here rather than there since we only need this for transparent objects
    if (mCachedCamera == cam)
        return mCachedCameraDist;

    Ogre::Node* n = mParentEntity->getParentNode();
    assert(n);
    Ogre::Real dist;
    if (!mSubMesh->extremityPoints.empty())
    {
        const Ogre::Vector3 &cp = cam->getDerivedPosition();
        const Ogre::Matrix4 &l2w = mParentEntity->_getParentNodeFullTransform();
        dist = std::numeric_limits<Ogre::Real>::infinity();
        for (Ogre::vector<Ogre::Vector3>::iterator i = mSubMesh->extremityPoints.begin();
             i != mSubMesh->extremityPoints.end (); ++i)
        {
            Ogre::Vector3 v = l2w * (*i);
            Ogre::Real d = (v - cp).squaredLength();
            
            dist = std::min(d, dist);
        }
    }
    else
        dist = n->getSquaredViewDepth(cam);

    mCachedCameraDist = dist;
    mCachedCamera = cam;

    return dist;
}
//-----------------------------------------------------------------------
const Ogre::LightList& GritSubEntity::getLights(void) const
{
    return mParentEntity->queryLights();
}
//-----------------------------------------------------------------------
void GritSubEntity::setVisible(bool visible)
{
    mVisible = visible;
}
//-----------------------------------------------------------------------
bool GritSubEntity::isVisible(void) const
{
    return mVisible;

}
//-----------------------------------------------------------------------
void GritSubEntity::prepareTempBlendBuffers(void)
{
    if (mSubMesh->useSharedVertices)
        return;

    if (mSkelAnimVertexData) 
    {
        OGRE_DELETE mSkelAnimVertexData;
        mSkelAnimVertexData = 0;
    }

    if (!mSubMesh->useSharedVertices)
    {

        if (mParentEntity->hasSkeleton())
        {
            // Create temporary vertex blend info
            // Prepare temp vertex data if needed
            // Clone without copying data, remove blending info
            // (since blend is performed in software)
            mSkelAnimVertexData = 
                mParentEntity->cloneVertexDataRemoveBlendInfo(mSubMesh->vertexData);
            mParentEntity->extractTempBufferInfo(mSkelAnimVertexData, &mTempSkelAnimInfo);

        }
    }
}
//-----------------------------------------------------------------------
bool GritSubEntity::getCastsShadows(void) const
{
    return mParentEntity->getCastShadows();
}
//-----------------------------------------------------------------------
Ogre::VertexData* GritSubEntity::_getSkelAnimVertexData(void) 
{
    assert (mSkelAnimVertexData && "Not software skinned or has no dedicated geometry!");
    return mSkelAnimVertexData;
}
//-----------------------------------------------------------------------
Ogre::TempBlendedBufferInfo* GritSubEntity::_getSkelAnimTempBufferInfo(void) 
{
    return &mTempSkelAnimInfo;
}
//-----------------------------------------------------------------------
void GritSubEntity::_markBuffersUnusedForAnimation(void)
{
    mVertexAnimationAppliedThisFrame = false;
}
//-----------------------------------------------------------------------------
void GritSubEntity::_markBuffersUsedForAnimation(void)
{
    mVertexAnimationAppliedThisFrame = true;
}

void GritSubEntity::setRenderQueueGroup(Ogre::uint8 queueID)
{
    mRenderQueueIDSet = true;
    mRenderQueueID = queueID;
}

void GritSubEntity::setRenderQueueGroupAndPriority(Ogre::uint8 queueID, Ogre::ushort priority)
{
    setRenderQueueGroup(queueID);
    mRenderQueuePrioritySet = true;
    mRenderQueuePriority = priority;
}

Ogre::uint8 GritSubEntity::getRenderQueueGroup(void) const
{
    return mRenderQueueID;
}

Ogre::ushort GritSubEntity::getRenderQueuePriority(void) const
{
    return mRenderQueuePriority;
}

bool GritSubEntity::isRenderQueueGroupSet(void) const
{
    return mRenderQueueIDSet;
}

bool GritSubEntity::isRenderQueuePrioritySet(void) const
{
    return mRenderQueuePrioritySet;
}

