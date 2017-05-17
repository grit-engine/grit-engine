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

// vdata to be allocated later because constructor requires OGRE to be initialised.
static Ogre::RenderOperation tri_op;
static Ogre::HardwareVertexBufferSharedPtr tri_vbuf;

static Ogre::RenderOperation lines_op;
static Ogre::HardwareVertexBufferSharedPtr lines_vbuf;

static Ogre::RenderOperation points_op;
static Ogre::HardwareVertexBufferSharedPtr points_vbuf;

// Initialised by gfx_debug_init().
static unsigned vdecl_size = 28;
GfxShader *shader;

// Strict alignment required here.
struct Vertex {
    Vector3 position;
    Vector3 colour;
    float alpha;
};

static void recreate_vbuf(Ogre::HardwareVertexBufferSharedPtr &vbuf, size_t num_vertexes)
{
    vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        vdecl_size, num_vertexes,
        Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
}

void gfx_debug_init (void)
{
    APP_ASSERT(sizeof(Vertex) == vdecl_size);
    size_t size;

    points_op.operationType = Ogre::RenderOperation::OT_POINT_LIST;
    points_op.vertexData = OGRE_NEW Ogre::VertexData();
    points_op.vertexData->vertexStart = 0;
    points_op.useIndexes = false;
    size = 0;
    size += points_op.vertexData->vertexDeclaration->addElement(
        0, size, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
    size += points_op.vertexData->vertexDeclaration->addElement(
        0, size, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE, 0).getSize();
    APP_ASSERT(size == vdecl_size);
    recreate_vbuf(points_vbuf, 20000);
    points_op.vertexData->vertexBufferBinding->setBinding(0, points_vbuf);

    lines_op.operationType = Ogre::RenderOperation::OT_LINE_LIST;
    lines_op.vertexData = OGRE_NEW Ogre::VertexData();
    lines_op.vertexData->vertexStart = 0;
    lines_op.useIndexes = false;
    size = 0;
    size += lines_op.vertexData->vertexDeclaration->addElement(
        0, size, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
    size += lines_op.vertexData->vertexDeclaration->addElement(
        0, size, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE, 0).getSize();
    APP_ASSERT(size == vdecl_size);
    recreate_vbuf(lines_vbuf, 20000);
    lines_op.vertexData->vertexBufferBinding->setBinding(0, lines_vbuf);

    tri_op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    tri_op.vertexData = OGRE_NEW Ogre::VertexData();
    tri_op.vertexData->vertexStart = 0;
    tri_op.useIndexes = false;
    size = 0;
    size += tri_op.vertexData->vertexDeclaration->addElement(
        0, size, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
    size += tri_op.vertexData->vertexDeclaration->addElement(
        0, size, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE, 0).getSize();
    APP_ASSERT(size == vdecl_size);
    recreate_vbuf(tri_vbuf, 30000);
    tri_op.vertexData->vertexBufferBinding->setBinding(0, tri_vbuf);

    std::string vertex_code = "out.position = transform_to_world(vert.position.xyz);";
    std::string colour_code =
        "out.colour = vert.colour.xyz;\n"
        "out.alpha = vert.colour.w;\n"
        "out.colour = out.colour * out.alpha;\n";
    shader = gfx_shader_make_or_reset("/system/Debug",
                                      vertex_code, "", colour_code, GfxGslRunParams{}, false);
}

void gfx_debug_shutdown (void)
{
    // Referenced buffers are managed via sharedptr.
    OGRE_DELETE tri_op.vertexData;
}

std::vector<Vertex> points_vertexes;

void gfx_debug_point (Vector3 pos, float radius, Vector3 col, float alpha)
{
    (void) radius;  // Looks like we have to have the same radius for all points.
    points_vertexes.push_back({pos, col, alpha});
}

std::vector<Vertex> lines_vertexes;

void gfx_debug_line (Vector3 from, Vector3 to, Vector3 col, float alpha)
{
    lines_vertexes.push_back({from, col, alpha});
    lines_vertexes.push_back({to, col, alpha});
}

std::vector<Vertex> tri_vertexes;

void gfx_debug_triangle (Vector3 v1, Vector3 v2, Vector3 v3, Vector3 col, float alpha)
{
    tri_vertexes.push_back({v1, col, alpha});
    tri_vertexes.push_back({v2, col, alpha});
    tri_vertexes.push_back({v3, col, alpha});
}

void gfx_debug_quad (Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4, Vector3 col, float alpha)
{
    gfx_debug_triangle(v1, v2, v3, col, alpha);
    gfx_debug_triangle(v3, v4, v1, col, alpha);
}

void gfx_debug_render (GfxPipeline *p)
{
    GfxShaderGlobals g = gfx_shader_globals_cam(p);

    const Ogre::Matrix4 world = Ogre::Matrix4::IDENTITY;
    
    if (points_vertexes.size() > 0) {
        // Update buffer.
        Ogre::VertexData *vdata = points_op.vertexData;
        vdata->vertexCount = points_vertexes.size();
        if (points_vbuf->getNumVertices() < vdata->vertexCount) {
            recreate_vbuf(points_vbuf, vdata->vertexCount);
            points_op.vertexData->vertexBufferBinding->setBinding(0, points_vbuf);
        }
        points_vbuf->writeData(
            vdata->vertexStart, vdata->vertexCount * vdecl_size, &points_vertexes[0], true);

        // ISSUE RENDER COMMANDS
        try {
            static const GfxTextureStateMap no_texs;
            static const GfxShaderBindings bindings;
            shader->bindShader(
                GFX_GSL_PURPOSE_HUD, false, false,  0, g, world, nullptr, 0, 1, no_texs, bindings);

            // Read but don't write depth buffer.
            ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_LESS_EQUAL);
            ogre_rs->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

            // All parameters after the boolean are ignored in GL3PlusRenderSystem...
            ogre_rs->_setPointParameters(10, false, 0, 1, 0, 0, 10);
            ogre_rs->setStencilCheckEnabled(false);
            ogre_rs->_setDepthBias(0, 0);

            ogre_rs->_render(points_op);

        } catch (const Exception &e) {
            CERR << "Rendering debug points, got: " << e << std::endl;
        } catch (const Ogre::Exception &e) {
            CERR << "Rendering debug points, got: " << e.getDescription() << std::endl;
        }
        points_vertexes.clear();
    }
    
    if (lines_vertexes.size() > 0) {
        // Update buffer.
        Ogre::VertexData *vdata = lines_op.vertexData;
        vdata->vertexCount = lines_vertexes.size();
        if (lines_vbuf->getNumVertices() < vdata->vertexCount) {
            recreate_vbuf(lines_vbuf, vdata->vertexCount);
            lines_op.vertexData->vertexBufferBinding->setBinding(0, lines_vbuf);
        }
        lines_vbuf->writeData(
            vdata->vertexStart, vdata->vertexCount * vdecl_size, &lines_vertexes[0], true);

        // ISSUE RENDER COMMANDS
        try {
            static const GfxTextureStateMap no_texs;
            static const GfxShaderBindings bindings;
            shader->bindShader(
                GFX_GSL_PURPOSE_HUD, false, false,  0, g, world, nullptr, 0, 1, no_texs, bindings);

            ogre_rs->_setCullingMode(Ogre::CULL_NONE);
            // Read but don't write depth buffer.
            ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_LESS_EQUAL);
            ogre_rs->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

            ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
            ogre_rs->setStencilCheckEnabled(false);
            ogre_rs->_setDepthBias(0, 0);

            ogre_rs->_render(lines_op);

        } catch (const Exception &e) {
            CERR << "Rendering debug lines, got: " << e << std::endl;
        } catch (const Ogre::Exception &e) {
            CERR << "Rendering debug lines, got: " << e.getDescription() << std::endl;
        }
        lines_vertexes.clear();
    }
    
    if (tri_vertexes.size() > 0) {
        // Update buffer.
        Ogre::VertexData *vdata = tri_op.vertexData;
        vdata->vertexCount = tri_vertexes.size();
        if (tri_vbuf->getNumVertices() < vdata->vertexCount) {
            recreate_vbuf(tri_vbuf, vdata->vertexCount);
            tri_op.vertexData->vertexBufferBinding->setBinding(0, tri_vbuf);
        }
        tri_vbuf->writeData(
            vdata->vertexStart, vdata->vertexCount * vdecl_size, &tri_vertexes[0], true);

        // ISSUE RENDER COMMANDS
        try {
            static const GfxTextureStateMap no_texs;
            static const GfxShaderBindings bindings;
            shader->bindShader(
                GFX_GSL_PURPOSE_HUD, false, false,  0, g, world, nullptr, 0, 1, no_texs, bindings);

            ogre_rs->_setCullingMode(Ogre::CULL_CLOCKWISE);
            // Read but don't write depth buffer.
            ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_LESS_EQUAL);
            ogre_rs->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

            ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
            ogre_rs->setStencilCheckEnabled(false);
            ogre_rs->_setDepthBias(0, 0);

            ogre_rs->_render(tri_op);

        } catch (const Exception &e) {
            CERR << "Rendering debug triangles, got: " << e << std::endl;
        } catch (const Ogre::Exception &e) {
            CERR << "Rendering debug triangles, got: " << e.getDescription() << std::endl;
        }
        tri_vertexes.clear();
    }
}
