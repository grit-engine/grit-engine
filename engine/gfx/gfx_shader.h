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

#include <math_util.h>

#include "../shared_ptr.h"

#include "gfx_gasoline.h"
#include "gfx_pipeline.h"
#include "gfx_material.h"

class GfxShader;

// Bindings reuse the Param type as an easy way to store primitive values.
// Textures are specified along side in a different structure.
typedef std::map<std::string, GfxGslParam> GfxShaderBindings;

#ifndef GFX_SHADER_H
#define GFX_SHADER_H

/** Some parameters that do not change from one object to the next, but do change from one
 * camera / target to another. */
struct GfxShaderGlobals {
    Vector3 camPos;
    Ogre::Matrix4 view;
    Ogre::Matrix4 invView;  // For first person rendering
    Ogre::Matrix4 proj;
    Vector3 rayTopLeft;
    Vector3 rayTopRight;
    Vector3 rayBottomLeft;
    Vector3 rayBottomRight;
    Vector2 viewportDim;
    bool renderTargetFlipping;
    GfxPipeline *pipe;
};

/** Specify a viewport other than the one bound to the camera. */
GfxShaderGlobals gfx_shader_globals_cam_ss (GfxPipeline *pipe, Ogre::Viewport *viewport);

/** Use an alternative projection matrix, e.g. for screen-space effects. */
GfxShaderGlobals gfx_shader_globals_cam (GfxPipeline *pipe, const Ogre::Matrix4 &proj);

GfxShaderGlobals gfx_shader_globals_cam (GfxPipeline *pipe);

// User always gives n strings, some of which can be empty.
// E.g. dangs shader for sky is not needed (no lighting equation)
// 

/** Represents the behavioural information provided by the user
 * (artist) for rendering a particular pass.
 *
 * From this, native shaders are generated on demand for different instantiations
 * of this shader (i.e., in different materials) and also for different purposes
 * (e.g. shadow casting vs emissive passes, etc).
 *
 * The rationale is that a material may define a Gasoline shader, but various rendering
 * ops are needed in practice to render objects using that material.  So we will recompile
 * the shdaer for these various purposes when neeeded.
 */
class GfxShader {

    public:
    enum Purpose {
        REGULAR,
        ALPHA,
        FIRST_PERSON,
        FIRST_PERSON_WIREFRAME,
        ADDITIONAL,
        SHADOW_CAST,
        WIREFRAME,

        // The following are internal cases.  It may make more sense
        // to have a completely different path for them.
        SKY,  // Also used for "misc" rendering.
        HUD,
        DECAL,
        DEFERRED_AMBIENT_SUN,
    };

    private:

    // These changed together, completely invalidate shader.
    std::string srcVertex, srcDangs, srcAdditional;
    GfxGslRunParams params;

    public:
    struct Split {
        Purpose purpose;
        GfxGslEnvironment env;
        bool operator== (const Split &other) const
        {
            return other.purpose == purpose
                && other.env == env;
        }
    };
    private:
    struct SplitHash {
        size_t operator()(const Split &s) const
        {
            size_t r = my_hash(unsigned(s.purpose));
            r = r * 31 + my_hash(s.env);
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
               const GfxGslRunParams &params,
               const std::string &src_vertex,
               const std::string &src_dangs,
               const std::string &src_additional)
      : name(name)
    {
        reset(params, src_vertex, src_dangs, src_additional);
    }


    void reset (const GfxGslRunParams &params,
                const std::string &src_vertex,
                const std::string &src_dangs,
                const std::string &src_additional);


    // New API, may throw compilation errors if not checked previously.
    void bindShader (Purpose purpose,
                     bool fade_dither, bool instanced, unsigned bone_weights,
                     const GfxShaderGlobals &params,
                     const Ogre::Matrix4 &world,
                     const Ogre::Matrix4 *bone_world_matrixes,
                     unsigned num_bone_world_matrixes,
                     float fade,
                     const GfxMaterialTextureMap &textures,
                     const GfxShaderBindings &bindings);

