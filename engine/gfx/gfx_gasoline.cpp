/* Copyright (c) The Grit Game Engine authors 2016
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


static const GfxGslFloatVec GSL_BLACK(0, 0, 0, 0);

std::ostream &operator<< (std::ostream &o, const GfxGslParam &p)
{
    switch (p.t) {
        case GFX_GSL_STATIC_INT1:
        case GFX_GSL_INT1:
        o << "Int(" << p.is.r << ")";
        break;
        case GFX_GSL_STATIC_INT2:
        case GFX_GSL_INT2:
        o << "Int2(" << p.is.r << ", " << p.is.g << ")";
        break;
        case GFX_GSL_STATIC_INT3:
        case GFX_GSL_INT3:
        o << "Int3(" << p.is.r << ", " << p.is.g << ", " << p.is.b << ")";
        break;
        case GFX_GSL_STATIC_INT4:
        case GFX_GSL_INT4:
        o << "Int4(" << p.is.r << ", " << p.is.g << ", " << p.is.b << ", " << p.is.a << ")";
        break;
        case GFX_GSL_STATIC_FLOAT1:
        case GFX_GSL_FLOAT1:
        o << "Float(" << p.fs.r << ")";
        break;
        case GFX_GSL_STATIC_FLOAT2:
        case GFX_GSL_FLOAT2:
        o << "Float2(" << p.fs.r << ", " << p.fs.g << ")";
        break;
        case GFX_GSL_STATIC_FLOAT3:
        case GFX_GSL_FLOAT3:
        o << "Float3(" << p.fs.r << ", " << p.fs.g << ", " << p.fs.b << ")";
        break;
        case GFX_GSL_STATIC_FLOAT4:
        case GFX_GSL_FLOAT4:
        case GFX_GSL_FLOAT_TEXTURE1:
        case GFX_GSL_FLOAT_TEXTURE2:
        case GFX_GSL_FLOAT_TEXTURE3:
        case GFX_GSL_FLOAT_TEXTURE4:
        case GFX_GSL_FLOAT_TEXTURE_CUBE:
        o << "Float4(" << p.fs.r << ", " << p.fs.g << ", " << p.fs.b << ", " << p.fs.a << ")";
        break;
        default:
        EXCEPTEX << "Not a first class parameter value." << ENDL;
    }
    if (gfx_gasoline_param_is_static(p)) {
        o << "  /* static */";
    } else if (gfx_gasoline_param_is_texture(p)) {
        o << "  /* Texture */";
    }
    return o;
}

bool operator== (const GfxGslParam &a, const GfxGslParam &b)
{
    if (a.t != b.t) return false;
    if (gfx_gasoline_param_is_int(a)) {
        if (a.is != b.is) return false;
    } else {
        if (a.fs != b.fs) return false;
    }
    return true;
}

