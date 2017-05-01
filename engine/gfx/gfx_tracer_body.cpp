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

#include <console.h>
#include <math_util.h>

#include "../vect_util.h"

#include "gfx.h"
#include "gfx_fertile_node.h"
#include "gfx_internal.h"
#include "gfx_tracer_body.h"
#include "gfx_pipeline.h"
#include "gfx_shader.h"

const std::string GfxTracerBody::className = "GfxTracerBody";

static std::set<GfxTracerBody*> all_tracer_bodies;

void GfxTracerBody::assertAlive (void) const
{
    if (dead)
        EXCEPT << className << " destroyed." << ENDL;
}

bool GfxTracerBody::isEnabled (void) const
{
    assertAlive();
    return enabled;
}
void GfxTracerBody::setEnabled (bool v)
{
    assertAlive();
    enabled = v;
}

Vector3 GfxTracerBody::getDiffuseColour (void) const
{
    assertAlive();
    return diffuseColour;
}
void GfxTracerBody::setDiffuseColour (const Vector3 &v)
{
    assertAlive();
    diffuseColour = v;
}

Vector3 GfxTracerBody::getEmissiveColour (void) const
{
    assertAlive();
    return emissiveColour;
}
void GfxTracerBody::setEmissiveColour (const Vector3 &v)
{
    assertAlive();
    emissiveColour = v;
}

float GfxTracerBody::getAlpha (void) const
{
    assertAlive();
    return alpha;
}
void GfxTracerBody::setAlpha (float v)
{
    assertAlive();
    alpha = v;
}

float GfxTracerBody::getFade (void) const
{
    assertAlive();
    return fade;
}
void GfxTracerBody::setFade (float v)
{
    assertAlive();
    fade = v;
}

float GfxTracerBody::getSize (void) const
{
    assertAlive();
    return size;
}
void GfxTracerBody::setSize (float v)
{
    assertAlive();
    size = v;
}

unsigned GfxTracerBody::getLength (void) const
{
    assertAlive();
    return length;
}
void GfxTracerBody::setLength (unsigned v)
{
    assertAlive();
    length = v;
}

GfxTextureDiskResource *GfxTracerBody::getTexture (void) const
{
    assertAlive();
    return &*texture;
}
void GfxTracerBody::setTexture (const DiskResourcePtr<GfxTextureDiskResource> &v)
{
    assertAlive();
    if (v != nullptr) {
        if (!v->isLoaded()) v->load();
    }
    texture = v;
}

GfxTracerBody::GfxTracerBody (const GfxNodePtr &par_)
  : GfxNode(par_),
    enabled(true),
    length(50),
    diffuseColour(0.5, 0.5, 0.5),
    emissiveColour(0, 0, 0),
    fade(1),
    alpha(1),
    size(0.1),
    buffer(this)
{
    all_tracer_bodies.insert(this);
}

GfxTracerBody::~GfxTracerBody (void)
{
    if (!dead) destroy();
}

void GfxTracerBody::destroy (void)
{
    assertAlive();
    all_tracer_bodies.erase(this);
    GfxNode::destroy();
}

static GfxShader *shader;
static GfxGslRunParams shader_params = {
    {"gbuffer0", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)},
    {"texture", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)}
};

void gfx_tracer_body_init (void)
{

    // If we encoded worldspace total distance in a vertex coord, we could have a texture u
    // coordinate as well.
    std::string vertex_code =
        "var fragment_v = vert.coord0.x;\n"
        "var element_half_depth = vert.coord1.x;\n"
        "var element_diffuse = vert.coord2.xyz;\n"
        "var element_emissive = vert.coord3.xyz;\n"
        "var element_alpha = vert.coord4.x;\n"
        "var element_colour = global.particleAmbient * element_diffuse + element_emissive;\n"
        "out.position = vert.position.xyz;\n"
        "var camera_to_fragment = out.position - global.cameraPos;\n"
        "";

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
        "var element_exposed_ = (scene_dist - fragment_dist + element_half_depth)\n"
        "                    / element_half_depth;\n"
        "var element_exposed = clamp(element_exposed_, 0.0, 1.0);\n"
        "var texel = sample(mat.texture, Float2(0.5, fragment_v));\n"
        "var alpha = texel.a * element_alpha * element_exposed;\n"
        "out.colour = texel.rgb * element_colour * element_exposed * alpha;\n"
        "out.alpha = alpha;\n"
        "";

    shader = gfx_shader_make_or_reset("/system/Tracer",
                                      vertex_code, "", colour_code, shader_params, true);


}

