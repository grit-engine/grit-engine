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

class ClutterBuffer;
class ClutterFactory;

#ifndef CLUTTER_H
#define CLUTTER_H

#include <cfloat>

#include <OgreMovableObject.h>
#include <OgreVertexIndexData.h>
#include <OgreMaterial.h>
#include <OgreMesh.h>


#include "../math_util.h"

#include "../CacheFriendlyRangeSpaceSIMD.h"

#include "../Streamer.h"


// Provides an interface whereby a fixed size vertex buffer is used to render a
// number of instances in a batch.  The instances can be added / removed and
// moved about arbitrarily.

// There are two kinds of instance -- geometry (an entire mesh) and quads (just
// a quad).  By reserving an instance, you get a 'ticket' which can then be
// used to update that instance (e.g. position, fade), or release it (i.e. stop
// using it).

// Current implementation: ?
// ClutterBuffer manages a section for each material involved
// fowards update calls to relevant sections
// Section stores vertex buffers, also usage (records which triangle is used) -- sparse representation
// data stores the data to be copied to the gpu (longer than usage by factor of 3 * vertex size)

// New implementation:
// -------------------

// Use octree to cull aggressively?  No.  Issues with multiple frusta -- shadow
// / reflection renders.  Would be necessary to build several buffers or union
// the frusta.  Can't be bothered and probably would not be a great reduction
// in polys anyway.

// Rebuild buffer every frame ignoring frustum?

// TODO: also need to play nice with the disk_resource subsystem

class ClutterBuffer {

    public:

        class Section : public Ogre::Renderable
        {
        protected:

            ClutterBuffer *mParent;
            Ogre::MaterialPtr mMaterial;
            Ogre::RenderOperation mRenderOperation;
            Ogre::VertexData mVertexData;

            std::vector<unsigned char> data;
            std::vector<bool> usage;
            unsigned marker;
            unsigned mDeclSize;
            unsigned first;
            unsigned last;
            unsigned usedTriangles;

            void updateFirstLast (void);
     
            struct BTicket {
                unsigned long offset;
                BTicket (void) : offset(0xFFFFFFFF) { }
                BTicket (unsigned offset_) : offset(offset_) { }
                BTicket (const BTicket &o) : offset(o.offset) { }
                bool valid (void) { return offset != 0xFFFFFFFF; }
            };

        public:

            // get a bit more type safety
            struct QTicket : public BTicket {
                QTicket (void)  : BTicket() { }
                QTicket (unsigned offset_) : BTicket(offset_) { }
                QTicket (const QTicket &o) : BTicket(o.offset) { }
            };

            struct MTicket : public BTicket {
                MTicket (void)  : BTicket() { }
                MTicket (unsigned offset_) : BTicket(offset_) { }
                MTicket (const MTicket &o) : BTicket(o.offset) { }
            };

            Section (ClutterBuffer *parent, unsigned triangles, const Ogre::MaterialPtr &m);
            ~Section (void);
     
            Ogre::RenderOperation *getRenderOperation (void) { return &mRenderOperation; }
     
            // Renderable overrides
            const Ogre::MaterialPtr& getMaterial (void) const { return mMaterial; }
            void getRenderOperation (Ogre::RenderOperation& op) { op = mRenderOperation; }
            void getWorldTransforms (Ogre::Matrix4* xform) const
            { xform[0] = Ogre::Matrix4::IDENTITY; }
            Ogre::Real getSquaredViewDepth (const Ogre::Camera *) const { return 0; }
            const Ogre::LightList &getLights (void) const { return mParent->mMovableObject->queryLights(); }
            
            
            MTicket reserveGeometry (Ogre::SubMesh *sm);
            void releaseGeometry (MTicket &t, Ogre::SubMesh *sm);
            void updateGeometry (const MTicket &t,
                                 const Ogre::SubMesh *sm,
                                 const Ogre::Vector3 &position,
                                 const Ogre::Quaternion &orientation,
                                 float vis);


            QTicket reserveQuad (void);
            void releaseQuad (QTicket &t);
            void updateQuad (const QTicket &t,
                             const Ogre::Vector3 (&pos)[4],
                             const Ogre::Vector3 (&norm)[4],
                             const Ogre::Vector2 (&uv)[4],
                             const Ogre::Vector3 (*tang)[4],
                             float vis);

