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

#include <centralised_log.h>

#include "gfx.h"
#include "gfx_gasoline.h"
#include "gfx_internal.h"
#include "gfx_shader.h"

GfxGslBackend backend = (gfx_d3d9() || getenv("GRIT_GL_CG") != nullptr)
                      ? GFX_GSL_BACKEND_CG : GFX_GSL_BACKEND_GLSL;

static const std::string dump_shader(getenv("GRIT_DUMP_SHADER") == nullptr
                                     ? "" : getenv("GRIT_DUMP_SHADER"));

typedef GfxShader::NativePair NativePair;

static std::ostream &operator << (std::ostream &o, const GfxShader::Split &s)
{
    o << s.env;
    return o;
}

static std::string fresh_name (void)
{
    static int counter = 0;
    std::stringstream ss;
    ss << "Gen:" << counter++;
    return ss.str();
}

GfxShaderGlobals gfx_shader_globals_cam_ss (GfxPipeline *pipe, Ogre::Viewport *viewport)
{
    Ogre::Camera *cam = pipe->getCamera();
    Ogre::Matrix4 view = Ogre::Matrix4::IDENTITY;
    Ogre::Matrix4 proj = Ogre::Matrix4::IDENTITY;

    bool render_target_flipping = viewport->getTarget()->requiresTextureFlipping();
    Vector3 cam_pos = from_ogre(cam->getPosition());
    Vector2 viewport_dim(viewport->getActualWidth(), viewport->getActualHeight());

    Vector3 ray_top_right = from_ogre(cam->getWorldSpaceCorners()[4]) - cam_pos;
    Vector3 ray_top_left = from_ogre(cam->getWorldSpaceCorners()[5]) - cam_pos;
    Vector3 ray_bottom_left = from_ogre(cam->getWorldSpaceCorners()[6]) - cam_pos;
    Vector3 ray_bottom_right = from_ogre(cam->getWorldSpaceCorners()[7]) - cam_pos;

    return {
        cam_pos, view, view.inverseAffine(), proj,
        ray_top_left, ray_top_right, ray_bottom_left, ray_bottom_right,
        viewport_dim, render_target_flipping, pipe
    };
}

GfxShaderGlobals gfx_shader_globals_cam (GfxPipeline *pipe, const Ogre::Matrix4 &proj)
{
    Ogre::Camera *cam = pipe->getCamera();
    Ogre::Matrix4 view = cam->getViewMatrix();

    Ogre::Viewport *viewport = cam->getViewport();
    bool render_target_flipping = viewport->getTarget()->requiresTextureFlipping();
    Vector3 cam_pos = from_ogre(cam->getPosition());
    Vector2 viewport_dim(viewport->getActualWidth(), viewport->getActualHeight());

    Vector3 ray_top_right = from_ogre(cam->getWorldSpaceCorners()[4]) - cam_pos;
    Vector3 ray_top_left = from_ogre(cam->getWorldSpaceCorners()[5]) - cam_pos;
    Vector3 ray_bottom_left = from_ogre(cam->getWorldSpaceCorners()[6]) - cam_pos;
    Vector3 ray_bottom_right = from_ogre(cam->getWorldSpaceCorners()[7]) - cam_pos;

    return {
        cam_pos, view, view.inverseAffine(), proj,
        ray_top_left, ray_top_right, ray_bottom_left, ray_bottom_right,
        viewport_dim, render_target_flipping, pipe
    };
}

GfxShaderGlobals gfx_shader_globals_cam (GfxPipeline *pipe)
{
    Ogre::Camera *cam = pipe->getCamera();
    return gfx_shader_globals_cam(pipe, cam->getProjectionMatrix());
}

void try_set_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                       const std::string &name, const Ogre::Matrix4 &v)
{
    p->setNamedConstant(name, v);
}

void try_set_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                       const std::string &name, const Ogre::Matrix4 *v, unsigned n)
{
    p->setNamedConstant(name, v, n);
}

void try_set_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                       const std::string &name, int v)
{
    p->setNamedConstant(name, v);
}

void try_set_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                       const std::string &name, float v)
{
    p->setNamedConstant(name, v);
}

void try_set_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                       const std::string &name, const Vector2 &v)
{
    p->setNamedConstant(name, to_ogre(v));
}

void try_set_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                       const std::string &name, const Vector3 &v)
{
    p->setNamedConstant(name, to_ogre(v));
}

void try_set_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                       const std::string &name, const Vector4 &v)
{
    p->setNamedConstant(name, to_ogre(v));
}


template<class T> static void hack_set_constant(const Ogre::GpuProgramParametersSharedPtr &p,
                                                const std::string &name, const T&v, unsigned n)
{
    try_set_constant(p, name, v, n);
}

template<class T> static void hack_set_constant(const Ogre::GpuProgramParametersSharedPtr &vp,
                                                const Ogre::GpuProgramParametersSharedPtr &fp,
                                                const std::string &name, const T&v, unsigned n)
{
    hack_set_constant(vp, name, v, n);
    hack_set_constant(fp, name, v, n);
}

template<class T> static void hack_set_constant(const Ogre::GpuProgramParametersSharedPtr &p,
                                                const std::string &name, const T&v)
{
    try_set_constant(p, name, v);
}

