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

#define THROW_DEAD(name) do { GRIT_EXCEPT(name+" has been destroyed."); } while (0)

#include <Ogre.h>
#include <OgreFontManager.h>
#include <OgreMeshManager.h>
#include <OgreOverlayElementFactory.h>
#include <OgreOctreeSceneManager.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreCustomCompositionPass.h>
#include <OgreCompositor.h>
#ifdef NO_PLUGINS
#  include "OgreOctreePlugin.h"
#  include "OgreGLPlugin.h"
#  include "OgreCgPlugin.h"
#  ifdef WIN32
#    define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) // workaround for main.h preventing d3d9.h from including properly
#    include "OgreD3D9Plugin.h"
#  endif
#endif

#include "gfx.h"
#include "gfx_option.h"

#define ANAGLYPH_COMPOSITOR "system/AnaglyphCompositor"
#define ANAGLYPH_FB_TEXTURE "system/AnaglyphFB"
#define ANAGLYPH_COMPOSITOR_MATERIAL "system/AnaglyphCompositorMaterialDesaturated"

#define DEFERRED_COMPOSITOR "/system/CoreCompositor"
#define DEFERRED_AMBIENT_SUN_MATERIAL "/system/DeferredAmbientSun"
#define DEFERRED_POINT_LIGHT_MATERIAL "/system/DeferredLights"
#define DEFERRED_POINT_LIGHT_INSIDE_MATERIAL "/system/DeferredLightsInside"
#define DEFERRED_LIGHTS_CUSTOM_PASS "DeferredLights"
#define CORONAS_CUSTOM_PASS "Coronas"

#define RESGRP "GRIT"

// render queues (also baked into system/Core.program)
// also hardcoded for ranged clutter in pbats common/map_classes
#define RQ_GBUFFER_OPAQUE 10
#define RQ_FORWARD_OPAQUE 20
#define RQ_SKY 30
#define RQ_FORWARD_EMISSIVE 40
#define RQ_SKY_ALPHA 50
#define RQ_FORWARD_ALPHA_DEPTH 60
#define RQ_FORWARD_ALPHA 70

#ifndef GFX_INTERNAL_H
#define GFX_INTERNAL_H

extern Ogre::Root *ogre_root;
extern Ogre::OctreeSceneManager *ogre_sm;
extern Ogre::SceneNode *ogre_root_node;
extern Ogre::Camera *left_eye;
extern Ogre::Camera *right_eye;
extern Ogre::Light *ogre_sun;
extern Ogre::SceneNode *ogre_celestial;
extern Ogre::SceneNode *ogre_sky_node;
extern Ogre::Entity *ogre_sky_ent;
extern Ogre::Viewport *overlay_vp;
extern Ogre::Viewport *left_vp;
extern Ogre::Viewport *right_vp;
extern Ogre::TexturePtr anaglyph_fb;
extern Ogre::RenderWindow *ogre_win;
#ifdef NO_PLUGINS
    extern Ogre::GLPlugin *gl;
    extern Ogre::OctreePlugin *octree;
    extern Ogre::CgPlugin *cg;
    #ifdef WIN32
        extern Ogre::D3D9Plugin *d3d9;
    #endif
#endif

static inline bool stereoscopic (void)
{ return gfx_option(GFX_CROSS_EYE) || gfx_option(GFX_ANAGLYPH); }

extern bool use_hwgamma;
extern GfxCallback *gfx_cb;
extern bool shutting_down;
extern float cam_separation;

extern Vector3 fog_colour;
extern float fog_density;

std::string freshname (const std::string &prefix);
std::string freshname (void);

void clean_compositors (void);
void do_reset_framebuffer (void);
void do_reset_compositors (void);

// API consumers should not see this
class GfxBaseMaterial {

    public:

    virtual ~GfxBaseMaterial (void) { }

    virtual void addDependencies (GfxDiskResource *into) = 0;

};

typedef std::map<std::string,GfxBaseMaterial*> GfxMaterialDB;

extern GfxMaterialDB material_db;

void gfx_material_add_dependencies (const std::string &name, GfxDiskResource *into);

void handle_dirty_materials (void);
void handle_dirty_sky_materials (void);


#endif

