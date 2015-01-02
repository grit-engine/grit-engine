/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <string>
#include <algorithm>
#include <map>

#include <math_util.h>

#include "../vect_util.h"

#include "gfx.h"
#include "gfx_internal.h"
#include "gfx_particle_system.h"
#include "gfx_pipeline.h"
#include "gfx_shader.h"

const unsigned cube_vertexes = 8;
static Ogre::HardwareVertexBufferSharedPtr cubeVertexBuffer;
static Ogre::IndexData cubeIndexData;

const unsigned quad_vertexes = 4;
static Ogre::HardwareVertexBufferSharedPtr quadVertexBuffer;
static Ogre::IndexData quadIndexData;

static inline Ogre::HighLevelGpuProgramPtr get_shader(const std::string &name)
{
    return Ogre::HighLevelGpuProgramManager::getSingleton().getByName(name, RESGRP);
}

#define QUAD(a,b,c,d) a, b, d, d, b, c

void gfx_particle_init (void)
{
    {
        // set up the cube geometry
        const unsigned cube_triangles = 12;

        cubeVertexBuffer =
            Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                12, cube_vertexes, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

        float vdata_raw[3*cube_vertexes] = {
            -1, -1, -1,
            -1, -1,  1,
            -1,  1, -1,
            -1,  1,  1,
             1, -1, -1,
             1, -1,  1,
             1,  1, -1,
             1,  1,  1,
        };
        cubeVertexBuffer->writeData(0, 12*cube_vertexes, &vdata_raw[0]);

        // Prepare index buffer
        cubeIndexData.indexBuffer = Ogre::HardwareBufferManager::getSingleton()
            .createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, 3*cube_triangles, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
        cubeIndexData.indexStart = 0;
        cubeIndexData.indexCount = 3*cube_triangles;
        uint16_t idata_raw[] = {
            QUAD(0,4,5,1),
            QUAD(2,0,1,3),
            QUAD(6,2,3,7),
            QUAD(4,6,7,5),
            QUAD(2,6,4,0),
            QUAD(1,5,7,3),
        };
        cubeIndexData.indexBuffer->writeData(cubeIndexData.indexStart, cubeIndexData.indexCount*sizeof(uint16_t), &idata_raw[0]);
    }
    {
        // set up the quad geometry
        const unsigned quad_triangles = 2;

        quadVertexBuffer =
            Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                12, quad_vertexes, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

        float vdata_raw[3*quad_vertexes] = {
            -1, 0, -1,
            -1, 0,  1,
             1, 0, -1,
             1, 0,  1,
        };
        quadVertexBuffer->writeData(0, 12*quad_vertexes, &vdata_raw[0]);

        // Prepare index buffer
        quadIndexData.indexBuffer = Ogre::HardwareBufferManager::getSingleton()
            .createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, 3*quad_triangles, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
        quadIndexData.indexStart = 0;
        quadIndexData.indexCount = 3*quad_triangles;
        uint16_t idata_raw[] = {
            QUAD(0,2,3,1),
        };
        quadIndexData.indexBuffer->writeData(quadIndexData.indexStart, quadIndexData.indexCount*sizeof(uint16_t), &idata_raw[0]);
    }

}


class ParticlesInstanceBuffer {
    Ogre::HardwareVertexBufferSharedPtr instBuf;
    Ogre::RenderOperation renderOp;
    Ogre::VertexData vertexData;
    unsigned instBufVertexSize;

    float *instPtr, *instPtr0;
    unsigned maxInstances;

    unsigned instancesAdded (void) { return ((instPtr - instPtr0)*sizeof(float)) / instBufVertexSize; }


    public:

