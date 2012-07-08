/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

#include <string>
#include <map>

#include "gfx_internal.h"

static void reset_particle_material (const Ogre::MaterialPtr &mat, std::string texname,
                                     GfxParticleSceneBlend blend, float alphaRej, bool emissive)
{
    mat->removeAllTechniques();
    Ogre::Pass *pass = mat->createTechnique()->createPass();
    {
        Ogre::TextureUnitState *tus = pass->createTextureUnitState();
        tus->setTextureFiltering(Ogre::FO_POINT, Ogre::FO_POINT, Ogre::FO_NONE);
        tus->setContentType(Ogre::TextureUnitState::CONTENT_COMPOSITOR);
        tus->setCompositorReference(DEFERRED_COMPOSITOR,"fat_fb",0);
    }
    {
        Ogre::TextureUnitState *tus = pass->createTextureUnitState();
        tus->setTextureName(texname);
        tus->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
    }
    std::string gpuprog_name;
    switch (blend) {
        case GFX_PARTICLE_OPAQUE:
        pass->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ZERO);
        pass->setDepthWriteEnabled(true);
        gpuprog_name = "O";
        break;
        case GFX_PARTICLE_ALPHA:
        pass->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
        pass->setDepthWriteEnabled(false);
        gpuprog_name = "A";
        break;
        case GFX_PARTICLE_ADD:
        pass->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);
        pass->setDepthWriteEnabled(false);
        gpuprog_name = "L";
        break;
        case GFX_PARTICLE_OCCLUDE_ADD:
        pass->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
        pass->setDepthWriteEnabled(false);
        gpuprog_name = "P";
        break;
    }
    gpuprog_name += "_";
    gpuprog_name += emissive ? "E" : "e";
    pass->setAlphaRejectFunction(Ogre::CMPF_GREATER_EQUAL);
    pass->setAlphaRejectValue(alphaRej);
    pass->setFragmentProgram("particle_f:"+gpuprog_name);
    pass->setVertexProgram("particle_v:"+gpuprog_name);
}

// a particle system holds the buffer for particles of a particular material
class GfxParticleSystem {
    Ogre::BillboardSet *bbset;
    float texHeight;
    float texWidth;
    public:
    GfxParticleSystem (const std::string &pname, std::string texname,
                       GfxParticleSceneBlend blend, float alphaRej, bool emissive)
    {
        bbset = ogre_sm->createBillboardSet(pname, 100);
        ogre_root_node->attachObject(bbset);
        bbset->setUseAccurateFacing(true);

        std::string mname = "P:"+pname;
        texname = texname.substr(1);
        Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().load(texname, RESGRP);
        texHeight = tex->getHeight();
        texWidth = tex->getWidth();
        Ogre::MaterialPtr mat =
            Ogre::MaterialManager::getSingleton().createOrRetrieve(mname, RESGRP).first;
        reset_particle_material(mat, texname, blend, alphaRej, emissive);
        bbset->setMaterialName(mname);

        bbset->setBillboardRotationType(Ogre::BBR_VERTEX);
        if (blend==GFX_PARTICLE_OPAQUE) {
            bbset->setRenderQueueGroup(RQ_FORWARD_OPAQUE);
            bbset->setSortingEnabled(false);
        } else {
            bbset->setRenderQueueGroup(RQ_FORWARD_ALPHA);
            bbset->setSortingEnabled(true);
        }
        //bbset->setBillboardsInWorldSpace(true);
        bbset->setBounds(Ogre::AxisAlignedBox::BOX_INFINITE, 100000);
    }
    ~GfxParticleSystem (void)
    {
        ogre_sm->destroyBillboardSet(bbset);
    }

    GfxParticle emit (void)
    {
        return GfxParticle (this);
    }

    void *acquire (void)
    {
        return bbset->createBillboard(Ogre::Vector3(0,0,0));
    }

    void release (void *bb_)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bbset->removeBillboard(bb);
    }

    void setPosition (void *bb_, const Vector3 &v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setPosition(to_ogre(v));
    }

    void setAmbient (void *bb_, const Vector3 &v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        Ogre::ColourValue cv = bb->getColour();
        cv.r = v.x;
        cv.g = v.y;
        cv.b = v.z;
        bb->setColour(cv);
    }

    void setAlpha (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        Ogre::ColourValue cv = bb->getColour();
        cv.a = v;
        bb->setColour(cv);
    }

    void setAngle (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setRotation(Ogre::Degree(v));
    }

    void setUV (void *bb_, float u1, float v1, float u2, float v2)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setTexcoordRect(u1/texWidth, v1/texHeight, u2/texWidth, v2/texHeight);
    }

    void setDefaultUV (void *bb_)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setTexcoordRect(0.5/texWidth, 0.5/texHeight,
                            (texWidth-0.5)/texWidth, (texHeight-0.5)/texHeight);
    }

    void setWidth (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setDimensions(v, bb->getOwnHeight());
    }

    void setHeight (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setDimensions(bb->getOwnWidth(), v);
    }

    void setDepth (void *bb_, float v)
    {
        Ogre::Billboard *bb = static_cast<Ogre::Billboard*>(bb_);
        bb->setBias(-v/2);
    }

};

static std::map<std::string, GfxParticleSystem*> psystems;

GfxParticle::GfxParticle (GfxParticleSystem *sys_)
  : sys(sys_)
{
    internal = sys->acquire();
}

GfxParticle &GfxParticle::operator= (const GfxParticle &other)
{
    sys = other.sys;
    internal = other.internal;
    return *this;
}

void GfxParticle::setPosition (const Vector3 &v)
{ sys->setPosition(internal, v); }

void GfxParticle::setAmbient (const Vector3 &v)
{ sys->setAmbient(internal, v); }

void GfxParticle::setAlpha (float v)
{ sys->setAlpha(internal, v); }

void GfxParticle::setAngle (float v)
{ sys->setAngle(internal, v); }

void GfxParticle::setUV (float u1, float v1, float u2, float v2)
{ sys->setUV(internal, u1,v1,u2,v2); }

void GfxParticle::setDefaultUV (void)
{ sys->setDefaultUV(internal); }

void GfxParticle::setWidth (float v)
{ sys->setWidth(internal, v); }

void GfxParticle::setHeight (float v)
{ sys->setHeight(internal, v); }

void GfxParticle::setDepth (float v)
{ sys->setDepth(internal, v); }

void GfxParticle::release (void)
{ sys->release(internal); }

void gfx_particle_define (const std::string &pname, const std::string &tex_name,
                          GfxParticleSceneBlend blend, float alpha_rej, bool emissive)
{
    GfxParticleSystem *&psys = psystems[pname];
    if (psys != NULL) delete psys;
    psys = new GfxParticleSystem(pname, tex_name, blend, alpha_rej, emissive);
}

GfxParticle gfx_particle_emit (const std::string &pname)
{
    GfxParticleSystem *&psys = psystems[pname];
    if (psys == NULL) GRIT_EXCEPT("No such particle: \""+pname+"\"");
    return psys->emit();
}

