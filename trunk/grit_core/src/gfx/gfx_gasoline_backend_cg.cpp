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

class CgBackend {
    std::stringstream ss;
    const GfxGslTypeSystem *ts;
    GfxGslKind kind;

    public:
    CgBackend (const GfxGslTypeSystem *ts, GfxGslKind kind)
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
                ss << "vert_" << ast->id;
            } else if (dynamic_cast<GfxGslFragType*>(ast->target->type)) {
                ss << "frag_" << ast->id;
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
            ss << "(";
            unparse(ast->a, indent);
            ss << " " << to_string(ast->op) << " ";
            unparse(ast->b, indent);
            ss << ")";

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
};

static std::map<std::string, std::string> frag_semantic = {
    {"position", "POSITION"},
    {"colour", "COLOR"},
    {"screen", "WPOS"},
    {"depth", "DEPTH"},
};

static void cg_preamble (const GfxGslTypeSystem *ts, std::ostream &ss)
{
    ss << "// This CG shader compiled from Gasoline, the Grit shading language.\n";
    ss << "\n";

    ss << "// Vert fields read: " << ts->getVertFieldsRead() << "\n";
    ss << "// Frag fields read: " << ts->getFragFieldsRead() << "\n";
    ss << "// Frag fields written: " << ts->getFragFieldsWritten() << "\n";
    ss << "// Material fields read: " << ts->getMatFieldsRead() << "\n";
    ss << "// Global fields read: " << ts->getGlobalFieldsRead() << "\n";
    ss << "// Trans-values: " << ts->getTrans() << "\n";
    ss << "\n";

    ss << "// GSL/CG Preamble:\n";
    ss << "typedef float Float;\n";
    ss << "typedef float2 Float2;\n";
    ss << "typedef float3 Float3;\n";
    ss << "typedef float4 Float4;\n";
    ss << "typedef float1x1 Float1x1;\n";
    ss << "typedef float1x2 Float1x2;\n";
    ss << "typedef float1x3 Float1x3;\n";
    ss << "typedef float1x4 Float1x4;\n";
    ss << "typedef float2x1 Float2x1;\n";
    ss << "typedef float2x2 Float2x2;\n";
    ss << "typedef float2x3 Float2x3;\n";
    ss << "typedef float2x4 Float2x4;\n";
    ss << "typedef float3x1 Float3x1;\n";
    ss << "typedef float3x2 Float3x2;\n";
    ss << "typedef float3x3 Float3x3;\n";
    ss << "typedef float3x4 Float3x4;\n";
    ss << "typedef float4x1 Float4x1;\n";
    ss << "typedef float4x2 Float4x2;\n";
    ss << "typedef float4x3 Float4x3;\n";
    ss << "typedef float4x4 Float4x4;\n";
    ss << "typedef sampler1D FloatTexture;\n";
    ss << "typedef sampler2D FloatTexture2;\n";
    ss << "typedef sampler3D FloatTexture3;\n";
    ss << "Float4 pma_decode (Float4 v) { return Float4(v.xyz/v.w, v.w); }\n";
    ss << "Float  gamma_decode (Float v)  { return pow(v, 2.2); }\n";
    ss << "Float2 gamma_decode (Float2 v) { return pow(v, 2.2); }\n";
    ss << "Float3 gamma_decode (Float3 v) { return pow(v, 2.2); }\n";
    ss << "Float4 gamma_decode (Float4 v) { return pow(v, 2.2); }\n";
    ss << "Float  gamma_encode (Float v)  { return pow(v, 1/2.2); }\n";
    ss << "Float2 gamma_encode (Float2 v) { return pow(v, 1/2.2); }\n";
    ss << "Float3 gamma_encode (Float3 v) { return pow(v, 1/2.2); }\n";
    ss << "Float4 gamma_encode (Float4 v) { return pow(v, 1/2.2); }\n";
    ss << "Float4 sample2D (FloatTexture2 tex, Float2 uv) { return tex2D(tex, uv); }\n";
    ss << "Float4 sample2D (FloatTexture2 tex, Float2 uv, Float2 ddx, Float2 ddy) { return tex2D(tex, uv, ddx, ddy); }\n";
    ss << "\n";
}

