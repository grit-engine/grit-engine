/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
 *
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
#include <functional>
#include <unordered_map>
#include <string>

#include <OgreHighLevelGpuProgram.h>

#include <math_util.h>

#include "../shared_ptr.h"

#include "gfx_gasoline.h"
#include "gfx_pipeline.h"
#include "gfx_material.h"

class GfxShader;
struct GfxShaderParam;

// Bindings reuse the Param type as an easy way to store primitive values.
// Textures are specified along side in a different structure.
typedef std::map<std::string, GfxShaderParam> GfxShaderBindings;

#ifndef GFX_SHADER_H
#define GFX_SHADER_H

/** Tagged union of the various values that can go into a shader param. */
struct GfxShaderParam {
    GfxGslParamType t; 
    // Use std::array instead of c-style array to work around a bug in Microsoft Visual Studio 2013
    typedef std::array<float, 4> F4;
    F4 fs;
    typedef std::array<int32_t, 4> I4;
    I4 is;
    Vector4 getVector4 (void) const { return Vector4(fs[0], fs[1], fs[2], fs[3]); }
    GfxShaderParam (void) { }
    // This form for textures
    GfxShaderParam (GfxGslParamType t, const Vector4 &v) : t(t), fs(F4{{v.x, v.y, v.z, v.w}}), is(I4{{0}}) { }
    GfxShaderParam (int v) : t(GFX_GSL_INT1), fs(F4{{0}}), is(I4{{v}}) { }
    GfxShaderParam (float v) : t(GFX_GSL_FLOAT1), fs(F4{{v}}), is(I4{{0}}) { }
    GfxShaderParam (const Vector2 &v) : t(GFX_GSL_FLOAT2), fs(F4{{v.x, v.y}}), is(I4{{0}}) { }
    GfxShaderParam (const Vector3 &v) : t(GFX_GSL_FLOAT3), fs(F4{{v.x, v.y, v.z}}), is(I4{{0}}) { }
    GfxShaderParam (const Vector4 &v) : t(GFX_GSL_FLOAT4), fs(F4{{v.x, v.y, v.z, v.w}}), is(I4{{0}}) { }
};

typedef std::map<std::string, GfxShaderParam> GfxShaderParamMap;

/** Some parameters that do not change from one object to the next. */
struct GfxShaderGlobals {
    Vector3 cam_pos;
    Ogre::Matrix4 view;
    Ogre::Matrix4 proj;
    Vector2 viewport_dim;
    bool render_target_flipping;
};

// User always gives n strings, some of which can be empty.
// E.g. dangs shader for sky is not needed (no lighting equation)
// 

/** Rrepresents the behavioural information provided by the user
 * (artist) for rendering a particular pass.
 *
 * From this, native shaders are generated on demand for different instantiations
 * of this shaders (in different materials) and also for different purposes
 * (e.g. shadow casting vs emissive passes, etc).
 */
class GfxShader {

    public:
    enum Purpose {
        REGULAR,
        SKY,
        FIRST_PERSON,
        HUD,
        WIRE_FRAME
    };

    private:

    // These changed together, completely invalidate shader.
    std::string srcVertex, srcDangs, srcAdditional;
    GfxShaderParamMap params;

    struct Split {
        Purpose purpose;
        std::set<std::string> boundTextures;
        // TODO: Split shader for (update SplitHash too)
        // * bones count
        // * fade status
        // * instancing
        // * enums (manual splits)
        bool operator== (const Split &other) const
        {
            return other.purpose == purpose
                && other.boundTextures == boundTextures;
        }
    };
    struct SplitHash {
        size_t operator()(const Split &s) const
        {
            size_t r = size_t(s.purpose) << 10;
            r ^= s.boundTextures.size();
            for (const auto &str : s.boundTextures)
                r ^= std::hash<std::string>()(str);
            return r;
        }
    };
    public:
    struct NativePair {
        Ogre::HighLevelGpuProgramPtr vp, fp;
    };

