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

#include "gfx_gasoline_backend.h"


void GfxGslBackendUnparser::unparse (const GfxGslAst *ast_, int indent)
{
    std::string space(4 * indent, ' ');
    if (auto ast = dynamic_cast<const GfxGslBlock*>(ast_)) {
        ss << space << "{\n";
        for (auto stmt : ast->stmts) {
            unparse(stmt, indent+1);
        }
        ss << space << "}\n";
    } else if (auto ast = dynamic_cast<const GfxGslShader*>(ast_)) {
        for (auto stmt : ast->stmts) {
            unparse(stmt, indent);
        }
    } else if (auto ast = dynamic_cast<const GfxGslDecl*>(ast_)) {
        ss << space << varPref << ast->id << " = ";
        unparse(ast->init, indent);
        ss << ";\n";
    } else if (auto ast = dynamic_cast<const GfxGslIf*>(ast_)) {
        ss << space << "if (";
        unparse(ast->cond, indent);
        ss << ") {\n";
        unparse(ast->yes, indent+1);
        if (ast->no) {
            ss << space << "} else {\n";
            unparse(ast->no, indent+1);
        }
        ss << space << "}\n";
    } else if (auto ast = dynamic_cast<const GfxGslAssign*>(ast_)) {
        ss << space;
        unparse(ast->target, indent);
        ss << " = ";
        unparse(ast->expr, indent);
        ss << ";\n";
    } else if (dynamic_cast<const GfxGslDiscard*>(ast_)) {
        ss << space << "discard;\n";
    } else if (dynamic_cast<const GfxGslReturn*>(ast_)) {
        ss << space << "return;\n";

    } else if (auto ast = dynamic_cast<const GfxGslCall*>(ast_)) {
        ss << ast->func;
        if (ast->args.size() == 0) {
            ss << "()";
        } else {
            const char *sep = "(";
            for (auto arg : ast->args) {
                ss << sep;
                unparse(arg, indent);
                sep = ", ";
            }
            ss << ")";
        }
    } else if (auto ast = dynamic_cast<const GfxGslField*>(ast_)) {
        if (dynamic_cast<GfxGslGlobalType*>(ast->target->type)) {
            ss << "global_" << ast->id;
        } else if (dynamic_cast<GfxGslMatType*>(ast->target->type)) {
            ss << "mat_" << ast->id;
        } else if (dynamic_cast<GfxGslVertType*>(ast->target->type)) {
            ss << "vert_" << ast->id;
        } else if (dynamic_cast<GfxGslFragType*>(ast->target->type)) {
            ss << "frag_" + ast->id;
        } else if (dynamic_cast<GfxGslOutType*>(ast->target->type)) {
            ss << "out_" + ast->id;
        } else if (dynamic_cast<GfxGslBodyType*>(ast->target->type)) {
            ss << "body_" + ast->id;
        } else if (auto *type = dynamic_cast<GfxGslCoordType*>(ast->target->type)) {
            if (type->dim == 1) {
                ASSERT(ast->id == "x" || ast->id == "r");
                ss << "(";
                unparse(ast->target, indent);
                ss << ")";
            } else {
                ss << "(";
                unparse(ast->target, indent);
                ss << ").";
                ss << ast->id;
            }
        } else {
            ss << "(";
            unparse(ast->target, indent);
            ss << ").";
            ss << ast->id;
        }
    } else if (auto ast = dynamic_cast<const GfxGslLiteralInt*>(ast_)) {
        ss << ast->val;
    } else if (auto ast = dynamic_cast<const GfxGslLiteralFloat*>(ast_)) {
        ss << ast->val;
    } else if (auto ast = dynamic_cast<const GfxGslLiteralBoolean*>(ast_)) {
        ss << (ast->val ? "true" : "false");
    } else if (auto ast = dynamic_cast<const GfxGslVar*>(ast_)) {
        ss << varPref << ast->id;
    } else if (auto ast = dynamic_cast<const GfxGslBinary*>(ast_)) {
        if (ast->op == GFX_GSL_OP_MOD) {
            ss << "mod(";
            unparse(ast->a, indent);
            ss << ", ";
            unparse(ast->b, indent);
            ss << ")";
        } else {
            ss << "(";
            unparse(ast->a, indent);
            ss << " " << to_string(ast->op) << " ";
            unparse(ast->b, indent);
            ss << ")";
        }
    } else if (dynamic_cast<const GfxGslGlobal*>(ast_)) {
        ss << "global";
    } else if (dynamic_cast<const GfxGslMat*>(ast_)) {
        ss << "mat";
    } else if (dynamic_cast<const GfxGslVert*>(ast_)) {
        ss << "vert";
    } else if (dynamic_cast<const GfxGslFrag*>(ast_)) {
        ss << "frag";
    } else if (dynamic_cast<const GfxGslOut*>(ast_)) {
        ss << "out";

    } else {
        EXCEPTEX << "INTERNAL ERROR: Unknown GfxGslAst." << ENDL;
    }
}

