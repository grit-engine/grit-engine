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

#include "gfx_gasoline.h"
#include "gfx_gasoline_parser.h"
#include "gfx_gasoline_type_system.h"
#include "gfx_gasoline_backend_gsl.h"
#include "gfx_gasoline_backend_cg.h"
#include "gfx_gasoline_backend_glsl.h"

std::pair<std::string, std::string> gfx_gasoline_compile (GfxGslBackend backend,
                                                          const std::string &vert_prog,
                                                          const std::string &frag_prog,
                                                          const GfxGslParams &params,
                                                          const GfxGslUnboundTextures &ubt)
{
    GfxGslAllocator alloc;

    GfxGslTypeMap mat_fields;
    for (auto pair : params) {
        switch (pair.second) {
            case GFX_GSL_INT1: {
                mat_fields[pair.first] = alloc.makeType<GfxGslIntType>(1);
            } break;
            case GFX_GSL_INT2: {
                mat_fields[pair.first] = alloc.makeType<GfxGslIntType>(2);
            } break;
            case GFX_GSL_INT3: {
                mat_fields[pair.first] = alloc.makeType<GfxGslIntType>(3);
            } break;
            case GFX_GSL_INT4: {
                mat_fields[pair.first] = alloc.makeType<GfxGslIntType>(4);
            } break;
            case GFX_GSL_FLOAT1: {
                mat_fields[pair.first] = alloc.makeType<GfxGslFloatType>(1);
            } break;
            case GFX_GSL_FLOAT2: {
                mat_fields[pair.first] = alloc.makeType<GfxGslFloatType>(2);
            } break;
            case GFX_GSL_FLOAT3: {
                mat_fields[pair.first] = alloc.makeType<GfxGslFloatType>(3);
            } break;
            case GFX_GSL_FLOAT4: {
                mat_fields[pair.first] = alloc.makeType<GfxGslFloatType>(4);
            } break;
            case GFX_GSL_FLOAT_TEXTURE1: {
                mat_fields[pair.first] = alloc.makeType<GfxGslFloatTextureType>(1);
            } break;
            case GFX_GSL_FLOAT_TEXTURE2: {
                mat_fields[pair.first] = alloc.makeType<GfxGslFloatTextureType>(2);
            } break;
            case GFX_GSL_FLOAT_TEXTURE3: {
                mat_fields[pair.first] = alloc.makeType<GfxGslFloatTextureType>(3);
            } break;
            case GFX_GSL_FLOAT_TEXTURE4: {
                mat_fields[pair.first] = alloc.makeType<GfxGslFloatTextureType>(4);
            } break;
        }
    }

    GfxGslAst *vert_ast;
    GfxGslTypeSystem vert_ts(alloc, GFX_GSL_VERT, mat_fields, GfxGslTypeMap());

    try {
        vert_ast = gfx_gasoline_parse(alloc, vert_prog);
        vert_ts.inferAndSet(vert_ast);
    } catch (const Exception &e) {
        EXCEPT << "Vertex shader: " << e << ENDL;
    }


    GfxGslAst *frag_ast;
    GfxGslTypeSystem frag_ts(alloc, GFX_GSL_FRAG, mat_fields, vert_ts.getVars());

    try {
        frag_ast = gfx_gasoline_parse(alloc, frag_prog);
        frag_ts.inferAndSet(frag_ast);
    } catch (const Exception &e) {
        EXCEPT << "Fragment shader: " << e << ENDL;
    }

    std::string vert_out;
    std::string frag_out;
    switch (backend) {
        case GFX_GSL_BACKEND_GSL:
        vert_out = gfx_gasoline_unparse_gsl(&vert_ts, vert_ast);
        frag_out = gfx_gasoline_unparse_gsl(&frag_ts, frag_ast);
        break;

        case GFX_GSL_BACKEND_CG:
        gfx_gasoline_unparse_cg(&vert_ts, vert_ast, vert_out,
                                &frag_ts, frag_ast, frag_out, ubt);
        break;

        case GFX_GSL_BACKEND_GLSL:
        gfx_gasoline_unparse_glsl(&vert_ts, vert_ast, vert_out,
                                  &frag_ts, frag_ast, frag_out, ubt);
        break;
    }

    return {vert_out, frag_out};
}
