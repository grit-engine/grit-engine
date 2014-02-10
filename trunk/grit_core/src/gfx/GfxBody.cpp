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

#include "OgreSceneManager.h"
#include "OgreSkeleton.h"
#include "OgreAxisAlignedBox.h"
#include "OgreVector4.h"
#include "OgreSkeletonInstance.h"
#include "OgreAnimation.h"
#include "OgreOptimisedUtil.h"
#include "OgreSubMesh.h"

#include "../CentralisedLog.h"

#include "gfx_internal.h"

#include "GfxBody.h"

const std::string GfxBody::className = "GfxBody";


// {{{ Sub

unsigned short GfxBody::Sub::getNumWorldTransforms(void) const
{
    if (!parent->numBoneMatrices) {
        // No skeletal animation, or software skinning
        return 1;
    }

    // Hardware skinning, count all actually used matrices
    const Ogre::Mesh::IndexMap& indexMap = subMesh->useSharedVertices ?
        subMesh->parent->sharedBlendIndexToBoneIndexMap : subMesh->blendIndexToBoneIndexMap;
    APP_ASSERT(indexMap.size() <= parent->numBoneMatrices);

    return static_cast<unsigned short>(indexMap.size());
}

void GfxBody::Sub::getWorldTransforms(Ogre::Matrix4* xform) const
{
    if (!parent->numBoneMatrices) {
        // No skeletal animation, or software skinning
        *xform = parent->_getParentNodeFullTransform();
        return;
    }

    // Hardware skinning, pass all actually used matrices
    const Ogre::Mesh::IndexMap& indexMap = subMesh->useSharedVertices ?
        subMesh->parent->sharedBlendIndexToBoneIndexMap : subMesh->blendIndexToBoneIndexMap;
    APP_ASSERT(indexMap.size() <= parent->numBoneMatrices);

    if (parent->boneWorldMatrices) {

        // Bones, use cached matrices built when _updateRenderQueue was called
        for (Ogre::Mesh::IndexMap::const_iterator i=indexMap.begin(),i_=indexMap.end() ; i!=i_; ++i) {
            *(xform++) = parent->boneWorldMatrices[*i];
        }

    } else {

        // All animations disabled, use parent GfxBody world transform only
        std::fill_n(xform, indexMap.size(), parent->_getParentNodeFullTransform());
    }
}

Ogre::Real GfxBody::Sub::getSquaredViewDepth (const Ogre::Camera* cam) const
{
    return parent->getParentSceneNode()->getSquaredViewDepth(cam);
}

const Ogre::LightList& GfxBody::Sub::getLights (void) const
{
    return parent->queryLights();
}

bool GfxBody::Sub::getCastShadows (void) const
{
    return parent->getCastShadows();
}

const Ogre::MaterialPtr& GfxBody::Sub::getMaterial(void) const
{
    return parent->renderMaterial;
}


// }}}





// {{{ RENDERING

void GfxBody::updateBoneMatrixes (void)
{
    if (skeleton) {

        //update the current hardware animation state
        skeleton->setAnimationState(animationState);
        skeleton->_getBoneMatrices(boneMatrices);

        updateWorldTransform();

        Ogre::OptimisedUtil::getImplementation()->concatenateAffineMatrices(
            _getParentNodeFullTransform(),
            boneMatrices,
            boneWorldMatrices,
            numBoneMatrices);
    }
}

