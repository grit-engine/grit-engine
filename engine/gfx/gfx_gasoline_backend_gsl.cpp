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

#include <cstdlib>

#include <exception.h>

#include <centralised_log.h>

#include "gfx_gasoline_backend_gsl.h"

static void unparse (std::stringstream &ss, const GfxGslAst *ast_, int indent)
{
    std::string space(4 * indent, ' ');
    if (auto ast = dynamic_cast<const GfxGslBlock*>(ast_)) {
        ss << space << "{\n";
        for (auto stmt : ast->stmts) {
            unparse(ss, stmt, indent+1);
        }
        ss << space << "}\n";
    } else if (auto ast = dynamic_cast<const GfxGslShader*>(ast_)) {
        for (auto stmt : ast->stmts) {
            unparse(ss, stmt, indent);
        }
    } else if (auto ast = dynamic_cast<const GfxGslDecl*>(ast_)) {
        ss << space << "var" << " " << ast->id << " = ";
        unparse(ss, ast->init, indent);
        ss << ";";
        if (ast->init->type != nullptr) ss << "  // " << ast->init->type;
        ss << "\n";
    } else if (auto ast = dynamic_cast<const GfxGslIf*>(ast_)) {
        ss << space << "if (";
        unparse(ss, ast->cond, indent);
        ss << ") {\n";
        unparse(ss, ast->yes, indent+1);
        if (ast->no) {
            ss << space << "} else {\n";
            unparse(ss, ast->no, indent+1);
        }
        ss << space << "}\n";
    } else if (auto ast = dynamic_cast<const GfxGslAssign*>(ast_)) {
        ss << space;
        unparse(ss, ast->target, indent);
        ss << " = ";
        unparse(ss, ast->expr, indent);
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
                unparse(ss, arg, indent);
                sep = ", ";
            }
            ss << ")";
        }
    } else if (auto ast = dynamic_cast<const GfxGslField*>(ast_)) {
        ss << "(";
        unparse(ss, ast->target, indent);
        ss << ").";
        ss << ast->id;
    } else if (auto ast = dynamic_cast<const GfxGslLiteralInt*>(ast_)) {
        ss << ast->val;
    } else if (auto ast = dynamic_cast<const GfxGslLiteralFloat*>(ast_)) {
        ss << ast->val;
    } else if (auto ast = dynamic_cast<const GfxGslLiteralBoolean*>(ast_)) {
        ss << (ast->val ? "true" : "false");
    } else if (auto ast = dynamic_cast<const GfxGslVar*>(ast_)) {
        ss << ast->id;
    } else if (auto ast = dynamic_cast<const GfxGslBinary*>(ast_)) {
        ss << "(";
        unparse(ss, ast->a, indent);
        ss << " " << to_string(ast->op) << " ";
        unparse(ss, ast->b, indent);
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
        EXCEPTEX << "INTERNAL ERROR: Unknown Ast." << ENDL;
    }
}

std::string gfx_gasoline_unparse_gsl (const GfxGslTypeSystem *ts, const GfxGslAst *ast)
{
    std::stringstream ss;
    ss << "// Vert fields read: " << ts->getVertFieldsRead() << "\n";
    ss << "// Frag fields read: " << ts->getFragFieldsRead() << "\n";
    ss << "// Material fields read: " << ts->getMatFieldsRead() << "\n";
    ss << "// Global fields read: " << ts->getGlobalFieldsRead() << "\n";
    ss << "// Trans-values: " << ts->getTrans() << "\n";
    unparse(ss, ast, 0);
    return ss.str();
}