    ParticlesInstanceBuffer (void)
    {
        // Prepare vertex buffer
        vertexData.vertexStart = 0;
        vertexData.vertexCount = quad_vertexes;

        // Non-instanced data
        unsigned vdecl_size = 0;
        vdecl_size += vertexData.vertexDeclaration->addElement(0, vdecl_size, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
        vertexData.vertexBufferBinding->setBinding(0, quadVertexBuffer);

        // Instanced data
        instBufVertexSize = 0;
        // mat 3xfloat
        instBufVertexSize += vertexData.vertexDeclaration->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 0).getSize();
        // mat 3xfloat
        instBufVertexSize += vertexData.vertexDeclaration->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 1).getSize();
        // mat 3xfloat
        instBufVertexSize += vertexData.vertexDeclaration->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 2).getSize();
        // position 3xfloat
        instBufVertexSize += vertexData.vertexDeclaration->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 3).getSize();
        // colour 3xfloat
        instBufVertexSize += vertexData.vertexDeclaration->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 4).getSize();
        // alpha
        instBufVertexSize += vertexData.vertexDeclaration->addElement(1, instBufVertexSize, Ogre::VET_FLOAT1, Ogre::VES_TEXTURE_COORDINATES, 5).getSize();
        // uv1, uv2
        instBufVertexSize += vertexData.vertexDeclaration->addElement(1, instBufVertexSize, Ogre::VET_FLOAT4, Ogre::VES_TEXTURE_COORDINATES, 6).getSize();

        renderOp.vertexData = &vertexData;
        renderOp.indexData = &quadIndexData;
        renderOp.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        renderOp.useIndexes = true;
        instPtr = NULL;
        instPtr0 = NULL;
        maxInstances = 0;
    }

    void beginParticles (unsigned instances)
    {
        
        if (instances > maxInstances) {
            maxInstances = instances;
            instBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                    instBufVertexSize, instances, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
            instBuf->setIsInstanceData(true);
            instBuf->setInstanceDataStepRate(1);
            vertexData.vertexBufferBinding->setBinding(1, instBuf);
        }

        instPtr0 = instPtr = static_cast<float*>(instBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
    }

    void addParticle (const Vector3 &cam_up, GfxParticle *p)
    {
        // right hand coordinate system -- +Z is towards the viewer
        Vector3 basis_y = p->fromCamNorm;
        Vector3 basis_z = cam_up; // not necessarily perpendicular to basis_z yet
        Vector3 basis_x = basis_y.cross(basis_z); // perp to z
        basis_y = basis_z.cross(basis_x); // perp to z and x

        basis_x *= p->dimensions.x / 2;
        basis_z *= p->dimensions.y / 2;
        basis_y *= p->dimensions.z / 2;

        // rotate around y
        Degree angle(p->angle);
        float sin_angle = gritsin(angle);
        float cos_angle = gritcos(angle);
        Vector3 basis_x2 = cos_angle*basis_x + sin_angle*basis_z;
        Vector3 basis_z2 = cos_angle*basis_z - sin_angle*basis_x;

        std::pair<unsigned, unsigned> tex_size = p->getTextureSize();

        *(instPtr++) = basis_x2.x;
        *(instPtr++) = basis_x2.y;
        *(instPtr++) = basis_x2.z;
        *(instPtr++) = basis_y.x;
        *(instPtr++) = basis_y.y;
        *(instPtr++) = basis_y.z;
        *(instPtr++) = basis_z2.x;
        *(instPtr++) = basis_z2.y;
        *(instPtr++) = basis_z2.z;
        *(instPtr++) = p->pos.x;
        *(instPtr++) = p->pos.y;
        *(instPtr++) = p->pos.z;
        *(instPtr++) = p->colour.x;
        *(instPtr++) = p->colour.y;
        *(instPtr++) = p->colour.z;
        *(instPtr++) = p->alpha;
        *(instPtr++) = p->u1 / tex_size.first;
        *(instPtr++) = p->v1 / tex_size.first;
        *(instPtr++) = p->u2 / tex_size.second;
        *(instPtr++) = p->v2 / tex_size.second;
    }

    void endParticles (void)
    {
        instBuf->unlock();
        renderOp.numberOfInstances = instancesAdded();
    }

    const Ogre::RenderOperation &getRenderOperation (void) { return renderOp; }

    
};


static GfxShader *particle_shaders[] = { nullptr, nullptr, nullptr, nullptr };

// a particle system holds the buffer for particles of a particular material
class GfxParticleSystem {
    fast_erase_vector<GfxParticle*> particles;

    std::string name;
    bool alphaBlend;
    float alphaRej;
    bool emissive;

    Ogre::TexturePtr tex;
    unsigned texHeight;
    unsigned texWidth;

    ParticlesInstanceBuffer outside;

    // every frame, must sort particles and compute new orientations

    public:
    GfxParticleSystem (const std::string &name, std::string texname,
                       bool alpha_blend, float alphaRej, bool emissive)
         : name(name), alphaBlend(alpha_blend), alphaRej(alphaRej), emissive(emissive)
    {
        APP_ASSERT(texname[0] == '/');
        texname = texname.substr(1);
        tex = Ogre::TextureManager::getSingleton().load(texname, RESGRP);
        texHeight = tex->getHeight();
        texWidth = tex->getWidth();
    }

    ~GfxParticleSystem (void)
    {
    }

    GfxParticle *emit (void)
    {
        GfxParticle *nu = new GfxParticle (this);
        particles.push_back(nu);
        return nu;
    }