template<class T> static void hack_set_constant(const Ogre::GpuProgramParametersSharedPtr &vp,
                                                const Ogre::GpuProgramParametersSharedPtr &fp,
                                                const std::string &name, const T&v)
{
    hack_set_constant(vp, name, v);
    hack_set_constant(fp, name, v);
}


void GfxShader::reset (const GfxGslRunParams &p,
                       const std::string &src_vertex,
                       const std::string &src_dangs,
                       const std::string &src_additional)
{
    params = p;
    srcVertex = src_vertex;
    srcDangs = src_dangs;
    srcAdditional = src_additional;

    // Destroy all currently built shaders
    for (const auto &pair : cachedShaders) {
        const NativePair &np = pair.second;
        Ogre::HighLevelGpuProgramManager::getSingleton().remove(np.vp->getName());
        Ogre::HighLevelGpuProgramManager::getSingleton().remove(np.fp->getName());
    }
    cachedShaders.clear();

    // all mats must reset now
}


void GfxShader::validate (void)
{
    legacy.fp->load();
    legacy.vp->load();

    if (!legacy.vp->isLoaded())
        EXCEPT << "Program not loaded: \"" << legacy.vp->getName() << "\"" << ENDL;

    if (!legacy.fp->isLoaded())
        EXCEPT << "Program not loaded: \"" << legacy.fp->getName() << "\"" << ENDL;

    Ogre::GpuProgram *vp_bd = legacy.vp->_getBindingDelegate();
    Ogre::GpuProgram *fp_bd = legacy.fp->_getBindingDelegate();

    if (vp_bd == nullptr)
        EXCEPT << "Program cannot be bound: \"" << legacy.vp->getName() << "\"";

    if (fp_bd == nullptr)
        EXCEPT << "Program cannot be bound: \"" << legacy.fp->getName() << "\"";

    if (backend == GFX_GSL_BACKEND_GLSL) {

        auto *vp_low = dynamic_cast<Ogre::GLSL::GLSLGpuProgram*>(vp_bd);
        auto *fp_low = dynamic_cast<Ogre::GLSL::GLSLGpuProgram*>(fp_bd);
        
        if (vp_low != nullptr && fp_low != nullptr) {
            // Force the actual compilation of it...
            Ogre::GLSL::GLSLLinkProgramManager::getSingleton().setActiveVertexShader(vp_low);
            Ogre::GLSL::GLSLLinkProgramManager::getSingleton().setActiveFragmentShader(fp_low);
            Ogre::GLSL::GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
        }

    }
}

NativePair GfxShader::getNativePair (Purpose purpose,
                                     bool fade_dither, unsigned env_boxes,
                                     bool instanced, unsigned bone_weights,
                                     const GfxMaterialTextureMap &textures,
                                     const GfxShaderBindings &bindings)
{
    GfxGslUnboundTextures ubt;
    for (const auto &u : params) {
        // Find undefined textures.
        if (gfx_gasoline_param_is_texture(u.second)) {
            if (textures.find(u.first) == textures.end()) {
                ubt.insert(u.first);
            }
        }
    }
    GfxShaderBindings statics;
    for (const auto &bind : bindings) {
        // Find undefined textures.
        if (gfx_gasoline_param_is_static(bind.second)) {
            statics[bind.first] = bind.second;
        }
    }
    return getNativePair(purpose, fade_dither, env_boxes, instanced, bone_weights,ubt, statics);
}

