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

#include <map>
#include <set>
#include <string>
#include <vector>

#include "gfx_gasoline.h"
#include "gfx_gasoline_parser.h"

#ifndef GFX_GASOLINE_TYPE_SYSTEM
#define GFX_GASOLINE_TYPE_SYSTEM

enum GfxGslKind {
    GFX_GSL_VERTEX,
    GFX_GSL_DANGS,
    GFX_GSL_COLOUR_ALPHA,
    GFX_GSL_TONE_MAP,
    GFX_GSL_FOG
};

typedef std::vector<GfxGslType*> GfxGslTypes;

std::ostream &operator<<(std::ostream &o, const GfxGslType *t_);

// Primitives

struct GfxGslArrayType : public GfxGslType {
    unsigned size;
    GfxGslType *elementType;
    GfxGslArrayType (unsigned size, GfxGslType *element_type)
      : size(size), elementType(element_type)
    { }
};

struct GfxGslCoordType : public GfxGslType {
    unsigned dim;
    GfxGslCoordType (unsigned dim) : dim(dim) { }
};

struct GfxGslFloatType : public GfxGslCoordType {
    GfxGslFloatType (unsigned dim) : GfxGslCoordType(dim) { }
};

struct GfxGslFloatMatrixType : public GfxGslType {
    unsigned w;
    unsigned h;
    GfxGslFloatMatrixType (unsigned w, unsigned h) : w(w), h(h) { }
};

struct GfxGslTextureType : public GfxGslType {
    GfxGslFloatVec solid;  // This should not be in the type, it should be in the GfxGslContext
    GfxGslTextureType(const GfxGslFloatVec &solid)
      : solid(solid) { }
};

struct GfxGslFloatTextureType : public GfxGslTextureType {
    unsigned dim;
    GfxGslFloatTextureType (const GfxGslFloatVec &solid, unsigned dim)
      : GfxGslTextureType(solid), dim(dim)
    { }
};

struct GfxGslFloatTextureCubeType : public GfxGslTextureType {
    GfxGslFloatTextureCubeType (const GfxGslFloatVec &solid)
      : GfxGslTextureType(solid)
    { }
};

struct GfxGslIntType : public GfxGslCoordType {
    GfxGslIntType (unsigned dim) : GfxGslCoordType(dim) { }
};

struct GfxGslBoolType : public GfxGslType {
    GfxGslBoolType (void) { }
};


struct GfxGslVoidType : public GfxGslType {
    GfxGslVoidType (void) { }
};

// Engine access types

struct GfxGslGlobalType : public GfxGslType {
    GfxGslGlobalType (void) { }
};

struct GfxGslMatType : public GfxGslType {
    GfxGslMatType (void) { }
};

struct GfxGslOutType : public GfxGslType {
    GfxGslOutType (void) { }
};

struct GfxGslBodyType : public GfxGslType {
    GfxGslBodyType (void) { }
};

struct GfxGslVertType : public GfxGslType {
    GfxGslVertType (void) { }
};

struct GfxGslFragType : public GfxGslType {
    GfxGslFragType (void) { }
};


struct GfxGslFunctionType : public GfxGslType {
    GfxGslTypes params;
    GfxGslType *ret;
    GfxGslFunctionType (const GfxGslTypes &params, GfxGslType *ret) : params(params), ret(ret) { }
};


struct GfxGslContext {
    GfxGslAllocator &alloc;
    std::map<std::string, std::vector<GfxGslFunctionType*>> funcTypes;
    GfxGslTypeMap globalFields;
    GfxGslTypeMap matFields;
    GfxGslTypeMap bodyFields;
    GfxGslUnboundTextures ubt;
    bool d3d9;

    const GfxGslType *getMatType (const std::string &f) const
    {
        auto it = matFields.find(f);
        if (it == matFields.end()) return nullptr;
        return it->second;
    }

    const GfxGslType *getGlobalType (const std::string &f) const
    {
        auto it = globalFields.find(f);
        if (it == globalFields.end()) return nullptr;
        return it->second;
    }

    const GfxGslType *getBodyType (const std::string &f) const
    {
        auto it = bodyFields.find(f);
        if (it == bodyFields.end()) return nullptr;
        return it->second;
    }

};

struct GfxGslTrans {
    enum Kind { USER, VERT, INTERNAL };
    Kind kind;
    std::vector<std::string> path;
    const GfxGslType *type;  // Of path[0]
    bool operator== (const GfxGslTrans &other) const
    {
        if (kind != other.kind) return false;
        if (path != other.path) return false;
        return true;
    }
    bool operator< (const GfxGslTrans &other) const
    {
        if (kind < other.kind) return true;
        if (kind > other.kind) return false;
        return path < other.path;
    }
};

// TODO(dcunnin):  Disallow explicit & implicit use of ddx/ddy in conditional code.
class GfxGslTypeSystem {
    GfxGslContext &ctx;

    GfxGslTypeMap outFields;
    GfxGslTypeMap fragFields;
    GfxGslTypeMap bodyFields;
    GfxGslTypeMap vertFields;
    GfxGslDefMap vars;

    std::set<std::string> fragFieldsRead;
    std::set<std::string> vertFieldsRead;
    std::set<std::string> globalFieldsRead;
    std::set<std::string> bodyFieldsRead;
    std::set<std::string> matFieldsRead;
    std::set<std::string> outFieldsWritten;