            void accumulateUtilisation (size_t &used, size_t &rendered, size_t &total);

            protected:

            void reserveTriangles (unsigned triangles, unsigned &off, unsigned &len);
            void releaseTriangles (unsigned off, unsigned len);
                    
        };

        struct QTicket {
            Ogre::MaterialPtr m; // used as key to get the right Section
            Section::QTicket t;
            QTicket (void) { }
            QTicket (const Ogre::MaterialPtr &m_, const Section::QTicket &t_) : m(m_), t(t_) { }
            QTicket &operator= (const QTicket &o) { m=o.m; t=o.t; return *this; }
            bool valid (void) { return t.valid(); }
        };
        struct MTicket {
            Ogre::MeshPtr mesh; // need to look at the mesh each time it is updated
            Section::MTicket *ts;
            MTicket (void) : ts(NULL) { }
            MTicket (const Ogre::MeshPtr &mesh_,
                     Section::MTicket *ts_) : mesh(mesh_), ts(ts_) { }
            MTicket &operator= (const MTicket &o) { mesh=o.mesh; ts=o.ts; return *this; }
            bool valid (void) { return ts != NULL; }
        };


        ClutterBuffer (Ogre::MovableObject *mobj, unsigned triangles, bool tangents);

        virtual ~ClutterBuffer (void);

        MTicket reserveGeometry (const Ogre::MeshPtr &mesh);
        void releaseGeometry (Section::MTicket *stkts, const Ogre::MeshPtr &mesh);
        void releaseGeometry (MTicket &t);
        void updateGeometry (const MTicket &t,
                             const Ogre::Vector3 &position,
                             const Ogre::Quaternion &orientation,
                             float vis);

        QTicket reserveQuad (const Ogre::MaterialPtr &m);
        void releaseQuad (QTicket &t);
        void updateQuad (const QTicket &t,
                         const Ogre::Vector3 (&pos)[4],
                         const Ogre::Vector3 (&norm)[4],
                         const Ogre::Vector2 (&uv)[4],
                         const Ogre::Vector3 (*tang)[4],
                         float vis);


        typedef std::map<Ogre::MaterialPtr, Section*> SectionMap;

        const SectionMap &getSections (void) { return sects; } 

        void getUtilisation (size_t &used, size_t &rendered, size_t &total);

    protected:

        Ogre::MovableObject *mMovableObject;

        unsigned mTriangles;
        bool mTangents;

        SectionMap sects;

        Section &getOrCreateSection (const Ogre::MaterialPtr &m) {
            SectionMap::iterator i = sects.find(m);
            if (i==sects.end()) {
                Section *&s = sects[m];
                s = new Section(this, mTriangles, m);
                return *s;
            } else {
                return *i->second;
            }
        
        }

};



// Mainly for testing, wraps ClutterBuffer but provides a MovableObject
// interface for Ogre's scenegraph, although it only has the identity transform
// and its bounds are infinite

class MovableClutter : public Ogre::MovableObject {

    public:

        MovableClutter (const Ogre::String &name, unsigned triangles, bool tangents)
            : Ogre::MovableObject(name), clutter(this,triangles,tangents) { }

        virtual ~MovableClutter (void) { }

        virtual const Ogre::String& getMovableType (void) const
        {
            static const Ogre::String type = "MovableClutter";
            return type;
        }

        virtual const Ogre::AxisAlignedBox& getBoundingBox (void) const
        { return Ogre::AxisAlignedBox::BOX_INFINITE; }

        virtual Ogre::Real getBoundingRadius (void) const
        { return FLT_MAX; }

        virtual void visitRenderables(Ogre::Renderable::Visitor *v, bool b);
        virtual void _updateRenderQueue(Ogre::RenderQueue *q);


        ClutterBuffer::MTicket reserveGeometry (const Ogre::MeshPtr &mesh)
        { return clutter.reserveGeometry(mesh); }
        void releaseGeometry (ClutterBuffer::MTicket &t)
        { return clutter.releaseGeometry(t); }
        void updateGeometry (const ClutterBuffer::MTicket &t,
                             const Ogre::Vector3 &position,
                             const Ogre::Quaternion &orientation,
                             float vis)
        { return clutter.updateGeometry(t,position,orientation,vis); }