static GfxGslFieldTypeMap make_body_fields (GfxGslAllocator &alloc)
{
    GfxGslFieldTypeMap m;

    m["world"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    m["worldView"] = GfxGslFieldType(alloc.makeType<GfxGslFloatMatrixType>(4,4), true, false);
    m["worldViewProj"] = GfxGslFieldType(alloc.makeType<GfxGslFloatMatrixType>(4,4), true, false);

    m["paintDiffuse0"] = alloc.makeType<GfxGslFloatType>(3);
    m["paintDiffuse1"] = alloc.makeType<GfxGslFloatType>(3);
    m["paintDiffuse2"] = alloc.makeType<GfxGslFloatType>(3);
    m["paintDiffuse3"] = alloc.makeType<GfxGslFloatType>(3);
    m["paintMetallic0"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintMetallic1"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintMetallic2"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintMetallic3"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintSpecular0"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintSpecular1"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintSpecular2"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintSpecular3"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintGloss0"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintGloss1"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintGloss2"] = alloc.makeType<GfxGslFloatType>(1);
    m["paintGloss3"] = alloc.makeType<GfxGslFloatType>(1);

    // Direct access to bone matrixes not allowed -- use transform_to_world.

    return m;
}
 
static GfxGslFieldTypeMap make_global_fields (GfxGslAllocator &alloc)
{
    GfxGslFieldTypeMap m;

    m["cameraPos"] = alloc.makeType<GfxGslFloatType>(3);
    m["fovY"] = alloc.makeType<GfxGslFloatType>(1);
    m["proj"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    m["time"] = alloc.makeType<GfxGslFloatType>(1);
    m["view"] = GfxGslFieldType(alloc.makeType<GfxGslFloatMatrixType>(4,4), true, false);
    m["invView"] = GfxGslFieldType(alloc.makeType<GfxGslFloatMatrixType>(4,4), true, false);
    m["viewportSize"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(2), false, false);
    m["viewProj"] = GfxGslFieldType(alloc.makeType<GfxGslFloatMatrixType>(4,4), false, false);
    m["rayTopLeft"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(3), true, false);
    m["rayTopRight"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(3), true, false);
    m["rayBottomLeft"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(3), true, false);
    m["rayBottomRight"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(3), true, false);
    m["nearClipDistance"] = alloc.makeType<GfxGslFloatType>(1);
    m["farClipDistance"] = alloc.makeType<GfxGslFloatType>(1);

    m["particleAmbient"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunlightDiffuse"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunlightDirection"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunlightSpecular"] = alloc.makeType<GfxGslFloatType>(3);

    m["fogColour"] = alloc.makeType<GfxGslFloatType>(3);
    m["fogDensity"] = alloc.makeType<GfxGslFloatType>(1);
    m["hellColour"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyCloudColour"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyCloudCoverage"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyGlareHorizonElevation"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyGlareSunDistance"] = alloc.makeType<GfxGslFloatType>(1);
    m["sunAlpha"] = alloc.makeType<GfxGslFloatType>(1);
    m["sunColour"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunDirection"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunFalloffDistance"] = alloc.makeType<GfxGslFloatType>(1);
    m["sunSize"] = alloc.makeType<GfxGslFloatType>(1);

    m["skyDivider1"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyDivider2"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyDivider3"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyDivider4"] = alloc.makeType<GfxGslFloatType>(1);

    m["skyColour0"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour1"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour2"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour3"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour4"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour5"] = alloc.makeType<GfxGslFloatType>(3);

    m["skySunColour0"] = alloc.makeType<GfxGslFloatType>(3);
    m["skySunColour1"] = alloc.makeType<GfxGslFloatType>(3);
    m["skySunColour2"] = alloc.makeType<GfxGslFloatType>(3);
    m["skySunColour3"] = alloc.makeType<GfxGslFloatType>(3);
    m["skySunColour4"] = alloc.makeType<GfxGslFloatType>(3);

    m["envCubeCrossFade"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(1), true, true);
    m["envCubeMipmaps0"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(1), true, true);
    m["envCubeMipmaps1"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(1), true, true);
    m["saturation"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(1), true, false);
    m["exposure"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(1), true, false);
    m["bloomThreshold"] = GfxGslFieldType(alloc.makeType<GfxGslFloatType>(1), true, false);
    m["shadowViewProj0"] = GfxGslFieldType(alloc.makeType<GfxGslFloatMatrixType>(4,4), true, true);
    m["shadowViewProj1"] = GfxGslFieldType(alloc.makeType<GfxGslFloatMatrixType>(4,4), true, true);
    m["shadowViewProj2"] = GfxGslFieldType(alloc.makeType<GfxGslFloatMatrixType>(4,4), true, true);

    m["envCube0"] =
        GfxGslFieldType(alloc.makeType<GfxGslFloatTextureCubeType>(GSL_BLACK), true, true);
    m["envCube1"] =
        GfxGslFieldType(alloc.makeType<GfxGslFloatTextureCubeType>(GSL_BLACK), true, true);
    m["fadeDitherMap"] =
        alloc.makeType<GfxGslFloatTextureType>(GSL_BLACK, 2);
    m["gbuffer0"] =
        GfxGslFieldType(alloc.makeType<GfxGslFloatTextureType>(GSL_BLACK, 2), true, false);
    m["shadowPcfNoiseMap"] =
        GfxGslFieldType(alloc.makeType<GfxGslFloatTextureType>(GSL_BLACK, 2), true, true);
    m["shadowMap0"] =
        GfxGslFieldType(alloc.makeType<GfxGslFloatTextureType>(GSL_BLACK, 2), true, true);
    m["shadowMap1"] =
        GfxGslFieldType(alloc.makeType<GfxGslFloatTextureType>(GSL_BLACK, 2), true, true);
    m["shadowMap2"] =
        GfxGslFieldType(alloc.makeType<GfxGslFloatTextureType>(GSL_BLACK, 2), true, true);

    return m;
}
 
GfxGslGlobalFuncTypeMap make_func_types (GfxGslAllocator &alloc)
{
    GfxGslGlobalFuncTypeMap m;

    auto is = [&] (int i) -> GfxGslIntType* { return alloc.makeType<GfxGslIntType>(i); };
    auto fs = [&] (int i) -> GfxGslFloatType* { return alloc.makeType<GfxGslFloatType>(i); };
    auto tex = [&] (int i) -> GfxGslFloatTextureType* {
        return alloc.makeType<GfxGslFloatTextureType>(GSL_BLACK, i);
    };

    auto *cube = alloc.makeType<GfxGslFloatTextureCubeType>(GSL_BLACK);

    //GfxGslType *b = alloc.makeType<GfxGslBoolType>();
    // ts holds the set of all functions: float_n -> float_n
    std::vector<const GfxGslFunctionType*> ts;
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i)}, fs(i)));
    m["tan"] = ts;
    m["atan"] = ts;
    m["sin"] = ts;
    m["asin"] = ts;
    m["cos"] = ts;
    m["acos"] = ts;
    m["ddx"] = ts;  // Only available in fragment shader
    m["ddy"] = ts;  // Only available in fragment shader
    m["sqrt"] = ts;
    m["fract"] = ts;
    m["floor"] = ts;
    m["ceil"] = ts;

    m["abs"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, is(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(2)}, is(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(3)}, is(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(4)}, is(4)),
    };

    m["pow"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(1)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4), fs(1)}, fs(4)),
    };

    m["strength"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(1)),
    };

    m["atan2"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(1))
    };

    m["dot"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(3)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(2)}, fs(1)),
    };
    m["normalise"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3))
    };
    m["reflect"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(3)}, fs(3))
    };
    m["cross"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(3)}, fs(3))
    };

    m["desaturate"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(1)}, fs(3))
    };

    // TODO: only available in vertex shader -- needs a lot of varyings for instanced geometry.
    m["rotate_to_world"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3))
    };

    // TODO: only available in vertex shader -- needs a lot of varyings for instanced geometry.
    m["transform_to_world"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3))
    };

    // ts holds the set of all functions: (float_n) -> float_1
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i)}, fs(1)));
    m["length"] = ts;

    // ts holds the set of all functions: (float_n, float_n, float_n) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(i), fs(i)}, fs(i)));
    m["clamp"] = ts;

    m["Float"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
    };
    m["Float2"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(2))
    };
    m["Float3"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(2)}, fs(3)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1), fs(1)}, fs(3)),
    };

    m["Float4"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(4)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(3)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(2)}, fs(4)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1), fs(2)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(2), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(1), fs(1)}, fs(4)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1), fs(1), fs(1)}, fs(4))
    };

    // ts holds the set of all functions: (float_n, float_n, float1) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(i), fs(i)}, fs(i)));
    m["lerp"] = ts;


    // ts holds the set of all functions: (float_n, float_n) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(i)}, fs(i)));
    m["max"] = ts;
    m["min"] = ts;

    // ts holds the set of all functions: (float_n, float_1) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(1)}, fs(i)));
    m["mod"] = ts;

    ts.clear();
    for (unsigned w=2 ; w<=4 ; ++w) {
        for (unsigned h=2 ; h<=4 ; ++h) {
            GfxGslType *m = alloc.makeType<GfxGslFloatMatrixType>(w, h);
            ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{m, fs(w)}, fs(h)));
        }
    }
    m["mul"] = ts;

    m["sample"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(2), fs(2)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(3), fs(3)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{cube, fs(3)}, fs(4)),
    };
    m["sampleGrad"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(2), fs(2), fs(2), fs(2)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(3), fs(3), fs(3), fs(3)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{cube, fs(3), fs(3), fs(3)}, fs(4)),
    };

    m["sampleLod"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(2), fs(2), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(3), fs(3), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{cube, fs(3), fs(1)}, fs(4)),
    };

    m["pma_decode"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
    };

    m["gamma_decode"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
    };

    m["gamma_encode"] = GfxGslFunctionTypes {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
    };

    m["sunlight"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(
                GfxGslTypes{fs(3), fs(3), fs(3), fs(3), fs(1), fs(1), fs(1)}, fs(3)),
        }, true, true);
        

    m["envlight"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(
                GfxGslTypes{fs(3), fs(3), fs(3), fs(1), fs(1)}, fs(3)),
        }, true, true);

    m["punctual_lighting"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(
                GfxGslTypes{fs(3), fs(3), fs(3), fs(3), fs(1), fs(1), fs(3), fs(3)}, fs(3)),
        }, true, false);

    m["unpack_deferred_diffuse_colour"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4), fs(4), fs(4)}, fs(3))
        }, true, false);
    m["unpack_deferred_specular"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4), fs(4), fs(4)}, fs(1))
        }, true, false);
    m["unpack_deferred_shadow_cutoff"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4), fs(4), fs(4)}, fs(1))
        }, true, false);
    m["unpack_deferred_gloss"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4), fs(4), fs(4)}, fs(1))
        }, true, false);
    m["unpack_deferred_cam_dist"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4), fs(4), fs(4)}, fs(1))
        }, true, false);
    m["unpack_deferred_normal"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4), fs(4), fs(4)}, fs(3))
        }, true, false);

    m["fog_weakness"] = GfxGslGlobalFuncType(
        GfxGslFunctionTypes {
            alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1))
        }, true, false);

    return m;
}