    std::set<GfxGslTrans> trans;

    GfxGslType *cloneType (const GfxGslType *t_);

    void initObjectTypes (GfxGslKind k);

    GfxGslFunctionType *lookupFunction (const GfxGslLocation &loc, const std::string &name,
                                        GfxGslAsts &asts);

    bool isVoid (GfxGslType *x)
    {
        return dynamic_cast<GfxGslVoidType*>(x) != nullptr;
    }

    bool isFirstClass (const GfxGslType *x);

    bool equal (GfxGslType *a_, GfxGslType *b_);

    
    // Can the type be automatically converted
    bool conversionExists (GfxGslType *from_, GfxGslType *to_);

    // Wrap from, if necessary, to convert type.
    void doConversion (GfxGslAst *&from, GfxGslType *to_);

    void unify (const GfxGslLocation &loc, GfxGslAst *&a, GfxGslAst *&b);

    struct Ctx {
        /* read indicates that the object that the expression resolves to (if any) may be read
         * write indicates that the object that the expression resolves to (if any) may be written
         * path is the field path we will read from this object
         */
        const Ctx *parent;
        bool read;
        bool write;
        bool topLevel;
        std::vector<std::string> path;
        GfxGslDefMap *vars;
        Ctx (void)
          : parent(nullptr), read(false), write(false), topLevel(true), vars(nullptr)
        { }
        Ctx setRead (bool v) const
        {
            Ctx c = resetPath();
            c.read = v;
            return c;
        }
        Ctx setWrite (bool v) const
        {
            Ctx c = resetPath();
            c.write = v;
            return c;
        }
        Ctx setTopLevel (bool v)const
        {
            Ctx c = *this;
            c.topLevel = v;
            return c;
        }
        Ctx appendVars (GfxGslDefMap *v)const
        {
            Ctx c = *this;
            c.parent = this;
            c.vars = v;
            c.topLevel = false;
            return c;
        }
        Ctx appendPath (const std::string &v) const
        {
            Ctx c = *this;
            c.path.insert(c.path.begin(), v);
            return c;
        }
        Ctx resetPath (void) const
        {
            Ctx c = *this;
            c.path.clear();
            return c;
        }
        const GfxGslDef *lookupVar(const std::string &id) const
        {
            if (vars != nullptr) {
                auto it = vars->find(id);
                if (it != vars->end())
                    return it->second;
            }
            if (parent != nullptr)
                return parent->lookupVar(id);
            return nullptr;
        }
    };

    void addTrans (const GfxGslLocation &loc, const std::vector<std::string> &path,
                   const GfxGslType *type, bool vert);

    void inferAndSet (GfxGslAst *ast_, const Ctx &c);

    public:

    const GfxGslKind kind;

    GfxGslTypeSystem (GfxGslContext &ctx, GfxGslKind kind)
      : ctx(ctx), kind(kind)
    {
        initObjectTypes(kind);
    }

    /** For all nodes in the AST, infer type and set that type within the node.
     *
     * This is the core type inference algorithm.
     */
    void inferAndSet (GfxGslShader *ast, const GfxGslDefMap &outer);

    /** Get the variables declared by this shader. */
    const GfxGslDefMap &getVars (void) const { return vars; }

    /** Get the vertex fields read by this shader (applies to all shaders). */
    const std::set<std::string> &getVertFieldsRead (void) const { return vertFieldsRead; }

    /** Get the fragment fields read by this shader (fragment shader only). */
    const std::set<std::string> &getFragFieldsRead (void) const { return fragFieldsRead; }

    /** Get the material fields read by this shader. */
    const std::set<std::string> &getMatFieldsRead (void) const { return matFieldsRead; }

    /** Get the global fields read by this shader. */
    const std::set<std::string> &getGlobalFieldsRead (void) const { return globalFieldsRead; }

    /** Get the global fields read by this shader. */
    const std::set<std::string> &getBodyFieldsRead (void) const { return bodyFieldsRead; }

    /** Get the out fields written by this shader. */
    const std::set<std::string> &getOutFieldsWritten (void) const { return outFieldsWritten; }

    /** Get type of a vertex attribute (only ones available to this shader). */
    const GfxGslType *getVertType (const std::string &f) const
    {
        auto it = vertFields.find(f);
        if (it == vertFields.end()) return nullptr;
        return it->second;
    }

    /** Get type of a fragment attribute (only ones available to this shader). */
    const GfxGslType *getFragType (const std::string &f) const
    {
        auto it = fragFields.find(f);
        if (it == fragFields.end()) return nullptr;
        return it->second;
    }

    /** The values (v.xy paths) that were used in this shader and captured from the one before it.
     */
    const std::set<GfxGslTrans> &getTrans (void) const { return trans; }

    /** Same as getTrans() but return a vector instead of a set. */
    std::vector<GfxGslTrans> getTransVector (void) const
    {
        return std::vector<GfxGslTrans>(trans.begin(), trans.end());
    }
};

static inline std::ostream &operator<< (std::ostream &o, const GfxGslTrans &t)
{
    const char *prefix = "";
    if (t.kind == GfxGslTrans::VERT)
        o << "vert.";
    for (unsigned i=0 ; i<t.path.size() ; ++i) {  
        o << prefix << t.path[i];
        prefix = ".";
    }
    return o;
}

#endif
