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

#include <string>
#include <vector>
#include <ostream>

#ifndef GFX_GASOLINE
#define GFX_GASOLINE

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

struct Type {
};

// Abstract Syntax Tree
struct Ast {
    GfxGslLocation loc;
    Type *type;
    Ast (const GfxGslLocation &loc) : loc(loc), type(nullptr) { }
    virtual ~Ast(void) { }
};

// Idents do not have/need location info, therefore are not Asts.
struct Ident {
    std::string name;
    Ident (const std::string &name)
      : name(name)
    { }
};

static inline std::ostream &operator<<(std::ostream &o, const Ident *id)
{   
    o << id->name;
    return o;
}


class Allocator {
    std::vector<Ast*> poolAst;
    std::vector<Type*> poolType;
    std::vector<Ident*> poolIdent;
    public:
    Allocator (void) { }
    ~Allocator (void)
    {
        for (auto id : poolIdent) delete id;
        for (auto a : poolAst) delete a;
    }
    template<class T, class... Args> Ast *makeAst (Args... args)
    {
        auto *r = new T(args...);
        poolAst.push_back(r);
        return r;
    }
    template<class T, class... Args> Ast *makeType (Args... args)
    {
        auto *r = new T(args...);
        poolType.push_back(r);
        return r;
    }
    Ident *makeIdent (const std::string &name)
    {
        auto *r  = new Ident(name);
        poolIdent.push_back(r);
        return r;
    }
};

Ast *gfx_gasoline_parse (Allocator &alloc, const std::string &shader);

std::string gfx_gasoline_unparse (const Ast *ast);

#endif
