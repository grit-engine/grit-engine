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

#include "gfx_internal.h"
#include "GfxLight.h"

static bool have_init_coronas = false;

static void ensure_coronas_init (void)
{
    if (have_init_coronas) return;
    // this only happens once
    gfx_particle_define("/system/Coronas", "/system/Corona.png", true, 0, true);
    have_init_coronas = true;
}

const std::string GfxLight::className = "GfxLight";

GfxLight::GfxLight (const GfxNodePtr &par_)
  : GfxNode(par_),
    enabled(true),
    fade(1),
    coronaLocalPos(0,0,0),
    coronaSize(1),
    coronaColour(1,1,1),
    diffuse(1,1,1),
    specular(1,1,1),
    aim(1,0,0,0),
    coronaInnerAngle(Degree(180)),
    coronaOuterAngle(Degree(180))
{
    light = ogre_sm->createLight();
    node->attachObject(light);
    light->setDirection(Ogre::Vector3(0,1,0));
    light->setAttenuation(10, 0, 0, 0);
    light->setSpotlightInnerAngle(Ogre::Degree(180));
    light->setSpotlightOuterAngle(Ogre::Degree(180));
    updateVisibility();
    ensure_coronas_init();
    corona = gfx_particle_emit("/system/Coronas");
    corona->setDefaultUV();
    corona->alpha = 1;
    corona->angle = 0;
    updateCorona(Vector3(0,0,0));
}

GfxLight::~GfxLight (void)
{
    if (!dead) destroy();
}

void GfxLight::destroy (void)
{
    if (dead) THROW_DEAD(className);
    if (light) ogre_sm->destroyLight(light);
    light = NULL;
    corona->release();
    GfxNode::destroy();
}

void GfxLight::updateCorona (const Vector3 &cam_pos)
{
    if (dead) THROW_DEAD(className);
    coronaPos = getWorldTransform() * coronaLocalPos;
    corona->pos = coronaPos;
    Vector3 col = enabled ? fade * coronaColour : Vector3(0,0,0);
    corona->dimensions = Vector3(coronaSize, coronaSize, coronaSize);

    Vector3 light_dir_ws = (cam_pos - getWorldTransform().pos).normalisedCopy();
    Vector3 light_aim_ws_ = getWorldTransform().removeTranslation() * Vector3(0,1,0);

    float angle = light_aim_ws_.dot(light_dir_ws);
    float inner = gritcos(coronaInnerAngle);
    float outer = gritcos(coronaOuterAngle);
    if (outer != inner) {
            float occlusion = std::min(std::max((angle-inner)/(outer-inner), 0.0f), 1.0f);
            col *= (1-occlusion);
    }
    corona->colour = col;
}

float GfxLight::getCoronaSize (void)
{
    if (dead) THROW_DEAD(className);
    return coronaSize;
}
void GfxLight::setCoronaSize (float v)
{
    if (dead) THROW_DEAD(className);
    coronaSize = v;
}

Vector3 GfxLight::getCoronaLocalPosition (void)
{
    if (dead) THROW_DEAD(className);
    return coronaLocalPos;
}

void GfxLight::setCoronaLocalPosition (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    coronaLocalPos = v;
}

Vector3 GfxLight::getCoronaColour (void)
{
    if (dead) THROW_DEAD(className);
    return coronaColour;
}

void GfxLight::setCoronaColour (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    coronaColour = v;
}

Vector3 GfxLight::getDiffuseColour (void)
{
    if (dead) THROW_DEAD(className);
    return diffuse;
}

Vector3 GfxLight::getSpecularColour (void)
{
    if (dead) THROW_DEAD(className);
    return specular;
}

void GfxLight::setDiffuseColour (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    diffuse = v;
    updateVisibility();
}

void GfxLight::setSpecularColour (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    specular = v;
    updateVisibility();
}

float GfxLight::getRange (void)
{
    if (dead) THROW_DEAD(className);
    return light->getAttenuationRange();
}

void GfxLight::setRange (float v)
{
    if (dead) THROW_DEAD(className);
    light->setAttenuation(v, 0, 0, 0);
}

Degree GfxLight::getInnerAngle (void)
{
    if (dead) THROW_DEAD(className);
    return from_ogre(light->getSpotlightInnerAngle());
}

void GfxLight::setInnerAngle (Degree v)
{
    if (dead) THROW_DEAD(className);
    light->setSpotlightInnerAngle(to_ogre(v));
}

Degree GfxLight::getOuterAngle (void)
{
    if (dead) THROW_DEAD(className);
    return from_ogre(light->getSpotlightOuterAngle());
}

void GfxLight::setOuterAngle (Degree v)
{
    if (dead) THROW_DEAD(className);
    light->setSpotlightOuterAngle(to_ogre(v));
}

Degree GfxLight::getCoronaInnerAngle (void)
{
    if (dead) THROW_DEAD(className);
    return coronaInnerAngle;
}

void GfxLight::setCoronaInnerAngle (Degree v)
{
    if (dead) THROW_DEAD(className);
    coronaInnerAngle = v;
}

Degree GfxLight::getCoronaOuterAngle (void)
{
    if (dead) THROW_DEAD(className);
    return coronaOuterAngle;
}

void GfxLight::setCoronaOuterAngle (Degree v)
{
    if (dead) THROW_DEAD(className);
    coronaOuterAngle = v;
}

bool GfxLight::isEnabled (void)
{
    if (dead) THROW_DEAD(className);
    return enabled;
}

void GfxLight::setEnabled (bool v)
{
    if (dead) THROW_DEAD(className);
    enabled = v;
    updateVisibility();
}

float GfxLight::getFade (void)
{
    if (dead) THROW_DEAD(className);
    return fade;
}
void GfxLight::setFade (float f)
{
    if (dead) THROW_DEAD(className);
    fade = f;
    updateVisibility();
}

void GfxLight::updateVisibility (void)
{
    light->setVisible(enabled && fade > 0.001);
    light->setDiffuseColour(to_ogre_cv(fade * diffuse));
    light->setSpecularColour(to_ogre_cv(fade * specular));
}