static GfxGslType *to_type (GfxGslAllocator &alloc, const GfxGslParam &p)
{
    switch (p.t) {
        case GFX_GSL_INT1: case GFX_GSL_STATIC_INT1: return alloc.makeType<GfxGslIntType>(1);
        case GFX_GSL_INT2: case GFX_GSL_STATIC_INT2: return alloc.makeType<GfxGslIntType>(2);
        case GFX_GSL_INT3: case GFX_GSL_STATIC_INT3: return alloc.makeType<GfxGslIntType>(3);
        case GFX_GSL_INT4: case GFX_GSL_STATIC_INT4: return alloc.makeType<GfxGslIntType>(4);
        case GFX_GSL_FLOAT1: case GFX_GSL_STATIC_FLOAT1: return alloc.makeType<GfxGslFloatType>(1);
        case GFX_GSL_FLOAT2: case GFX_GSL_STATIC_FLOAT2: return alloc.makeType<GfxGslFloatType>(2);
        case GFX_GSL_FLOAT3: case GFX_GSL_STATIC_FLOAT3: return alloc.makeType<GfxGslFloatType>(3);
        case GFX_GSL_FLOAT4: case GFX_GSL_STATIC_FLOAT4: return alloc.makeType<GfxGslFloatType>(4);
        case GFX_GSL_FLOAT_TEXTURE1: return alloc.makeType<GfxGslFloatTextureType>(p.fs, 1);
        case GFX_GSL_FLOAT_TEXTURE2: return alloc.makeType<GfxGslFloatTextureType>(p.fs, 2);
        case GFX_GSL_FLOAT_TEXTURE3: return alloc.makeType<GfxGslFloatTextureType>(p.fs, 3);
        case GFX_GSL_FLOAT_TEXTURE4: return alloc.makeType<GfxGslFloatTextureType>(p.fs, 4);
        case GFX_GSL_FLOAT_TEXTURE_CUBE: return alloc.makeType<GfxGslFloatTextureCubeType>(p.fs);
    }
    EXCEPTEX << ENDL;
}

