/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include "../centralised_log.h"

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
};

static std::string preamble (void)
{
    std::stringstream ss;
    ss << "#version 130\n";
    ss << "// This GLSL shader compiled from Gasoline, the Grit shading language.\n";
    ss << "\n";

    ss << "// GSL/GLSL Preamble:\n";
    ss << "#define Float float\n";
    ss << "#define Float2 vec2\n";
    ss << "#define Float3 vec3\n";
    ss << "#define Float4 vec4\n";
    ss << "#define Float2x2 mat2x2\n";
    ss << "#define Float2x3 mat2x3\n";
    ss << "#define Float2x4 mat2x4\n";
    ss << "#define Float3x2 mat3x2\n";
    ss << "#define Float3x3 mat3x3\n";
    ss << "#define Float3x4 mat3x4\n";
    ss << "#define Float4x2 mat4x2\n";
    ss << "#define Float4x3 mat4x3\n";
    ss << "#define Float4x4 mat4x4\n";
    ss << "#define FloatTexture sampler1D\n";
    ss << "#define FloatTexture2 sampler2D\n";
    ss << "#define FloatTexture3 sampler3D\n";
    ss << "#define FloatTextureCube samplerCube\n";
    ss << "\n";

    return ss.str();
}

static std::string generate_funcs (void)
{
    std::stringstream ss;

    ss << "// Standard library\n";
    ss << "Float strength (Float p, Float n) { return pow(max(0.00000001, p), n); }\n";
    ss << "Float atan2 (Float y, Float x) { return atan(y, x); }\n";
    ss << "Float2 mul (Float2x2 m, Float2 v) { return m * v; }\n";
    ss << "Float3 mul (Float2x3 m, Float2 v) { return m * v; }\n";
    ss << "Float4 mul (Float2x4 m, Float2 v) { return m * v; }\n";
    ss << "Float2 mul (Float3x2 m, Float3 v) { return m * v; }\n";
    ss << "Float3 mul (Float3x3 m, Float3 v) { return m * v; }\n";
    ss << "Float4 mul (Float3x4 m, Float3 v) { return m * v; }\n";
    ss << "Float2 mul (Float4x2 m, Float4 v) { return m * v; }\n";
    ss << "Float3 mul (Float4x3 m, Float4 v) { return m * v; }\n";
    ss << "Float4 mul (Float4x4 m, Float4 v) { return m * v; }\n";
    ss << "Float3 normalise (Float3 v) { return normalize(v); }\n";
    ss << "Float4 pma_decode (Float4 v) { return Float4(v.xyz/v.w, v.w); }\n";
    ss << "Float  gamma_decode (Float v)  { return pow(v, 2.2); }\n";
    ss << "Float2 gamma_decode (Float2 v) { return pow(v, Float2(2.2, 2.2)); }\n";
    ss << "Float3 gamma_decode (Float3 v) { return pow(v, Float3(2.2, 2.2, 2.2)); }\n";
    ss << "Float4 gamma_decode (Float4 v) { return pow(v, Float4(2.2, 2.2, 2.2, 2.2)); }\n";
    ss << "Float  gamma_encode (Float v)  { return pow(v, 1/2.2); }\n";
    ss << "Float2 gamma_encode (Float2 v) { return pow(v, Float2(1/2.2, 1/2.2)); }\n";
    ss << "Float3 gamma_encode (Float3 v) { return pow(v, Float3(1/2.2, 1/2.2, 1/2.2)); }\n";
    ss << "Float4 gamma_encode (Float4 v) { return pow(v, Float4(1/2.2, 1/2.2, 1/2.2, 1/2.2)); }\n";
    ss << "Float lerp (Float a, Float b, Float v) { return v*b + (1-v)*a; }\n";
    ss << "Float2 lerp (Float2 a, Float2 b, Float v) { return v*b + (1-v)*a; }\n";
    ss << "Float3 lerp (Float3 a, Float3 b, Float v) { return v*b + (1-v)*a; }\n";
    ss << "Float4 lerp (Float4 a, Float4 b, Float v) { return v*b + (1-v)*a; }\n";

    ss << "Float3 transform_to_world (Float3 v) { return mul(global_world, Float4(v, 1)).xyz; }\n";
    ss << "Float3 rotate_to_world (Float3 v) { return mul(global_world, Float4(v, 0)).xyz; }\n";

    ss << "uniform Float internal_rt_flip;\n";
    ss << "\n";

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

    // Real ones
    ss << "Float4 sample (FloatTexture2 tex, Float2 uv) { return texture(tex, uv); }\n";
    ss << "Float4 sampleGrad (FloatTexture2 tex, Float2 uv, Float2 ddx, Float2 ddy)\n";
    ss << "{ return textureGrad(tex, uv, ddx, ddy); }\n";
    ss << "Float4 sampleLod (FloatTexture2 tex, Float2 uv, Float lod)\n";
    ss << "{ return textureLod(tex, uv, lod); }\n";

    ss << "Float4 sample (FloatTextureCube tex, Float3 uvw) { return texture(tex, uvw); }\n";
    ss << "Float4 sampleGrad (FloatTextureCube tex, Float3 uvw, Float3 ddx, Float3 ddy)\n";
    ss << "{ return textureGrad(tex, uvw, ddx, ddy); }\n";
    ss << "Float4 sampleLod (FloatTextureCube tex, Float3 uvw, Float lod)\n";
    ss << "{ return textureLod(tex, uvw, lod); }\n";

    // 'Fake' ones
    ss << "Float4 sample (Float4 c, Float2 uv) { return c; }\n";
    ss << "Float4 sample (Float4 c, Float3 uvw) { return c; }\n";
    ss << "Float4 sampleGrad (Float4 c, Float2 uv, Float2 ddx, Float2 ddy) { return c; }\n";
    ss << "Float4 sampleGrad (Float4 c, Float3 uvw, Float3 ddx, Float3 ddy) { return c; }\n";
    ss << "Float4 sampleLod (Float4 c, Float2 uv, Float lod) { return c; }\n";
    ss << "Float4 sampleLod (Float4 c, Float3 uvw, Float lod) { return c; }\n";
    ss << "\n";
    return ss.str();
}