    void release (GfxParticle *p)
    {
        particles.erase(p);
        delete p;
    }

    static bool particleCompare (GfxParticle *a, GfxParticle *b)
    {
        // furthest particle first
        return a->fromCamDist > b->fromCamDist;
    }

    void render (GfxPipeline *pipe)
    {
        const CameraOpts &cam_opts = pipe->getCameraOpts();
        const Vector3 &cam_pos = cam_opts.pos;
        const Vector3 &cam_up = cam_opts.dir * Vector3(0,0,1);
        Ogre::Viewport *viewport = ogre_sm->getCurrentViewport();
        Ogre::RenderTarget *render_target = viewport->getTarget();

        // PREPARE BUFFERS

        std::vector<GfxParticle*> visible;
        // populate visible
        for (unsigned i=0 ; i<particles.size() ; ++i) {
            GfxParticle *p = particles[i];
            p->preProcess(cam_pos);
            if (false) continue;
            visible.push_back(p);
        }
        if (alphaBlend) {
            std::sort(visible.begin(), visible.end(), particleCompare);
        }

        // early out for nothing to render
        if (visible.size() == 0) return;

        // use particles.size() (an upper bound on visible.size()) to try and avoid frequent reallocation
        outside.beginParticles(particles.size());
        for (unsigned i=0 ; i<visible.size() ; ++i) {
            GfxParticle *p = visible[i];
            outside.addParticle(cam_up, p);
        }
        
        outside.endParticles();


        // ISSUE RENDER COMMANDS
        try {


            // various camera and lighting things
            Ogre::Camera *cam = pipe->getCamera();
            Ogre::Matrix4 view = cam->getViewMatrix();
            Ogre::Matrix4 proj = cam->getProjectionMatrixWithRSDepth();
            Ogre::Matrix4 proj_view = proj*view;

            // corners mapping for worldspace fragment position reconstruction:
            // top-right near, top-left near, bottom-left near, bottom-right near,
            // top-right far, top-left far, bottom-left far, bottom-right far. 
            Ogre::Vector3 top_right_ray = cam->getWorldSpaceCorners()[4] - to_ogre(cam_pos);
            Ogre::Vector3 top_left_ray = cam->getWorldSpaceCorners()[5] - to_ogre(cam_pos);
            Ogre::Vector3 bottom_left_ray = cam->getWorldSpaceCorners()[6] - to_ogre(cam_pos);
            Ogre::Vector3 bottom_right_ray = cam->getWorldSpaceCorners()[7] - to_ogre(cam_pos);

            int suff_index = int(emissive) + 2*int(alphaBlend);

            GfxShader *shader = particle_shaders[suff_index];

            if (shader == nullptr) {
                static const std::string suffixes[] = { "a_e", "a_E", "A_e", "A_E" };

                const std::string shader_name = "particle:"+suffixes[suff_index];

                const std::string vp_name = "particle_v:"+suffixes[suff_index];
                const std::string fp_name = "particle_f:"+suffixes[suff_index];

                Ogre::HighLevelGpuProgramPtr vp = get_shader(vp_name);
                Ogre::HighLevelGpuProgramPtr fp = get_shader(fp_name);

                shader = gfx_shader_make_from_existing(shader_name, vp, fp, {});
                shader->validate();
                particle_shaders[suff_index] = shader;
            }

            Ogre::HighLevelGpuProgramPtr vp = shader->getOgreVertexProgram();
            Ogre::HighLevelGpuProgramPtr fp = shader->getOgreFragmentProgram();

            try_set_named_constant(vp, "view_proj", proj_view);
            float render_target_flipping = render_target->requiresTextureFlipping() ? -1.0f : 1.0f;
            try_set_named_constant(vp, "render_target_flipping", render_target_flipping);
            if (!emissive) {
                try_set_named_constant(vp, "particle_ambient", to_ogre(gfx_particle_ambient()));
            }
            try_set_named_constant(vp, "camera_pos_ws", to_ogre(cam_pos));

            try_set_named_constant(fp, "top_left_ray", top_left_ray);
            try_set_named_constant(fp, "top_right_ray", top_right_ray);
            try_set_named_constant(fp, "bottom_left_ray", bottom_left_ray);
            try_set_named_constant(fp, "bottom_right_ray", bottom_right_ray);


            Ogre::Vector3 the_fog_params(fog_density, 1, global_exposure);
            try_set_named_constant(fp, "the_fog_params", the_fog_params);
            try_set_named_constant(fp, "the_fog_colour", to_ogre(fog_colour));
            try_set_named_constant(fp, "far_clip_distance", cam->getFarClipDistance());
            try_set_named_constant(fp, "camera_pos_ws", to_ogre(cam_pos));
            if (d3d9) {
                Ogre::Vector4 viewport_size(     viewport->getActualWidth(),      viewport->getActualHeight(),
                                            1.0f/viewport->getActualWidth(), 1.0f/viewport->getActualHeight());
                try_set_named_constant(fp, "viewport_size",viewport_size);
            }
            try_set_named_constant(fp, "alpha_rej", alphaRej);

            ogre_rs->_setTexture(0, true, pipe->getGBufferTexture(0));
            ogre_rs->_setTexture(1, true, tex);
            ogre_rs->_setTextureUnitFiltering(1, Ogre::FT_MIN, Ogre::FO_ANISOTROPIC);
            ogre_rs->_setTextureUnitFiltering(1, Ogre::FT_MAG, Ogre::FO_ANISOTROPIC);
            ogre_rs->_setTextureUnitFiltering(1, Ogre::FT_MIP, Ogre::FO_LINEAR);

            // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
            ogre_rs->bindGpuProgram(vp->_getBindingDelegate());
            ogre_rs->bindGpuProgram(fp->_getBindingDelegate());

            ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, fp->getDefaultParameters(), Ogre::GPV_ALL);
            ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, vp->getDefaultParameters(), Ogre::GPV_ALL);

