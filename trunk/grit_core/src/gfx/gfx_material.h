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

class GfxMaterial;

#ifndef GFX_MATERIAL_H
#define GFX_MATERIAL_H

#include <mutex>

#include <OgreMaterial.h>

#include "gfx.h"
#include "gfx_internal.h"
#include "gfx_shader.h"


/* LOCKING STRATEGY:
 *
 * The background resource loading thread needs to know what dependent textures to load
 * after loading a mesh referencing a particular set of materials.  This means it has
 * to access the material database, which is also being accessed from the primary render
 * thread.
 */
extern std::recursive_mutex gfx_material_lock;
#define GFX_MAT_SYNC std::lock_guard<std::recursive_mutex> _scoped_lock(gfx_material_lock)


struct GfxMaterialTexture {
    GfxTextureDiskResource *texture;
    bool clamp;
    int anisotropy;
};

typedef std::map<std::string, GfxMaterialTexture> GfxMaterialTextureMap;
    
        
class GfxBaseMaterial {

    protected:

    GfxShader *shader;
    GfxShaderBindingsPtr bindings;
    GfxMaterialTextureMap textures;
    
    public:
    
    GfxBaseMaterial (const std::string name, GfxShader *shader);
    virtual ~GfxBaseMaterial (void) { }
    
    const std::string name;
    
    const GfxMaterialTextureMap &getTextures (void) const { return textures; } 
    void setTextures (const GfxMaterialTextureMap &v) { GFX_MAT_SYNC; textures = v; }

    void addDependencies (DiskResource *into) const;

    const GfxShaderBindingsPtr &getBindings (void) const { return bindings; }
    void setBindings (const GfxShaderBindingsPtr &v) { bindings = v; }
    
    GfxShader *getShader (void) const { return shader; }
    void setShader (GfxShader *v);
    
};

typedef std::map<std::string, GfxBaseMaterial*> GfxMaterialDB;

extern GfxMaterialDB material_db;


typedef std::vector<GfxMaterial*> GfxMaterials;

enum GfxMaterialSceneBlend { GFX_MATERIAL_OPAQUE, GFX_MATERIAL_ALPHA, GFX_MATERIAL_ALPHA_DEPTH };

enum GfxMaterialSpecularMode {
    GFX_MATERIAL_SPEC_NONE,
    GFX_MATERIAL_SPEC_ADJUSTED_DIFFUSE_COLOUR,
    GFX_MATERIAL_SPEC_DIFFUSE_ALPHA,
    GFX_MATERIAL_SPEC_MAP,
    GFX_MATERIAL_SPEC_MAP_WITH_GLOSS
};  


class GfxMaterial : public GfxBaseMaterial {
    public: // hack
    Ogre::MaterialPtr regularMat;     // no suffix
    Ogre::MaterialPtr fadingMat;      // ' can be NULL
    Ogre::MaterialPtr emissiveMat;
    //Ogre::MaterialPtr shadowMat;      // ! can be simply a link to the default
    Ogre::MaterialPtr worldMat;       // & 
    //Ogre::MaterialPtr worldShadowMat; // % can be simply a link to the default
    Ogre::MaterialPtr wireframeMat;     // |


    private: GfxMaterial (const std::string &name);

    private: GfxMaterialSceneBlend sceneBlend;
    public: GfxMaterialSceneBlend getSceneBlend (void) const { GFX_MAT_SYNC; return sceneBlend; }
    public: void setSceneBlend (GfxMaterialSceneBlend v);

    private: bool castShadows;
    public: bool getCastShadows (void) const { GFX_MAT_SYNC; return castShadows; }
    public: void setCastShadows (bool v);

    public:

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