GfxTracerBody::Buffer::Buffer (const GfxTracerBody *body)
  : body(body)
{
    // Compute elementVertexSize and initialize vertexDeclaration.
    auto *d = vertexData.vertexDeclaration;
    auto pos = Ogre::VES_POSITION;
    auto tc = Ogre::VES_TEXTURE_COORDINATES;
    elementVertexSize = 0;

    // position
    elementVertexSize += d->addElement(0, elementVertexSize, Ogre::VET_FLOAT3, pos).getSize();
    // texture coord
    elementVertexSize += d->addElement(0, elementVertexSize, Ogre::VET_FLOAT1, tc, 0).getSize();
    // element half depth
    elementVertexSize += d->addElement(0, elementVertexSize, Ogre::VET_FLOAT1, tc, 1).getSize();
    // diffuse
    elementVertexSize += d->addElement(0, elementVertexSize, Ogre::VET_FLOAT3, tc, 2).getSize();
    // emissive
    elementVertexSize += d->addElement(0, elementVertexSize, Ogre::VET_FLOAT3, tc, 3).getSize();
    // alpha
    elementVertexSize += d->addElement(0, elementVertexSize, Ogre::VET_FLOAT1, tc, 4).getSize();

    APP_ASSERT(elementVertexSize == sizeof(float) * 12);

    vertexData.vertexStart = 0;

    indexData.indexStart = 0;

    renderOp.vertexData = &vertexData;
    renderOp.indexData = &indexData;
    renderOp.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    renderOp.useIndexes = true;
    vertexPtr = vertexPtr0 = NULL;
    indexPtr = indexPtr0 = NULL;
    maxElements = 0;
}

