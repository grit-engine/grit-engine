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

#include "../main.h"
#include "gfx.h"



////////////////////////////////////////////////////////////////////////////////////////////////////
// ClutterBuffer ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ClutterBuffer::ClutterBuffer (Ogre::MovableObject *mobj, unsigned triangles, bool tangents)
      : mMovableObject(mobj), mTriangles(triangles), mTangents(tangents)
{
}

ClutterBuffer::~ClutterBuffer (void)
{
    for (SectionMap::iterator i=sects.begin(),i_=sects.end() ; i!=i_ ; ++i) {
        delete i->second;
    }
}

static Ogre::MaterialPtr mat_from_submesh (const Ogre::MeshPtr &mesh,
                                           Ogre::SubMesh *sm, bool err=false)
{
        std::string name = sm->getMaterialName();
        name += "&"; // use a different material (in order to get a different shader)
        Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().getByName(name, "GRIT");
        if (m.isNull()) {
            if (err) {
                CERR << "Material not found: \"" << sm->getMaterialName() << "\" "
                     << "in mesh \"" << mesh->getName() << "\"" << std::endl;
            }
            m = Ogre::MaterialManager::getSingleton().getByName("/BaseWhite&", "GRIT");
        }
        return m;
}

ClutterBuffer::MTicket ClutterBuffer::reserveGeometry (const Ogre::MeshPtr &mesh)
{
    mesh->load();
    for (int i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        APP_ASSERT(sm->operationType == Ogre::RenderOperation::OT_TRIANGLE_LIST);
        Ogre::MaterialPtr m = mat_from_submesh(mesh,sm,true);
        getOrCreateSection(m);
    }
    Section::MTicket *stkts = new Section::MTicket[mesh->getNumSubMeshes()]();
    for (int i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        Ogre::MaterialPtr m = mat_from_submesh(mesh,sm);
        Section &s = getOrCreateSection(m);
        stkts[i] = s.reserveGeometry(sm);
        if (!stkts[i].valid()) {
            releaseGeometry(stkts, mesh);
            return MTicket(mesh, NULL);
        }
    }
    return MTicket(mesh, stkts);
}

void ClutterBuffer::releaseGeometry (Section::MTicket *stkts, const Ogre::MeshPtr &mesh)
{
    for (int i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        Ogre::MaterialPtr m = mat_from_submesh(mesh,sm);
        Section &s = getOrCreateSection(m);
        Section::MTicket t2 = stkts[i];
        if (t2.valid()) { // it will only be invalid if we aborted half way through construction
            s.releaseGeometry(t2, sm);
        }
    }
    delete [] stkts;
}

void ClutterBuffer::releaseGeometry (MTicket &t)
{
    if (!t.valid()) return;
    releaseGeometry(t.ts, t.mesh);
}

void ClutterBuffer::updateGeometry (const MTicket &t,
                                    const Ogre::Vector3 &position,
                                    const Ogre::Quaternion &orientation,
                                    float vis)
{
    for (int i=0 ; i<t.mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = t.mesh->getSubMesh(i);
        Ogre::MaterialPtr m = mat_from_submesh(t.mesh,sm);
        Section &s = getOrCreateSection(m);
        s.updateGeometry(t.ts[i], sm, position, orientation, vis);
    }
}

ClutterBuffer::QTicket ClutterBuffer::reserveQuad (const Ogre::MaterialPtr &m)
{
    Section &s = getOrCreateSection(m);
    return QTicket(m, s.reserveQuad());
}

void ClutterBuffer::releaseQuad (QTicket &t)
{
    Section &s = getOrCreateSection(t.m);
    s.releaseQuad(t.t);
}

void ClutterBuffer::updateQuad (const QTicket &t,
                                const Ogre::Vector3 (&pos)[4],
                                const Ogre::Vector3 (&norm)[4],
                                const Ogre::Vector2 (&uv)[4],
                                const Ogre::Vector3 (*tang)[4],
                                float vis)

