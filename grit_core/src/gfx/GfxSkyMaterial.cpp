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

#include <string>
#include <map>

#include "../CentralisedLog.h"

#include "gfx_internal.h"
#include "GfxSkyMaterial.h"
#include "GfxSkyBody.h"

static std::set<GfxSkyMaterial*> dirty_sky_mats;
void handle_dirty_sky_materials (void)
{   
    if (dirty_sky_mats.empty()) return;

    for (unsigned long i=0 ; i<gfx_all_sky_bodies.size() ; ++i) {
        GfxSkyBody *b = gfx_all_sky_bodies[i];
        bool needs_update = false;
        for (unsigned j=0 ; j<b->getNumSubMeshes() ; ++j) {
            GfxSkyMaterial *m = b->getMaterial(j);
            if (dirty_sky_mats.find(m)!=dirty_sky_mats.end()) {
                needs_update = true;
            }
        }
        if (needs_update) {
            b->updateProperties();
        }
    }
    
    dirty_sky_mats.clear();
}   
    

GfxSkyMaterial *gfx_sky_material_add (const std::string &name)
{
    GFX_MAT_SYNC;
    if (gfx_sky_material_has(name)) GRIT_EXCEPT("Material already exists: \""+name+"\"");
    GfxSkyMaterial *r = new GfxSkyMaterial(name);
    material_db[name] = r;
    return r;
}

GfxSkyMaterial *gfx_sky_material_add_or_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (gfx_material_has_any(name)) {
        GfxSkyMaterial *mat = dynamic_cast<GfxSkyMaterial*>(material_db[name]);
        if (mat==NULL) GRIT_EXCEPT("Material already exists but is the wrong kind: \""+name+"\"");
        return mat;
    }
    return gfx_sky_material_add(name);
}

GfxSkyMaterial *gfx_sky_material_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (!gfx_sky_material_has(name)) GRIT_EXCEPT("Sky material does not exist: \""+name+"\"");
    GfxSkyMaterial *mat = dynamic_cast<GfxSkyMaterial*>(material_db[name]);
    if (mat==NULL) GRIT_EXCEPT("Wrong kind of material: \""+name+"\"");
    return mat;
}

bool gfx_sky_material_has (const std::string &name)
{
    GFX_MAT_SYNC;
    GfxMaterialDB::iterator it = material_db.find(name);
    if (it == material_db.end()) return false;
    return dynamic_cast<GfxSkyMaterial*>(it->second) != NULL;
}

GfxSkyMaterial::GfxSkyMaterial (const std::string &name_)
  : special(false),
    alpha(1),
    alphaRejectThreshold(0.5),
    emissiveColour(Vector3(1,1,1)),
    sceneBlend(GFX_SKY_MATERIAL_OPAQUE),
    name(name_)
{
    mat = Ogre::MaterialManager::getSingleton().create("SKY:"+name, RESGRP);
    updateInternalMat();
}

template<class T> void try_set_named_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                                               const char *name, const T &v)
{
    try {
        p->setNamedConstant(name,v);
    } catch (const Ogre::Exception &e) {
        if (e.getNumber() != Ogre::Exception::ERR_INVALIDPARAMS) {
            throw e;
        } else {
            // silently ignore
        }
    }
}   

void GfxSkyMaterial::updateInternalMat (void)
{
    Ogre::Pass *p = mat->getTechnique(0)->getPass(0);
    p->setPolygonModeOverrideable(false);
    p->removeAllTextureUnitStates();

    p->setDepthWriteEnabled(false);
    switch (sceneBlend) {
        case GFX_SKY_MATERIAL_OPAQUE:
        p->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
        break;
        case GFX_SKY_MATERIAL_ALPHA:
        p->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
        break;
        case GFX_SKY_MATERIAL_ADD:
        p->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);
        break;
    }

    if (special) {
        p->setVertexProgram("/system/SkyProgram_v");
        p->setFragmentProgram("/system/SkyProgram_f");

        Ogre::TextureUnitState *t;

        t = p->createTextureUnitState();
        t->setTextureName("system/starfield.dds");
        t->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
        t->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_ANISOTROPIC);

        t = p->createTextureUnitState();
        t->setTextureName("system/PerlinNoise.bmp");

        t = p->createTextureUnitState();
        t->setTextureName("system/PerlinNoiseN.bmp");

    } else {
        p->setVertexProgram("/system/SkyDefault_v");
        p->setFragmentProgram("/system/SkyDefault_f");

        if (hasEmissiveMap()) {
            Ogre::TextureUnitState *t = p->createTextureUnitState();
            t->setTextureName(emissiveMap.substr(1));
            t->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
            t->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_ANISOTROPIC);
        }
        
        try_set_named_constant(p->getFragmentProgramParameters(), "alpha_reject_threshold", alphaRejectThreshold);
        try_set_named_constant(p->getFragmentProgramParameters(), "alpha", alpha);
        try_set_named_constant(p->getFragmentProgramParameters(), "emissive_colour", to_ogre(emissiveColour));
    }
}

    
void GfxSkyMaterial::setSceneBlend (GfxSkyMaterialSceneBlend v)
{
    GFX_MAT_SYNC;
    sceneBlend = v;

    updateInternalMat();

    // update any entities that need to be
    dirty_sky_mats.insert(this);
}

void GfxSkyMaterial::addDependencies (GfxDiskResource *into)
{
    if (hasEmissiveMap()) into->addDependency(getEmissiveMap());
    
}   
