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

#include "gfx_gasoline_backend.h"
#include "gfx_gasoline_backend_cg.h"
#include "gfx_gasoline_type_system.h"

static std::map<std::string, std::string> vert_semantic = {
    {"position", "POSITION"},
    {"coord0", "TEXCOORD0"},
    {"coord1", "TEXCOORD1"},
    {"coord2", "TEXCOORD2"},
    {"coord3", "TEXCOORD3"},
    {"coord4", "TEXCOORD4"},
    {"coord5", "TEXCOORD5"},
    {"coord6", "TEXCOORD6"},
    {"coord7", "TEXCOORD7"},
    {"normal", "NORMAL"},
    {"tangent", "TANGENT"},
    {"colour", "COLOR"},
    {"boneWeights", "BLENDWEIGHT"},
    {"boneAssignments", "BLENDINDICES"},
};

static std::map<std::string, std::string> frag_semantic = {
    {"position", "POSITION"},
    {"colour", "COLOR"},
    {"screen", "WPOS"},
    {"depth", "DEPTH"},
};

static std::string cg_preamble (void)
{
    std::stringstream ss;

    ss << "// This CG shader compiled from Gasoline, the Grit shading language.\n";
    ss << "\n";

    ss << "// GSL/CG Preamble:\n";
    ss << "typedef int Int;\n";
    ss << "typedef int2 Int2;\n";
    ss << "typedef int3 Int3;\n";
    ss << "typedef int4 Int4;\n";
    ss << "typedef float Float;\n";
    ss << "typedef float2 Float2;\n";
    ss << "typedef float3 Float3;\n";
    ss << "typedef float4 Float4;\n";
    ss << "typedef float2x2 Float2x2;\n";
    ss << "typedef float2x3 Float2x3;\n";
    ss << "typedef float2x4 Float2x4;\n";
    ss << "typedef float3x2 Float3x2;\n";
    ss << "typedef float3x3 Float3x3;\n";
    ss << "typedef float3x4 Float3x4;\n";
    ss << "typedef float4x2 Float4x2;\n";
    ss << "typedef float4x3 Float4x3;\n";
    ss << "typedef float4x4 Float4x4;\n";
    ss << "typedef sampler1D FloatTexture;\n";
    ss << "typedef sampler2D FloatTexture2;\n";
    ss << "typedef sampler3D FloatTexture3;\n";
    ss << "typedef samplerCUBE FloatTextureCube;\n";

    ss << "\n";

    return ss.str();
}


