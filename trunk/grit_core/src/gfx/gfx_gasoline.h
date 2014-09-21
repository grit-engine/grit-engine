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
#include <ostream>
#include <string>

#include "../centralised_log.h"

#ifndef GFX_GASOLINE_H
#define GFX_GASOLINE_H

enum GfxGslParamType {
    GFX_GSL_FLOAT1,
    GFX_GSL_FLOAT2,
    GFX_GSL_FLOAT3,
    GFX_GSL_FLOAT4,
    GFX_GSL_FLOAT_TEXTURE1,
    GFX_GSL_FLOAT_TEXTURE2,
    GFX_GSL_FLOAT_TEXTURE3,
    GFX_GSL_FLOAT_TEXTURE4
};

static inline bool gfx_gasoline_param_is_texture (GfxGslParamType t)
{
    switch (t) {
        case GFX_GSL_FLOAT_TEXTURE1:
        case GFX_GSL_FLOAT_TEXTURE2:
        case GFX_GSL_FLOAT_TEXTURE3:
        case GFX_GSL_FLOAT_TEXTURE4:
        return true;
        case GFX_GSL_FLOAT1:
        case GFX_GSL_FLOAT2:
        case GFX_GSL_FLOAT3:
        case GFX_GSL_FLOAT4:
        return false;
    }
    EXCEPTEX << "Internal error" << ENDL;
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

std::pair<std::string, std::string> gfx_gasoline_compile (GfxGslBackend backend,
                                                          const std::string &vert_prog,
                                                          const std::string &frag_prog,
                                                          const GfxGslParams &params,
                                                          const GfxGslUnboundTextures &ubt);

#endif
