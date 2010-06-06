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

class Clutter;
class ClutterFactory;

#ifndef CLUTTER_H
#define CLUTTER_H

#include <cfloat>

#include <OgreMovableObject.h>
#include <OgreVertexIndexData.h>


class Clutter : public Ogre::MovableObject {

    public:

        class Section : public Ogre::Renderable, public Ogre::MovableAlloc
        {
        protected:
            Clutter *mParent;
            Ogre::MaterialPtr mMaterial;
            Ogre::RenderOperation mRenderOperation;
            Ogre::VertexData mVertexData;

            std::vector<unsigned char> data;
            std::vector<bool> usage;
            uint32_t marker;
            uint32_t mDeclSize;
     
            struct BTicket {
                const uint32_t offset;
                BTicket(uint32_t offset_) : offset(offset_) { }
                BTicket(const BTicket &o) : offset(o.offset) { }
                bool failed (void) { return offset == 0xFFFFFFFF; }
            };

        public:

            struct QTicket : public BTicket {
                QTicket(uint32_t offset_) : BTicket(offset_) { }
                QTicket(const QTicket &o) : BTicket(o.offset) { }
            };

            struct MTicket : public BTicket {
                const uint32_t length;
                MTicket(uint32_t offset_, uint32_t length_) : BTicket(offset_), length(length_) { }
                MTicket(const MTicket &o) : BTicket(o.offset), length(o.length) { }
            };

            Section (Clutter *parent, uint32_t triangles, const Ogre::MaterialPtr &m);
            ~Section (void);
     
            Ogre::RenderOperation *getRenderOperation (void) { return &mRenderOperation; }
     
            // Renderable overrides
            const Ogre::MaterialPtr& getMaterial (void) const { return mMaterial; }
            void getRenderOperation (Ogre::RenderOperation& op) { op = mRenderOperation; }
            void getWorldTransforms (Ogre::Matrix4* xform) const
            { xform[0] = Ogre::Matrix4::IDENTITY; }
            Ogre::Real getSquaredViewDepth (const Ogre::Camera *) const { return 0; }
            const Ogre::LightList &getLights (void) const { return mParent->queryLights(); }
            
            
            MTicket reserveGeometry (uint32_t triangles);

            void releaseGeometry (const MTicket &t);

            void updateGeometry (const MTicket &t,
                                 const Ogre::Vector3 &position,
                                 const Ogre::Quaternion &orientation);


            QTicket reserveQuad (void);

            void releaseQuad (const QTicket &t);

            void updateQuad (const QTicket &t,
                             const Ogre::Vector3 (&pos)[4],
                             const Ogre::Vector3 (&norm)[4],
                             const Ogre::Vector2 (&uv)[4]);

            protected:

            void reserveTriangles (uint32_t triangles, uint32_t &off, uint32_t &len);
                    
        };      

        struct QTicket {
            const Ogre::MaterialPtr &m;
            const Section::QTicket t;
            QTicket(const Ogre::MaterialPtr &m_, const Section::QTicket &t_) : m(m_), t(t_) { }
            QTicket(const QTicket &o) : m(o.m), t(o.t) { }
        };
        typedef Section::MTicket MTicket;

        Clutter (const Ogre::String &name, uint32_t triangles);

        virtual ~Clutter (void);

        virtual const Ogre::String& getMovableType (void) const
        {
            static const Ogre::String type = "Clutter";
            return type;
        }

        virtual const Ogre::AxisAlignedBox& getBoundingBox (void) const
        { return Ogre::AxisAlignedBox::BOX_INFINITE; }

        virtual Ogre::Real getBoundingRadius (void) const
        { return FLT_MAX; }

        virtual void visitRenderables(Ogre::Renderable::Visitor*, bool);
        virtual void _updateRenderQueue(Ogre::RenderQueue*);


        MTicket reserveGeometry (const Ogre::MeshPtr &mesh);
        void releaseGeometry (const MTicket &t);
        void updateGeometry (const MTicket &t,
                             const Ogre::MeshPtr &mesh,
                             const Ogre::Vector3 &position,
                             const Ogre::Quaternion &orientation);

        QTicket reserveQuad (const Ogre::MaterialPtr &m);
        void releaseQuad (const QTicket &t);
        void updateQuad (const QTicket &t,
                         const Ogre::Vector3 (&pos)[4],
                         const Ogre::Vector3 (&norm)[4],
                         const Ogre::Vector2 (&uv)[4]);


    protected:

        uint32_t mTriangles;

        typedef std::map<Ogre::MaterialPtr, Section*> SectionMap;
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




class ClutterFactory : public Ogre::MovableObjectFactory {

    public:

        virtual const Ogre::String& getType (void) const
        {
                static const Ogre::String typeName = "Clutter";
                return typeName;
        }

        virtual Ogre::MovableObject* createInstanceImpl (
               const Ogre::String& name, const Ogre::NameValuePairList* params);

        virtual void destroyInstance (Ogre::MovableObject* obj);

};

#endif

// vim: ts=4:sw=4:expandtab