    private:

    std::unordered_map<Split, NativePair, SplitHash> cachedShaders;

    public:

    const std::string name;

    // Gasoline shaders
    GfxShader (const std::string &name,
               const GfxShaderParamMap &params,
               const std::string &src_vertex,
               const std::string &src_dangs,
               const std::string &src_additional)
      : name(name)
    {
        reset(params, src_vertex, src_dangs, src_additional);
    }


    void reset (const GfxShaderParamMap &params,
                const std::string &src_vertex,
                const std::string &src_dangs,
                const std::string &src_additional);



    // New API, may throw compilation errors if not checked previously.
    NativePair getNativePair (Purpose purpose, const std::set<std::string> &textures);
    NativePair getNativePair (Purpose purpose, const GfxMaterialTextureMap &textures);
    void bindShader (Purpose purpose,
                     const GfxShaderGlobals &params,
                     const Ogre::Matrix4 &world,
                     const GfxMaterialTextureMap &textures,
                     const GfxShaderBindings &bindings);
    protected:
    void bindGlobals (const NativePair &np, const GfxShaderGlobals &params,
                      const Ogre::Matrix4 &world);
    void explicitBinding (const NativePair &np, const std::string &name, const Ogre::Matrix4 &v);
    void explicitBinding (const NativePair &np, const std::string &name, int v);
    void explicitBinding (const NativePair &np, const std::string &name, float v);
    void explicitBinding (const NativePair &np, const std::string &name, const Vector2 &v);
    void explicitBinding (const NativePair &np, const std::string &name, const Vector3 &v);
    void explicitBinding (const NativePair &np, const std::string &name, const Vector4 &v);
    public:

    // Legacy API
    GfxShader (const std::string &name,
               const GfxShaderParamMap &params,
               const Ogre::HighLevelGpuProgramPtr &vp,
               const Ogre::HighLevelGpuProgramPtr &fp)
      : params(params), name(name)
    {
        legacy = {vp, fp};
    }

    NativePair legacy;
    void bindGlobals (const GfxShaderGlobals &params, const Ogre::Matrix4 &world);
    void bindShaderParams (void);
    void validate (void);
    void explicitBinding (const std::string &name, const Ogre::Matrix4 &v);
    void explicitBinding (const std::string &name, int v);
    void explicitBinding (const std::string &name, float v);
    void explicitBinding (const std::string &name, const Vector2 &v);
    void explicitBinding (const std::string &name, const Vector3 &v);
    void explicitBinding (const std::string &name, const Vector4 &v);
    const Ogre::HighLevelGpuProgramPtr &getHackOgreVertexProgram (void) { return legacy.vp; }
    const Ogre::HighLevelGpuProgramPtr &getHackOgreFragmentProgram (void) { return legacy.fp; }
    
};

void gfx_shader_bind_global_textures (const GfxShader::NativePair &np);

// Ensure the given source code works for the given purpose.
void gfx_shader_check (const std::string &name,
                       const std::string &new_vertex_code,
                       const std::string &new_dangs_code,
                       const std::string &new_additional_code,
                       const GfxShaderParamMap &params);

/** Temporary hack to allow transition to a purely gasoline setup. */
GfxShader *gfx_shader_make_from_existing (const std::string &name,
                                          const Ogre::HighLevelGpuProgramPtr &vp,
                                          const Ogre::HighLevelGpuProgramPtr &fp,
                                          const GfxShaderParamMap &params);

GfxShader *gfx_shader_make_or_reset (const std::string &name,
                                     const std::string &new_vertex_code,
                                     const std::string &new_dangs_code,
                                     const std::string &new_additional_code,
                                     const GfxShaderParamMap &params);

GfxShader *gfx_shader_get (const std::string &name);
bool gfx_shader_has (const std::string &name);

void gfx_shader_init (void);
void gfx_shader_shutdown (void);

#endif
