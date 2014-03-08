/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

#include <vector>

class GfxSkyMaterial;
class GfxSkyShader;
typedef std::vector<GfxSkyMaterial*> GfxSkyMaterials;
typedef std::vector<GfxSkyShader*> GfxSkyShaders;

#ifndef GFX_SKY_MATERIAL_H
#define GFX_SKY_MATERIAL_H

#include <OgreMaterial.h>

#include "gfx_material.h"
#include "gfx_internal.h"

typedef std::vector<std::string> GfxSkyShaderVariation;

enum GfxSkyShaderUniformKind {
    GFX_SHADER_UNIFORM_KIND_PARAM_TEXTURE2D,
    GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT1,
    //GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT2,
    GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT3,
    GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT4
};

struct GfxSkyShaderUniform {
    GfxSkyShaderUniformKind kind;
    std::vector<float> defaults;

    // texture-specific stuff
    Vector3 defaultColour;
    float defaultAlpha;
};

typedef std::map<std::string, GfxSkyShaderUniform> GfxSkyShaderUniformMap;

class GfxSkyShader {
    std::string vertexCode;
    std::string fragmentCode;
    Ogre::HighLevelGpuProgramPtr vp, fp;
    std::vector<GfxSkyShaderVariation> variations;
    GfxSkyShaderUniformMap uniforms;

    public:
    const std::string name;
    GfxSkyShader (const std::string &name);

    void reset (const std::string &new_vertex_source,
                const std::string &new_fragment_source,
                const std::vector<GfxSkyShaderVariation> &new_variations,
                const GfxSkyShaderUniformMap &new_uniforms);
    const Ogre::HighLevelGpuProgramPtr &getFP() { return fp; }
    const Ogre::HighLevelGpuProgramPtr &getVP() { return vp; }

    const GfxSkyShaderUniformMap &getUniforms (void) { return uniforms; }

};
            
GfxSkyShader *gfx_sky_shader_add (const std::string &name);

GfxSkyShader *gfx_sky_shader_add_or_get (const std::string &name);

GfxSkyShader *gfx_sky_shader_get (const std::string &name);
    
bool gfx_sky_shader_has (const std::string &name);



struct GfxSkyMaterialUniform {
    GfxSkyShaderUniformKind kind;
    std::vector<float> values;
    
    GfxTextureDiskResource *texture;
    bool clamp;
    int anisotropy;
};

typedef std::map<std::string, GfxSkyMaterialUniform> GfxSkyMaterialUniformMap;


enum GfxSkyMaterialSceneBlend { GFX_SKY_MATERIAL_OPAQUE, GFX_SKY_MATERIAL_ALPHA, GFX_SKY_MATERIAL_ADD };

class GfxSkyMaterial : public GfxBaseMaterial {

    GfxSkyMaterial (const std::string &name);

    private: GfxSkyMaterialUniformMap uniforms;
    public: const GfxSkyMaterialUniformMap &getUniforms (void) { return uniforms; } // take MAT_SYNC when iterating through this stuff
    public: void setUniforms (const GfxSkyMaterialUniformMap &v) { GFX_MAT_SYNC; uniforms = v; /*TODO: check this stuff*/ }
    
    private: GfxSkyShader *shader;
    public: GfxSkyShader *getShader (void) const { return shader; }
    public: void setShader (GfxSkyShader *v) { GFX_MAT_SYNC; shader = v; uniforms.clear(); }

    private: GfxSkyMaterialSceneBlend sceneBlend;
    public: GfxSkyMaterialSceneBlend getSceneBlend (void) const { return sceneBlend; }
    public: void setSceneBlend (GfxSkyMaterialSceneBlend v) { sceneBlend = v; }

    public:

    void addDependencies (DiskResource *into);

    const std::string name;

    friend GfxSkyMaterial *gfx_sky_material_add(const std::string &);
    friend class GfxSkyBody;
};  
    
GfxSkyMaterial *gfx_sky_material_add (const std::string &name);

GfxSkyMaterial *gfx_sky_material_add_or_get (const std::string &name);

GfxSkyMaterial *gfx_sky_material_get (const std::string &name);
    
bool gfx_sky_material_has (const std::string &name);

void gfx_sky_material_shutdown (void);

#endif