static GfxGslFieldTypeMap make_mat_fields (GfxGslAllocator &alloc, const GfxGslRunParams &params)
{
    GfxGslFieldTypeMap r;
    for (auto pair : params)
        r[pair.first] = to_type(alloc, pair.second);
    return r;
}

typedef GfxGasolineResult Continuation (const GfxGslContext &ctx,
                                        const GfxGslTypeSystem &vert_ts,
                                        const GfxGslAst *vert_ast,
                                        const GfxGslTypeSystem &dangs_ts,
                                        const GfxGslAst *dangs_ast,
                                        const GfxGslTypeSystem &additional_ts,
                                        const GfxGslAst *additional_ast,
                                        const GfxGslMetadata &md);

// Intended to be called with lambdas that finish off compilation.
template<class Continuation>
static GfxGasolineResult type_check (const std::string &vert_prog,
                                     const std::string &dangs_prog,
                                     const std::string &additional_prog,
                                     const GfxGslMetadata &md,
                                     Continuation cont)
{
    GfxGslAllocator alloc;
    GfxGslContext ctx = {
        alloc, make_func_types(alloc), make_global_fields(alloc),
        make_mat_fields(alloc, md.params), make_body_fields(alloc), md.matEnv.ubt,
        md.matEnv.staticValues, md.d3d9, md.internal, md.lightingTextures
    };

    GfxGslShader *vert_ast;
    GfxGslTypeSystem vert_ts(ctx, GFX_GSL_VERTEX);

    try {
        vert_ast = gfx_gasoline_parse(alloc, vert_prog);
        vert_ts.inferAndSet(vert_ast, GfxGslDefMap { });
    } catch (const Exception &e) {
        EXCEPT << "Vertex shader: " << e << ENDL;
    }


    GfxGslShader *dangs_ast;
    GfxGslTypeSystem dangs_ts(ctx, GFX_GSL_DANGS);

    try {
        dangs_ast = gfx_gasoline_parse(alloc, dangs_prog);
        dangs_ts.inferAndSet(dangs_ast, vert_ast->vars);
    } catch (const Exception &e) {
        EXCEPT << "DANGS shader: " << e << ENDL;
    }

    GfxGslShader *additional_ast;
    GfxGslTypeSystem additional_ts(ctx, GFX_GSL_COLOUR_ALPHA);

    try {
        additional_ast = gfx_gasoline_parse(alloc, additional_prog);
        additional_ts.inferAndSet(additional_ast, vert_ast->vars);
    } catch (const Exception &e) {
        EXCEPT << "Additional shader: " << e << ENDL;
    }

    return cont(ctx, vert_ts, vert_ast, dangs_ts, dangs_ast, additional_ts, additional_ast, md);
}

