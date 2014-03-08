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

#include <set>

#include "gfx_internal.h"
#include "gfx_material.h"
#include "gfx_body.h"


boost::recursive_mutex gfx_material_lock;

GfxMaterial::GfxMaterial (const std::string &name_)
  : fadingMat(NULL),
    alpha(1),
    sceneBlend(GFX_MATERIAL_OPAQUE),
    stipple(true),
    paintMode(GFX_MATERIAL_PAINT_NONE),
    paintByDiffuseAlpha(false),
    specularMode(GFX_MATERIAL_SPEC_NONE),
    numTextureBlends(1),

    name(name_)
{
}
    
void GfxMaterial::addDependencies (DiskResource *into)
{
    if (hasEmissiveMap()) into->addDependency(getEmissiveMap());
    if (hasPaintMap()) into->addDependency(getPaintMap());
    for (unsigned j=0 ; j<getNumTextureBlends() ; ++j) {
        GfxMaterialTextureBlendUnit &u = texBlends[j];
        if (hasDiffuseMap()) into->addDependency(u.getDiffuseMap());
        if (hasNormalMap()) into->addDependency(u.getNormalMap());
        if (hasSpecularMap()) into->addDependency(u.getSpecularMap());
    }

}

// FIXME: certain updates to material properties need to be propagated to the GfxBodies that use them...

void GfxMaterial::setSceneBlend (GfxMaterialSceneBlend v)
{
    GFX_MAT_SYNC;
    sceneBlend = v;
}

void GfxMaterial::setCastShadows (bool v)
{   
    GFX_MAT_SYNC;
    castShadows = v;
}       
        
void GfxMaterial::setStipple (bool v)
{   
    GFX_MAT_SYNC;
    stipple = v;
}       
        
void GfxMaterial::setEmissiveColour (const Vector3 &v)
{       
    GFX_MAT_SYNC;
    emissiveColour = v; 
    if (emissiveColour != Vector3(0,0,0) ) {
            std::string ename = name+std::string("^");
            emissiveMat = Ogre::MaterialManager::getSingleton().getByName(ename,"GRIT");
    } else {
            emissiveMat.setNull();
    }
}   
    
GfxMaterial *gfx_material_add (const std::string &name)
{
    GFX_MAT_SYNC;
    if (gfx_material_has_any(name)) GRIT_EXCEPT("Material already exists: \""+name+"\"");
    GfxMaterial *r = new GfxMaterial(name);
    material_db[name] = r;
    return r;
}

GfxMaterial *gfx_material_add_or_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (gfx_material_has_any(name)) {
        GfxMaterial *mat = dynamic_cast<GfxMaterial*>(material_db[name]);
        if (mat==NULL) GRIT_EXCEPT("Material already exists but is the wrong kind: \""+name+"\"");
        return mat;
    }    
    return gfx_material_add(name);
}

GfxMaterial *gfx_material_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (!gfx_material_has_any(name)) GRIT_EXCEPT("Material does not exist: \""+name+"\"");
    GfxMaterial *mat = dynamic_cast<GfxMaterial*>(material_db[name]);
    if (mat==NULL) GRIT_EXCEPT("Wrong kind of material: \""+name+"\"");
    return mat;
}


bool gfx_material_has (const std::string &name)
{
    GFX_MAT_SYNC;
    GfxMaterialDB::iterator it = material_db.find(name);
    if (it == material_db.end()) return false;
    return dynamic_cast<GfxMaterial*>(it->second) != NULL;
}