static std::string generate_funcs (const GfxGslEnvironment &env)
{
    std::stringstream ss;

    ss << "// Standard library\n";
    ss << "Float strength (Float p, Float n) { return pow(max(0.00000001, p), n); }\n";
    ss << "Int max (Int a, Int b) { return a > b ? a : b; }\n";
    ss << "Int2 max (Int2 a, Int2 b) {\n";
    ss << "    return Int2(\n";
    ss << "        max(a.x, b.x),\n";
    ss << "        max(a.y, b.y)\n";
    ss << "    );\n";
    ss << "}\n";
    ss << "Int3 max (Int3 a, Int3 b) {\n";
    ss << "    return Int3(\n";
    ss << "        max(a.x, b.x),\n";
    ss << "        max(a.y, b.y),\n";
    ss << "        max(a.z, b.z)\n";
    ss << "    );\n";
    ss << "}\n";
    ss << "Int4 max (Int4 a, Int4 b) {\n";
    ss << "    return Int4(\n";
    ss << "        max(a.x, b.x),\n";
    ss << "        max(a.y, b.y),\n";
    ss << "        max(a.z, b.z),\n";
    ss << "        max(a.w, b.w)\n";
    ss << "    );\n";
    ss << "}\n";
    ss << "Float max (Float a, Float b) { return a > b ? a : b; }\n";
    ss << "Float2 max (Float2 a, Float2 b) {\n";
    ss << "    return Float2(\n";
    ss << "        max(a.x, b.x),\n";
    ss << "        max(a.y, b.y)\n";
    ss << "    );\n";
    ss << "}\n";
    ss << "Float3 max (Float3 a, Float3 b) {\n";
    ss << "    return Float3(\n";
    ss << "        max(a.x, b.x),\n";
    ss << "        max(a.y, b.y),\n";
    ss << "        max(a.z, b.z)\n";
    ss << "    );\n";
    ss << "}\n";
    ss << "Float4 max (Float4 a, Float4 b) {\n";
    ss << "    return Float4(\n";
    ss << "        max(a.x, b.x),\n";
    ss << "        max(a.y, b.y),\n";
    ss << "        max(a.z, b.z),\n";
    ss << "        max(a.w, b.w)\n";
    ss << "    );\n";
    ss << "}\n";
    ss << "\n";
    ss << "Int abs (Int v) { return max(v, -v); }\n";
    ss << "Int2 abs (Int2 v) { return max(v, -v); }\n";
    ss << "Int3 abs (Int3 v) { return max(v, -v); }\n";
    ss << "Int4 abs (Int4 v) { return max(v, -v); }\n";
    ss << "Float abs (Float v) { return max(v, -v); }\n";
    ss << "Float2 abs (Float2 v) { return max(v, -v); }\n";
    ss << "Float3 abs (Float3 v) { return max(v, -v); }\n";
    ss << "Float4 abs (Float4 v) { return max(v, -v); }\n";
    // Rename frac to fract
    ss << "Float fract (Float v) { return frac(v); }\n";
    ss << "Float2 fract (Float2 v) { return frac(v); }\n";
    ss << "Float3 fract (Float3 v) { return frac(v); }\n";
    ss << "Float4 fract (Float4 v) { return frac(v); }\n";
    ss << "\n";

    ss << "Float4x4 get_inst_matrix()\n";
    ss << "{\n";
    ss << "    return Float4x4(\n";
    ss << "        vert_coord1[0], vert_coord1[1], vert_coord1[2], vert_coord4[0],\n";
    ss << "        vert_coord2[0], vert_coord2[1], vert_coord2[2], vert_coord4[1],\n";
    ss << "        vert_coord3[0], vert_coord3[1], vert_coord3[2], vert_coord4[2],\n";
    ss << "                   0.0,            0.0,            0.0,            1.0);\n";
    ss << "}\n";

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
        ss << "        vert_coord1[0], vert_coord1[1], vert_coord1[2], vert_coord4[0],\n";
        ss << "        vert_coord2[0], vert_coord2[1], vert_coord2[2], vert_coord4[1],\n";
        ss << "        vert_coord3[0], vert_coord3[1], vert_coord3[2], vert_coord4[2],\n";
        ss << "                   0.0,            0.0,            0.0,            1.0);\n";
        ss << "}\n";
        ss << "\n";
    }

    return ss.str();
}

