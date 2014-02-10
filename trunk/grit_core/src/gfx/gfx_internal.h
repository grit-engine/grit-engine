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
#include <sstream>

#define THROW_DEAD(name) do { GRIT_EXCEPT(name+" has been destroyed."); } while (0)

#include <Ogre.h>
#include <OgreMeshManager.h>
#include <OgreOctreeSceneManager.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreCustomCompositionPass.h>
#include <OgreCompositor.h>
#include <OgreOctreePlugin.h>
#include <OgreCgPlugin.h>
#include <OgreRenderSystem.h>
#include <OgreGLRenderSystem.h>
#ifdef WIN32
#  include <OgreD3D9RenderSystem.h>
#endif

#include "gfx.h"
#include "gfx_option.h"

#define RESGRP "GRIT"

// render queues, ordering now actually determined by GfxPipeline
#define RQ_GBUFFER_OPAQUE 10
#define RQ_FORWARD_OPAQUE 20
#define RQ_FORWARD_OPAQUE_EMISSIVE 21
#define RQ_FORWARD_ALPHA_DEPTH 60
#define RQ_FORWARD_ALPHA_DEPTH_EMISSIVE 61
#define RQ_FORWARD_ALPHA 70
#define RQ_FORWARD_ALPHA_EMISSIVE 71
#define RQ_BULLET_DEBUG_DRAWER 75

#ifndef GFX_INTERNAL_H
#define GFX_INTERNAL_H

extern bool d3d9;
extern Ogre::Root *ogre_root;
extern Ogre::RenderWindow *ogre_win;
extern Ogre::RenderSystem *ogre_rs;

extern Ogre::OctreeSceneManager *ogre_sm;
extern Ogre::SceneNode *ogre_root_node;
extern Ogre::Light *ogre_sun;
extern GfxEnvCubeDiskResource *scene_env_cube;
extern Ogre::Matrix4 shadow_view_proj[3];

static inline bool stereoscopic (void)
{ return gfx_option(GFX_CROSS_EYE) || gfx_option(GFX_ANAGLYPH); }

extern bool use_hwgamma;
extern GfxCallback *gfx_cb;
extern bool shutting_down;

extern Vector3 particle_ambient;
extern Vector3 fog_colour;
extern float fog_density;
extern float env_brightness;
extern float global_exposure;
extern float global_saturation;
extern GfxColourGradeLUTDiskResource *scene_colour_grade_lut;

extern Vector3 sun_direction;
extern Vector3 sun_colour;
extern float sun_alpha;
extern float sun_size;
extern float sun_falloff_distance;
extern float sky_glare_sun_distance;
extern float sky_glare_horizon_elevation;
extern float sky_divider[4];
extern Vector3 sky_colour[6];
extern Vector3 sky_sun_colour[5];
extern float sky_alpha[6];
extern float sky_sun_alpha[5];
extern Vector3 sky_cloud_colour;
extern float sky_cloud_coverage;
extern Vector3 hell_colour;

#define ANIM_TIME_MAX 2310.0f
extern float anim_time;

std::string freshname (const std::string &prefix);
std::string freshname (void);

void clean_compositors (void);
void do_reset_framebuffer (void);
void do_reset_compositors (void);

// API consumers should not see this
class GfxBaseMaterial {

    public:

    virtual ~GfxBaseMaterial (void) { }

    virtual void addDependencies (DiskResource *into) = 0;

};

typedef std::map<std::string,GfxBaseMaterial*> GfxMaterialDB;

extern GfxMaterialDB material_db;

void gfx_material_add_dependencies (const std::string &name, DiskResource *into);

void handle_dirty_materials (void);


class GfxSkyShader;

typedef std::map<std::string,GfxSkyShader*> GfxShaderDB;

extern GfxShaderDB shader_db;


inline void load_and_validate_shader (const Ogre::HighLevelGpuProgramPtr &prog)
{
    prog->load();

    if (!prog->isLoaded()) {
        std::stringstream ss;
        ss << "Program not loaded: \"" << prog->getName() << "\"";
        GRIT_EXCEPT(ss.str());
    }

    if (prog->_getBindingDelegate() == NULL) {
        std::stringstream ss;
        ss << "Program cannot be bound: \"" << prog->getName() << "\"";
        GRIT_EXCEPT(ss.str());
    }
}

inline Ogre::HighLevelGpuProgramPtr load_and_validate_shader (const std::string &name)
{
    Ogre::HighLevelGpuProgramPtr prog = Ogre::HighLevelGpuProgramManager::getSingleton().getByName(name, RESGRP);
    std::stringstream ss;
    if (prog.isNull()) {
        ss << "Program not found: \"" << name << "\"";
        GRIT_EXCEPT(ss.str());
    }

    load_and_validate_shader(prog);
    return prog;
}

template<class T> void try_set_named_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                                               const std::string &name, const T &v, bool silent_fail=false)
{
    p->setIgnoreMissingParams(silent_fail);
    try {
        p->setNamedConstant(name,v);
    } catch (const Ogre::Exception &e) {
        if (e.getNumber() == Ogre::Exception::ERR_INVALIDPARAMS) {
                CLOG << "WARNING: " << e.getDescription() << std::endl;
        } else {
            throw e;
        }
    }
}   
    
template<class T> void try_set_named_constant (const Ogre::HighLevelGpuProgramPtr &p,
                                               const std::string &name, const T &v, bool silent_fail=false)
{
    try_set_named_constant(p->getDefaultParameters(), name, v, silent_fail);
}   
    
#endif

