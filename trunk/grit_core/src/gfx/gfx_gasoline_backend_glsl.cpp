/* Copyright (c) David Cunningham and the Grit Game Engine project 2013
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

#include "gfx_gasoline_backend_cg.h"

#include "../centralised_log.h"

static std::map<std::string, std::string> vert_global = {
    {"position", "vertex"},
    {"coord0", "uv0"},
    {"coord1", "uv1"},
    {"coord2", "uv2"},
    {"coord3", "uv3"},
    {"coord4", "uv4"},
    {"coord5", "uv5"},
    {"coord6", "uv6"},
    {"coord7", "uv7"},
};

static std::map<std::string, std::string> frag_global = {
    {"position", "gl_Position"},
    {"colour", "out_colour"},
    {"screen", "gl_FragCoord"},
    {"depth", "gl_FragDepth"},
};

namespace {
    class Backend {
        std::stringstream ss;
        const GfxGslTypeSystem *ts;
        GfxGslKind kind;

        public:
        Backend (const GfxGslTypeSystem *ts, GfxGslKind kind)
          : ts(ts), kind(kind)
        { }

        void unparse (const GfxGslAst *ast_, int indent)
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
                ss << space << ast->init->type;
                ss << " user_" << ast->id << " = ";
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
                    if (kind == GFX_GSL_VERT) {
                        ss << vert_global[ast->id];
                    } else {
                        ss << "vert_" << ast->id;
                    }
                } else if (dynamic_cast<GfxGslFragType*>(ast->target->type)) {
                    ss << frag_global[ast->id];
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
                ss << "user_" << ast->id;
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

            } else {
                EXCEPTEX << "INTERNAL ERROR: Unknown GfxGslAst." << ENDL;
            }
        }

        std::string getOutput (void) { return ss.str(); }
    };
}

static inline bool in_set(const std::set<std::string> &set, const std::string &s)
{ return set.find(s) != set.end(); }

template<class T> 
static std::ostream &operator<< (std::ostream &o, const std::vector<T> &s) 
{ 
    if (s.empty()) { 
        o << "[ ]"; 
    } else { 
        const char *prefix = "["; 
        for (const auto &str : s) { 
            o << prefix << str; 
            prefix = ", "; 
        } 
        o << "]"; 
    } 
    return o; 
} 
 
template<class T> 
static std::ostream &operator<< (std::ostream &o, const std::set<T> &s) 
{ 
    if (s.empty()) { 
        o << "{ }"; 
    } else { 
        const char *prefix = "{"; 
        for (const auto &str : s) { 
            o << prefix << str; 
            prefix = ", "; 
        } 
        o << "}"; 
    } 
    return o; 
} 

template<class T> 
static std::ostream &operator<< (std::ostream &o, const std::map<std::string, T> &s) 
{ 
    if (s.empty()) { 
        o << "{ }"; 
    } else { 
        const char *prefix = "{"; 
        for (const auto &pair : s) { 
            o << prefix << pair.first << ": " << pair.second; 
            prefix = ", "; 
        } 
        o << "}"; 
    } 
    return o; 
} 

static void preamble (const GfxGslTypeSystem *ts, GfxGslKind k, std::ostream &ss,
                      const GfxGslUnboundTextures &ubt)
{
    ss << "#version 130\n";
    ss << "// " << (k == GFX_GSL_VERT ? "Vertex" : "Fragment") << " shader.\n";
    ss << "// This GLSL shader compiled from Gasoline, the Grit shading language.\n";
    ss << "\n";

    ss << "// Vert fields read: " << ts->getVertFieldsRead() << "\n";
    ss << "// Frag fields read: " << ts->getFragFieldsRead() << "\n";
    ss << "// Frag fields written: " << ts->getFragFieldsWritten() << "\n";
    ss << "// Material fields read: " << ts->getMatFieldsRead() << "\n";
    ss << "// Global fields read: " << ts->getGlobalFieldsRead() << "\n";
    ss << "// Trans-values: " << ts->getTrans() << "\n";
    ss << "// Unbound textures: " << ubt << "\n";
    ss << "\n";

    ss << "// GSL/GLSL Preamble:\n";
    ss << "#define Float float\n";
    ss << "#define Float2 vec2\n";
    ss << "#define Float3 vec3\n";
    ss << "#define Float4 vec4\n";
    ss << "#define Float1x1 float\n";
    ss << "#define Float1x2 mat1x2\n";
    ss << "#define Float1x3 mat1x3\n";
    ss << "#define Float1x4 mat1x4\n";
    ss << "#define Float2x1 mat2x1\n";
    ss << "#define Float2x2 mat2x2\n";
    ss << "#define Float2x3 mat2x3\n";
    ss << "#define Float2x4 mat2x4\n";
    ss << "#define Float3x1 mat3x1\n";
    ss << "#define Float3x2 mat3x2\n";
    ss << "#define Float3x3 mat3x3\n";
    ss << "#define Float3x4 mat3x4\n";
    ss << "#define Float4x1 mat4x1\n";
    ss << "#define Float4x2 mat4x2\n";
    ss << "#define Float4x3 mat4x3\n";
    ss << "#define Float4x4 mat4x4\n";
    ss << "#define FloatTexture sampler1D\n";
    ss << "#define FloatTexture2 sampler2D\n";
    ss << "#define FloatTexture3 sampler3D\n";
    ss << "Float atan2 (Float y, Float x) { return atan(y, x); }\n";
    if (k == GFX_GSL_FRAG) {
        ss << "Float ddx (Float v) { return dFdx(v); }\n";
        ss << "Float ddy (Float v) { return dFdy(v); }\n";
        ss << "Float2 ddx (Float2 v) { return dFdx(v); }\n";
        ss << "Float2 ddy (Float2 v) { return dFdy(v); }\n";
        ss << "Float3 ddx (Float3 v) { return dFdx(v); }\n";
        ss << "Float3 ddy (Float3 v) { return dFdy(v); }\n";
        ss << "Float4 ddx (Float4 v) { return dFdx(v); }\n";
        ss << "Float4 ddy (Float4 v) { return dFdy(v); }\n";
        ss << "Float4 sample2D (FloatTexture2 tex, Float2 uv) { return texture(tex, uv); }\n";
        ss << "Float4 sample2D (FloatTexture2 tex, Float2 uv, Float2 ddx, Float2 ddy) { return textureGrad(tex, uv, ddx, ddy); }\n";
        ss << "Float4 sample2D (Float4 c, Float2 uv) { return c; }\n";
        ss << "Float4 sample2D (Float4 c, Float2 uv, Float2 ddx, Float2 ddy) { return c; }\n";
    }
    ss << "Float mul (Float1x1 m, Float v) { return m * v; }\n";
    ss << "Float2 mul (Float2x2 m, Float2 v) { return m * v; }\n";
    ss << "Float3 mul (Float3x3 m, Float3 v) { return m * v; }\n";
    ss << "Float4 mul (Float4x4 m, Float4 v) { return m * v; }\n";
    ss << "Float4 pma_decode (Float4 v) { return Float4(v.xyz/v.w, v.w); }\n";
    ss << "Float  gamma_decode (Float v)  { return pow(v, 2.2); }\n";
    ss << "Float2 gamma_decode (Float2 v) { return pow(v, vec2(2.2, 2.2)); }\n";
    ss << "Float3 gamma_decode (Float3 v) { return pow(v, vec3(2.2, 2.2, 2.2)); }\n";
    ss << "Float4 gamma_decode (Float4 v) { return pow(v, vec4(2.2, 2.2, 2.2, 2.2)); }\n";
    ss << "Float  gamma_encode (Float v)  { return pow(v, 1/2.2); }\n";
    ss << "Float2 gamma_encode (Float2 v) { return pow(v, vec2(1/2.2, 1/2.2)); }\n";
    ss << "Float3 gamma_encode (Float3 v) { return pow(v, vec3(1/2.2, 1/2.2, 1/2.2)); }\n";
    ss << "Float4 gamma_encode (Float4 v) { return pow(v, vec4(1/2.2, 1/2.2, 1/2.2, 1/2.2)); }\n";
    ss << "Float lerp (Float a, Float b, Float v) { return v*b + (1-v)*a; }\n";
    ss << "Float2 lerp (Float2 a, Float2 b, Float v) { return v*b + (1-v)*a; }\n";
    ss << "Float3 lerp (Float3 a, Float3 b, Float v) { return v*b + (1-v)*a; }\n";
    ss << "Float4 lerp (Float4 a, Float4 b, Float v) { return v*b + (1-v)*a; }\n";
    ss << "\n";
}

void gfx_gasoline_unparse_glsl (const GfxGslTypeSystem *vert_ts, const GfxGslAst *vert_ast,
                                std::string &vert_output, const GfxGslTypeSystem *frag_ts,
                                const GfxGslAst *frag_ast, std::string &frag_output,
                                const GfxGslUnboundTextures &ubt)
{
    Backend vert_backend(vert_ts, GFX_GSL_VERT);
    vert_backend.unparse(vert_ast, 1);
    Backend frag_backend(frag_ts, GFX_GSL_FRAG);
    frag_backend.unparse(frag_ast, 1);

    std::stringstream vert_ss;
    preamble(vert_ts, GFX_GSL_VERT, vert_ss, ubt);

    const auto &trans = frag_ts->getTrans();
    auto vert_in = vert_ts->getVertFieldsRead();
    for (const auto &tran : trans) {
        if (!tran.isVert) continue;
        vert_in.insert(tran.path[0]);
    }

    for (const auto &f : vert_in) {
        vert_ss << "in " << vert_ts->getVertType(f) << " " << vert_global[f] << ";\n";
    }
    for (const auto &f : vert_ts->getGlobalFieldsRead())
        vert_ss << "uniform " << vert_ts->getGlobalType(f) << " global_" << f << ";\n";
    for (const auto &f : vert_ts->getMatFieldsRead()) {
        auto it = ubt.find(f);
        if (it != ubt.end()) {
            vert_ss << "const Float4 mat_" << f << " = " << it->second << ";\n";
        } else {
            vert_ss << "uniform " << vert_ts->getMatType(f) << " mat_" << f << ";\n";
        }
    }
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        vert_ss << "out " << type.str() << " trans" << i/4 << ";\n";
    }

    vert_ss << "void main (void)\n";
    vert_ss << "{\n";
    for (const auto &f : vert_ts->getFragFieldsWritten()) {
        vert_ss << "    " << frag_global[f] << " = vec4(0.0, 0.0, 0.0, 0.0);\n";
    }
    vert_ss << vert_backend.getOutput();
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        static const char *fields = "xyzw";
        for (unsigned j=0 ; j<sz ; ++j) {
            const auto &tran = trans[i+j];
            std::stringstream expr;
            if (tran.isVert)
                expr << vert_global[tran.path[0]];
            else
                expr << "user_" << tran.path[0];
            for (unsigned k=1 ; k<tran.path.size() ; ++k)
                expr << "." << tran.path[k];
            vert_ss << "    trans" << i/4;
            if (sz > 1) vert_ss << "." << fields[j];
            vert_ss << " = " << expr.str() << ";\n";

        }
    }
    vert_ss << "}\n";

    std::stringstream frag_ss;
    preamble(frag_ts, GFX_GSL_FRAG, frag_ss, ubt);

    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        frag_ss << "in " << type.str() << " trans" << i/4 << ";\n";
    }
    for (const auto &f : frag_ts->getGlobalFieldsRead())
        frag_ss << "uniform " << frag_ts->getGlobalType(f) << " global_" << f << ";\n";
    for (const auto &f : frag_ts->getMatFieldsRead()) {
        auto it = ubt.find(f);
        if (it != ubt.end()) {
            frag_ss << "const Float4 mat_" << f << " = " << it->second << ";\n";
        } else {
            frag_ss << "uniform " << frag_ts->getMatType(f) << " mat_" << f << ";\n";
        }
    }
    frag_ss << "out Float4 out_colour;\n";

    frag_ss << "void main (void)\n";
    frag_ss << "{\n";
    for (const auto &f : frag_ts->getFragFieldsWritten()) {
        frag_ss << "    " << frag_global[f] << " = vec4(0.0, 0.0, 0.0, 0.0);\n";
    }
    std::set<std::string> trans_vert;
    std::set<std::string> trans_var;
    for (const auto &tran : trans) {
        if (tran.isVert) {
            trans_vert.insert(tran.path[0]);
        } else {
            trans_var.insert(tran.path[0]);
        }
    }
    for (const auto &tv : trans_vert) {
        const GfxGslType *type = frag_ts->getVertType(tv);
        frag_ss << "    " << type << " vert_" << tv << ";\n";
    }
    for (const auto &tv : trans_var) {
        const GfxGslType *type = frag_ts->getVarType(tv);
        frag_ss << "    " << type << " user_" << tv << ";\n";
    }
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        static const char *fields = "xyzw";
        for (unsigned j=0 ; j<sz ; ++j) {
            const auto &tran = trans[i+j];
            std::stringstream expr;
            const char *prefix = tran.isVert ? "vert_" : "user_";
            for (unsigned k=0 ; k<tran.path.size() ; ++k) {
                expr << prefix << tran.path[k];
                prefix = ".";
            }
            frag_ss << "    " << expr.str() << " = ";
            frag_ss << "trans" << i/4;
            if (sz > 1) frag_ss << "." << fields[j];
            frag_ss << ";\n";
        }
    }
    frag_ss << frag_backend.getOutput();
    frag_ss << "}\n";

    vert_output = vert_ss.str();
    frag_output = frag_ss.str();
}
