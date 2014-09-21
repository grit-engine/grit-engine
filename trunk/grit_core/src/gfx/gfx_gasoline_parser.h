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

#include <cstdlib>
#include <cstdint>

#include <map>
#include <string>
#include <vector>

#include <exception.h>

#ifndef GFX_GASOLINE_PARSER
#define GFX_GASOLINE_PARSER

struct GfxGslLocation {
    int line, col;
    GfxGslLocation (void) : line(0), col(0) { }
    GfxGslLocation (int line, int col) : line(line), col(col) { }
    bool valid (void) { return line > 0; }
};

static inline std::ostream &operator<<(std::ostream &o, const GfxGslLocation &loc)
{   
    o << loc.line << ":" << loc.col;
    return o;
}

struct GfxGslType {
    bool writeable;
    bool readable;
    GfxGslType() : writeable(false), readable(true) { }
    virtual ~GfxGslType (void) { }
};

// Abstract Syntax Tree
struct GfxGslAst {
    GfxGslLocation loc;
    GfxGslType *type;
    GfxGslAst (const GfxGslLocation &loc) : loc(loc), type(nullptr) { }
    virtual ~GfxGslAst(void) { }
};

typedef std::vector<GfxGslAst*> GfxGslAsts;

struct GfxGslShader : public GfxGslAst {
    GfxGslAsts stmts;
    GfxGslShader (const GfxGslLocation &loc, const GfxGslAsts &stmts)
      : GfxGslAst(loc), stmts(stmts)
    { }
};

struct GfxGslBlock : public GfxGslAst {
    GfxGslAsts stmts;
    GfxGslBlock (const GfxGslLocation &loc, const GfxGslAsts &stmts)
      : GfxGslAst(loc), stmts(stmts)
    { }
};

struct GfxGslDecl : public GfxGslAst {
    bool immutable;
    const std::string id;
    GfxGslAst *init;
    GfxGslDecl (const GfxGslLocation &loc, bool immutable, const std::string id, GfxGslAst *init)
      : GfxGslAst(loc), immutable(immutable), id(id), init(init)
    { }
};

struct GfxGslIf : public GfxGslAst {
    GfxGslAst *cond;
    GfxGslAst *yes;
    GfxGslAst *no;
    GfxGslIf (const GfxGslLocation &loc, GfxGslAst *cond, GfxGslAst *yes, GfxGslAst *no)
      : GfxGslAst(loc), cond(cond), yes(yes), no(no)
    { }
};

struct GfxGslAssign : public GfxGslAst {
    GfxGslAst *target;
    GfxGslAst *expr;
    GfxGslAssign (const GfxGslLocation &loc, GfxGslAst *target, GfxGslAst *expr)
      : GfxGslAst(loc), target(target), expr(expr)
    { }
};

struct GfxGslCall : public GfxGslAst {
    const std::string func;
    GfxGslAsts args;
    GfxGslCall (const GfxGslLocation &loc, const std::string &func, GfxGslAsts args)
      : GfxGslAst(loc), func(func), args(args)
    { }
};

struct GfxGslField : public GfxGslAst {
    GfxGslAst *target;
    const std::string id;
    GfxGslField (const GfxGslLocation &loc, GfxGslAst *target, const std::string &id)
      : GfxGslAst(loc), target(target), id(id)
    { }
};

template<class T> struct GfxGslLiteral : public GfxGslAst {
    T val;
    GfxGslLiteral (const GfxGslLocation &loc, T val)
      : GfxGslAst(loc), val(val)
    { }
};

typedef GfxGslLiteral<int32_t> GfxGslLiteralInt;
typedef GfxGslLiteral<float> GfxGslLiteralFloat;
typedef GfxGslLiteral<bool> GfxGslLiteralBoolean;

struct GfxGslVar : public GfxGslAst {
    const std::string id;
    GfxGslVar (const GfxGslLocation &loc, const std::string &id)
      : GfxGslAst(loc), id(id)
    { }
};