    void bindShaderPass (Ogre::Pass *p, Purpose purpose,
                         bool fade_dither, bool instanced, unsigned bone_weights,
                         const GfxShaderGlobals &params,
                         const GfxMaterialTextureMap &textures,
                         const GfxShaderBindings &bindings);

    protected:
    NativePair getNativePair (Purpose purpose,
                              bool fade_dither, unsigned env_boxes,
                              bool instanced, unsigned bone_weights,
                              const GfxGslUnboundTextures &ubt,
                              const GfxShaderBindings &statics);
    public:
    NativePair getNativePair (Purpose purpose,
                              bool fade_dither, unsigned env_boxes,
                              bool instanced, unsigned bone_weights,
                              const GfxMaterialTextureMap &textures,
                              const GfxShaderBindings &statics);
    protected:
    void bindShaderParams (const Ogre::GpuProgramParametersSharedPtr &vparams,
                           const Ogre::GpuProgramParametersSharedPtr &fparams,
                           const GfxMaterialTextureMap &textures,
                           const GfxShaderBindings &bindings);
    void bindShaderParamsPass (Ogre::Pass *p, const GfxMaterialTextureMap &textures);
    void bindShaderParamsRs (const GfxMaterialTextureMap &textures);
    void bindBodyParamsPass (const Ogre::GpuProgramParametersSharedPtr &vparams,
                             const Ogre::GpuProgramParametersSharedPtr &fparams);
    void bindBodyParams (const Ogre::GpuProgramParametersSharedPtr &vparams,
                         const Ogre::GpuProgramParametersSharedPtr &fparams,
                         const GfxShaderGlobals &p,
                         const Ogre::Matrix4 &world,
                         const Ogre::Matrix4 *bone_world_matrixes,
                         unsigned num_bone_world_matrixes, float fade);
    void bindGlobals (const Ogre::GpuProgramParametersSharedPtr &vparams,
                      const Ogre::GpuProgramParametersSharedPtr &fparams,
                      const GfxShaderGlobals &params);
    void bindGlobalsPass (Ogre::Pass *p, Purpose purpose);
    void bindGlobalsRs (const GfxShaderGlobals &params, Purpose purpose);

    public:
    // Legacy API
    GfxShader (const std::string &name,
               const GfxGslRunParams &params,
               const Ogre::HighLevelGpuProgramPtr &vp,
               const Ogre::HighLevelGpuProgramPtr &fp)
      : params(params), name(name)
    {
        legacy = {vp, fp};
    }

    NativePair legacy;
/*
    void bindBodyParams (const GfxShaderGlobals &params, const Ogre::Matrix4 &world, float fade);
    void bindGlobals (const GfxShaderGlobals &params);
*/
    void validate (void);
    const Ogre::HighLevelGpuProgramPtr &getHackOgreVertexProgram (void) { return legacy.vp; }
    const Ogre::HighLevelGpuProgramPtr &getHackOgreFragmentProgram (void) { return legacy.fp; }
    
};

// Ensure the given source code works for the given purpose.
void gfx_shader_check (const std::string &name,
                       const std::string &new_vertex_code,
                       const std::string &new_dangs_code,
                       const std::string &new_additional_code,
                       const GfxGslRunParams &params);

/** Temporary hack to allow transition to a purely gasoline setup. */
GfxShader *gfx_shader_make_from_existing (const std::string &name,
                                          const Ogre::HighLevelGpuProgramPtr &vp,
                                          const Ogre::HighLevelGpuProgramPtr &fp,
                                          const GfxGslRunParams &params);

GfxShader *gfx_shader_make_or_reset (const std::string &name,
                                     const std::string &new_vertex_code,
                                     const std::string &new_dangs_code,
                                     const std::string &new_additional_code,
                                     const GfxGslRunParams &params);

GfxShader *gfx_shader_get (const std::string &name);
bool gfx_shader_has (const std::string &name);

void gfx_shader_init (void);
void gfx_shader_shutdown (void);

#endif
