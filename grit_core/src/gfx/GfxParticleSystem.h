/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

class GfxParticleSystem;
struct GfxParticle;


#ifndef GfxParticleSystem_h
#define GfxParticleSystem_h

#include "../vect_util.h"
#include "../math_util.h"

struct GfxParticle {
    GfxParticleSystem *sys;
    void *internal;

    GfxParticle (void) { }

    GfxParticle (GfxParticleSystem *sys);

    GfxParticle &operator= (const GfxParticle &other);

    void setPosition (const Vector3 &v);
    void setAmbient (const Vector3 &v);
    void setAlpha (float v);
    void setAngle (float v);
    void setUV (float u1, float v1, float u2, float v2);
    void setDefaultUV (void);
    void setWidth (float v);
    void setHeight (float v);
    void setDepth (float v);

    void release (void);
};

enum GfxParticleSceneBlend { GFX_PARTICLE_OPAQUE, GFX_PARTICLE_ALPHA, GFX_PARTICLE_ADD, GFX_PARTICLE_OCCLUDE_ADD };

void gfx_particle_define (const std::string &pname, const std::string &tex_name,
                          GfxParticleSceneBlend blend, float alpha_rej, bool emissive);
GfxParticle gfx_particle_emit (const std::string &pname);

#endif
