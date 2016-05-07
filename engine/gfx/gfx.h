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

/* TODO:
 * shaders
 * materials
 */

#include "../vect_util.h"
#include "../shared_ptr.h"

// Only things that are referenced from Lua AND can be destroyed (before shutdown) get a SharedPtr
struct GfxCallback;
struct GfxLastRenderStats;
struct GfxLastFrameStats;
struct GfxRunningFrameStats;
struct GfxPaintColour;

typedef std::map<std::string, std::string> GfxStringMap;

// Used as a default param in some calls
extern const GfxStringMap gfx_empty_string_map;

#ifndef gfx_h
#define gfx_h

#include <string>

#include <math_util.h>

#include "gfx_disk_resource.h"
#include "gfx_node.h"

#include "gfx_particle_system.h"

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}


extern fast_erase_vector<GfxNode*> gfx_all_nodes;

struct GfxCallback {
    virtual ~GfxCallback (void) { }
    virtual void clickedClose (void) = 0;
    virtual void windowResized (int width, int height) = 0;
    virtual void messageLogged (const std::string &msg) = 0;
};


/** Are we in Direct3D, driven by environment var.
 * 
 * Can be used in static scope (i.e. to initialise globals before main().
 */
bool gfx_d3d9 (void);

size_t gfx_init (GfxCallback &cb);

void gfx_render (float elapsed, const Vector3 &cam_pos, const Quaternion &cam_dir);

void gfx_screenshot (const std::string &filename);

void gfx_bake_env_cube (const std::string &filename, unsigned size, const Vector3 &cam_pos, float saturation, const Vector3 &ambient);


// lighting parameters
GfxEnvCubeDiskResource *gfx_env_cube (unsigned i);
void gfx_env_cube (unsigned i, const DiskResourcePtr<GfxEnvCubeDiskResource> &v);
float gfx_env_cube_cross_fade (void);
void gfx_env_cube_cross_fade (float);

GfxTextureDiskResource *gfx_shadow_pcf_noise_map (void);
void gfx_shadow_pcf_noise_map (const DiskResourcePtr<GfxTextureDiskResource> &v);
GfxTextureDiskResource *gfx_fade_dither_map (void);
void gfx_fade_dither_map (const DiskResourcePtr<GfxTextureDiskResource> &v);
GfxTextureDiskResource *gfx_corona_map (void);
void gfx_corona_map (const DiskResourcePtr<GfxTextureDiskResource> &v);


Vector3 gfx_particle_ambient (void);
void gfx_particle_ambient (const Vector3 &v);

Vector3 gfx_sunlight_diffuse (void);
void gfx_sunlight_diffuse (const Vector3 &v);

Vector3 gfx_sunlight_specular (void);
void gfx_sunlight_specular (const Vector3 &v);

Vector3 gfx_sunlight_direction (void);
void gfx_sunlight_direction (const Vector3 &v);


// fog
Vector3 gfx_fog_colour (void);
void gfx_fog_colour (const Vector3 &v);

float gfx_fog_density (void);
void gfx_fog_density (float v);


// sun (as it appears as a disc in the sky)
Vector3 gfx_sun_direction (void);
void gfx_sun_direction (const Vector3 &v);

Vector3 gfx_sun_colour (void);
void gfx_sun_colour (const Vector3 &v);

float gfx_sun_alpha (void);
void gfx_sun_alpha (float v);

float gfx_sun_size (void);
void gfx_sun_size (float v);

float gfx_sun_falloff_distance (void);
void gfx_sun_falloff_distance (float v);


// sky
float gfx_sky_glare_sun_distance (void);
void gfx_sky_glare_sun_distance (float v);

float gfx_sky_glare_horizon_elevation (void);
void gfx_sky_glare_horizon_elevation (float v);

float gfx_sky_divider (unsigned i);
void gfx_sky_divider (unsigned i, float v);

Vector3 gfx_sky_colour (unsigned i);
void gfx_sky_colour (unsigned i, const Vector3 &v);

Vector3 gfx_sky_sun_colour (unsigned i);
void gfx_sky_sun_colour (unsigned i, const Vector3 &v);

float gfx_sky_alpha (unsigned i);
void gfx_sky_alpha (unsigned i, float v);

float gfx_sky_sun_alpha (unsigned i);
void gfx_sky_sun_alpha (unsigned i, float v);

Vector3 gfx_sky_cloud_colour (void);
void gfx_sky_cloud_colour (const Vector3 &v);

