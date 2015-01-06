#include <OgreGLGpuProgram.h>
#include <OgreGLSLGpuProgram.h>
#include <OgreGLSLProgram.h>
#include <OgreGLSLLinkProgramManager.h>
#include <OgreCgProgram.h>


#include "../centralised_log.h"

#include "gfx.h"
#include "gfx_gasoline.h"
#include "gfx_internal.h"
#include "gfx_shader.h"

GfxGslBackend backend = gfx_d3d9() ? GFX_GSL_BACKEND_CG : GFX_GSL_BACKEND_GLSL;

static std::string fresh_name (void)
{
    static int counter = 0;
    std::stringstream ss;
    ss << "Gen:" << counter++;
    return ss.str();
}


void GfxShader::reset (const GfxShaderParamMap &p,
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

    if (!gfx_d3d9()) {

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

void try_set_constant (const Ogre::HighLevelGpuProgramPtr &p,
                       const std::string &name, const Ogre::Matrix4 &v)
{
    p->getDefaultParameters()->setIgnoreMissingParams(true);
    p->getDefaultParameters()->setNamedConstant(name, v);
}

void try_set_constant (const Ogre::HighLevelGpuProgramPtr &p,
                       const std::string &name, int v)
{
    p->getDefaultParameters()->setIgnoreMissingParams(true);
    p->getDefaultParameters()->setNamedConstant(name, v);
}

void try_set_constant (const Ogre::HighLevelGpuProgramPtr &p,
                       const std::string &name, float v)
{
    p->getDefaultParameters()->setIgnoreMissingParams(true);
    p->getDefaultParameters()->setNamedConstant(name, v);
}

void try_set_constant (const Ogre::HighLevelGpuProgramPtr &p,
                       const std::string &name, const Vector2 &v)
{
    p->getDefaultParameters()->setIgnoreMissingParams(true);
    (void) name;
    (void) v;
    EXCEPTEX << "Ogre does not implement this." << ENDL;
    //p->getDefaultParameters()->setNamedConstant(name, to_ogre(v));
}

void try_set_constant (const Ogre::HighLevelGpuProgramPtr &p,
                       const std::string &name, const Vector3 &v)
{
    p->getDefaultParameters()->setIgnoreMissingParams(true);
    p->getDefaultParameters()->setNamedConstant(name, to_ogre(v));
}

void try_set_constant (const Ogre::HighLevelGpuProgramPtr &p,
                       const std::string &name, const Vector4 &v)
{
    p->getDefaultParameters()->setIgnoreMissingParams(true);
    p->getDefaultParameters()->setNamedConstant(name, to_ogre(v));
}


template<class T> static void hack_set_constant(const Ogre::HighLevelGpuProgramPtr &vp,
                                                const Ogre::HighLevelGpuProgramPtr &fp,
                                                const std::string &name, const T&v)
{
    try_set_constant(vp, name, v);
    try_set_constant(fp, name, v);
}

template<class T> static void hack_set_constant(const GfxShader::NativePair &np,
                                                const std::string &name, const T&v)
{
    hack_set_constant(np.vp, np.fp, name, v);
}

void GfxShader::bindShader (Purpose purpose,
                            const GfxShaderGlobals &globs,
                            const Ogre::Matrix4 &world,
                            const GfxMaterialTextureMap &textures,
                            const GfxShaderBindings &bindings)
{
    // Need to choose / maybe compile a shader for this combination of textures and bindings.
    Split split;
    split.purpose = purpose;
    for (const auto &pair : textures)
        split.boundTextures.insert(pair.first);
    auto it = cachedShaders.find(split);
    NativePair np;

    if (it == cachedShaders.end()) {
        // Need to build it.
        Ogre::HighLevelGpuProgramPtr vp;
        Ogre::HighLevelGpuProgramPtr fp;

        std::string name = fresh_name();
        if (backend == GFX_GSL_BACKEND_CG) {
            vp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                name+"_v", RESGRP, "cg", Ogre::GPT_VERTEX_PROGRAM);
            fp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                name+"_f", RESGRP, "cg", Ogre::GPT_FRAGMENT_PROGRAM);
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
                name+"_v", RESGRP, "glsl", Ogre::GPT_VERTEX_PROGRAM);
            fp = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
                name+"_f", RESGRP, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
        }

        GfxGasolineResult output;
        switch (purpose) {
            case REGULAR: EXCEPTEX << "Internal error." << ENDL;
            case HUD:
            case SKY: {
                // Would probably be quicker if gsl compiler took set of textures and the
                // GfxShaderParams map instead of GfxGslParams.
                GfxGslUnboundTextures ubt;
                GfxGslParams gsl_params;
                for (const auto &u : params) {
                    // We only need the types to compile it.
                    gsl_params[u.first] = u.second.t;
                    // Find undefined textures, substitute values
                    if (gfx_gasoline_param_is_texture(u.second.t)) {
                        if (textures.find(u.first) == textures.end()) {
                            Vector4 val = u.second.getVector4();
                            ubt[u.first] = GfxGslColour(val.x, val.y, val.z, val.w);
                        }
                    }
                }

                output = gfx_gasoline_compile_colour(backend, srcVertex, srcAdditional,
                                                     gsl_params, ubt);
            } break;
            case FIRST_PERSON: EXCEPTEX << "Internal error." << ENDL;
        }
        vp->setSource(output.vertexShader);
        fp->setSource(output.fragmentShader);
        vp->load();
        fp->load();

        Ogre::GpuProgram *vp_bd = vp->_getBindingDelegate();
        Ogre::GpuProgram *fp_bd = fp->_getBindingDelegate();
        APP_ASSERT(vp->_getBindingDelegate() != nullptr);
        APP_ASSERT(fp->_getBindingDelegate() != nullptr);

        if (!gfx_d3d9()) {

            auto *vp_low = dynamic_cast<Ogre::GLSL::GLSLGpuProgram*>(vp_bd);
            auto *fp_low = dynamic_cast<Ogre::GLSL::GLSLGpuProgram*>(fp_bd);
            
            if (vp_low != nullptr && fp_low != nullptr) {
                // Force the actual compilation of it...
                Ogre::GLSL::GLSLLinkProgramManager::getSingleton().setActiveVertexShader(vp_low);
                Ogre::GLSL::GLSLLinkProgramManager::getSingleton().setActiveFragmentShader(fp_low);
                Ogre::GLSL::GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
            }
        }
        np = {vp, fp};
        cachedShaders[split] = np;
        
    } else {

        np = it->second;

    }
    

    // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
    ogre_rs->bindGpuProgram(np.vp->_getBindingDelegate());
    ogre_rs->bindGpuProgram(np.fp->_getBindingDelegate());

    int counter = 0;
    for (auto pair : params) {
        const std::string &name = pair.first;
        const auto &param = pair.second;
        if (gfx_gasoline_param_is_texture(param.t)) {

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
                    APP_ASSERT(tex != NULL);
                    ogre_rs->_setTexture(counter, true, tex->texture->getOgreTexturePtr());
                    //ogre_rs->_setTextureLayerAnisotropy(counter, tex->anisotropy);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIN, Ogre::FO_ANISOTROPIC);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MAG, Ogre::FO_ANISOTROPIC);
                    ogre_rs->_setTextureUnitFiltering(counter, Ogre::FT_MIP, Ogre::FO_LINEAR);
                    auto mode = tex->clamp ? Ogre::TextureUnitState::TAM_CLAMP
                                               : Ogre::TextureUnitState::TAM_WRAP;
                    Ogre::TextureUnitState::UVWAddressingMode am;
                    am.u = mode;
                    am.v = mode;
                    am.w = mode;
                    ogre_rs->_setTextureAddressingMode(counter, am);
                }
                break;
                case GFX_GSL_FLOAT_TEXTURE3: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;
                case GFX_GSL_FLOAT_TEXTURE4: {
                    EXCEPTEX << "Not yet implemented." << ENDL;
                } break;

                default: EXCEPTEX << "Internal error." << ENDL;
            }
            if (!gfx_d3d9()) {
                explicitBinding(np, "mat_" + name, counter);
            }
            counter++;

        } else {
            const GfxShaderParam *vptr = &param;
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
                explicitBinding(np, "mat_" + name, v.fs[0]);
                break;

                case GFX_GSL_FLOAT2:
                explicitBinding(np, "mat_" + name, Vector2(v.fs[0], v.fs[1]));
                break;

                case GFX_GSL_FLOAT3:
                explicitBinding(np, "mat_" + name, Vector3(v.fs[0], v.fs[1], v.fs[2]));
                break;

                case GFX_GSL_FLOAT4:
                explicitBinding(np, "mat_" + name, Vector4(v.fs[0], v.fs[1], v.fs[2], v.fs[3]));
                break;

                case GFX_GSL_INT1:
                explicitBinding(np, "mat_" + name, v.is[0]);
                break;

                case GFX_GSL_INT2:
                case GFX_GSL_INT3:
                case GFX_GSL_INT4:
                EXCEPTEX << "Not implemented." << ENDL;

                default: EXCEPTEX << "Internal error." << ENDL;
            }
        }
    }

    bindGlobals(np, globs, world);

    ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, np.vp->getDefaultParameters(), Ogre::GPV_ALL);
    ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, np.fp->getDefaultParameters(), Ogre::GPV_ALL);

}

