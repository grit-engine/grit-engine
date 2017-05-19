/* Copyright (c) The Grit Game Engine authors 2016
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
#include "gfx_texture_state.h"

class GfxShader;

// Bindings reuse the Param type as an easy way to store primitive values.
// Textures are specified along side in a different structure.
typedef std::map<std::string, GfxGslParam> GfxShaderBindings;

#ifndef GFX_SHADER_H
#define GFX_SHADER_H

struct GfxPaintColour {
    Vector3 diff;
    float met; // metallic paint (0 -> 1)
    float gloss;
    float spec;
};

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

    private:

    // These changed together, completely invalidate shader.
    std::string srcVertex, srcDangs, srcAdditional;
    GfxGslRunParams params;
    bool internal;


    struct Split {
        GfxGslPurpose purpose;
        GfxGslMeshEnvironment meshEnv;
        bool operator== (const Split &other) const
        {
            return other.purpose == purpose
                && other.meshEnv == meshEnv;
        }
    };

    struct SplitHash {
        size_t operator()(const Split &s) const
        {
            size_t r = my_hash(unsigned(s.purpose));
            r = r * 31 + my_hash(s.meshEnv);
            return r;
        }
    };

    struct NativePair {
        Ogre::HighLevelGpuProgramPtr vp, fp;
    };

    typedef std::unordered_map<Split, NativePair, SplitHash> ShaderCacheBySplit;

    struct GfxGslMaterialEnvironmentHash {
        size_t operator()(const GfxGslMaterialEnvironment &mat_env) const
        { return my_hash(mat_env); }
    };
    typedef std::unordered_map
        <GfxGslMaterialEnvironment, ShaderCacheBySplit, GfxGslMaterialEnvironmentHash>
            ShaderCacheByMaterial;

    struct GfxGslConfigEnvironmentHash {
        size_t operator()(const GfxGslConfigEnvironment &cfg_env) const
        { return my_hash(cfg_env); }
    };
    typedef std::unordered_map
        <GfxGslConfigEnvironment, ShaderCacheByMaterial, GfxGslConfigEnvironmentHash>
            ShaderCacheByConfig;

    ShaderCacheByConfig shaderCache;

    public:

    const std::string name;

    // Gasoline shaders
    GfxShader (const std::string &name,
               const GfxGslRunParams &params,
               const std::string &src_vertex,
               const std::string &src_dangs,
               const std::string &src_additional,
               bool internal)
      : name(name)
    {
        reset(params, src_vertex, src_dangs, src_additional, internal);
    }


    void reset (const GfxGslRunParams &params,
                const std::string &src_vertex,
                const std::string &src_dangs,
                const std::string &src_additional,
                bool internal);

    const GfxGslRunParams &getParams(void) const { return params; }


    // New API, may throw compilation errors if not checked previously.
    void bindShader (GfxGslPurpose purpose,
                     const GfxGslMaterialEnvironment &mat_env,
                     const GfxGslMeshEnvironment &mesh_env,
                     const GfxShaderGlobals &globs,
                     const Ogre::Matrix4 &world,
                     const Ogre::Matrix4 *bone_world_matrixes,
                     unsigned num_bone_world_matrixes,
                     float fade,
                     const GfxPaintColour *paint_colours,  // Array of 4
                     const GfxTextureStateMap &textures,
                     const GfxShaderBindings &bindings);

    // New API, may throw compilation errors if not checked previously. (DEPRECATED)
    void bindShader (GfxGslPurpose purpose,
                     bool fade_dither, bool instanced, unsigned bone_weights,
                     const GfxShaderGlobals &globs,
                     const Ogre::Matrix4 &world,
                     const Ogre::Matrix4 *bone_world_matrixes,
                     unsigned num_bone_world_matrixes,
                     float fade,
                     const GfxPaintColour *paint_colours,  // Array of 4
                     const GfxTextureStateMap &textures,
                     const GfxShaderBindings &bindings);

    // Defaults the paint_colours for the many cases that don't use them.
    void bindShader (GfxGslPurpose purpose,
                     const GfxGslMaterialEnvironment &mat_env,
                     const GfxGslMeshEnvironment &mesh_env,
                     const GfxShaderGlobals &globs,
                     const Ogre::Matrix4 &world,
                     const Ogre::Matrix4 *bone_world_matrixes,
                     unsigned num_bone_world_matrixes,
                     float fade,
                     const GfxTextureStateMap &textures,
                     const GfxShaderBindings &bindings);

    // Defaults the paint_colours for the many cases that don't use them. (DEPRECATED)
    void bindShader (GfxGslPurpose purpose,
                     bool fade_dither, bool instanced, unsigned bone_weights,
                     const GfxShaderGlobals &globs,
                     const Ogre::Matrix4 &world,
                     const Ogre::Matrix4 *bone_world_matrixes,
                     unsigned num_bone_world_matrixes,
                     float fade,
                     const GfxTextureStateMap &textures,
                     const GfxShaderBindings &bindings);

    // When the material is created (or reset)
    void initPass (Ogre::Pass *p, GfxGslPurpose purpose,
                   const GfxGslMaterialEnvironment &mat_env,
                   const GfxGslMeshEnvironment &mesh_env,
                   const GfxTextureStateMap &textures,
                   const GfxShaderBindings &bindings);

    // Every frame
    void updatePass (Ogre::Pass *p,
                     const GfxShaderGlobals &globs,
                     GfxGslPurpose purpose,
                     const GfxGslMaterialEnvironment &mat_env,
                     const GfxGslMeshEnvironment &mesh_env,
                     const GfxTextureStateMap &textures,
                     const GfxShaderBindings &bindings);

    void populateMatEnv (bool fade_dither,
                         const GfxTextureStateMap &textures,
                         const GfxShaderBindings &bindings,
                         GfxGslMaterialEnvironment &mat_env);

    void populateMeshEnv (bool instanced, unsigned bone_weights,
                          GfxGslMeshEnvironment &mesh_env);

    protected:

    NativePair getNativePair (GfxGslPurpose purpose,
                              const GfxGslMaterialEnvironment &mat_env,
                              const GfxGslMeshEnvironment &mesh_env);

    // Generic: binds uniforms (not textures, but texture indexes) for both RS and passes
    void bindGlobals (const Ogre::GpuProgramParametersSharedPtr &vparams,
                      const Ogre::GpuProgramParametersSharedPtr &fparams,
                      const GfxShaderGlobals &params, GfxGslPurpose purpose);
    void bindShaderParams (int counter,
                           const Ogre::GpuProgramParametersSharedPtr &vparams,
                           const Ogre::GpuProgramParametersSharedPtr &fparams,
                           const GfxTextureStateMap &textures,
                           const GfxShaderBindings &bindings);

    // RenderSystem bindings
    // gloal textures
    int bindGlobalTexturesRs (const GfxShaderGlobals &params, GfxGslPurpose purpose);
    // user-defined textures
    void bindShaderParamsRs (int counter, const GfxTextureStateMap &textures);
    // body stuff
    void bindBodyParamsRS (const Ogre::GpuProgramParametersSharedPtr &vparams,
                           const Ogre::GpuProgramParametersSharedPtr &fparams,
                           const GfxShaderGlobals &p,
                           const Ogre::Matrix4 &world,
                           const Ogre::Matrix4 *bone_world_matrixes,
                           unsigned num_bone_world_matrixes,
                           float fade,
                           const GfxPaintColour *paint_colours,  // Array of 4
                           GfxGslPurpose purpose);

    // Init pass (set up everything)
    void initPassGlobalTextures (Ogre::Pass *p, GfxGslPurpose purpose);
    void initPassTextures (Ogre::Pass *p, const GfxTextureStateMap &textures);
    void initPassBodyParams (const Ogre::GpuProgramParametersSharedPtr &vparams,
                             const Ogre::GpuProgramParametersSharedPtr &fparams,
                             GfxGslPurpose purpose);

    void updatePassGlobalTextures (Ogre::Pass *p, GfxGslPurpose purpose);
    void updatePassTextures (Ogre::Pass *p, int counter, const GfxTextureStateMap &textures);
    void updatePassGlobals (const Ogre::GpuProgramParametersSharedPtr &vparams,
                            const Ogre::GpuProgramParametersSharedPtr &fparams,
                            const GfxShaderGlobals &params, GfxGslPurpose purpose);
    
    friend std::ostream &operator << (std::ostream &, const Split &);
};


// Ensure the given source code is statically correct.
void gfx_shader_check (const std::string &name,
                       const std::string &new_vertex_code,
                       const std::string &new_dangs_code,
                       const std::string &new_additional_code,
                       const GfxGslRunParams &params,
                       bool internal);

GfxShader *gfx_shader_make_or_reset (const std::string &name,
                                     const std::string &new_vertex_code,
                                     const std::string &new_dangs_code,
                                     const std::string &new_additional_code,
                                     const GfxGslRunParams &params,
                                     bool internal);

GfxShader *gfx_shader_get (const std::string &name);
bool gfx_shader_has (const std::string &name);

void gfx_shader_init (void);
void gfx_shader_shutdown (void);

#endif