{
    Section &s = getOrCreateSection(t.m);
    s.updateQuad(t.t, pos, norm, uv, tang, vis);
}




////////////////////////////////////////////////////////////////////////////////////////////////////
// ClutterBuffer::Section //////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// what would it take, for triangles to be automatic
// * recreate HWVB
// * copy data from old to new (probably easy because cached in 'data')
// what would it take, for tangents to be automatic
// * the above points
// * change vertex declaration

ClutterBuffer::Section::Section (ClutterBuffer *parent, unsigned triangles,
                                 const Ogre::MaterialPtr &m)
{
    mParent = parent;
    mMaterial = m;
    APP_ASSERT(!m.isNull());
    marker = 0;

    mRenderOperation.useIndexes = false;
    mRenderOperation.vertexData = &mVertexData;
    mRenderOperation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;

    mDeclSize = 0;
    mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT2,
                                                            Ogre::VES_TEXTURE_COORDINATES);
    mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
    mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT1,
                                                            Ogre::VES_TEXTURE_COORDINATES,1);
    mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT1);
    if (mParent->mTangents) {
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,Ogre::VES_TANGENT);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    }

    // Make the vertex buffer larger if estimated vertex count higher
    // to allow for user-configured growth area
    Ogre::HardwareVertexBufferSharedPtr vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            mDeclSize, 3*triangles,
            Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

    mVertexData.vertexBufferBinding->setBinding(0, vbuf);

    usage.resize(triangles);
    data.resize(3*triangles*mDeclSize);
    vbuf->writeData(0, 3*triangles*mDeclSize, &data[0]);
    first = 0;
    last = 0;
    usedTriangles = 0;
    
    //mVertexData.vertexCount = 3*triangles;
    updateFirstLast();
}

ClutterBuffer::Section::~Section (void)
{
}

void ClutterBuffer::Section::updateFirstLast (void)
{
    if (first == last && last == 0) {
        mVertexData.vertexStart = 0;
        mVertexData.vertexCount = 0;
    } else {
        mVertexData.vertexStart = first * 3;
        mVertexData.vertexCount = (last-first+1) * 3;
    }
}

void ClutterBuffer::Section::reserveTriangles (unsigned triangles, unsigned &off, unsigned &len)
{
    if (usedTriangles==usage.size()) {
        len = 0;
        return;
    }
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
    for (unsigned j=off ; j<marker ; ++j) {
        usage[j] = true;
    }
    if (off<first) {
        first = off;
    }
    if (marker-1>last) {
        last = marker-1;
    }
    updateFirstLast();
    usedTriangles += len;
}

void ClutterBuffer::Section::releaseTriangles (unsigned off, unsigned len)
{
    usedTriangles -= len;
    for (unsigned o=off ; o<off+len ; ++o) {
        usage[o] = false;
    }
    while (usage[last] == false) {
        if (last==0) {
            break;
        }
        last--;
    }
    while (usage[first] == false) {
        if (first>=last) {
            first = 0;
            break;
        }
        first++;
    }
    updateFirstLast();
    memset(&data[3*mDeclSize*off], 0, 3*mDeclSize*len);
    mVertexData.vertexBufferBinding->getBuffer(0)
        ->writeData(off*3*mDeclSize, len*3*mDeclSize, &data[off*3*mDeclSize]);
}



ClutterBuffer::Section::MTicket ClutterBuffer::Section::reserveGeometry (Ogre::SubMesh *sm)
{

    Ogre::IndexData* idata = sm->indexData;
    unsigned triangles = idata->indexCount / 3u;
    unsigned off, len;
    reserveTriangles(triangles, off, len);
    if (len==0) return MTicket();
    APP_ASSERT(len == triangles);
    return MTicket (off);
}

void ClutterBuffer::Section::releaseGeometry (MTicket &t, Ogre::SubMesh *sm)
{
    Ogre::IndexData* idata = sm->indexData;
    int triangles = idata->indexCount / 3;
    releaseTriangles(t.offset, triangles);
    t.offset= 0xFFFFFFFF;
}