NativePair GfxShader::getNativePair (Purpose purpose,
                                     bool fade_dither, unsigned env_boxes,
                                     bool instanced, unsigned bone_weights,
                                     const GfxGslUnboundTextures &ubt,
                                     const GfxShaderBindings &statics)
{
    // Need to choose / maybe compile a shader for this combination of textures and bindings.
    GfxGslEnvironment env = shader_scene_env;
    env.fadeDither = fade_dither;
    env.envBoxes = env_boxes;
    env.instanced = instanced;
    env.boneWeights = bone_weights;
    env.ubt = ubt;
    env.staticValues = statics;

    Split split;
    split.purpose = purpose;
    split.env = env;
    auto it = cachedShaders.find(split);

    if (it == cachedShaders.end()) {
        // Need to build it.
        Ogre::HighLevelGpuProgramPtr vp;
        Ogre::HighLevelGpuProgramPtr fp;

        CVERB << "Compiling: " << name << " " << split << std::endl;

        std::string oname = fresh_name();
        if (backend == GFX_GSL_BACKEND_CG) {
            vp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                oname+"_v", RESGRP, "cg", Ogre::GPT_VERTEX_PROGRAM);
            fp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                oname+"_f", RESGRP, "cg", Ogre::GPT_FRAGMENT_PROGRAM);
            Ogre::StringVector vp_profs, fp_profs;
            if (gfx_d3d9()) {
                vp_profs.push_back("vs_3_0");
                fp_profs.push_back("ps_3_0");
            } else {
                vp_profs.push_back("gpu_vp");
                fp_profs.push_back("gp4fp");
            }
            Ogre::CgProgram *tmp_vp = static_cast<Ogre::CgProgram*>(&*vp);
            tmp_vp->setEntryPoint("main");
            tmp_vp->setProfiles(vp_profs);
            tmp_vp->setCompileArguments("-I. -O3");

            Ogre::CgProgram *tmp_fp = static_cast<Ogre::CgProgram*>(&*fp);
            tmp_fp->setEntryPoint("main");
            tmp_fp->setProfiles(fp_profs);
            tmp_fp->setCompileArguments("-I. -O3");
        } else {
            vp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                oname+"_v", RESGRP, "glsl", Ogre::GPT_VERTEX_PROGRAM);
            fp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                oname+"_f", RESGRP, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
        }


        GfxGslMetadata md;
        md.params = params;
        md.env = env;
        md.d3d9 = gfx_d3d9();

        GfxGasolineResult output;
        try {
            switch (purpose) {
                case REGULAR: EXCEPTEX << "Internal error." << ENDL;
                case ALPHA: EXCEPTEX << "Internal error." << ENDL;
                case FIRST_PERSON:
                output = gfx_gasoline_compile_first_person(backend, srcVertex, srcDangs,
                                                           srcAdditional, md);
                break;
                case FIRST_PERSON_WIREFRAME:
                output = gfx_gasoline_compile_first_person_wireframe(backend, srcVertex, md);
                break;
                case ADDITIONAL:
                output = gfx_gasoline_compile_additional(backend, srcVertex, srcAdditional, md);
                break;
                case SHADOW_CAST: EXCEPTEX << "Internal error." << ENDL;
                case WIREFRAME:
                output = gfx_gasoline_compile_wireframe(backend, srcVertex, md);
                break;
                case SKY:
                output = gfx_gasoline_compile_sky(backend, srcVertex, srcAdditional, md);
                break;
                case HUD:
                output = gfx_gasoline_compile_hud(backend, srcVertex, srcAdditional, md);
                break;
                case DECAL:
                output = gfx_gasoline_compile_decal(backend, srcDangs, srcAdditional, md);
                break;
                case DEFERRED_AMBIENT_SUN:
                output = gfx_gasoline_compile_das(backend, srcVertex, srcAdditional, md);
                break;
            }
        } catch (const Exception &e) {
            EXCEPT << name << ": " << e.msg << ENDL;
        }
        if (dump_shader == "*" || dump_shader == name) {
            CVERB << name << " (vertex)\n" << output.vertexShader << std::endl;
            CVERB << name << " (fragment)\n" << output.fragmentShader << std::endl;
        }
        vp->setSource(output.vertexShader);
        fp->setSource(output.fragmentShader);
        vp->load();
        fp->load();

        Ogre::GpuProgram *vp_bd = vp->_getBindingDelegate();
        Ogre::GpuProgram *fp_bd = fp->_getBindingDelegate();
        APP_ASSERT(vp->_getBindingDelegate() != nullptr);
        APP_ASSERT(fp->_getBindingDelegate() != nullptr);

        if (backend == GFX_GSL_BACKEND_GLSL) {

            auto *vp_low = dynamic_cast<Ogre::GLSL::GLSLGpuProgram*>(vp_bd);
            auto *fp_low = dynamic_cast<Ogre::GLSL::GLSLGpuProgram*>(fp_bd);
            
            if (vp_low != nullptr && fp_low != nullptr) {
                // Force the actual compilation of it...
                Ogre::GLSL::GLSLLinkProgramManager::getSingleton().setActiveVertexShader(vp_low);
                Ogre::GLSL::GLSLLinkProgramManager::getSingleton().setActiveFragmentShader(fp_low);
                Ogre::GLSL::GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
            }
        }
        NativePair np = {vp, fp};
        cachedShaders[split] = np;

        return np;
        
    } else {

        return it->second;

    }
}


void GfxShader::bindShaderParams (const Ogre::GpuProgramParametersSharedPtr &vparams,
                                  const Ogre::GpuProgramParametersSharedPtr &fparams,
                                  const GfxMaterialTextureMap &textures,
                                  const GfxShaderBindings &bindings)
{
    int counter = NUM_GLOBAL_TEXTURES;
    for (auto pair : params) {
        const std::string &name = pair.first;
        const auto &param = pair.second;
        if (gfx_gasoline_param_is_texture(param)) {

            auto it = textures.find(name);
            // material might leave a given texture undefined in which case we
            // are using the shader without that texture so do not bind it
            if (it == textures.end()) continue;
            if (backend == GFX_GSL_BACKEND_GLSL) {
                hack_set_constant(vparams, fparams, "mat_" + name, counter);
            }
            counter++;

        } else {
            const GfxGslParam *vptr = &param;
            auto bind = bindings.find(name);
            if (bind != bindings.end()) {
                GfxGslParamType bt = bind->second.t;
                if (bt == param.t) {
                    vptr = &bind->second;
                } else {
                    CERR << "Binding \"" << name << "\" had wrong type in shader "
                         << "\"" << name << "\": got " << bt << " but expected " << vptr->t << std::endl;
                }
            }
            const auto &v = *vptr;
            switch (v.t) {
                case GFX_GSL_FLOAT1:
                hack_set_constant(vparams, fparams, "mat_" + name, v.fs.r);
                break;

                case GFX_GSL_FLOAT2:
                hack_set_constant(vparams, fparams, "mat_" + name, Vector2(v.fs.r, v.fs.g));
                break;

                case GFX_GSL_FLOAT3:
                hack_set_constant(vparams, fparams, "mat_" + name, Vector3(v.fs.r, v.fs.g, v.fs.b));
                break;

                case GFX_GSL_FLOAT4:
                hack_set_constant(vparams, fparams, "mat_" + name, Vector4(v.fs.r, v.fs.g, v.fs.b, v.fs.a));
                break;

                case GFX_GSL_INT1:
                hack_set_constant(vparams, fparams, "mat_" + name, v.is.r);
                break;

                case GFX_GSL_INT2:
                case GFX_GSL_INT3:
                case GFX_GSL_INT4:
                EXCEPTEX << "Not implemented." << ENDL;

                case GFX_GSL_STATIC_FLOAT1:
                case GFX_GSL_STATIC_FLOAT2:
                case GFX_GSL_STATIC_FLOAT3:
                case GFX_GSL_STATIC_FLOAT4:
                case GFX_GSL_STATIC_INT1:
                case GFX_GSL_STATIC_INT2:
                case GFX_GSL_STATIC_INT3:
                case GFX_GSL_STATIC_INT4:
                // Do nothing -- these are baked into the shader already.
                break;

                default: EXCEPTEX << "Internal error." << ENDL;
            }
        }
    }

}

