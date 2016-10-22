/* Copyright (c) The Grit Game Engine authors 2016
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

#include "gfx_internal.h"
#include "gfx_sky_body.h"
#include "gfx_sky_material.h"
#include "gfx_pipeline.h"

fast_erase_vector<GfxSkyBody*> gfx_all_sky_bodies;

const std::string GfxSkyBody::className = "GfxSkyBody";

static void validate_sky_mesh (const Ogre::MeshPtr &mesh)
{
    mesh->load();
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


GfxSkyBody::GfxSkyBody (const DiskResourcePtr<GfxMeshDiskResource> &gdr, short z_order)
  : dead(false),
    enabled(true),
    zOrder(z_order),
    orientation(1,0,0,0),
    gdr(gdr)
{
    const Ogre::MeshPtr &rp = gdr->getOgreMeshPtr();

    mesh = rp;

    reinitialise();

    gfx_all_sky_bodies.push_back(this);
}

void GfxSkyBody::reinitialise()
{
    // must set materials here!
    validate_sky_mesh(mesh);
    materials = std::vector<GfxSkyMaterial*>(mesh->getNumSubMeshes());
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        std::string matname = sm->getMaterialName();
        materials[i] = gfx_sky_material_get(matname);
    }
}

GfxSkyBody::~GfxSkyBody (void)
{
    if (!dead) destroy();
}

void GfxSkyBody::destroy (void)
{
    dead = true;
    gfx_all_sky_bodies.erase(this);
}

GfxSkyBodyPtr GfxSkyBody::make (const std::string &mesh_name, short z_order)
{
    auto gdr = disk_resource_use<GfxMeshDiskResource>(mesh_name);
    if (gdr == nullptr) GRIT_EXCEPT("Resource is not a mesh: \"" + mesh_name + "\"");
    return GfxSkyBodyPtr(new GfxSkyBody(gdr, z_order));
}

Quaternion GfxSkyBody::getOrientation (void)
{
    if (dead) THROW_DEAD(className);
    return orientation;
}
void GfxSkyBody::setOrientation (const Quaternion &q)
{
    if (dead) THROW_DEAD(className);
    orientation = q;
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
}

GfxSkyMaterial *GfxSkyBody::getMaterial (unsigned i)
{
    if (i >= materials.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return materials[i];
}

unsigned GfxSkyBody::getSubMeshByOriginalMaterialName (const std::string &n)
{
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        if (sm->getMaterialName() == n) {
            return i;
        }
    }
    CERR << "Mesh did not contain material \""<<n<<"\"" <<std::endl;
    return 0;
}

void GfxSkyBody::setMaterial (unsigned i, GfxSkyMaterial *m)
{
    if (i >= materials.size()) GRIT_EXCEPT("Submesh id out of range. ");
    if (materials[i] == m) return;
    materials[i] = m;
}



void GfxSkyBody::render (const GfxShaderGlobals &g)
{
    if (!enabled) return;

    Ogre::Matrix3 rot;
    to_ogre(orientation).ToRotationMatrix(rot);

    Ogre::Matrix4 world(rot);



    // this is to protect the material to texture mappings which might be
    // concurrently read by the background loading thread
    // TODO: use a RW lock to avoid stalls
    GFX_MAT_SYNC;

    // iterate through the materials
    for (unsigned i=0 ; i<materials.size() ; ++i) {

        GfxSkyMaterial *mat = materials[i];
        Ogre::SubMesh *sm = mesh->getSubMesh(i);

        // render sm using mat
 
        const GfxTextureStateMap &mat_texs = mat->getTextures();
        mat->getShader()->bindShader(GFX_GSL_PURPOSE_SKY, 0, false, false,
                                     g, world, nullptr, 0, 1, mat_texs, mat->getBindings());

        ogre_rs->_setCullingMode(Ogre::CULL_NONE);
        // read but don't write depth buffer
        ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_LESS_EQUAL);
        switch (mat->getSceneBlend()) {
            case GFX_SKY_MATERIAL_OPAQUE:
            ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
            break;
            case GFX_SKY_MATERIAL_ALPHA:
            ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
            break;
            case GFX_SKY_MATERIAL_ADD:
            ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);
            break;
        }

        ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
        ogre_rs->setStencilCheckEnabled(false);
        ogre_rs->_setDepthBias(0, 0);

        Ogre::RenderOperation op;    
        sm->_getRenderOperation(op);
        ogre_rs->_render(op);

        for (unsigned i=0 ; i<mat_texs.size() ; ++i) {
            ogre_rs->_disableTextureUnit(i);
        }

    }
}

static bool sky_body_compare (GfxSkyBody *a, GfxSkyBody *b)
{
    // furthest particle first
    return a->getZOrder() > b->getZOrder();
}


// called every frame
void gfx_sky_render (GfxPipeline *p)
{
    GfxShaderGlobals g = gfx_shader_globals_cam(p);
    g.view.setTrans(Ogre::Vector3(0,0,0)); 

    // sort by z order into separate container
    std::vector<GfxSkyBody*> sorted;
    for (unsigned i=0 ; i<gfx_all_sky_bodies.size() ; ++i) {
        sorted.push_back(gfx_all_sky_bodies[i]);
    }
    std::sort(sorted.begin(), sorted.end(), sky_body_compare);

    for (unsigned i=0 ; i<sorted.size() ; ++i) {
        sorted[i]->render(g);
    }
}