static std::string generate_vert_header (const GfxGslContext &ctx,
                                         const GfxGslTypeSystem *ts,
                                         const std::vector<GfxGslTrans> &trans,
                                         const std::set<std::string> &vert_in)
{
    std::stringstream ss;

    ss << "// Vertex header2\n";

    // In (vertex attributes)
    for (const auto &f : vert_in) {
        ss << "in " << ts->getVertType(f) << " " << vert_global[f] << ";\n";
        ss << ts->getVertType(f) << " vert_" << f << ";\n";
    }
    ss << gfx_gasoline_generate_global_fields(ctx, false);
    // Out (trans)
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        ss << "out " << type.str() << " trans" << i/4 << ";\n";
    }

    ss << "\n";

    return ss.str();
}

static std::string generate_frag_header(const GfxGslContext &ctx,
                                        const std::vector<GfxGslTrans> &trans)
{
    std::stringstream ss;

    ss << "// Fragment header\n";

    // In (trans)
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        ss << "in " << type.str() << " trans" << i/4 << ";\n";
    }

    ss << gfx_gasoline_generate_global_fields(ctx, false);

    // Out
    ss << "out Float4 out_colour_alpha;\n";

    ss << "\n";

    return ss.str();
}

void gfx_gasoline_unparse_glsl (GfxGslContext &ctx,
                                const GfxGslTypeSystem *vert_ts,
                                const GfxGslAst *vert_ast,
                                std::string &vert_output,
                                const GfxGslTypeSystem *frag_ts,
                                const GfxGslAst *frag_ast,
                                std::string &frag_output,
                                bool flat_z)
{
    GfxGslBackendUnparser vert_backend("user_");
    vert_backend.unparse(vert_ast, 1);
    GfxGslBackendUnparser frag_backend("user_");
    frag_backend.unparse(frag_ast, 1);

    GfxGslTypeMap vert_vars, frag_vars;
    auto trans = frag_ts->getTransVector();
    std::set<std::string> vert_in = vert_ts->getVertFieldsRead();
    vert_in.insert("position");
    for (const auto &tran : trans) {
        const std::string &tv = tran.path[0];
        switch (tran.kind) {
            case GfxGslTrans::VERT:
            vert_in.insert(tv);
            frag_vars["vert_" + tv] = frag_ts->getVertType(tv);
            break;
            case GfxGslTrans::INTERNAL:
            frag_vars["internal_" + tv] = frag_ts->getVertType(tv);
            break;
            default:;
        }
    }
    for (const auto &pair : vert_ts->getVars())
        vert_vars["user_" + pair.first] = pair.second;
    for (const auto &pair : frag_ts->getVars())
        frag_vars["user_" + pair.first] = pair.second;

    // VERTEX

    std::stringstream vert_ss;
    vert_ss << preamble();
    vert_ss << generate_vert_header(ctx, vert_ts, trans, vert_in);
    vert_ss << generate_funcs();
    vert_ss << gfx_gasoline_generate_var_decls(vert_vars);
    vert_ss << vert_backend.getUserVertexFunction();
    vert_ss << "void main (void)\n";
    vert_ss << "{\n";
    for (const auto &f : vert_in)
        vert_ss << "    vert_" << f << " = " << vert_global[f] << ";\n";
    vert_ss << "    Float3 world_pos;\n";
    vert_ss << "    func_user_vertex(world_pos);\n";
    vert_ss << "    Float4 clip_pos = mul(global_viewProj, Float4(world_pos, 1));\n";
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
    frag_ss << preamble();
    frag_ss << generate_frag_header(ctx, trans);
    frag_ss << "Float2 frag_screen;\n";
    frag_ss << generate_funcs();
    frag_ss << generate_funcs_frag();
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
    frag_ss << "    out_colour_alpha = Float4(out_colour, out_alpha);\n";
    frag_ss << "}\n";

    frag_output = frag_ss.str();
}