void gfx_gasoline_unparse_cg (const GfxGslTypeSystem *vert_ts, const GfxGslAst *vert_ast,
                              std::string &vert_output, const GfxGslTypeSystem *frag_ts,
                              const GfxGslAst *frag_ast, std::string &frag_output)
{
    CgBackend vert_backend(vert_ts, GFX_GSL_VERT);
    vert_backend.unparse(vert_ast, 1);
    CgBackend frag_backend(frag_ts, GFX_GSL_FRAG);
    frag_backend.unparse(frag_ast, 1);

    std::stringstream vert_ss;
    cg_preamble(vert_ts, vert_ss);

    const auto &trans = frag_ts->getTrans();
    auto vert_in = vert_ts->getVertFieldsRead();
    for (const auto &tran : trans) {
        if (!tran.isVert) continue;
        vert_in.insert(tran.path[0]);
    }

    vert_ss << "void main (\n";
    for (const auto &f : vert_in) {
        vert_ss << "    in " << vert_ts->getVertType(f) << " vert_" << f
                << " : " << vert_semantic[f] << ",\n";
    }

    for (const auto &f : vert_ts->getGlobalFieldsRead())
        vert_ss << "    uniform " << vert_ts->getGlobalType(f) << " global_" << f << ",\n";
    for (const auto &f : vert_ts->getMatFieldsRead())
        vert_ss << "    uniform " << vert_ts->getMatType(f) << " mat_" << f << ",\n";

    for (const auto &f : vert_ts->getFragFieldsWritten()) {
        vert_ss << "    out " << vert_ts->getFragType(f) << " frag_" << f << " : "
                << frag_semantic[f] << ",\n";
    }
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        vert_ss << "    out " << type.str() << " trans" << i/4 << " : TEXCOORD" << i/4 << ",\n";
    }

    vert_ss << "    uniform float vert_unused\n";
    vert_ss << ") {\n";
    for (const auto &f : vert_ts->getFragFieldsWritten()) {
        vert_ss << "    frag_" << f << " = 0.0;\n";
    }
    vert_ss << vert_backend.getOutput();
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
            vert_ss << "    trans" << i/4 << "." << fields[j] << " = " << expr.str() << ";\n";
        }
    }
    vert_ss << "}\n";

    std::stringstream frag_ss;
    cg_preamble(frag_ts, frag_ss);

    frag_ss << "void main (\n";
    for (const auto &f : frag_ts->getFragFieldsRead()) {
        frag_ss << "    in " << frag_ts->getFragType(f) << " frag_" << f
                << " : " << frag_semantic[f] << ",\n";
    }
    for (unsigned i=0 ; i<trans.size() ; i+=4) {
        unsigned sz = trans.size()-i > 4 ? 4 : trans.size()-i;
        std::stringstream type;
        type << "Float";
        if (sz > 1) type << sz;
        frag_ss << "    in " << type.str() << " trans" << i/4 << " : TEXCOORD" << i/4 << ",\n";
    }
    for (const auto &f : frag_ts->getGlobalFieldsRead())
        frag_ss << "    uniform " << frag_ts->getGlobalType(f) << " global_" << f << ",\n";
    for (const auto &f : frag_ts->getMatFieldsRead())
        frag_ss << "    uniform " << frag_ts->getMatType(f) << " mat_" << f << ",\n";

    for (const auto &f : frag_ts->getFragFieldsWritten())
        frag_ss << "    out " << frag_ts->getFragType(f) << " frag_" << f << ",\n";

    frag_ss << "    uniform float frag_unused\n";
    frag_ss << ") {\n";
    for (const auto &f : frag_ts->getFragFieldsWritten()) {
        frag_ss << "    frag_" << f << " = 0.0;\n";
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
            frag_ss << "    " << expr.str() << " = " << "trans" << i/4 << "." << fields[j] << ";\n";
        }
    }
    frag_ss << frag_backend.getOutput();
    frag_ss << "}\n";

    vert_output = vert_ss.str();
    frag_output = frag_ss.str();
}