void GfxShader::bindShaderParamsPass (Ogre::Pass *p, const GfxMaterialTextureMap &textures)
{
    // Must be called after globals.
    for (auto pair : params) {
        const std::string &name = pair.first;
        const auto &param = pair.second;
        if (gfx_gasoline_param_is_texture(param)) {

            auto it = textures.find(name);

            // material might leave a given texture undefined in which case we
            // are using the shader without that texture so do not bind it

            if (it == textures.end()) continue;

            const GfxMaterialTexture *tex = &it->second;

            switch (param.t) {
                case GFX_GSL_FLOAT_TEXTURE1: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;
                case GFX_GSL_FLOAT_TEXTURE2: {
                    Ogre::TextureUnitState *tus = p->createTextureUnitState();
                    // TODO(dcunnin): tex is null as a temporary hack to allow binding of gbuffer
                    if (tex->texture == nullptr) break;
                    tus->setTextureName(tex->texture->getOgreTexturePtr()->getName());
                    //tus->setTextureAnisotropy(tex->anisotropy);
                    tus->setTextureFiltering(Ogre::FO_ANISOTROPIC,
                                             Ogre::FO_ANISOTROPIC, Ogre::FO_LINEAR);
                    auto mode = tex->clamp ? Ogre::TextureUnitState::TAM_CLAMP
                                           : Ogre::TextureUnitState::TAM_WRAP;
                    Ogre::TextureUnitState::UVWAddressingMode am = { mode, mode, mode };
                    tus->setTextureAddressingMode(am);
                }
                break;
                case GFX_GSL_FLOAT_TEXTURE3: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;
                case GFX_GSL_FLOAT_TEXTURE4: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;
                case GFX_GSL_FLOAT_TEXTURE_CUBE: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;

                default: EXCEPTEX << "Internal error." << ENDL;
            }
        }
    }

}

void GfxShader::bindShaderParamsRs (const GfxMaterialTextureMap &textures)
{
    int counter = NUM_GLOBAL_TEXTURES;
    for (auto pair : params) {
        const std::string &name = pair.first;
        const auto &param = pair.second;
        if (gfx_gasoline_param_is_texture(param)) {

            auto it = textures.find(name);

            // material might leave a given texture undefined in which case we
            // are using the shader without that texture so do not bind it

            if (it == textures.end()) continue;

            const GfxMaterialTexture *tex = &it->second;

            switch (param.t) {
                case GFX_GSL_FLOAT_TEXTURE1: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;
                case GFX_GSL_FLOAT_TEXTURE2: {
                    // TODO(dcunnin): tex is null as a temporary hack to allow binding of gbuffer
                    if (tex->texture == nullptr) break;
                    ogre_rs->_setTexture(counter, true, tex->texture->getOgreTexturePtr());
                    //ogre_rs->_setTextureLayerAnisotropy(counter, tex->anisotropy);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_ANISOTROPIC);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_ANISOTROPIC);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_LINEAR);
                    auto mode = tex->clamp ? Ogre::TextureUnitState::TAM_CLAMP
                                           : Ogre::TextureUnitState::TAM_WRAP;
                    Ogre::TextureUnitState::UVWAddressingMode am = { mode, mode, mode };
                    ogre_rs->_setTextureAddressingMode(counter, am);
                }
                break;
                case GFX_GSL_FLOAT_TEXTURE3: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;
                case GFX_GSL_FLOAT_TEXTURE4: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;
                case GFX_GSL_FLOAT_TEXTURE_CUBE: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;

                default: EXCEPTEX << "Internal error." << ENDL;
            }
            counter++;
        }
    }

}