void GfxBody::_updateRenderQueue(Ogre::RenderQueue* queue)
{
    bool shadow_cast = ogre_sm->_getCurrentRenderStage() == Ogre::SceneManager::IRS_RENDER_TO_TEXTURE;

    if (!enabled || fade < 0.000001) return;

    bool do_wireframe = (wireframe || gfx_option(GFX_WIREFRAME));
    bool do_regular = !(do_wireframe && gfx_option(GFX_WIREFRAME_SOLID));

    if (shadow_cast) {

        // Add each visible Sub to the queue
        for (unsigned i=0 ; i<subList.size() ; ++i) {

            Sub *sub = subList[i];

            GfxMaterial *m = sub->material;

            if (!sub->getCastShadows()) continue;
            if (!m->getCastShadows()) continue;

            renderMaterial = m->regularMat;

            queue->addRenderable(sub, 0, 0);
        }

    } else {

        freshFrame = true;

        // Add each visible Sub to the queue
        for (unsigned i=0 ; i<subList.size() ; ++i) {

            Sub *sub = subList[i];

            // car paint
            sub->setCustomParameter(0, Ogre::Vector4(fade,0,0,0));
            for (int k=0 ; k<4 ; ++k) {
                const GfxPaintColour &c = colours[k];
                sub->setCustomParameter(2*k+1, Ogre::Vector4(c.diff.x, c.diff.y, c.diff.z, c.met));
                sub->setCustomParameter(2*k+2, Ogre::Vector4(c.spec.x, c.spec.y, c.spec.z, 0.0f));
            }

            GfxMaterial *m = sub->material;

            if (do_regular) {

                /* TODO: include other criteria to pick a specific Ogre::Material:
                 * bones: 1 2 3 4
                 * vertex colours: false/true
                 * vertex alpha: false/true
                 * Fading: false/true
                 * World: false/true
                 */

                if (fade < 1 && !m->getStipple()) {
                    renderMaterial = m->fadingMat;
                } else {
                    renderMaterial = m->regularMat;
                }

                int queue_group = RQ_GBUFFER_OPAQUE;
                switch (m->getSceneBlend()) {
                    case GFX_MATERIAL_OPAQUE:      queue_group = RQ_GBUFFER_OPAQUE; break;
                    case GFX_MATERIAL_ALPHA:       queue_group = RQ_FORWARD_ALPHA; break;
                    case GFX_MATERIAL_ALPHA_DEPTH: queue_group = RQ_FORWARD_ALPHA_DEPTH; break;
                }
                queue->addRenderable(sub, queue_group, 0);

                if (!m->emissiveMat.isNull() && sub->emissiveEnabled) {
                    renderMaterial = m->emissiveMat;
                    switch (m->getSceneBlend()) {
                        case GFX_MATERIAL_OPAQUE:      queue_group = RQ_FORWARD_OPAQUE_EMISSIVE; break;
                        case GFX_MATERIAL_ALPHA:       queue_group = RQ_FORWARD_ALPHA_EMISSIVE; break;
                        case GFX_MATERIAL_ALPHA_DEPTH: queue_group = RQ_FORWARD_ALPHA_DEPTH_EMISSIVE; break;
                    }
                    queue->addRenderable(sub, queue_group, 0);
                }
            }

            if (do_wireframe) {
                renderMaterial = m->wireframeMat;
                queue->addRenderable(sub, RQ_FORWARD_ALPHA, 0);
            }

        }

    }

    renderMaterial.setNull();

}

void GfxBody::visitRenderables(Ogre::Renderable::Visitor* visitor, bool)
{
    // Visit each Sub
    for (SubList::iterator i = subList.begin(); i != subList.end(); ++i) {
        GfxMaterial *m = (*i)->material;
        renderMaterial = m->regularMat;
        visitor->visit(*i, 0, false);
        renderMaterial.setNull();
    }
}


const Ogre::AxisAlignedBox& GfxBody::getBoundingBox(void) const
{
    return mesh->getBounds();
}

Ogre::Real GfxBody::getBoundingRadius(void) const
{
    return mesh->getBoundingSphereRadius();
}

 // }}}









static const std::string &apply_map (const GfxStringMap &sm, const std::string &s)
{
    GfxStringMap::const_iterator i = sm.find(s);
    return i==sm.end() ? s : i->second;
}

