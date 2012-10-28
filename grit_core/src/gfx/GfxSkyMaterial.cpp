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

#include "../CentralisedLog.h"

#include "gfx_internal.h"
#include "GfxSkyMaterial.h"
#include "GfxSkyBody.h"


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

typedef std::set<std::string>::iterator SI;

/*
static std::ostream &operator<< (std::ostream &o, std::set<std::string> s)
{
    bool first = true;;
    for (SI i=s.begin(), i_=s.end() ; i!=i_ ; ++i) {
        o << (first ? "[ " : ", ");
        o << *i;
        first = false;
    }
    if (!first) o << " ";
    o << "]";
    return o;
}
*/

std::string compile_grit_to_cg (const std::string &grit_code,
                                const std::vector<GfxSkyShaderVariation> &new_variations,
                                const GfxSkyShaderUniformMap &new_uniforms)
{
    (void) new_variations;
    // this is where we put functions that return defualt texture colour/alphas
    std::string preludes;
    preludes += "#include <system/uber.cgh>\n";


    std::set<std::string> in = identifiers(grit_code, "in_");
    std::string in_code;
    for (SI i=in.begin(), i_=in.end() ; i!=i_ ; ++i) {
        // TODO: get types right here
        in_code += "    in float4 in_"+*i+": "+*i+",\n";
    }

    std::string scene_uniforms_code;
    scene_uniforms_code += "    uniform float4x4 su_worldViewProj,\n";
    scene_uniforms_code += "    uniform float4x4 su_worldView,\n";
    scene_uniforms_code += "    uniform float4x4 su_viewProj,\n";
    scene_uniforms_code += "    uniform float4x4 su_world,\n";
    scene_uniforms_code += "    uniform float4x4 su_view,\n";
    scene_uniforms_code += "    uniform float4x4 su_proj,\n";
    scene_uniforms_code += "    uniform float4 su_viewportSize,\n";
    scene_uniforms_code += "    uniform float su_fovY,\n";
    scene_uniforms_code += "    uniform float su_time,\n";
    scene_uniforms_code += "    uniform float3 su_particleAmbient,\n";
    scene_uniforms_code += "    uniform float3 su_sunlightDiffuse,\n";
    scene_uniforms_code += "    uniform float3 su_sunlightSpecular,\n";
    scene_uniforms_code += "    uniform float3 su_sunlightDirection,\n";
    scene_uniforms_code += "    uniform float3 su_fogColour,\n";
    scene_uniforms_code += "    uniform float su_fogDensity,\n";
    scene_uniforms_code += "    uniform float3 su_sunDirection,\n";
    scene_uniforms_code += "    uniform float3 su_sunColour,\n";
    scene_uniforms_code += "    uniform float su_sunSize,\n";
    scene_uniforms_code += "    uniform float su_sunFalloffDistance,\n";
    scene_uniforms_code += "    uniform float su_skyGlareSunDistance,\n";
    scene_uniforms_code += "    uniform float su_skyGlareHorizonElevation,\n";
    scene_uniforms_code += "    uniform float3 su_skyCloudColour,\n";
    scene_uniforms_code += "    uniform float su_skyCloudCoverage,\n";
    scene_uniforms_code += "    uniform float3 su_hellColour,\n";
    scene_uniforms_code += "    uniform float su_skyDivider1,\n";
    scene_uniforms_code += "    uniform float su_skyDivider2,\n";
    scene_uniforms_code += "    uniform float su_skyDivider3,\n";
    scene_uniforms_code += "    uniform float su_skyDivider4,\n";
    scene_uniforms_code += "    uniform float3 su_skyColour0,\n";
    scene_uniforms_code += "    uniform float3 su_skyColour1,\n";
    scene_uniforms_code += "    uniform float3 su_skyColour2,\n";
    scene_uniforms_code += "    uniform float3 su_skyColour3,\n";
    scene_uniforms_code += "    uniform float3 su_skyColour4,\n";
    scene_uniforms_code += "    uniform float3 su_skyColour5,\n";
    scene_uniforms_code += "    uniform float3 su_skySunColour0,\n";
    scene_uniforms_code += "    uniform float3 su_skySunColour1,\n";
    scene_uniforms_code += "    uniform float3 su_skySunColour2,\n";
    scene_uniforms_code += "    uniform float3 su_skySunColour3,\n";
    scene_uniforms_code += "    uniform float3 su_skySunColour4,\n";

    std::string material_uniforms_code;
    std::string tex_substitute_code;
    unsigned counter = 0;
    for (GfxSkyShaderUniformMap::const_iterator i=new_uniforms.begin(),i_=new_uniforms.end() ; i!=i_ ; ++i) {
        const std::string &name = i->first;
        const GfxSkyShaderUniform &u = i->second;
        switch (u.kind) {
            case GFX_SHADER_UNIFORM_KIND_PARAM_TEXTURE2D:
            if (true) {
                std::stringstream extra;
                extra << " : register(s"<<counter<<")";
                counter++;
                material_uniforms_code += "    uniform sampler2D mu_"+name+extra.str()+",\n";
            } else {
                std::stringstream extra;
                extra<<"float4("<<u.defaultColour.x<<", "<<u.defaultColour.y<<", "<<u.defaultColour.z<<", "<<u.defaultAlpha<<")";
                tex_substitute_code += "    float4 mu_"+name+" = "+extra.str()+";\n";
            }
            break;
            case GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT1:
            material_uniforms_code += "    uniform float mu_"+name+",\n";
            break;
            //case GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT2: type="float2"; break;
            case GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT3:
            material_uniforms_code += "    uniform float3 mu_"+name+",\n";
            break;
            case GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT4:
            material_uniforms_code += "    uniform float4 mu_"+name+",\n";
            break;
        }
        
    }

    std::string out_code;
    std::set<std::string> out = identifiers(grit_code, "out_");
    for (SI i=out.begin(), i_=out.end() ; i!=i_ ; ++i) {
        // TODO: get types right here
        out_code += "    out float4 out_"+*i+": "+*i+",\n";
    }

    std::string compiled_code;
    compiled_code += preludes;
    compiled_code += "void main (\n";
    compiled_code += in_code;
    compiled_code += scene_uniforms_code;
    compiled_code += material_uniforms_code;
    compiled_code += out_code;
    compiled_code += "    uniform float _UNUSED\n"; // deal with trailing comma
    compiled_code += ") {\n";
    compiled_code += tex_substitute_code;
    compiled_code += grit_code+"\n";
    compiled_code += "}\n";

    //CVERB << compiled_code << std::endl;

    return compiled_code;
}


