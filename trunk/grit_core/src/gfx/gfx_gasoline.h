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
#include <ostream>
#include <string>

#include "../centralised_log.h"

#ifndef GFX_GASOLINE_H
#define GFX_GASOLINE_H

enum GfxGslParamType {
    GFX_GSL_FLOAT1 = 0x1,
    GFX_GSL_FLOAT2 = 0x2,
    GFX_GSL_FLOAT3 = 0x3,
    GFX_GSL_FLOAT4 = 0x4,
    GFX_GSL_INT1 = 0x11,
    GFX_GSL_INT2 = 0x12,
    GFX_GSL_INT3 = 0x13,
    GFX_GSL_INT4 = 0x14,
    GFX_GSL_FLOAT_TEXTURE1 = 0x101,
    GFX_GSL_FLOAT_TEXTURE2 = 0x102,
    GFX_GSL_FLOAT_TEXTURE3 = 0x103,
    GFX_GSL_FLOAT_TEXTURE4 = 0x104
};

static inline const char *to_string (GfxGslParamType t)
{
    switch (t) {
        case GFX_GSL_FLOAT1: return "Float";
        case GFX_GSL_FLOAT2: return "Float2";
        case GFX_GSL_FLOAT3: return "Float3";
        case GFX_GSL_FLOAT4: return "Float4";
        case GFX_GSL_INT1: return "Int";
        case GFX_GSL_INT2: return "Int2";
        case GFX_GSL_INT3: return "Int3";
        case GFX_GSL_INT4: return "Int4";
        case GFX_GSL_FLOAT_TEXTURE1: return "FloatTexture1";
        case GFX_GSL_FLOAT_TEXTURE2: return "FloatTexture2";
        case GFX_GSL_FLOAT_TEXTURE3: return "FloatTexture3";
        case GFX_GSL_FLOAT_TEXTURE4: return "FloatTexture4";
        default: return "Unknown";
    }
}
static inline std::ostream &operator<< (std::ostream &o, GfxGslParamType t)
{
    o << to_string(t);
    return o;
}

static inline bool gfx_gasoline_param_is_texture (GfxGslParamType t)
{
    return t & 0x100;
}

enum GfxGslBackend {
    GFX_GSL_BACKEND_GSL,
    GFX_GSL_BACKEND_CG,
    GFX_GSL_BACKEND_GLSL
};

struct GfxGslColour {
    float r, g, b, a;
    GfxGslColour (void) { }
    GfxGslColour (float r, float g, float b, float a)
      : r(r), g(g), b(b), a(a)
    { }
};

static inline std::ostream &operator<< (std::ostream &o, const GfxGslColour &c)
{
    o << "Float4(" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ")";
    return o;
}

typedef std::map<std::string, GfxGslParamType> GfxGslParams;
typedef std::map<std::string, GfxGslColour> GfxGslUnboundTextures;

/** Return the shader code that defines things needsd by the generated code. */
std::string gfx_gasoline_preamble (GfxGslBackend backend);

struct GfxGasolineResult {
    std::string vertexShader;
    std::string fragmentShader;
};

GfxGasolineResult gfx_gasoline_compile_colour (GfxGslBackend backend,
                                               const std::string &vert_prog,
                                               const std::string &colour_prog,
                                               const GfxGslParams &params,
                                               const GfxGslUnboundTextures &ubt);

GfxGasolineResult gfx_gasoline_compile_first_person (GfxGslBackend backend,
                                                     const std::string &vert_prog,
                                                     const std::string &dangs_prog,
                                                     const std::string &additional_prog,
                                                     const GfxGslParams &params,
                                                     const GfxGslUnboundTextures &ubt);

// To come!
GfxGasolineResult gfx_gasoline_compile_forward (GfxGslBackend backend,
                                                const std::string &vert_prog,
                                                const std::string &fwd_prog,
                                                const GfxGslParams &params,
                                                const GfxGslUnboundTextures &ubt);

GfxGasolineResult gfx_gasoline_compile_shadow_cast (GfxGslBackend backend,
                                                    const std::string &vert_prog,
                                                    const std::string &fwd_prog,
                                                    const GfxGslParams &params,
                                                    const GfxGslUnboundTextures &ubt);

#endif
