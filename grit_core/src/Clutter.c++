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

#include <OgreMaterialManager.h>
#include <OgreMaterial.h>
#include <OgreHardwareBufferManager.h>

#include "Clutter.h"
#include "CentralisedLog.h"




////////////////////////////////////////////////////////////////////////////////////////////////////
// Clutter /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Clutter::Clutter (const Ogre::String &name, unsigned triangles)
      : Ogre::MovableObject(name), mTriangles(triangles)
{
}

Clutter::~Clutter (void)
{
    for (SectionMap::iterator i=sects.begin(),i_=sects.end() ; i!=i_ ; ++i) {
        delete i->second;
    }
}



void Clutter::visitRenderables (Ogre::Renderable::Visitor *visitor, bool debug)
{
    (void) debug;
    for (SectionMap::iterator i=sects.begin(),i_=sects.end() ; i!=i_ ; ++i) {
        visitor->visit(i->second, 0, false);
    }
}


void Clutter::_updateRenderQueue (Ogre::RenderQueue *queue)
{
    for (SectionMap::iterator i=sects.begin(),i_=sects.end() ; i!=i_ ; ++i) {
        Section &s = *i->second;
        if (s.getRenderOperation()->vertexData->vertexCount == 0) return;

        if (mRenderQueuePrioritySet) {
            assert(mRenderQueueIDSet == true);
            queue->addRenderable(&s, mRenderQueueID,
                                     mRenderQueuePriority);
        } else if (mRenderQueueIDSet) {
            queue->addRenderable(&s, mRenderQueueID,
                                     queue->getDefaultRenderablePriority());
        } else {
            queue->addRenderable(&s, queue->getDefaultQueueGroup(),
                                     queue->getDefaultRenderablePriority());
        }
    }
}

Clutter::MTicket Clutter::reserveGeometry (const Ogre::MeshPtr &mesh)
{
    (void) mesh;
    return MTicket(42,42);
}

void Clutter::releaseGeometry (const MTicket &t)
{
    (void) t;
}

void Clutter::updateGeometry (const MTicket &t,
                              const Ogre::MeshPtr &mesh,
                              const Ogre::Vector3 &position,
                              const Ogre::Quaternion &orientation)
{
    (void) t;
    (void) mesh;
    (void) position;
    (void) orientation;
}

Clutter::QTicket Clutter::reserveQuad (const Ogre::MaterialPtr &m)
{
    Section &s = getOrCreateSection(m);
    return QTicket(m, s.reserveQuad());
}

void Clutter::releaseQuad (const QTicket &t)
{
    Section &s = getOrCreateSection(t.m);
    s.releaseQuad(t.t);
}

void Clutter::updateQuad (const QTicket &t,
                          const Ogre::Vector3 (&pos)[4],
                          const Ogre::Vector3 (&norm)[4],
                          const Ogre::Vector2 (&uv)[4])
{
    Section &s = getOrCreateSection(t.m);
    s.updateQuad(t.t, pos, norm, uv);
}




////////////////////////////////////////////////////////////////////////////////////////////////////
// Clutter::Section ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Clutter::Section::Section (Clutter *parent, uint32_t triangles, const Ogre::MaterialPtr &m)
{
    mParent = parent;
    mMaterial = m;
    marker = 0;

    mRenderOperation.useIndexes = false;
    mRenderOperation.vertexData = &mVertexData;
    mRenderOperation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    mVertexData.vertexCount = triangles * 3;

    mDeclSize = 0;
    mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT2,
                                                            Ogre::VES_TEXTURE_COORDINATES);
    mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

    // Make the vertex buffer larger if estimated vertex count higher
    // to allow for user-configured growth area
    Ogre::HardwareVertexBufferSharedPtr vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            mDeclSize, mVertexData.vertexCount,
            Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

    mVertexData.vertexBufferBinding->setBinding(0, vbuf);

    usage.resize(triangles);
    data.resize(mVertexData.vertexCount*mDeclSize);
    vbuf->writeData(0, mVertexData.vertexCount*mDeclSize, &data[0]);
}

Clutter::Section::~Section (void)
{
}