void GfxShader::bindShaderParams (void)
{
    ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, legacy.vp->getDefaultParameters(), Ogre::GPV_ALL);
    ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, legacy.fp->getDefaultParameters(), Ogre::GPV_ALL);
}

void GfxShader::bindGlobals (const GfxShaderGlobals &p, const Ogre::Matrix4 &world)
{
    bindGlobals(legacy, p, world);
}

void GfxShader::bindGlobals (const NativePair &np, const GfxShaderGlobals &p,
                             const Ogre::Matrix4 &world)
{
    Ogre::Matrix4 world_view = p.view * world;
    Ogre::Matrix4 view_proj = p.proj * p.view; 
    Ogre::Matrix4 world_view_proj = p.proj * world_view;
    Vector4 viewport_size(p.viewport_dim.x, p.viewport_dim.y,
                          1.0f/p.viewport_dim.x, 1.0f/p.viewport_dim.y);
    float render_target_flipping_factor = p.render_target_flipping ? -1.0f : 1.0f;

    hack_set_constant(np, "global_world", world);
    hack_set_constant(np, "global_view", p.view);
    hack_set_constant(np, "global_proj", p.proj);
    hack_set_constant(np, "global_worldView", world_view);
    hack_set_constant(np, "global_viewProj", view_proj);
    hack_set_constant(np, "global_worldViewProj", world_view_proj);
    hack_set_constant(np, "global_viewportSize", viewport_size);
    hack_set_constant(np, "global_fovY", gfx_option(GFX_FOV));
    hack_set_constant(np, "global_time", anim_time); // FIXME:

    hack_set_constant(np, "global_particleAmbient", particle_ambient);
    hack_set_constant(np, "global_sunlightDiffuse", sunlight_diffuse);
    hack_set_constant(np, "global_sunlightSpecular", sunlight_specular);
    hack_set_constant(np, "global_sunlightDirection", sunlight_direction);
    hack_set_constant(np, "global_fogColour", fog_colour);
    hack_set_constant(np, "global_fogDensity", fog_density);
    hack_set_constant(np, "global_sunDirection", sun_direction);
    hack_set_constant(np, "global_sunColour", sun_colour);
    hack_set_constant(np, "global_sunAlpha", sun_alpha);
    hack_set_constant(np, "global_sunSize", sun_size);
    hack_set_constant(np, "global_sunFalloffDistance", sun_falloff_distance);
    hack_set_constant(np, "global_skyGlareSunDistance", sky_glare_sun_distance);
    hack_set_constant(np, "global_skyGlareHorizonElevation", sky_glare_horizon_elevation);
    hack_set_constant(np, "global_skyCloudColour", sky_cloud_colour);
    hack_set_constant(np, "global_skyCloudCoverage", sky_cloud_coverage);
    hack_set_constant(np, "global_hellColour", hell_colour);

    hack_set_constant(np, "global_skyDivider1", sky_divider[0]);
    hack_set_constant(np, "global_skyDivider2", sky_divider[1]);
    hack_set_constant(np, "global_skyDivider3", sky_divider[2]);
    hack_set_constant(np, "global_skyDivider4", sky_divider[3]);

    hack_set_constant(np, "global_skyColour0", sky_colour[0]);
    hack_set_constant(np, "global_skyColour1", sky_colour[1]);
    hack_set_constant(np, "global_skyColour2", sky_colour[2]);
    hack_set_constant(np, "global_skyColour3", sky_colour[3]);
    hack_set_constant(np, "global_skyColour4", sky_colour[4]);
    hack_set_constant(np, "global_skyColour5", sky_colour[5]);

    hack_set_constant(np, "global_skySunColour0", sky_sun_colour[0]);
    hack_set_constant(np, "global_skySunColour1", sky_sun_colour[1]);
    hack_set_constant(np, "global_skySunColour2", sky_sun_colour[2]);
    hack_set_constant(np, "global_skySunColour3", sky_sun_colour[3]);
    hack_set_constant(np, "global_skySunColour4", sky_sun_colour[4]);

    hack_set_constant(np, "internal_rt_flip", render_target_flipping_factor);
}

