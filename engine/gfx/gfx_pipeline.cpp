/* Copyright (c) The Grit Game Engine authors 2016 *
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

#include <math_util.h>
#include <sleep.h>

#include "gfx_body.h"
#include "gfx_debug.h"
#include "gfx_decal.h"
#include "gfx_particle_system.h"
#include "gfx_pipeline.h"
#include "gfx_sky_body.h"
#include "gfx_tracer_body.h"

Ogre::VertexData *screen_quad_vdata;
Ogre::HardwareVertexBufferSharedPtr screen_quad_vbuf;

// The 0th one is the real one, the others are for debug display.
static GfxShader *deferred_ambient_sun[9];
static GfxShader *deferred_lights;
static GfxShader *compositor_tonemap;
static GfxShader *compositor_exposure_filter_then_horz_blur;
static GfxShader *compositor_vert_blur;
static GfxShader *compositor_horz_blur;
static GfxShader *compositor_vert_blur_combine_tonemap;

void gfx_pipeline_init (void)
{
    // Prepare vertex buffer
    screen_quad_vdata = new Ogre::VertexData();
    screen_quad_vdata->vertexStart = 0;
    screen_quad_vdata->vertexCount = 6;

    // Non-instanced data
    unsigned vdecl_size = 0;
    vdecl_size += screen_quad_vdata->vertexDeclaration->addElement(0, vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_POSITION).getSize();
    screen_quad_vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            vdecl_size, 6, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
    screen_quad_vdata->vertexBufferBinding->setBinding(0, screen_quad_vbuf);

    float vdata_raw[] = {
        -1, -1, // bottom left
         1, -1, // bottom right
        -1,  1, // top left
         1,  1, // top right
         1, -1, // bottom right
        -1,  1 // top left
    };
    screen_quad_vbuf->writeData(screen_quad_vdata->vertexStart, screen_quad_vdata->vertexCount*vdecl_size, vdata_raw);

    GfxGslRunParams gbuffer_shader_params = {
        {"gbuffer0", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)},
        {"gbuffer1", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)},
        {"gbuffer2", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)},
        {"shadowRes", GfxGslParam::float1(512)},
        {"shadowFactor", GfxGslParam::float1(5000)},
    };

    std::string das_vertex_code =
        "var pos_ss = vert.position.xy;\n"
        "out.position = Float3(pos_ss, 0.5);\n"

        "var uv = pos_ss / Float2(2, -2) + Float2(0.5, 0.5);\n"
        "var ray = lerp(lerp(global.rayTopLeft, global.rayTopRight, uv.x),\n"
        "               lerp(global.rayBottomLeft, global.rayBottomRight, uv.x),\n"
        "               uv.y);\n"
        + std::string(gfx_d3d9() ? "uv = uv + Float2(0.5, 0.5) / global.viewportSize.xy;\n" : "");

    std::string deferred_colour_code =
        "var texel0 = sample(mat.gbuffer0, uv);\n"
        "var texel1 = sample(mat.gbuffer1, uv);\n"
        "var texel2 = sample(mat.gbuffer2, uv);\n"
        "var shadow_oblique_cutoff = unpack_deferred_shadow_cutoff(texel0, texel1, texel2);\n"
        "var d = unpack_deferred_diffuse_colour(texel0, texel1, texel2);\n"
        "var s = unpack_deferred_specular(texel0, texel1, texel2);\n"
        "var g = unpack_deferred_gloss(texel0, texel1, texel2);\n"
        "var normalised_cam_dist = unpack_deferred_cam_dist(texel0, texel1, texel2);\n"
        "var normal_vs = unpack_deferred_normal(texel0, texel1, texel2);\n"
        "if (normalised_cam_dist>=1) discard;\n"
        "var pos_ws = global.cameraPos + normalised_cam_dist*ray;\n"
        "var cam_dist = normalised_cam_dist * global.farClipDistance;\n"
        "var v2c = -normalise(ray);\n"
        "var normal_ws = mul(global.invView, Float4(normal_vs, 0)).xyz;\n";

    deferred_ambient_sun[0] = gfx_shader_make_or_reset(
        "/system/DeferredAmbientSun0",
        das_vertex_code,
        "",
        deferred_colour_code +
        "var sun = sunlight(pos_ws, v2c, d, normal_ws, g, s, cam_dist);\n"
        "var env = envlight(v2c, d, normal_ws, g, s);\n"
        "out.colour = sun + env;\n"
        "out.colour = lerp(global.fogColour, out.colour, fog_weakness(cam_dist));\n",
        gbuffer_shader_params,
        true);

    // Diffuse only.
    deferred_ambient_sun[1] = gfx_shader_make_or_reset(
        "/system/DeferredAmbientSun1",
        das_vertex_code,
        "",
        deferred_colour_code +
        "out.colour = d;\n",
        gbuffer_shader_params,
        true);

    // Normal
    deferred_ambient_sun[2] = gfx_shader_make_or_reset(
        "/system/DeferredAmbientSun2",
        das_vertex_code,
        "",
        deferred_colour_code +
        "out.colour = 0.5 + normal_ws/2;\n",
        gbuffer_shader_params,
        true);

    // Specular
    deferred_ambient_sun[3] = gfx_shader_make_or_reset(
        "/system/DeferredAmbientSun3",
        das_vertex_code,
        "",
        deferred_colour_code +
        "out.colour = s;\n",
        gbuffer_shader_params,
        true);

    // Gloss
    deferred_ambient_sun[4] = gfx_shader_make_or_reset(
        "/system/DeferredAmbientSun4",
        das_vertex_code,
        "",
        deferred_colour_code +
        "out.colour = g;\n",
        gbuffer_shader_params,
        true);

    // Position (world space)
    deferred_ambient_sun[5] = gfx_shader_make_or_reset(
        "/system/DeferredAmbientSun5",
        das_vertex_code,
        "",
        deferred_colour_code +
        "out.colour = (pos_ws % 1 + 1) % 1;\n",
        gbuffer_shader_params,
        true);

    // Render shadow map 0 in the middle of the screen.
    deferred_ambient_sun[6] = gfx_shader_make_or_reset(
        "/system/DeferredAmbientSun6",
        "out.position = Float3(vert.position.xy, 0.5);\n",
        "",
        "var ray = (global.rayTopLeft + global.rayTopRight\n"
        "           + global.rayBottomLeft + global.rayBottomRight) / 4;\n"
        "var pos_ws = ray * 0.9;\n" // Rear-centre frustum.
        "var offset = floor((global.viewportSize - mat.shadowRes) / 2);\n"
        "var shad_uv = (frag.screen - offset) / mat.shadowRes;\n"
        "if (shad_uv.x > 1 || shad_uv.y > 1 || shad_uv.x < 0 || shad_uv.y < 0) {\n"
        "    out.colour = Float3(0.1, 0, 0);\n"  
        "} else {\n"
        "    var sun_dist = (sample(global.shadowMap0, shad_uv).x / 2 + 0.5) * mat.shadowFactor;\n"
        "    out.colour.x = sun_dist % 1;\n"
        "    out.colour.y = ((sun_dist - out.colour.x) / 255.0) % 1;\n"
        "    out.colour.z = (sun_dist - out.colour.x - out.colour.y * 255) / 255.0 / 255.0;\n"
        "}\n",
        gbuffer_shader_params,
        true);

    // Render shadow map 1 in the middle of the screen.
    deferred_ambient_sun[7] = gfx_shader_make_or_reset(
        "/system/DeferredAmbientSun7",
        "out.position = Float3(vert.position.xy, 0.5);\n",
        "",
        "var ray = (global.rayTopLeft + global.rayTopRight\n"
        "           + global.rayBottomLeft + global.rayBottomRight) / 4;\n"
        "var pos_ws = ray * 0.9;\n" // Rear-centre frustum.
        "var offset = floor((global.viewportSize - mat.shadowRes) / 2);\n"
        "var shad_uv = (frag.screen - offset) / mat.shadowRes;\n"
        "if (shad_uv.x > 1 || shad_uv.y > 1 || shad_uv.x < 0 || shad_uv.y < 0) {\n"
        "    out.colour = Float3(0.1, 0, 0);\n"  
        "} else {\n"
        "    var sun_dist = (sample(global.shadowMap1, shad_uv).x / 2 + 0.5) * mat.shadowFactor;\n"
        "    out.colour.x = sun_dist % 1;\n"
        "    out.colour.y = ((sun_dist - out.colour.x) / 255.0) % 1;\n"
        "    out.colour.z = (sun_dist - out.colour.x - out.colour.y * 255) / 255.0 / 255.0;\n"
        "}\n",
        gbuffer_shader_params,
        true);

    // Render shadow map 2 in the middle of the screen.
    deferred_ambient_sun[8] = gfx_shader_make_or_reset(
        "/system/DeferredAmbientSun8",
        "out.position = Float3(vert.position.xy, 0.5);\n",
        "",
        "var ray = (global.rayTopLeft + global.rayTopRight\n"
        "           + global.rayBottomLeft + global.rayBottomRight) / 4;\n"
        "var pos_ws = ray * 0.9;\n" // Rear-centre frustum.
        "var offset = floor((global.viewportSize - mat.shadowRes) / 2);\n"
        "var shad_uv = (frag.screen - offset) / mat.shadowRes;\n"
        "if (shad_uv.x > 1 || shad_uv.y > 1 || shad_uv.x < 0 || shad_uv.y < 0) {\n"
        "    out.colour = Float3(0.1, 0, 0);\n"  
        "} else {\n"
        "    var sun_dist = (sample(global.shadowMap2, shad_uv).x / 2 + 0.5) * mat.shadowFactor;\n"
        "    out.colour.x = sun_dist % 1;\n"
        "    out.colour.y = ((sun_dist - out.colour.x) / 255.0) % 1;\n"
        "    out.colour.z = (sun_dist - out.colour.x - out.colour.y * 255) / 255.0 / 255.0;\n"
        "}\n",
        gbuffer_shader_params,
        true);


    // TODO: Not clear if it's more efficient to compute ray at the vertexes.
    std::string identity_vertex_code =
        "out.position = vert.position.xyz;\n";


    std::string lights_colour_code =
        "var uv = frag.screen / global.viewportSize;\n"
        "uv.y = 1 - uv.y;\n"  // frag.screen is lower-left, textures upper-right.
        "var ray = lerp(lerp(global.rayTopLeft, global.rayTopRight, uv.x),\n"
        "               lerp(global.rayBottomLeft, global.rayBottomRight, uv.x),\n"
        "               uv.y);\n"
        // This code shouldn't be needed if frag.screen is working properly.
        //+ std::string(gfx_d3d9() ? "uv = uv + Float2(0.5, 0.5) / global.viewportSize.xy;\n" : "")
        + deferred_colour_code +
        "var light_aim_ws = vert.normal;\n"
        "var light_pos_ws = vert.coord2.xyz;\n"
        "var inner = vert.coord3.x;\n"
        "var outer = vert.coord3.y;\n"
        "var range = vert.coord3.z;\n"
        "var diff_colour = vert.coord0.xyz;\n"
        "var spec_colour = vert.coord1.xyz;\n"

        "var light_ray_ws = light_pos_ws - pos_ws;\n"
        "var light_dist = length(light_ray_ws);\n"
        "var surf_to_light = light_ray_ws / light_dist;\n"

        "var dist = min(1.0, light_dist / range);\n"
        // This is the fadeoff equation that should probably be changed.
        "var light_intensity = 2*dist*dist*dist - 3*dist*dist + 1;\n"

        "var angle = -dot(light_aim_ws, surf_to_light);\n"
        "if (outer != inner) {\n"
        "    var occlusion = clamp((angle-inner)/(outer-inner), 0.0, 1.0);\n"
        "    light_intensity = light_intensity * (1 - occlusion);\n"
        "}\n"

        "out.colour = light_intensity * punctual_lighting(\n"
        "    surf_to_light,\n"
        "    v2c,\n"
        "    d,\n"
        "    normal_ws,\n"
        "    g,\n"
        "    s,\n"
        "    diff_colour,\n"
        "    spec_colour\n"
        ");\n"
    ;

    deferred_lights = gfx_shader_make_or_reset("/system/DeferredLights",
                                               identity_vertex_code, "", lights_colour_code,
                                               gbuffer_shader_params, true);


    //////////////
    // TONE MAP //
    //////////////

    GfxGslRunParams hdr_shader_params = {
        {"hdr", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)},
        {"colourGradeLut", GfxGslParam(GFX_GSL_FLOAT_TEXTURE3, 1, 1, 1, 1)},
    };

    // CAUTION: tonemapping also done by /system/VertBlurCombineTonemap, needs to match this.
    std::string compositor_tonemap_colour =
        "var uv = frag.screen / global.viewportSize;\n"
        "uv.y = 1 - uv.y;\n"  // frag.screen is origin lower-left, textures origin upper-left.
        "out.colour = global.exposure * sample(mat.hdr, uv).xyz;\n"
        // Tone map algorithm:
        "out.colour = out.colour / (1 + out.colour);\n"
        "out.colour = gamma_encode(out.colour);\n"
        "var lut_uvw = (out.colour * 31 + 0.5) / 32;\n"  // Hardcode 32x32x32 dimensions.
        "out.colour = sample(mat.colourGradeLut, lut_uvw).rgb;\n"
        "out.colour = desaturate(out.colour, global.saturation);\n";


    // TODO: dithering
    compositor_tonemap = gfx_shader_make_or_reset("/system/TonemapOnly",
                                                  identity_vertex_code, "",
                                                  compositor_tonemap_colour, hdr_shader_params,
                                                  true);

    ///////////
    // BLOOM //
    ///////////

    GfxGslRunParams bloom_params = {
        {"bloomTexScale", GfxGslParam::float1(1.0f)},
        {"colourGradeLut", GfxGslParam(GFX_GSL_FLOAT_TEXTURE3, 1, 1, 1, 1)},
        {"original", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)},
        {"srcTex", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)},
    };

    

    std::stringstream ss;  // FIRST HORZ
    ss << "var pascal_row11 = []Float{ 252/638.0, 210/638.0, 120/638.0,\n";
    ss << "                            45/638.0, 10/638.0, 1/638.0 };\n";
    ss << "var uv = frag.screen / global.viewportSize;\n";
    ss << "uv.y = 1 - uv.y;\n";  // frag.screen is origin lower-left, textures origin upper-left.
    ss << "var off = mat.bloomTexScale / global.viewportSize.x;\n";
    ss << "for (var i=-4 ; i<=4 ; i = i + 1) {\n";
    ss << "    var bloom_uv = uv * mat.bloomTexScale + off * Float2(i, 0);\n";
    ss << "    bloom_uv = clamp(bloom_uv,\n";
    ss << "                     Float2(off, off) * mat.bloomTexScale,\n";
    ss << "                     Float2(1 - off, 1 - off) * mat.bloomTexScale);\n";
    ss << "    var colour = global.exposure * sample(mat.srcTex, bloom_uv).xyz;\n";
    // First iteration takes unfiltered unscaled hdr fb as input.
    ss << "    var t = colour - global.bloomThreshold;\n";
    ss << "    var bloom = colour * clamp((t.r + t.g + t.b)/3.0, 0.0, 1.0);\n";
    ss << "    out.colour = out.colour + pascal_row11[abs(i)] * bloom;\n";
    ss << "}\n";

    compositor_exposure_filter_then_horz_blur = gfx_shader_make_or_reset(
        "/system/ExposureFilterThenHorzBlur", identity_vertex_code, "", ss.str(),
        bloom_params, true);


    ss.str(""); // VERT
    ss << "var pascal_row11 = []Float{ 252/638.0, 210/638.0, 120/638.0,\n";
    ss << "                            45/638.0, 10/638.0, 1/638.0 };\n";
    ss << "var uv = frag.screen / global.viewportSize;\n";
    ss << "uv.y = 1 - uv.y;\n";  // frag.screen is origin lower-left, textures origin upper-left.
    ss << "var off = mat.bloomTexScale / global.viewportSize.x;\n";
    ss << "for (var i=-4 ; i<=4 ; i = i + 1) {\n";
    ss << "    var bloom_uv = uv * mat.bloomTexScale + off * Float2(0, i);\n";
    ss << "    bloom_uv = clamp(bloom_uv,\n";
    ss << "                     Float2(off, off) * mat.bloomTexScale,\n";
    ss << "                     Float2(1 - off, 1 - off) * mat.bloomTexScale);\n";
    ss << "    var colour = sample(mat.srcTex, bloom_uv).xyz;\n";
    ss << "    out.colour = out.colour + pascal_row11[abs(i)] * colour;\n";
    ss << "}\n";

    compositor_vert_blur = gfx_shader_make_or_reset(
        "/system/VertBlur", identity_vertex_code, "", ss.str(),
        bloom_params, true);

    ss.str(""); // HORZ
    ss << "var pascal_row11 = []Float{ 252/638.0, 210/638.0, 120/638.0,\n";
    ss << "                            45/638.0, 10/638.0, 1/638.0 };\n";
    ss << "var uv = frag.screen / global.viewportSize;\n";
    ss << "uv.y = 1 - uv.y;\n";  // frag.screen is origin lower-left, textures origin upper-left.
    ss << "var off = mat.bloomTexScale / global.viewportSize.x;\n";
    ss << "for (var i=-4 ; i<=4 ; i = i + 1) {\n";
    ss << "    var bloom_uv = uv * mat.bloomTexScale + off * Float2(i, 0);\n";
    ss << "    bloom_uv = clamp(bloom_uv,\n";
    ss << "                     Float2(off, off) * mat.bloomTexScale,\n";
    ss << "                     Float2(1 - off, 1 - off) * mat.bloomTexScale);\n";
    ss << "    var colour = sample(mat.srcTex, bloom_uv).xyz;\n";
    ss << "    out.colour = out.colour + pascal_row11[abs(i)] * colour;\n";
    ss << "}\n";

    compositor_horz_blur = gfx_shader_make_or_reset(
        "/system/HorzBlur", identity_vertex_code, "", ss.str(),
        bloom_params, true);

    ss.str(""); // LAST VERT
    ss << "var pascal_row11 = []Float{ 252/638.0, 210/638.0, 120/638.0,\n";
    ss << "                            45/638.0, 10/638.0, 1/638.0 };\n";
    ss << "var uv = frag.screen / global.viewportSize;\n";
    ss << "uv.y = 1 - uv.y;\n";  // frag.screen is origin lower-left, textures origin upper-left.
    ss << "var off = mat.bloomTexScale / global.viewportSize.x;\n";
    ss << "for (var i=-4 ; i<=4 ; i = i + 1) {\n";
    ss << "    var bloom_uv = uv * mat.bloomTexScale + off * Float2(0, i);\n";
    ss << "    bloom_uv = clamp(bloom_uv,\n";
    ss << "                     Float2(off, off) * mat.bloomTexScale,\n";
    ss << "                     Float2(1 - off, 1 - off) * mat.bloomTexScale);\n";
    ss << "    var colour = sample(mat.srcTex, bloom_uv).xyz;\n";
    ss << "    out.colour = out.colour + pascal_row11[abs(i)] * colour;\n";
    ss << "}\n";
    ss << "out.colour = out.colour + global.exposure * sample(mat.original, uv).xyz;\n";
    // Tone map algorithm:
    ss << "out.colour = out.colour / (1 + out.colour);\n";
    ss << "out.colour = gamma_encode(out.colour);\n";
    ss << "var lut_uvw = (out.colour * 31 + 0.5) / 32;\n";  // Hardcode 32x32x32 dimensions.
    ss << "out.colour = sample(mat.colourGradeLut, lut_uvw).rgb;\n";
    ss << "out.colour = desaturate(out.colour, global.saturation);\n";

    compositor_vert_blur_combine_tonemap = gfx_shader_make_or_reset(
        "/system/VertBlurCombineTonemap", identity_vertex_code, "", ss.str(),
        bloom_params, true);

}


// {{{ (Deferred rendering) PointLightsGeometry

// Buffers constructed every frame to render the point lights with deferred shading
// (Cube and pyramid shaped bounding boxes for each light)
class PointLightsGeometry {

protected:

    Ogre::RenderOperation mRenderOperation;
    Ogre::VertexData mVertexData;
    Ogre::IndexData mIndexData;

    unsigned mDeclSize;
    unsigned mMaxVertexesGPU;
    unsigned mMaxIndexesGPU;

    float *mVertexPtr0; // hold the base pointer
    uint16_t *mIndexPtr0; // hold the base pointer
    float *mVertexPtr; // hold the marching pointer
    uint16_t *mIndexPtr; // hold the marching pointer

public:

    PointLightsGeometry (void)
      :
        mDeclSize(0),
        mMaxVertexesGPU(0), // increase later
        mMaxIndexesGPU(0), // increase later
        mVertexPtr0(NULL),
        mIndexPtr0(NULL)
    {
        mRenderOperation.vertexData = &mVertexData;
        mRenderOperation.indexData = &mIndexData;
        mRenderOperation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        mRenderOperation.useIndexes = true;

        // position on screen (just distinguishes the corners frome each other, -1 or 1)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_POSITION);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // direction the light is pointing (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_NORMAL);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // diffuse colour (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_TEXTURE_COORDINATES, 0);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // specular colour (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_TEXTURE_COORDINATES, 1);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // light centre (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_TEXTURE_COORDINATES, 2);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // inner/outer cone dot product and range (same for all 8 corners of the cube)
        mVertexData.vertexDeclaration->addElement(0, mDeclSize, Ogre::VET_FLOAT3,
                                                                Ogre::VES_TEXTURE_COORDINATES, 3);
        mDeclSize += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

    }

    const Ogre::RenderOperation &getRenderOperation (void) { return mRenderOperation; }

    unsigned indexesUsed (void) { return mIndexPtr - mIndexPtr0; }
    unsigned vertexesUsed (void) { return ((mVertexPtr-mVertexPtr0)*sizeof(float)) / mDeclSize; }

    void beginLights (unsigned lights)
    {
        unsigned vertexes = lights * 8; // assume worst case -- all cubes
        if (vertexes > mMaxVertexesGPU) {
            mMaxVertexesGPU = vertexes;
            Ogre::HardwareVertexBufferSharedPtr vbuf =
                Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(mDeclSize, vertexes, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
            mVertexData.vertexBufferBinding->setBinding(0, vbuf);
        }

        unsigned indexes = lights * 36; // assume worst case -- all cubes
        if (indexes > mMaxIndexesGPU) {
            mMaxIndexesGPU = indexes;
            mIndexData.indexBuffer = Ogre::HardwareBufferManager::getSingleton().
                createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, indexes, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
        }

        mVertexPtr0 = mVertexPtr = static_cast<float*>(mVertexData.vertexBufferBinding->getBuffer(0)->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        mIndexPtr0 = mIndexPtr = static_cast<uint16_t*>(mIndexData.indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
    }

    #define quad_vertexes(a,b,c,d) a, b, d, a, d, c

    void pyramidLight (const Ogre::Vector3 &wpos,
                       const Ogre::Vector3 *cuboid_planes, const Ogre::Vector3 &dir_ws,
                       const Ogre::ColourValue &diff, const Ogre::ColourValue &spec,
                       float inner, float outer, float range)
    {

        Ogre::Vector3 vertexes[] = {
            wpos + cuboid_planes[0] + cuboid_planes[2] + cuboid_planes[5],
            wpos + cuboid_planes[0] + cuboid_planes[3] + cuboid_planes[5],
            wpos + cuboid_planes[1] + cuboid_planes[2] + cuboid_planes[5],
            wpos + cuboid_planes[1] + cuboid_planes[3] + cuboid_planes[5],
            wpos,
        };

        static unsigned indexes[] = {
            quad_vertexes(3,1,2,0),
            3, 4, 1,
            1, 4, 0,
            0, 4, 2,
            2, 4, 3,
        };


        unsigned offset = vertexesUsed();
        for (int j=0 ; j<5 ; ++j) {
            // position
            *(mVertexPtr++) = vertexes[j].x;
            *(mVertexPtr++) = vertexes[j].y;
            *(mVertexPtr++) = vertexes[j].z;
            // direction
            *(mVertexPtr++) = dir_ws.x;
            *(mVertexPtr++) = dir_ws.y;
            *(mVertexPtr++) = dir_ws.z;
            // diffuse colour
            *(mVertexPtr++) = diff.r;
            *(mVertexPtr++) = diff.g;
            *(mVertexPtr++) = diff.b;
            // specular colour
            *(mVertexPtr++) = spec.r;
            *(mVertexPtr++) = spec.g;
            *(mVertexPtr++) = spec.b;
            // centre position
            *(mVertexPtr++) = wpos.x;
            *(mVertexPtr++) = wpos.y;
            *(mVertexPtr++) = wpos.z;
            // inner/outer cone dot product, range
            *(mVertexPtr++) = inner;
            *(mVertexPtr++) = outer;
            *(mVertexPtr++) = range;
        }
        for (int j=0 ; j<18 ; ++j) {
            *(mIndexPtr++) = offset+indexes[j];
        }
    }

    void cubeLight (const Ogre::Vector3 &wpos,
                    const Ogre::Vector3 *cuboid_planes, const Ogre::Vector3 &dir_ws,
                    const Ogre::ColourValue &diff, const Ogre::ColourValue &spec,
                    float inner, float outer, float range)
    {
        Ogre::Vector3 vertexes[] = {
            wpos + cuboid_planes[0] + cuboid_planes[2] + cuboid_planes[4],
            wpos + cuboid_planes[0] + cuboid_planes[2] + cuboid_planes[5],
            wpos + cuboid_planes[0] + cuboid_planes[3] + cuboid_planes[4],
            wpos + cuboid_planes[0] + cuboid_planes[3] + cuboid_planes[5],
            wpos + cuboid_planes[1] + cuboid_planes[2] + cuboid_planes[4],
            wpos + cuboid_planes[1] + cuboid_planes[2] + cuboid_planes[5],
            wpos + cuboid_planes[1] + cuboid_planes[3] + cuboid_planes[4],
            wpos + cuboid_planes[1] + cuboid_planes[3] + cuboid_planes[5],
        };

        static unsigned indexes[] = {
            quad_vertexes(2,6,0,4),
            quad_vertexes(7,3,5,1),
            quad_vertexes(3,2,1,0),
            quad_vertexes(6,7,4,5),
            quad_vertexes(0,4,1,5),
            quad_vertexes(3,7,2,6)
        };

        unsigned offset = vertexesUsed();
        for (int j=0 ; j<8 ; ++j) {
            // position
            *(mVertexPtr++) = vertexes[j].x;
            *(mVertexPtr++) = vertexes[j].y;
            *(mVertexPtr++) = vertexes[j].z;
            // direction
            *(mVertexPtr++) = dir_ws.x;
            *(mVertexPtr++) = dir_ws.y;
            *(mVertexPtr++) = dir_ws.z;
            // diffuse colour
            *(mVertexPtr++) = diff.r;
            *(mVertexPtr++) = diff.g;
            *(mVertexPtr++) = diff.b;
            // specular colour
            *(mVertexPtr++) = spec.r;
            *(mVertexPtr++) = spec.g;
            *(mVertexPtr++) = spec.b;
            // centre position
            *(mVertexPtr++) = wpos.x;
            *(mVertexPtr++) = wpos.y;
            *(mVertexPtr++) = wpos.z;
            // inner/outer cone dot product, range
            *(mVertexPtr++) = inner;
            *(mVertexPtr++) = outer;
            *(mVertexPtr++) = range;
        }
        for (int j=0 ; j<36 ; ++j) {
            *(mIndexPtr++) = offset+indexes[j];
        }
    }


    void endLights ()
    {
        mVertexData.vertexBufferBinding->getBuffer(0)->unlock();
        mIndexData.indexBuffer->unlock();

        mVertexData.vertexCount = vertexesUsed();
        mIndexData.indexCount = indexesUsed();
    }


};

// }}}


// {{{ (Deferred rendering) DeferredLightingPasses

class DeferredLightingPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    PointLightsGeometry mdl;
    PointLightsGeometry mdlInside;

    public:
    DeferredLightingPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0, ""), pipe(pipe)
    {

        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        Ogre::Camera *cam = pipe->getCamera();
        Ogre::Vector3 cam_pos = cam->getPosition();

        GfxTextureStateMap texs;
        // fill these in later as they are are Ogre internal textures
        texs["gbuffer0"] = gfx_texture_state_point(nullptr);
        texs["gbuffer1"] = gfx_texture_state_point(nullptr);
        texs["gbuffer2"] = gfx_texture_state_point(nullptr);

        GfxShaderBindings binds;
        binds["gbuffer0"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1,1,1,1);
        binds["gbuffer1"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1,1,1,1);
        binds["gbuffer2"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1,1,1,1);
        binds["shadowRes"] = GfxGslParam::float1(gfx_option(GFX_SHADOW_RES));
        binds["shadowFactor"] = GfxGslParam::float1(shader_scene_env.shadowFactor);

        const Ogre::Matrix4 &I = Ogre::Matrix4::IDENTITY;

        try {
            GfxShaderGlobals globs = gfx_shader_globals_cam(pipe);

            GfxShader *shader = deferred_ambient_sun[pipe->getCameraOpts().debugMode];
            shader->bindShader(GFX_GSL_PURPOSE_DEFERRED_AMBIENT_SUN, false,
                               false, 0, globs, I, nullptr, 0, 1, texs, binds);

            for (unsigned i=0 ; i<3 ; ++i) {
                ogre_rs->_setTexture(NUM_GLOBAL_TEXTURES_LIGHTING + i, true, pipe->getGBufferTexture(i));
            }

            ogre_rs->_setCullingMode(Ogre::CULL_NONE);
            ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);
            ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
            ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
            ogre_rs->setStencilCheckEnabled(false);
            ogre_rs->_setDepthBias(0, 0);

            // Sun + ambient render
            Ogre::RenderOperation op;
            op.useIndexes = false;
            op.vertexData = screen_quad_vdata;
            op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
            ogre_rs->_render(op);

            // This should be in shader.
            for (unsigned i=0 ; i<NUM_GLOBAL_TEXTURES_LIGHTING + 3 ; ++i) {
                ogre_rs->_disableTextureUnit(i);
            }

        } catch (const Exception &e) {
            CERR << "Rendering deferred sun, got: " << e << std::endl;
        } catch (const Ogre::Exception &e) {
            CERR << "Rendering deferred sun, got: " << e.getDescription() << std::endl;
        }

        if (pipe->getCameraOpts().debugMode) return;

        if (!pipe->getCameraOpts().pointLights) return;

        try {

            /////////////////////
            // deferred lights //
            /////////////////////

            const Ogre::LightList &ll = ogre_sm->_getLightsAffectingFrustum();

            mdl.beginLights(ll.size());
            mdlInside.beginLights(ll.size());

            int light_counter = 0;
            for (Ogre::LightList::const_iterator i=ll.begin(),i_=ll.end(); i!=i_ ; ++i) {
                Ogre::Light *l = *i;
                if (l == ogre_sun) continue;
                light_counter++;
                const Ogre::Vector3 &dir_ws = l->getDerivedDirection();
                const Ogre::ColourValue &diff = l->getDiffuseColour();
                const Ogre::ColourValue &spec = l->getSpecularColour();
                float inner = Ogre::Math::Cos(l->getSpotlightInnerAngle());
                float outer = Ogre::Math::Cos(l->getSpotlightOuterAngle());
                float range = l->getAttenuationRange();
                Ogre::Vector3 wpos = l->getDerivedPosition();
                // define cone space in terms of world space
                Ogre::Vector3 dir_z = dir_ws;
                Ogre::Vector3 dir_x = dir_z.perpendicular();
                Ogre::Vector3 dir_y = dir_z.crossProduct(dir_x);
                bool use_pyramid = outer >= 0.4;
                // define a bounding cuboid in cone space
                float outer2 = std::max(0.0f, outer); // stop it at the 90 degree point
                float cs_x_max = range * sqrtf(1 - outer2*outer2);
                if (use_pyramid) {
                        cs_x_max /= outer;
                };
                float cs_x_min = -cs_x_max;
                float cs_y_max = cs_x_max;
                float cs_y_min = cs_x_min;
                float cs_z_max = range;
                float cs_z_min = range * std::min(0.0f, outer);
                Ogre::Vector3 light_to_cam = cam_pos - wpos;

                // the 'inside' method always works but is less efficient as it
                // does not allow us to avoid lighting when there is an object
                // between the lit surface and the camera occluding the fragments
                // to be shaded.

                // on the other hand, the 'inside' method does have an optimisation
                // in the case where the light is between the camera and a fragment
                // but the fragment is out of range of the light

                // I am choosing to prefer the occlusion optimisation as lights that are
                // too far from surfaces should be avoided by good level design.

                // however 'inside' is the only method that can be used if the
                // camera is inside the light volume.

                bool inside = between<float>(light_to_cam.dotProduct(dir_x), cs_x_min-1, cs_x_max+1)
                           && between<float>(light_to_cam.dotProduct(dir_y), cs_y_min-1, cs_y_max+1)
                           && between<float>(light_to_cam.dotProduct(dir_z), cs_z_min-1, cs_z_max+1);

                //inside = true;

                Ogre::Vector3 cuboid_planes[] = {
                    cs_x_min * dir_x, cs_x_max * dir_x,
                    cs_y_min * dir_y, cs_y_max * dir_y,
                    cs_z_min * dir_z, cs_z_max * dir_z,
                };

                PointLightsGeometry &mdl_ = inside ? mdlInside : mdl;

                if (use_pyramid) {
                    mdl_.pyramidLight(wpos, cuboid_planes, dir_ws,diff,spec,inner,outer,range);
                } else {
                    mdl_.cubeLight(wpos, cuboid_planes, dir_ws,diff,spec,inner,outer,range);
                }
            }
            mdl.endLights();
            mdlInside.endLights();
            //CVERB << "Total lights: " << light_counter << std::endl;

            if (light_counter > 0) {

                GfxShaderGlobals globs = gfx_shader_globals_cam(pipe);

                deferred_lights->bindShader(GFX_GSL_PURPOSE_HUD, false, false, 0,
                                            globs, I, nullptr, 0, 1, texs, binds);

                for (unsigned i=0 ; i<3 ; ++i)
                    ogre_rs->_setTexture(NUM_GLOBAL_TEXTURES_NO_LIGHTING + i, true, pipe->getGBufferTexture(i));

                ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);
                ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
                ogre_rs->setStencilCheckEnabled(false);
                ogre_rs->_setDepthBias(0, 0);

                if (mdl.indexesUsed() > 0) {
                    ogre_rs->_setCullingMode(Ogre::CULL_CLOCKWISE);
                    ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_LESS_EQUAL);
                    ogre_rs->_render(mdl.getRenderOperation());
                }

                if (mdlInside.indexesUsed() > 0) {
                    ogre_rs->_setCullingMode(Ogre::CULL_ANTICLOCKWISE);
                    ogre_rs->_setDepthBufferParams(true, false, Ogre::CMPF_GREATER_EQUAL);
                    ogre_rs->_render(mdlInside.getRenderOperation());
                }

                for (unsigned i=0 ; i<NUM_GLOBAL_TEXTURES_NO_LIGHTING + 3 ; ++i) {
                    ogre_rs->_disableTextureUnit(i);
                }
            }

        } catch (const Exception &e) {
            CERR << "Rendering deferred point lights, got: " << e << std::endl;
        } catch (const Ogre::Exception &e) {
            CERR << "Rendering deferred point lights, got: " << e.getDescription() << std::endl;
        }

    }
    
};

// }}}



// {{{ Decals passes 

class DecalPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    public:
    DecalPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0, ""), pipe(pipe)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        if (pipe->getCameraOpts().decals) gfx_decal_render(pipe);
    }
    
};

// }}}



// {{{ Debug passes 

class DebugPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    public:
    DebugPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0, ""), pipe(pipe)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        gfx_debug_render(pipe);
    }
    
};

// }}}



// {{{ Particles passes 

class ParticlesPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    public:
    ParticlesPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0, ""), pipe(pipe)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        if (pipe->getCameraOpts().particles) gfx_particle_render(pipe);
    }
    
};

// }}}



// {{{ Tracer passes 

class TracerPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    public:
    TracerPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0, ""), pipe(pipe)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        if (pipe->getCameraOpts().tracers) gfx_tracer_body_render(pipe);
    }
    
};

// }}}



// {{{ Sky passes 

class SkyPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;

    public:
    SkyPasses (GfxPipeline *pipe)
      : Ogre::RenderQueueInvocation(0), pipe(pipe)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        try {
            if (pipe->getCameraOpts().sky) gfx_sky_render(pipe);
        } catch (Ogre::Exception &e) {
            CERR << "While rendering skies: " << e.getDescription() << std::endl;
        } catch (const Exception &e) {
            CERR << "While rendering skies: " << e << std::endl;
        }
    }
    
};

// }}}



// {{{ Reset pass

class ResetPass : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;
    unsigned int buffers;
    Vector3 colour;
    float depth;
    unsigned short stencil;

    public:
    ResetPass (GfxPipeline *pipe, unsigned int buffers,
               const Vector3 &colour=Vector3(1,1,1), float depth=1.0f, unsigned short stencil=0)
      : Ogre::RenderQueueInvocation(0), pipe(pipe), buffers(buffers),
        colour(colour), depth(depth), stencil(stencil)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        try {
            pipe->getCamera()->getViewport()->clear(buffers, to_ogre_cv(colour), depth, stencil);
        } catch (Ogre::Exception &e) {
            CERR << "While clearing buffer: " << e.getDescription() << std::endl;
        }
    }
    
};

// }}}



// {{{ First Person

class FirstPersonPasses : public Ogre::RenderQueueInvocation {
    GfxPipeline *pipe;
    bool alphaBlend;

    public:
    FirstPersonPasses (GfxPipeline *pipe, bool alpha_blend)
      : Ogre::RenderQueueInvocation(0), pipe(pipe), alphaBlend(alpha_blend)
    {
        setSuppressShadows(true);
    }

    void invoke (Ogre::RenderQueueGroup *, Ogre::SceneManager *)
    {
        try {
            if (pipe->getCameraOpts().firstPerson) gfx_body_render_first_person(pipe, alphaBlend);
        } catch (Ogre::Exception &e) {
            CERR << "While rendering first person: " << e.getDescription() << std::endl;
        } catch (const Exception &e) {
            CERR << "While rendering first person: " << e << std::endl;
        }
    }
    
};

// }}}



GfxPipeline::GfxPipeline (const std::string &name, Ogre::Viewport *target_viewport)
  : targetViewport(target_viewport)
{

    unsigned width = targetViewport->getActualWidth();
    unsigned height = targetViewport->getActualHeight();

    gBuffer = ogre_rs->createMultiRenderTarget(name+":G");
    // make textures for it
    const char *gbuffer_el_names[] = { ":G0", ":G1", ":G2", ":G3" };
    for (unsigned i=0 ; i<3 ; ++i) {
        gBufferElements[i] =
            Ogre::TextureManager::getSingleton().createManual(
                name+gbuffer_el_names[i], RESGRP, Ogre::TEX_TYPE_2D,
                width, height, 1,
                0,
                Ogre::PF_A8R8G8B8,
                Ogre::TU_RENDERTARGET,
                NULL,
                use_hwgamma);
        gBuffer->bindSurface(i, gBufferElements[i]->getBuffer()->getRenderTarget());
    }


    for (unsigned i=0 ; i<sizeof(hdrFb)/sizeof(*hdrFb) ; ++i) {
        std::stringstream ss; ss << i;
        hdrFb[i] = Ogre::TextureManager::getSingleton().createManual(
                    name+":hdrFb"+ss.str(), RESGRP, Ogre::TEX_TYPE_2D,
                    width, height, 1,
                    0,
                    Ogre::PF_FLOAT16_RGB,
                    Ogre::TU_RENDERTARGET,
                    NULL,
                    false);
        hdrFb[i]->getBuffer()->getRenderTarget()->setDepthBufferPool(
            Ogre::DepthBuffer::POOL_MANUAL_USAGE);
    }

    cam = ogre_sm->createCamera(name+":cam");
    cam->setAutoAspectRatio(true);

    rqisGbuffer = ogre_root->createRenderQueueInvocationSequence(name+":gbuffer");
    rqisGbuffer->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_GBUFFER_OPAQUE));

    // Opaque passes
    rqisDebug = ogre_root->createRenderQueueInvocationSequence(name+":debug");
    rqisDebug->add(new DeferredLightingPasses(this));

    // Opaque passes
    rqisDeferred = ogre_root->createRenderQueueInvocationSequence(name+":deferred");
    rqisDeferred->add(new DeferredLightingPasses(this));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_OPAQUE));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_OPAQUE_EMISSIVE));
    rqisDeferred->add(new SkyPasses(this));
    // Alpha passes
    rqisDeferred->add(new DecalPasses(this));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA_DEPTH));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA_DEPTH_EMISSIVE));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA));
    rqisDeferred->add(OGRE_NEW Ogre::RenderQueueInvocation(RQ_FORWARD_ALPHA_EMISSIVE));
    rqisDeferred->add(new ParticlesPasses(this));
    rqisDeferred->add(new TracerPasses(this));
    // Debug passes
    rqisDeferred->add(new DebugPasses(this));
    // First person passes
    rqisDeferred->add(new ResetPass(this, Ogre::FBT_DEPTH));
    rqisDeferred->add(new FirstPersonPasses(this, false));
    rqisDeferred->add(new FirstPersonPasses(this, true));
}

GfxPipeline::~GfxPipeline (void) {
    ogre_sm->destroyCamera(cam);

    // destroy MRT
    ogre_rs->destroyRenderTarget(gBuffer->getName());
    for (unsigned i=0 ; i<3 ; ++i) {
         Ogre::TextureManager::getSingleton().remove(gBufferElements[i]);
    }

    for (unsigned i=0 ; i<sizeof(hdrFb)/sizeof(*hdrFb) ; ++i) {
        Ogre::TextureManager::getSingleton().remove(hdrFb[i]);
    }

    ogre_root->destroyRenderQueueInvocationSequence(rqisGbuffer->getName());
    ogre_root->destroyRenderQueueInvocationSequence(rqisDeferred->getName());
    ogre_root->destroyRenderQueueInvocationSequence(rqisDebug->getName());
}

template<unsigned n> struct RenderQuadParams {
    Ogre::TexturePtr texs[n];
    unsigned next;
    GfxShader *shader;
    Ogre::SceneBlendFactor targetBlend;
    RenderQuadParams(GfxShader *shader, Ogre::SceneBlendFactor target_blend=Ogre::SBF_ZERO)
      : next(0), shader(shader), targetBlend(target_blend)
    { }
    RenderQuadParams<n> tex (const Ogre::TexturePtr &t) {
        APP_ASSERT(next < n);
        texs[next] = t;
        next++;
        return *this;
    };
};

// Must assign textures in alphabetical order by name in tex_binds
static void render_quad (
    GfxPipeline *p,
    Ogre::Viewport *viewport,
    GfxShader *shader,
    const GfxTextureStateMap &texs,
    const GfxShaderBindings &binds,
    size_t num_texs,
    const Ogre::TexturePtr **tex_arr,  // array of length num_texs
    Ogre::SceneBlendFactor target_blend)
{
    ogre_rs->_setViewport(viewport);
    ogre_rs->_beginFrame();

    GfxShaderGlobals globs = gfx_shader_globals_cam_ss(p, viewport);

    shader->bindShader(
        GFX_GSL_PURPOSE_HUD, false, false, 0, globs,
        Ogre::Matrix4::IDENTITY, nullptr, 0, 1, texs, binds);

    ogre_rs->_setCullingMode(Ogre::CULL_NONE);
    ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);

    for (unsigned i=0 ; i<num_texs ; ++i) {
        unsigned index = NUM_GLOBAL_TEXTURES_NO_LIGHTING + i;
        ogre_rs->_setTexture(index, true, *tex_arr[i]);
        ogre_rs->_setTextureUnitFiltering(index, Ogre::FT_MIN, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(index, Ogre::FT_MAG, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(index, Ogre::FT_MIP, Ogre::FO_NONE);
        Ogre::TextureUnitState::UVWAddressingMode am;
        am.u = Ogre::TextureUnitState::TAM_CLAMP;
        am.v = Ogre::TextureUnitState::TAM_CLAMP;
        am.w = Ogre::TextureUnitState::TAM_CLAMP;
        ogre_rs->_setTextureAddressingMode(index, am);
    }

    ogre_rs->_setCullingMode(Ogre::CULL_NONE);
    ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);
    ogre_rs->_setSceneBlending(Ogre::SBF_ONE, target_blend);
    ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
    ogre_rs->setStencilCheckEnabled(false);
    ogre_rs->_setDepthBias(0, 0);


    // render the quad
    Ogre::RenderOperation op;
    op.useIndexes = false;
    op.vertexData = screen_quad_vdata;
    op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    ogre_rs->_render(op);

    for (unsigned i=0 ; i<num_texs ; ++i)
        ogre_rs->_disableTextureUnit(NUM_GLOBAL_TEXTURES_NO_LIGHTING + i);

    ogre_rs->_endFrame();

}

// Render onto the given RTT, set appropriate viewport_size and manage a
// temporary viewport for this render.
static void render_quad (
    GfxPipeline *p,
    Ogre::RenderTarget *rt,
    const Ogre::Vector4 &win,
    GfxShader *shader,
    const GfxTextureStateMap &texs,
    const GfxShaderBindings &binds,
    size_t num_texs,
    const Ogre::TexturePtr **tex_arr,  // array of length num_texs
    Ogre::SceneBlendFactor target_blend)
{
    Ogre::Viewport *vp = rt->addViewport(NULL, 0, win.x, win.y, win.z, win.w);
    render_quad(p, vp, shader, texs, binds, num_texs, tex_arr, target_blend);
    rt->removeViewport(vp->getZOrder());
}

// Render onto the given RTT
static void render_quad (
    GfxPipeline *p,
    const Ogre::TexturePtr &rtt,
    const Ogre::Vector4 &win,
    GfxShader *shader,
    const GfxTextureStateMap &texs,
    const GfxShaderBindings &binds,
    size_t num_texs,
    const Ogre::TexturePtr **tex_arr,  // array of length num_texs
    Ogre::SceneBlendFactor target_blend)
{
    render_quad(p, rtt->getBuffer()->getRenderTarget(), win,
                 shader, texs, binds, num_texs, tex_arr, target_blend);
}

static void clear_rt (Ogre::RenderTarget *rt)
{
    Ogre::Viewport *vp = rt->addViewport(NULL);
    ogre_rs->_setViewport(vp);
    ogre_rs->clearFrameBuffer(Ogre::FBT_COLOUR);
    rt->removeViewport(vp->getZOrder());
}

static void clear_rt (const Ogre::TexturePtr &rtt)
{
    return clear_rt(rtt->getBuffer()->getRenderTarget());
}

void GfxPipeline::render (const CameraOpts &cam_opts, bool additive)
{
    opts = cam_opts;

    cam->setPosition(to_ogre(opts.pos));
    // Ogre cameras point towards Z whereas in Grit the convention is that 'unrotated' means pointing towards y (north)
    cam->setOrientation(to_ogre(opts.dir*Quaternion(Degree(90),Vector3(1,0,0))));
    cam->setFOVy(Ogre::Degree(opts.fovY));
    cam->setNearClipDistance(opts.nearClip);
    cam->setFarClipDistance(opts.farClip);
    cam->setFrustumOffset(opts.frustumOffset);
    cam->setFocalLength(1);

    Ogre::Viewport *vp;

    // populate gbuffer
    vp = gBuffer->addViewport(cam);

    if (gfx_option(GFX_UPDATE_MATERIALS)) {
        GfxShaderGlobals globs = gfx_shader_globals_cam(this);
        for (const auto &pair : material_db) {
            GfxMaterial *m = dynamic_cast<GfxMaterial*>(pair.second);
            if (m == nullptr) continue;
            // CVERB << "Rebuilding: " << m->name << std::endl;
            m->updateOgreMaterials(globs);
        }
    }

    vp->setShadowsEnabled(true);
    // white here makes sure that the depth (remember that it is 3 bytes) is maximal
    vp->setBackgroundColour(Ogre::ColourValue::White);
    vp->setRenderQueueInvocationSequenceName(rqisGbuffer->getName());
    unsigned long long micros_before = micros();
    vp->update();
    unsigned long long micros_after_gbuffer = micros();
    gBuffer->removeViewport(vp->getZOrder());
    gBufferStats.batches = ogre_rs->_getBatchCount();
    gBufferStats.triangles = ogre_rs->_getFaceCount();;
    gBufferStats.micros = micros_after_gbuffer - micros_before;

    if (!opts.bloomAndToneMap || opts.debugMode > 0) {

        // render gbuffer and alpha, sky, etc into ldr window
        vp = targetViewport;
        vp->setBackgroundColour(Ogre::ColourValue(0.3, 0.3, 0.3));
        vp->setCamera(cam);
        vp->setShadowsEnabled(false);
        if (opts.debugMode > 0) {
            vp->setRenderQueueInvocationSequenceName(rqisDebug->getName());
        } else {
            vp->setRenderQueueInvocationSequenceName(rqisDeferred->getName());
        }
        vp->update();
        unsigned long long micros_after_deferred = micros();
        deferredStats.batches = ogre_rs->_getBatchCount();
        deferredStats.triangles = ogre_rs->_getFaceCount();
        deferredStats.micros = micros_after_deferred - micros_after_gbuffer;
        return;

    }

    // render from gbuffer, the alpha passes, sky, etc into hdr viewport
    hdrFb[0]->getBuffer()->getRenderTarget()->attachDepthBuffer(gBuffer->getDepthBuffer());
    vp = hdrFb[0]->getBuffer()->getRenderTarget()->addViewport(cam);
    vp->setClearEveryFrame(true, Ogre::FBT_COLOUR);
    vp->setBackgroundColour(Ogre::ColourValue(0.3, 0.3, 0.3));
    vp->setShadowsEnabled(false);
    vp->setRenderQueueInvocationSequenceName(rqisDeferred->getName());
    vp->update();
    unsigned long long micros_after_deferred = micros();
    deferredStats.batches = ogre_rs->_getBatchCount();
    deferredStats.triangles = ogre_rs->_getFaceCount();
    deferredStats.micros = micros_after_deferred - micros_after_gbuffer;
    hdrFb[0]->getBuffer()->getRenderTarget()->removeViewport(vp->getZOrder());
    hdrFb[0]->getBuffer()->getRenderTarget()->detachDepthBuffer();


    Ogre::SceneBlendFactor target_blend = additive ? Ogre::SBF_ONE : Ogre::SBF_ZERO;

    unsigned bloom_iterations = gfx_option(GFX_BLOOM_ITERATIONS);

    if (bloom_iterations == 0) {

        // hdrFb[0] is an Ogre::TexturePtr and will never be a GfxDiskResource, so
        // bind it explicitly with Ogre-level calls in render_quad.
        GfxTextureStateMap texs;
        texs["colourGradeLut"] = gfx_texture_state_anisotropic(nullptr, GFX_AM_CLAMP);
        texs["hdr"] = gfx_texture_state_point(nullptr);
        GfxShaderBindings binds;
        binds["colourGradeLut"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE3, 1,1,1,1);
        binds["hdr"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1,1,1,1);

        const Ogre::TexturePtr *ogre_texs[] = {
            &colour_grade_lut->getOgreTexturePtr(),
            &hdrFb[0],
        };
        render_quad(this, targetViewport, compositor_tonemap, texs, binds,
                    2, ogre_texs, target_blend);

        return;
    }

    GfxTextureStateMap texs;
    texs["colourGradeLut"] = gfx_texture_state_anisotropic(nullptr, GFX_AM_CLAMP);
    texs["original"] = gfx_texture_state_point(nullptr);
    texs["srcTex"] = gfx_texture_state_point(nullptr);
    GfxShaderBindings binds;
    binds["colourGradeLut"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE3, 1,1,1,1);
    binds["original"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1,1,1,1);
    binds["srcTex"] = GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1,1,1,1);

    const Ogre::TexturePtr *ogre_texs[3];
    ogre_texs[0] = &colour_grade_lut->getOgreTexturePtr();

    // half_bloom = filter_then_horz_blur(raw)
    // loop 1..n-1 {
    //     bloom = vert_blur(half_bloom);
    //     half_bloom = scale_then_horz_blur(bloom);
    // }
    // fb = vert_blur_combine_and_tonemap(raw, half_bloom) 

    Ogre::Vector4 vp_dim(0,0,1,1);

    binds["bloomTexScale"] = GfxGslParam::float1(vp_dim.z);
    ogre_texs[1] = &hdrFb[0];
    ogre_texs[2] = &hdrFb[0];
    render_quad(this, hdrFb[1], vp_dim, compositor_exposure_filter_then_horz_blur,
                 texs, binds, 3, ogre_texs, Ogre::SBF_ZERO);

    for (unsigned i=1 ; i<bloom_iterations ; ++i) {
        clear_rt(hdrFb[2]);
        ogre_texs[1] = &hdrFb[1];
        ogre_texs[2] = &hdrFb[1];
        render_quad(this, hdrFb[2], vp_dim, compositor_vert_blur,
                     texs, binds, 3, ogre_texs, Ogre::SBF_ZERO);
        vp_dim /= 2;

        clear_rt(hdrFb[1]);
        ogre_texs[1] = &hdrFb[2];
        ogre_texs[2] = &hdrFb[2];
        render_quad(this, hdrFb[1], vp_dim, compositor_horz_blur,
                     texs, binds, 3, ogre_texs, Ogre::SBF_ZERO);

        binds["bloomTexScale"] = GfxGslParam::float1(vp_dim.z);
    }
    // Note that hdrFb[0] is original, hdrFb[1] is srcTex, which are in alphabetical order.
    ogre_texs[1] = &hdrFb[0];
    ogre_texs[2] = &hdrFb[1];
    render_quad(this, targetViewport, compositor_vert_blur_combine_tonemap,
                 texs, binds, 3, ogre_texs, target_blend);

}
