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

static float global_physics_time = 0;
static float global_left_over_time = 0;

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

float GfxTracerBody::getLength (void) const
{
    assertAlive();
    return length;
}
void GfxTracerBody::setLength (float v)
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
    {"texture", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)},
    {"total_length", GfxGslParam::float1(0.0)},
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
        "var element_dist = vert.coord5.x;\n"
        "var element_colour = global.particleAmbient * element_diffuse * element_alpha\n"
        "                     + element_emissive;\n"
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
        "                     / element_half_depth;\n"
        "var element_exposed = clamp(element_exposed_, 0.0, 1.0);\n"
        // We do not decode PMA here, because we are outputting in PMA form.  We also
        // leave the user to premultiply their element_emissive by their element_alpha
        "var texel = sample(mat.texture, Float2(0.5, fragment_v));\n"
        "var cap_alpha = min(1.0, min(\n"
        "    element_dist / element_half_depth,\n"
        "    (mat.total_length - element_dist) / element_half_depth,\n"
        "));\n"
        "if (element_half_depth <= 0) {\n"
        "    cap_alpha = 1;\n"
        "}\n"
        "var alpha = texel.a * element_alpha * element_exposed * cap_alpha;\n"
        "out.colour = texel.rgb * element_colour;\n"
        "out.alpha = alpha;\n"
        "out.colour = out.colour * element_exposed * cap_alpha;\n"
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
    // distance
    elementVertexSize += d->addElement(0, elementVertexSize, Ogre::VET_FLOAT1, tc, 5).getSize();

    APP_ASSERT(elementVertexSize == sizeof(float) * 13);

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


void GfxTracerBody::Buffer::addOnlyTwoTraceElements (const Vector3 &cam_pos,
                                                     const Element &element0,
                                                     const Element &element1)
{
    // If we ever want the size to be screen-space neutral, we can only normalise the trace_ray.
    Vector3 element_to_cam = cam_pos - element0.pos;
    Vector3 trace_ray = element1.pos - element0.pos;
    Vector3 rib = element_to_cam.cross(trace_ray);
    rib.normalise();

    addTraceElement(element0, rib);
    addTraceElement(element1, rib);
}

void GfxTracerBody::Buffer::addFirstTraceElement (const Vector3 &cam_pos,
                                                  const Element &element,
                                                  const Element &element_next)
{
    Vector3 element_to_cam = cam_pos - element.pos;
    Vector3 trace_ray = element_next.pos - element.pos;
    Vector3 rib = element_to_cam.cross(trace_ray);
    rib.normalise();

    addTraceElement(element, rib);
    lastPos = element.pos;
}

void GfxTracerBody::Buffer::addMiddleTraceElement (const Vector3 &cam_pos,
                                                   const Element &element,
                                                   const Element &element_next)
{
    Vector3 element_to_cam = (cam_pos - element.pos);
    Vector3 trace_ray = (element_next.pos - element.pos).normalisedCopy()
                        + (element.pos - lastPos).normalisedCopy();
    Vector3 rib = element_to_cam.cross(trace_ray);
    rib.normalise();
    
    addTraceElement(element, rib);
    lastPos = element.pos;
}

void GfxTracerBody::Buffer::addLastTraceElement (const Vector3 &cam_pos,
                                                 const Element &element)
{

    Vector3 element_to_cam = cam_pos - element.pos;
    Vector3 trace_ray = element.pos - lastPos;
    Vector3 rib = element_to_cam.cross(trace_ray);
    rib.normalise();
    
    addTraceElement(element, rib);
}

