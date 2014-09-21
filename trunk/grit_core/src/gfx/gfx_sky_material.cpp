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

#include <string>
#include <map>

#include <OgreCgProgram.h>

#include "../centralised_log.h"

#include "gfx_internal.h"
#include "gfx_sky_material.h"
#include "gfx_sky_body.h"


GfxSkyShader *gfx_sky_shader_add (const std::string &name)
{
    if (gfx_sky_shader_has(name)) GRIT_EXCEPT("Shader already exists: \""+name+"\"");
    GfxSkyShader *r = new GfxSkyShader(name);
    shader_db[name] = r;
    return r;
}

GfxSkyShader *gfx_sky_shader_add_or_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (gfx_sky_shader_has(name)) {
        GfxSkyShader *mat = dynamic_cast<GfxSkyShader*>(shader_db[name]);
        return mat;
    }
    return gfx_sky_shader_add(name);
}

GfxSkyShader *gfx_sky_shader_get (const std::string &name)
{
    if (!gfx_sky_shader_has(name)) GRIT_EXCEPT("Sky shader does not exist: \""+name+"\"");
    GfxSkyShader *mat = dynamic_cast<GfxSkyShader*>(shader_db[name]);
    if (mat==NULL) GRIT_EXCEPT("Wrong kind of shader: \""+name+"\"");
    return mat;
}

bool gfx_sky_shader_has (const std::string &name)
{
    GfxShaderDB::iterator it = shader_db.find(name);
    if (it == shader_db.end()) return false;
    return dynamic_cast<GfxSkyShader*>(it->second) != NULL;
}

GfxSkyShader::GfxSkyShader (const std::string &name)
  : name(name)
{
    vp.setNull();
    fp.setNull();
}

std::set<std::string> identifiers (const std::string &code, const std::string &prefix)
{       
    std::set<std::string> r;
    std::string token;
    for (unsigned i=0 ; i<code.length() ; ++i) {
        char c = code[i];
        if (    
            (c>='A' && c<='Z')
                ||
            (c>='a' && c<='z')
                ||
            (c>='0' && c<='9' && token.size() != 0) // first char cannot be a number
                ||
            (c=='_')
        ) {     
            token += c;
        } else if (token.size() != 0) {
            if (token.substr(0, prefix.size()) == prefix) {
                std::string suffix = token.substr(prefix.size());
                r.insert(suffix);
            }
            token.clear();
        }
    }
    if (token.size() != 0) {
        if (token.substr(0, prefix.size()) == prefix) {
            std::string suffix = token.substr(prefix.size());
            r.insert(suffix);
        }
    }
    return r;
}

void GfxSkyShader::reset (const std::string &new_vertex_code,
                          const std::string &new_fragment_code,
                          const std::vector<GfxSkyShaderVariation> &new_variations,
                          const GfxSkyShaderUniformMap &new_uniforms)
{
    GfxGslBackend backend = gfx_d3d9() ? GFX_GSL_BACKEND_CG : GFX_GSL_BACKEND_GLSL;
    GfxGslParams params;
    for (const auto &u : new_uniforms) {
        params[u.first] = u.second.kind;
    }
    // TODO: actually generate more than one shader
    GfxGslUnboundTextures ubt;
    auto shaders = gfx_gasoline_compile(backend, new_vertex_code, new_fragment_code,
                                        params, ubt);

    // No errors should be possible beyond this point.

    vertexCode = new_vertex_code;
    fragmentCode = new_fragment_code;
    variations = new_variations;
    uniforms = new_uniforms;

    // Set up Ogre objects
    if (backend == GFX_GSL_BACKEND_CG) {

        Ogre::StringVector vp_profs, fp_profs;
        if (gfx_d3d9()) {
            vp_profs.push_back("vs_3_0");
            fp_profs.push_back("ps_3_0");
        } else {
            vp_profs.push_back("gpu_vp");
            fp_profs.push_back("gp4fp");
        }
        if (vp.isNull())
            vp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                name+"_v", RESGRP, "cg", Ogre::GPT_VERTEX_PROGRAM);
        APP_ASSERT(!vp.isNull());
        Ogre::CgProgram *tmp_vp = static_cast<Ogre::CgProgram*>(&*vp);
        vp->setSource(shaders.first);
        tmp_vp->setEntryPoint("main");
        tmp_vp->setProfiles(vp_profs);
        tmp_vp->setCompileArguments("-I. -O3");
        vp->unload();
        vp->load();

        if (fp.isNull())
            fp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                name+"_f", RESGRP, "cg", Ogre::GPT_FRAGMENT_PROGRAM);
        APP_ASSERT(!fp.isNull());
        Ogre::CgProgram *tmp_fp = static_cast<Ogre::CgProgram*>(&*fp);
        fp->setSource(shaders.second);
        tmp_fp->setEntryPoint("main");
        tmp_fp->setProfiles(fp_profs);
        tmp_fp->setCompileArguments("-I. -O3");
        fp->unload();
        fp->load();

        load_and_validate_shader(vp);
        load_and_validate_shader(fp);

    } else {
        if (vp.isNull())
            vp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                name+"_v", RESGRP, "glsl", Ogre::GPT_VERTEX_PROGRAM);
        APP_ASSERT(!vp.isNull());
        vp->setSource(shaders.first);
        vp->unload();
        vp->load();

        if (fp.isNull())
            fp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                name+"_f", RESGRP, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
        APP_ASSERT(!fp.isNull());
        fp->setSource(shaders.second);
        fp->unload();
        fp->load();

        load_and_validate_shader(vp);
        load_and_validate_shader(fp);
    }
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
  : shader(gfx_sky_shader_get("/system/SkyDefault")),
    sceneBlend(GFX_SKY_MATERIAL_OPAQUE),
    name(name_)
{
}

void GfxSkyMaterial::addDependencies (DiskResource *into)
{
    GFX_MAT_SYNC;
    // iterate through textures, add them as dependencies
    for (const auto &i : uniforms) {
        if (gfx_gasoline_param_is_texture(i.second.kind))
            into->addDependency(i.second.texture);
    }
}