GfxBodyPtr GfxBody::make (const std::string &mesh_name,
                          const GfxStringMap &sm,
                          const GfxNodePtr &par_)
{
    DiskResource *dr = disk_resource_get(mesh_name);
    if (dr==NULL) GRIT_EXCEPT("No such resource: \""+mesh_name+"\"");

    if (!dr->isLoaded()) GRIT_EXCEPT("Resource not yet loaded: \""+mesh_name+"\"");

    GfxMeshDiskResource *gdr = dynamic_cast<GfxMeshDiskResource*>(dr);
    if (gdr==NULL) GRIT_EXCEPT("Resource is not a mesh: \""+mesh_name+"\"");

    return GfxBodyPtr(new GfxBody(gdr, sm, par_));
}

GfxBody::GfxBody (GfxMeshDiskResource *gdr, const GfxStringMap &sm, const GfxNodePtr &par_)
  : GfxFertileNode(par_), initialMaterialMap(sm)
{
    node->attachObject(this);

    mesh = gdr->getOgreMeshPtr();

    mesh->load();

    memset(colours, 0, sizeof(colours));

    fade = 1;
    enabled = true;
    castShadows = true;
    skeleton = NULL;
    wireframe = false;
    freshFrame = true;

    reinitialise();

}

unsigned GfxBody::getSubMeshByOriginalMaterialName (const std::string &n)
{
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        if (sm->getMaterialName() == n) {
            return i;
        }
    }
    CERR << "Mesh did not contain material \""<<n<<"\"" <<std::endl;
    return 0;
}

void GfxBody::updateBones (void)
{
    if (skeleton == NULL) {
        manualBones.clear();
        return;
    }
    manualBones.resize(skeleton->getNumBones());
    for (unsigned i=0 ; i<skeleton->getNumBones() ; ++i) {
        skeleton->getBone(i)->setManuallyControlled(manualBones[i]);
    }
    skeleton->_notifyManualBonesDirty(); // HACK: ogre should do this itself
}

void GfxBody::destroyGraphics (void)
{
    for (SubList::iterator i=subList.begin(),i_=subList.end() ; i!=i_ ; ++i) {
        delete *i;
    }
    subList.clear();
    
    if (skeleton) {
        OGRE_FREE_SIMD(boneWorldMatrices, Ogre::MEMCATEGORY_ANIMATION);
        OGRE_DELETE skeleton;
        OGRE_FREE_SIMD(boneMatrices, Ogre::MEMCATEGORY_ANIMATION);
    }
}

void GfxBody::reinitialise (void)
{
    APP_ASSERT(mesh->isLoaded());

    destroyGraphics();

    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        Sub* sub = new Sub(this, sm);
        subList.push_back(sub);
        GFX_MAT_SYNC;
        std::string matname = apply_map(initialMaterialMap, sm->getMaterialName());
        if (!gfx_material_has(matname)) {
            CERR << "Mesh \"/"<<mesh->getName()<<"\" references non-existing material "
                 << "\""<<matname<<"\""<<std::endl;
            matname = "/system/FallbackMaterial";
        }
        sub->material = gfx_material_get(matname);
    }


    if (!mesh->getSkeleton().isNull()) {
        skeleton = OGRE_NEW Ogre::SkeletonInstance(mesh->getSkeleton());
        skeleton->load();
        numBoneMatrices = skeleton->getNumBones();
        boneMatrices      = static_cast<Ogre::Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Ogre::Matrix4) * numBoneMatrices, Ogre::MEMCATEGORY_ANIMATION));
        boneWorldMatrices = static_cast<Ogre::Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Ogre::Matrix4) * numBoneMatrices, Ogre::MEMCATEGORY_ANIMATION));

        mesh->_initAnimationState(&animationState);
    } else {
        skeleton = NULL;
        numBoneMatrices = 0;
        boneMatrices      = NULL;
        boneWorldMatrices = NULL;
    }

    updateBones();
}

GfxBody::~GfxBody (void)
{
    if (!dead) destroy();
}

void GfxBody::destroy (void)
{
    if (dead) THROW_DEAD(className);
    destroyGraphics();
    GfxFertileNode::destroy();
}