void gfx_gasoline_unparse_first_person_glsl(GfxGslContext &ctx,
                                            const GfxGslTypeSystem *vert_ts,
                                            const GfxGslAst *vert_ast,
                                            const GfxGslTypeSystem *dangs_ts,
                                            const GfxGslAst *dangs_ast,
                                            const GfxGslTypeSystem *additional_ts,
                                            const GfxGslAst *additional_ast,
                                            std::string &vert_out,
                                            std::string &frag_out)
{
    GfxGslBackendUnparser vert_backend("uvert_");
    vert_backend.unparse(vert_ast, 1);
    GfxGslBackendUnparser dangs_backend( "udangs_");
    dangs_backend.unparse(dangs_ast, 1);
    GfxGslBackendUnparser additional_backend("uadd_");
    additional_backend.unparse(additional_ast, 1);

    std::set<GfxGslTrans> trans_set = dangs_ts->getTrans();
    trans_set.insert(additional_ts->getTrans().begin(), additional_ts->getTrans().end());

    std::vector<GfxGslTrans> trans(trans_set.begin(), trans_set.end());
    trans.emplace_back(GfxGslTrans{GfxGslTrans::INTERNAL, {"vertex_to_cam", "x"}});
    trans.emplace_back(GfxGslTrans{GfxGslTrans::INTERNAL, {"vertex_to_cam", "y"}});
    trans.emplace_back(GfxGslTrans{GfxGslTrans::INTERNAL, {"vertex_to_cam", "z"}});

    GfxGslTypeMap vert_vars, frag_vars;
    std::set<std::string> vert_in = vert_ts->getVertFieldsRead();
    vert_in.insert("position");
    GfxGslTypeMap trans_vert;
    for (const auto &tran : trans) {
        const std::string &tv = tran.path[0];
        if (tran.kind == GfxGslTrans::VERT) {
            frag_vars["vert_" + tv] = vert_ts->getVertType(tv);  // Any type system object will do
            vert_in.insert(tv);
        }
    }
    for (const auto &pair : vert_ts->getVars())
        vert_vars["uvert_" + pair.first] = pair.second;
    for (const auto &pair : dangs_ts->getVars())
        frag_vars["udangs_" + pair.first] = pair.second;
    for (const auto &pair : additional_ts->getVars())
        frag_vars["uadd_" + pair.first] = pair.second;
    vert_vars["internal_vertex_to_cam"] = ctx.alloc.makeType<GfxGslFloatType>(3);
    frag_vars["internal_vertex_to_cam"] = ctx.alloc.makeType<GfxGslFloatType>(3);


    // VERTEX

    std::stringstream vert_ss;
    vert_ss << preamble();
    vert_ss << generate_vert_header(ctx, vert_ts, trans, vert_in);
    vert_ss << generate_funcs();
    vert_ss << gfx_gasoline_generate_var_decls(vert_vars);
    vert_ss << vert_backend.getUserVertexFunction();
    vert_ss << "void main (void)\n";
    vert_ss << "{\n";
    for (const auto &f : vert_in)
        vert_ss << "    vert_" << f << " = " << vert_global[f] << ";\n";
    vert_ss << "    Float3 world_pos;\n";
    vert_ss << "    func_user_vertex(world_pos);\n";
    vert_ss << "    gl_Position = mul(global_viewProj, Float4(world_pos, 1));\n";
    vert_ss << "    internal_vertex_to_cam = global_cameraPos - world_pos;\n";
    vert_ss << gfx_gasoline_generate_trans_encode(trans, "uvert_");
    vert_ss << "}\n";

    vert_out = vert_ss.str();


    // FRAGMENT

    std::stringstream frag_ss;
    frag_ss << preamble();
    frag_ss << generate_frag_header(ctx, trans);
    frag_ss << generate_funcs();
    frag_ss << generate_funcs_frag();
    frag_ss << gfx_gasoline_generate_var_decls(frag_vars);
    frag_ss << gfx_gasoline_preamble_lighting();
    frag_ss << "Float2 frag_screen;\n";
    frag_ss << dangs_backend.getUserDangsFunction();
    frag_ss << additional_backend.getUserColourAlphaFunction();

    // Main function
    frag_ss << "void main (void)\n";
    frag_ss << "{\n";
    frag_ss << "    frag_screen = gl_FragCoord.xy;\n";
    frag_ss << "    if (internal_rt_flip < 0)\n";
    frag_ss << "        frag_screen.y = global_viewportSize.y - frag_screen.y;\n";
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "vert_", GfxGslTrans::VERT);
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "udangs_", GfxGslTrans::USER);
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "uadd_", GfxGslTrans::USER);
    frag_ss << gfx_gasoline_generate_trans_decode(trans, "internal_", GfxGslTrans::INTERNAL);
    frag_ss << "    Float3 d;\n";
    frag_ss << "    Float a;\n";
    frag_ss << "    Float3 n;\n";
    frag_ss << "    Float g;\n";
    frag_ss << "    Float s;\n";
    frag_ss << "    func_user_dangs(d, a, n, g, s);\n";
    frag_ss << "    n = normalise(n);\n";
    frag_ss << "    Float3 s2c = normalize(internal_vertex_to_cam);\n";
    frag_ss << "    Float3 sun = punctual_lighting(-global_sunlightDirection, s2c,\n";
    frag_ss << "        d, n, g, s, global_sunlightDiffuse, global_sunlightSpecular);\n";
    frag_ss << "    Float3 env0 = env_lighting(s2c,\n";
    frag_ss << "        d, n, g, s, global_envCube0, global_envCubeMipmaps0);\n";
    frag_ss << "    Float3 env = env0;\n";
    frag_ss << "    Float3 additional;\n";
    frag_ss << "    Float unused;\n";
    frag_ss << "    func_user_colour(additional, unused);\n";
    frag_ss << "    out_colour_alpha = Float4(sun + env + additional, 1);\n";
    frag_ss << "}\n";

    frag_out = frag_ss.str();
}

