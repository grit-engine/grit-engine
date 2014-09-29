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


GfxSkyBody::GfxSkyBody (GfxMeshDiskResource *gdr, short z_order)
  : dead(false),
    enabled(true),
    zOrder(z_order),
    orientation(1,0,0,0)
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
    DiskResource *dr = disk_resource_get(mesh_name);
    if (dr==NULL) GRIT_EXCEPT("No such resource: \""+mesh_name+"\"");

    if (!dr->isLoaded()) GRIT_EXCEPT("Resource not yet loaded: \""+mesh_name+"\"");

    GfxMeshDiskResource *gdr = dynamic_cast<GfxMeshDiskResource*>(dr);
    if (gdr==NULL) GRIT_EXCEPT("Resource is not a mesh: \""+mesh_name+"\"");

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

void GfxSkyBody::render (GfxPipeline *p)
{
    if (!enabled) return;

    Ogre::Matrix3 rot;
    to_ogre(orientation).ToRotationMatrix(rot);

    Ogre::Matrix4 world(rot);

    Ogre::Matrix4 view = p->getCamera()->getViewMatrix();
    // do not want translation part of view matrix (i.e. just camera direction)
    view.setTrans(Ogre::Vector3(0,0,0)); 

    Ogre::Viewport *viewport = p->getCamera()->getViewport();
    float render_target_flipping = viewport->getTarget()->requiresTextureFlipping() ? -1.0f : 1.0f;
    Ogre::Matrix4 proj = p->getCamera()->getProjectionMatrixWithRSDepth();
    // Invert transformed y if necessary
    proj[1][0] *= render_target_flipping;
    proj[1][1] *= render_target_flipping;
    proj[1][2] *= render_target_flipping;
    proj[1][3] *= render_target_flipping;

    Vector2 viewport_dim(viewport->getActualWidth(), viewport->getActualHeight());


    // this is to protect the material to texture mappings which might be
    // concurrently read by the background loading thread
    // TODO: use a RW lock to avoid stalls
    GFX_MAT_SYNC;

    // iterate through the materials
    for (unsigned i=0 ; i<materials.size() ; ++i) {
        GfxSkyMaterial *mat = materials[i];

        Ogre::SubMesh *sm = mesh->getSubMesh(i);

        // render sm using mat

        GfxShader *shader = mat->getShader()->getShader();

        shader->bindShader();
        shader->bind(mat->getBindings());
        shader->bindGlobals(world, view, proj, viewport_dim);
        shader->bindShaderParams();

        const GfxShaderParamMap &params = shader->getParams();
        const GfxSkyMaterialTextureMap &mat_texs = mat->getTextures();

        unsigned counter = 0;
        for (const auto &pair : params) {
            const std::string &name = pair.first;
            const GfxShaderParam &param = pair.second;
            if (!gfx_gasoline_param_is_texture(param.t)) continue;

            const GfxSkyMaterialTexture *sky_tex = NULL;
            auto it = mat_texs.find(name);
            // material might leave a given uniform undefined in which case revert to shader default
            if (it != mat_texs.end()) sky_tex = &it->second;

            switch (param.t) {
                case GFX_GSL_FLOAT_TEXTURE1: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;
                case GFX_GSL_FLOAT_TEXTURE2: {
                    // TODO: use this to pick a shader!
                    APP_ASSERT(sky_tex != NULL);
                    ogre_rs->_setTexture(counter, true, sky_tex->texture->getOgreTexturePtr());
                    //ogre_rs->_setTextureLayerAnisotropy(counter, sky_tex->anisotropy);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_ANISOTROPIC);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_ANISOTROPIC);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_LINEAR);
                    auto mode = sky_tex->clamp ? Ogre::TextureUnitState::TAM_CLAMP
                                               : Ogre::TextureUnitState::TAM_WRAP;
                    Ogre::TextureUnitState::UVWAddressingMode am;
                    am.u = mode;
                    am.v = mode;
                    am.w = mode;
                    ogre_rs->_setTextureAddressingMode(counter, am);
                    counter++;
                }
                break;
                case GFX_GSL_FLOAT_TEXTURE3: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;
                case GFX_GSL_FLOAT_TEXTURE4: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;

                case GFX_GSL_FLOAT1:
                case GFX_GSL_FLOAT2:
                case GFX_GSL_FLOAT3:
                case GFX_GSL_FLOAT4:
                case GFX_GSL_INT1:
                case GFX_GSL_INT2:
                case GFX_GSL_INT3:
                case GFX_GSL_INT4:
                break;
            }
        }

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

        Ogre::RenderOperation op;    
        sm->_getRenderOperation(op);
        ogre_rs->_render(op);

        for (unsigned i=0 ; i<counter ; ++i) {
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
    // sort by z order into separate container
    std::vector<GfxSkyBody*> sorted;
    for (unsigned i=0 ; i<gfx_all_sky_bodies.size() ; ++i) {
        sorted.push_back(gfx_all_sky_bodies[i]);
    }
    std::sort(sorted.begin(), sorted.end(), sky_body_compare);

    for (unsigned i=0 ; i<sorted.size() ; ++i) {
        sorted[i]->render(p);
    }
}

