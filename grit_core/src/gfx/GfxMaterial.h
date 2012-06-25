/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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
struct GfxPaintColour;

#ifndef GFX_MATERIAL_H
#define GFX_MATERIAL_H

#include "gfx.h"

#include <OgreMaterial.h>

typedef std::vector<GfxMaterial*> GfxMaterials;

enum GfxMaterialSceneBlend { GFX_MATERIAL_OPAQUE, GFX_MATERIAL_ALPHA, GFX_MATERIAL_ALPHA_DEPTH };

enum GfxMaterialSpecularMode {
    GFX_MATERIAL_SPEC_NONE,
    GFX_MATERIAL_SPEC_ADJUSTED_DIFFUSE_COLOUR,
    GFX_MATERIAL_SPEC_DIFFUSE_ALPHA,
    GFX_MATERIAL_SPEC_MAP,
    GFX_MATERIAL_SPEC_MAP_WITH_GLOSS
};  

extern boost::recursive_mutex gfx_material_lock;
#define GFX_MAT_SYNC boost::recursive_mutex::scoped_lock _scoped_lock(gfx_material_lock)


// textures and texture settings
class GfxMaterialTextureBlendUnit {

    public:
    
    GfxMaterialTextureBlendUnit (void)
      : specularDiffuseBrightness(0), specularDiffuseContrast(0), specularDiffuseSaturation(0),
        textureAnimationX(1), textureAnimationY(1),
        textureScaleX(1), textureScaleY(1)
    {   
     
    }   
    
    private: std::string diffuseMap;
    public: std::string getDiffuseMap (void) const { GFX_MAT_SYNC; return diffuseMap; }
    public: void setDiffuseMap (const std::string &v) { GFX_MAT_SYNC; diffuseMap = v; }
    
    private: std::string normalMap;
    public: std::string getNormalMap (void) const { GFX_MAT_SYNC; return normalMap; }
    public: void setNormalMap (const std::string &v) { GFX_MAT_SYNC; normalMap = v; }
    
    private: std::string specularMap;
    public: std::string getSpecularMap (void) const { GFX_MAT_SYNC; return specularMap; }
    public: void setSpecularMap (const std::string &v) { GFX_MAT_SYNC; specularMap = v; }
    
    private: float specularDiffuseBrightness;
    public: float getSpecularDiffuseBrightness (void) const { GFX_MAT_SYNC; return specularDiffuseBrightness; }
    public: void setSpecularDiffuseBrightness (float v) { GFX_MAT_SYNC; specularDiffuseBrightness = v; }
    
    private: float specularDiffuseContrast;
    public: float getSpecularDiffuseContrast (void) const { GFX_MAT_SYNC; return specularDiffuseContrast; }
    public: void setSpecularDiffuseContrast (float v) { GFX_MAT_SYNC; specularDiffuseContrast = v; }
    
    private: float specularDiffuseSaturation;
    public: float getSpecularDiffuseSaturation (void) const { GFX_MAT_SYNC; return specularDiffuseSaturation; }
    public: void setSpecularDiffuseSaturation (float v) { GFX_MAT_SYNC; specularDiffuseSaturation = v; }
    
    private: float textureAnimationX;
    public: float getTextureAnimationX (void) const { GFX_MAT_SYNC; return textureAnimationX; }
    public: void setTextureAnimationX (float v) { GFX_MAT_SYNC; textureAnimationX = v; }

    private: float textureAnimationY;
    public: float getTextureAnimationY (void) const { GFX_MAT_SYNC; return textureAnimationY; }
    public: void setTextureAnimationY (float v) { GFX_MAT_SYNC; textureAnimationY = v; }

    private: float textureScaleX;
    public: float getTextureScaleX (void) const { GFX_MAT_SYNC; return textureScaleX; }
    public: void setTextureScaleX (float v) { GFX_MAT_SYNC; textureScaleX = v; }

    private: float textureScaleY;
    public: float getTextureScaleY (void) const { GFX_MAT_SYNC; return textureScaleY; }
    public: void setTextureScaleY (float v) { GFX_MAT_SYNC; textureScaleY = v; }

    public:

    bool hasDiffuseMap (void) { GFX_MAT_SYNC; return diffuseMap.length() > 0; }

    bool hasNormalMap (void) { GFX_MAT_SYNC; return normalMap.length() > 0; }

};

enum GfxMaterialPaintMode {
    GFX_MATERIAL_PAINT_NONE,
    GFX_MATERIAL_PAINT_1,
    GFX_MATERIAL_PAINT_2,
    GFX_MATERIAL_PAINT_3,
    GFX_MATERIAL_PAINT_4,
    GFX_MATERIAL_PAINT_MAP
};

