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

#include <set>

#include "gfx_internal.h"
#include "GfxMaterial.h"
#include "GfxBody.h"


static std::set<GfxMaterial*> dirty_mats;
void handle_dirty_materials (void)
{
    if (dirty_mats.empty()) return;

    for (unsigned long i=0 ; i<gfx_all_bodies.size() ; ++i) {
        GfxBody *b = gfx_all_bodies[i];
        bool needs_update = false;
        for (unsigned j=0 ; j<b->getNumSubMeshes() ; ++j) {
            GfxMaterial *m = b->getMaterial(j);
            if (dirty_mats.find(m)!=dirty_mats.end()) {
                needs_update = true;
            }
        }
        if (needs_update) {
            b->updateProperties();
        }
    }
    
    dirty_mats.clear();
}

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
    
// FIXME: certain updates to material properties need to be propagated to the GfxBodies that use them...

void GfxMaterial::setSceneBlend (GfxMaterialSceneBlend v)
{
    GFX_MAT_SYNC;
    sceneBlend = v;
    dirty_mats.insert(this);
}

void GfxMaterial::setStipple (bool v)
{   
    GFX_MAT_SYNC;
    stipple = v;
    dirty_mats.insert(this);
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
    dirty_mats.insert(this);
}   
    
typedef std::map<std::string,GfxMaterial*> GfxMaterialDB;
static GfxMaterialDB material_db;

GfxMaterial *gfx_material_add (const std::string &name)
{
    GFX_MAT_SYNC;
    if (gfx_material_has(name)) GRIT_EXCEPT("Material already exists: \""+name+"\"");
    GfxMaterial *r = new GfxMaterial(name);
    material_db[name] = r;
    return r;
}

GfxMaterial *gfx_material_add_or_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (gfx_material_has(name)) return gfx_material_get(name);
    return gfx_material_add(name);
}

GfxMaterial *gfx_material_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (!gfx_material_has(name)) GRIT_EXCEPT("Material does not exist: \""+name+"\"");
    return material_db[name];
}

bool gfx_material_has (const std::string &name)
{
    GFX_MAT_SYNC;
    return material_db.find(name) != material_db.end();
}

void gfx_material_shutdown (void)
{
    material_db.clear();
}
