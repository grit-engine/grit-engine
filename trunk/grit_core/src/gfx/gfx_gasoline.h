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

#include "gfx_gasoline_parser.h"
#include "gfx_gasoline_type_system.h"
#include "gfx_gasoline_backend_gsl.h"
#include "gfx_gasoline_backend_cg.h"

#ifndef GFX_GASOLINE
#define GFX_GASOLINE

enum GfxGslShaderParamType {
    GFX_GSL_FLOAT1,
    GFX_GSL_FLOAT2,
    GFX_GSL_FLOAT3,
    GFX_GSL_FLOAT4,
    GFX_GSL_FLOAT_TEXTURE1,
    GFX_GSL_FLOAT_TEXTURE2,
    GFX_GSL_FLOAT_TEXTURE3,
    GFX_GSL_FLOAT_TEXTURE4
};

typedef std::map<std::string, GfxGslShaderParamType> GfxGslShaderParams;

class GfxGslCompiler {

    protected:

    const std::string vertProg;
    const std::string fragProg;
    const GfxGslShaderParams params;
    GfxGslAst *vertAst;
    GfxGslAst *fragAst;
    GfxGslAllocator alloc;
    GfxGslTypeSystem *vertTypeSystem;
    GfxGslTypeSystem *fragTypeSystem;
    std::string vertOutput;
    std::string fragOutput;

    public:

    GfxGslCompiler (const std::string &vert_prog, const std::string &frag_prog,
                    const GfxGslShaderParams &params)
      : vertProg(vert_prog), fragProg(frag_prog), params(params),
        vertAst(nullptr), fragAst(nullptr), vertTypeSystem(nullptr), fragTypeSystem(nullptr)
    { }

    virtual ~GfxGslCompiler (void)
    {
        delete vertTypeSystem;
        delete fragTypeSystem;
    }

    void frontend (void);

    virtual void compile (void) = 0;

    std::string getVertOutput (void) const { return vertOutput; }
    std::string getFragOutput (void) const { return fragOutput; }
        
};

class GfxGslCompilerCg : public GfxGslCompiler {
    public:
    GfxGslCompilerCg (const std::string &vert_prog, const std::string &frag_prog,
                      const GfxGslShaderParams &params)
      : GfxGslCompiler(vert_prog, frag_prog, params)
    { }

    void compile (void)
    {
        frontend();
        gfx_gasoline_unparse_cg(vertTypeSystem, vertAst, vertOutput,
                                fragTypeSystem, fragAst, fragOutput);
    }
};

class GfxGslCompilerGsl : public GfxGslCompiler {
    public:
    GfxGslCompilerGsl (const std::string &vert_prog, const std::string &frag_prog,
                       const GfxGslShaderParams &params)
      : GfxGslCompiler(vert_prog, frag_prog, params)
    { }

    void compile (void)
    {
        frontend();
        vertOutput = gfx_gasoline_unparse_gsl(vertTypeSystem, vertAst);
        fragOutput = gfx_gasoline_unparse_gsl(fragTypeSystem, fragAst);
    }
};

#endif