float gfx_sky_cloud_coverage (void);
void gfx_sky_cloud_coverage (float v);

Vector3 gfx_hell_colour (void);
void gfx_hell_colour (const Vector3 &v);


// tone mapping
GfxColourGradeLUTDiskResource *gfx_colour_grade (void);
void gfx_colour_grade (const DiskResourcePtr<GfxColourGradeLUTDiskResource> &v);

float gfx_global_saturation (void);
void gfx_global_saturation (float);

float gfx_global_exposure (void);
void gfx_global_exposure (float);


struct GfxLastRenderStats {
    float batches;
    float triangles;
    float micros;
    GfxLastRenderStats (const GfxLastRenderStats &o)
        : batches(o.batches), triangles(o.triangles), micros(o.micros) { }
    GfxLastRenderStats (void) : batches(0), triangles(0), micros(0) { }
    GfxLastRenderStats &operator+= (const GfxLastRenderStats &o) {
        batches += o.batches; triangles += o.triangles; micros += o.micros;
        return *this;
    }
    GfxLastRenderStats &operator/= (float time) {
        batches /= time; triangles /= time; micros /= time;
        return *this;
    }
};

struct GfxLastFrameStats {
    GfxLastRenderStats shadow[3];
    GfxLastRenderStats left_gbuffer;
    GfxLastRenderStats left_deferred;
    GfxLastRenderStats right_gbuffer;
    GfxLastRenderStats right_deferred;
    GfxLastRenderStats hud;
};

struct GfxRunningFrameStats {
    GfxLastFrameStats min;
    GfxLastFrameStats avg;
    GfxLastFrameStats max;
};

GfxLastFrameStats gfx_last_frame_stats (void);
GfxRunningFrameStats gfx_running_frame_stats (void);

enum GfxMaterialType {
    GFX_MATERIAL,
    GFX_SKY_MATERIAL
};

GfxMaterialType gfx_material_type (const std::string &name);

bool gfx_material_has_any (const std::string &name);


void gfx_shutdown_lua (lua_State *L);

void gfx_shutdown (void);

Vector3 gfx_colour_grade_look_up (const Vector3 &v);

bool gfx_window_active (void);
Vector2 gfx_window_size (void);
Vector2 gfx_window_size_in_scene (void);
Vector3 gfx_world_to_screen (const Vector3 &cam_pos, const Quaternion &cam_dir, const Vector3 &p);
Vector3 gfx_screen_to_world (const Vector3 &cam_pos, const Quaternion &cam_dir, const Vector2 &p);

// FIXME: everything below here is a horrible hack
extern Ogre::Root *ogre_root; // FIXME: hack
extern Ogre::OctreeSceneManager *ogre_sm; // FIXME: hack
extern Ogre::RenderWindow *ogre_win; // FIXME: hack


static inline Ogre::Vector2 to_ogre (const Vector2 &v)
{ return Ogre::Vector2(v.x, v.y); }
static inline Ogre::Vector3 to_ogre (const Vector3 &v)
{ return Ogre::Vector3(v.x, v.y, v.z); }
static inline Ogre::Vector4 to_ogre (const Vector4 &v)
{ return Ogre::Vector4(v.x, v.y, v.z, v.w); }
static inline Ogre::Quaternion to_ogre (const Quaternion &v)
{ return Ogre::Quaternion(v.w, v.x, v.y, v.z); }
static inline Ogre::ColourValue to_ogre_cv (const Vector3 &v)
{ return Ogre::ColourValue(v.x, v.y, v.z); }
static inline Ogre::Degree to_ogre (const Degree &v)
{ return Ogre::Degree(v.inDegrees()); }
static inline Ogre::Radian to_ogre (const Radian &v)
{ return Ogre::Degree(v.inRadians()); }

static inline Vector3 from_ogre (const Ogre::Vector3 &v)
{ return Vector3(v.x,v.y,v.z); }
static inline Quaternion from_ogre (const Ogre::Quaternion &v)
{ return Quaternion(v.w,v.x,v.y,v.z); }
static inline Vector3 from_ogre_cv (const Ogre::ColourValue &v)
{ return Vector3(v.r, v.g, v.b); }
static inline Degree from_ogre (const Ogre::Degree &v)
{ return Degree(v.valueDegrees()); }
static inline Radian from_ogre (const Ogre::Radian &v)
{ return Radian(v.valueRadians()); }

#endif 