void GfxShader::bindShader (Purpose purpose,
                            bool fade_dither,
                            bool instanced,
                            unsigned bone_weights,
                            const GfxShaderGlobals &globs,
                            const Ogre::Matrix4 &world,
                            const Ogre::Matrix4 *bone_world_matrixes,
                            unsigned num_bone_world_matrixes,
                            float fade,
                            const GfxMaterialTextureMap &textures,
                            const GfxShaderBindings &bindings)
{
    auto np = getNativePair(purpose, fade_dither, env_cube_count, instanced, bone_weights,
                            textures, bindings);

    // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
    ogre_rs->bindGpuProgram(np.vp->_getBindingDelegate());
    ogre_rs->bindGpuProgram(np.fp->_getBindingDelegate());

    const Ogre::GpuProgramParametersSharedPtr &vparams = np.vp->getDefaultParameters();
    const Ogre::GpuProgramParametersSharedPtr &fparams = np.fp->getDefaultParameters();
    vparams->setIgnoreMissingParams(true);
    fparams->setIgnoreMissingParams(true);

    bindGlobals(vparams, fparams, globs);
    bindGlobalsRs(globs, purpose);
    bindShaderParams(vparams, fparams, textures, bindings);
    bindShaderParamsRs(textures);
    bindBodyParams(vparams, fparams, globs, world, bone_world_matrixes, num_bone_world_matrixes,
                   fade);

    ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, vparams, Ogre::GPV_ALL);
    ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, fparams, Ogre::GPV_ALL);
}

void GfxShader::bindShaderPass (Ogre::Pass *p,
                                Purpose purpose,
                                bool fade_dither,
                                bool instanced,
                                unsigned bone_weights,
                                const GfxShaderGlobals &globs,
                                const GfxMaterialTextureMap &textures,
                                const GfxShaderBindings &bindings)
{
    auto np = getNativePair(purpose, fade_dither, env_cube_count, instanced, bone_weights,
                            textures, bindings);

    p->setFragmentProgram(np.fp->getName());
    p->setVertexProgram(np.vp->getName());

    const Ogre::GpuProgramParametersSharedPtr &vparams = p->getVertexProgramParameters();
    const Ogre::GpuProgramParametersSharedPtr &fparams = p->getFragmentProgramParameters();
    vparams->setIgnoreMissingParams(true);
    fparams->setIgnoreMissingParams(true);

    bindGlobals(vparams, fparams, globs);
    bindGlobalsPass(p, purpose);
    bindShaderParams(vparams, fparams, textures, bindings);
    bindShaderParamsPass(p, textures);
    bindBodyParamsPass(vparams, fparams);

}

void GfxShader::bindBodyParams (const Ogre::GpuProgramParametersSharedPtr &vp,
                                const Ogre::GpuProgramParametersSharedPtr &fp,
                                const GfxShaderGlobals &p, 
                                const Ogre::Matrix4 &world,
                                const Ogre::Matrix4 *bone_world_matrixes,
                                unsigned num_bone_world_matrixes,
                                float fade)
{   
    Ogre::Matrix4 world_view = p.view * world;
    Ogre::Matrix4 world_view_proj = p.proj * world_view;
    
    hack_set_constant(vp, fp, "body_worldViewProj", world_view_proj);
    hack_set_constant(vp, fp, "body_worldView", world_view);
    hack_set_constant(vp, fp, "body_world", world);
    hack_set_constant(vp, fp, "body_boneWorlds", bone_world_matrixes, num_bone_world_matrixes);
    hack_set_constant(vp, fp, "internal_fade", fade);
}