class GfxMaterial {
    public: // hack
    Ogre::MaterialPtr regularMat;     // no suffix
    Ogre::MaterialPtr fadingMat;      // ' can be NULL
    Ogre::MaterialPtr emissiveMat;
    //Ogre::MaterialPtr shadowMat;      // ! can be simply a link to the default
    //Ogre::MaterialPtr worldMat;       // & 
    //Ogre::MaterialPtr worldShadowMat; // % can be simply a link to the default


    private: GfxMaterial (const std::string &name);

    private: float alpha;
    public: float getAlpha (void) const { GFX_MAT_SYNC; return alpha; }
    public: void setAlpha (float v) { GFX_MAT_SYNC; alpha = v; }

    private: GfxMaterialSceneBlend sceneBlend;
    public: GfxMaterialSceneBlend getSceneBlend (void) const { GFX_MAT_SYNC; return sceneBlend; }
    public: void setSceneBlend (GfxMaterialSceneBlend v);

    private: bool stipple;
    public: bool getStipple (void) const { GFX_MAT_SYNC; return stipple; }
    public: void setStipple (bool v);

    private: Vector3 emissiveColour;
    public: Vector3 getEmissiveColour (void) const { GFX_MAT_SYNC; return emissiveColour; }
    public: void setEmissiveColour (const Vector3 &v);


    private: std::string emissiveMap;
    public: std::string getEmissiveMap (void) const { GFX_MAT_SYNC; return emissiveMap; }
    public: void setEmissiveMap (const std::string &v) { GFX_MAT_SYNC; emissiveMap = v; }

    private: std::string paintMap;
    public: std::string getPaintMap (void) const { GFX_MAT_SYNC; return paintMap; }
    public: void setPaintMap (const std::string &v) { GFX_MAT_SYNC; paintMap = v; }

    private: GfxMaterialPaintMode paintMode;
    public: GfxMaterialPaintMode getPaintMode (void) const { GFX_MAT_SYNC; return paintMode; }
    public: void setPaintMode (const GfxMaterialPaintMode &v) { GFX_MAT_SYNC; paintMode = v; }

    private: bool paintByDiffuseAlpha;
    public: bool getPaintByDiffuseAlpha (void) const { GFX_MAT_SYNC; return paintByDiffuseAlpha; }
    public: void setPaintByDiffuseAlpha (const bool &v) { GFX_MAT_SYNC; paintByDiffuseAlpha = v; }

    private: GfxMaterialSpecularMode specularMode;
    public: GfxMaterialSpecularMode getSpecularMode (void) const { GFX_MAT_SYNC; return specularMode; }
    public: void setSpecularMode (const GfxMaterialSpecularMode &v) { GFX_MAT_SYNC; specularMode = v; }

    private: unsigned numTextureBlends;
    public: unsigned getNumTextureBlends (void) const { GFX_MAT_SYNC; return numTextureBlends; }
    public: void setNumTextureBlends (const unsigned &v) { GFX_MAT_SYNC; numTextureBlends = v; }

    public: GfxMaterialTextureBlendUnit texBlends[4];

    public:

    bool hasDiffuseMap() {
        GFX_MAT_SYNC;
        for (unsigned i=0 ; i<numTextureBlends ; ++i) {
            if (texBlends[i].hasDiffuseMap()) return true;
        }
        return false;
    }

    bool hasNormalMap() {
        GFX_MAT_SYNC;
        for (unsigned i=0 ; i<numTextureBlends ; ++i) {
            if (texBlends[i].hasNormalMap()) return true;
        }
        return false;
    }

    bool hasSpecularMap (void) {
        GFX_MAT_SYNC;
        return specularMode == GFX_MATERIAL_SPEC_MAP || specularMode == GFX_MATERIAL_SPEC_MAP_WITH_GLOSS;
    }

    bool hasEmissiveMap() {
        GFX_MAT_SYNC;
        return emissiveMap.length() > 0;
    }

    bool hasPaintMap() {
        GFX_MAT_SYNC;
        return paintMode == GFX_MATERIAL_PAINT_MAP;
    }

    const std::string name;

    friend GfxMaterial *gfx_material_add(const std::string &);
    friend class GfxBody;
};

GfxMaterial *gfx_material_add (const std::string &name);

GfxMaterial *gfx_material_add_or_get (const std::string &name);

GfxMaterial *gfx_material_get (const std::string &name);

bool gfx_material_has (const std::string &name);

struct GfxPaintColour {
    Vector3 diff, spec;
    float met; // metallic paint (0 -> 1)
};

void gfx_material_shutdown (void);

#endif