static std::string generate_funcs_frag (void)
{
    std::stringstream ss;
    ss << "// Standard library (fragment shader specific calls)\n";

    // Real ones
    // 2D
    ss << "Float4 sample (FloatTexture2 tex, Float2 uv) { return tex2D(tex, uv); }\n";
    ss << "Float4 sampleGrad (FloatTexture2 tex, Float2 uv, Float2 ddx, Float2 ddy)\n";
    ss << "{ return tex2DGrad(tex, uv, ddx, ddy); }\n";
    ss << "Float4 sampleLod (FloatTexture2 tex, Float2 uv, Float lod)\n";
    ss << "{ return tex2Dlod(tex, Float4(uv, 0, lod)); }\n";

    // 3D (volumetric)
    ss << "Float4 sample (FloatTexture3 tex, Float3 uvw) { return tex3D(tex, uvw); }\n";
    ss << "Float4 sampleGrad (FloatTexture3 tex, Float3 uvw, Float3 ddx, Float3 ddy)\n";
    ss << "{ return tex3Dgrad(tex, uvw, ddx, ddy); }\n";
    ss << "Float4 sampleLod (FloatTexture3 tex, Float3 uvw, Float lod)\n";
    ss << "{ return tex3Dlod(tex, Float4(uvw, lod)); }\n";

    // Cube
    ss << "Float4 sample (FloatTextureCube tex, Float3 uvw) { return texCUBE(tex, uvw); }\n";
    ss << "Float4 sampleGrad (FloatTextureCube tex, Float3 uvw, Float3 ddx, Float3 ddy)\n";
    ss << "{ return texCUBEgrad(tex, uvw, ddx, ddy); }\n";
    ss << "Float4 sampleLod (FloatTextureCube tex, Float3 uvw, Float lod)\n";
    ss << "{ return texCUBElod(tex, Float4(uvw, lod)); }\n";

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
                                         const std::set<std::string> &vert_in)
{
    std::stringstream ss;

    ss << "// Vertex header2\n";

    // In (vertex attributes)
    for (const auto &f : vert_in) {
        ss << "in " << ts->getVertType(f) << " vert_" << f << " : " << vert_semantic[f] << ";\n";
    }
    ss << gfx_gasoline_generate_global_fields(ctx, true);

    ss << "\n";

    return ss.str();
}


static std::string generate_frag_header(const GfxGslContext &ctx,
                                        const std::vector<GfxGslTrans> &trans)
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
        ss << "in " << type.str() << " trans" << i/4 << " : TEXCOORD" << i/4 << ";\n";
    }

    ss << gfx_gasoline_generate_global_fields(ctx, true);

    ss << "\n";

    return ss.str();
}




