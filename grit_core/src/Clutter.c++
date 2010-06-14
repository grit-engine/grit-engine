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

#include <OgreHardwareBufferManager.h>
#include <OgreMaterialManager.h>
#include <OgreSubMesh.h>
#include <OgreMesh.h>

#include "Clutter.h"
#include "CentralisedLog.h"




////////////////////////////////////////////////////////////////////////////////////////////////////
// Clutter /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Clutter::Clutter (const Ogre::String &name, unsigned triangles, bool tangents)
      : Ogre::MovableObject(name), mTriangles(triangles), mTangents(tangents)
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
    int i=0;
    mesh->load();
    //for (int i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        APP_ASSERT(sm->operationType == Ogre::RenderOperation::OT_TRIANGLE_LIST);
        Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().getByName(sm->getMaterialName(), "GRIT");
        Section &s = getOrCreateSection(m);
        Section::MTicket stkt = s.reserveGeometry(sm);
    //}
    //FIXME:  need some way of having an arbitrary number of Section::MTicket inside our own MTicket
    return MTicket(m, mesh, stkt);
}

void Clutter::releaseGeometry (const MTicket &t)
{
    (void) t;
}

void Clutter::updateGeometry (const MTicket &t,
                              const Ogre::Vector3 &position,
                              const Ogre::Quaternion &orientation)
{
    int i=0;
    //for (int i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = t.mesh->getSubMesh(i);
        Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().getByName(sm->getMaterialName(), "GRIT");
        Section &s = getOrCreateSection(t.m);
        s.updateGeometry(t.t, sm, position, orientation);
    //}
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
                          const Ogre::Vector2 (&uv)[4],
                          const Ogre::Vector3 (*tang)[4])

{
    Section &s = getOrCreateSection(t.m);
    s.updateQuad(t.t, pos, norm, uv, tang);
}




////////////////////////////////////////////////////////////////////////////////////////////////////
// Clutter::Section ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Clutter::Section::Section (Clutter *parent, unsigned triangles, const Ogre::MaterialPtr &m)
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
    if (mParent->mTangents) {
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,Ogre::VES_TANGENT);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    }

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

void Clutter::Section::reserveTriangles (unsigned triangles, unsigned &off, unsigned &len)
{
    // do a linear search to find a free block of size 'triangles'
    unsigned found = 0;
    for (unsigned i=0 ; i<usage.size() ; ++i) {
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

void Clutter::Section::releaseTriangles (unsigned off, unsigned len)
{
    for (unsigned o=off ; o<off+len ; ++o) {
        usage[o] = false;
    }
    memset(&data[3*mDeclSize*off], 0, 3*mDeclSize*len);
    mVertexData.vertexBufferBinding->getBuffer(0)
        ->writeData(off*3*mDeclSize, len*3*mDeclSize, &data[0]);
}



Clutter::Section::MTicket Clutter::Section::reserveGeometry (Ogre::SubMesh *sm)
{

    Ogre::IndexData* idata = sm->indexData;
    unsigned triangles = idata->indexCount / 3u;
    unsigned off, len;
    reserveTriangles(triangles, off, len);
    APP_ASSERT(len == triangles);
    return MTicket (off);
}

void Clutter::Section::releaseGeometry (const MTicket &t, Ogre::SubMesh *sm)
{
    Ogre::IndexData* idata = sm->indexData;
    int triangles = idata->indexCount / 3;
    releaseTriangles(t.offset, triangles);
}

void Clutter::Section::updateGeometry (const MTicket &t,
                                       const Ogre::SubMesh *sm,
                                       const Ogre::Vector3 &position,
                                       const Ogre::Quaternion &orientation)
{
    Ogre::IndexData* idata = sm->indexData;
    unsigned triangles = idata->indexCount / 3;
    Ogre::VertexData* vdata = sm->useSharedVertices ? sm->parent->sharedVertexData
                                                    : sm->vertexData;
    Ogre::VertexDeclaration *vdecl = vdata->vertexDeclaration;

    const Ogre::VertexElement *vel_pos = vdecl->findElementBySemantic (Ogre::VES_POSITION);
    APP_ASSERT(vel_pos->getType() == Ogre::VET_FLOAT3);
    const Ogre::VertexElement *vel_norm = vdecl->findElementBySemantic (Ogre::VES_NORMAL);
    APP_ASSERT(vel_norm->getType() == Ogre::VET_FLOAT3);
    const Ogre::VertexElement *vel_uv= vdecl->findElementBySemantic (Ogre::VES_TEXTURE_COORDINATES);
    APP_ASSERT(vel_uv->getType() == Ogre::VET_FLOAT2);
    const Ogre::VertexElement *vel_tang;
    if (mParent->mTangents) {
        vel_tang = vdecl->findElementBySemantic (Ogre::VES_TANGENT);
        APP_ASSERT(vel_tang->getType() == Ogre::VET_FLOAT3);
    }


    Ogre::VertexBufferBinding *vbind = vdata->vertexBufferBinding;

    APP_ASSERT(1==vbind->getBufferCount());

    const Ogre::HardwareIndexBufferSharedPtr &ibuf = idata->indexBuffer;
    const Ogre::HardwareVertexBufferSharedPtr &vbuf = vbind->getBuffer(0);

    APP_ASSERT(ibuf->getType() == Ogre::HardwareIndexBuffer::IT_16BIT);

    const char *the_vbuf = (const char*)vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY);
    const unsigned short *the_ibuf =
        (const unsigned short*)ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY);


    for (unsigned i=0 ; i<3*triangles ; ++i) {
        unsigned index = the_ibuf[idata->indexStart + i];
        unsigned vo = (index + vdata->vertexStart) * vdecl->getVertexSize(0);

        Ogre::Vector3 pos;
        memcpy(&pos.x, &the_vbuf[vo + vel_pos->getOffset()], vel_pos->getSize());
        pos = orientation * pos + position;

        Ogre::Vector3 norm;
        memcpy(&norm.x, &the_vbuf[vo + vel_norm->getOffset()], vel_norm->getSize());
        norm = orientation * norm;

        Ogre::Vector3 tang;
        if (mParent->mTangents) {
            memcpy(&tang.x, &the_vbuf[vo + vel_tang->getOffset()], vel_tang->getSize());
            tang = orientation * tang;
        }

        Ogre::Vector2 uv;
        memcpy(&uv.x, &the_vbuf[vo + vel_uv->getOffset()], vel_uv->getSize());

        unsigned vi = (i + t.offset*3) * mDeclSize;
        memcpy(&data[vi + 0*sizeof(float)],  &pos.x, 3*sizeof(float));
        memcpy(&data[vi + 3*sizeof(float)], &norm.x, 3*sizeof(float));
        memcpy(&data[vi + 6*sizeof(float)],   &uv.x, 2*sizeof(float));
        if (mParent->mTangents) {
            memcpy(&data[vi + 8*sizeof(float)], &tang.x, 3*sizeof(float));
        }
    }

    vbuf->unlock();
    ibuf->unlock();

    mVertexData.vertexBufferBinding->getBuffer(0)
        ->writeData(t.offset*3*mDeclSize, triangles*3*mDeclSize, &data[t.offset*3*mDeclSize]);
}



