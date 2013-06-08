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
#include "OgreStableHeaders.h"

#include "OgreMeshManager.h"
#include "OgreSubMesh.h"
#include "OgreException.h"
#include "OgreSceneManager.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"
#include "OgreBone.h"
#include "OgreCamera.h"
#include "OgreAxisAlignedBox.h"
#include "OgreHardwareBufferManager.h"
#include "OgreVector4.h"
#include "OgreRoot.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreSkeletonInstance.h"
#include "OgreStringConverter.h"
#include "OgreAnimation.h"
#include "OgreOptimisedUtil.h"
#include "OgreSceneNode.h"
#include "OgreMaterialManager.h"

#include "GritEntity.h"
#include "GritSubEntity.h"
#include "../CentralisedLog.h"

GritEntity::GritEntity ()
    : mAnimationState(NULL),
      mHardwareAnimationDirty(true),
      mSkelAnimVertexData(0),
      mBoneWorldMatrices(NULL),
      mBoneMatrices(NULL),
      mNumBoneMatrices(0),
      mFrameAnimationLastUpdated(std::numeric_limits<unsigned long>::max()),
      mFrameBonesLastUpdated(NULL),
      mCurrentHWAnimationState(false),
      mSkipAnimStateUpdates(false),
      mSkeletonInstance(0),
      mInitialised(false),
      mLastParentXform(Ogre::Matrix4::ZERO),
      mMeshStateCount(0),
      mFullBoundingBox()
{
}
//-----------------------------------------------------------------------
GritEntity::GritEntity( const Ogre::String& name, const Ogre::MeshPtr& mesh) :
    MovableObject(name),
    mMesh(mesh),
    mAnimationState(NULL),
    mHardwareAnimationDirty(true),
    mSkelAnimVertexData(0),
    mBoneWorldMatrices(NULL),
    mBoneMatrices(NULL),
    mNumBoneMatrices(0),
    mFrameAnimationLastUpdated(std::numeric_limits<unsigned long>::max()),
    mFrameBonesLastUpdated(NULL),
    mCurrentHWAnimationState(false),
    mSkipAnimStateUpdates(false),
    mSkeletonInstance(0),
    mInitialised(false),
    mLastParentXform(Ogre::Matrix4::ZERO),
    mMeshStateCount(0),
    mFullBoundingBox()
{
    _initialise();
}
//-----------------------------------------------------------------------
void GritEntity::_initialise(bool forceReinitialise)
{
    
    typedef std::map<int, Ogre::Renderable::CustomParameterMap> M;
    M params;
    if (forceReinitialise && mInitialised) {
        // save custom params
        int counter = 0;
        for (SubEntityList::iterator i=mSubEntityList.begin(), iend=mSubEntityList.end(); i!=iend; ++i)
        {
            params[counter++] = (*i)->getCustomParameters();
        }
        _deinitialise();
    }

    if (mInitialised)
        return;

    APP_ASSERT(mMesh->isLoaded());

    // Is mesh skeletally animated?
    if (mMesh->hasSkeleton() && !mMesh->getSkeleton().isNull())
    {
        mSkeletonInstance = OGRE_NEW Ogre::SkeletonInstance(mMesh->getSkeleton());
        mSkeletonInstance->load();
    }

    // Build main subentity list
    buildSubEntityList(mMesh, &mSubEntityList);


    // Initialise the AnimationState, if Mesh has animation
    if (hasSkeleton())
    {
        mFrameBonesLastUpdated = OGRE_NEW_T(unsigned long, Ogre::MEMCATEGORY_ANIMATION)(std::numeric_limits<unsigned long>::max());
        mNumBoneMatrices = mSkeletonInstance->getNumBones();
        mBoneMatrices = static_cast<Ogre::Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Ogre::Matrix4) * mNumBoneMatrices, Ogre::MEMCATEGORY_ANIMATION));
    }
    if (hasSkeleton())
    {
        mAnimationState = OGRE_NEW Ogre::AnimationStateSet();
        mMesh->_initAnimationState(mAnimationState);
        prepareTempBlendBuffers();
    }

    reevaluateVertexProcessing();
    
    // Update of bounds of the parent SceneNode, if GritEntity already attached
    // this can happen if Mesh is loaded in background or after reinitialisation
    if( mParentNode )
    {
        getParentSceneNode()->needUpdate();
    }

    mInitialised = true;
    mMeshStateCount = mMesh->getStateCount();

    int counter = 0;
    for (SubEntityList::iterator i=mSubEntityList.begin(), iend=mSubEntityList.end(); i!=iend; ++i)
    {
        int index = counter;
        if (params.find(index)==params.end()) index = 0;
        if (params.find(index)!=params.end()) {
            (*i)->setCustomParameters(params[index]);
        }
        counter++;
    }

}
//-----------------------------------------------------------------------
void GritEntity::_deinitialise(void)
{
    if (!mInitialised)
        return;

    // Delete submeshes
    SubEntityList::iterator i, iend;
    iend = mSubEntityList.end();
    for (i = mSubEntityList.begin(); i != iend; ++i)
    {
        // Delete GritSubEntity
        OGRE_DELETE *i;
        *i = 0;
    }
    mSubEntityList.clear();
    
    if (mSkeletonInstance) {
        OGRE_FREE_SIMD(mBoneWorldMatrices, Ogre::MEMCATEGORY_ANIMATION);
        mBoneWorldMatrices = 0;

        OGRE_FREE(mFrameBonesLastUpdated, Ogre::MEMCATEGORY_ANIMATION); mFrameBonesLastUpdated = 0;
        OGRE_DELETE mSkeletonInstance; mSkeletonInstance = 0;
        OGRE_FREE_SIMD(mBoneMatrices, Ogre::MEMCATEGORY_ANIMATION); mBoneMatrices = 0;
        OGRE_DELETE mAnimationState; mAnimationState = 0;
    }

    OGRE_DELETE mSkelAnimVertexData; mSkelAnimVertexData = 0;

    mInitialised = false;
}
//-----------------------------------------------------------------------
GritEntity::~GritEntity()
{
    _deinitialise();
}
//-----------------------------------------------------------------------
const Ogre::MeshPtr& GritEntity::getMesh(void) const
{
    return mMesh;
}
//-----------------------------------------------------------------------
GritSubEntity* GritEntity::getSubEntity(unsigned int index) const
{
    if (index >= mSubEntityList.size())
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
        "Index out of bounds.",
        "GritEntity::getSubEntity");
    return mSubEntityList[index];
}
//-----------------------------------------------------------------------
GritSubEntity* GritEntity::getSubEntity(const Ogre::String& name) const
{
    ushort index = mMesh->_getSubMeshIndex(name);
    return getSubEntity(index);
}
//-----------------------------------------------------------------------
unsigned int GritEntity::getNumSubEntities(void) const
{
    return static_cast< unsigned int >( mSubEntityList.size() );
}
//-----------------------------------------------------------------------