void gfx_gasoline_unparse_cg(const GfxGslContext &ctx,
                             const GfxGslTypeSystem *vert_ts,
                             const GfxGslAst *vert_ast,
                             std::string &vert_output,
                             const GfxGslTypeSystem *frag_ts,
                             const GfxGslAst *frag_ast,
                             std::string &frag_output,
                             const GfxGslEnvironment &env,
                             bool flat_z,
                             bool das)
{
    GfxGslBackendUnparser vert_backend("user_");
    vert_backend.unparse(vert_ast, 1);
    GfxGslBackendUnparser frag_backend("user_");
    frag_backend.unparse(frag_ast, 1);

    GfxGslTypeMap vert_vars, frag_vars;
    auto trans = frag_ts->getTransVector();
    std::set<std::string> vert_in = vert_ts->getVertFieldsRead();
    vert_in.insert("position");
    if (env.boneWeights > 0) {
        vert_in.insert("boneWeights");
        vert_in.insert("boneAssignments");
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
            frag_vars["user_" + tv] = tran.type;
            break;
        }
    }
    for (const auto &pair : vert_ts->getVars())
        vert_vars["user_" + pair.first] = pair.second->type;
    for (const auto &pair : frag_ts->getVars())
        frag_vars["user_" + pair.first] = pair.second->type;


    std::stringstream vert_ss;
    vert_ss << cg_preamble();
    vert_ss << generate_vert_header(ctx, vert_ts, vert_in);
    vert_ss << generate_funcs(env);
    vert_ss << generate_funcs_vert(env);
    vert_ss << gfx_gasoline_preamble_transformation(false, env);
    vert_ss << gfx_gasoline_generate_var_decls(vert_vars);
    vert_ss << vert_backend.getUserVertexFunction();
    vert_ss << "void main (\n";
    // Out (trans)
    for (unsigned i = 0; i < trans.size(); i += 4) {
        unsigned sz = trans.size() - i > 4 ? 4 : trans.size() - i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        vert_ss << "    out " << type.str() << " trans" << i/4 << " : TEXCOORD" << i/4 << ",\n";
    }
    vert_ss << "    out Float4 out_position : POSITION\n";
    vert_ss << ") {\n";
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
    if (flat_z) {
        // Hack to maximum depth, but avoid hitting the actual backplane.
        // (Without this dcunnin saw some black lightning-like artifacts on Nvidia.)
        vert_ss << "    clip_pos.z = clip_pos.w * (1 - 1.0/65536);\n";
    }
    if (ctx.d3d9) {
        // Convert z/w from -1 to 1 range to 0 to 1 range by modifying z only.
        vert_ss << "    clip_pos.z = (clip_pos.z + clip_pos.w) / 2.0;\n";
    } else {
        vert_ss << "    clip_pos.y *= internal_rt_flip;\n";
    }
    vert_ss << "    out_position = clip_pos;\n";
    vert_ss << gfx_gasoline_generate_trans_encode(trans, "user_");
    vert_ss << "}\n";

    std::stringstream frag_ss;
    frag_ss << cg_preamble();
    frag_ss << "// Fragment shader\n";
    frag_ss << "\n";
    frag_ss << generate_frag_header(ctx, trans);
    frag_ss << generate_funcs(env);
    frag_ss << generate_funcs_frag();
    if (ctx.lightingTextures)
        frag_ss << gfx_gasoline_preamble_lighting(env);
    frag_ss << gfx_gasoline_preamble_fade(env);
    frag_ss << gfx_gasoline_generate_var_decls(frag_vars);
    frag_ss << frag_backend.getUserColourAlphaFunction();


    frag_ss << "void main (in Float2 wpos : WPOS,\n";
    frag_ss << "           out Float4 out_colour_alpha : COLOR,\n";
    frag_ss << "           out Float out_depth : DEPTH\n";
    frag_ss << ") {\n";

    frag_ss << "    frag_screen = wpos.xy;\n";
    if (ctx.d3d9) {
        // CG targeting ps_3_0 exposes VPOS as WPOS, which is not what we want.
        frag_ss << "    frag_screen.y = global_viewportSize.y - frag_screen.y;\n";
        frag_ss << "    frag_screen += Float2(0.5, -0.5);\n";
    }
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
    }
    frag_ss << "    out_colour_alpha = Float4(out_colour * out_alpha, out_alpha);\n";
    if (das) {
        // Whether we are using d3d9 or gl rendersystems,
        // ogre gives us the view_proj in a 'standard' form, which is
        // right-handed with a depth range of [-1,+1].
        // Since we are outputing depth in the fragment shader, the range either way is [0,1]
        //frag_ss << "    Float4 projected = mul(global_viewProj, Float4(user_pos_ws, 1));\n";
        //frag_ss << "    out_depth = 0.5 + (projected.z / projected.w) / 2.0;\n";
    }
    frag_ss << "}\n";

    vert_output = vert_ss.str();
    frag_output = frag_ss.str();
}

