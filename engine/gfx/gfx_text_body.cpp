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

#include <centralised_log.h>

#include "gfx_internal.h"

#include "gfx_text_body.h"

const std::string GfxTextBody::className = "GfxTextBody";

// {{{ Sub

unsigned short GfxTextBody::Sub::getNumWorldTransforms(void) const
{
    return 1;
}

void GfxTextBody::Sub::getWorldTransforms(Ogre::Matrix4 *xform) const
{
    *xform = parent->toOgre();
}

Ogre::Real GfxTextBody::Sub::getSquaredViewDepth (const Ogre::Camera *cam) const
{
    Ogre::Vector3 diff = to_ogre(parent->worldTransform.pos) - cam->getDerivedPosition();
    return diff.squaredLength();
}

static Ogre::LightList EMPTY_LIGHTS;

const Ogre::LightList& GfxTextBody::Sub::getLights (void) const
{
    return EMPTY_LIGHTS;
}

bool GfxTextBody::Sub::getCastShadows (void) const
{
    return parent->getCastShadows();
}

const Ogre::MaterialPtr& GfxTextBody::Sub::getMaterial(void) const
{
    return parent->renderMaterial;
}

// }}}





// {{{ RENDERING

void GfxTextBody::_updateRenderQueue(Ogre::RenderQueue* queue)
{
    bool shadow_cast =
        ogre_sm->_getCurrentRenderStage() == Ogre::SceneManager::IRS_RENDER_TO_TEXTURE;

    if (!enabled || fade < 0.000001) return;

    bool do_wireframe = gfx_option(GFX_WIREFRAME);
    bool do_regular = !do_wireframe || gfx_option(GFX_WIREFRAME_SOLID);

    // fade is used by both shadow_cast and regular pass
    // we could potentially move this out of the frame loop if it affects performance
    sub.setCustomParameter(0, Ogre::Vector4(fade,0,0,0));

    if (shadow_cast) {

        // Ogre chases ->getTechnique(0)->getShadowCasterMaterial() to get the actual one
        // which is material->castMat
        renderMaterial = material->regularMat;
        queue->addRenderable(&sub, 0, 0);
        renderMaterial.setNull();
        return;
    }

    if (do_regular) {
        // TODO: Warn if mesh does not have required vertex attributes for this material
        // taking into account dead code due to particular uniform values.
        // E.g. vertex colours, alpha, normals, tangents, extra tex coords.


        /* TODO(dcunnin): Pick a specific material by
         * Fading: false/true
         */

        renderMaterial = material->regularMat;

        int queue_group = RQ_GBUFFER_OPAQUE;
        switch (material->getSceneBlend()) {
            case GFX_MATERIAL_OPAQUE:      queue_group = RQ_GBUFFER_OPAQUE; break;
            case GFX_MATERIAL_ALPHA:       queue_group = RQ_FORWARD_ALPHA; break;
            case GFX_MATERIAL_ALPHA_DEPTH: queue_group = RQ_FORWARD_ALPHA_DEPTH; break;
        }
        queue->addRenderable(&sub, queue_group, 0);

        if (material->getAdditionalLighting() && emissiveEnabled) {
            renderMaterial = material->additionalMat;
            switch (material->getSceneBlend()) {
                case GFX_MATERIAL_OPAQUE:      queue_group = RQ_FORWARD_OPAQUE_EMISSIVE; break;
                case GFX_MATERIAL_ALPHA:       queue_group = RQ_FORWARD_ALPHA_EMISSIVE; break;
                case GFX_MATERIAL_ALPHA_DEPTH: queue_group = RQ_FORWARD_ALPHA_DEPTH_EMISSIVE; break;
            }
            queue->addRenderable(&sub, queue_group, 0);
        }
    }

    if (do_wireframe) {
        renderMaterial = material->wireframeMat;
        queue->addRenderable(&sub, RQ_FORWARD_ALPHA, 0);
    }

    renderMaterial.setNull();

}

