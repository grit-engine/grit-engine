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

#include <string>
#include <sstream>

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
#include <OgreGLGpuProgram.h>
#include <OgreGLSLGpuProgram.h>
#include <OgreGLSLProgram.h>
#include <OgreGLSLLinkProgramManager.h>
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

#define NUM_GLOBAL_TEXTURES 8

extern bool d3d9;
extern Ogre::Root *ogre_root;
extern Ogre::RenderWindow *ogre_win;
extern Ogre::RenderSystem *ogre_rs;

extern Ogre::OctreeSceneManager *ogre_sm;
extern Ogre::SceneNode *ogre_root_node;
extern Ogre::Light *ogre_sun;
extern DiskResourcePtr<GfxEnvCubeDiskResource> global_env_cube0;
extern DiskResourcePtr<GfxEnvCubeDiskResource> global_env_cube1;
extern float global_env_cube_cross_fade;
extern Ogre::Matrix4 shadow_view_proj[3];
extern DiskResourcePtr<GfxTextureDiskResource> fade_dither_map;
extern DiskResourcePtr<GfxTextureDiskResource> shadow_pcf_noise_map;

static inline bool stereoscopic (void)
{ return gfx_option(GFX_CROSS_EYE) || gfx_option(GFX_ANAGLYPH); }

extern bool use_hwgamma;
extern GfxCallback *gfx_cb;
extern bool shutting_down;

extern float env_cube_cross_fade;
extern Vector3 particle_ambient;
extern Vector3 fog_colour;
extern float fog_density;
extern float env_brightness;
extern float global_exposure;
extern float global_saturation;
extern DiskResourcePtr<GfxColourGradeLUTDiskResource> colour_grade_lut;

extern Vector3 sun_direction;
extern Vector3 sunlight_direction;
extern Vector3 sun_colour;
extern Vector3 sunlight_diffuse;
extern Vector3 sunlight_specular;
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

void gfx_material_add_dependencies (const std::string &name, DiskResource *into);

void handle_dirty_materials (void);


class GfxShader;

typedef std::map<std::string,GfxShader*> GfxShaderDB;

extern GfxShaderDB shader_db;


template<class T> void try_set_named_constant_ (const Ogre::GpuProgramParametersSharedPtr &p,
                                                const std::string &name, const T &v, bool silent_fail)
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
    
/*
static inline void try_set_named_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                             const std::string &name, const Ogre::ColourValue &v, bool silent_fail=false)
{ try_set_named_constant_(p, name, v, silent_fail); }
*/

static inline void try_set_named_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                             const std::string &name, const Ogre::Matrix4 &v, bool silent_fail=false)
{ try_set_named_constant_(p, name, v, silent_fail); }

static inline void try_set_named_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                             const std::string &name, float v, bool silent_fail=false)
{ try_set_named_constant_(p, name, v, silent_fail); }

/*
void try_set_named_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                             const std::string &name, const Ogre::Vector2 &v, bool silent_fail=false)
{ try_set_named_constant_(p, name, v, silent_fail); }
*/

static inline void try_set_named_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                             const std::string &name, const Ogre::Vector3 &v, bool silent_fail=false)
{ try_set_named_constant_(p, name, v, silent_fail); }

static inline void try_set_named_constant (const Ogre::GpuProgramParametersSharedPtr &p,
                             const std::string &name, const Ogre::Vector4 &v, bool silent_fail=false)
{ try_set_named_constant_(p, name, v, silent_fail); }

template<class T> void try_set_named_constant (const Ogre::HighLevelGpuProgramPtr &p,
                                               const std::string &name, const T &v, bool silent_fail=false)
{ try_set_named_constant(p->getDefaultParameters(), name, v, silent_fail); }
    
#endif
