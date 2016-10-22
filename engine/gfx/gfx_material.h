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

class GfxMaterial;

#ifndef GFX_MATERIAL_H
#define GFX_MATERIAL_H

#include <mutex>

#include "gfx.h"
#include "gfx_internal.h"
#include "gfx_shader.h"
#include "gfx_texture_state.h"


/* LOCKING STRATEGY:
 *
 * The background resource loading thread needs to know what dependent textures to load
 * after loading a mesh referencing a particular set of materials.  This means it has
 * to access the material database, which is also being accessed from the primary render
 * thread.
 */
extern std::recursive_mutex gfx_material_lock;
#define GFX_MAT_SYNC std::lock_guard<std::recursive_mutex> _scoped_lock(gfx_material_lock)


class GfxBaseMaterial {

    protected:

    GfxShader *shader;
    GfxShaderBindings bindings;
    GfxTextureStateMap textures;
    
    public:

    GfxBaseMaterial (const std::string name, GfxShader *shader);
    virtual ~GfxBaseMaterial (void) { }
    
    const std::string name;
    
    const GfxTextureStateMap &getTextures (void) const { return textures; } 
    void setTextures (const GfxTextureStateMap &v) { GFX_MAT_SYNC; textures = v; }

    void addDependencies (DiskResource *into) const;

    const GfxShaderBindings &getBindings (void) const { return bindings; }
    void setBindings (const GfxShaderBindings &v) { bindings = v; }
    
    GfxShader *getShader (void) const { return shader; }
    void setShader (GfxShader *v);
    
};

typedef std::map<std::string, GfxBaseMaterial*> GfxMaterialDB;

extern GfxMaterialDB material_db;


typedef std::vector<GfxMaterial*> GfxMaterials;

enum GfxMaterialSceneBlend {
    GFX_MATERIAL_OPAQUE,
    GFX_MATERIAL_ALPHA,
    GFX_MATERIAL_ALPHA_DEPTH,
};

class GfxMaterial : public GfxBaseMaterial {
    public: // hack
    Ogre::MaterialPtr regularMat;     // Either just forward or complete (for alpha, etc)
    Ogre::MaterialPtr additionalMat;  // Just the additional lighting as an additive pass
    Ogre::MaterialPtr castMat;        // For shadow cast phase
    Ogre::MaterialPtr instancingMat;  // For rendering with instanced geometry
    Ogre::MaterialPtr instancingCastMat; // Shadow cast phase (instanced geometry)
    Ogre::MaterialPtr wireframeMat;     // Just white (complete shader).

    private:
    GfxMaterial (const std::string &name);
    GfxMaterialSceneBlend sceneBlend;
    bool backfaces;
    bool castShadows;
    // TODO(dcunnin):  Infer this from the additional gasoline (constant propagation).
    // In particular, first person and alpha do not honour this annotation (so it's broken).
    bool additionalLighting;
    // How many bones can be blended at a given vertex.
    unsigned boneBlendWeights;
    // Whether to use discard from dangs shader
    // TODO(dcunnin): Infer this from the dangs gasoline (is it possible to discard a fragment?)
    bool shadowAlphaReject;
    float shadowBias;

    // TODO: various addressing modes for textures

    public:
    GfxMaterialSceneBlend getSceneBlend (void) const { return sceneBlend; }
    void setSceneBlend (GfxMaterialSceneBlend v);

    bool getBackfaces (void) const { return backfaces; }
    void setBackfaces (bool v);

    unsigned getBoneBlendWeights (void) const { return boneBlendWeights; }
    void setBoneBlendWeights (unsigned v);

    bool getCastShadows (void) const { return castShadows; }
    void setCastShadows (bool v);

    bool getAdditionalLighting (void) const { return additionalLighting; }
    void setAdditionalLighting (bool v);

    bool getShadowBias (void) const { return shadowBias; }
    void setShadowBias (float v);

    bool getShadowAlphaReject (void) const { return shadowAlphaReject; }
    void setShadowAlphaReject (bool v);

    void buildOgreMaterials (void);
    void updateOgreMaterials (const GfxShaderGlobals &globs);

    friend GfxMaterial *gfx_material_add(const std::string &);
    friend class GfxBody;
};


GfxMaterial *gfx_material_add (const std::string &name);

GfxMaterial *gfx_material_add_or_get (const std::string &name);

GfxMaterial *gfx_material_get (const std::string &name);

bool gfx_material_has_any (const std::string &name);

bool gfx_material_has (const std::string &name);

void gfx_material_init (void);

#endif
