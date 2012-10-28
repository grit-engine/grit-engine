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
#include "GfxPipeline.h"

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


GfxSkyBody::GfxSkyBody (GfxDiskResource *gdr, short z_order)
  : dead(false),
    enabled(true),
    zOrder(z_order),
    orientation(1,0,0,0)
{
    const Ogre::ResourcePtr &rp = gdr->getOgreResourcePtr();

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

    GfxDiskResource *gdr = dynamic_cast<GfxDiskResource*>(dr);
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

template<class T> static void hack_set_constant(GfxSkyShader *shader, const std::string &name, const T&v)
{
    try_set_named_constant(shader->getVP(), name, v, true);
    try_set_named_constant(shader->getFP(), name, v, true);
}
static void hack_set_constant(GfxSkyShader *shader, const std::string &name, const Vector3 &v)
{
    Ogre::Vector3 v2 = to_ogre(v);
    hack_set_constant(shader,name,v2);
}

void GfxSkyBody::render (GfxPipeline *p)
{
    if (!enabled) return;

    Ogre::Viewport *viewport = p->getCamera()->getViewport();

    float render_target_flipping = viewport->getTarget()->requiresTextureFlipping() ? -1.0f : 1.0f;

    Ogre::Matrix3 rot;
    to_ogre(orientation).ToRotationMatrix(rot);
    Ogre::Matrix4 world(rot);
    Ogre::Matrix4 view = p->getCamera()->getViewMatrix();
    view.setTrans(Ogre::Vector3(0,0,0)); // do not want translation part of view matrix (i.e. just camera direction)
    Ogre::Matrix4 proj = p->getCamera()->getProjectionMatrixWithRSDepth();
    // Invert transformed y if necessary
    proj[1][0] *= render_target_flipping;
    proj[1][1] *= render_target_flipping;
    proj[1][2] *= render_target_flipping;
    proj[1][3] *= render_target_flipping;
    Ogre::Matrix4 world_view = view * world;
    Ogre::Matrix4 view_proj = proj * view;
    Ogre::Matrix4 world_view_proj = proj * view * world;

    Ogre::Vector4 viewport_size(     viewport->getActualWidth(),      viewport->getActualHeight(),
                                1.0f/viewport->getActualWidth(), 1.0f/viewport->getActualHeight());

    float fov_y = p->getCameraOpts().fovY;


    GFX_MAT_SYNC; // this is to protect the material to texture mappings which might be concurrently read by the background loading thread
    // TODO: use a RW lock to avoid stalls

    // iterate through the materials
    for (unsigned i=0 ; i<materials.size() ; ++i) {
        GfxSkyMaterial *mat = materials[i];

        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        // render this submesh using mat

        GfxSkyShader *shader = mat->getShader();

        // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
        ogre_rs->bindGpuProgram(shader->getFP()->_getBindingDelegate());
        ogre_rs->bindGpuProgram(shader->getVP()->_getBindingDelegate());

        const GfxSkyShaderUniformMap &s_unis = shader->getUniforms();
        const GfxSkyMaterialUniformMap &m_unis = mat->getUniforms();

        unsigned counter = 0;
        for (GfxSkyShaderUniformMap::const_iterator i=s_unis.begin(), i_=s_unis.end() ; i!=i_ ; ++i) {
            const std::string &name = i->first;
            const GfxSkyShaderUniform *s_uni = &i->second;
            const GfxSkyMaterialUniform *m_uni = NULL;
            GfxSkyMaterialUniformMap::const_iterator m_uni_it = m_unis.find(name);
            if (m_uni_it != m_unis.end()) m_uni = &m_uni_it->second; // material might leave a given uniform undefined in which case revert to shader default

            APP_ASSERT(m_uni==NULL || m_uni->kind == s_uni->kind);
            
            switch (s_uni->kind) {
                case GFX_SHADER_UNIFORM_KIND_PARAM_TEXTURE2D: {
                    APP_ASSERT(m_uni != NULL); // todo: use this to pick a shader!
                    ogre_rs->_setTexture(counter, true, m_uni->texture->getOgreResourcePtr());
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_ANISOTROPIC);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_ANISOTROPIC);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_LINEAR);
                    Ogre::TextureUnitState::UVWAddressingMode am;
                    am.u = Ogre::TextureUnitState::TAM_WRAP;
                    am.v = Ogre::TextureUnitState::TAM_WRAP;
                    am.w = Ogre::TextureUnitState::TAM_WRAP;
                    ogre_rs->_setTextureAddressingMode(counter, am);
                    counter++;
                }
                break;
                case GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT1: {
                    float v1 = m_uni == NULL ? s_uni->defaults[0] : m_uni->values[0];
                    float v = v1;
                    hack_set_constant(shader, "mu_"+name, v);
                }
                break;
/*
                case GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT2: {
                    float v1 = m_uni == NULL ? s_uni->defaults[0] : m_uni->values[0];
                    float v2 = m_uni == NULL ? s_uni->defaults[1] : m_uni->values[1];
                    Ogre::Vector2 v(v1,v2);
                    try_set_named_constant(shader->getFP(), "mu_"+name, v, true);
                    try_set_named_constant(shader->getVP(), "mu_"+name, v, true);
                }
                break;
*/
                case GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT3: {
                    float v1 = m_uni == NULL ? s_uni->defaults[0] : m_uni->values[0];
                    float v2 = m_uni == NULL ? s_uni->defaults[1] : m_uni->values[1];
                    float v3 = m_uni == NULL ? s_uni->defaults[2] : m_uni->values[2];
                    Ogre::Vector3 v(v1,v2,v3);
                    hack_set_constant(shader, "mu_"+name, v);
                }
                break;
                case GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT4: {
                    float v1 = m_uni == NULL ? s_uni->defaults[0] : m_uni->values[0];
                    float v2 = m_uni == NULL ? s_uni->defaults[1] : m_uni->values[1];
                    float v3 = m_uni == NULL ? s_uni->defaults[2] : m_uni->values[2];
                    float v4 = m_uni == NULL ? s_uni->defaults[3] : m_uni->values[3];
                    Ogre::Vector4 v(v1,v2,v3,v4);
                    hack_set_constant(shader, "mu_"+name, v);
                }
                break;
            }
        }

        hack_set_constant(shader, "su_world", world);
        hack_set_constant(shader, "su_view", view);
        hack_set_constant(shader, "su_proj", proj);
        hack_set_constant(shader, "su_worldView", world_view);
        hack_set_constant(shader, "su_viewProj", view_proj);
        hack_set_constant(shader, "su_worldViewProj", world_view_proj);
        hack_set_constant(shader, "su_viewportSize", viewport_size);
        hack_set_constant(shader, "su_fovY", fov_y);
        hack_set_constant(shader, "su_time", anim_time); // FIXME:

        hack_set_constant(shader, "su_particleAmbient", particle_ambient);
        hack_set_constant(shader, "su_sunlightDiffuse", gfx_sunlight_diffuse());
        hack_set_constant(shader, "su_sunlightSpecular", gfx_sunlight_specular());
        hack_set_constant(shader, "su_sunlightDirection", gfx_sunlight_direction());
        hack_set_constant(shader, "su_fogColour", fog_colour);
        hack_set_constant(shader, "su_fogDensity", fog_density);
        hack_set_constant(shader, "su_sunDirection", sun_direction);
        hack_set_constant(shader, "su_sunColour", sun_colour);
        hack_set_constant(shader, "su_sunSize", sun_size);
        hack_set_constant(shader, "su_sunFalloffDistance", sun_falloff_distance);
        hack_set_constant(shader, "su_skyGlareSunDistance", sky_glare_sun_distance);
        hack_set_constant(shader, "su_skyGlareHorizonElevation", sky_glare_horizon_elevation);
        hack_set_constant(shader, "su_skyCloudColour", sky_cloud_colour);
        hack_set_constant(shader, "su_skyCloudCoverage", sky_cloud_coverage);
        hack_set_constant(shader, "su_hellColour", hell_colour);

        hack_set_constant(shader, "su_skyDivider1", sky_divider[0]);
        hack_set_constant(shader, "su_skyDivider2", sky_divider[1]);
        hack_set_constant(shader, "su_skyDivider3", sky_divider[2]);
        hack_set_constant(shader, "su_skyDivider4", sky_divider[3]);

        hack_set_constant(shader, "su_skyColour0", sky_colour[0]);
        hack_set_constant(shader, "su_skyColour1", sky_colour[1]);
        hack_set_constant(shader, "su_skyColour2", sky_colour[2]);
        hack_set_constant(shader, "su_skyColour3", sky_colour[3]);
        hack_set_constant(shader, "su_skyColour4", sky_colour[4]);
        hack_set_constant(shader, "su_skyColour5", sky_colour[5]);

        hack_set_constant(shader, "su_skySunColour0", sky_sun_colour[0]);
        hack_set_constant(shader, "su_skySunColour1", sky_sun_colour[1]);
        hack_set_constant(shader, "su_skySunColour2", sky_sun_colour[2]);
        hack_set_constant(shader, "su_skySunColour3", sky_sun_colour[3]);
        hack_set_constant(shader, "su_skySunColour4", sky_sun_colour[4]);



        ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, shader->getFP()->getDefaultParameters(), Ogre::GPV_ALL);
        ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, shader->getVP()->getDefaultParameters(), Ogre::GPV_ALL);

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

