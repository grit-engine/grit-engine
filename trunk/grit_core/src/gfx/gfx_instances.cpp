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

#include <algorithm>

#include "../main.h"

#include "gfx_internal.h"
#include "gfx_material.h"
#include "gfx_instances.h"

const std::string GfxInstances::className = "GfxInstances";

const unsigned instance_data_floats = 13;
const unsigned instance_data_bytes = instance_data_floats*4;

// One of these for each amterial in the original mesh
class GfxInstances::Section : public Ogre::Renderable {

    GfxInstances *parent;
    GfxMaterial *mat;
    Ogre::MaterialPtr omat;
    Ogre::RenderOperation op;

    public:

    Section (GfxInstances *parent, GfxMaterial *mat, Ogre::SubMesh *sm)
      : parent(parent), mat(mat), queueID(0), queuePriority(0)
    {
        sm->_getRenderOperation(op, 0);
        op.vertexData = parent->sharedVertexData;
        setNumInstances(0);
    }

    void setNumInstances (unsigned v) { op.numberOfInstances = v; }

    // Ogre has already used the name setMaterial so be explicit for this one
    GfxMaterial *getGritMaterial (void) { return mat; }

    unsigned char queueID;
    unsigned short queuePriority;
    // Renderable stuff

    void setMaterial (const Ogre::MaterialPtr &m) { omat = m; }

    const Ogre::MaterialPtr& getMaterial (void) const { return omat; }
    void getRenderOperation (Ogre::RenderOperation& o) { o = op; }
    void getWorldTransforms (Ogre::Matrix4* xform) const
    { xform[0] = Ogre::Matrix4::IDENTITY; }
    Ogre::Real getSquaredViewDepth (const Ogre::Camera *) const { return 0; }
    const Ogre::LightList &getLights (void) const { return parent->queryLights(); }

};


GfxInstancesPtr GfxInstances::make (const std::string &mesh_name, const GfxNodePtr &par_)
{       
    DiskResource *dr = disk_resource_get(mesh_name);
    if (dr==NULL) GRIT_EXCEPT("No such resource: \""+mesh_name+"\"");
    
    if (!dr->isLoaded()) GRIT_EXCEPT("Resource not yet loaded: \""+mesh_name+"\"");
    
    GfxMeshDiskResource *gdr = dynamic_cast<GfxMeshDiskResource*>(dr);
    if (gdr==NULL) GRIT_EXCEPT("Resource is not a mesh: \""+mesh_name+"\"");

    return GfxInstancesPtr(new GfxInstances(gdr, par_));
}

GfxInstances::GfxInstances (GfxMeshDiskResource *gdr, const GfxNodePtr &par_)
  : GfxNode(par_),
    dirty(false),
    enabled(true),
    mBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE),
    mBoundingRadius(std::numeric_limits<float>::max())
{   
    mesh = gdr->getOgreMeshPtr();

    node->attachObject(this);

    reinitialise();

    reserveSpace(0);
}   

void GfxInstances::updateSections (void)
{
    mesh->load();

    APP_ASSERT(mesh->sharedVertexData != NULL);
    sharedVertexData = mesh->sharedVertexData->clone(false);

    // add instancing declarations on to the end of it
    Ogre::VertexDeclaration &vdecl = *sharedVertexData->vertexDeclaration;
    unsigned vdecl_inst_sz = 0;
    vdecl_inst_sz += vdecl.addElement(1, vdecl_inst_sz, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 1).getSize();
    vdecl_inst_sz += vdecl.addElement(1, vdecl_inst_sz, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 2).getSize();
    vdecl_inst_sz += vdecl.addElement(1, vdecl_inst_sz, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 3).getSize();
    vdecl_inst_sz += vdecl.addElement(1, vdecl_inst_sz, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 4).getSize();
    vdecl_inst_sz += vdecl.addElement(1, vdecl_inst_sz, Ogre::VET_FLOAT1, Ogre::VES_TEXTURE_COORDINATES, 5).getSize();
    APP_ASSERT(vdecl_inst_sz == instance_data_bytes);

    numSections = mesh->getNumSubMeshes();
    sections = new Section*[numSections];

    for (unsigned i=0 ; i<numSections ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        std::string matname = sm->getMaterialName();
        if (!gfx_material_has(matname)) {
            CERR << "Mesh \"/"<<mesh->getName()<<"\" references non-existing material "
                 << "\""<<matname<<"\""<<std::endl;
            matname = "/system/FallbackMaterial";
            sm->setMaterialName(matname);
        }

        sections[i] = new Section(this, gfx_material_get(matname), sm);
    }

}

void GfxInstances::reinitialise (void)
{
    updateSections();
    updateProperties();
}

void GfxInstances::reserveSpace (unsigned new_capacity)
{
    new_capacity = indexes.reserve(new_capacity);

    instBufRaw.reserve(new_capacity * instance_data_floats);

    instBuf.setNull();
    if (new_capacity > 0) {
        instBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                        instance_data_bytes,
                        new_capacity,
                        Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
        instBuf->setIsInstanceData(true);
        instBuf->setInstanceDataStepRate(1);
        dirty = true; // will be lazily copied from host
    }
    sharedVertexData->vertexBufferBinding->setBinding(1, instBuf);

}

