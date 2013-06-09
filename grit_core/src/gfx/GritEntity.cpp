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

#include "OgreSceneManager.h"
#include "OgreSkeleton.h"
#include "OgreAxisAlignedBox.h"
#include "OgreVector4.h"
#include "OgreSkeletonInstance.h"
#include "OgreAnimation.h"
#include "OgreOptimisedUtil.h"
#include "OgreSubMesh.h"


#include "../CentralisedLog.h"

#include "GritEntity.h"

// {{{ Sub

GritEntity::Sub::Sub (GritEntity* parent, Ogre::SubMesh* subMeshBasis)
    : Renderable(), mParentEntity(parent), mSubMesh(subMeshBasis)
{
    mVisible = true;
    mRenderQueueIDSet = false;
    mRenderQueuePrioritySet = false;
}

GritEntity::Sub::~Sub()
{
}

void GritEntity::Sub::getRenderOperation(Ogre::RenderOperation& op)
{
    mSubMesh->_getRenderOperation(op, 0);
}

unsigned short GritEntity::Sub::getNumWorldTransforms(void) const
{
    if (!mParentEntity->mNumBoneMatrices) {
        // No skeletal animation, or software skinning
        return 1;
    }

    // Hardware skinning, count all actually used matrices
    const Ogre::Mesh::IndexMap& indexMap = mSubMesh->useSharedVertices ?
        mSubMesh->parent->sharedBlendIndexToBoneIndexMap : mSubMesh->blendIndexToBoneIndexMap;
    assert(indexMap.size() <= mParentEntity->mNumBoneMatrices);

    return static_cast<unsigned short>(indexMap.size());
}

void GritEntity::Sub::getWorldTransforms(Ogre::Matrix4* xform) const
{
    if (!mParentEntity->mNumBoneMatrices) {
        // No skeletal animation, or software skinning
        *xform = mParentEntity->_getParentNodeFullTransform();
        return;
    }

    // Hardware skinning, pass all actually used matrices
    const Ogre::Mesh::IndexMap& indexMap = mSubMesh->useSharedVertices ?
        mSubMesh->parent->sharedBlendIndexToBoneIndexMap : mSubMesh->blendIndexToBoneIndexMap;
    assert(indexMap.size() <= mParentEntity->mNumBoneMatrices);

    if (mParentEntity->mBoneWorldMatrices) {

        // Bones, use cached matrices built when Entity::_updateRenderQueue was called
        for (Ogre::Mesh::IndexMap::const_iterator i=indexMap.begin(),i_=indexMap.end() ; i!=i_; ++i) {
            *(xform++) = mParentEntity->mBoneWorldMatrices[*i];
        }

    } else {

        // All animations disabled, use parent entity world transform only
        std::fill_n(xform, indexMap.size(), mParentEntity->_getParentNodeFullTransform());
    }
}

Ogre::Real GritEntity::Sub::getSquaredViewDepth (const Ogre::Camera* cam) const
{
    return mParentEntity->getParentSceneNode()->getSquaredViewDepth(cam);
}

const Ogre::LightList& GritEntity::Sub::getLights (void) const
{
    return mParentEntity->queryLights();
}

bool GritEntity::Sub::getCastsShadows (void) const
{
    return mParentEntity->getCastShadows();
}

void GritEntity::Sub::setRenderQueueGroup(Ogre::uint8 queueID)
{
    mRenderQueueIDSet = true;
    mRenderQueueID = queueID;
}

void GritEntity::Sub::setRenderQueueGroupAndPriority(Ogre::uint8 queueID, Ogre::ushort priority)
{
    setRenderQueueGroup(queueID);
    mRenderQueuePrioritySet = true;
    mRenderQueuePriority = priority;
}

Ogre::uint8 GritEntity::Sub::getRenderQueueGroup(void) const
{
    return mRenderQueueID;
}

Ogre::ushort GritEntity::Sub::getRenderQueuePriority(void) const
{
    return mRenderQueuePriority;
}

bool GritEntity::Sub::isRenderQueueGroupSet(void) const
{
    return mRenderQueueIDSet;
}

bool GritEntity::Sub::isRenderQueuePrioritySet(void) const
{
    return mRenderQueuePrioritySet;
}


// }}}