void GfxShader::explicitBinding (const NativePair &np, const std::string &name, const Ogre::Matrix4 &v)
{
    hack_set_constant(np, name, v);
}

void GfxShader::explicitBinding (const NativePair &np, const std::string &name, int v)
{
    hack_set_constant(np, name, v);
}

void GfxShader::explicitBinding (const NativePair &np, const std::string &name, float v)
{
    hack_set_constant(np, name, v);
}

void GfxShader::explicitBinding (const NativePair &np, const std::string &name, const Vector2 &v)
{
    hack_set_constant(np, name, v);
}

void GfxShader::explicitBinding (const NativePair &np, const std::string &name, const Vector3 &v)
{
    hack_set_constant(np, name, v);
}

void GfxShader::explicitBinding (const NativePair &np, const std::string &name, const Vector4 &v)
{
    hack_set_constant(np, name, v);
}


void GfxShader::explicitBinding (const std::string &name, const Ogre::Matrix4 &v)
{
    explicitBinding(legacy, name, v);
}

void GfxShader::explicitBinding (const std::string &name, int v)
{
    explicitBinding(legacy, name, v);
}

void GfxShader::explicitBinding (const std::string &name, float v)
{
    explicitBinding(legacy, name, v);
}

void GfxShader::explicitBinding (const std::string &name, const Vector2 &v)
{
    explicitBinding(legacy, name, v);
}

