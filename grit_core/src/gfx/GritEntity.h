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

#include "OgreMovableObject.h"
#include "OgreMesh.h"
#include "OgreRenderable.h"
#include "OgreMatrix4.h"

class GritEntity : public Ogre::MovableObject {

    public:

    class Sub : public Ogre::Renderable {

        protected:

        GritEntity *mParentEntity;

        Ogre::MaterialPtr mMaterial;

        Ogre::SubMesh *mSubMesh;

        bool mVisible;

        Ogre::uint8 mRenderQueueID;
        bool mRenderQueueIDSet;
        Ogre::ushort mRenderQueuePriority;
        bool mRenderQueuePrioritySet;

        public:

        Sub (GritEntity* parent, Ogre::SubMesh* sub_mesh);
        virtual ~Sub();

        GritEntity* getParent(void) const { return mParentEntity; }
        Ogre::SubMesh* getSubMesh(void) const { return mSubMesh; }
        
        const Ogre::MaterialPtr& getMaterial(void) const { return mMaterial; }
        void setMaterial (const Ogre::MaterialPtr& material) { mMaterial = material; }

        virtual bool isVisible(void) const { return mVisible; }
        virtual void setVisible(bool visible) { mVisible = visible; }


        virtual void setRenderQueueGroup(Ogre::uint8 queueID);
        
        virtual void setRenderQueueGroupAndPriority(Ogre::uint8 queueID, Ogre::ushort priority);

        virtual Ogre::uint8 getRenderQueueGroup(void) const;

        virtual Ogre::ushort getRenderQueuePriority(void) const;
            
        virtual bool isRenderQueueGroupSet(void) const;
            
        virtual bool isRenderQueuePrioritySet(void) const;

        
        
        void getRenderOperation(Ogre::RenderOperation& op);
        
        void getWorldTransforms(Ogre::Matrix4* xform) const;
        unsigned short getNumWorldTransforms(void) const;
        Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
        const Ogre::LightList& getLights(void) const;
        bool getCastsShadows(void) const;
                
    };      

    
    GritEntity( const std::string& name, const Ogre::MeshPtr& mesh);
    ~GritEntity();

    const Ogre::AxisAlignedBox& getBoundingBox (void) const;
    float getBoundingRadius (void) const;

    void _updateRenderQueue (Ogre::RenderQueue* queue);
    void visitRenderables (Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

    const std::string& getMovableType (void) const;


    Ogre::AnimationStateSet &getAllAnimationStates (void) { return mAnimationState; }
    const Ogre::Matrix4* _getBoneMatrices (void) const { return mBoneMatrices;}
    unsigned short _getNumBoneMatrices (void) const { return mNumBoneMatrices; }
    Ogre::SkeletonInstance* getSkeleton (void) const { return mSkeletonInstance; }
    void _updateAnimation (void);

    const Ogre::MeshPtr& getMesh (void) const { return mMesh; }

    unsigned int getNumSubEntities (void) const { return mSubList.size(); }
    Sub* getSubEntity (unsigned int index) const;

protected:

    Ogre::MeshPtr mMesh;

    typedef std::vector<Sub*> SubList;
    SubList mSubList;

    Ogre::AnimationStateSet mAnimationState;

    Ogre::Matrix4 *mBoneWorldMatrices;
    Ogre::Matrix4 *mBoneMatrices;
    unsigned short mNumBoneMatrices;
    Ogre::SkeletonInstance* mSkeletonInstance;

};

#endif
