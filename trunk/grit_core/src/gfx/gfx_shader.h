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
#include <string>

#include <OgreHighLevelGpuProgram.h>

#include <math_util.h>

#include "../shared_ptr.h"

#include "gfx_gasoline.h"
#include "gfx_pipeline.h"

class GfxShader;
class GfxShaderBindings;
typedef SharedPtr<GfxShaderBindings> GfxShaderBindingsPtr;


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
    GfxShaderParam (void) { }
    // This form for textures
    GfxShaderParam (GfxGslParamType t, const Vector4 &v) : t(t), fs(F4{{v.x, v.y, v.z, v.w}}), is(I4{{0}}) { }
    GfxShaderParam (int v) : t(GFX_GSL_FLOAT1), fs(F4{{0}}), is(I4{{v}}) { }
    GfxShaderParam (float v) : t(GFX_GSL_FLOAT1), fs(F4{{v}}), is(I4{{0}}) { }
    GfxShaderParam (const Vector2 &v) : t(GFX_GSL_FLOAT2), fs(F4{{v.x, v.y}}), is(I4{{0}}) { }
    GfxShaderParam (const Vector3 &v) : t(GFX_GSL_FLOAT3), fs(F4{{v.x, v.y, v.z}}), is(I4{{0}}) { }
    GfxShaderParam (const Vector4 &v) : t(GFX_GSL_FLOAT4), fs(F4{{v.x, v.y, v.z, v.w}}), is(I4{{0}}) { }
};

typedef std::map<std::string, GfxShaderParam> GfxShaderParamMap;

class GfxShader {
    const std::string name;
    const Ogre::HighLevelGpuProgramPtr vp, fp;
    GfxShaderParamMap params;
    public:
    GfxShader (const std::string &name,
               const Ogre::HighLevelGpuProgramPtr &vp,
               const Ogre::HighLevelGpuProgramPtr &fp,
               const GfxShaderParamMap &params)
      : name(name), vp(vp), fp(fp), params(params)
    {
    }

    void validate (void);

    GfxShaderBindingsPtr makeBindings (void);
    const GfxShaderParamMap &getParams (void) const { return params; }
    void setParams (const GfxShaderParamMap &v) { params = v; }

    void bindShader (void);
    void bindGlobals (const Ogre::Matrix4 &world,
                      const Ogre::Matrix4 &view,
                      const Ogre::Matrix4 &proj,
                      const Vector2 &viewport_dim,
                      bool render_target_flipping);
    void bind (const GfxShaderBindingsPtr &bindings);
    void bindShaderParams (void);


    const Ogre::HighLevelGpuProgramPtr &getOgreVertexProgram (void) { return vp; }
    const Ogre::HighLevelGpuProgramPtr &getOgreFragmentProgram (void) { return fp; }
    void explicitBinding (const std::string &name, const Ogre::Matrix4 &v);
    void explicitBinding (const std::string &name, int v);
    void explicitBinding (const std::string &name, float v);
    void explicitBinding (const std::string &name, const Vector2 &v);
    void explicitBinding (const std::string &name, const Vector3 &v);
    void explicitBinding (const std::string &name, const Vector4 &v);
    void explicitTextures (const std::vector<std::string> &texs);
    
};

class GfxShaderBindings {
    typedef std::map<std::string, GfxShaderParam> Map;
    Map bindings;
    public:
    GfxShaderBindings (void)
    { }
    template<class T> void setBinding (const std::string &name, const T &v)
    {
        bindings[name] = GfxShaderParam(v);
    }
    GfxShaderParam getBinding (const std::string &name);
    friend class GfxShader;
};

/** Temporary hack to allow transition to a purely gasoline setup. */
GfxShader *gfx_shader_make_from_existing (const std::string &name,
                                          const Ogre::HighLevelGpuProgramPtr &vp,
                                          const Ogre::HighLevelGpuProgramPtr &fp,
                                          const GfxShaderParamMap &params);

GfxShader *gfx_shader_make_or_reset (const std::string &name,
                                     const std::string &new_vertex_code,
                                     const std::string &new_forward_code,
                                     const std::string &new_emissive_code,
                                     const GfxShaderParamMap &params);

GfxShader *gfx_shader_make_or_reset_sky (const std::string &name,
                                         const std::string &new_vertex_code,
                                         const std::string &new_fragment_code,
                                         const GfxShaderParamMap &params);

GfxShader *gfx_shader_get (const std::string &name);
bool gfx_shader_has (const std::string &name);

void gfx_shader_init (void);
void gfx_shader_shutdown (void);

#endif
