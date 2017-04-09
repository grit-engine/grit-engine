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

#include "gfx_internal.h"
#include "gfx_sprite_body.h"

const std::string GfxSpriteBody::className = "GfxSpriteBody";

GfxSpriteBody::GfxSpriteBody (const std::string &particle_name, const GfxNodePtr &par_)
  : GfxNode(par_),
    enabled(true),
    fade(1),
    alpha(1),
    particle(gfx_particle_emit(particle_name))
{
    particle->dimensions = Vector3(1, 1, 1);
    particle->diffuse = Vector3(1, 1, 1);  // More like ambient colour than diffuse.
    particle->emissive = Vector3(0, 0, 0);
    particle->setDefaultUV();
    particle->angle = 0;
    update();
}

GfxSpriteBody::~GfxSpriteBody (void)
{
    if (!dead) destroy();
}

void GfxSpriteBody::destroy (void)
{
    if (dead) THROW_DEAD(className);
    particle->release();
    GfxNode::destroy();
}

void GfxSpriteBody::update (void)
{
    if (dead) THROW_DEAD(className);
    particle->pos = getWorldTransform() * Vector3(0, 0, 0);
    if (enabled) {
        particle->alpha = fade * alpha;
    } else {
        particle->alpha = 0;
    }
}

float GfxSpriteBody::getAlpha (void) const
{
    if (dead) THROW_DEAD(className);
    return alpha;
}
void GfxSpriteBody::setAlpha (float v)
{
    if (dead) THROW_DEAD(className);
    alpha = v;
}

Vector3 GfxSpriteBody::getDimensions (void) const
{
    if (dead) THROW_DEAD(className);
    return particle->dimensions;
}

void GfxSpriteBody::setDimensions (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    particle->dimensions = v;
}

Vector3 GfxSpriteBody::getDiffuse (void) const
{
    if (dead) THROW_DEAD(className);
    return particle->diffuse;
}

void GfxSpriteBody::setDiffuse (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    particle->diffuse = v;
}

Vector3 GfxSpriteBody::getEmissive (void) const
{
    if (dead) THROW_DEAD(className);
    return particle->emissive;
}

void GfxSpriteBody::setEmissive (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    particle->emissive = v;
}

float GfxSpriteBody::getAngle (void) const
{
    if (dead) THROW_DEAD(className);
    return particle->angle;
}

void GfxSpriteBody::setAngle (float v)
{
    if (dead) THROW_DEAD(className);
    particle->angle = v;
}

bool GfxSpriteBody::isEnabled (void) const
{
    if (dead) THROW_DEAD(className);
    return enabled;
}

void GfxSpriteBody::setEnabled (bool v)
{
    if (dead) THROW_DEAD(className);
    enabled = v;
}

float GfxSpriteBody::getFade (void) const
{
    if (dead) THROW_DEAD(className);
    return fade;
}
void GfxSpriteBody::setFade (float f)
{
    if (dead) THROW_DEAD(className);
    fade = f;
}
