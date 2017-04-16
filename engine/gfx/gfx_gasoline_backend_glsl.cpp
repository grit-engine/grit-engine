/* Copyright (c) The Grit Game Engine authors 2016
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

#include <centralised_log.h>

#include "gfx_gasoline_backend.h"
#include "gfx_gasoline_backend_cg.h"

static std::map<std::string, std::string> vert_global = {
    {"position", "vertex"},
    {"normal", "normal"},
    {"tangent", "tangent"},
    {"colour", "colour"},
    {"coord0", "uv0"},
    {"coord1", "uv1"},
    {"coord2", "uv2"},
    {"coord3", "uv3"},
    {"coord4", "uv4"},
    {"coord5", "uv5"},
    {"coord6", "uv6"},
    {"coord7", "uv7"},
    {"boneWeights", "blendWeights"},
    {"boneAssignments", "blendIndices"},
};

static std::string preamble (bool glsl33)
{
    std::stringstream ss;
    if (glsl33) {
        ss << "#version 330\n";
        ss << "#extension GL_ARB_separate_shader_objects: require\n";
    } else {
        ss << "#version 130\n";
    }
        
    ss << "// This GLSL shader compiled from Gasoline, the Grit shading language.\n";
    ss << "\n";

    ss << "// GSL/GLSL Preamble:\n";
    ss << "#define Int int\n";
    ss << "#define Int2 ivec2\n";
    ss << "#define Int3 ivec3\n";
    ss << "#define Int4 ivec4\n";
    ss << "#define Float float\n";
    ss << "#define Float2 vec2\n";
    ss << "#define Float3 vec3\n";
    ss << "#define Float4 vec4\n";
    ss << "#define Float2x2 mat2x2\n";
    ss << "#define Float2x3 mat3x2\n";
    ss << "#define Float2x4 mat4x2\n";
    ss << "#define Float3x2 mat2x3\n";
    ss << "#define Float3x3 mat3x3\n";
    ss << "#define Float3x4 mat4x3\n";
    ss << "#define Float4x2 mat2x4\n";
    ss << "#define Float4x3 mat3x4\n";
    ss << "#define Float4x4 mat4x4\n";
    ss << "#define FloatTexture sampler1D\n";
    ss << "#define FloatTexture2 sampler2D\n";
    ss << "#define FloatTexture3 sampler3D\n";
    ss << "#define FloatTextureCube samplerCube\n";
    ss << "\n";

    return ss.str();
}

static std::string generate_funcs (const GfxGslEnvironment &env)
{
    std::stringstream ss;

    ss << "// Standard library\n";
    ss << "Float strength (Float p, Float n) { return pow(max(0.00000001, p), n); }\n";
    ss << "Float atan2 (Float y, Float x) { return atan(y, x); }\n";
    ss << "Float2 mul (Float2x2 m, Float2 v) { return m * v; }\n";
    ss << "Float2 mul (Float2x3 m, Float3 v) { return m * v; }\n";
    ss << "Float2 mul (Float2x4 m, Float4 v) { return m * v; }\n";
    ss << "Float3 mul (Float3x2 m, Float2 v) { return m * v; }\n";
    ss << "Float3 mul (Float3x3 m, Float3 v) { return m * v; }\n";
    ss << "Float3 mul (Float3x4 m, Float4 v) { return m * v; }\n";
    ss << "Float4 mul (Float4x2 m, Float2 v) { return m * v; }\n";
    ss << "Float4 mul (Float4x3 m, Float3 v) { return m * v; }\n";
    ss << "Float4 mul (Float4x4 m, Float4 v) { return m * v; }\n";
    ss << "Float lerp (Float a, Float b, Float v) { return v*b + (1-v)*a; }\n";
    ss << "Float2 lerp (Float2 a, Float2 b, Float2 v) { return v*b + (Float2(1,1)-v)*a; }\n";
    ss << "Float3 lerp (Float3 a, Float3 b, Float3 v) { return v*b + (Float3(1,1,1)-v)*a; }\n";
    ss << "Float4 lerp (Float4 a, Float4 b, Float4 v) { return v*b + (Float4(1,1,1,1)-v)*a; }\n";

    // TODO(dcunnin): Move this to a body section
    ss << "uniform Float4x4 body_boneWorlds[50];\n";

    // Shadow casting:
    ss << "uniform Float4x4 internal_shadow_view_proj;\n";
    ss << "uniform Float internal_shadow_additional_bias;\n";

    ss << "uniform Float internal_rt_flip;\n";
    ss << "uniform Float4x4 internal_inv_world;\n";
    if (!env.instanced) {
        // When instanced, comes from vertex coord.
        ss << "uniform Float internal_fade;\n";
    }
    ss << "\n";

    ss << gfx_gasoline_generate_preamble_functions();

    return ss.str();
}

static std::string generate_funcs_vert (const GfxGslEnvironment &env)
{
    std::stringstream ss;
    ss << "// Standard library (vertex shader specific calls)\n";
    ss << "\n";

    if (env.instanced) {
        ss << "Float4x4 get_inst_matrix()\n";
        ss << "{\n";
        ss << "    return Float4x4(\n";
        ss << "        vert_coord1[0], vert_coord2[0], vert_coord3[0], 0.0,\n";
        ss << "        vert_coord1[1], vert_coord2[1], vert_coord3[1], 0.0,\n";
        ss << "        vert_coord1[2], vert_coord2[2], vert_coord3[2], 0.0,\n";
        ss << "        vert_coord4[0], vert_coord4[1], vert_coord4[2], 1.0);\n";
        ss << "}\n";
        ss << "\n";
    }

    return ss.str();
}

static std::string generate_funcs_frag (void)
{

    std::stringstream ss;
    ss << "// Standard library (fragment shader specific calls)\n";
    ss << "Float ddx (Float v) { return dFdx(v); }\n";
    ss << "Float ddy (Float v) { return dFdy(v); }\n";
    ss << "Float2 ddx (Float2 v) { return dFdx(v); }\n";
    ss << "Float2 ddy (Float2 v) { return dFdy(v); }\n";
    ss << "Float3 ddx (Float3 v) { return dFdx(v); }\n";
    ss << "Float3 ddy (Float3 v) { return dFdy(v); }\n";
    ss << "Float4 ddx (Float4 v) { return dFdx(v); }\n";
    ss << "Float4 ddy (Float4 v) { return dFdy(v); }\n";

    // Real texture lookups
    // 2D
    ss << "Float4 sample (FloatTexture2 tex, Float2 uv) { return texture(tex, uv); }\n";
    ss << "Float4 sampleGrad (FloatTexture2 tex, Float2 uv, Float2 ddx, Float2 ddy)\n";
    ss << "{ return textureGrad(tex, uv, ddx, ddy); }\n";
    ss << "Float4 sampleLod (FloatTexture2 tex, Float2 uv, Float lod)\n";
    ss << "{ return textureLod(tex, uv, lod); }\n";

    // 3D (volumetric)
    ss << "Float4 sample (FloatTexture3 tex, Float3 uvw) { return texture(tex, uvw); }\n";
    ss << "Float4 sampleGrad (FloatTexture3 tex, Float3 uvw, Float3 ddx, Float3 ddy)\n";
    ss << "{ return textureGrad(tex, uvw, ddx, ddy); }\n";
    ss << "Float4 sampleLod (FloatTexture3 tex, Float3 uvw, Float lod)\n";
    ss << "{ return textureLod(tex, uvw, lod); }\n";

    // Cube
    ss << "Float4 sample (FloatTextureCube tex, Float3 uvw) { return texture(tex, uvw); }\n";
    ss << "Float4 sampleGrad (FloatTextureCube tex, Float3 uvw, Float3 ddx, Float3 ddy)\n";
    ss << "{ return textureGrad(tex, uvw, ddx, ddy); }\n";
    ss << "Float4 sampleLod (FloatTextureCube tex, Float3 uvw, Float lod)\n";
    ss << "{ return textureLod(tex, uvw, lod); }\n";

    // 'Fake' texture lookups
    ss << "Float4 sample (Float4 c, Float2 uv) { return c; }\n";
    ss << "Float4 sample (Float4 c, Float3 uvw) { return c; }\n";
    ss << "Float4 sampleGrad (Float4 c, Float2 uv, Float2 ddx, Float2 ddy) { return c; }\n";
    ss << "Float4 sampleGrad (Float4 c, Float3 uvw, Float3 ddx, Float3 ddy) { return c; }\n";
    ss << "Float4 sampleLod (Float4 c, Float2 uv, Float lod) { return c; }\n";
    ss << "Float4 sampleLod (Float4 c, Float3 uvw, Float lod) { return c; }\n";

    ss << "\n";
    return ss.str();
}

static std::string generate_vert_header (bool glsl33,
                                         const GfxGslContext &ctx,
                                         const GfxGslTypeSystem *ts,
                                         const std::vector<GfxGslTrans> &trans,
                                         const std::set<std::string> &vert_in)
{
    std::stringstream ss;

    ss << "// Vertex header\n";

    // In (vertex attributes)
    for (const auto &f : vert_in) {
        // I don't think it's possible to use the layout qualifier here, without
        // changing (or at least examining) the way that Ogre::Mesh maps to gl buffers.
        ss << "in " << ts->getVertType(f) << " " << vert_global[f] << ";\n";
        ss << ts->getVertType(f) << " vert_" << f << ";\n";
    }
    ss << gfx_gasoline_generate_global_fields(ctx, false);
    // Out (position)
    if (glsl33) {
        ss << "out gl_PerVertex\n";
        ss << "{\n";
        // invariant is required to avoid z fighitng on multi-pass rendering techniques.
        // Unfortunately it is broken on Intel GL.
        // ss << "    invariant vec4 gl_Position;\n";
        ss << "    vec4 gl_Position;\n";
        ss << "    float gl_PointSize;\n";
        ss << "    float gl_ClipDistance[];\n";
        ss << "};\n";
    }
    // Out (trans)
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        if (glsl33) {
            ss << "layout(location = " << i / 4 << ") ";
        }
        ss << "out " << type.str() << " trans" << i/4 << ";\n";
    }

    ss << "\n";

    return ss.str();
}

static std::string generate_frag_header(bool glsl33, const GfxGslContext &ctx,
                                        const std::vector<GfxGslTrans> &trans, bool gbuffer)
{
    std::stringstream ss;

    ss << "// Fragment header\n";

    ss << "Float2 frag_screen;\n";

    // In (trans)
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        if (glsl33) {
            ss << "layout(location = " << i / 4 << ") ";
        }
        ss << "in " << type.str() << " trans" << i/4 << ";\n";
    }

    ss << gfx_gasoline_generate_global_fields(ctx, false);

    // Out
    if (gbuffer) {
        if (glsl33) {
            ss << "layout(location = 0) out Float4 out_gbuffer0;\n";
            ss << "layout(location = 1) out Float4 out_gbuffer1;\n";
            ss << "layout(location = 2) out Float4 out_gbuffer2;\n";
        } else {
            // #version 130 only supports this:
            ss << "out Float4 out_gbuffer0, out_gbuffer1, out_gbuffer2;\n";
        }
    } else {
        if (glsl33) {
            ss << "layout(location = 0) ";
        }
        ss << "out Float4 out_colour_alpha;\n";
    }
    //ss << "layout (depth_any) out Float out_depth;\n";


    ss << "\n";

    return ss.str();
}

void gfx_gasoline_unparse_glsl (const GfxGslContext &ctx,
                                const GfxGslTypeSystem *vert_ts,
                                const GfxGslAst *vert_ast,
                                std::string &vert_output,
                                const GfxGslTypeSystem *frag_ts,
                                const GfxGslAst *frag_ast,
                                std::string &frag_output,
                                const GfxGslEnvironment &env,
                                bool glsl33,
                                bool flat_z,
                                bool das)
{
    GfxGslTypeMap vert_vars, frag_vars;
    std::set<GfxGslTrans> trans_set;
    std::stringstream header;
    header << "// env: " << env << "\n";
    header << "// flat_z: " << flat_z << "\n";
    header << "// das: " << das << "\n";
    header << "\n";

    GfxGslBackendUnparser vert_backend("user_");
    GfxGslBackendUnparser frag_backend("user_");

    for (const auto &pair : vert_ts->getVars())
        vert_vars["user_" + pair.first] = pair.second->type;
    vert_backend.unparse(vert_ast, 1);

    for (const auto &pair : frag_ts->getVars())
        frag_vars["user_" + pair.first] = pair.second->type;
    frag_backend.unparse(frag_ast, 1);

    trans_set.insert(frag_ts->getTrans().begin(), frag_ts->getTrans().end());

    std::set<std::string> vert_in = vert_ts->getVertFieldsRead();
    vert_in.insert("position");
    if (env.boneWeights > 0) {
        vert_in.insert("boneAssignments");
        vert_in.insert("boneWeights");
    }

    std::vector<GfxGslTrans> trans(trans_set.begin(), trans_set.end());
    std::map<std::string, const GfxGslFloatType *> internals;
    gfx_gasoline_add_internal_trans(internals, trans);

    for (const auto &tran : trans) {
        const std::string &tv = tran.path[0];
        switch (tran.kind) {
            case GfxGslTrans::VERT:
            vert_in.insert(tv);
            frag_vars["vert_" + tv] = tran.type;
            break;
            case GfxGslTrans::INTERNAL:
            vert_vars["internal_" + tv] = tran.type;
            frag_vars["internal_" + tv] = tran.type;
            break;
            case GfxGslTrans::USER:
            frag_vars["user_" + tv] = tran.type;
            break;
        }
    }



    // VERTEX

    std::stringstream vert_ss;
    vert_ss << preamble(glsl33);
    vert_ss << header.str();
    vert_ss << generate_vert_header(glsl33, ctx, vert_ts, trans, vert_in);
    vert_ss << generate_funcs(env);
    vert_ss << generate_funcs_vert(env);
    vert_ss << gfx_gasoline_preamble_transformation(false, env);
    vert_ss << gfx_gasoline_generate_var_decls(vert_vars);
    vert_ss << vert_backend.getUserVertexFunction();
    vert_ss << "void main (void)\n";
    vert_ss << "{\n";
    for (const auto &f : vert_in)
        vert_ss << "    vert_" << f << " = " << vert_global[f] << ";\n";
    vert_ss << "    Float3 world_pos;\n";
    vert_ss << "    func_user_vertex(world_pos);\n";
    if (das) {
        vert_ss << "    Float4 clip_pos = Float4(world_pos, 1);\n";
    } else {
        // For additive passes to compute same depth as the gbuffer passes, do not
        // multiple by viewproj here.
        vert_ss << "    Float3 pos_vs = mul(global_view, Float4(world_pos, 1)).xyz;\n";
        vert_ss << "    Float4 clip_pos = mul(global_proj, Float4(pos_vs, 1));\n";
    }
    vert_ss << "    clip_pos.y *= internal_rt_flip;\n";
    // Hack to maximum depth, but avoid hitting the actual backplane.
    // (Without this dcunnin saw some black lightning-like artifacts on Nvidia.)
    if (flat_z)
        vert_ss << "    clip_pos.z = clip_pos.w * (1 - 1.0/65536);\n";
    vert_ss << "    gl_Position = clip_pos;\n";
    vert_ss << gfx_gasoline_generate_trans_encode(trans, "user_");
    vert_ss << "}\n";

    vert_output = vert_ss.str();


    // FRAGMENT

    std::stringstream frag_ss;
    frag_ss << preamble(glsl33);
    vert_ss << header.str();
    frag_ss << generate_frag_header(glsl33, ctx, trans, false);
    frag_ss << generate_funcs(env);
    frag_ss << generate_funcs_frag();
    if (ctx.lightingTextures)
        frag_ss << gfx_gasoline_preamble_lighting(env);
    frag_ss << gfx_gasoline_preamble_fade(env);
    frag_ss << gfx_gasoline_generate_var_decls(frag_vars);
    frag_ss << frag_backend.getUserColourAlphaFunction();
    frag_ss << "void main (void)\n";
    frag_ss << "{\n";
    frag_ss << "    frag_screen = gl_FragCoord.xy;\n";
    frag_ss << "    if (internal_rt_flip < 0)\n";
    frag_ss << "        frag_screen.y = global_viewportSize.y - frag_screen.y;\n";
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "vert_", GfxGslTrans::VERT);
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "user_", GfxGslTrans::USER);
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "internal_", GfxGslTrans::INTERNAL);
    frag_ss << "    Float3 out_colour;\n";
    frag_ss << "    Float out_alpha;\n";
    frag_ss << "    func_user_colour(out_colour, out_alpha);\n";
    if (env.fadeDither) {
        frag_ss << "    fade();\n";
    } else {
        frag_ss << "    out_alpha *= internal_fade;\n";
    }
    frag_ss << "    out_colour_alpha = Float4(out_colour * out_alpha, out_alpha);\n";
    if (das) {
        // Whether we are using d3d9 or gl rendersystems,
        // ogre gives us the view_proj in a 'standard' form, which is
        // right-handed with a depth range of [-1,+1].
        // Since we are outputing depth in the fragment shader, the range is [0,1]
        // Note that this code pre-supposes that all DAS shaders create a user variable
        // called pos_ws, but they are all internal shaders so we can guarantee that.
        // The pos_ws needs to be a point within the frustum.
        // frag_ss << "    Float4 projected = mul(global_viewProj, Float4(user_pos_ws, 1));\n";
        // frag_ss << "    gl_FragDepth = 0.5 + (projected.z / projected.w) / 2.0;\n";
    }
    frag_ss << "}\n";

    frag_output = frag_ss.str();
}

void gfx_gasoline_unparse_body_glsl(const GfxGslContext &ctx,
                                    const GfxGslTypeSystem *vert_ts,
                                    const GfxGslAst *vert_ast,
                                    const GfxGslTypeSystem *dangs_ts,
                                    const GfxGslAst *dangs_ast,
                                    const GfxGslTypeSystem *additional_ts,
                                    const GfxGslAst *additional_ast,
                                    std::string &vert_out,
                                    std::string &frag_out,
                                    const GfxGslEnvironment &env,
                                    bool glsl33,
                                    bool first_person,
                                    bool wireframe,
                                    bool forward_only,
                                    bool cast)
{
    GfxGslTypeMap vert_vars, frag_vars;
    std::set<GfxGslTrans> trans_set;
    std::stringstream header;
    header << "// env: " << env << "\n";
    header << "// first_person: " << first_person << "\n";
    header << "// wireframe: " << wireframe << "\n";
    header << "// forward_only: " << forward_only << "\n";
    header << "// cast: " << cast << "\n";
    header << "\n";

    GfxGslBackendUnparser vert_backend("uvert_");
    GfxGslBackendUnparser dangs_backend( "udangs_");
    GfxGslBackendUnparser additional_backend("uadd_");

    std::set<std::string> vert_in = vert_ts->getVertFieldsRead();
    for (const auto &pair : vert_ts->getVars())
        vert_vars["uvert_" + pair.first] = pair.second->type;
    vert_backend.unparse(vert_ast, 1);

    bool using_additional = !(cast || forward_only);

    if (!wireframe) {
        for (const auto &pair : dangs_ts->getVars())
            frag_vars["udangs_" + pair.first] = pair.second->type;
        dangs_backend.unparse(dangs_ast, 1);
        trans_set.insert(dangs_ts->getTrans().begin(), dangs_ts->getTrans().end());
    }

    if (using_additional) {
        for (const auto &pair : additional_ts->getVars())
            frag_vars["uadd_" + pair.first] = pair.second->type;
        additional_backend.unparse(additional_ast, 1);
        trans_set.insert(additional_ts->getTrans().begin(), additional_ts->getTrans().end());
    }

    std::vector<GfxGslTrans> trans(trans_set.begin(), trans_set.end());

    std::map<std::string, const GfxGslFloatType *> internals;
    auto *f1 = ctx.alloc.makeType<GfxGslFloatType>(1);
    auto *f3 = ctx.alloc.makeType<GfxGslFloatType>(3);
    if (cast) {
        internals["light_dist"] = f1;
    } else if (!wireframe) {
        internals["vertex_to_cam"] = f3;
        if (!first_person)
            internals["cam_dist"] = f1;
    }
    if (env.instanced) {
        internals["fade"] = f1;
    }

    gfx_gasoline_add_internal_trans(internals, trans);

    vert_in.insert("position");
    if (env.instanced) {
        vert_in.insert("coord1");
        vert_in.insert("coord2");
        vert_in.insert("coord3");
        vert_in.insert("coord4");
        vert_in.insert("coord5");
    }
    if (env.boneWeights > 0) {
        vert_in.insert("boneAssignments");
        vert_in.insert("boneWeights");
    }

    for (const auto &tran : trans) {
        const std::string &tv = tran.path[0];
        switch (tran.kind) {
            case GfxGslTrans::VERT:
            vert_in.insert(tv);
            frag_vars["vert_" + tv] = tran.type;
            break;

            case GfxGslTrans::INTERNAL:
            vert_vars["internal_" + tv] = tran.type;
            frag_vars["internal_" + tv] = tran.type;
            break;

            case GfxGslTrans::USER:
            if (!wireframe)
                frag_vars["udangs_" + tv] = tran.type;
            if (using_additional)
                frag_vars["uadd_" + tv] = tran.type;
            break;
        }
    }


    // VERTEX

    std::stringstream vert_ss;
    vert_ss << preamble(glsl33);
    vert_ss << header.str();
    vert_ss << generate_vert_header(glsl33, ctx, vert_ts, trans, vert_in);
    vert_ss << generate_funcs(env);
    vert_ss << generate_funcs_vert(env);
    vert_ss << gfx_gasoline_preamble_transformation(first_person, env);
    vert_ss << gfx_gasoline_generate_var_decls(vert_vars);
    vert_ss << vert_backend.getUserVertexFunction();
    vert_ss << "void main (void)\n";
    vert_ss << "{\n";
    for (const auto &f : vert_in)
        vert_ss << "    vert_" << f << " = " << vert_global[f] << ";\n";
    vert_ss << "    Float3 pos_ws;\n";
    vert_ss << "    func_user_vertex(pos_ws);\n";
    if (cast) {
        vert_ss << "    gl_Position = mul(internal_shadow_view_proj, Float4(pos_ws, 1));\n";
        vert_ss << "    internal_light_dist = dot(global_sunlightDirection, pos_ws);\n";
    } else {
        vert_ss << "    Float3 pos_vs = mul(global_view, Float4(pos_ws, 1)).xyz;\n";
        vert_ss << "    gl_Position = mul(global_proj, Float4(pos_vs, 1));\n";
        vert_ss << "    gl_Position.y *= internal_rt_flip;\n";
        if (!wireframe)
            vert_ss << "    internal_vertex_to_cam = global_cameraPos - pos_ws;\n";
        if (!first_person)
            vert_ss << "    internal_cam_dist = -pos_vs.z;\n";
    }
    if (env.instanced) {
        // The 0.5/255 is necessary because apparently we lose some precision through rasterisation.
        vert_ss << "    internal_fade = vert_coord5.x + 0.5/255;\n";
    }
    vert_ss << gfx_gasoline_generate_trans_encode(trans, "uvert_");
    vert_ss << "}\n";

    vert_out = vert_ss.str();


    // FRAGMENT

    std::stringstream frag_ss;
    frag_ss << preamble(glsl33);
    frag_ss << header.str();
    frag_ss << generate_frag_header(glsl33, ctx, trans, forward_only);
    frag_ss << generate_funcs(env);
    frag_ss << generate_funcs_frag();
    frag_ss << gfx_gasoline_generate_var_decls(frag_vars);
    if (ctx.lightingTextures)
        frag_ss << gfx_gasoline_preamble_lighting(env);
    frag_ss << gfx_gasoline_preamble_fade(env);
    if (!wireframe)
        frag_ss << dangs_backend.getUserDangsFunction();
    if (using_additional)
        frag_ss << additional_backend.getUserColourAlphaFunction();

    // Main function
    frag_ss << "void main (void)\n";
    frag_ss << "{\n";
    frag_ss << "    frag_screen = gl_FragCoord.xy;\n";
    frag_ss << "    if (internal_rt_flip < 0)\n";
    frag_ss << "        frag_screen.y = global_viewportSize.y - frag_screen.y;\n";
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "vert_", GfxGslTrans::VERT);
    if (!wireframe)
        frag_ss << gfx_gasoline_generate_trans_decode(trans, "udangs_", GfxGslTrans::USER);
    if (using_additional)
        frag_ss << gfx_gasoline_generate_trans_decode(trans, "uadd_", GfxGslTrans::USER);
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "internal_", GfxGslTrans::INTERNAL);
    if (cast) {
        // Bias
        // TODO: Consider using http://www.dissidentlogic.com/old/#Normal%20Offset%20Shadows
        // Run ddx/ddy before possible discard.
        frag_ss << "    Float slope_bias = abs(ddx(internal_light_dist))\n";
        frag_ss << "                       + abs(ddy(internal_light_dist));\n";
    }
    if (env.fadeDither) {
        frag_ss << "    fade();\n";
    }
    if (cast) {
        // Run this only for the discard.  Ignore actual value outputs.
        frag_ss << "    Float3 d;\n";
        frag_ss << "    Float a;\n";
        frag_ss << "    Float3 n;\n";
        frag_ss << "    Float g;\n";
        frag_ss << "    Float s;\n";
        frag_ss << "    func_user_dangs(d, a, n, g, s);\n";

        // On large flat surfaces, with the light coming in at an
        // oblique angle, the required bias can get very high.
        // We may need to review this cap of 1m.
        // Especially for the 3rd shadow zone where the shadow texels
        // cover a large amount of space, and low shadow resolutions too.
        // - the 0.5 is the error due to the rounding to discrete texels
        frag_ss << "    Float bias = internal_shadow_additional_bias\n";
        frag_ss << "                 + (0.5 + " << env.shadowFilterSize << ") * slope_bias;\n";
        frag_ss << "    bias = min(bias, 1.0);\n";
        frag_ss << "    internal_light_dist += bias;\n";

        frag_ss << "    out_colour_alpha.x = internal_light_dist / " << env.shadowFactor << ";\n";
    } else {
        if (wireframe) {
            frag_ss << "    Float3 additional;\n";
            frag_ss << "    Float unused;\n";
            frag_ss << "    func_user_colour(additional, unused);\n";
            frag_ss << "    out_colour_alpha.xyzw = Float4(additional, 1);\n";
        } else {
            frag_ss << "    Float shadow_oblique_cutoff = 0;\n";  // TODO(dcunnin)
            frag_ss << "    Float3 d;\n";
            frag_ss << "    Float a;\n";
            frag_ss << "    Float3 n;\n";
            frag_ss << "    Float g;\n";
            frag_ss << "    Float s;\n";
            frag_ss << "    func_user_dangs(d, a, n, g, s);\n";
            frag_ss << "    n = normalise(n);\n";
            if (forward_only) {
                frag_ss << "    Float3 n_vs = mul(global_view, Float4(n, 0)).xyz;\n";
                frag_ss << "    internal_cam_dist /= global_farClipDistance;\n";
                frag_ss << "    pack_deferred(out_gbuffer0, out_gbuffer1, out_gbuffer2,\n";
                frag_ss << "                  shadow_oblique_cutoff, d, n_vs, s,\n";
                frag_ss << "                  internal_cam_dist, g);\n";
            } else {
                frag_ss << "    Float3 v2c = normalise(internal_vertex_to_cam);\n";
                frag_ss << "    Float3 pos_ws = global_cameraPos - internal_vertex_to_cam;\n";
                if (first_person) {
                    // Always use 'near' shadows.
                    frag_ss << "    Float internal_cam_dist = 0;\n";
                    frag_ss << "    pos_ws = global_cameraPos;\n";
                }
                frag_ss << "    Float3 sun = sunlight(pos_ws, v2c, d, n, g, s,\n";
                frag_ss << "                          internal_cam_dist);\n";
                frag_ss << "    Float3 env = envlight(v2c, d, n, g, s);\n";
                frag_ss << "    out_colour_alpha.xyz = sun + env;\n";
                // Fog applied here.
                frag_ss << "    Float fw = fog_weakness(internal_cam_dist);\n";
                frag_ss << "    out_colour_alpha.xyz = lerp(global_fogColour,\n";
                frag_ss << "                                out_colour_alpha.xyz,\n";
                frag_ss << "                                fw * Float3(1, 1, 1));\n",

                frag_ss << "    out_colour_alpha.w = a;\n";
                if (!env.fadeDither) {
                    // Fade out the contribution of sun / env light according to internal fade.
                    frag_ss << "    out_colour_alpha.w *= internal_fade;\n";
                }
                // Pre-multiply alpha.
                frag_ss << "    out_colour_alpha.xyz *= out_colour_alpha.w;\n";

                frag_ss << "    Float3 additional;\n";
                frag_ss << "    Float unused;\n";
                frag_ss << "    func_user_colour(additional, unused);\n";
                if (!env.fadeDither) {
                    // Fade out the contribution of additional light according to internal fade.
                    frag_ss << "    additional *= internal_fade;\n";
                }
                // Fog applied here too.
                frag_ss << "    out_colour_alpha.xyz += additional\n";
                frag_ss << "                            * fog_weakness(internal_cam_dist);\n";
            }
        }
    }
    frag_ss << "}\n";

    frag_out = frag_ss.str();
}


void gfx_gasoline_unparse_decal_glsl(const GfxGslContext &ctx,
                                     const GfxGslTypeSystem *dangs_ts,
                                     const GfxGslAst *dangs_ast,
                                     const GfxGslTypeSystem *additional_ts,
                                     const GfxGslAst *additional_ast,
                                     std::string &vert_out,
                                     std::string &frag_out,
                                     const GfxGslEnvironment &env,
                                     bool glsl33)
{
    GfxGslBackendUnparser dangs_backend("udangs_");
    dangs_backend.unparse(dangs_ast, 1);
    GfxGslBackendUnparser additional_backend("uadd_");
    additional_backend.unparse(additional_ast, 1);

    std::stringstream header;
    header << "// decal shader\n";
    header << "// env: " << env << "\n";
    header << "\n";

    std::set<std::string> vert_in;
    vert_in.insert("position");
    vert_in.insert("coord0");
    vert_in.insert("coord1");
    vert_in.insert("normal");

    GfxGslTypeMap vert_vars, frag_vars;
    for (const auto &pair : dangs_ts->getVars())
        frag_vars["udangs_" + pair.first] = pair.second->type;
    for (const auto &pair : additional_ts->getVars())
        frag_vars["uadd_" + pair.first] = pair.second->type;


    std::vector<GfxGslTrans> trans;
    auto *f3 = ctx.alloc.makeType<GfxGslFloatType>(3);
    auto *f4 = ctx.alloc.makeType<GfxGslFloatType>(4);
    trans.emplace_back(GfxGslTrans{ GfxGslTrans::VERT, { "coord0", "x" }, f4 });
    trans.emplace_back(GfxGslTrans{ GfxGslTrans::VERT, { "coord0", "y" }, f4 });
    trans.emplace_back(GfxGslTrans{ GfxGslTrans::VERT, { "coord1", "x" }, f4 });
    trans.emplace_back(GfxGslTrans{ GfxGslTrans::VERT, { "coord1", "y" }, f4 });
    trans.emplace_back(GfxGslTrans{ GfxGslTrans::VERT, { "normal", "x" }, f3 });
    trans.emplace_back(GfxGslTrans{ GfxGslTrans::VERT, { "normal", "y" }, f3 });
    trans.emplace_back(GfxGslTrans{ GfxGslTrans::VERT, { "normal", "z" }, f3 });

    std::map<std::string, const GfxGslFloatType *> internals;
    internals["normal"] = ctx.alloc.makeType<GfxGslFloatType>(3);

    gfx_gasoline_add_internal_trans(internals, trans);

    for (const auto &tran : trans) {
        const std::string &tv = tran.path[0];
        switch (tran.kind) {
            case GfxGslTrans::VERT:
            vert_in.insert(tv);
            frag_vars["vert_" + tv] = tran.type;
            break;

            case GfxGslTrans::INTERNAL:
            vert_vars["internal_" + tv] = tran.type;
            frag_vars["internal_" + tv] = tran.type;
            break;

            case GfxGslTrans::USER:
            frag_vars["udangs_" + tv] = tran.type;
            frag_vars["uadd_" + tv] = tran.type;
            break;
        }
    }

    // VERTEX

    std::stringstream vert_ss;
    vert_ss << preamble(glsl33);
    vert_ss << header.str();
    vert_ss << generate_vert_header(glsl33, ctx, dangs_ts, trans, vert_in);
    vert_ss << generate_funcs(env);
    vert_ss << generate_funcs_vert(env);
    vert_ss << gfx_gasoline_preamble_transformation(false, env);
    vert_ss << gfx_gasoline_generate_var_decls(vert_vars);
    vert_ss << "void main (void)\n";
    vert_ss << "{\n";
    for (const auto &f : vert_in)
        vert_ss << "    vert_" << f << " = " << vert_global[f] << ";\n";
    vert_ss << "    Float3 pos_ws = transform_to_world(vert_position.xyz);\n";
    vert_ss << "    internal_normal = rotate_to_world(Float3(0, 1, 0));\n";
    vert_ss << "    gl_Position = mul(global_viewProj, Float4(pos_ws, 1));\n";
    vert_ss << "    gl_Position.y *= internal_rt_flip;\n";
    vert_ss << gfx_gasoline_generate_trans_encode(trans, "uvert_");
    vert_ss << "}\n";

    vert_out = vert_ss.str();


    // FRAGMENT

    std::stringstream frag_ss;
    frag_ss << preamble(glsl33);
    frag_ss << header.str();
    frag_ss << generate_frag_header(glsl33, ctx, trans, false);
    frag_ss << generate_funcs(env);
    frag_ss << generate_funcs_frag();
    frag_ss << gfx_gasoline_generate_var_decls(frag_vars);
    if (ctx.lightingTextures)
        frag_ss << gfx_gasoline_preamble_lighting(env);
    frag_ss << gfx_gasoline_preamble_fade(env);
    frag_ss << dangs_backend.getUserDangsFunction();
    frag_ss << additional_backend.getUserColourAlphaFunction();

    // Main function
    frag_ss << "void main (void)\n";
    frag_ss << "{\n";
    frag_ss << "    frag_screen = gl_FragCoord.xy;\n";
    frag_ss << "    if (internal_rt_flip < 0)\n";
    frag_ss << "        frag_screen.y = global_viewportSize.y - frag_screen.y;\n";
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "vert_", GfxGslTrans::VERT);
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "frag_", GfxGslTrans::USER);
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "internal_", GfxGslTrans::INTERNAL);

    frag_ss << "    Float2 uv = frag_screen / global_viewportSize;\n";
    frag_ss << "    Float3 ray = lerp(\n";
    frag_ss << "        lerp(global_rayBottomLeft, global_rayBottomRight, Float3(uv.x)),\n";
    frag_ss << "        lerp(global_rayTopLeft, global_rayTopRight, Float3(uv.x)),\n";
    frag_ss << "        Float3(uv.y));\n";
    frag_ss << "    uv.y = 1 - uv.y;\n";
    frag_ss << "    Float3 bytes = 255 * sample(global_gbuffer0, uv).xyz;\n";
    frag_ss << "    Float depth_int = 256.0*256.0*bytes.x + 256.0*bytes.y + bytes.z;\n";
    frag_ss << "    Float normalised_cam_dist = depth_int / (256.0*256.0*256.0 - 1);\n";
    frag_ss << "    Float3 pos_ws = normalised_cam_dist * ray + global_cameraPos;\n";

    // Apply world backwards
    frag_ss << "    Float3 pos_os = mul(internal_inv_world, Float4(pos_ws, 1)).xyz;\n";
    frag_ss << "    pos_os += Float3(0.5, 0, 0.5);\n";
    frag_ss << "    if (pos_os.x < 0) discard;\n";
    frag_ss << "    if (pos_os.x > 1) discard;\n";
    frag_ss << "    if (pos_os.z < 0) discard;\n";
    frag_ss << "    if (pos_os.z > 1) discard;\n";
    frag_ss << "    if (pos_os.y < -0.5) discard;\n";
    frag_ss << "    if (pos_os.y > 0.5) discard;\n";
    // Overwriting the vertex coord is a bit weird but it's the easiest way to make it available
    // to the dangs shader.
    frag_ss << "    vert_coord0.xy = lerp(vert_coord0.xy, vert_coord1.xy, pos_os.xz);\n";
    frag_ss << "    vert_coord0.zw = Float2(1 - abs(2 * pos_os.y), 0);\n";
    frag_ss << "    vert_normal.xyz = internal_normal;\n";

    frag_ss << "    Float cam_dist = normalised_cam_dist * global_farClipDistance;\n";
    frag_ss << "    Float3 d;\n";
    frag_ss << "    Float a;\n";
    frag_ss << "    Float3 n;\n";
    frag_ss << "    Float g;\n";
    frag_ss << "    Float s;\n";
    frag_ss << "    func_user_dangs(d, a, n, g, s);\n";
    frag_ss << "    n = normalise(n);\n";
    //frag_ss << "    Float3 internal_vertex_to_cam = global_cameraPos - pos_ws;\n";
    frag_ss << "    Float3 v2c = normalise(-ray);\n";
    frag_ss << "    Float3 sun = sunlight(pos_ws, v2c, d, n, g, s, cam_dist);\n";
    frag_ss << "    Float3 env = envlight(v2c, d, n, g, s);\n";
    frag_ss << "    Float3 additional;\n";
    frag_ss << "    Float unused;\n";
    frag_ss << "    func_user_colour(additional, unused);\n";
    frag_ss << "    out_colour_alpha = Float4((sun + env) * a + additional, a);\n";
    // frag_ss << "    out_colour_alpha = Float4(fract(pos_ws), 1);\n";
    // frag_ss << "    out_colour_alpha = Float4(vert_coord0.xy, 0, 0.5);\n";

    if (env.fadeDither) {
        frag_ss << "    fade();\n";
    }

    frag_ss << "}\n";

    frag_out = frag_ss.str();
}