const Ogre::AxisAlignedBox& GritEntity::getBoundingBox(void) const
{
    // Get from Mesh
    if (mMesh->isLoaded())
    {
        mFullBoundingBox = mMesh->getBounds();

        // Don't scale here, this is taken into account when world BBox calculation is done
    }
    else
        mFullBoundingBox.setNull();

    return mFullBoundingBox;
}
//-----------------------------------------------------------------------
void GritEntity::_updateRenderQueue(Ogre::RenderQueue* queue)
{
    // Do nothing if not initialised yet
    if (!mInitialised)
        return;

    // Check mesh state count, will be incremented if reloaded
    if (mMesh->getStateCount() != mMeshStateCount)
    {
        // force reinitialise
        _initialise(true);
    }

    // Add each visible GritSubEntity to the queue
    SubEntityList::iterator i, iend;
    iend = mSubEntityList.end();
    for (i = mSubEntityList.begin(); i != iend; ++i)
    {
        if((*i)->isVisible())
        {
            // Order: first use subentity queue settings, if available
            //        if not then use entity queue settings, if available
            //        finally fall back on default queue settings
            if((*i)->isRenderQueuePrioritySet())
            {
                assert((*i)->isRenderQueueGroupSet() == true);
                queue->addRenderable(*i, (*i)->getRenderQueueGroup(), (*i)->getRenderQueuePriority());
            }
            else if((*i)->isRenderQueueGroupSet())
            {
                queue->addRenderable(*i, (*i)->getRenderQueueGroup());
            }
            else if (mRenderQueuePrioritySet)
            {
                assert(mRenderQueueIDSet == true);
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
    if (hasSkeleton())
    {
        updateAnimation();
    }

}
Ogre::AnimationStateSet* GritEntity::getAllAnimationStates(void) const
{
    return mAnimationState;
}
//-----------------------------------------------------------------------
static std::string mtypename = "GritEntity";
const Ogre::String& GritEntity::getMovableType(void) const
{
    return mtypename;
}
//-----------------------------------------------------------------------
bool GritEntity::tempSkelAnimBuffersBound(bool requestNormals) const
{
    // Do we still have temp buffers for software skeleton animation bound?
    if (mSkelAnimVertexData)
    {
        if (!mTempSkelAnimInfo.buffersCheckedOut(true, requestNormals))
            return false;
    }
    for (SubEntityList::const_iterator i = mSubEntityList.begin();
        i != mSubEntityList.end(); ++i)
    {
        GritSubEntity* sub = *i;
        if (sub->isVisible() && sub->mSkelAnimVertexData)
        {
            if (!sub->mTempSkelAnimInfo.buffersCheckedOut(true, requestNormals))
                return false;
        }
    }
    return true;
}
//-----------------------------------------------------------------------
void GritEntity::updateAnimation(void)
{
    // Do nothing if not initialised yet
    if (!mInitialised)
        return;

    bool hwAnimation = isHardwareAnimationEnabled();
    bool isNeedUpdateHardwareAnim = hwAnimation && !mCurrentHWAnimationState;
    bool softwareAnimation = !hwAnimation;
    // Blend normals in s/w only if we're not using h/w animation,
    // since shadows only require positions
    bool blendNormals = !hwAnimation;
    // Animation dirty if animation state modified or manual bones modified
    bool animationDirty =
        (mFrameAnimationLastUpdated != mAnimationState->getDirtyFrameNumber()) ||
        (hasSkeleton() && getSkeleton()->getManualBonesDirty());
    
    //update the current hardware animation state
    mCurrentHWAnimationState = hwAnimation;

    // We only do these tasks if animation is dirty
    // Or, if we're using a skeleton and manual bones have been moved
    // Or, if we're using software animation and temp buffers are unbound
    if (animationDirty ||
        (softwareAnimation && hasSkeleton() && !tempSkelAnimBuffersBound(blendNormals)))
    {
        if (hasSkeleton())
        {
            cacheBoneMatrices();

            // Software blend?
            if (softwareAnimation)
            {
                const Ogre::Matrix4* blendMatrices[256];

                // Ok, we need to do a software blend
                // Firstly, check out working vertex buffers
                if (mSkelAnimVertexData)
                {
                    // Blend shared geometry
                    // NB we suppress hardware upload while doing blend if we're
                    // hardware animation, because the only reason for doing this
                    // is for shadow, which need only be uploaded then
                    mTempSkelAnimInfo.checkoutTempCopies(true, blendNormals);
                    mTempSkelAnimInfo.bindTempCopies(mSkelAnimVertexData,
                        hwAnimation);
                    // Prepare blend matrices, TODO: Move out of here
                    Ogre::Mesh::prepareMatricesForVertexBlend(blendMatrices,
                        mBoneMatrices, mMesh->sharedBlendIndexToBoneIndexMap);
                    // Blend, taking source from either mesh data or morph data
                    Ogre::Mesh::softwareVertexBlend(
                        mMesh->sharedVertexData,
                        mSkelAnimVertexData,
                        blendMatrices, mMesh->sharedBlendIndexToBoneIndexMap.size(),
                        blendNormals);
                }
                SubEntityList::iterator i, iend;
                iend = mSubEntityList.end();
                for (i = mSubEntityList.begin(); i != iend; ++i)
                {
                    // Blend dedicated geometry
                    GritSubEntity* se = *i;
                    if (se->isVisible() && se->mSkelAnimVertexData)
                    {
                        se->mTempSkelAnimInfo.checkoutTempCopies(true, blendNormals);
                        se->mTempSkelAnimInfo.bindTempCopies(se->mSkelAnimVertexData,
                            hwAnimation);
                        // Prepare blend matrices, TODO: Move out of here
                        Ogre::Mesh::prepareMatricesForVertexBlend(blendMatrices,
                            mBoneMatrices, se->mSubMesh->blendIndexToBoneIndexMap);
                        // Blend, taking source from either mesh data or morph data
                        Ogre::Mesh::softwareVertexBlend(
                            se->mSubMesh->vertexData,
                            se->mSkelAnimVertexData,
                            blendMatrices, se->mSubMesh->blendIndexToBoneIndexMap.size(),
                            blendNormals);
                    }

                }

            }
        }

        mFrameAnimationLastUpdated = mAnimationState->getDirtyFrameNumber();
    }

    // Need to update the child object's transforms when animation dirty
    // or parent node transform has altered.
    if (hasSkeleton() && 
        (isNeedUpdateHardwareAnim || 
        animationDirty || mLastParentXform != _getParentNodeFullTransform()))
    {
        // Cache last parent transform for next frame use too.
        mLastParentXform = _getParentNodeFullTransform();

        // Also calculate bone world matrices, since are used as replacement world matrices,
        // but only if it's used (when using hardware animation and skeleton animated).
        if (hwAnimation && _isSkeletonAnimated())
        {
            // Allocate bone world matrices on demand, for better memory footprint
            // when using software animation.
            if (!mBoneWorldMatrices)
            {
                mBoneWorldMatrices =
                    static_cast<Ogre::Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Ogre::Matrix4) * mNumBoneMatrices, Ogre::MEMCATEGORY_ANIMATION));
            }

            Ogre::OptimisedUtil::getImplementation()->concatenateAffineMatrices(
                mLastParentXform,
                mBoneMatrices,
                mBoneWorldMatrices,
                mNumBoneMatrices);
        }
    }
}
//-----------------------------------------------------------------------
Ogre::ushort GritEntity::initHardwareAnimationElements(Ogre::VertexData* vdata,
    Ogre::ushort numberOfElements, bool animateNormals)
{
    Ogre::ushort elemsSupported = numberOfElements;
    if (vdata->hwAnimationDataList.size() < numberOfElements)
    {
        elemsSupported = 
            vdata->allocateHardwareAnimationElements(numberOfElements, animateNormals);
    }
    // Initialise parametrics incase we don't use all of them
    for (size_t i = 0; i < vdata->hwAnimationDataList.size(); ++i)
    {
        vdata->hwAnimationDataList[i].parametric = 0.0f;
    }
    // reset used count
    vdata->hwAnimDataItemsUsed = 0;
            
    return elemsSupported;

}
//---------------------------------------------------------------------
void GritEntity::_updateAnimation(void)
{
    // Externally visible method
    if (hasSkeleton())
    {
        updateAnimation();
    }
}
//-----------------------------------------------------------------------
bool GritEntity::_isAnimated(void) const
{
    return (mAnimationState && mAnimationState->hasEnabledAnimationState()) ||
           (getSkeleton() && getSkeleton()->hasManualBones());
}
//-----------------------------------------------------------------------
bool GritEntity::_isSkeletonAnimated(void) const
{
    return getSkeleton() &&
        (mAnimationState->hasEnabledAnimationState() || getSkeleton()->hasManualBones());
}
//-----------------------------------------------------------------------
Ogre::VertexData* GritEntity::_getSkelAnimVertexData(void) const
{
    assert (mSkelAnimVertexData && "Not software skinned or has no shared vertex data!");
    return mSkelAnimVertexData;
}
//-----------------------------------------------------------------------
Ogre::TempBlendedBufferInfo* GritEntity::_getSkelAnimTempBufferInfo(void)
{
    return &mTempSkelAnimInfo;
}
//-----------------------------------------------------------------------
bool GritEntity::cacheBoneMatrices(void)
{
    Ogre::Root& root = Ogre::Root::getSingleton();
    unsigned long currentFrameNumber = root.getNextFrameNumber();
    if ((*mFrameBonesLastUpdated != currentFrameNumber) ||
        (hasSkeleton() && getSkeleton()->getManualBonesDirty()))
    {
        if ((!mSkipAnimStateUpdates) && (*mFrameBonesLastUpdated != currentFrameNumber))
            mSkeletonInstance->setAnimationState(*mAnimationState);
        mSkeletonInstance->_getBoneMatrices(mBoneMatrices);
        *mFrameBonesLastUpdated  = currentFrameNumber;

        return true;
    }
    return false;
}
//-----------------------------------------------------------------------
void GritEntity::buildSubEntityList(Ogre::MeshPtr& mesh, SubEntityList* sublist)
{
    // Create SubEntities
    unsigned short i, numSubMeshes;
    Ogre::SubMesh* subMesh;
    GritSubEntity* subEnt;

    numSubMeshes = mesh->getNumSubMeshes();
    for (i = 0; i < numSubMeshes; ++i)
    {
        subMesh = mesh->getSubMesh(i);
        subEnt = OGRE_NEW GritSubEntity(this, subMesh);
        sublist->push_back(subEnt);
    }
}
//-----------------------------------------------------------------------
void GritEntity::prepareTempBlendBuffers(void)
{
    if (mSkelAnimVertexData)
    {
        OGRE_DELETE mSkelAnimVertexData;
        mSkelAnimVertexData = 0;
    }

    if (hasSkeleton())
    {
        // Shared data
        if (mMesh->sharedVertexData)
        {
            // Create temporary vertex blend info
            // Prepare temp vertex data if needed
            // Clone without copying data, remove blending info
            // (since blend is performed in software)
            mSkelAnimVertexData =
                cloneVertexDataRemoveBlendInfo(mMesh->sharedVertexData);
            extractTempBufferInfo(mSkelAnimVertexData, &mTempSkelAnimInfo);
        }

    }

    // Do SubEntities
    SubEntityList::iterator i, iend;
    iend = mSubEntityList.end();
    for (i = mSubEntityList.begin(); i != iend; ++i)
    {
        GritSubEntity* s = *i;
        s->prepareTempBlendBuffers();
    }

}
//-----------------------------------------------------------------------
void GritEntity::extractTempBufferInfo(Ogre::VertexData* sourceData, Ogre::TempBlendedBufferInfo* info)
{
    info->extractFrom(sourceData);
}
//-----------------------------------------------------------------------
Ogre::VertexData* GritEntity::cloneVertexDataRemoveBlendInfo(const Ogre::VertexData* source)
{
    // Clone without copying data
    Ogre::VertexData* ret = source->clone(false);
    bool removeIndices = Ogre::Root::getSingleton().isBlendIndicesGpuRedundant();
    bool removeWeights = Ogre::Root::getSingleton().isBlendWeightsGpuRedundant();
     
    unsigned short safeSource = 0xFFFF;
    const Ogre::VertexElement* blendIndexElem =
        source->vertexDeclaration->findElementBySemantic(Ogre::VES_BLEND_INDICES);
    if (blendIndexElem)
    {
        //save the source in order to prevent the next stage from unbinding it.
        safeSource = blendIndexElem->getSource();
        if (removeIndices)
        {
            // Remove buffer reference
            ret->vertexBufferBinding->unsetBinding(blendIndexElem->getSource());
        }
    }
    if (removeWeights)
    {
        // Remove blend weights
        const Ogre::VertexElement* blendWeightElem =
            source->vertexDeclaration->findElementBySemantic(Ogre::VES_BLEND_WEIGHTS);
        if (blendWeightElem &&
            blendWeightElem->getSource() != safeSource)
        {
            // Remove buffer reference
            ret->vertexBufferBinding->unsetBinding(blendWeightElem->getSource());
        }
    }

    // remove elements from declaration
    if (removeIndices)
        ret->vertexDeclaration->removeElement(Ogre::VES_BLEND_INDICES);
    if (removeWeights)
        ret->vertexDeclaration->removeElement(Ogre::VES_BLEND_WEIGHTS);

    // Close gaps in bindings for effective and safely
    if (removeWeights || removeIndices)
        ret->closeGapsInBindings();

    return ret;
}
//-----------------------------------------------------------------------
bool GritEntity::isHardwareAnimationEnabled(void)
{
    //find whether the entity has hardware animation for the current active sceme
    if (mHardwareAnimationDirty) {
        mHardwareAnimationDirty = false;
        if (hasSkeleton()) {
            for (SubEntityList::iterator i=mSubEntityList.begin(), i_=mSubEntityList.end() ; i!=i_ ; ++i) {

                GritSubEntity* sub = *i;

                const Ogre::MaterialPtr& m = sub->getMaterial();

                Ogre::Technique* t = m->getTechnique(0);
                Ogre::Pass* p = t->getPass(0);

                if (!p->getVertexProgram()->isSkeletalAnimationIncluded()) {
                    CERR << "Entity with skeleton has material without blendedBones: \""<<m->getName()<<"\"" << std::endl;
                }
            }
            mHardwareAnimation = true;
        } else {
            mHardwareAnimation = false;
        }
    }
    return mHardwareAnimation;
}