void GfxShader::bindBodyParamsPass (const Ogre::GpuProgramParametersSharedPtr &vp,
                                    const Ogre::GpuProgramParametersSharedPtr &fp)
{
    std::array<Ogre::GpuProgramParametersSharedPtr, 2> params = {vp, fp};

    for (const auto &p : params) { 
        p->setNamedAutoConstant("body_worldViewProj",
                                Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
        p->setNamedAutoConstant("body_worldView",
                                Ogre::GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
        p->setNamedAutoConstant("body_world",
                                Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
        p->setNamedAutoConstant("body_boneWorlds",
                                Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY);
        p->setNamedAutoConstant("internal_fade",
                                Ogre::GpuProgramParameters::ACT_CUSTOM, 0);
    }
}

static void inc (const Ogre::GpuProgramParametersSharedPtr &vp,
                 const Ogre::GpuProgramParametersSharedPtr &fp,
                 int &counter,
                 const char *name)
{
    if (backend == GFX_GSL_BACKEND_GLSL)
        hack_set_constant(vp, fp, name, counter);
    counter++;
}

void GfxShader::bindGlobals (const Ogre::GpuProgramParametersSharedPtr &vp,
                             const Ogre::GpuProgramParametersSharedPtr &fp,
                             const GfxShaderGlobals &g)
{
    Ogre::Matrix4 view_proj = g.proj * g.view; 
    Vector4 viewport_size(g.viewportDim.x, g.viewportDim.y,
                          1.0f/g.viewportDim.x, 1.0f/g.viewportDim.y);
    float render_target_flipping_factor = g.renderTargetFlipping ? -1.0f : 1.0f;

    hack_set_constant(vp, fp, "global_cameraPos", g.camPos);
    hack_set_constant(vp, fp, "global_fovY", gfx_option(GFX_FOV));
    hack_set_constant(vp, fp, "global_proj", g.proj);
    hack_set_constant(vp, fp, "global_time", anim_time); // FIXME:
    hack_set_constant(vp, fp, "global_viewportSize", viewport_size);
    hack_set_constant(vp, fp, "global_viewProj", view_proj);
    hack_set_constant(vp, fp, "global_view", g.view);
    hack_set_constant(vp, fp, "global_invView", g.invView);
    hack_set_constant(vp, fp, "global_rayTopLeft", g.rayTopLeft);
    hack_set_constant(vp, fp, "global_rayTopRight", g.rayTopRight);
    hack_set_constant(vp, fp, "global_rayBottomLeft", g.rayBottomLeft);
    hack_set_constant(vp, fp, "global_rayBottomRight", g.rayBottomRight);
    hack_set_constant(vp, fp, "global_nearClipDistance", gfx_option(GFX_NEAR_CLIP));
    hack_set_constant(vp, fp, "global_farClipDistance", gfx_option(GFX_FAR_CLIP));

    hack_set_constant(vp, fp, "global_shadowViewProj0", shadow_view_proj[0]);
    hack_set_constant(vp, fp, "global_shadowViewProj1", shadow_view_proj[1]);
    hack_set_constant(vp, fp, "global_shadowViewProj2", shadow_view_proj[2]);

    hack_set_constant(vp, fp, "global_particleAmbient", particle_ambient);
    hack_set_constant(vp, fp, "global_sunlightDiffuse", sunlight_diffuse);
    hack_set_constant(vp, fp, "global_sunlightDirection", sunlight_direction);
    hack_set_constant(vp, fp, "global_sunlightSpecular", sunlight_specular);

    hack_set_constant(vp, fp, "global_fogColour", fog_colour);
    hack_set_constant(vp, fp, "global_fogDensity", fog_density);
    hack_set_constant(vp, fp, "global_hellColour", hell_colour);
    hack_set_constant(vp, fp, "global_skyCloudColour", sky_cloud_colour);
    hack_set_constant(vp, fp, "global_skyCloudCoverage", sky_cloud_coverage);
    hack_set_constant(vp, fp, "global_skyGlareHorizonElevation", sky_glare_horizon_elevation);
    hack_set_constant(vp, fp, "global_skyGlareSunDistance", sky_glare_sun_distance);
    hack_set_constant(vp, fp, "global_sunAlpha", sun_alpha);
    hack_set_constant(vp, fp, "global_sunColour", sun_colour);
    hack_set_constant(vp, fp, "global_sunDirection", sun_direction);
    hack_set_constant(vp, fp, "global_sunFalloffDistance", sun_falloff_distance);
    hack_set_constant(vp, fp, "global_sunSize", sun_size);

    hack_set_constant(vp, fp, "global_skyDivider1", sky_divider[0]);
    hack_set_constant(vp, fp, "global_skyDivider2", sky_divider[1]);
    hack_set_constant(vp, fp, "global_skyDivider3", sky_divider[2]);
    hack_set_constant(vp, fp, "global_skyDivider4", sky_divider[3]);

    hack_set_constant(vp, fp, "global_skyColour0", sky_colour[0]);
    hack_set_constant(vp, fp, "global_skyColour1", sky_colour[1]);
    hack_set_constant(vp, fp, "global_skyColour2", sky_colour[2]);
    hack_set_constant(vp, fp, "global_skyColour3", sky_colour[3]);
    hack_set_constant(vp, fp, "global_skyColour4", sky_colour[4]);
    hack_set_constant(vp, fp, "global_skyColour5", sky_colour[5]);

    hack_set_constant(vp, fp, "global_skySunColour0", sky_sun_colour[0]);
    hack_set_constant(vp, fp, "global_skySunColour1", sky_sun_colour[1]);
    hack_set_constant(vp, fp, "global_skySunColour2", sky_sun_colour[2]);
    hack_set_constant(vp, fp, "global_skySunColour3", sky_sun_colour[3]);
    hack_set_constant(vp, fp, "global_skySunColour4", sky_sun_colour[4]);

    hack_set_constant(vp, fp, "global_envCubeCrossFade", env_cube_cross_fade);
    hack_set_constant(vp, fp, "global_envCubeMipmaps0", 9.0f);
    hack_set_constant(vp, fp, "global_envCubeMipmaps1", 9.0f);

    hack_set_constant(vp, fp, "global_saturation", global_saturation);
    hack_set_constant(vp, fp, "global_exposure", global_exposure);
    hack_set_constant(vp, fp, "global_bloomThreshold", gfx_option(GFX_BLOOM_THRESHOLD));

    hack_set_constant(vp, fp, "internal_rt_flip", render_target_flipping_factor);

    int counter = 0;

    // CAUTION!!! These texture bindings must be in alphabetical order.
    // And match those in bind_global_textures

    inc(vp, fp, counter, "global_colourGradeLut");
    inc(vp, fp, counter, "global_envCube0");
    inc(vp, fp, counter, "global_envCube1");
    inc(vp, fp, counter, "global_fadeDitherMap");
    inc(vp, fp, counter, "global_gbuffer0");
    const static char *name[] = { "global_shadowMap0", "global_shadowMap1", "global_shadowMap2" };
    for (unsigned i=0 ; i<3 ; ++i) inc(vp, fp, counter, name[i]);
    inc(vp, fp, counter, "global_shadowPcfNoiseMap");

    APP_ASSERT(counter == NUM_GLOBAL_TEXTURES);
}

void GfxShader::bindGlobalsPass (Ogre::Pass *p, Purpose purpose)
{
    const auto clamp = Ogre::TextureUnitState::TAM_CLAMP;
    const auto wrap = Ogre::TextureUnitState::TAM_WRAP;
    Ogre::TextureUnitState *tus;

    // CAUTION!!! These texture bindings must be in alphabetical order.
    tus = p->createTextureUnitState();
    tus->setTextureName(colour_grade_lut->getOgreTexturePtr()->getName());
    tus->setTextureFiltering(Ogre::FO_POINT, Ogre::FO_LINEAR, Ogre::FO_NONE);
    tus->setTextureAddressingMode(
        Ogre::TextureUnitState::UVWAddressingMode {clamp, clamp, clamp});

    if (global_env_cube0 != nullptr && global_env_cube1 != nullptr) {
        // Both env cubes
        tus = p->createTextureUnitState();
        tus->setTextureName(global_env_cube0->getOgreTexturePtr()->getName());
        tus->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_LINEAR);
        tus->setTextureAddressingMode(
            Ogre::TextureUnitState::UVWAddressingMode {wrap, wrap, wrap});

        tus = p->createTextureUnitState();
        tus->setTextureName(global_env_cube1->getOgreTexturePtr()->getName());
        tus->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_LINEAR);
        tus->setTextureAddressingMode(
            Ogre::TextureUnitState::UVWAddressingMode {wrap, wrap, wrap});

    } else if (global_env_cube0 != nullptr) {
        // One env cube
        tus = p->createTextureUnitState();
        tus->setTextureName(global_env_cube0->getOgreTexturePtr()->getName());
        tus->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_LINEAR);
        tus->setTextureAddressingMode(
            Ogre::TextureUnitState::UVWAddressingMode {wrap, wrap, wrap});

        tus = p->createTextureUnitState();

    } else if (global_env_cube1 != nullptr) {
        // Other env cube
        tus = p->createTextureUnitState();

        tus = p->createTextureUnitState();
        tus->setTextureName(global_env_cube1->getOgreTexturePtr()->getName());
        tus->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_LINEAR);
        tus->setTextureAddressingMode(
            Ogre::TextureUnitState::UVWAddressingMode {wrap, wrap, wrap});
    } else {
        // No env cube
        tus = p->createTextureUnitState();

        tus = p->createTextureUnitState();
    }
    

    tus = p->createTextureUnitState();
    if (fade_dither_map != nullptr) {
        tus->setTextureName(fade_dither_map->getOgreTexturePtr()->getName());
        tus->setTextureFiltering(Ogre::FO_POINT, Ogre::FO_POINT, Ogre::FO_NONE);
        tus->setTextureAddressingMode(
            Ogre::TextureUnitState::UVWAddressingMode {clamp, clamp, clamp});
    }

    tus = p->createTextureUnitState();
    if (purpose == GfxShader::DECAL) {
        // Decals will never be rendered by Ogre::Pass.
    }

    for (unsigned i=0 ; i<3 ; ++i) {
        tus = p->createTextureUnitState();
        tus->setTextureName(ogre_sm->getShadowTexture(i)->getName());
        tus->setTextureFiltering(Ogre::FO_POINT, Ogre::FO_POINT, Ogre::FO_NONE);
        tus->setTextureAddressingMode(
            Ogre::TextureUnitState::UVWAddressingMode {clamp, clamp, clamp});
    }

    tus = p->createTextureUnitState();
    if (shadow_pcf_noise_map != nullptr) {
        tus->setTextureName(shadow_pcf_noise_map->getOgreTexturePtr()->getName());
        tus->setTextureFiltering(Ogre::FO_POINT, Ogre::FO_POINT, Ogre::FO_NONE);
        tus->setTextureAddressingMode(
            Ogre::TextureUnitState::UVWAddressingMode {clamp, clamp, clamp});
    }

    APP_ASSERT(p->getNumTextureUnitStates() == NUM_GLOBAL_TEXTURES);

}

