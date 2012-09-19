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

/* TODO:
 * clutter / rclutter
 * shaders
 * materials
 */

#include "../SharedPtr.h"
#include "../vect_util.h"

// Only things that are referenced from Lua AND can be destroyed (before shutdown) get a SharedPtr
class GfxPipeline;

#ifndef GfxPipeline_h
#define GfxPipeline_h

#include "gfx_internal.h"

extern float eye_separation;

void gfx_pipeline_init (void);

void do_reset_eyes (void);

class GfxPipeline {
    bool left;
    Ogre::Camera *cam;
    Vector3 viewPos;
    Quaternion viewDir;
    GfxLastRenderStats gBufferStats;
    GfxLastRenderStats deferredStats;

    // gbuffer target
    Ogre::TexturePtr gBufferElements[3];
    Ogre::MultiRenderTarget *gBuffer;
    Ogre::RenderQueueInvocationSequence *rqisGbuffer;

    Ogre::TexturePtr hdrFb[3];

    // ultimate target
    Ogre::RenderTarget *target;
    Ogre::Viewport *targetViewport;
    Ogre::RenderQueueInvocationSequence *rqisDeferred;


    public:
    GfxPipeline (const std::string &name, Ogre::RenderTarget *target, bool left);

    ~GfxPipeline (void);

    void render (const Vector3 &cam_pos, const Quaternion &cam_dir);

    void setNearClipDistance (float v) { cam->setNearClipDistance(v); }

    void setFarClipDistance (float v) { cam->setFarClipDistance(v); }

    void setFOVy (float v) { cam->setFOVy(Ogre::Degree(v)); }

    void setFrustumOffset (float v) { cam->setFrustumOffset(v); }
    void setFocalLength (float v) { cam->setFocalLength(v); }

    const GfxLastRenderStats &getGBufferStats (void) { return gBufferStats; }
    const GfxLastRenderStats &getDeferredStats (void) { return deferredStats; }
    const Vector3 &getViewPos (void) { return viewPos; }
    const Quaternion &getViewDir (void) { return viewDir; }

    Ogre::Camera *getCamera (void) { return cam; }
    Ogre::RenderTarget *getTarget (void) { return target; }
    Ogre::Viewport *getTargetViewport (void) { return targetViewport; }
    const Ogre::TexturePtr &getGBufferTexture (unsigned i) { return gBufferElements[i]; }
};

extern GfxPipeline *eye_left, *eye_right;

#endif 