void gfx_gasoline_check (const std::string &vert_prog,
                         const std::string &dangs_prog,
                         const std::string &additional_prog,
                         const GfxGslMetadata &md)
{
    auto cont = [] (const GfxGslContext &,
                    const GfxGslTypeSystem &,
                    const GfxGslAst *,
                    const GfxGslTypeSystem &,
                    const GfxGslAst *,
                    const GfxGslTypeSystem &,
                    const GfxGslAst *,
                    const GfxGslMetadata &)
    -> GfxGasolineResult
    {
        return GfxGasolineResult();
    };

    type_check(vert_prog, dangs_prog, additional_prog, md, cont);
}

static GfxGasolineResult gfx_gasoline_compile_colour (const GfxGslBackend backend,
                                                      const std::string &vert_prog,
                                                      const std::string &colour_prog,
                                                      const GfxGslMetadata &md,
                                                      const bool flat_z, const bool das)
{
    auto cont = [backend, flat_z, das] (
        const GfxGslContext &ctx,
        const GfxGslTypeSystem &vert_ts,
        const GfxGslAst *vert_ast,
        const GfxGslTypeSystem &dangs_ts,
        const GfxGslAst *dangs_ast,
        const GfxGslTypeSystem &additional_ts,
        const GfxGslAst *additional_ast,
        const GfxGslMetadata &md)
    -> GfxGasolineResult
    {
        (void) dangs_ts; (void) dangs_ast;
        std::string vert_out;
        std::string frag_out;
        switch (backend) {
            case GFX_GSL_BACKEND_CG:
            gfx_gasoline_unparse_cg(
                ctx, &vert_ts, vert_ast, vert_out, &additional_ts, additional_ast, frag_out,
                md.cfgEnv, md.matEnv, md.meshEnv, flat_z, das);
            break;

            case GFX_GSL_BACKEND_GLSL33:
            gfx_gasoline_unparse_glsl(
                ctx, &vert_ts, vert_ast, vert_out, &additional_ts, additional_ast, frag_out,
                md.cfgEnv, md.matEnv, md.meshEnv, true, flat_z, das);
            break;
        }
        return {vert_out, frag_out};
    };

    return type_check(vert_prog, "", colour_prog, md, cont);
}

static GfxGasolineResult gfx_gasoline_compile_body (const GfxGslBackend backend,
                                                    const std::string &vert_prog,
                                                    const std::string &dangs_prog,
                                                    const std::string &additional_prog,
                                                    const GfxGslMetadata &md,
                                                    bool first_person,
                                                    bool wireframe,
                                                    bool forward_only,
                                                    bool cast)
{
    auto cont = [backend, first_person, wireframe, forward_only, cast] (
        const GfxGslContext &ctx,
        const GfxGslTypeSystem &vert_ts,
        const GfxGslAst *vert_ast,
        const GfxGslTypeSystem &dangs_ts,
        const GfxGslAst *dangs_ast,
        const GfxGslTypeSystem &additional_ts,
        const GfxGslAst *additional_ast,
        const GfxGslMetadata &md)
    -> GfxGasolineResult
    {
        std::string vert_out;
        std::string frag_out;
        switch (backend) {
            case GFX_GSL_BACKEND_CG:
            gfx_gasoline_unparse_body_cg(
                ctx, &vert_ts, vert_ast, &dangs_ts, dangs_ast, &additional_ts, additional_ast,
                vert_out, frag_out, md.cfgEnv, md.matEnv, md.meshEnv, first_person, wireframe,
                forward_only, cast);
            break;

            case GFX_GSL_BACKEND_GLSL33:
            gfx_gasoline_unparse_body_glsl(
                ctx, &vert_ts, vert_ast, &dangs_ts, dangs_ast, &additional_ts, additional_ast,
                vert_out, frag_out, md.cfgEnv, md.matEnv, md.meshEnv, true, first_person, wireframe,
                forward_only, cast);
            break;
        }
        return {vert_out, frag_out};
    };

    return type_check(vert_prog, dangs_prog, additional_prog, md, cont);
}

