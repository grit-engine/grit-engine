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

#include <array>
#include <map>
#include <ostream>
#include <string>

#include <math_util.h>

#include "../centralised_log.h"

#ifndef GFX_GASOLINE_H
#define GFX_GASOLINE_H

template<class T> static inline size_t my_hash (const T &x) { return std::hash<T>()(x); }

enum GfxGslBackend {
    GFX_GSL_BACKEND_CG,
    GFX_GSL_BACKEND_GLSL
};

struct GfxGslFloatVec {
    float r, g, b, a;
    GfxGslFloatVec (void) { }
    GfxGslFloatVec (float r, float g=0, float b=0, float a=0)
      : r(r), g(g), b(b), a(a)
    { }
    bool operator== (const GfxGslFloatVec &other) const
    {
        return other.r == r && other.g == g && other.b == b && other.a == a;
    }
};

static inline std::ostream &operator<< (std::ostream &o, const GfxGslFloatVec &c)
{
    o << "Float4(" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ")";
    return o;
}

namespace std {
    template<> struct hash<GfxGslFloatVec> {
        size_t operator()(const GfxGslFloatVec &a) const
        {
            size_t r = 0;
            r = r * 31 + my_hash(a.r);
            r = r * 31 + my_hash(a.g);
            r = r * 31 + my_hash(a.b);
            r = r * 31 + my_hash(a.a);
            return r;
        }
    };
}

struct GfxGslIntVec {
    int32_t r, g, b, a;
    GfxGslIntVec (void) { }
    GfxGslIntVec (int32_t r, int32_t g=0, int32_t b=0, int32_t a=0)
      : r(r), g(g), b(b), a(a)
    { }
    bool operator== (const GfxGslIntVec &other) const
    {
        return other.r == r && other.g == g && other.b == b && other.a == a;
    }
};

static inline std::ostream &operator<< (std::ostream &o, const GfxGslIntVec &c)
{
    o << "Int4(" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ")";
    return o;
}

namespace std {
    template<> struct hash<GfxGslIntVec> {
        size_t operator()(const GfxGslIntVec &a) const
        {
            size_t r = 0;
            r = r * 31 + my_hash(a.r);
            r = r * 31 + my_hash(a.g);
            r = r * 31 + my_hash(a.b);
            r = r * 31 + my_hash(a.a);
            return r;
        }
    };
}

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
    GFX_GSL_FLOAT_TEXTURE4 = 0x104,
    GFX_GSL_FLOAT_TEXTURE_CUBE = 0x105
};

/** Tagged union of the various values that can go into a shader param. */
struct GfxGslParam {
    GfxGslParamType t; 

    // Default value, which is important for shader compilation only if t is a texture typ.
    GfxGslFloatVec fs;
    GfxGslIntVec is;

    private:
    GfxGslParam (GfxGslParamType t, const GfxGslFloatVec &fs)
      : t(t), fs(fs), is(GfxGslIntVec(0))
    { }

    GfxGslParam (GfxGslParamType t, const GfxGslIntVec &is)
      : t(t), fs(GfxGslFloatVec(0)), is(is)
    { }

    public:

    GfxGslParam (void) { }
    // This form for textures.
    GfxGslParam (GfxGslParamType t, float r, float g, float b, float a)
      : t(t), fs(r, g, b, a), is(0)
    { }

