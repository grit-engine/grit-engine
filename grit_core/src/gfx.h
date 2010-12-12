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

#if 0
some way of distinguishing sky from everything else

scnmgr
entity
manobj
light
clutter
rclutter
psys
viewport

mat
tex
mesh
skel
gpuprog
rtex
rwin


Script or user customisable graphical elements within a typical grit game:

Entities, Entity hierarchy, Bone hierarchy
Clutter
Lights (high level lights with coronas etc)
Particles
Material definitions
Sky attributes
Ambient lighting
Shadow config
Shaders?
screenshot

no need to touch meshes/textures except to reload -- probably will have reload_mesh(string) or similar

#endif

#ifndef gfx_h
#define gfx_h

#include <string>

#include "HUD.h"
#include "math_util.h"
#include "SharedPtr.h"

struct GfxCallback {
    virtual ~GfxCallback (void) { }
    virtual void clickedClose (void) = 0;
    virtual void windowResized (int width, int height) = 0;
    virtual void messageLogged (const std::string &msg) = 0;
};

size_t gfx_init (GfxCallback &cb);

void gfx_render (float elapsed, const Vector3 &cam_pos, const Quaternion &cam_dir, float cam_chase);
void gfx_pump();

enum GfxBoolOption {
    GFX_AUTOUPDATE,
    GFX_SHADOW_RECEIVE,
    GFX_SHADOW_CAST,
    GFX_VSYNC,
    GFX_FULLSCREEN,
    GFX_FOG,
    GFX_DEFERRED,
    GFX_WIREFRAME,
    GFX_ANAGLYPH,
    GFX_CROSS_EYE,
    GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST,
    GFX_SHADOW_AGGRESSIVE_FOCUS_REGION
};

enum GfxIntOption {
    GFX_FULLSCREEN_WIDTH,
    GFX_FULLSCREEN_HEIGHT,
    GFX_SHADOW_RES,
    GFX_RAM
};

enum GfxFloatOption {
    GFX_FOV,
    GFX_NEAR_CLIP,
    GFX_FAR_CLIP,
    GFX_EYE_SEPARATION, // FOR 'real' 3d
    GFX_MONITOR_HEIGHT, // FOR 'real' 3d
    GFX_MONITOR_EYE_DISTANCE, // FOR 'real' 3d
    GFX_MIN_PERCEIVED_DEPTH, // FOR 'real' 3d -- distance from eyes of the front clip plane
    GFX_MAX_PERCEIVED_DEPTH, // FOR 'real' 3d -- distance from eyes of the rear clip plane
    GFX_SHADOW_START,
    GFX_SHADOW_END0,
    GFX_SHADOW_END1,
    GFX_SHADOW_END2,
    GFX_SHADOW_OPTIMAL_ADJUST0,
    GFX_SHADOW_OPTIMAL_ADJUST1,
    GFX_SHADOW_OPTIMAL_ADJUST2,
    GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD,
    GFX_SHADOW_PADDING,
    GFX_ANAGLYPH_LEFT_RED_MASK,
    GFX_ANAGLYPH_LEFT_GREEN_MASK,
    GFX_ANAGLYPH_LEFT_BLUE_MASK,
    GFX_ANAGLYPH_RIGHT_RED_MASK,
    GFX_ANAGLYPH_RIGHT_GREEN_MASK,
    GFX_ANAGLYPH_RIGHT_BLUE_MASK,
    GFX_ANAGLYPH_DESATURATION
};

std::string gfx_option_to_string (GfxBoolOption o);
std::string gfx_option_to_string (GfxIntOption o);
std::string gfx_option_to_string (GfxFloatOption o);

// set's t to either 0,1,2 and fills in the approriate argument
void gfx_option_from_string (const std::string &s,
                             int &t,
                             GfxBoolOption &o0,
                             GfxIntOption &o1,
                             GfxFloatOption &o2);

void gfx_option (GfxBoolOption o, bool v);
bool gfx_option (GfxBoolOption o);
void gfx_option (GfxIntOption o, int v);
int gfx_option (GfxIntOption o);
void gfx_option (GfxFloatOption o, float v);
float gfx_option (GfxFloatOption o);

class GfxBody {
    protected:
    Vector3 pos; Quaternion quat; Vector3 scl;
    GfxBody *par;
    Ogre::Entity *ent;
    Ogre::SceneNode *node;

    GfxBody (const std::string &mesh_name);

    GfxBody *getParent (void) const { return par; }
    void setParent(GfxBody *par_);

    Vector3 transformPosition (const Vector3 &v)
    {
        if (par==NULL) return v;
        return getWorldPosition() + getWorldScale()*(getWorldOrientation()*v);
    }
    Quaternion transformOrientation (const Quaternion &v)
    {
        if (par==NULL) return v;
        return getWorldOrientation()*v;
    }
    Vector3 transformScale (const Vector3 &v)
    {
        if (par==NULL) return v;
        return getWorldScale()*v;
    }

    const Vector3 &getLocalPosition (void) { return pos; }
    void setLocalPosition (const Vector3 &p);
    const Vector3 getWorldPosition (void)
    { return par==NULL ? pos : par->transformPosition(pos); }

    const Quaternion &getLocalOrientation (void) { return quat; }
    void setLocalOrientation (const Quaternion &q);
    const Quaternion getWorldOrientation (void)
    {return par==NULL ? quat : par->transformOrientation(quat); }

    const Vector3 &getLocalScale (void) { return scl; }
    void setLocalScale (const Vector3 &s);
    const Vector3 getWorldScale (void)
    { return  par==NULL? scl : par->transformScale(scl); }
};

typedef SharedPtr<GfxBody> GfxBodyPtr;

void gfx_screenshot (const std::string &filename);

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
    GfxLastRenderStats left;
    GfxLastRenderStats right;
};

struct GfxRunningFrameStats {
    GfxLastFrameStats min;
    GfxLastFrameStats avg;
    GfxLastFrameStats max;
};

GfxLastFrameStats gfx_last_frame_stats (void);
GfxRunningFrameStats gfx_running_frame_stats (void);

void gfx_reload_resources (void);

HUD::RootPtr gfx_init_hud (void);

void gfx_shutdown (void);

// FIXME: everything below here is a horrible hack
#include <OgreRoot.h>
#include <OgreOctreeSceneManager.h>
#include <OgreRenderWindow.h>
extern Ogre::Root *ogre_root; // FIXME: hack
extern Ogre::OctreeSceneManager *ogre_sm; // FIXME: hack
extern Ogre::RenderWindow *ogre_win; // FIXME: hack


static inline Ogre::Vector3 to_ogre (const Vector3 &v)
{ return Ogre::Vector3(v.x,v.y,v.z); }
static inline Ogre::Quaternion to_ogre (const Quaternion &v)
{ return Ogre::Quaternion(v.w,v.x,v.y,v.z); }

static inline Vector3 from_ogre (const Ogre::Vector3 &v)
{ return Vector3(v.x,v.y,v.z); }
static inline Quaternion from_ogre (const Ogre::Quaternion &v)
{ return Quaternion(v.w,v.x,v.y,v.z); }

#endif 