void gfx_gasoline_unparse_body_cg(const GfxGslContext &ctx,
                                  const GfxGslTypeSystem *vert_ts,
                                  const GfxGslAst *vert_ast,
                                  const GfxGslTypeSystem *dangs_ts,
                                  const GfxGslAst *dangs_ast,
                                  const GfxGslTypeSystem *additional_ts,
                                  const GfxGslAst *additional_ast,
                                  std::string &vert_out,
                                  std::string &frag_out,
                                  const GfxGslEnvironment &env,
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
    if (cast) {
        // vert_in.insert("normal");
    }
    if (env.instanced) {
        vert_in.insert("coord1");
        vert_in.insert("coord2");
        vert_in.insert("coord3");
        vert_in.insert("coord4");
        vert_in.insert("coord5");
    }
    if (env.boneWeights > 0) {
        vert_in.insert("boneWeights");
        vert_in.insert("boneAssignments");
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
            frag_vars["udangs_" + tv] = tran.type;
            frag_vars["uadd_" + tv] = tran.type;
            break;
        }
    }


    // VERTEX

    std::stringstream vert_ss;
    vert_ss << cg_preamble();
    vert_ss << header.str();
    vert_ss << generate_vert_header(ctx, vert_ts, vert_in);
    vert_ss << generate_funcs(env);
    vert_ss << gfx_gasoline_preamble_transformation(first_person, env);
    vert_ss << gfx_gasoline_generate_var_decls(vert_vars);
    vert_ss << vert_backend.getUserVertexFunction();
    vert_ss << "void main (\n";
    // Out (trans)
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        vert_ss << "    out " << type.str() << " trans" << i/4 << " : TEXCOORD" << i/4 << ",\n";
    }
    vert_ss << "    out Float4 out_position : POSITION)\n";
    vert_ss << "{\n";
    vert_ss << "    Float3 pos_ws;\n";
    vert_ss << "    func_user_vertex(pos_ws);\n";
    if (cast) {
        // vert_ss << "    pos_ws -= vert_normal * 0.08;\n";
        vert_ss << "    Float4 clip_pos = mul(internal_shadow_view_proj, Float4(pos_ws, 1));\n";
        vert_ss << "    internal_light_dist = dot(global_sunlightDirection, pos_ws);\n";
    } else {
        vert_ss << "    Float3 pos_vs = mul(global_view, Float4(pos_ws, 1)).xyz;\n";
        vert_ss << "    Float4 clip_pos = mul(global_proj, Float4(pos_vs, 1));\n";
        if (ctx.d3d9) {
            vert_ss << "    clip_pos.z = (clip_pos.z + clip_pos.w) / 2.0;\n";
        } else {
            vert_ss << "    clip_pos.y *= internal_rt_flip;\n";
        }
        if (!wireframe)
            vert_ss << "    internal_vertex_to_cam = global_cameraPos - pos_ws;\n";
        if (!first_person)
            vert_ss << "    internal_cam_dist = -pos_vs.z;\n";
    }
    if (env.instanced) {
        // The 0.5/255 is necessary because apparently we lose some precision through rasterisation.
        vert_ss << "    internal_fade = vert_coord5.x + 0.5/255;\n";
    }
    vert_ss << "    out_position = clip_pos;\n";
    vert_ss << gfx_gasoline_generate_trans_encode(trans, "uvert_");
    vert_ss << "}\n";

    vert_out = vert_ss.str();



    // FRAGMENT

    std::stringstream frag_ss;
    frag_ss << cg_preamble();
    frag_ss << header.str();
    frag_ss << generate_frag_header(ctx, trans);
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
    frag_ss << "void main (\n";
    frag_ss << "    in Float2 wpos : WPOS,\n";
    if (forward_only) {
        frag_ss << "    out Float4 out_gbuffer0 : COLOR0,\n";
        frag_ss << "    out Float4 out_gbuffer1 : COLOR1,\n";
        frag_ss << "    out Float4 out_gbuffer2 : COLOR2\n";
    } else {
        frag_ss << "    out Float4 out_colour_alpha : COLOR\n";
    }
    frag_ss << ") {\n";

    frag_ss << "    frag_screen = wpos.xy + Float2(0.5, 0.5);\n";
    // Due to a bug in CG targeting ps_3_0, wpos is origin top right, so correct that:
    if (ctx.d3d9) {
        frag_ss << "    frag_screen.y = global_viewportSize.y - frag_screen.y;\n";
    }
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
    frag_ss << "    if (internal_rt_flip < 0)\n";
    frag_ss << "        frag_screen.y = global_viewportSize.y - frag_screen.y;\n";


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
                if (first_person) {
                    // Always use 'near' shadows.
                    frag_ss << "    Float internal_cam_dist = 0;\n";
                }
                frag_ss << "    Float3 sun = sunlight(global_cameraPos, v2c, d, n, g, s,\n";
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