void ClutterBuffer::Section::updateGeometry (const MTicket &t,
                                             const Ogre::SubMesh *sm,
                                             const Ogre::Vector3 &position,
                                             const Ogre::Quaternion &orientation,
                                             float vis)
{
    Ogre::IndexData* idata = sm->indexData;
    unsigned triangles = idata->indexCount / 3;
    Ogre::VertexData* vdata = sm->useSharedVertices ? sm->parent->sharedVertexData
                                                    : sm->vertexData;
    Ogre::VertexDeclaration *vdecl = vdata->vertexDeclaration;

    const Ogre::VertexElement *vel_pos = vdecl->findElementBySemantic(Ogre::VES_POSITION);
    APP_ASSERT(vel_pos->getType() == Ogre::VET_FLOAT3);
    const Ogre::VertexElement *vel_norm = vdecl->findElementBySemantic(Ogre::VES_NORMAL);
    APP_ASSERT(vel_norm->getType() == Ogre::VET_FLOAT3);
    const Ogre::VertexElement *vel_uv= vdecl->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);
    APP_ASSERT(vel_uv->getType() == Ogre::VET_FLOAT2);
    const Ogre::VertexElement *vel_tang = NULL; //initialise to avoid warning
    if (mParent->mTangents) {
        vel_tang = vdecl->findElementBySemantic (Ogre::VES_TANGENT);
        APP_ASSERT(vel_tang==NULL || vel_tang->getType()==Ogre::VET_FLOAT3);
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

        Ogre::Vector3 tang = Ogre::Vector3(0,0,0);
        if (vel_tang!=NULL) {
            memcpy(&tang.x, &the_vbuf[vo + vel_tang->getOffset()], vel_tang->getSize());
            tang = orientation * tang;
        }

        Ogre::Vector2 uv;
        memcpy(&uv.x, &the_vbuf[vo + vel_uv->getOffset()], vel_uv->getSize());

        unsigned vi = (i + t.offset*3) * mDeclSize;
        memcpy(&data[vi + 0*sizeof(float)],  &pos.x, 3*sizeof(float));
        memcpy(&data[vi + 3*sizeof(float)], &norm.x, 3*sizeof(float));
        memcpy(&data[vi + 6*sizeof(float)],   &uv.x, 2*sizeof(float));
        memcpy(&data[vi + 8*sizeof(float)],    &vis, 1*sizeof(float));
        if (mParent->mTangents) {
            memcpy(&data[vi + 9*sizeof(float)], &tang.x, 3*sizeof(float));
        }
    }

    vbuf->unlock();
    ibuf->unlock();

    mVertexData.vertexBufferBinding->getBuffer(0)
        ->writeData(t.offset*3*mDeclSize, triangles*3*mDeclSize, &data[t.offset*3*mDeclSize]);
}



ClutterBuffer::Section::QTicket ClutterBuffer::Section::reserveQuad (void)
{
    unsigned off, len;
    reserveTriangles(2, off, len);
    if (len==0) return QTicket();
    return QTicket(off);
}

void ClutterBuffer::Section::releaseQuad (QTicket &t)
{
    releaseTriangles(t.offset,2);
    t.offset = 0xFFFFFFFF;
}