GfxGasolineResult gfx_gasoline_compile (GfxGslPurpose purpose,
                                        GfxGslBackend backend,
                                        const std::string &vert_prog,
                                        const std::string &dangs_prog,
                                        const std::string &additional_prog,
                                        const GfxGslMetadata &md)
{
    switch (purpose) {
        case GFX_GSL_PURPOSE_FORWARD:
        return gfx_gasoline_compile_body(backend, vert_prog, dangs_prog, additional_prog, md, false, false, true, false);

        case GFX_GSL_PURPOSE_ALPHA:
        return gfx_gasoline_compile_body(backend, vert_prog, dangs_prog, additional_prog, md, false, false, false, false);

        case GFX_GSL_PURPOSE_FIRST_PERSON:
        return gfx_gasoline_compile_body(backend, vert_prog, dangs_prog, additional_prog, md, true, true, false, false);

        case GFX_GSL_PURPOSE_FIRST_PERSON_WIREFRAME: {
            std::string colour_prog = "out.colour = Float3(1, 1, 1);\n";
            return gfx_gasoline_compile_body(backend, vert_prog, "", colour_prog, md, true, true, false, false);
        }

        case GFX_GSL_PURPOSE_ADDITIONAL:
        return gfx_gasoline_compile_colour(backend, vert_prog, additional_prog, md, false, false);

        case GFX_GSL_PURPOSE_WIREFRAME: {
            std::string white_prog = "out.colour = Float3(1, 1, 1);\n";
            return gfx_gasoline_compile_colour(backend, vert_prog, white_prog, md, false, false);
        }

        case GFX_GSL_PURPOSE_SKY:
        return gfx_gasoline_compile_colour(backend, vert_prog, additional_prog, md, true, false);

        case GFX_GSL_PURPOSE_HUD:
        return gfx_gasoline_compile_colour(backend, vert_prog, additional_prog, md, false, false);

        case GFX_GSL_PURPOSE_DECAL: {
            auto cont = [backend] (const GfxGslContext &ctx,
                                   const GfxGslTypeSystem &vert_ts,
                                   const GfxGslAst *vert_ast,
                                   const GfxGslTypeSystem &dangs_ts,
                                   const GfxGslAst *dangs_ast,
                                   const GfxGslTypeSystem &additional_ts,
                                   const GfxGslAst *additional_ast,
                                   const GfxGslMetadata &md)
            -> GfxGasolineResult
            {
                // TODO: Ensure the dangs & additional shaders do not use vertex fields.
                (void) vert_ts;
                (void) vert_ast;
                std::string vert_out;
                std::string frag_out;
                switch (backend) {
                    case GFX_GSL_BACKEND_CG:
                    gfx_gasoline_unparse_decal_cg(ctx, &dangs_ts, dangs_ast,
                                                  &additional_ts, additional_ast,
                                                  vert_out, frag_out,
                                                  md.cfgEnv, md.matEnv, md.meshEnv);
                    break;

                    case GFX_GSL_BACKEND_GLSL33:
                    gfx_gasoline_unparse_decal_glsl(ctx, &dangs_ts, dangs_ast,
                                                    &additional_ts, additional_ast,
                                                    vert_out, frag_out,
                                                    md.cfgEnv, md.matEnv, md.meshEnv, true);
                    break;
                }
                return {vert_out, frag_out};
            };

            return type_check("", dangs_prog, additional_prog, md, cont);
        }

        case GFX_GSL_PURPOSE_DEFERRED_AMBIENT_SUN:
        return gfx_gasoline_compile_colour(backend, vert_prog, additional_prog, md, false, true);

        case GFX_GSL_PURPOSE_CAST:
        return gfx_gasoline_compile_body(backend, vert_prog, dangs_prog, additional_prog, md, false, false, false, true);

    }

    EXCEPTEX << "Unreachable" << ENDL;  // g++ bug requires this.
}
