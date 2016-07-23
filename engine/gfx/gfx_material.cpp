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

#include <set>

#include "gfx_internal.h"
#include "gfx_material.h"
#include "gfx_body.h"
#include "gfx_shader.h"

// Global lock, see header for documentation.
std::recursive_mutex gfx_material_lock;


GfxBaseMaterial::GfxBaseMaterial(const std::string name, GfxShader *shader)
      : shader(shader),
        bindings(),
        name(name)
{ }

void GfxBaseMaterial::addDependencies (DiskResource *into) const
{   
    GFX_MAT_SYNC;
    for (const auto &i : textures) {
        into->addDependency(i.second.texture);
    }
}

void GfxBaseMaterial::setShader (GfxShader *v)
{
    GFX_MAT_SYNC;
    shader = v;
    bindings.clear();
    textures.clear();
}

GfxMaterial::GfxMaterial (const std::string &name)
  : GfxBaseMaterial(name, gfx_shader_get("/system/Default")),
    fadingMat(static_cast<Ogre::Material*>(nullptr)),
    sceneBlend(GFX_MATERIAL_OPAQUE),
    castShadows(true),
    additionalLighting(false),
    boneBlendWeights(0)
{
}

void GfxMaterial::setSceneBlend (GfxMaterialSceneBlend v)
{
    sceneBlend = v;
}

void GfxMaterial::setCastShadows (bool v)
{   
    castShadows = v;
}       
        
void GfxMaterial::setAdditionalLighting (bool v)
{   
    additionalLighting = v;
}       
        
void GfxMaterial::setBoneBlendWeights (unsigned v)
{
    boneBlendWeights = v;
}


static Ogre::Pass *create_or_reset_material (const std::string &name)
{
    auto &mm = Ogre::MaterialManager::getSingleton();
    Ogre::MaterialPtr m = mm.createOrRetrieve(name, RESGRP).first.staticCast<Ogre::Material>();
    m->removeAllTechniques();
    Ogre::Technique *t = m->createTechnique();
    return t->createPass();
}

void GfxMaterial::rebuildOgreMaterials (const GfxShaderGlobals &globs)
{
    GFX_MAT_SYNC;
    Ogre::Pass *p;
    bool fade_dither = sceneBlend == GFX_MATERIAL_OPAQUE;
    bool instanced = false;  // TODO: Should be a material for both possibilities

    p = create_or_reset_material(name + ":wireframe");
    shader->bindShaderPass(p, GfxShader::WIREFRAME, fade_dither, instanced, boneBlendWeights,
                           globs, textures, bindings);
    p->setCullingMode(Ogre::CULL_NONE);
    p->setPolygonMode(Ogre::PM_WIREFRAME);
    p->setDepthWriteEnabled(false);
    p->setDepthBias(0, 0);
    wireframeMat = Ogre::MaterialManager::getSingleton().getByName(name + ":wireframe", "GRIT");

    forwardMat = Ogre::MaterialManager::getSingleton().getByName(name, "GRIT");
    fadingMat = forwardMat;
    worldMat = Ogre::MaterialManager::getSingleton().getByName(name + "&", "GRIT");

    p = create_or_reset_material(name + ":additional");
    shader->bindShaderPass(p, GfxShader::ADDITIONAL, false, instanced, boneBlendWeights,
                           globs, textures, bindings);
    p->setDepthWriteEnabled(false);
    // TODO(dcunnin): hack as we get z fighting and i don't know why.  Perhaps because of different
    // rounding error between old shader and gsl?  But code looks the same.
    p->setDepthBias(0, 0);
    p->setDepthFunction(Ogre::CMPF_LESS_EQUAL);
    p->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);

    additionalMat = Ogre::MaterialManager::getSingleton().getByName(name + ":additional", "GRIT");

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
        if (mat == NULL) GRIT_EXCEPT("Material already exists but is the wrong kind: \""+name+"\"");
        return mat;
    }    
    return gfx_material_add(name);
}

GfxMaterial *gfx_material_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (!gfx_material_has_any(name)) GRIT_EXCEPT("Material does not exist: \""+name+"\"");
    GfxMaterial *mat = dynamic_cast<GfxMaterial*>(material_db[name]);
    if (mat == NULL) GRIT_EXCEPT("Wrong kind of material: \""+name+"\"");
    return mat;
}


bool gfx_material_has (const std::string &name)
{
    GFX_MAT_SYNC;
    GfxMaterialDB::iterator it = material_db.find(name);
    if (it == material_db.end()) return false;
    return dynamic_cast<GfxMaterial*>(it->second) != NULL;
}

void gfx_material_init (void)
{
    std::string vs =
        "out.position = transform_to_world(vert.position.xyz);\n"
        "var normal_ws = rotate_to_world(vert.normal.xyz);\n";
    std::string fs = "out.normal = normal_ws;";
    std::string as = "";
                     
    gfx_shader_make_or_reset("/system/Default", vs, fs, as, { }); 
    gfx_material_add("/system/Default");
}   

