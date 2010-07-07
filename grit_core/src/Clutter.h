/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

#include "math_util.h"

#include "CacheFriendlyRangeSpaceSIMD.h"

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
     
            struct BTicket {
                const unsigned offset;
                BTicket(unsigned offset_) : offset(offset_) { }
                BTicket(const BTicket &o) : offset(o.offset) { }
                bool failed (void) { return offset == 0xFFFFFFFF; }
            };

        public:

            // get a bit more type safety
            struct QTicket : public BTicket {
                QTicket(unsigned offset_) : BTicket(offset_) { }
                QTicket(const QTicket &o) : BTicket(o.offset) { }
            };

            struct MTicket : public BTicket {
                MTicket(unsigned offset_) : BTicket(offset_) { }
                MTicket(const MTicket &o) : BTicket(o.offset) { }
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
            void releaseGeometry (const MTicket &t, Ogre::SubMesh *sm);
            void updateGeometry (const MTicket &t,
                                 const Ogre::SubMesh *sm,
                                 const Ogre::Vector3 &position,
                                 const Ogre::Quaternion &orientation);


            QTicket reserveQuad (void);
            void releaseQuad (const QTicket &t);
            void updateQuad (const QTicket &t,
                             const Ogre::Vector3 (&pos)[4],
                             const Ogre::Vector3 (&norm)[4],
                             const Ogre::Vector2 (&uv)[4],
                             const Ogre::Vector3 (*tang)[4]);

            protected:

            void reserveTriangles (unsigned triangles, unsigned &off, unsigned &len);
            void releaseTriangles (unsigned off, unsigned len);
                    
        };      

        struct QTicket {
            Ogre::MaterialPtr m; // used as key to get the right Section
            const Section::QTicket t;
            QTicket (const Ogre::MaterialPtr &m_, const Section::QTicket &t_) : m(m_), t(t_) { }
            QTicket (const QTicket &o) : m(o.m), t(o.t) { }
        };
        struct MTicket {
            Ogre::MaterialPtr m;
            Ogre::MeshPtr mesh; // need to look at the mesh each time it is updated
            const Section::MTicket t;
            MTicket (const Ogre::MaterialPtr &m_,
                     const Ogre::MeshPtr &mesh_,
                     const Section::MTicket &t_) : m(m_), mesh(mesh_), t(t_) { }
            MTicket (const MTicket &o) : m(o.m), mesh(o.mesh), t(o.t) { }
        };

        ClutterBuffer (Ogre::MovableObject *mobj, unsigned triangles, bool tangents);

        virtual ~ClutterBuffer (void);

        MTicket reserveGeometry (const Ogre::MeshPtr &mesh);
        void releaseGeometry (const MTicket &t);
        void updateGeometry (const MTicket &t,
                             const Ogre::Vector3 &position,
                             const Ogre::Quaternion &orientation);

        QTicket reserveQuad (const Ogre::MaterialPtr &m);
        void releaseQuad (const QTicket &t);
        void updateQuad (const QTicket &t,
                         const Ogre::Vector3 (&pos)[4],
                         const Ogre::Vector3 (&norm)[4],
                         const Ogre::Vector2 (&uv)[4],
                         const Ogre::Vector3 (*tang)[4]);


        typedef std::map<Ogre::MaterialPtr, Section*> SectionMap;

        const SectionMap &getSections (void) { return sects; } 


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
        void releaseGeometry (const ClutterBuffer::MTicket &t)
        { return clutter.releaseGeometry(t); }
        void updateGeometry (const ClutterBuffer::MTicket &t,
                             const Ogre::Vector3 &position,
                             const Ogre::Quaternion &orientation)
        { return clutter.updateGeometry(t,position,orientation); }

        ClutterBuffer::QTicket reserveQuad (const Ogre::MaterialPtr &m)
        { return clutter.reserveQuad(m); }
        void releaseQuad (const ClutterBuffer::QTicket &t)
        { return clutter.releaseQuad(t); }
        void updateQuad (const ClutterBuffer::QTicket &t,
                         const Ogre::Vector3 (&pos)[4],
                         const Ogre::Vector3 (&norm)[4],
                         const Ogre::Vector2 (&uv)[4],
                         const Ogre::Vector3 (*tang)[4])
        { return clutter.updateQuad(t,pos,norm,uv,tang); }


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
class RangedClutter : public Ogre::MovableObject {

    public:

        RangedClutter (const Ogre::String &name, unsigned triangles, bool tangents)
            : Ogre::MovableObject(name),
              mBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE), mBoundingRadius(FLT_MAX),
              mClutter(this,triangles,tangents)
        { }

        virtual ~RangedClutter (void) { }

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


        void update (const Ogre::Vector3 &);

        void push_back (const Transform &t);

        size_t size (void) { return mSpace.size(); }

    protected:

        struct Item {
            RangedClutter *parent;
            int index;
            Ogre::Vector3 pos;
            Ogre::Quaternion quat;
            void updateSphere (float x_, float y_, float z_, float r_)
            {
                parent->mSpace.updateSphere(index, x_, y_, z_, r_);
            }
            void updateIndex (int index_)
            {
                index = index_;
            }
        };

        Ogre::AxisAlignedBox mBoundingBox;
        Ogre::Real mBoundingRadius;
        ClutterBuffer mClutter;
        CacheFriendlyRangeSpace<Item> mSpace;
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
