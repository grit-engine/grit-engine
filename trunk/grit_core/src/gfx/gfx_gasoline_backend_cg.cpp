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
    TypeSystem *ts;

    public:
    CgBackend (TypeSystem *ts)
      : ts(ts)
    { }

    void unparse (const GfxGslAst *ast_, int indent)
    {
        std::string space(4 * indent, ' ');
        if (auto ast = dynamic_cast<const Block*>(ast_)) {
            ss << space << "{\n";
            for (auto stmt : ast->stmts) {
                unparse(stmt, indent+1);
            }
            ss << space << "}\n";
        } else if (auto ast = dynamic_cast<const Shader*>(ast_)) {
            for (auto stmt : ast->stmts) {
                unparse(stmt, indent);
            }
        } else if (auto ast = dynamic_cast<const Decl*>(ast_)) {
            ss << space << ast_->init->type;
            ss << " " << ast->id << " = ";
            unparse(ast->init, indent);
            ss << ";\n";
        } else if (auto ast = dynamic_cast<const If*>(ast_)) {
            ss << space << "if (";
            unparse(ast->cond, indent);
            ss << ") {\n";
            unparse(ss, ast->yes, indent+1);
            if (ast->no) {
                ss << space << "} else {\n";
                unparse(ast->no, indent+1);
            }
            ss << space << "}\n";
        } else if (auto ast = dynamic_cast<const Assign*>(ast_)) {
            ss << space;
            unparse(ast->target, indent);
            ss << " = ";
            unparse(ast->expr, indent);
            ss << ";\n";
        } else if (dynamic_cast<const Discard*>(ast_)) {
            ss << space << "discard;\n";
        } else if (dynamic_cast<const Return*>(ast_)) {
            ss << space << "return;\n";

        } else if (auto ast = dynamic_cast<const Call*>(ast_)) {
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
        } else if (auto ast = dynamic_cast<const Field*>(ast_)) {
            ss << "(";
            unparse(ast->target, indent);
            ss << ").";
            ss << ast->id;
        } else if (auto ast = dynamic_cast<const LiteralInt*>(ast_)) {
            ss << ast->val;
        } else if (auto ast = dynamic_cast<const LiteralFloat*>(ast_)) {
            ss << ast->val;
        } else if (auto ast = dynamic_cast<const LiteralBoolean*>(ast_)) {
            ss << (ast->val ? "true" : "false");
        } else if (auto ast = dynamic_cast<const Var*>(ast_)) {
            ss << ast->id;
        } else if (auto ast = dynamic_cast<const Binary*>(ast_)) {
            ss << "(";
            unparse(ast->a, indent);
            ss << " " << to_string(ast->op) << " ";
            unparse(ast->b, indent);
            ss << ")";

        } else if (dynamic_cast<const Global*>(ast_)) {
            ss << "global";
        } else if (dynamic_cast<const Mat*>(ast_)) {
            ss << "mat";
        } else if (dynamic_cast<const Vert*>(ast_)) {
            ss << "vert";
        } else if (dynamic_cast<const Frag*>(ast_)) {
            ss << "frag";

        } else {
            EXCEPTEX << "INTERNAL ERROR: Unknown GfxGslAst." << ENDL;
        }
    }

    std::string getOutput (void) { return ss.str(); }
};

std::string gfx_gasoline_unparse_cg (GfxGslTypeSystem *ts, const GfxGslAst *ast)
{
    CgBackend backend(ts);
    backend.unparse(ast, 0);
    return backend.getOutput();
}