void GfxShader::explicitBinding (const std::string &name, const Vector3 &v)
{
    explicitBinding(legacy, name, v);
}

void GfxShader::explicitBinding (const std::string &name, const Vector4 &v)
{
    explicitBinding(legacy, name, v);
}

GfxShader *gfx_shader_make_from_existing (const std::string &name,
                                          const Ogre::HighLevelGpuProgramPtr &vp,
                                          const Ogre::HighLevelGpuProgramPtr &fp,
                                          const GfxShaderParamMap &params)
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
                                     const GfxShaderParamMap &params)
{
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

void gfx_shader_check (GfxShader::Purpose purpose,
                       const std::string &src_vertex,
                       const std::string &src_dangs,
                       const std::string &src_additional,
                       const GfxShaderParamMap &params)
{
    (void) src_dangs;
    switch (purpose) {
        case GfxShader::REGULAR: EXCEPTEX << "Internal error." << ENDL;
        case GfxShader::HUD:
        case GfxShader::SKY: {
            GfxGslUnboundTextures ubt;
            GfxGslParams gsl_params;
            for (const auto &u : params) {
                // We only need the types to compile it.
                gsl_params[u.first] = u.second.t;
            }
            gfx_gasoline_compile_colour(backend, src_vertex, src_additional, gsl_params, ubt);
        } break;
        case GfxShader::FIRST_PERSON: EXCEPTEX << "Internal error." << ENDL;
    }
}



void gfx_shader_init (void)
{
}

void gfx_shader_shutdown (void)
{
}