// dangs & additional shaders will want to know:
// 1) world position
// 2) position within object decal 
// 3) world normal
// 4) normal within object decal
//
void gfx_gasoline_unparse_decal_cg(const GfxGslContext &ctx,
                                   const GfxGslTypeSystem *dangs_ts,
                                   const GfxGslAst *dangs_ast,
                                   const GfxGslTypeSystem *additional_ts,
                                   const GfxGslAst *additional_ast,
                                   std::string &vert_out,
                                   std::string &frag_out,
                                   const GfxGslEnvironment &env)
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
    vert_ss << cg_preamble();
    vert_ss << header.str();
    vert_ss << generate_vert_header(ctx, dangs_ts, vert_in);
    vert_ss << generate_funcs(env);
    vert_ss << generate_funcs_vert(env);
    vert_ss << gfx_gasoline_preamble_transformation(false, env);
    vert_ss << gfx_gasoline_generate_var_decls(vert_vars);
    vert_ss << "void main (\n";
    // Out (trans)
    for (unsigned i = 0; i<trans.size(); i += 4) {
        unsigned sz = trans.size() - i > 4 ? 4 : trans.size() - i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        vert_ss << "    out " << type.str() << " trans" << i / 4 << " : TEXCOORD" << i / 4 << ",\n";
    }
    vert_ss << "    out Float4 out_position : POSITION)\n";
    vert_ss << "{\n";
    vert_ss << "    Float3 pos_ws = transform_to_world(vert_position.xyz);\n";
    vert_ss << "    internal_normal = rotate_to_world(Float3(0, 1, 0));\n";
    vert_ss << "    Float3 pos_vs = mul(global_view, Float4(pos_ws, 1)).xyz;\n";
    vert_ss << "    Float4 clip_pos = mul(global_proj, Float4(pos_vs, 1));\n";
    if (ctx.d3d9) {
        vert_ss << "    clip_pos.z = (clip_pos.z + clip_pos.w) / 2.0;\n";
    }
    else {
        vert_ss << "    clip_pos.y *= internal_rt_flip;\n";
    }
    vert_ss << "    out_position = clip_pos;\n";
    vert_ss << gfx_gasoline_generate_trans_encode(trans, "uvert_");
    vert_ss << "}\n";

    vert_out = vert_ss.str();


    // FRAGMENT

    std::stringstream frag_ss;
    frag_ss << cg_preamble();
    frag_ss << header.str();
    frag_ss << generate_frag_header(ctx, trans);
    frag_ss << generate_funcs(env);
    frag_ss << generate_funcs_frag();
    frag_ss << gfx_gasoline_generate_var_decls(frag_vars);
    if (ctx.lightingTextures)
        frag_ss << gfx_gasoline_preamble_lighting(env);
    frag_ss << gfx_gasoline_preamble_fade(env);
    frag_ss << dangs_backend.getUserDangsFunction();
    frag_ss << additional_backend.getUserColourAlphaFunction();

    // Main function
    frag_ss << "void main (\n";
    frag_ss << "    in Float2 wpos : WPOS,\n";
    frag_ss << "    out Float4 out_colour_alpha : COLOR\n";
    frag_ss << ") {\n";
    // frag_screen should be origin bottom left, centered in the middle of pixels.
    frag_ss << "    frag_screen = wpos.xy;\n";
    if (ctx.d3d9) {
        frag_ss << "    frag_screen.y = global_viewportSize.y - frag_screen.y;\n";
        // It turned out to be very important to do the 0.5 correction after inverting the screen.
        // Otherwise, there were places where the decal did not match up with the geometry underneath.
        frag_ss << "    frag_screen += Float2(0.5, -0.5);\n";
    }
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
    frag_ss << "    Float4 texel0 = sample(global_gbuffer0, uv);\n";
    frag_ss << "    Float4 texel1 = Float4(0, 0, 0, 0);\n";
    frag_ss << "    Float4 texel2 = Float4(0, 0, 0, 0);\n";
    frag_ss << "    Float normalised_cam_dist = unpack_deferred_cam_dist(texel0, texel1, texel2);\n";
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

