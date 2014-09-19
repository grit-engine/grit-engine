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

void GfxGslCompiler::frontend (void)
{
    GfxGslTypeMap mat_fields;
    for (auto pair : params) {
        switch (pair.second) {
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

    GfxGslTypeMap vars;

    try {
        vertAst = gfx_gasoline_parse(alloc, vertProg);
        vertTypeSystem = new GfxGslTypeSystem(alloc, GFX_GSL_VERT, mat_fields, vars);
        vertTypeSystem->inferAndSet(vertAst);
    } catch (const Exception &e) {
        EXCEPT << "Vertex shader " << e << ENDL;
    }

    try {
        fragAst = gfx_gasoline_parse(alloc, fragProg);
        vars = vertTypeSystem->getVars();
        fragTypeSystem = new GfxGslTypeSystem(alloc, GFX_GSL_FRAG, mat_fields, vars);
        fragTypeSystem->inferAndSet(fragAst);
    } catch (const Exception &e) {
        EXCEPT << "Fragment shader " << e << ENDL;
    }
}
