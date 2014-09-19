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
#include <ostream>
#include <string>

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

typedef std::map<std::string, GfxGslShaderParamType> GfxGslShaderParams;
typedef std::map<std::string, GfxGslColour> GfxGslUnboundTextures;

std::pair<std::string, std::string> gfx_gasoline_compile (const std::string &lang,
                                                          const std::string &vert_prog,
                                                          const std::string &frag_prog,
                                                          const GfxGslShaderParams &params,
                                                          const GfxGslUnboundTextures &texs);

#endif