// TODO:
// need to compile the shader code into an actual cg program
// need to use the generated shader to render
void GfxSkyShader::reset (const std::string &new_vertex_code,
                          const std::string &new_fragment_code,
                          const std::vector<GfxSkyShaderVariation> &new_variations,
                          const GfxSkyShaderUniformMap &new_uniforms)
{
    // PRELUDE
    Ogre::StringVector vp_profs, fp_profs;
    vp_profs.push_back("vs_3_0"); fp_profs.push_back("ps_3_0"); // d3d9
    vp_profs.push_back("gpu_vp"); fp_profs.push_back("gp4fp"); // gl

    vertexCode = new_vertex_code;
    fragmentCode = new_fragment_code;
    variations = new_variations;
    uniforms = new_uniforms;

    // FIXME: should error check the incoming code, so that if we get to this point, the following calls will not fail

    // COMPILE PHASE
    std::string compiled_vertex_code = compile_grit_to_cg(new_vertex_code, new_variations, new_uniforms);

    std::string compiled_fragment_code = compile_grit_to_cg(new_fragment_code, new_variations, new_uniforms);

    // Set up Ogre objects
    if (vp.isNull())
        vp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(name+"_v", RESGRP, "cg", Ogre::GPT_VERTEX_PROGRAM);
    APP_ASSERT(!vp.isNull());
    Ogre::CgProgram *tmp_vp = static_cast<Ogre::CgProgram*>(&*vp);
    vp->setSource(compiled_vertex_code);
    tmp_vp->setEntryPoint("main");
    tmp_vp->setProfiles(vp_profs);
    tmp_vp->setCompileArguments("-I. -O3");
    vp->unload();
    vp->load();

    if (fp.isNull())
        fp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(name+"_f", RESGRP, "cg", Ogre::GPT_FRAGMENT_PROGRAM);
    APP_ASSERT(!fp.isNull());
    Ogre::CgProgram *tmp_fp = static_cast<Ogre::CgProgram*>(&*fp);
    fp->setSource(compiled_fragment_code);
    tmp_fp->setEntryPoint("main");
    tmp_fp->setProfiles(fp_profs);
    tmp_fp->setCompileArguments("-I. -O3");
    fp->unload();
    fp->load();


    load_and_validate_shader(vp);
    load_and_validate_shader(fp);
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

/*
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

    p->setVertexProgram(shader->getVP()->getName());
    p->setFragmentProgram(shader->getFP()->getName());

    if (shader->name=="/system/SkyProgram") {

        Ogre::TextureUnitState *t;

        t = p->createTextureUnitState();
        t->setTextureName("system/starfield.dds");
        t->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
        t->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_ANISOTROPIC);

        t = p->createTextureUnitState();
        t->setTextureName("system/PerlinNoise.bmp");

        t = p->createTextureUnitState();
        t->setTextureName("system/PerlinNoiseN.bmp");

    } else if (shader->name=="/system/SkyClouds") {

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

        if (hasEmissiveMap()) {
            Ogre::TextureUnitState *t = p->createTextureUnitState();
            t->setTextureName(emissiveMap.substr(1));
            t->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
            t->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_ANISOTROPIC);
        }
        
        try_set_named_constant(p->getFragmentProgramParameters(), "mu_alphaRejectThreshold", alphaRejectThreshold);
        try_set_named_constant(p->getFragmentProgramParameters(), "mu_alpha", alpha);
        try_set_named_constant(p->getFragmentProgramParameters(), "mu_emissiveMask", to_ogre(emissiveColour));
    }
}
*/

    
void GfxSkyMaterial::addDependencies (GfxDiskResource *into)
{
    GFX_MAT_SYNC;
    // iterate through textures, add them as dependencies
    for (GfxSkyMaterialUniformMap::iterator i=uniforms.begin(),i_=uniforms.end() ; i!=i_ ; ++i) {
        if (i->second.kind != GFX_SHADER_UNIFORM_KIND_PARAM_TEXTURE2D) continue;
        into->addDependency(i->second.texture);
    }
}   