GfxMaterial *GfxBody::getMaterial (unsigned i)
{
    if (i >= subList.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return subList[i]->material;
}

const std::string &GfxBody::getOriginalMaterialName (unsigned i)
{
    if (i >= subList.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return mesh->getSubMesh(i)->getMaterialName();
}

void GfxBody::setMaterial (unsigned i, GfxMaterial *m)
{
    if (i >= subList.size()) GRIT_EXCEPT("Submesh id out of range. ");
    if (subList[i]->material == m) return;
    subList[i]->material = m;
}

bool GfxBody::getEmissiveEnabled (unsigned i)
{
    if (i >= subList.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return subList[i]->emissiveEnabled;
}

void GfxBody::setEmissiveEnabled (unsigned i, bool v)
{
    if (i >= subList.size()) GRIT_EXCEPT("Submesh id out of range. ");
    subList[i]->emissiveEnabled = v;
}

unsigned GfxBody::getBatches (void) const
{
    return subList.size();
}

unsigned GfxBody::getBatchesWithChildren (void) const
{
    return getBatches() + GfxFertileNode::getBatchesWithChildren();
}

unsigned GfxBody::getVertexes (void) const
{
    return mesh->sharedVertexData->vertexCount;
}

unsigned GfxBody::getVertexesWithChildren (void) const
{
    return getVertexes() + GfxFertileNode::getVertexesWithChildren();
}

unsigned GfxBody::getTriangles (void) const
{
    unsigned total = 0;
    for (unsigned i=0 ; i<subList.size() ; ++i) {
        total += subList[i]->getSubMesh()->indexData->indexCount/3;
    }
    return total;
}

unsigned GfxBody::getTrianglesWithChildren (void) const
{
    return getTriangles() + GfxFertileNode::getTrianglesWithChildren();
}

float GfxBody::getFade (void)
{
    if (dead) THROW_DEAD(className);
    return fade;
}
void GfxBody::setFade (float f)
{
    if (dead) THROW_DEAD(className);
    fade = f;
}

bool GfxBody::getCastShadows (void)
{
    return castShadows;
}
void GfxBody::setCastShadows (bool v)
{
    if (dead) THROW_DEAD(className);
    castShadows = v;
}

bool GfxBody::getWireframe (void)
{
    return wireframe;
}
void GfxBody::setWireframe (bool v)
{
    if (dead) THROW_DEAD(className);
    wireframe = v;
}

GfxPaintColour GfxBody::getPaintColour (int i)
{
    if (dead) THROW_DEAD(className);
    return colours[i];
}
void GfxBody::setPaintColour (int i, const GfxPaintColour &c)
{
    if (dead) THROW_DEAD(className);
    colours[i] = c;
}

unsigned GfxBody::getNumBones (void) const
{
    if (dead) THROW_DEAD(className);
    return manualBones.size();
}

bool GfxBody::hasBoneName (const std::string name) const
{
    if (dead) THROW_DEAD(className);
    if (skeleton == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    return skeleton->hasBone(name);
}

unsigned GfxBody::getBoneId (const std::string name) const
{
    if (dead) THROW_DEAD(className);
    if (skeleton == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    if (!skeleton->hasBone(name)) GRIT_EXCEPT("GfxBody has no bone \""+name+"\"");
    return skeleton->getBone(name)->getHandle();
}

void GfxBody::checkBone (unsigned n) const
{
    if (dead) THROW_DEAD(className);
    if (manualBones.size()==0) GRIT_EXCEPT("GfxBody has no skeleton");
    if (n >= manualBones.size()) {
        std::stringstream ss;
        ss << "Bone " << n << " out of range [0," << manualBones.size() << ")";
        GRIT_EXCEPT(ss.str());
    }
}

const std::string &GfxBody::getBoneName (unsigned n) const
{
    checkBone(n);
    return skeleton->getBone(n)->getName();
}

bool GfxBody::getBoneManuallyControlled (unsigned n)
{
    checkBone(n);
    return manualBones[n];
}

void GfxBody::setBoneManuallyControlled (unsigned n, bool v)
{
    checkBone(n);
    manualBones[n] = v;
    updateBones();
}

void GfxBody::setAllBonesManuallyControlled (bool v)
{
    if (dead) THROW_DEAD(className);
    for (unsigned i=0 ; i<manualBones.size() ; ++i) {
        manualBones[i] = v;
    }
    updateBones();
}

Vector3 GfxBody::getBoneInitialPosition (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    return from_ogre(bone->getInitialPosition());
}

Vector3 GfxBody::getBoneWorldPosition (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    return from_ogre(bone->_getDerivedPosition());
}

Vector3 GfxBody::getBoneLocalPosition (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    return from_ogre(bone->getPosition());
}

Quaternion GfxBody::getBoneInitialOrientation (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    return from_ogre(bone->getInitialOrientation());
}

Quaternion GfxBody::getBoneWorldOrientation (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    return from_ogre(bone->_getDerivedOrientation());
}

Quaternion GfxBody::getBoneLocalOrientation (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    return from_ogre(bone->getOrientation());
}

Vector3 GfxBody::getBoneInitialScale (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    return from_ogre(bone->getInitialScale());
}

Vector3 GfxBody::getBoneWorldScale (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    return from_ogre(bone->_getDerivedScale());
}

Vector3 GfxBody::getBoneLocalScale (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    return from_ogre(bone->getScale());
}


Transform GfxBody::getBoneWorldTransform (unsigned n)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    Transform t(from_ogre(bone->_getDerivedPosition()), from_ogre(bone->_getDerivedOrientation()), from_ogre(bone->_getDerivedScale()));
    updateWorldTransform();
    return worldTransform * t;
}


void GfxBody::setBoneLocalPosition (unsigned n, const Vector3 &v)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    bone->setPosition(to_ogre(v));
}

void GfxBody::setBoneLocalOrientation (unsigned n, const Quaternion &v)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    bone->setOrientation(to_ogre(v));
}

void GfxBody::setBoneLocalScale (unsigned n, const Vector3 &v)
{
    checkBone(n);
    Ogre::Bone *bone = skeleton->getBone(n);
    bone->setScale(to_ogre(v));
}

std::vector<std::string> GfxBody::getAnimationNames (void)
{
    std::vector<std::string> r;

    if (dead) THROW_DEAD(className);

    if (skeleton == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    
    Ogre::AnimationStateIterator it = animationState.getAnimationStateIterator();

    while (it.hasMoreElements()) r.push_back(it.getNext()->getAnimationName());

    return r;
}

Ogre::AnimationState *GfxBody::getAnimState (const std::string &name)
{
    if (skeleton == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::AnimationState *state = animationState.getAnimationState(name);
    if (state == NULL) GRIT_EXCEPT("GfxBody has no animation called \""+name+"\"");
    return state;
}

float GfxBody::getAnimationLength (const std::string &name)
{
    if (dead) THROW_DEAD(className);
    return getAnimState(name)->getLength();
}

float GfxBody::getAnimationPos (const std::string &name)
{
    if (dead) THROW_DEAD(className);
    return getAnimState(name)->getTimePosition();
}

void GfxBody::setAnimationPos (const std::string &name, float v)
{
    if (dead) THROW_DEAD(className);
    getAnimState(name)->setTimePosition(v);
}

float GfxBody::getAnimationMask (const std::string &name)
{
    if (dead) THROW_DEAD(className);
    Ogre::AnimationState *state = getAnimState(name);
    if (!state->getEnabled()) return 0.0f;
    return state->getWeight();
}

void GfxBody::setAnimationMask (const std::string &name, float v)
{
    if (dead) THROW_DEAD(className);
    Ogre::AnimationState *state = getAnimState(name);
    state->setWeight(v);
    state->setEnabled(v > 0.0f);
}

bool GfxBody::isEnabled (void)
{
    if (dead) THROW_DEAD(className);
    return enabled;
}

void GfxBody::setEnabled (bool v)
{
    if (dead) THROW_DEAD(className);
    enabled = v;
}

const std::string &GfxBody::getMeshName (void)
{
    return mesh->getName();
}