void ClutterBuffer::Section::updateQuad (const QTicket &t,
                                         const Ogre::Vector3 (&pos)[4],
                                         const Ogre::Vector3 (&norm)[4],
                                         const Ogre::Vector2 (&uv)[4],
                                         const Ogre::Vector3 (*tang)[4],
                                         float vis)
{
    const int idxs[6] = { 0, 3, 1, 0, 2, 3 };
    if (mParent->mTangents) {
        APP_ASSERT(mDeclSize == 12*sizeof(float));
    } else {
        APP_ASSERT(mDeclSize == 9*sizeof(float));
    }
    for (unsigned i=0 ; i<6 ; ++i) {
        unsigned vi = (i + t.offset*3) * mDeclSize;
        //std::cout << t.offset << " " << vi << std::endl;
        memcpy(&data[vi + 0*sizeof(float)],  &pos[idxs[i]].x, 3*sizeof(float));
        memcpy(&data[vi + 3*sizeof(float)], &norm[idxs[i]].x, 3*sizeof(float));
        memcpy(&data[vi + 6*sizeof(float)],   &uv[idxs[i]].x, 2*sizeof(float));
        memcpy(&data[vi + 8*sizeof(float)],             &vis, 1*sizeof(float));
        if (mParent->mTangents) {
            memcpy(&data[vi + 9*sizeof(float)], &(*tang)[idxs[i]].x, 3*sizeof(float));
        }
    }

    mVertexData.vertexBufferBinding->getBuffer(0)
        ->writeData(t.offset*3*mDeclSize, 2*3*mDeclSize, &data[t.offset*3*mDeclSize]);
}

void ClutterBuffer::Section::accumulateUtilisation (size_t &used, size_t &rendered, size_t &total)
{
    used += usedTriangles;
    rendered += (last==0 && first==0) ? 0 : (last-first+1);
    total += usage.size();
}

typedef ClutterBuffer::SectionMap::const_iterator I;

void ClutterBuffer::getUtilisation (size_t &used, size_t &rendered, size_t &total)
{
    for (I i=sects.begin(),i_=sects.end() ; i!=i_ ; ++i) {
        Section &s = *i->second;
        s.accumulateUtilisation(used,rendered,total);
    }
}





////////////////////////////////////////////////////////////////////////////////////////////////////
// MovableClutter //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableClutter::visitRenderables (Ogre::Renderable::Visitor *visitor, bool debug)
{
    (void) debug;
    for (I i=clutter.getSections().begin(),i_=clutter.getSections().end() ; i!=i_ ; ++i) {
        visitor->visit(i->second, 0, false);
    }
}


