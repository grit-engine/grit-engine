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
    bool writeable;  // When the type represents an object, if fields of that object can be written.
    GfxGslType() : writeable(false) { }
    virtual ~GfxGslType (void) { }
};
typedef std::vector<GfxGslType*> GfxGslTypes;
typedef std::map<std::string, const GfxGslType*> GfxGslTypeMap;

// When a variable is in scope, features of that variable are stored here.
// This would be a good place for constness, volatile, or things like that.
struct GfxGslDef {
    GfxGslType *type;  // Of the values that can be assigned to the variable.
    bool trans;  // Whether it is interpolated.
    bool topLevel;  // Whether it can be captured by fragment shader.
    GfxGslDef(GfxGslType *type, bool trans, bool top_level)
      : type(type), trans(trans), topLevel(top_level)
    { }
};  
typedef std::map<std::string, const GfxGslDef*> GfxGslDefMap;

std::ostream &operator<<(std::ostream &o, const GfxGslType *t_);


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
    GfxGslDefMap vars;
    GfxGslShader (const GfxGslLocation &loc, const GfxGslAsts &stmts)
      : GfxGslAst(loc), stmts(stmts)
    { }
};

struct GfxGslBlock : public GfxGslAst {
    GfxGslAsts stmts;
    GfxGslDefMap vars;
    GfxGslBlock (const GfxGslLocation &loc, const GfxGslAsts &stmts)
      : GfxGslAst(loc), stmts(stmts)
    { }
};

struct GfxGslDecl : public GfxGslAst {
    std::string id;
    GfxGslDef *def;
    GfxGslType *annot;
    GfxGslAst *init;
    GfxGslDecl (const GfxGslLocation &loc, const std::string &id, GfxGslType *annot,
                GfxGslAst *init)
      : GfxGslAst(loc), id(id), def(nullptr), annot(annot), init(init)
    { }
};

struct GfxGslFor : public GfxGslAst {
    std::string id;  // If id != "" then of the form for (var x = init; ...)
    GfxGslDef *def;
    GfxGslType *annot;
    GfxGslAst *init;  // Otherwise, of the form for (init; ...)
    GfxGslDefMap vars;
    GfxGslAst *cond;
    GfxGslAst *inc;
    GfxGslAst *body;
    GfxGslFor (const GfxGslLocation &loc, const std::string &id, GfxGslType *annot, GfxGslAst *init,
               GfxGslAst *cond, GfxGslAst *inc, GfxGslAst *body)
      : GfxGslAst(loc), id(id), def(nullptr), annot(annot), init(init), cond(cond), inc(inc),
        body(body)
    { }
};

struct GfxGslIf : public GfxGslAst {
    GfxGslAst *cond;
    GfxGslAst *yes;
    GfxGslDefMap yesVars;
    GfxGslAst *no;
    GfxGslDefMap noVars;
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

struct GfxGslLiteralArray : public GfxGslAst {
    std::vector<GfxGslAst*> elements;
    GfxGslLiteralArray (const GfxGslLocation &loc, const std::vector<GfxGslAst*> &elements)
      : GfxGslAst(loc), elements(elements)
    { }
};

struct GfxGslArrayLookup : public GfxGslAst {
    GfxGslAst *target;
    GfxGslAst *index;
    GfxGslArrayLookup (const GfxGslLocation &loc, GfxGslAst *target, GfxGslAst *index)
      : GfxGslAst(loc), target(target), index(index)
    { }
};

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
        case GFX_GSL_OP_NE: return "!=";
        case GFX_GSL_OP_LT: return "<";
        case GFX_GSL_OP_LTE: return "<=";
        case GFX_GSL_OP_GT: return ">";
        case GFX_GSL_OP_GTE: return ">=";
        case GFX_GSL_OP_AND: return "&&";
        case GFX_GSL_OP_OR: return "||";
        default: EXCEPTEX << "INTERNAL ERROR: Unknown binary operator: " << int(op) << ENDL;
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
struct GfxGslOut : public GfxGslAst {
    GfxGslOut (const GfxGslLocation &loc) : GfxGslAst(loc) { }
};
struct GfxGslBody : public GfxGslAst {
    GfxGslBody (const GfxGslLocation &loc) : GfxGslAst(loc) { }
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
    std::vector<GfxGslDef*> poolDef;
    public:
    GfxGslAllocator (void) { }
    ~GfxGslAllocator (void)
    {
        for (auto a : poolAst) delete a;
        for (auto t : poolType) delete t;
        for (auto t : poolDef) delete t;
    }
    template<class T, class... Args> T *makeAst (Args... args)
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
    template<class T, class... Args> T *makeDef (Args... args)
    {
        auto *r = new T(args...);
        poolDef.push_back(r);
        return r;
    }
};

GfxGslShader *gfx_gasoline_parse (GfxGslAllocator &alloc, const std::string &shader);

#endif
