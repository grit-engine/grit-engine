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
#include "gfx_decal.h"
#include "gfx_material.h"

const std::string GfxDecal::className = "GfxDecal";

// vdata/idata to be allocated later because constructor requires ogre to be initialised
static Ogre::RenderOperation box_op;

// Samew winding order for quad as for triangles generated>
#define quad_vertexes(a,b,c,d) a, b, d, a, d, c

void gfx_decal_init (void)
{
    // TODO: The 4x3 transform will need to be 
    // TODO: Eventually we'd like this uv rect to be configurable per decal.
    Vector2 uv1(0, 0);
    Vector2 uv2(1, 1);

    box_op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;

    Ogre::VertexData *vdata = OGRE_NEW Ogre::VertexData();
    box_op.vertexData = vdata;
    vdata->vertexStart = 0;
    const unsigned vertex_count = 8;
    vdata->vertexCount = vertex_count;
    unsigned vdecl_size = 0;
    // strict alignment required here
    struct Vertex { Vector3 position; Vector2 uv1; Vector2 uv2; };
    vdecl_size += vdata->vertexDeclaration->addElement(
        0, vdecl_size, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
    vdecl_size += vdata->vertexDeclaration->addElement(
        0, vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0).getSize();
    vdecl_size += vdata->vertexDeclaration->addElement(
        0, vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 1).getSize();

    Ogre::HardwareVertexBufferSharedPtr vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            vdecl_size, vdata->vertexCount,
            Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
    vdata->vertexBufferBinding->setBinding(0, vbuf);
    Vertex vdata_raw[vertex_count] = {
        { Vector3(-0.5, -0.5, -0.5), uv1, uv2 },
        { Vector3(-0.5, -0.5,  0.5), uv1, uv2 },
        { Vector3(-0.5,  0.5, -0.5), uv1, uv2 },
        { Vector3(-0.5,  0.5,  0.5), uv1, uv2 },
        { Vector3( 0.5, -0.5, -0.5), uv1, uv2 },
        { Vector3( 0.5, -0.5,  0.5), uv1, uv2 },
        { Vector3( 0.5,  0.5, -0.5), uv1, uv2 },
        { Vector3( 0.5,  0.5,  0.5), uv1, uv2 }
    };
    vbuf->writeData(vdata->vertexStart, vdata->vertexCount*vdecl_size, vdata_raw, true);

    Ogre::IndexData *idata = OGRE_NEW Ogre::IndexData();
    box_op.indexData = idata;
    box_op.useIndexes = true;
    const unsigned indexes = 36;  // 3 per triangle therefore 6 per face, 6 faces total
    idata->indexStart = 0;
    idata->indexCount = indexes;
    idata->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
        Ogre::HardwareIndexBuffer::IT_16BIT, indexes, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
    uint16_t idata_raw[indexes] = {
        quad_vertexes(2,6,0,4),
        quad_vertexes(7,3,5,1),
        quad_vertexes(3,2,1,0),
        quad_vertexes(6,7,4,5),
        quad_vertexes(0,4,1,5),
        quad_vertexes(3,7,2,6)
    };
    idata->indexBuffer->writeData(0, sizeof(idata_raw), idata_raw, true);
}

void gfx_decal_shutdown (void)
{
    // Referenced buffers are managed via sharedptr.
    OGRE_DELETE box_op.vertexData;
    OGRE_DELETE box_op.indexData;
}


static std::set<GfxDecal*> all_decals;

GfxDecal::GfxDecal (GfxMaterial *material, const GfxNodePtr &par_)
  : GfxNode(par_),
    enabled(true),
    fade(1),
    material(material)
{
    all_decals.insert(this);
}

GfxDecal::~GfxDecal (void)
{
    if (!dead) destroy();
}

void GfxDecal::destroy (void)
{
    if (dead) THROW_DEAD(className);
    all_decals.erase(this);
    GfxNode::destroy();
}

bool GfxDecal::isEnabled (void)
{
    if (dead) THROW_DEAD(className);
    return enabled;
}

void GfxDecal::setEnabled (bool v)
{
    if (dead) THROW_DEAD(className);
    enabled = v;
}

GfxMaterial *GfxDecal::getMaterial (void)
{
    if (dead) THROW_DEAD(className);
    return material;
}
void GfxDecal::setMaterial (GfxMaterial *m)
{
    if (dead) THROW_DEAD(className);
    material = m;
}

float GfxDecal::getFade (void)
{
    if (dead) THROW_DEAD(className);
    return fade;
}
void GfxDecal::setFade (float f)
{
    if (dead) THROW_DEAD(className);
    fade = f;
}


/*
 * Read depth.  Write diffuse, normal, spec, gloss.  Alpha = 1.
 * Read depth, normal.  Write diffuse, spec, gloss.  Alpha = 1.
 * Read everything.  Write colour.  Alpha < 1.
 */
void GfxDecal::render (const GfxShaderGlobals &g)
{
    if (!enabled) return;

    const Ogre::Matrix4 &world = node->_getFullTransform(); 
    
    // ISSUE RENDER COMMANDS
    try {
        const GfxTextureStateMap &mat_texs = material->getTextures();
        GfxShader *shader = material->getShader();

        // TODO(dcunnin): What if we don't want the dither fade?
        const GfxGslMaterialEnvironment &mat_env = material->getMaterialEnvironment();

        GfxGslMeshEnvironment mesh_env;
        shader->populateMeshEnv(false, 0, mesh_env);

        material->getShader()->bindShader(
            GFX_GSL_PURPOSE_DECAL, mat_env, mesh_env, g, world, nullptr, 0, 1,
            mat_texs, material->getBindings());
        
        
        float dist = (world * Ogre::Vector4(1, 1, 1, 0)).xyz().length();
        Ogre::Vector3 decal_to_cam = (world * Ogre::Vector4(0, 0, 0, 1)).xyz() - to_ogre(g.camPos);
        bool inside = decal_to_cam.length() - 0.4 < dist;

        ogre_rs->_setCullingMode(inside ? Ogre::CULL_ANTICLOCKWISE : Ogre::CULL_CLOCKWISE);
        // read but don't write depth buffer
        if (inside) {
            ogre_rs->_setDepthBufferParams(false, false);
        } else {
            ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_LESS_EQUAL);
        }
        switch (material->getSceneBlend()) {
            case GFX_MATERIAL_OPAQUE:
            ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
            break;
            case GFX_MATERIAL_ALPHA:
            case GFX_MATERIAL_ALPHA_DEPTH:
            ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
            break;
        }

        ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
        ogre_rs->setStencilCheckEnabled(false);
        ogre_rs->_setDepthBias(0, 0);

        ogre_rs->_render(box_op);

        for (unsigned i=0 ; i<mat_texs.size() ; ++i) {
            ogre_rs->_disableTextureUnit(i);
        }


    } catch (const Exception &e) {
        CERR << "Rendering decals, got: " << e << std::endl;
    } catch (const Ogre::Exception &e) {
        CERR << "Rendering decals, got: " << e.getDescription() << std::endl;
    }
}


void gfx_decal_render (GfxPipeline *p)
{
    GfxShaderGlobals g = gfx_shader_globals_cam(p);

    for (GfxDecal *decal : all_decals) {
        decal->render(g);
    }
}