enum GfxGslOp {
    GFX_GSL_OP_ADD,
    GFX_GSL_OP_SUB,
    GFX_GSL_OP_MUL,
    GFX_GSL_OP_DIV,
    GFX_GSL_OP_MOD,
    GFX_GSL_OP_EQ,
    GFX_GSL_OP_NE,
    GFX_GSL_OP_LT,
    GFX_GSL_OP_LTE,
    GFX_GSL_OP_GT,
    GFX_GSL_OP_GTE,
    GFX_GSL_OP_AND,
    GFX_GSL_OP_OR
};

static inline std::string to_string (GfxGslOp op)
{
    switch (op) {
        case GFX_GSL_OP_ADD: return "+";
        case GFX_GSL_OP_SUB: return "-";
        case GFX_GSL_OP_MUL: return "*";
        case GFX_GSL_OP_DIV: return "/";
        case GFX_GSL_OP_MOD: return "%";
        case GFX_GSL_OP_EQ: return "==";
        case GFX_GSL_OP_LT: return "<";
        case GFX_GSL_OP_LTE: return "<=";
        case GFX_GSL_OP_GT: return ">";
        case GFX_GSL_OP_GTE: return ">=";
        case GFX_GSL_OP_AND: return "&&";
        case GFX_GSL_OP_OR: return "||";
        default: EXCEPTEX << "INTERNAL ERROR: Unknown binary operator: " << op << ENDL;
    }
}

static inline std::ostream &operator<< (std::ostream &o, GfxGslOp op)
{
    o << to_string(op);
    return o;
}

static const std::map<std::string, GfxGslOp> op_map = {
    {"*", GFX_GSL_OP_MUL},
    {"/", GFX_GSL_OP_DIV},
    {"%", GFX_GSL_OP_MOD},

    {"+", GFX_GSL_OP_ADD},
    {"-", GFX_GSL_OP_SUB},

    {"<", GFX_GSL_OP_LT},
    {"<=", GFX_GSL_OP_LTE},
    {">", GFX_GSL_OP_GT},
    {">=", GFX_GSL_OP_GTE},

    {"==", GFX_GSL_OP_EQ},
    {"!=", GFX_GSL_OP_NE},

    {"&&", GFX_GSL_OP_AND},
    {"||", GFX_GSL_OP_OR},
};


struct GfxGslBinary : public GfxGslAst {
    GfxGslAst *a;
    GfxGslOp op;
    GfxGslAst *b;
    GfxGslBinary (const GfxGslLocation &loc, GfxGslAst *a, GfxGslOp op, GfxGslAst *b)
      : GfxGslAst(loc), a(a), op(op), b(b)
    { }
};


// Keywords

struct GfxGslGlobal : public GfxGslAst {
    GfxGslGlobal (const GfxGslLocation &loc) : GfxGslAst(loc) { }
};
struct GfxGslMat : public GfxGslAst {
    GfxGslMat (const GfxGslLocation &loc) : GfxGslAst(loc) { }
};
struct GfxGslVert : public GfxGslAst {
    GfxGslVert (const GfxGslLocation &loc) : GfxGslAst(loc) { }
};
struct GfxGslFrag : public GfxGslAst {
    GfxGslFrag (const GfxGslLocation &loc) : GfxGslAst(loc) { }
};
struct GfxGslDiscard : public GfxGslAst {
    GfxGslDiscard (const GfxGslLocation &loc) : GfxGslAst(loc) { }
};
struct GfxGslReturn : public GfxGslAst {
    GfxGslReturn (const GfxGslLocation &loc) : GfxGslAst(loc) { }
};

class GfxGslAllocator {
    std::vector<GfxGslAst*> poolAst;
    std::vector<GfxGslType*> poolType;
    public:
    GfxGslAllocator (void) { }
    ~GfxGslAllocator (void)
    {
        for (auto a : poolAst) delete a;
        for (auto t : poolType) delete t;
    }
    template<class T, class... Args> GfxGslAst *makeAst (Args... args)
    {
        auto *r = new T(args...);
        poolAst.push_back(r);
        return r;
    }
    template<class T, class... Args> T *makeType (Args... args)
    {
        auto *r = new T(args...);
        poolType.push_back(r);
        return r;
    }
};

GfxGslAst *gfx_gasoline_parse (GfxGslAllocator &alloc, const std::string &shader);

#endif
