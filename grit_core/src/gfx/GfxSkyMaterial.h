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

class GfxSkyMaterial;
typedef std::vector<GfxSkyMaterial*> GfxSkyMaterials;

#ifndef GFX_SKY_MATERIAL_H
#define GFX_SKY_MATERIAL_H

#include <OgreMaterial.h>

#include "gfx_internal.h"

enum GfxSkyMaterialSceneBlend { GFX_SKY_MATERIAL_OPAQUE, GFX_SKY_MATERIAL_ALPHA, GFX_SKY_MATERIAL_ADD };
            
class GfxSkyMaterial : public GfxBaseMaterial {

    Ogre::MaterialPtr mat; // internal ogre material

    void updateInternalMat (void);
        
    GfxSkyMaterial (const std::string &name);
    
    private: bool special;
    public: bool getSpecial (void) const { GFX_MAT_SYNC; return special; }
    public: void setSpecial (bool v) { GFX_MAT_SYNC; special = v; updateInternalMat(); }

    private: float alpha;
    public: float getAlpha (void) const { GFX_MAT_SYNC; return alpha; }
    public: void setAlpha (float v) { GFX_MAT_SYNC; alpha = v; updateInternalMat(); }

    private: float alphaRejectThreshold;
    public: float getAlphaRejectThreshold (void) const { GFX_MAT_SYNC; return alphaRejectThreshold; }
    public: void setAlphaRejectThreshold (float v) { GFX_MAT_SYNC; alphaRejectThreshold = v; updateInternalMat(); }
    
    private: std::string emissiveMap;
    public: std::string getEmissiveMap (void) const { GFX_MAT_SYNC; return emissiveMap; }
    public: void setEmissiveMap (const std::string &v) { GFX_MAT_SYNC; emissiveMap = v; updateInternalMat(); }

    private: Vector3 emissiveColour;
    public: Vector3 getEmissiveColour (void) const { GFX_MAT_SYNC; return emissiveColour; }
    public: void setEmissiveColour (const Vector3 &v) { GFX_MAT_SYNC; emissiveColour = v; updateInternalMat(); }

    private: GfxSkyMaterialSceneBlend sceneBlend;
    public: GfxSkyMaterialSceneBlend getSceneBlend (void) const { GFX_MAT_SYNC; return sceneBlend; }
    public: void setSceneBlend (GfxSkyMaterialSceneBlend v);

    public:

    bool hasEmissiveMap() { return emissiveMap.length() > 0; }

    void addDependencies (GfxDiskResource *into);

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
