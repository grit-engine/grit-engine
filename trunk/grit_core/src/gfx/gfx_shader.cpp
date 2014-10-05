#include <OgreGLGpuProgram.h>
#include <OgreGLSLGpuProgram.h>
#include <OgreGLSLProgram.h>
#include <OgreGLSLLinkProgramManager.h>

#include "../centralised_log.h"

#include "gfx_shader.h"
#include "gfx.h"

std::map<std::string, GfxShader*> all_shaders;

void GfxShader::validate (void)
{
    fp->load();
    vp->load();

    if (!vp->isLoaded())
        EXCEPT << "Program not loaded: \"" << vp->getName() << "\"" << ENDL;

    if (!fp->isLoaded())
        EXCEPT << "Program not loaded: \"" << fp->getName() << "\"" << ENDL;

    Ogre::GpuProgram *vp_bd = vp->_getBindingDelegate();
    Ogre::GpuProgram *fp_bd = fp->_getBindingDelegate();

    if (vp_bd == nullptr)
        EXCEPT << "Program cannot be bound: \"" << vp->getName() << "\"";

    if (fp_bd == nullptr)
        EXCEPT << "Program cannot be bound: \"" << fp->getName() << "\"";

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

GfxShaderBindingsPtr GfxShader::makeBindings (void)
{
    GfxShaderBindingsPtr r(new GfxShaderBindings(this));
    return r;
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


template<class T> static void hack_set_constant(GfxShader *shader, const std::string &name, const T&v)
{
    try_set_constant(shader->getOgreVertexProgram(), name, v);
    try_set_constant(shader->getOgreFragmentProgram(), name, v);
}

void GfxShader::bindGlobals (const Ogre::Matrix4 &world,
                             const Ogre::Matrix4 &view,
                             const Ogre::Matrix4 &proj,
                             const Vector2 &viewport_dim,
                             bool render_target_flipping)
{
    Ogre::Matrix4 world_view = view * world;
    Ogre::Matrix4 view_proj = proj * view; 
    Ogre::Matrix4 world_view_proj = proj * view * world;
    Vector4 viewport_size(viewport_dim.x, viewport_dim.y,
                          1.0f/viewport_dim.x, 1.0f/viewport_dim.y);
    float render_target_flipping_factor = render_target_flipping ? -1.0f : 1.0f;

    hack_set_constant(this, "global_world", world);
    hack_set_constant(this, "global_view", view);
    hack_set_constant(this, "global_proj", proj);
    hack_set_constant(this, "global_worldView", world_view);
    hack_set_constant(this, "global_viewProj", view_proj);
    hack_set_constant(this, "global_worldViewProj", world_view_proj);
    hack_set_constant(this, "global_viewportSize", viewport_size);
    hack_set_constant(this, "global_fovY", gfx_option(GFX_FOV));
    hack_set_constant(this, "global_time", anim_time); // FIXME:

    hack_set_constant(this, "global_particleAmbient", particle_ambient);
    hack_set_constant(this, "global_sunlightDiffuse", sunlight_diffuse);
    hack_set_constant(this, "global_sunlightSpecular", sunlight_specular);
    hack_set_constant(this, "global_sunlightDirection", sunlight_direction);
    hack_set_constant(this, "global_fogColour", fog_colour);
    hack_set_constant(this, "global_fogDensity", fog_density);
    hack_set_constant(this, "global_sunDirection", sun_direction);
    hack_set_constant(this, "global_sunColour", sun_colour);
    hack_set_constant(this, "global_sunAlpha", sun_alpha);
    hack_set_constant(this, "global_sunSize", sun_size);
    hack_set_constant(this, "global_sunFalloffDistance", sun_falloff_distance);
    hack_set_constant(this, "global_skyGlareSunDistance", sky_glare_sun_distance);
    hack_set_constant(this, "global_skyGlareHorizonElevation", sky_glare_horizon_elevation);
    hack_set_constant(this, "global_skyCloudColour", sky_cloud_colour);
    hack_set_constant(this, "global_skyCloudCoverage", sky_cloud_coverage);
    hack_set_constant(this, "global_hellColour", hell_colour);

    hack_set_constant(this, "global_skyDivider1", sky_divider[0]);
    hack_set_constant(this, "global_skyDivider2", sky_divider[1]);
    hack_set_constant(this, "global_skyDivider3", sky_divider[2]);
    hack_set_constant(this, "global_skyDivider4", sky_divider[3]);

    hack_set_constant(this, "global_skyColour0", sky_colour[0]);
    hack_set_constant(this, "global_skyColour1", sky_colour[1]);
    hack_set_constant(this, "global_skyColour2", sky_colour[2]);
    hack_set_constant(this, "global_skyColour3", sky_colour[3]);
    hack_set_constant(this, "global_skyColour4", sky_colour[4]);
    hack_set_constant(this, "global_skyColour5", sky_colour[5]);

    hack_set_constant(this, "global_skySunColour0", sky_sun_colour[0]);
    hack_set_constant(this, "global_skySunColour1", sky_sun_colour[1]);
    hack_set_constant(this, "global_skySunColour2", sky_sun_colour[2]);
    hack_set_constant(this, "global_skySunColour3", sky_sun_colour[3]);
    hack_set_constant(this, "global_skySunColour4", sky_sun_colour[4]);

    hack_set_constant(this, "internal_rt_flip", render_target_flipping_factor);
}

void GfxShader::bindShader (void)
{

    APP_ASSERT(vp->_getBindingDelegate()!=NULL);
    APP_ASSERT(fp->_getBindingDelegate()!=NULL);

    // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
    ogre_rs->bindGpuProgram(vp->_getBindingDelegate());
    ogre_rs->bindGpuProgram(fp->_getBindingDelegate());
}

void GfxShader::bindShaderParams (void)
{

    ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, vp->getDefaultParameters(), Ogre::GPV_ALL);
    ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, fp->getDefaultParameters(), Ogre::GPV_ALL);
}