void GfxShader::bindGlobalsRs (const GfxShaderGlobals &g, Purpose purpose)
{
    int counter = 0;
    const auto clamp = Ogre::TextureUnitState::TAM_CLAMP;
    const auto wrap = Ogre::TextureUnitState::TAM_WRAP;

    ogre_rs->_setTexture(counter, true, colour_grade_lut->getOgreTexturePtr());
    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_POINT);
    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_LINEAR);
    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_NONE);
    ogre_rs->_setTextureAddressingMode(
        counter, Ogre::TextureUnitState::UVWAddressingMode {clamp, clamp, clamp});
    counter++;

    if (global_env_cube0 != nullptr && global_env_cube1 != nullptr) {
        // Both env cubes
        ogre_rs->_setTexture(counter, true, global_env_cube0->getOgreTexturePtr());
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_LINEAR);
        ogre_rs->_setTextureAddressingMode(
            counter, Ogre::TextureUnitState::UVWAddressingMode {wrap, wrap, wrap});
        counter++;
        ogre_rs->_setTexture(counter, true, global_env_cube1->getOgreTexturePtr());
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_LINEAR);
        ogre_rs->_setTextureAddressingMode(
            counter, Ogre::TextureUnitState::UVWAddressingMode {wrap, wrap, wrap});
        counter++;

    } else if (global_env_cube0 != nullptr) {
        // One env cube
        ogre_rs->_setTexture(counter, true, global_env_cube0->getOgreTexturePtr());
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_LINEAR);
        ogre_rs->_setTextureAddressingMode(
            counter, Ogre::TextureUnitState::UVWAddressingMode {wrap, wrap, wrap});
        counter++;

        ogre_rs->_setTexture(counter, false, "");
        counter++;

    } else if (global_env_cube1 != nullptr) {
        // Other env cube
        ogre_rs->_setTexture(counter, false, "");
        counter++;

        ogre_rs->_setTexture(counter, true, global_env_cube1->getOgreTexturePtr());
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_LINEAR);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_LINEAR);
        ogre_rs->_setTextureAddressingMode(
            counter, Ogre::TextureUnitState::UVWAddressingMode {wrap, wrap, wrap});
        counter++;
    } else {
        // No env cube
        ogre_rs->_setTexture(counter, false, "");
        counter++;

        ogre_rs->_setTexture(counter, false, "");
        counter++;
    }
    

    if (fade_dither_map != nullptr) {
        ogre_rs->_setTexture(counter, true, fade_dither_map->getOgreTexturePtr());
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_POINT);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_POINT);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_NONE);
        ogre_rs->_setTextureAddressingMode(
            counter, Ogre::TextureUnitState::UVWAddressingMode {clamp, clamp, clamp});
    } else {
        ogre_rs->_setTexture(counter, false, "");
    }
    counter++;

    if (purpose == GfxShader::DECAL) {
        ogre_rs->_setTexture(counter, true, g.pipe->getGBufferTexture(0));
    } else {
        ogre_rs->_setTexture(counter, false, "");
    }
    counter++;

    for (unsigned i=0 ; i<3 ; ++i) {
        ogre_rs->_setTexture(counter, true, ogre_sm->getShadowTexture(i));
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_POINT);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_POINT);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_NONE);
        ogre_rs->_setTextureAddressingMode(
            counter, Ogre::TextureUnitState::UVWAddressingMode {clamp, clamp, clamp});
        counter++;
    }

    if (shadow_pcf_noise_map != nullptr) {
        ogre_rs->_setTexture(counter, true, shadow_pcf_noise_map->getOgreTexturePtr());
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_POINT);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_POINT);
        ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_NONE);
        ogre_rs->_setTextureAddressingMode(
            counter, Ogre::TextureUnitState::UVWAddressingMode {clamp, clamp, clamp});
    } else {
        ogre_rs->_setTexture(counter, false, "");
    }
    counter++;

    // CAUTION!!! These texture bindings must be in alphabetical order.

    APP_ASSERT(counter == NUM_GLOBAL_TEXTURES);

}