void GfxTracerBody::Buffer::beginTrace (unsigned elements)
{
    if (elements > maxElements) {
        maxElements = elements;
        // Two vertexes per element.
        vertexBuffer = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            elementVertexSize, 2 * elements, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
        vertexData.vertexBufferBinding->setBinding(0, vertexBuffer);
        unsigned lines = elements - 1;
        // Two triangles (6 indexes) per line.
        indexData.indexBuffer =
            Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
                Ogre::HardwareIndexBuffer::IT_16BIT,
                6 * lines,
                Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
    }

    vertexPtr = vertexPtr0 = static_cast<float*>(
        vertexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
    indexPtr = indexPtr0 = static_cast<uint16_t*>(
        indexData.indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
    counter = 0;
}

void GfxTracerBody::Buffer::addTraceElement (const Vector3 &cam_pos,
                                             const GfxTracerBody::Element &element)
{

    (void) cam_pos;
    // Vector3 element_to_cam = cam_pos - element.pos;
    
    Vector3 rib = Vector3(0, 0, 1);

    // First one has to be rendered specially.
    // Cache the previous position.

    Vector3 pos1 = element.pos + rib * element.size / 2;
    Vector3 pos2 = element.pos - rib * element.size / 2;

    *(vertexPtr++) = pos1.x;
    *(vertexPtr++) = pos1.y;
    *(vertexPtr++) = pos1.z;
    *(vertexPtr++) = 0;
    *(vertexPtr++) = element.size / 2;
    *(vertexPtr++) = element.diffuseColour.x;
    *(vertexPtr++) = element.diffuseColour.y;
    *(vertexPtr++) = element.diffuseColour.z;
    *(vertexPtr++) = element.emissiveColour.x;
    *(vertexPtr++) = element.emissiveColour.y;
    *(vertexPtr++) = element.emissiveColour.z;
    *(vertexPtr++) = element.alpha * body->fade;

    *(vertexPtr++) = pos2.x;
    *(vertexPtr++) = pos2.y;
    *(vertexPtr++) = pos2.z;
    *(vertexPtr++) = 1;
    *(vertexPtr++) = element.size / 2;
    *(vertexPtr++) = element.diffuseColour.x;
    *(vertexPtr++) = element.diffuseColour.y;
    *(vertexPtr++) = element.diffuseColour.z;
    *(vertexPtr++) = element.emissiveColour.x;
    *(vertexPtr++) = element.emissiveColour.y;
    *(vertexPtr++) = element.emissiveColour.z;
    *(vertexPtr++) = element.alpha * body->fade;

    if (counter > 0) {
        // a-------------c
        // b-------------d
        int a = 2 * counter - 2;
        int b = 2 * counter - 1;
        int c = 2 * counter + 0;
        int d = 2 * counter + 1;
        *(indexPtr++) = a;
        *(indexPtr++) = b;
        *(indexPtr++) = d;
        *(indexPtr++) = a;
        *(indexPtr++) = d;
        *(indexPtr++) = c;
    }

    counter++;
}

void GfxTracerBody::Buffer::endTrace (void)
{
    vertexBuffer->unlock();
    indexData.indexBuffer->unlock();
    vertexData.vertexCount = counter * 2;
    indexData.indexCount = (counter - 1) * 6;
    unsigned vertex_floats = elementVertexSize / sizeof(float);
    APP_ASSERT(size_t(vertexPtr - vertexPtr0) == vertexData.vertexCount * vertex_floats);
    APP_ASSERT(size_t(indexPtr - indexPtr0) == indexData.indexCount);
}

void GfxTracerBody::render (const GfxPipeline *pipe, const GfxShaderGlobals &globs)
{
    if (!enabled) return;
    if (dead) return;

    const CameraOpts &cam_opts = pipe->getCameraOpts();
    const Vector3 &cam_pos = cam_opts.pos;

    // PREPARE BUFFER

    // Trim the vector to the last 'length' elements.
    unsigned counter = 0;
    unsigned start = 0;
    if (elements.size() > length) start = elements.size() - length;
    for (unsigned i = start; i < elements.size() ; ++i) {
        elements[counter++] = elements[i];
    }
    elements.resize(counter);

    if (elements.size() <= 1) return;

    buffer.beginTrace(elements.size());
    for (const Element &el : elements) {
        buffer.addTraceElement(cam_pos, el);
    }
    
    buffer.endTrace();


    // ISSUE RENDER COMMANDS
    try {
        GfxTextureStateMap texs;
        // Bind this manually underneath as it is an Ogre internal texture.
        texs["gbuffer0"] = gfx_texture_state_point(nullptr);
        texs["texture"] = gfx_texture_state_anisotropic(&*texture, GFX_AM_CLAMP);

        const Ogre::Matrix4 &I = Ogre::Matrix4::IDENTITY;

        // We may use a special tracer "purpose" in future but this works for now.
        shader->bindShader(GFX_GSL_PURPOSE_HUD, false, false, 0,
                           globs, I, nullptr, 0, 1, texs, shader_params);

        // Manual bind as it is an Ogre internal texture.
        ogre_rs->_setTexture(NUM_GLOBAL_TEXTURES_NO_LIGHTING, true, pipe->getGBufferTexture(0));

        ogre_rs->_setCullingMode(Ogre::CULL_NONE);
        // Tracers behind walls will still be drawn, but will be attenuated.
        // It is probably possible to use the depth buffer for cases where the tracer is very
        // far behind the wall, depending on the max 'size' attribute of trace elements in the
        // buffer.
        ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);
        ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
        ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
        ogre_rs->setStencilCheckEnabled(false);
        ogre_rs->_setDepthBias(0, 0);

        ogre_rs->_render(buffer.getRenderOperation());

        ogre_rs->_disableTextureUnit(0);
        ogre_rs->_disableTextureUnit(1);

    } catch (const Exception &e) {
        CERR << "Rendering tracer, got: " << e << std::endl;
    } catch (const Ogre::Exception &e) {
        CERR << "Rendering tracer, got: " << e.getDescription() << std::endl;
    }
}

void GfxTracerBody::pump (void)
{
    if (dead) return;

    elements.emplace_back(
        diffuseColour, emissiveColour, alpha, size, getWorldTransform() * Vector3(0, 0, 0));
}


void gfx_tracer_body_render (GfxPipeline *pipe)
{
    GfxShaderGlobals globals = gfx_shader_globals_cam(pipe);

    for (GfxTracerBody *body : all_tracer_bodies) {
        body->render(pipe, globals);
    }

}

void gfx_tracer_body_pump (void)
{
    for (GfxTracerBody *body : all_tracer_bodies) {
        body->pump();
    }
}