void GfxShader::bind (const GfxShaderBindingsPtr &bindings)
{
    const auto &binds = bindings->bindings;
    int counter = 0;
    for (auto param : params) {
        const auto *vptr = &param.second;
        auto bind = binds.find(param.first);
        if (bind != binds.end()) {
            GfxGslParamType bt = bind->second.t;
            if (bt == vptr->t) {
                vptr = &bind->second;
            } else {
                CERR << "Binding \"" << param.first << "\" had wrong type in shader "
                     << "\"" << name << "\": got " << bt << " but expected " << vptr->t << std::endl;
            }
        }
        const auto &v = *vptr;
        switch (v.t) {
            case GFX_GSL_FLOAT1:
            explicitBinding("mat_" + param.first, v.fs[0]);
            break;

            case GFX_GSL_FLOAT2:
            explicitBinding("mat_" + param.first, Vector2(v.fs[0], v.fs[1]));
            break;

            case GFX_GSL_FLOAT3:
            explicitBinding("mat_" + param.first, Vector3(v.fs[0], v.fs[1], v.fs[2]));
            break;

            case GFX_GSL_FLOAT4:
            explicitBinding("mat_" + param.first, Vector4(v.fs[0], v.fs[1], v.fs[2], v.fs[3]));
            break;

            case GFX_GSL_INT1:
            explicitBinding("mat_" + param.first, v.is[0]);
            break;

            case GFX_GSL_INT2:
            case GFX_GSL_INT3:
            case GFX_GSL_INT4:
            EXCEPTEX << "Not implemented." << ENDL;

            case GFX_GSL_FLOAT_TEXTURE1:
            case GFX_GSL_FLOAT_TEXTURE2:
            case GFX_GSL_FLOAT_TEXTURE3:
            case GFX_GSL_FLOAT_TEXTURE4:
            if (!gfx_d3d9()) {
                explicitBinding("mat_" + param.first, counter++);
            }
        }
    }
}

void GfxShader::explicitBinding (const std::string &name, const Ogre::Matrix4 &v)
{
    hack_set_constant(this, name, v);
}

void GfxShader::explicitBinding (const std::string &name, int v)
{
    hack_set_constant(this, name, v);
}

void GfxShader::explicitBinding (const std::string &name, float v)
{
    hack_set_constant(this, name, v);
}

void GfxShader::explicitBinding (const std::string &name, const Vector2 &v)
{
    hack_set_constant(this, name, v);
}

void GfxShader::explicitBinding (const std::string &name, const Vector3 &v)
{
    hack_set_constant(this, name, v);
}

void GfxShader::explicitBinding (const std::string &name, const Vector4 &v)
{
    hack_set_constant(this, name, v);
}


GfxShaderParam GfxShaderBindings::getBinding (const std::string &name)
{
    return bindings[name];
}


GfxShader *gfx_shader_make_from_existing (const std::string &name,
                                          const Ogre::HighLevelGpuProgramPtr &vp,
                                          const Ogre::HighLevelGpuProgramPtr &fp,
                                          const GfxShaderParamMap &params)
{
    if (all_shaders.find(name) != all_shaders.end())
        EXCEPT << "GfxShader already exists: " << name << ENDL;

    auto *s = new GfxShader(name, vp, fp, params);
    all_shaders[name] = s;
    return s;
}

GfxShader *gfx_shader_get (const std::string &name)
{
    auto it = all_shaders.find(name);
    if (it == all_shaders.end())
        EXCEPT << "GfxShader not found: " << name << ENDL;
    return it->second;
}

void gfx_shader_init (void)
{
}

void gfx_shader_shutdown (void)
{
    for (auto pair : all_shaders)
        delete pair.second;
    all_shaders.clear();
}