GfxShader *gfx_shader_make_from_existing (const std::string &name,
                                          const Ogre::HighLevelGpuProgramPtr &vp,
                                          const Ogre::HighLevelGpuProgramPtr &fp,
                                          const GfxGslRunParams &params)
{
    if (gfx_shader_has(name))
        EXCEPT << "Shader already exists: " << name << ENDL;
    auto *s = new GfxShader(name, params, vp, fp);
    shader_db[name] = s;
    return s;
}


GfxShader *gfx_shader_make_or_reset (const std::string &name,
                                     const std::string &new_vertex_code,
                                     const std::string &new_dangs_code,
                                     const std::string &new_additional_code,
                                     const GfxGslRunParams &params)
{
    gfx_shader_check(name, new_vertex_code, new_dangs_code, new_additional_code, params);
    GfxShader *shader;
    if (gfx_shader_has(name)) {
        shader = gfx_shader_get(name);
        shader->reset(params, new_vertex_code, new_dangs_code, new_additional_code);
        // TODO: go through materials, checking them all
    } else {
        shader = new GfxShader(name, params, new_vertex_code, new_dangs_code, new_additional_code);
        shader_db[name] = shader;
    }
    return shader;
}

GfxShader *gfx_shader_get (const std::string &name)
{
    if (!gfx_shader_has(name)) GRIT_EXCEPT("Shader does not exist: \"" + name + "\"");
    return shader_db[name];
}

bool gfx_shader_has (const std::string &name)
{
    GfxShaderDB::iterator it = shader_db.find(name);
    if (it == shader_db.end()) return false;
    return true;
}

void gfx_shader_check (const std::string &name,
                       const std::string &src_vertex,
                       const std::string &src_dangs,
                       const std::string &src_additional,
                       const GfxGslRunParams &params)
{
    GfxGslMetadata md;
    md.params = params;
    md.d3d9 = gfx_d3d9();
    try {
        gfx_gasoline_check(src_vertex, src_dangs, src_additional, md);
    } catch (const Exception &e) {
        EXCEPT << name << ": " << e.msg << ENDL;
    }
}



void gfx_shader_init (void)
{
}

void gfx_shader_shutdown (void)
{
}


