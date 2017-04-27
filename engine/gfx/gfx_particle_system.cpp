/* Copyright (c) The Grit Game Engine authors 2016
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

const unsigned quad_vertexes = 4;
static Ogre::HardwareVertexBufferSharedPtr quadVertexBuffer;
static Ogre::IndexData quadIndexData;

#define QUAD(a,b,c,d) a, b, d, d, b, c

static GfxShader *shader;

void gfx_particle_init (void)
{
    // set up the quad geometry
    const unsigned quad_triangles = 2;

    quadVertexBuffer =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            12, quad_vertexes, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

    float vdata_raw[3 * quad_vertexes] = {
        -1, 0, -1,
        -1, 0,  1,
         1, 0, -1,
         1, 0,  1,
    };
    quadVertexBuffer->writeData(0, 3 * quad_vertexes * sizeof(float), &vdata_raw[0]);

    // Prepare index buffer
    quadIndexData.indexBuffer = Ogre::HardwareBufferManager::getSingleton()
        .createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, 3*quad_triangles,
                           Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
    quadIndexData.indexStart = 0;
    quadIndexData.indexCount = 3*quad_triangles;
    uint16_t idata_raw[] = { QUAD(0,2,3,1), };
    quadIndexData.indexBuffer->writeData(0, 3 * quad_triangles * sizeof(uint16_t), &idata_raw[0]);

    GfxGslRunParams shader_params = {
        {"gbuffer0", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)},
        {"particleAtlas", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)}
    };

    std::string vertex_code =
        "var part_basis_x = vert.coord0.xyz;\n"
        "var part_half_depth = vert.coord1.x;\n"
        "var part_basis_z = vert.coord2.xyz;\n"
        "var part_pos = vert.coord3.xyz;\n"
        "var part_diffuse = vert.coord4.xyz;\n"
        "var part_alpha = vert.coord5.x;\n"
        "var part_emissive = vert.coord6.xyz;\n"

        "var fragment_uv = lerp(vert.coord7.xy, vert.coord7.zw,\n"
        "                       vert.position.xz / Float2(2, -2) + Float2(0.5, 0.5));\n"
        "var part_colour = global.particleAmbient * part_diffuse + part_emissive;\n"

        "out.position = vert.position.x * part_basis_x\n"
        "             + vert.position.z * part_basis_z\n"
        "             + part_pos;\n"

        "var camera_to_fragment = out.position - global.cameraPos;\n";

    std::string colour_code =
        "var uv = frag.screen / global.viewportSize;\n"
        "var ray = lerp(lerp(global.rayBottomLeft, global.rayBottomRight, uv.x),\n"
        "               lerp(global.rayTopLeft, global.rayTopRight, uv.x),\n"
        "               uv.y);\n"
        "uv.y = 1 - uv.y;  // Textures addressed from top left, frag.screen is bottom left\n"

        "var bytes = sample(mat.gbuffer0, uv).xyz;\n"
        "var normalised_cam_dist = 255.0 * (256.0*256.0*bytes.x + 256.0*bytes.y + bytes.z)\n"
        "                                / (256.0*256.0*256.0 - 1);\n"

        "var scene_dist = length(normalised_cam_dist * ray);\n"
        "var fragment_dist = length(camera_to_fragment);\n"
        "var part_exposed_ = (scene_dist - fragment_dist + part_half_depth)\n"
        "                    / part_half_depth;\n"
        "var part_exposed = clamp(part_exposed_, 0.0, 1.0);\n"
        "var texel = sample(mat.particleAtlas, fragment_uv);\n"
        "out.colour = texel.rgb * part_colour * part_exposed;\n"
        "out.alpha = texel.a * part_alpha * part_exposed;\n";

    shader = gfx_shader_make_or_reset("/system/Particle",
                                      vertex_code, "", colour_code, shader_params, true);


}


class ParticlesInstanceBuffer {
    Ogre::HardwareVertexBufferSharedPtr instBuf;
    Ogre::RenderOperation renderOp;
    Ogre::VertexData vertexData;
    unsigned instBufVertexSize;

    float *instPtr, *instPtr0;
    unsigned maxInstances;

    unsigned instancesAdded (void)
    { return ((instPtr - instPtr0)*sizeof(float)) / instBufVertexSize; }


    public:

    ParticlesInstanceBuffer (void)
    {
        auto d = vertexData.vertexDeclaration;
        // Prepare vertex buffer
        vertexData.vertexStart = 0;
        vertexData.vertexCount = quad_vertexes;

        // Non-instanced data
        unsigned vdecl_size = 0;
        vdecl_size += d->addElement(0, vdecl_size, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
        vertexData.vertexBufferBinding->setBinding(0, quadVertexBuffer);

        auto tc = Ogre::VES_TEXTURE_COORDINATES;
        // Instanced data
        instBufVertexSize = 0;
        // basis_x
        instBufVertexSize += d->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, tc, 0).getSize();
        // half_depth
        instBufVertexSize += d->addElement(1, instBufVertexSize, Ogre::VET_FLOAT1, tc, 1).getSize();
        // basis_z
        instBufVertexSize += d->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, tc, 2).getSize();
        // position 3xfloat
        instBufVertexSize += d->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, tc, 3).getSize();
        // diffuse 3xfloat
        instBufVertexSize += d->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, tc, 4).getSize();
        // alpha
        instBufVertexSize += d->addElement(1, instBufVertexSize, Ogre::VET_FLOAT1, tc, 5).getSize();
        // emissive 3xfloat
        instBufVertexSize += d->addElement(1, instBufVertexSize, Ogre::VET_FLOAT3, tc, 6).getSize();
        // uv1, uv2
        instBufVertexSize += d->addElement(1, instBufVertexSize, Ogre::VET_FLOAT4, tc, 7).getSize();

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

        basis_x *= p->dimensions.x / 2;
        basis_z *= p->dimensions.y / 2;

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
        *(instPtr++) = p->dimensions.z / 2;
        *(instPtr++) = basis_z2.x;
        *(instPtr++) = basis_z2.y;
        *(instPtr++) = basis_z2.z;
        *(instPtr++) = p->pos.x;
        *(instPtr++) = p->pos.y;
        *(instPtr++) = p->pos.z;
        *(instPtr++) = p->diffuse.x;
        *(instPtr++) = p->diffuse.y;
        *(instPtr++) = p->diffuse.z;
        *(instPtr++) = p->alpha;
        *(instPtr++) = p->emissive.x;
        *(instPtr++) = p->emissive.y;
        *(instPtr++) = p->emissive.z;
        *(instPtr++) = p->u1 / tex_size.first;
        *(instPtr++) = p->v1 / tex_size.second;
        *(instPtr++) = p->u2 / tex_size.first;
        *(instPtr++) = p->v2 / tex_size.second;
    }

    void endParticles (void)
    {
        instBuf->unlock();
        renderOp.numberOfInstances = instancesAdded();
    }

    const Ogre::RenderOperation &getRenderOperation (void) { return renderOp; }

    
};


// a particle system holds the buffer for particles of a particular material
class GfxParticleSystem {
    fast_erase_vector<GfxParticle*> particles;

    std::string name;

    DiskResourcePtr<GfxTextureDiskResource> tex;
    unsigned texHeight;
    unsigned texWidth;

    ParticlesInstanceBuffer buffer;

    // every frame, must sort particles and compute new orientations

    public:
    GfxParticleSystem (const std::string &name, const DiskResourcePtr<GfxTextureDiskResource> &tex)
         : name(name)
    {
        setTexture(tex);
    }

    ~GfxParticleSystem (void)
    {
    }

    // Old particles will have wrong uvs if texture changes dimensions.
    void setTexture (const DiskResourcePtr<GfxTextureDiskResource> &v)
    {
        tex = v;
        tex->getOgreTexturePtr()->unload();
        tex->getOgreTexturePtr()->setHardwareGammaEnabled(true);
        tex->getOgreTexturePtr()->load();
        texHeight = tex->getOgreTexturePtr()->getHeight();
        texWidth = tex->getOgreTexturePtr()->getWidth();
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

    void render (GfxPipeline *pipe, const GfxShaderGlobals &globs)
    {
        const CameraOpts &cam_opts = pipe->getCameraOpts();
        const Vector3 &cam_pos = cam_opts.pos;
        const Vector3 &cam_up = cam_opts.dir * Vector3(0, 0, 1);

        // PREPARE BUFFERS

        // temporary list for sorting in
        std::vector<GfxParticle*> tmp_list;
        for (unsigned i=0 ; i<particles.size() ; ++i) {
            GfxParticle *p = particles[i];
            p->preProcess(cam_pos);
            tmp_list.push_back(p);
        }

        // early out for nothing to render
        if (tmp_list.size() == 0) return;

        std::sort(tmp_list.begin(), tmp_list.end(), particleCompare);


        // use particles.size() to avoid frequent reallocation
        buffer.beginParticles(particles.size());
        for (unsigned i=0 ; i<tmp_list.size() ; ++i) {
            GfxParticle *p = tmp_list[i];
            buffer.addParticle(cam_up, p);
        }
        
        buffer.endParticles();


        // ISSUE RENDER COMMANDS
        try {
            GfxTextureStateMap texs;
            // Bind this manually underneath as it is an Ogre internal texture.
            texs["gbuffer0"] = gfx_texture_state_point(nullptr);
            texs["particleAtlas"] = gfx_texture_state_anisotropic(&*tex, GFX_AM_CLAMP);

            GfxShaderBindings binds;
            binds["gbuffer0"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1,1,1,1);
            binds["particleAtlas"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1,1,1,1);


            const Ogre::Matrix4 &I = Ogre::Matrix4::IDENTITY;

            // We may use a special particle "purpose" in future but this works for now.
            shader->bindShader(GFX_GSL_PURPOSE_HUD, false, false, 0,
                               globs, I, nullptr, 0, 1, texs, binds);

            ogre_rs->_setTexture(NUM_GLOBAL_TEXTURES_NO_LIGHTING, true, pipe->getGBufferTexture(0));

            ogre_rs->_setCullingMode(Ogre::CULL_NONE);
            ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);
            ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
            ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
            ogre_rs->setStencilCheckEnabled(false);
            ogre_rs->_setDepthBias(0, 0);

            ogre_rs->_render(buffer.getRenderOperation());

            ogre_rs->_disableTextureUnit(0);
            ogre_rs->_disableTextureUnit(1);

        } catch (const Exception &e) {
            CERR << "Rendering particles, got: " << e << std::endl;
        } catch (const Ogre::Exception &e) {
            CERR << "Rendering particles, got: " << e.getDescription() << std::endl;
        }

    }

    std::pair<unsigned, unsigned> getTextureSize (void) const
    {
        return std::pair<unsigned,unsigned>(texWidth, texHeight);
    }

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

std::pair<unsigned, unsigned> GfxParticle::getTextureSize (void) const
{
    return sys->getTextureSize(); 
}


void GfxParticle::setDefaultUV (void)
{
    std::pair<unsigned,unsigned> tex_sz = getTextureSize(); 
    u1 = 0;
    v1 = 0;
    u2 = float(tex_sz.first);
    v2 = float(tex_sz.second);
}




typedef std::map<std::string, GfxParticleSystem*> PSysMap;
static PSysMap psystems;

void gfx_particle_define (const std::string &pname,
                          const DiskResourcePtr<GfxTextureDiskResource> &tex)
{
    GfxParticleSystem *&psys = psystems[pname];
    if (psys != NULL) {
        // Old particles will have wrong uvs if texture changes dimensions.
        psys->setTexture(tex);
    } else {
        psys = new GfxParticleSystem(pname, tex);
    }
}

GfxParticle *gfx_particle_emit (const std::string &pname)
{
    GfxParticleSystem *&psys = psystems[pname];
    if (psys == NULL) EXCEPT << "No such particle: \"" << pname << "\"" << ENDL;
    return psys->emit();
}

void gfx_particle_render (GfxPipeline *p)
{
    GfxShaderGlobals g = gfx_shader_globals_cam(p);

    for (PSysMap::iterator i=psystems.begin(),i_=psystems.end() ; i!=i_ ; ++i) {
        GfxParticleSystem *psys = i->second;
        psys->render(p, g);
    }
}

std::vector<std::string> gfx_particle_all (void)
{
    std::vector<std::string> r;
    for (PSysMap::iterator i=psystems.begin(),i_=psystems.end() ; i!=i_ ; ++i) {
        r.push_back(i->first);
    }
    return r;
}