void GfxTextBody::visitRenderables(Ogre::Renderable::Visitor* visitor, bool)
{
    // Internally accesses renderMaterial.
    renderMaterial = material->regularMat;
    visitor->visit(&sub, 0, false);
    renderMaterial.setNull();
}


void GfxTextBody::recomputeBounds (void)
{
    Vector2 dim = textBuffer.getDrawnDimensions();
    Vector3 min_bound(0, -dim.y, 0);
    Vector3 max_bound(dim.x, 0, 0);
    bounds = Ogre::AxisAlignedBox(to_ogre(min_bound), to_ogre(max_bound));
}

Ogre::Real GfxTextBody::getBoundingRadius(void) const
{
    return textBuffer.getDrawnDimensions().length();
}

// }}}









GfxTextBodyPtr GfxTextBody::make (GfxFont *font, GfxMaterial *mat, const GfxNodePtr &par_)
{
    return GfxTextBodyPtr(new GfxTextBody(font, mat, par_));
}

GfxTextBody::GfxTextBody (GfxFont *font, GfxMaterial *mat, const GfxNodePtr &par_)
  : GfxFertileNode(par_), sub(this), material(mat), emissiveEnabled(true),
    fade(1), textBuffer(font), enabled(true), castShadows(true)
{
    node->attachObject(this);
}

GfxTextBody::~GfxTextBody (void)
{
    if (!dead) destroy();
}

void GfxTextBody::destroy (void)
{
    if (dead) THROW_DEAD(className);
    GfxFertileNode::destroy();
}

GfxMaterial *GfxTextBody::getMaterial (void) const
{
    if (dead) THROW_DEAD(className);
    return material;
}

void GfxTextBody::setMaterial (GfxMaterial *m)
{
    if (dead) THROW_DEAD(className);
    material = m;
}

bool GfxTextBody::getEmissiveEnabled (void) const
{
    if (dead) THROW_DEAD(className);
    return emissiveEnabled;
}

void GfxTextBody::setEmissiveEnabled (bool v)
{
    if (dead) THROW_DEAD(className);
    emissiveEnabled = v;
}

unsigned GfxTextBody::getBatches (void) const
{
    if (dead) THROW_DEAD(className);
    return 1;
}

unsigned GfxTextBody::getBatchesWithChildren (void) const
{
    if (dead) THROW_DEAD(className);
    return getBatches() + GfxFertileNode::getBatchesWithChildren();
}

unsigned GfxTextBody::getVertexes (void) const
{
    if (dead) THROW_DEAD(className);
    return textBuffer.getVertexes();
}

unsigned GfxTextBody::getVertexesWithChildren (void) const
{
    if (dead) THROW_DEAD(className);
    return getVertexes() + GfxFertileNode::getVertexesWithChildren();
}

unsigned GfxTextBody::getTriangles (void) const
{
    if (dead) THROW_DEAD(className);
    return textBuffer.getTriangles();
}

unsigned GfxTextBody::getTrianglesWithChildren (void) const
{
    if (dead) THROW_DEAD(className);
    return getTriangles() + GfxFertileNode::getTrianglesWithChildren();
}

float GfxTextBody::getFade (void) const
{
    if (dead) THROW_DEAD(className);
    return fade;
}
void GfxTextBody::setFade (float f)
{
    if (dead) THROW_DEAD(className);
    fade = f;
}

bool GfxTextBody::getCastShadows (void) const
{
    if (dead) THROW_DEAD(className);
    return castShadows;
}
void GfxTextBody::setCastShadows (bool v)
{
    if (dead) THROW_DEAD(className);
    castShadows = v;
}

bool GfxTextBody::isEnabled (void) const
{
    if (dead) THROW_DEAD(className);
    return enabled;
}

void GfxTextBody::setEnabled (bool v)
{
    if (dead) THROW_DEAD(className);
    enabled = v;
}

