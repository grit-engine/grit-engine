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

// Bindings reuse the Param type as an easy way to store primitive values.
// Textures are specified along side in a different structure.
typedef std::map<std::string, GfxGslParam> GfxShaderBindings;

#ifndef GFX_SHADER_H
#define GFX_SHADER_H

/** Some parameters that do not change from one object to the next, but do change from one
 * camera or target to another. */
struct GfxShaderGlobals {
    Vector3 cam_pos;
    Ogre::Matrix4 view;
    Ogre::Matrix4 invView;  // For first person rendering
    Ogre::Matrix4 proj;
    Vector3 rayTopLeft;
    Vector3 rayTopRight;
    Vector3 rayBottomLeft;
    Vector3 rayBottomRight;
    Vector2 viewport_dim;
    bool render_target_flipping;
    GfxPipeline *pipe;
};

GfxShaderGlobals gfx_shader_globals_cam (GfxPipeline *pipe, const Ogre::Matrix4 &proj);
GfxShaderGlobals gfx_shader_globals_cam (GfxPipeline *pipe);

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
        ALPHA,
        FIRST_PERSON,
        EMISSIVE,
        SHADOW_CAST,
        WIRE_FRAME,
        SKY,
        HUD,
        DECAL,
    };

    private:

    // These changed together, completely invalidate shader.
    std::string srcVertex, srcDangs, srcAdditional;
    GfxGslRunParams params;

    public:
    struct Split {
        Purpose purpose;
        GfxGslEnvironment env;
        // TODO: Split shader for (also update std::hash)
        // * user-defined enums
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
                     bool fade_dither, unsigned env_boxes, bool instanced, unsigned bone_weights,
                     const GfxShaderGlobals &params,
                     const Ogre::Matrix4 &world,
                     const Ogre::Matrix4 *bone_world_matrixes,
                     unsigned num_bone_world_matrixes,
                     float fade,
                     const GfxMaterialTextureMap &textures,
                     const GfxShaderBindings &bindings);

    protected:
    NativePair getNativePair (Purpose purpose,
                              bool fade_dither, unsigned env_boxes,
                              bool instanced, unsigned bone_weights,
                              const GfxMaterialTextureMap &textures);
    void bindBodyParams (const NativePair &np, const GfxShaderGlobals &p,
                         const Ogre::Matrix4 &world,
                         const Ogre::Matrix4 *bone_world_matrixes,
                         unsigned num_bone_world_matrixes, float fade);
    void bindGlobals (const NativePair &np, const GfxShaderGlobals &params, Purpose purpose);
    void explicitBinding (const NativePair &np, const std::string &name, const Ogre::Matrix4 &v);
    void explicitBinding (const NativePair &np, const std::string &name, int v);
    void explicitBinding (const NativePair &np, const std::string &name, float v);
    void explicitBinding (const NativePair &np, const std::string &name, const Vector2 &v);
    void explicitBinding (const NativePair &np, const std::string &name, const Vector3 &v);
    void explicitBinding (const NativePair &np, const std::string &name, const Vector4 &v);

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