            ogre_rs->_setCullingMode(Ogre::CULL_NONE);
            if (alphaBlend) {
                ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);
                ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
            } else {
                ogre_rs->_setDepthBufferParams(true, true, Ogre::CMPF_LESS_EQUAL);
                ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
            }
            ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
            ogre_rs->setStencilCheckEnabled(false);
        
            ogre_rs->_render(outside.getRenderOperation());

            ogre_rs->_disableTextureUnit(0);
            ogre_rs->_disableTextureUnit(1);

        } catch (const Exception &e) {
            CERR << "Rendering particles, got: " << e << std::endl;
        } catch (const Ogre::Exception &e) {
            CERR << "Rendering particles, got: " << e.getDescription() << std::endl;
        }

    }

    std::pair<unsigned,unsigned> getTextureSize (void)
    {
        return std::pair<unsigned,unsigned>(texWidth, texHeight);
    }

    bool getAlphaBlend (void) { return alphaBlend; }

};


void GfxParticle::preProcess (const Vector3 &cam)
{
    Vector3 from_cam = pos - cam;
    fromCamDist = from_cam.length();
    fromCamNorm = from_cam / fromCamDist;
}


bool GfxParticle::inside (const Vector3 &p)
{
    // Add the 1 on the end to account for near clip and anything else
    return (p - pos).length2() < dimensions.length2() + 1;
}

GfxParticle::GfxParticle (GfxParticleSystem *sys_)
  : sys(sys_)
{
}

void GfxParticle::release (void)
{
    sys->release(this);
}

std::pair<unsigned,unsigned> GfxParticle::getTextureSize (void)
{
    return sys->getTextureSize(); 
}


void GfxParticle::setDefaultUV (void)
{
    std::pair<unsigned,unsigned> tex_sz = sys->getTextureSize(); 
    u1 = 0;
    v1 = 0;
    u2 = float(tex_sz.first);
    v2 = float(tex_sz.second);
}




typedef std::map<std::string, GfxParticleSystem*> PSysMap;
static PSysMap psystems;

void gfx_particle_define (const std::string &pname, const std::string &tex_name,
                          bool alpha_blend, float alpha_rej, bool emissive)
{
    if (pname[0] != '/') EXCEPT << "Not an absolute path: \"" << pname << "\"" << ENDL;
    if (tex_name[0] != '/') EXCEPT << "Not an absolute path: \"" << tex_name << "\"" << ENDL;
    GfxParticleSystem *&psys = psystems[pname];
    if (psys != NULL) delete psys;
    psys = new GfxParticleSystem(pname, tex_name, alpha_blend, alpha_rej, emissive);
}

GfxParticle *gfx_particle_emit (const std::string &pname)
{
    GfxParticleSystem *&psys = psystems[pname];
    if (psys == NULL) GRIT_EXCEPT("No such particle: \""+pname+"\"");
    return psys->emit();
}

void gfx_particle_render (GfxPipeline *p, bool alpha_blend)
{
    for (PSysMap::iterator i=psystems.begin(),i_=psystems.end() ; i!=i_ ; ++i) {
        GfxParticleSystem *psys = i->second;
        if (psys->getAlphaBlend() == alpha_blend) psys->render(p);
    }
}