std::string gfx_gasoline_generate_global_fields (const GfxGslContext &ctx, bool reg)
{
    std::stringstream ss;

    unsigned counter = 0;

    for (const auto &pair : ctx.globalFields) {
        ss << "uniform " << pair.second << " global_" << pair.first;
        if (reg && dynamic_cast<const GfxGslTextureType*>(pair.second)) {
            ss << " : register(s" << counter << ")";
            counter++;
        }
        ss << ";\n";
    }
    for (const auto &pair : ctx.matFields) {
        auto it = ctx.ubt.find(pair.first);
        if (it != ctx.ubt.end()) {
            ss << "const Float4 mat_" << pair.first << " = " << it->second << ";\n";
        } else {
            ss << "uniform " << pair.second << " mat_" << pair.first;
            if (reg && dynamic_cast<const GfxGslTextureType*>(pair.second)) {
                ss << " : register(s" << counter << ")";
                counter++;
            }
            ss << ";\n";
        }
    }
    for (const auto &pair : ctx.bodyFields) {
        ss << "uniform " << pair.second << " body_" << pair.first;
        if (reg && dynamic_cast<const GfxGslTextureType*>(pair.second)) {
            EXCEPTEX << "Not allowed" << ENDL;
            ss << " : register(s" << counter << ")";
            counter++;
        }
        ss << ";\n";
    }

    return ss.str();
}

std::string gfx_gasoline_generate_var_decls (const GfxGslTypeMap &vars) 
{
    std::stringstream ss;

    if (vars.size() == 0) return "";

    ss << "// Variable declarations\n";

    for (const auto &pair : vars)
        ss << pair.second << " " << pair.first << ";\n";

    ss << "\n";

    return ss.str();
}

std::string gfx_gasoline_generate_trans_encode (const std::vector<GfxGslTrans> &trans,
                                                const std::string &var_pref)
{
    std::stringstream ss;

    if (trans.size() == 0) return "";

    ss << "    // Encode interpolated vars\n";

    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        static const char *fields = "xyzw";
        for (unsigned j=0 ; j<sz ; ++j) {
            const auto &tran = trans[i+j];
            std::stringstream expr;
            switch (tran.kind) {
                case GfxGslTrans::VERT:
                expr << "vert_" << tran.path[0];
                break;
                case GfxGslTrans::USER:
                expr << var_pref << tran.path[0];
                break;
                case GfxGslTrans::INTERNAL:
                expr << "internal_" << tran.path[0];
                break;
            }
            for (unsigned k=1 ; k<tran.path.size() ; ++k)
                expr << "." << tran.path[k];
            ss << "    trans" << i/4;
            if (sz > 1) ss << "." << fields[j];
            ss << " = " << expr.str() << ";\n";

        }
    }
    ss << "\n";

    return ss.str();
}


std::string gfx_gasoline_generate_trans_decode(const std::vector<GfxGslTrans> &trans,
                                               const std::string &pref,
                                               GfxGslTrans::Kind only)
{
    std::stringstream ss;

    if (trans.size() == 0) return "";

    ss << "// Decode interpolated vars\n";

    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        static const char *fields = "xyzw";
        for (unsigned j=0 ; j<sz ; ++j) {
            const auto &tran = trans[i+j];
            std::stringstream expr;
            std::string prefix = pref;
            for (unsigned k=0 ; k<tran.path.size() ; ++k) {
                expr << prefix << tran.path[k];
                prefix = ".";
            }
            if (tran.kind == only) {
                ss << "    " << expr.str() << " = ";
                ss << "trans" << i/4;
                if (sz > 1) ss << "." << fields[j];
                ss << ";\n";
            }
        }
    }

    ss << "\n";

    return ss.str();
}