GritEntity::GritEntity (const std::string &name, const Ogre::MeshPtr& mesh) :
    MovableObject(name), mMesh(mesh)
{
    APP_ASSERT(mMesh->isLoaded());

    for (unsigned short i = 0; i < mMesh->getNumSubMeshes(); ++i) {
        Sub* sub_ent = new Sub(this, mMesh->getSubMesh(i));
        mSubList.push_back(sub_ent);
    }

    if (!mMesh->getSkeleton().isNull()) {
        mSkeletonInstance = OGRE_NEW Ogre::SkeletonInstance(mMesh->getSkeleton());
        mSkeletonInstance->load();
        mNumBoneMatrices = mSkeletonInstance->getNumBones();
        mBoneMatrices      = static_cast<Ogre::Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Ogre::Matrix4) * mNumBoneMatrices, Ogre::MEMCATEGORY_ANIMATION));
        mBoneWorldMatrices = static_cast<Ogre::Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Ogre::Matrix4) * mNumBoneMatrices, Ogre::MEMCATEGORY_ANIMATION));

        mMesh->_initAnimationState(&mAnimationState);
    } else {
        mSkeletonInstance = NULL;
        mNumBoneMatrices = 0;
        mBoneMatrices      = NULL;
        mBoneWorldMatrices = NULL;
    }
}

GritEntity::~GritEntity()
{
    for (SubList::iterator i=mSubList.begin(),i_=mSubList.end() ; i!=i_ ; ++i) {
        delete *i;
    }
    
    if (mSkeletonInstance) {
        OGRE_FREE_SIMD(mBoneWorldMatrices, Ogre::MEMCATEGORY_ANIMATION);
        OGRE_DELETE mSkeletonInstance;
        OGRE_FREE_SIMD(mBoneMatrices, Ogre::MEMCATEGORY_ANIMATION);
    }
}

static std::string mtypename = "GritEntity";
const Ogre::String& GritEntity::getMovableType(void) const
{
    return mtypename;
}



GritEntity::Sub* GritEntity::getSubEntity(unsigned int index) const
{
    APP_ASSERT(index < mSubList.size());
    return mSubList[index];
}






//-----------------------------------------------------------------------
void GritEntity::_updateAnimation(void)
{
    // only kind of animation this entity supports
    if (!mSkeletonInstance) return;

    //update the current hardware animation state
    mSkeletonInstance->setAnimationState(mAnimationState);
    mSkeletonInstance->_getBoneMatrices(mBoneMatrices);

    Ogre::OptimisedUtil::getImplementation()->concatenateAffineMatrices(
        _getParentNodeFullTransform(),
        mBoneMatrices,
        mBoneWorldMatrices,
        mNumBoneMatrices);
}


// {{{ RENDERING

void GritEntity::_updateRenderQueue(Ogre::RenderQueue* queue)
{
    // Add each visible Sub to the queue
    SubList::iterator i, iend;
    iend = mSubList.end();
    for (i = mSubList.begin(); i != iend; ++i)
    {
        if((*i)->isVisible())
        {
            // Order: first use subentity queue settings, if available
            //        if not then use entity queue settings, if available
            //        finally fall back on default queue settings
            if((*i)->isRenderQueuePrioritySet())
            {
                queue->addRenderable(*i, (*i)->getRenderQueueGroup(), (*i)->getRenderQueuePriority());
            }
            else if((*i)->isRenderQueueGroupSet())
            {
                queue->addRenderable(*i, (*i)->getRenderQueueGroup());
            }
            else if (mRenderQueuePrioritySet)
            {
                queue->addRenderable(*i, mRenderQueueID, mRenderQueuePriority);
            }
            else if(mRenderQueueIDSet)
            {
                queue->addRenderable(*i, mRenderQueueID);
            }
            else
            {
                queue->addRenderable(*i);
            }
        }
    }

    // Since we know we're going to be rendered, take this opportunity to
    // update the animation
    _updateAnimation();

}

void GritEntity::visitRenderables(Ogre::Renderable::Visitor* visitor, bool)
{
    // Visit each Sub
    for (SubList::iterator i = mSubList.begin(); i != mSubList.end(); ++i) {
        visitor->visit(*i, 0, false);
    }
}


const Ogre::AxisAlignedBox& GritEntity::getBoundingBox(void) const
{
    return mMesh->getBounds();
}

Ogre::Real GritEntity::getBoundingRadius(void) const
{
    return mMesh->getBoundingSphereRadius();
}

 // }}}