void Clutter::Section::reserveTriangles (uint32_t triangles, uint32_t &off, uint32_t &len)
{
    // do a linear search to find a free block of size 'triangles'
    uint32_t found = 0;
    for (uint32_t i=0 ; i<usage.size() ; ++i) {
        if (marker == usage.size()) {
            found = 0;
            marker = 0;
        }
        bool tri_used = usage[marker++];
        if (tri_used) {
            found = 0;
        } else {
            found++;
            if (found==triangles) break;
        }
    }
    len = found;
    off = marker-found;
}

Clutter::Section::MTicket Clutter::Section::reserveGeometry (uint32_t triangles)
{
    uint32_t off, len;
    reserveTriangles(triangles, off, len);
    return MTicket (off, len);
}

void Clutter::Section::releaseGeometry (const MTicket &t)
{
    for (unsigned o=t.offset ; o<t.offset+t.length ; ++o) {
        usage[o] = false;
    }

    float data[3*8] = { 0 };

    mVertexData.vertexBufferBinding->getBuffer(0)
        ->writeData(t.offset*3*mDeclSize, t.length*3*mDeclSize, data);
}

void Clutter::Section::updateGeometry (const MTicket &t,
                                       const Ogre::Vector3 &position,
                                       const Ogre::Quaternion &orientation)
{
    (void) t;
    (void) position;
    (void) orientation;
}

Clutter::Section::QTicket Clutter::Section::reserveQuad (void)
{
    uint32_t off, len;
    reserveTriangles(2, off, len);
    if (len==0) return QTicket(0xFFFFFFFF);
    return QTicket(off);
}

void Clutter::Section::releaseQuad (const QTicket &t)
{
    releaseGeometry(MTicket(t.offset,2));
}

void Clutter::Section::updateQuad (const QTicket &t,
                                   const Ogre::Vector3 (&pos)[4],
                                   const Ogre::Vector3 (&norm)[4],
                                   const Ogre::Vector2 (&uv)[4])
{
    const int idxs[6] = { 0, 3, 1, 0, 2, 3 };
    APP_ASSERT(mDeclSize == 8*sizeof(float));
    for (uint32_t i=0 ; i<6 ; ++i) {
        uint32_t vi = (i + t.offset*3) * mDeclSize;
        //std::cout << t.offset << " " << vi << std::endl;
        memcpy(&data[vi + 0*sizeof(float)],  &pos[idxs[i]].x, sizeof(float));
        memcpy(&data[vi + 1*sizeof(float)],  &pos[idxs[i]].y, sizeof(float));
        memcpy(&data[vi + 2*sizeof(float)],  &pos[idxs[i]].z, sizeof(float));
        memcpy(&data[vi + 3*sizeof(float)], &norm[idxs[i]].x, sizeof(float));
        memcpy(&data[vi + 4*sizeof(float)], &norm[idxs[i]].y, sizeof(float));
        memcpy(&data[vi + 5*sizeof(float)], &norm[idxs[i]].z, sizeof(float));
        memcpy(&data[vi + 6*sizeof(float)],   &uv[idxs[i]].x, sizeof(float));
        memcpy(&data[vi + 7*sizeof(float)],   &uv[idxs[i]].y, sizeof(float));
    }

    mVertexData.vertexBufferBinding->getBuffer(0)
        ->writeData(t.offset*3*mDeclSize, 2*3*mDeclSize, &data[t.offset*3*mDeclSize]);
}




////////////////////////////////////////////////////////////////////////////////////////////////////
// ClutterFactory //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Ogre::MovableObject* ClutterFactory::createInstanceImpl (
        const Ogre::String& name, const Ogre::NameValuePairList* params)
{
    Ogre::NameValuePairList::const_iterator ni;

    if (params==0 || (ni = params->find("triangles"))==params->end())
        GRIT_EXCEPT("Must give triangles when creating Clutter object");

    // must have mesh parameter
    return OGRE_NEW Clutter(name, atoi(ni->second.c_str()));
}


void ClutterFactory::destroyInstance (Ogre::MovableObject* obj)
{
    OGRE_DELETE obj;
}

// vim: ts=4:sw=4:expandtab
