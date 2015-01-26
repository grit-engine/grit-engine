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
    fadingMat(nullptr),
    sceneBlend(GFX_MATERIAL_OPAQUE)
{
}

// FIXME:  certain updates to material properties need to be propagated to the GfxBodies that use
// them...

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