        ClutterBuffer::QTicket reserveQuad (const Ogre::MaterialPtr &m)
        { return clutter.reserveQuad(m); }
        void releaseQuad (ClutterBuffer::QTicket &t)
        { return clutter.releaseQuad(t); }
        void updateQuad (const ClutterBuffer::QTicket &t,
                         const Ogre::Vector3 (&pos)[4],
                         const Ogre::Vector3 (&norm)[4],
                         const Ogre::Vector2 (&uv)[4],
                         const Ogre::Vector3 (*tang)[4],
                         float vis)
        { return clutter.updateQuad(t,pos,norm,uv,tang,vis); }


    protected:

        ClutterBuffer clutter;

};

class MovableClutterFactory : public Ogre::MovableObjectFactory {

    public:

        virtual const Ogre::String& getType (void) const
        {
                static const Ogre::String typeName = "MovableClutter";
                return typeName;
        }

        virtual Ogre::MovableObject* createInstanceImpl (
               const Ogre::String& name, const Ogre::NameValuePairList* params);

        virtual void destroyInstance (Ogre::MovableObject* obj);

};



// Object streams in
// RangedClutter mobj created, initialised using scatter query
// attached to same node as object entity
// added to a table in streamer that updates it every frame
class RangedClutter : public Ogre::MovableObject, public StreamerCallback {

    public:

        RangedClutter (const Ogre::String &name, unsigned triangles, bool tangents)
            : Ogre::MovableObject(name),
              mItemRenderingDistance(40),
              mVisibility(1),
              mStepSize(100000),
              mBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE), mBoundingRadius(FLT_MAX),
              mClutter(this,triangles,tangents)
        { registerMe(); }

        virtual ~RangedClutter (void) { unregisterMe(); }

        void registerMe (void);
        void unregisterMe (void);

        virtual const Ogre::String& getMovableType (void) const
        {
            static const Ogre::String type = "RangedClutter";
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


        void update (const Vector3 &new_pos);

        // be compatible with std::vector
        void push_back (const Transform &t);
        void reserve (size_t s) {
            items.reserve(s);
            mSpace.reserve(s);
        };

        void setNextMesh (const Ogre::MeshPtr &m) { mNextMesh = m; }
        Ogre::MeshPtr getNextMesh (void) { return mNextMesh; }

        float mItemRenderingDistance;
        float mVisibility;
        float mStepSize;

        size_t size (void) { return mSpace.size(); }

        void getUtilisation (size_t &used, size_t &rendered, size_t &total);

    protected:

        struct Item {
            RangedClutter *parent;
            int index; // maintained by the RangeSpace class
            Ogre::MeshPtr mesh;
            Vector3 pos;
            Quaternion quat;
            bool activated;
            int activatedIndex;
            ClutterBuffer::MTicket ticket;
            float renderingDistance;
            float lastFade;
            void updateSphere (const Vector3 &pos_, float r_)
            {
                renderingDistance = r_;
                pos = pos_;
                parent->mSpace.updateSphere(index, pos.x, pos.y, pos.z, r_);
            }
            void updateIndex (int index_) { index = index_; }
            float range2 (const Vector3 &new_pos) const
            {
                return (new_pos-pos).length2() / renderingDistance / renderingDistance;
            }
            float calcFade (float range2);
        };
        typedef std::vector<Item> Items;

        Ogre::MeshPtr mNextMesh;
        Ogre::AxisAlignedBox mBoundingBox;
        Ogre::Real mBoundingRadius;
        ClutterBuffer mClutter;
        typedef CacheFriendlyRangeSpace<Item*> RS;
        typedef RS::Cargo Cargo;
        RS mSpace;
        Items items;
        Cargo activated;
};




class RangedClutterFactory : public Ogre::MovableObjectFactory {

    public:

        virtual const Ogre::String& getType (void) const
        {
                static const Ogre::String typeName = "RangedClutter";
                return typeName;
        }

        virtual Ogre::MovableObject* createInstanceImpl (
               const Ogre::String& name, const Ogre::NameValuePairList* params);

        virtual void destroyInstance (Ogre::MovableObject* obj);

};

#endif

// vim: ts=4:sw=4:expandtab
