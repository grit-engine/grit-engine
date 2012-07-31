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
#include "GfxInstances.h"

const std::string GfxInstances::className = "GfxInstances";

const unsigned instance_data_floats = 13;
const unsigned instance_data_bytes = instance_data_floats*4;

class GfxInstance : public fast_erase_index {
    
};

// One of these for each amterial in the original mesh
class GfxInstances::Section : public Ogre::Renderable {

    GfxInstances *parent;
    Ogre::MaterialPtr mat;
    Ogre::RenderOperation op;
    Ogre::VertexData vdata;

    public:

    Section (GfxInstances *parent, Ogre::MaterialPtr mat, Ogre::SubMesh *sm)
      : parent(parent), mat(mat)
    {
        sm->_getRenderOperation(op, 0);
        op.vertexData = parent->sharedVertexData;
        setNumInstances(0);
    }

    void setNumInstances (unsigned v) { op.numberOfInstances = v; }

    // Renderable stuff

    const Ogre::MaterialPtr& getMaterial (void) const { return mat; }
    void getRenderOperation (Ogre::RenderOperation& o) { o = op; }
    void getWorldTransforms (Ogre::Matrix4* xform) const
    { xform[0] = Ogre::Matrix4::IDENTITY; }
    Ogre::Real getSquaredViewDepth (const Ogre::Camera *) const { return 0; }
    const Ogre::LightList &getLights (void) const { return parent->queryLights(); }

};


GfxInstancesPtr GfxInstances::make (const std::string &mesh_name, const GfxBodyPtr &par_)
{       
    DiskResource *dr = disk_resource_get(mesh_name);
    if (dr==NULL) GRIT_EXCEPT("No such resource: \""+mesh_name+"\"");
    
    if (!dr->isLoaded()) GRIT_EXCEPT("Resource not yet loaded: \""+mesh_name+"\"");
    
    GfxDiskResource *gdr = dynamic_cast<GfxDiskResource*>(dr);
    if (gdr==NULL) GRIT_EXCEPT("Resource is not a mesh: \""+mesh_name+"\"");

    return GfxInstancesPtr(new GfxInstances(gdr, par_));
}

static Ogre::MaterialPtr mat_from_submesh (const Ogre::MeshPtr &mesh, Ogre::SubMesh *sm)
{
    std::string name = sm->getMaterialName();

    // TODO: look up name GfxMaterial database and pull out OGRE material

    name += "&"; // use a different material (in order to get a different shader)
    Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().getByName(name, "GRIT");

    if (m.isNull()) {
        CERR << "Material not found: \"" << sm->getMaterialName() << "\" "
             << "in mesh \"" << mesh->getName() << "\"" << std::endl;
        m = Ogre::MaterialManager::getSingleton().getByName("/system/FallbackMaterial&", "GRIT");
    }
    return m;
}

GfxInstances::GfxInstances (GfxDiskResource *gdr, const GfxBodyPtr &par_)
  : GfxNode(par_),
    dirty(false),
    mBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE),
    mBoundingRadius(std::numeric_limits<float>::max())
{   
    const Ogre::ResourcePtr &rp = gdr->getOgreResourcePtr();
    mesh = rp;

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

    reserve(0);

    numSections = mesh->getNumSubMeshes();
    sections = new Section*[numSections];

    for (unsigned i=0 ; i<numSections ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        Ogre::MaterialPtr mat = mat_from_submesh(mesh, sm);
        sections[i] = new Section(this, mat, sm);
    }

    //gfx_all_bodies.push_back(this);
    //registerMe();
}   
    
void GfxInstances::reserve (unsigned new_capacity)
{
    if (new_capacity < instances.size()) new_capacity = instances.size();
    capacity = new_capacity;

    instBufRaw.reserve(capacity * instance_data_floats);

    if (capacity == 0) {
        instBuf.setNull();
    } else {
        Ogre::HardwareVertexBufferSharedPtr old = instBuf;
        instBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                        instance_data_bytes,
                        capacity,
                        Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
        instBuf->setIsInstanceData(true);
        instBuf->setInstanceDataStepRate(1);
        dirty = true;
    }

    sharedVertexData->vertexBufferBinding->setBinding(1, instBuf);
    instances.reserve(capacity);
}

void GfxInstances::copyToGPU (void)
{
    copyToGPU(0, instances.size(), true);
}

void GfxInstances::copyToGPU (unsigned from, unsigned to, bool discard)
{
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

GfxInstance *GfxInstances::add (const Vector3 &pos, const Quaternion &q, float fade)
{
    if (instances.size()+1 > capacity) reserve(std::max(128u, unsigned(capacity * 1.3)));
    GfxInstance *nu = new GfxInstance();
    instances.push_back(nu);
    instBufRaw.resize(instBufRaw.size() + instance_data_floats);
    for (unsigned i=0 ; i<numSections ; ++i) sections[i]->setNumInstances(instances.size());
    update(nu, pos, q, fade);
    return nu;
}

void GfxInstances::update (GfxInstance *inst, const Vector3 &pos, const Quaternion &q, float fade)
{
    float *base = &instBufRaw[inst->_index * instance_data_floats];
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

void GfxInstances::remove (GfxInstance *inst)
{
    unsigned index = inst->_index;
    instances.erase(inst);
    (void) index;
    // reorganise instance_buffer to move last 13 into this position
    delete inst;
}


/*
void GfxInstances::registerMe (void)
{
    streamer_callback_register(this);
}

void GfxInstances::unregisterMe (void)
{
    streamer_callback_unregister(this);
}

void GfxInstances::update (const Vector3 &new_pos)
{
    (void) new_pos;
}
*/


void GfxInstances::visitRenderables(Ogre::Renderable::Visitor *visitor, bool debug)
{
    (void) debug;
    for (unsigned i=0 ; i<numSections ; ++i)
        visitor->visit(sections[i], 0, false);
}

void GfxInstances::_updateRenderQueue(Ogre::RenderQueue *queue)
{
    if (dirty) {
        copyToGPU();
        dirty = false;
    }
    for (unsigned i=0 ; i<numSections ; ++i) {
        Section *s = sections[i];

        if (mRenderQueuePrioritySet) {
            assert(mRenderQueueIDSet == true);
            queue->addRenderable(s, mRenderQueueID,
                                    mRenderQueuePriority);
        } else if (mRenderQueueIDSet) {
            queue->addRenderable(s, mRenderQueueID,
                                    queue->getDefaultRenderablePriority());
        } else {
            queue->addRenderable(s, queue->getDefaultQueueGroup(),
                                    queue->getDefaultRenderablePriority());
        }
    }
}