std::string gfx_gasoline_preamble_lighting (void) 
{
    std::stringstream ss;

    ss << "// Lighting functions\n";

    // Punctual lighting simulates the lighting of a fragment by rays all eminating from
    // the same point.  Arguments light_diffuse and light_specular are usually the same value.
    //     surf_to_light: surface to light vector
    //     surf_to_cam: surface to camera vector (typically computed in vertex shader)
    //     sd: surface diffuse colour
    //     sn: surface normal vector
    //     sg: surface gloss (0 to 1)
    //     ss: surface spec (0 to 1)
    //     light_diffuse: light intensity 0 to 1 or greater for hdr (number of photons, basically)
    //     light_specular: light intensity 0 to 1 or greater for hdr (number of photons, basically)
    ss << "Float3 punctual_lighting(Float3 surf_to_light, Float3 surf_to_cam,\n"
       << "                         Float3 sd, Float3 sn, Float sg, Float ss,\n";
    ss << "                         Float3 light_diffuse, Float3 light_specular)\n";
    ss << "{\n";

    ss << "    Float3 diff_component = light_diffuse * sd;\n";

    // specular component
    ss << "    Float3 surf_to_half = normalize(0.5*(surf_to_cam + surf_to_light));\n";
    ss << "    Float fresnel = ss + (1-ss)\n";
    ss << "                   * strength(1.0 - dot(surf_to_light, surf_to_half), 5);\n";
    ss << "    Float gloss = pow(4096.0, sg);\n";
    ss << "    Float highlight = 1.0/8 * (gloss+2) * strength(dot(sn, surf_to_half), gloss);\n";
    ss << "    Float3 spec_component = light_specular * fresnel * highlight;\n";

    ss << "    return (spec_component + diff_component) * max(0.0, dot(sn, surf_to_light));\n";
    ss << "}\n";

    // Env cube lighting simulates global illumination (i.e. light from all directions).
    ss << "Float3 env_lighting(Float3 surf_to_cam,\n"
       << "                    Float3 sd, Float3 sn, Float sg, Float ss,\n";
    ss << "                    FloatTextureCube cube, Float map_mipmaps)\n";
    ss << "{\n";
    ss << "    Float3 reflect_ws = -reflect(surf_to_cam, sn);\n";
    // Use 20 as the max mipmap, as we will never see a texture 2^20 big.
    ss << "    Float3 fresnel_light = gamma_decode(sampleLod(cube, reflect_ws, map_mipmaps - 1).rgb);\n";
    ss << "    Float3 diff_light = gamma_decode(sampleLod(cube, sn, map_mipmaps - 1).rgb);\n";
    ss << "    Float spec_mm = (1 - sg) * map_mipmaps;\n";  // Spec mip map
    ss << "    Float3 spec_light = gamma_decode(sampleLod(cube, reflect_ws, spec_mm).rgb);\n";
    ss << "    Float3 diff_component = sd * diff_light;\n";
    ss << "    Float3 spec_component = ss * spec_light;\n";
    ss << "    Float fresnel_factor = strength(1.0 - dot(sn, surf_to_cam), 5);\n";
    ss << "    Float3 fresnel_component = sg * fresnel_factor * fresnel_light;\n";
	ss << "    return 16 * (diff_component + spec_component + fresnel_component);\n";
	ss << "}\n";

    ss << "\n";

    return ss.str();
}


std::string gfx_gasoline_preamble_transformation (bool first_person, unsigned bone_weights,
                                                  bool instanced)
{
    (void) instanced;
    // TODO(dcunnin): Need to specialise transform_to_world and rotate_to_world for instanced
    // geometry.

    std::stringstream ss;
    ss << "// Standard library (vertex shader specific calls)\n";

    ss << "Float3 transform_to_world (Float3 v)\n";
    ss << "{\n";
    ss << "    Float3 r = Float3(0, 0, 0);\n";
    if (bone_weights == 0) {
        ss << "    r = mul(body_world, Float4(v, 1)).xyz;\n";
    } else {
        // The bone weights are supposed to sum to 1, we could remove this overconservative divide
        // if we could assert that property at mesh loading time.
        ss << "    Float total = 0;\n";
        for (unsigned i=0 ; i < bone_weights ; ++i) {
            ss << "    r += vert_boneWeights[" << i
               << "] * mul(body_boneWorlds[int(vert_boneAssignments[" << i
               << "])], Float4(v, 1)).xyz;\n";
            ss << "    total += vert_boneWeights[" << i << "];\n";
        }
        ss << "    r /= total;\n";
    }
    if (first_person)
        ss << "    r = mul(global_invView, Float4(r, 1)).xyz;\n";
    ss << "    return r;\n";
    ss << "}\n";
    ss << "Float3 rotate_to_world (Float3 v)\n";
    ss << "{\n";
    ss << "    Float3 r = Float3(0, 0, 0);\n";
    if (bone_weights == 0) {
        ss << "    r = mul(body_world, Float4(v, 0)).xyz;\n";
    } else {
        // The bone weights are supposed to sum to 1, we could remove this overconservative divide
        // if we could assert that property at mesh loading time.
        ss << "    Float total = 0;\n";
        for (unsigned i=0 ; i < bone_weights ; ++i) {
            ss << "    r += vert_boneWeights[" << i
               << "] * mul(body_boneWorlds[int(vert_boneAssignments[" << i
               << "])], Float4(v, 0)).xyz;\n";
            ss << "    total += vert_boneWeights[" << i << "];\n";
        }
        ss << "    r /= total;\n";
    }
    if (first_person)
        ss << "    r = mul(global_invView, Float4(r, 0)).xyz;\n";
    ss << "    return r;\n";
    ss << "}\n";
    ss << "\n";

    return ss.str();
}