    // These for regular params.
    static GfxGslParam int1 (int r)
    { return GfxGslParam(GFX_GSL_INT1, GfxGslIntVec(r)); }
    static GfxGslParam int2 (int r, int g)
    { return GfxGslParam(GFX_GSL_INT2, GfxGslIntVec(r, g)); }
    static GfxGslParam int3 (int r, int g, int b)
    { return GfxGslParam(GFX_GSL_INT3, GfxGslIntVec(r, g, b)); }
    static GfxGslParam int4 (int r, int g, int b, int a)
    { return GfxGslParam(GFX_GSL_INT4, GfxGslIntVec(r, g, b, a)); }
    static GfxGslParam float1 (float r)
    { return GfxGslParam(GFX_GSL_FLOAT1, GfxGslFloatVec(r)); }
    static GfxGslParam float2 (float r, float g)
    { return GfxGslParam(GFX_GSL_FLOAT2, GfxGslFloatVec(r, g)); }
    static GfxGslParam float3 (float r, float g, float b)
    { return GfxGslParam(GFX_GSL_FLOAT3, GfxGslFloatVec(r, g, b)); }
    static GfxGslParam float4 (float r, float g, float b, float a)
    { return GfxGslParam(GFX_GSL_FLOAT4, GfxGslFloatVec(r, g, b, a)); }
    GfxGslParam (const Vector2 &v) : t(GFX_GSL_FLOAT2), fs(v.x, v.y, 0, 0), is(0) { }
    GfxGslParam (const Vector3 &v) : t(GFX_GSL_FLOAT3), fs(v.x, v.y, v.z, 0), is(0) { }
    GfxGslParam (const Vector4 &v) : t(GFX_GSL_FLOAT4), fs(v.x, v.y, v.z, v.w), is(0) { }
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
        case GFX_GSL_FLOAT_TEXTURE_CUBE: return "FloatTextureCube";
        default: return "Unknown";
    }
}
static inline std::ostream &operator<< (std::ostream &o, GfxGslParamType t)
{
    o << to_string(t);
    return o;
}

/*
static inline const char *to_string (const GfxGslParam &p)
{
    return to_string(p.t);
}
static inline std::ostream &operator<< (std::ostream &o, const GfxGslParam &p)
{
    o << to_string(p);
    return o;
}
*/

static inline bool gfx_gasoline_param_is_texture (const GfxGslParam &p)
{
    return p.t & 0x100;
}

typedef std::map<std::string, GfxGslParam> GfxGslRunParams;
typedef std::set<std::string> GfxGslUnboundTextures;

/** Return the shader code that defines things needsd by the generated code. */
std::string gfx_gasoline_preamble (GfxGslBackend backend);

struct GfxGasolineResult {
    std::string vertexShader;
    std::string fragmentShader;
};

// The stdlib is missing these for some reason.
namespace std {
    template<typename T, size_t N> struct hash<array<T, N> > {
        typedef array<T, N> argument_type;
        typedef size_t result_type;
        result_type operator()(const argument_type& a) const
        {
            result_type h = 0;
            for (result_type i = 0; i < N; ++i)
                h = h * 31 + my_hash(a[i]);
            return h;
        }
    };
    template<typename K, typename V> struct hash<map<K, V> > {
        typedef map<K, V> argument_type;
        typedef size_t result_type;
        result_type operator()(const argument_type& a) const
        {
            result_type h = 0;
            for (typename argument_type::const_iterator i=a.begin(), i_=a.end() ; i!=i_ ; ++i) {
                h = h * 31 + my_hash(i->first);
                h = h * 31 + my_hash(i->second);
            }
            return h;
        }
    };
    template<typename T> struct hash<set<T> > {
        typedef set<T> argument_type;
        typedef size_t result_type;
        result_type operator()(const argument_type& a) const
        {
            result_type h = 0;
            for (typename argument_type::const_iterator i=a.begin(), i_=a.end() ; i!=i_ ; ++i) {
                h = h * 31 + my_hash(*i);
            }
            return h;
        }
    };
}

/** These are things the shader needs that are not in the material.
 */
struct GfxGslEnvironment {
    // Varies dependning on the material.
    bool fadeDither;
    // What textures should be bound as a solid colour.
    GfxGslUnboundTextures ubt;

    // Varies depending on the geometry.
    unsigned boneWeights;
    bool instanced;

    // Varies depending on the scene / settings:
    unsigned envBoxes;
    unsigned shadowRes;
    typedef std::array<float, 3> F3;
    F3 shadowDist;
    F3 shadowSpread;
    float shadowFadeStart;
    float shadowFadeEnd;
    float shadowFactor;
    unsigned shadowFilterTaps;
    enum ShadowDitherMode {
        SHADOW_DITHER_NONE,
        SHADOW_DITHER_NOISE,
        SHADOW_DITHER_PLAIN
    } shadowDitherMode;