unsigned GfxInstances::getTrianglesPerInstance (void)
{
    unsigned total = 0;
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        total += mesh->getSubMesh(i)->indexData->indexCount/3;
    }
    return total;
}

unsigned GfxInstances::getBatches (void)
{
    return mesh->getNumSubMeshes();
}

void GfxInstances::copyToGPU (void)
{
    copyToGPU(0, indexes.size(), true);
}

void GfxInstances::copyToGPU (unsigned from, unsigned to, bool discard)
{
    if (to - from == 0) return;
    unsigned offset = from * instance_data_bytes;
    unsigned len = (to - from) * instance_data_bytes;
    void *data = &instBufRaw[from * instance_data_floats];
    instBuf->writeData(offset, len, data, discard);
}


GfxInstances::~GfxInstances (void)
{
    if (!dead) destroy();
    //unregisterMe();
    OGRE_DELETE sharedVertexData;
    delete [] sections;
}

void GfxInstances::destroy (void)
{
    if (dead) THROW_DEAD(className);
    GfxNode::destroy();
}

unsigned GfxInstances::add (const Vector3 &pos, const Quaternion &q, float fade)
{
    if (indexes.size() >= indexes.capacity()) reserveSpace(std::max(128u, unsigned(indexes.capacity() * 1.3)));
    unsigned sparse_index = indexes.newSparseIndex();
    instBufRaw.resize(instBufRaw.size() + instance_data_floats);
    for (unsigned i=0 ; i<numSections ; ++i) sections[i]->setNumInstances(indexes.size());
    update(sparse_index, pos, q, fade);
    return sparse_index;
}

void GfxInstances::update (unsigned sparse_index, const Vector3 &pos, const Quaternion &q, float fade)
{
    indexes.sparseIndexValid(sparse_index);
    unsigned dense_index = indexes.denseIndex(sparse_index);

    float *base = &instBufRaw[dense_index * instance_data_floats];
    Ogre::Matrix3 rot;
    to_ogre(q).ToRotationMatrix(rot);
    float *rot_base = rot[0];
    for (int i=0 ; i<9 ; ++i) {
        base[i] = rot_base[i];
    }
    base[ 9] = pos.x;
    base[10] = pos.y;
    base[11] = pos.z;
    base[12] = fade;
    dirty = true;
}

void GfxInstances::del (unsigned sparse_index)
{
    indexes.sparseIndexValid(sparse_index);
    unsigned dense_index = indexes.denseIndex(sparse_index);
    unsigned last = indexes.size()-1;
    indexes.delSparseIndex(sparse_index);

    if (dense_index != last) {
        // reorganise buffer to move last instance into the position of the one just removed
        for (unsigned i=0 ; i<instance_data_floats ; ++i) {
            instBufRaw[dense_index * instance_data_floats + i] = instBufRaw[last * instance_data_floats + i];
        }
    }


    instBufRaw.resize(instance_data_floats * last);
    for (unsigned i=0 ; i<numSections ; ++i) sections[i]->setNumInstances(last);

    dirty = true;
}


// Currently needs to be updated when these material properties change:
// getSceneBlend
// getStipple
void GfxInstances::updateProperties (void)
{
    GFX_MAT_SYNC;

    for (unsigned i=0 ; i<numSections ; ++i) {

        Section *s = sections[i];

        GfxMaterial *gfx_material = s->getGritMaterial();

        s->setMaterial(gfx_material->worldMat);

        switch (gfx_material->getSceneBlend()) {
            case GFX_MATERIAL_OPAQUE:
            s->queueID = RQ_GBUFFER_OPAQUE;
            break;
            case GFX_MATERIAL_ALPHA:
            s->queueID = RQ_FORWARD_ALPHA;
            break;
            case GFX_MATERIAL_ALPHA_DEPTH:
            s->queueID = RQ_FORWARD_ALPHA_DEPTH;
            break;
        }

    }
}

void GfxInstances::visitRenderables(Ogre::Renderable::Visitor *visitor, bool debug)
{
    (void) debug;
    for (unsigned i=0 ; i<numSections ; ++i)
        visitor->visit(sections[i], 0, false);
}

void GfxInstances::_updateRenderQueue(Ogre::RenderQueue *queue)
{
    if (indexes.size() == 0) return;
    if (!enabled) return;
    if (dirty) {
        copyToGPU();
        dirty = false;
    }
    for (unsigned i=0 ; i<numSections ; ++i) {
        Section *s = sections[i];

        queue->addRenderable(s, s->queueID, s->queuePriority);
    }
}

const std::string &GfxInstances::getMeshName (void)
{   
    static std::string empty_string = "";
    return mesh.isNull() ? empty_string : mesh->getName();
}

