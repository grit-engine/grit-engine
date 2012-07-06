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


#include <OgreMeshManager.h>

#include "gfx_internal.h"
#include "GfxSkyBody.h"
#include "GfxSkyMaterial.h"

fast_erase_vector<GfxSkyBody*> gfx_all_sky_bodies;

const std::string GfxSkyBody::className = "GfxSkyBody";

static void validate_sky_mesh (const Ogre::MeshPtr &mesh)
{
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        std::string matname = sm->getMaterialName();
        if (!gfx_sky_material_has(matname)) {
            CERR << "Mesh \"/"<<mesh->getName()<<"\" references non-existing material "
                 << "\""<<matname<<"\""<<std::endl;
            matname = "/system/FallbackMaterial";
            sm->setMaterialName(matname);
        }
    }
}


GfxSkyBody::GfxSkyBody (GfxDiskResource *gdr, short z_order)
  : dead(false),
    enabled(true),
    zOrder(z_order),
    ent(NULL),
    node(NULL)
{
    const Ogre::ResourcePtr &rp = gdr->getOgreResourcePtr();

    mesh = rp;

    reinitialise();

    gfx_all_sky_bodies.push_back(this);
}

void GfxSkyBody::reinitialise()
{
    if (node != NULL) ogre_sm->destroySceneNode(node->getName());
    node = ogre_sky_node->createChildSceneNode();

    if (ent != NULL) ogre_sm->destroyEntity(ent);
    ent = ogre_sm->createEntity(freshname(), mesh->getName());
    ent->setCastShadows(false);
    node->attachObject(ent);

    // must set materials here!
    validate_sky_mesh(mesh);
    materials = std::vector<GfxSkyMaterial*>(mesh->getNumSubMeshes());
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        std::string matname = sm->getMaterialName();
        materials[i] = gfx_sky_material_get(matname);
    }

    updateProperties();
}

GfxSkyBody::~GfxSkyBody (void)
{
    if (!dead) CERR << className+" has not been destroyed properly." << std::endl;
}

void GfxSkyBody::destroy (void)
{
    ogre_sm->destroyEntity(ent);
    ent = NULL;
    ogre_sm->destroySceneNode(node->getName());
    node = NULL;
    dead = true;
    gfx_all_sky_bodies.erase(this);
}

GfxSkyBodyPtr GfxSkyBody::make (const std::string &mesh_name, short z_order)
{
    DiskResource *dr = disk_resource_get(mesh_name);
    if (dr==NULL) GRIT_EXCEPT("No such resource: \""+mesh_name+"\"");

    if (!dr->isLoaded()) GRIT_EXCEPT("Resource not yet loaded: \""+mesh_name+"\"");

    GfxDiskResource *gdr = dynamic_cast<GfxDiskResource*>(dr);
    if (gdr==NULL) GRIT_EXCEPT("Resource is not a mesh: \""+mesh_name+"\"");

    return GfxSkyBodyPtr(new GfxSkyBody(gdr, z_order));
}

Quaternion GfxSkyBody::getOrientation (void)
{
    if (dead) THROW_DEAD(className);
    return from_ogre(node->getOrientation());
}
void GfxSkyBody::setOrientation (const Quaternion &q)
{
    if (dead) THROW_DEAD(className);
    return node->setOrientation(to_ogre(q));
}

unsigned char GfxSkyBody::getZOrder (void)
{
    if (dead) THROW_DEAD(className);
    return zOrder;
}
void GfxSkyBody::setZOrder (unsigned char v)
{
    if (dead) THROW_DEAD(className);
    zOrder = v;
    updateProperties();
}

bool GfxSkyBody::isEnabled (void)
{
    if (dead) THROW_DEAD(className);
    return enabled;
}

void GfxSkyBody::setEnabled (bool v)
{
    if (dead) THROW_DEAD(className);
    enabled = v;
    ent->setVisible(enabled);
}

GfxSkyMaterial *GfxSkyBody::getMaterial (unsigned i)
{
    if (i >= materials.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return materials[i];
}

void GfxSkyBody::updateProperties (void)
{
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        GfxSkyMaterial *mat = materials[i];
        Ogre::SubEntity *se = ent->getSubEntity(i);
        se->setMaterial(mat->mat);
        if (mat->getSceneBlend() != GFX_SKY_MATERIAL_OPAQUE) {
            se->setRenderQueueGroupAndPriority(RQ_SKY, 65535-short(zOrder));
        } else {
            se->setRenderQueueGroupAndPriority(RQ_SKY, 65535-short(zOrder));
        }
        se->setCustomParameter(0, Ogre::Vector4(zOrder,0,0,0));
    }

}