void MovableClutter::_updateRenderQueue (Ogre::RenderQueue *queue)
{
    for (I i=clutter.getSections().begin(),i_=clutter.getSections().end() ; i!=i_ ; ++i) {
        ClutterBuffer::Section &s = *i->second;
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




////////////////////////////////////////////////////////////////////////////////////////////////////
// MovableClutterFactory ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Ogre::MovableObject* MovableClutterFactory::createInstanceImpl (
        const Ogre::String& name, const Ogre::NameValuePairList* params)
{
    Ogre::NameValuePairList::const_iterator ni;

    if (params==0 || (ni = params->find("triangles"))==params->end())
        GRIT_EXCEPT("Must give triangles when creating MovableClutter object");
    unsigned triangles = atoi(ni->second.c_str());

    bool tangents = false; // optional
    if (params>0 && (ni = params->find("tangents"))!=params->end()) {
        tangents = ni->second == "true";
    }

    // must have mesh parameter
    return OGRE_NEW MovableClutter(name, triangles, tangents);
}


void MovableClutterFactory::destroyInstance (Ogre::MovableObject* obj)
{
    OGRE_DELETE obj;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// RangedClutter ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef ClutterBuffer::SectionMap::const_iterator I;

float RangedClutter::Item::calcFade (float range2)
{
    const float out = streamer->fadeOutFactor;
    float range = ::sqrtf(range2);
                    
    float fade = 1.0;       

    if (range > out) {
            fade = (1-range) / (1-out);
    }           

    return fade;
}

void RangedClutter::visitRenderables (Ogre::Renderable::Visitor *visitor, bool debug)
{
    (void) debug;
    for (I i=mClutter.getSections().begin(),i_=mClutter.getSections().end() ; i!=i_ ; ++i) {
        visitor->visit(i->second, 0, false);
    }
}

void RangedClutter::registerMe (void)
{
    streamer->registerUpdateHook(this);
}

void RangedClutter::unregisterMe (void)
{
    streamer->unregisterUpdateHook(this);
}

void RangedClutter::_updateRenderQueue (Ogre::RenderQueue *queue)
{
    for (I i=mClutter.getSections().begin(),i_=mClutter.getSections().end() ; i!=i_ ; ++i) {
        ClutterBuffer::Section &s = *i->second;
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

void RangedClutter::update (const Vector3 &new_pos)
{
    const float vis2 = mVisibility * mVisibility;
    typedef Cargo::iterator I;

    // iterate through all activated guys to see who is too far to stay activated
    Cargo victims = activated;
    for (I i=victims.begin(), i_=victims.end() ; i!=i_ ; ++i) {
        Item *o = *i;
         //note we use vis2 not visibility
        float range2 = o->range2(new_pos) / vis2;

        if (range2 > 1) {
            // now out of range
            mClutter.releaseGeometry(o->ticket);
            o->activated = false;
            Item *filler = activated[activated.size()-1];
            activated[o->activatedIndex] = filler;
            filler->activatedIndex = o->activatedIndex;
            activated.pop_back();
        } else {
            // still in range, update visibility
            float fade = o->calcFade(range2);
            if (fade!=o->lastFade) {
                mClutter.updateGeometry(o->ticket, to_ogre(o->pos), to_ogre(o->quat), fade);
                o->lastFade = fade;
            }
        }
    }

    Cargo cargo;
    mSpace.getPresent(new_pos.x, new_pos.y, new_pos.z, mStepSize, mVisibility, cargo);
    // iterate through the cargo to see who needs to become activated
    for (Cargo::iterator i=cargo.begin(),i_=cargo.end() ; i!=i_ ; ++i) {
        Item *o = *i;

        if (o->activated) continue;

        float range2 = o->range2(new_pos) / vis2;

        // not in range yet
        if (range2 > 1) continue;

        float fade = o->calcFade(range2);
        o->lastFade = fade;

        //activate o
        o->ticket = mClutter.reserveGeometry(o->mesh);
        if (!o->ticket.valid()) continue;
        mClutter.updateGeometry(o->ticket, to_ogre(o->pos), to_ogre(o->quat), fade);
        o->activatedIndex = activated.size();
        activated.push_back(o);
        o->activated = true;
    }
}

void RangedClutter::push_back (const Transform &t)
{
    // it is very important that the number of samples does not increase over time, or
    // the vector will resize and these pointers will become invalid
    items.push_back(Item());
    Item &item = items[items.size()-1];
    item.parent = this;
    item.activated = false;
    item.mesh = mNextMesh;
    item.quat = Quaternion(t.r.w, t.r.x, t.r.y, t.r.z);
    item.lastFade = 0;
    mSpace.add(&item);
    item.updateSphere(t.p, mItemRenderingDistance);
}

void RangedClutter::getUtilisation (size_t &used, size_t &rendered, size_t &total)
{
    mClutter.getUtilisation(used,rendered,total);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// RangedClutterFactory ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Ogre::MovableObject* RangedClutterFactory::createInstanceImpl (
        const Ogre::String& name, const Ogre::NameValuePairList* params)
{
    Ogre::NameValuePairList::const_iterator ni;

    if (params==0 || (ni = params->find("triangles"))==params->end())
        GRIT_EXCEPT("Must give triangles when creating RangedClutter object");
    unsigned triangles = atoi(ni->second.c_str());

    bool tangents = false; // optional
    if (params>0 && (ni = params->find("tangents"))!=params->end()) {
        tangents = ni->second == "true";
    }

    // must have mesh parameter
    return OGRE_NEW RangedClutter(name, triangles, tangents);
}


void RangedClutterFactory::destroyInstance (Ogre::MovableObject* obj)
{
    OGRE_DELETE obj;
}

// pim: ts=4:sw=4:expandtab