//-----------------------------------------------------------------------
void GritEntity::reevaluateVertexProcessing(void)
{
    mHardwareAnimationDirty = true;
}
//-----------------------------------------------------------------------
void GritEntity::refreshAvailableAnimationState(void)
{
    mMesh->_refreshAnimationState(mAnimationState);
}
//-----------------------------------------------------------------------
Ogre::uint32 GritEntity::getTypeFlags(void) const
{
    return Ogre::SceneManager::ENTITY_TYPE_MASK;
}
//-----------------------------------------------------------------------
Ogre::VertexData* GritEntity::getVertexDataForBinding(void)
{
    GritEntity::VertexDataBindChoice c = chooseVertexDataForBinding();
    switch(c)
    {
    case BIND_ORIGINAL:
        return mMesh->sharedVertexData;
    case BIND_SOFTWARE_SKELETAL:
        return mSkelAnimVertexData;
    };
    // keep compiler happy
    return mMesh->sharedVertexData;
}
//-----------------------------------------------------------------------
GritEntity::VertexDataBindChoice GritEntity::chooseVertexDataForBinding()
{
    if (hasSkeleton())
    {
        if (!isHardwareAnimationEnabled())
        {
            // all software skeletal binds same vertex data
            // may be a 2-stage s/w transform including morph earlier though
            return BIND_SOFTWARE_SKELETAL;
        }
        else
        {
            // hardware skeletal, no morphing
            return BIND_ORIGINAL;
        }
    }
    else
    {
        return BIND_ORIGINAL;
    }

}
//---------------------------------------------------------------------
void GritEntity::visitRenderables(Ogre::Renderable::Visitor* visitor, bool)
{
    // Visit each GritSubEntity
    for (SubEntityList::iterator i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
    {
        visitor->visit(*i, 0, false);
    }
}


Ogre::Real GritEntity::getBoundingRadius(void) const
{
    return mMesh->getBoundingSphereRadius();
}

