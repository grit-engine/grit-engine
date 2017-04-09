/* Copyright (c) The Grit Game Engine authors 2016
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

class GfxSpriteBody;
typedef SharedPtr<GfxSpriteBody> GfxSpriteBodyPtr;

#ifndef GFX_SPRITE_BODY_H
#define GFX_SPRITE_BODY_H

#include "gfx_fertile_node.h"
#include "gfx_particle_system.h"

class GfxSpriteBody : public GfxNode {
    protected:
    static const std::string className;
    bool enabled;
    // Particle's alpha is managed to be the product of these.
    float fade;
    float alpha;
    GfxParticle * const particle;

    GfxSpriteBody (const std::string &particle_name, const GfxNodePtr &par_);
    ~GfxSpriteBody ();

    public:
    static GfxSpriteBodyPtr make (const std::string &particle_name,
                              const GfxNodePtr &par_=GfxNodePtr(NULL))
    { return GfxSpriteBodyPtr(new GfxSpriteBody(particle_name, par_)); }
    
    Vector3 getDiffuse (void) const;
    void setDiffuse (const Vector3 &v);

    Vector3 getEmissive (void) const;
    void setEmissive (const Vector3 &v);

    Vector3 getDimensions (void) const;
    void setDimensions (const Vector3 &v);

    bool isEnabled (void) const;
    void setEnabled (bool v);

    float getAngle (void) const;
    void setAngle (float f);

    float getFade (void) const;
    void setFade (float f);

    float getAlpha (void) const;
    void setAlpha (float f);

    Vector2 getUV1 (void) const;
    void setUV1 (const Vector2 &v);

    Vector2 getUV2 (void) const;
    void setUV2 (const Vector2 &v);

    // Update particle position, float
    void update (void);

    void destroy (void);

    friend class SharedPtr<GfxSpriteBody>;
};

#endif