void GfxTracerBody::Buffer::addTraceElement (const GfxTracerBody::Element &element,
                                             const Vector3 &rib)
{
    // First one has to be rendered specially.
    // Cache the previous position.

    Vector3 pos1 = element.pos + rib * element.size / 2;
    Vector3 pos2 = element.pos - rib * element.size / 2;

    // Higher counter means closer to the fresh end.
    // death ranges between 1 (dead) and 0 (alive)
    float death = (global_physics_time + global_left_over_time - element.timestamp) / body->length;
    // Convert it to a spline which is flat at death == 0 and death == 1 and smooth between.
    float trail_fade = 1 - (3 - 2 * death) * death * death;

    float additional_alpha = body->fade * trail_fade;

    *(vertexPtr++) = pos1.x;
    *(vertexPtr++) = pos1.y;
    *(vertexPtr++) = pos1.z;
    *(vertexPtr++) = 0;
    *(vertexPtr++) = element.size / 2;
    *(vertexPtr++) = element.diffuseColour.x * additional_alpha;
    *(vertexPtr++) = element.diffuseColour.y * additional_alpha;
    *(vertexPtr++) = element.diffuseColour.z * additional_alpha;
    *(vertexPtr++) = element.emissiveColour.x * additional_alpha;
    *(vertexPtr++) = element.emissiveColour.y * additional_alpha;
    *(vertexPtr++) = element.emissiveColour.z * additional_alpha;
    *(vertexPtr++) = element.alpha * additional_alpha;
    *(vertexPtr++) = element.distance;

    *(vertexPtr++) = pos2.x;
    *(vertexPtr++) = pos2.y;
    *(vertexPtr++) = pos2.z;
    *(vertexPtr++) = 1;
    *(vertexPtr++) = element.size / 2;
    *(vertexPtr++) = element.diffuseColour.x * additional_alpha;
    *(vertexPtr++) = element.diffuseColour.y * additional_alpha;
    *(vertexPtr++) = element.diffuseColour.z * additional_alpha;
    *(vertexPtr++) = element.emissiveColour.x * additional_alpha;
    *(vertexPtr++) = element.emissiveColour.y * additional_alpha;
    *(vertexPtr++) = element.emissiveColour.z * additional_alpha;
    *(vertexPtr++) = element.alpha * additional_alpha;
    *(vertexPtr++) = element.distance;

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

    // Add on the interpolated one.  It will be removed after we have generated geometry.
    bool remove = false;
    if (global_left_over_time > 0 && elements.size() >= 2) {
        const Element &last1 = elements[elements.size() - 2];
        const Element &last2 = elements[elements.size() - 1];
        float interval_secs = last2.timestamp - last1.timestamp;
        if (interval_secs > 0) {
            Vector3 vel = (last2.pos - last1.pos) / interval_secs;
            elements.emplace_back(last2);
            elements.back().pos += global_left_over_time * vel;
            elements.back().timestamp += global_left_over_time;
            remove = true;
        } else {
            // We could potentially look back further in the buffer to try and interpolate
            // but I'm not sure if this case will arise sufficiently often to warrant
            // that complexity.
        }
    }
        

    // Trim the vector to only the live elements, and mark which ones we're actually going to
    // render.
    float age_limit = global_physics_time + global_left_over_time - length;
    unsigned start = 0;
    while (start < elements.size() && elements[start].timestamp < age_limit) start++;
    unsigned last_not_skipped = 0;  // Index of the last element we render.
    unsigned elements_not_skipped = 0;  // Number of elements to render.
    float distance = 0;
    const Vector3 *last_pos = nullptr;
    for (unsigned i = start; i < elements.size() ; ++i) {
        unsigned counter = i - start;
        if (counter > 0) {
            distance += (elements[i].pos - *last_pos).length();
        }
        elements[i].distance = distance;
        last_pos = &elements[i].pos;

        elements[counter] = elements[i];
        if (i > start
            && (elements[counter].pos - elements[last_not_skipped].pos).length2() < 0.000001) {
            elements[counter].skip = true;
        } else {
            elements[counter].skip = false;
            elements_not_skipped++;
            last_not_skipped = counter;
        }
    }
    elements.resize(elements.size() - start);

    if (elements_not_skipped <= 1) return;

    buffer.beginTrace(elements_not_skipped);
    if (elements_not_skipped == 2) {
        unsigned curr = 0;
        unsigned next;
        for (next = curr + 1 ; elements[next].skip ; next++);
        buffer.addOnlyTwoTraceElements(cam_pos, elements[curr], elements[next]);
    } else {
        unsigned curr;
        for (curr = 0 ; elements[curr].skip ; curr++);
        unsigned next;
        for (next = curr + 1 ; elements[next].skip ; next++);
        buffer.addFirstTraceElement(cam_pos, elements[curr], elements[next]);
        do {
            curr = next;
            for (next = curr + 1 ; elements[next].skip ; next++);
            buffer.addMiddleTraceElement(cam_pos, elements[curr], elements[next]);
        } while (next < last_not_skipped);
        buffer.addLastTraceElement(cam_pos, elements[next]);
    }
    
    buffer.endTrace();

    // Remove interpolated element.
    if (remove && elements.size() > 0)
        elements.pop_back();


    // ISSUE RENDER COMMANDS
    try {
        GfxTextureStateMap texs;
        // Bind this manually underneath as it is an Ogre internal texture.
        texs["gbuffer0"] = gfx_texture_state_point(nullptr);
        if (texture != nullptr)
            texs["texture"] = gfx_texture_state_anisotropic(&*texture, GFX_AM_CLAMP);

        const Ogre::Matrix4 &I = Ogre::Matrix4::IDENTITY;

        GfxGslRunParams bindings;
        bindings["total_length"] = GfxGslParam::float1(distance);

        // We may use a special tracer "purpose" in future but this works for now.
        shader->bindShader(GFX_GSL_PURPOSE_HUD, false, false, 0,
                           globs, I, nullptr, 0, 1, texs, bindings);

        // Manual bind as it is an Ogre internal texture.
        ogre_rs->_setTexture(NUM_GLOBAL_TEXTURES_NO_LIGHTING, true, pipe->getGBufferTexture(0));


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

    Vector3 pos = getWorldTransform() * Vector3(0, 0, 0);

    elements.emplace_back(
        diffuseColour, emissiveColour, alpha, size, pos, global_physics_time);
}


void gfx_tracer_body_set_left_over_time (float left_over_time)
{
    global_left_over_time = left_over_time;
}

void gfx_tracer_body_render (GfxPipeline *pipe)
{
    GfxShaderGlobals globals = gfx_shader_globals_cam(pipe);

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

    for (GfxTracerBody *body : all_tracer_bodies) {
        body->render(pipe, globals);
    }

}

void gfx_tracer_body_pump (float elapsed)
{
    global_physics_time += elapsed;
    for (GfxTracerBody *body : all_tracer_bodies) {
        body->pump();
    }
}