    GfxGslEnvironment (void)
        : fadeDither(false), boneWeights(0), instanced(false),
          envBoxes(0), shadowRes(512), shadowDist(F3{{10, 20, 30}}), shadowSpread(F3{{1, 1, 1}}),
          shadowFadeStart(50), shadowFadeEnd(60), shadowFactor(5000),
          shadowFilterTaps(0), shadowDitherMode(SHADOW_DITHER_NONE)
    { }
    bool operator== (const GfxGslEnvironment &other) const
    {
        return other.fadeDither == fadeDither
            && other.ubt == ubt
            && other.boneWeights == boneWeights
            && other.instanced == instanced
            && other.envBoxes == envBoxes
            && other.shadowRes == shadowRes
            && other.shadowDist == shadowDist
            && other.shadowSpread == shadowSpread
            && other.shadowFadeStart == shadowFadeStart
            && other.shadowFadeEnd == shadowFadeEnd
            && other.shadowFactor == shadowFactor
            && other.shadowFilterTaps == shadowFilterTaps
            && other.shadowDitherMode == shadowDitherMode;
    }
};
namespace std {
    template<> struct hash<GfxGslEnvironment> {
        size_t operator()(const GfxGslEnvironment &a) const
        {
            size_t r = 0;
            r = r * 31 + my_hash(a.fadeDither);
            r = r * 31 + my_hash(a.ubt);
            r = r * 31 + my_hash(a.boneWeights);
            r = r * 31 + my_hash(a.instanced);
            r = r * 31 + my_hash(a.envBoxes);
            r = r * 31 + my_hash(a.shadowRes);
            r = r * 31 + my_hash(a.shadowDist);
            r = r * 31 + my_hash(a.shadowSpread);
            r = r * 31 + my_hash(a.shadowFadeStart);
            r = r * 31 + my_hash(a.shadowFadeEnd);
            r = r * 31 + my_hash(a.shadowFactor);
            r = r * 31 + my_hash(a.shadowFilterTaps);
            r = r * 31 + my_hash(unsigned(a.shadowDitherMode));
            return r;
        }
    };
}
static inline std::ostream &operator << (std::ostream &o, const GfxGslEnvironment &e)
{
    o << "[";
    o << (e.fadeDither ? "F" : "f");
    o << e.envBoxes;
    o << (e.instanced ? "I" : "i");
    o << e.boneWeights;
    o << e.ubt;
    o << "]";
    return o;
}



/** All additional metadata for building a given shader.  Each shader is built for a single
 * set of these values only. */
struct GfxGslMetadata {
    // What the shader needs from all materials (types).
    GfxGslRunParams params;
    // Environment
    GfxGslEnvironment env;

};

void gfx_gasoline_check (const std::string &vert_prog,
                         const std::string &dangs_prog,
                         const std::string &additional_prog,
                         const GfxGslMetadata &md);

GfxGasolineResult gfx_gasoline_compile_wire_frame (GfxGslBackend backend,
                                                   const std::string &vert_prog,
                                                   const GfxGslMetadata &md);

GfxGasolineResult gfx_gasoline_compile_hud (GfxGslBackend backend,
                                            const std::string &vert_prog,
                                            const std::string &colour_prog,
                                            const GfxGslMetadata &md);

GfxGasolineResult gfx_gasoline_compile_sky (GfxGslBackend backend,
                                            const std::string &vert_prog,
                                            const std::string &colour_prog,
                                            const GfxGslMetadata &md);

GfxGasolineResult gfx_gasoline_compile_first_person (GfxGslBackend backend,
                                                     const std::string &vert_prog,
                                                     const std::string &dangs_prog,
                                                     const std::string &additional_prog,
                                                     const GfxGslMetadata &md);

GfxGasolineResult gfx_gasoline_compile_deferred_sun (GfxGslBackend backend,
                                                     const std::string &fog_prog,
                                                     const GfxGslMetadata &md);


GfxGasolineResult gfx_gasoline_compile_decal (GfxGslBackend backend,
                                              const std::string &dangs_prog,
                                              const std::string &additional_prog,
                                              const GfxGslMetadata &md);

// To come!
GfxGasolineResult gfx_gasoline_compile_forward (GfxGslBackend backend,
                                                const std::string &vert_prog,
                                                const std::string &fwd_prog,
                                                const GfxGslMetadata &md);

GfxGasolineResult gfx_gasoline_compile_shadow_cast (GfxGslBackend backend,
                                                    const std::string &vert_prog,
                                                    const std::string &fwd_prog,
                                                    const GfxGslMetadata &md);

#endif
