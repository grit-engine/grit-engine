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

#include <map>
#include <set>
#include <string>
#include <vector>

#include "gfx_gasoline_parser.h"

#ifndef GFX_GASOLINE_TYPE_SYSTEM
#define GFX_GASOLINE_TYPE_SYSTEM

enum GfxGslKind {
    GFX_GSL_VERT,
    GFX_GSL_FRAG
};

typedef std::vector<GfxGslType*> GfxGslTypes;
typedef std::map<std::string, const GfxGslType*> GfxGslTypeMap;

std::ostream &operator<<(std::ostream &o, const GfxGslType *t_);

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

struct GfxGslFloatTextureType : public GfxGslType {
    unsigned dim;
    GfxGslFloatTextureType (unsigned dim) : dim(dim) { }
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

struct GfxGslGlobalType : public GfxGslType {
    GfxGslGlobalType (void) { }
};

struct GfxGslMatType : public GfxGslType {
    GfxGslMatType (void) { }
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

class GfxGslTypeSystem {
    GfxGslAllocator &alloc;

    std::map<std::string, std::vector<GfxGslFunctionType*>> funcTypes;
    struct FieldType {
        GfxGslType *t;
        bool vr; // readable / writeable in fragment/vertex shader
        bool vw;
        bool fr;
        bool fw;
        FieldType (GfxGslType *t, bool vr, bool vw, bool fr, bool fw)
          : t(t), vr(vr), vw(vw), fr(fr), fw(fw)
        { }
        FieldType (void)
        { }
    };
    std::map<std::string, FieldType> fragFields;
    GfxGslTypeMap vertFields;
    GfxGslTypeMap globalFields;
    GfxGslTypeMap matFields;
    GfxGslTypeMap vars;
    const GfxGslTypeMap outerVars;

    std::set<std::string> fragFieldsRead;
    std::set<std::string> vertFieldsRead;
    std::set<std::string> globalFieldsRead;
    std::set<std::string> matFieldsRead;


    public:
    struct Trans {
        bool isVert;
        std::vector<std::string> path;
    };

    private:
    std::vector<Trans> trans;

    std::set<std::string> fragFieldsWritten;

    GfxGslKind kind;

    GfxGslType *cloneType (const GfxGslType *t_);

    void initObjectTypes (void);

    void initFuncTypes (void);

    GfxGslFunctionType *lookupFunction(const GfxGslLocation &loc, const std::string &name,
                                       GfxGslAsts &asts);

    bool isVoid (GfxGslType *x)
    {
        return dynamic_cast<GfxGslVoidType*>(x) != nullptr;
    }

    bool isFirstClass (GfxGslType *x);

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
        bool read;
        bool write;
        std::vector<std::string> path;
        Ctx (void) : read(false), write(false) { }
        Ctx setRead (bool v) const
        {
            Ctx c = *this;
            c.read = v;
            return c;
        }
        Ctx setWrite (bool v) const
        {
            Ctx c = *this;
            c.write = v;
            return c;
        }
        Ctx appendPath (const std::string &v) const
        {
            Ctx c = *this;
            c.path.insert(c.path.begin(), v);
            return c;
        }
    };

    void addTrans (const GfxGslLocation &loc, const std::vector<std::string> &path,
                   const GfxGslType *type, bool vert);

    void inferAndSet (GfxGslAst *ast_, const Ctx &c);

    public:

    GfxGslTypeSystem (GfxGslAllocator &alloc, GfxGslKind kind, const GfxGslTypeMap &matFields,
                      const GfxGslTypeMap &vars)
      : alloc(alloc), matFields(matFields), vars(vars), outerVars(vars), kind(kind)
    {
        initFuncTypes();
        initObjectTypes();
    }


    void inferAndSet (GfxGslAst *ast_)
    {
        inferAndSet(ast_, Ctx());
    }

    const GfxGslTypeMap &getVars (void) const { return vars; }

    const std::set<std::string> &getVertFieldsRead (void) const { return vertFieldsRead; }
    const std::set<std::string> &getFragFieldsRead (void) const { return fragFieldsRead; }
    const std::set<std::string> &getMatFieldsRead (void) const { return matFieldsRead; }
    const std::set<std::string> &getGlobalFieldsRead (void) const { return globalFieldsRead; }
    const std::set<std::string> &getFragFieldsWritten (void) const { return fragFieldsWritten; }

    const GfxGslType *getVertType (const std::string &f) const
    {
        auto it = vertFields.find(f);
        if (it == vertFields.end()) return nullptr;
        return it->second;
    }

    const GfxGslType *getFragType (const std::string &f) const
    {
        auto it = fragFields.find(f);
        if (it == fragFields.end()) return nullptr;
        return it->second.t;
    }

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

    const GfxGslType *getVarType (const std::string &v) const
    {
        auto it = vars.find(v);
        if (it == vars.end()) return nullptr;
        return it->second;
    }

    const std::vector<Trans> &getTrans (void) const { return trans; }
};

static inline std::ostream &operator<< (std::ostream &o, const GfxGslTypeSystem::Trans &t)
{
    const char *prefix = "";
    if (t.isVert)
        o << "vert.";
    for (unsigned i=0 ; i<t.path.size() ; ++i) {  
        o << prefix << t.path[i];
        prefix = ".";
    }
    return o;
}

#endif
