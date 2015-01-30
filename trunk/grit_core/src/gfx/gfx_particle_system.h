/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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
class GfxParticle;
class GfxPipeline;


#ifndef GfxParticleSystem_h
#define GfxParticleSystem_h

#include <utility>

#include "../vect_util.h"
#include <math_util.h>


// Modify particle attributes whenever you want
class GfxParticle : public fast_erase_index {
protected:
    GfxParticleSystem *sys;
public:
    Vector3 pos;
    Vector3 dimensions;
    Vector3 diffuse;
    Vector3 emissive;
    float alpha;
    float angle;
    float u1, v1, u2, v2;

    // these guys computed at rending time
    Vector3 fromCamNorm;
    float fromCamDist;
    // by this function
    void preProcess (const Vector3 &cam_pos);

    GfxParticle (GfxParticleSystem *sys);

    std::pair<unsigned,unsigned> getTextureSize (void);
    void setDefaultUV (void);
    void release (void);
    bool inside (const Vector3 &v);
};

// called once during program init
void gfx_particle_init (void);

// set up a new particle system
void gfx_particle_define (const std::string &pname, const std::string &tex_name);

// create a new particle in a given system (get rid of it by calling particle->release())
GfxParticle *gfx_particle_emit (const std::string &pname);

std::vector<std::string> gfx_particle_all (void);

// called every frame
void gfx_particle_render (GfxPipeline *p);

#endif