Clutter::Section::QTicket Clutter::Section::reserveQuad (void)
{
    unsigned off, len;
    reserveTriangles(2, off, len);
    if (len==0) return QTicket(0xFFFFFFFF);
    return QTicket(off);
}

void Clutter::Section::releaseQuad (const QTicket &t)
{
    releaseTriangles(t.offset,2);
}

void Clutter::Section::updateQuad (const QTicket &t,
                                   const Ogre::Vector3 (&pos)[4],
                                   const Ogre::Vector3 (&norm)[4],
                                   const Ogre::Vector2 (&uv)[4],
                                   const Ogre::Vector3 (*tang)[4])
{
    const int idxs[6] = { 0, 3, 1, 0, 2, 3 };
    if (mParent->mTangents) {
        APP_ASSERT(mDeclSize == 11*sizeof(float));
    } else {
        APP_ASSERT(mDeclSize == 8*sizeof(float));
    }
    for (unsigned i=0 ; i<6 ; ++i) {
        unsigned vi = (i + t.offset*3) * mDeclSize;
        //std::cout << t.offset << " " << vi << std::endl;
        memcpy(&data[vi + 0*sizeof(float)],  &pos[idxs[i]].x, 3*sizeof(float));
        memcpy(&data[vi + 3*sizeof(float)], &norm[idxs[i]].x, 3*sizeof(float));
        memcpy(&data[vi + 6*sizeof(float)],   &uv[idxs[i]].x, 2*sizeof(float));
        if (mParent->mTangents) {
            memcpy(&data[vi + 8*sizeof(float)], &(*tang)[idxs[i]].x, 3*sizeof(float));
        }
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
    unsigned triangles = atoi(ni->second.c_str());

    bool tangents = false; // optional
    if (params>0 && (ni = params->find("triangles"))!=params->end()) {
        tangents = ni->second != "false";
    }

    // must have mesh parameter
    return OGRE_NEW Clutter(name, triangles, tangents);
}


void ClutterFactory::destroyInstance (Ogre::MovableObject* obj)
{
    OGRE_DELETE obj;
}

// vim: ts=4:sw=4:expandtab
