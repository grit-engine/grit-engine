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

#include "GfxBody.h"
#include "GritEntity.h"

const std::string GfxBody::className = "GfxBody";

static const std::string &apply_map (const GfxStringMap &sm, const std::string &s)
{
    GfxStringMap::const_iterator i = sm.find(s);
    return i==sm.end() ? s : i->second;
}

static void validate_mesh (const Ogre::MeshPtr &mesh, const GfxStringMap &gsm)
{
/*
    for (GfxStringMap::const_iterator i=gsm.begin(),i_=gsm.end() ; i!=i_ ; ++i) {
        CVERB << i->first << " " << i->second << std::endl;
    }
*/
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        std::string matname = apply_map(gsm, sm->getMaterialName());
        if (!gfx_material_has(matname)) {
            CERR << "Mesh \"/"<<mesh->getName()<<"\" references non-existing material "
                 << "\""<<matname<<"\""<<std::endl;
            matname = "/system/FallbackMaterial";
            sm->setMaterialName(matname);
        }
    }
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
  : GfxNode(par_), initialMaterialMap(sm)
{

    mesh = gdr->getOgreMeshPtr();

    mesh->load();

    memset(colours, 0, sizeof(colours));

    ent = NULL;
    entEmissive = NULL;
    fade = 1;
    enabled = true;
    castShadows = false;

    reinitialise();

    gfx_all_bodies.push_back(this);
}

void GfxBody::updateMaterials (void)
{
    GFX_MAT_SYNC;
    if (mesh.isNull()) return;
    validate_mesh(mesh, initialMaterialMap);
    materials = std::vector<GfxMaterial*>(mesh->getNumSubMeshes());
    for (unsigned i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
        Ogre::SubMesh *sm = mesh->getSubMesh(i);
        std::string matname = apply_map(initialMaterialMap, sm->getMaterialName());
        materials[i] = gfx_material_get(matname);
    }
    unsigned old_size = emissiveEnabled.size();
    emissiveEnabled.resize(mesh->getNumSubMeshes());
    for (unsigned i=old_size ; i<emissiveEnabled.size() ; ++i) {
        emissiveEnabled[i] = true;
    }
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
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) {
        manualBones.clear();
        return;
    }
    manualBones.resize(skel->getNumBones());
    for (unsigned i=0 ; i<skel->getNumBones() ; ++i) {
        skel->getBone(i)->setManuallyControlled(manualBones[i]);
    }
    skel->_notifyManualBonesDirty(); // HACK: ogre should do this itself
}

void GfxBody::reinitialise (void)
{
    updateMaterials();
    if (entEmissive) ogre_sm->destroyEntity(entEmissive);
    entEmissive = NULL;

    if (ent!=NULL) delete ent;
    ent = new GritEntity(freshname(), mesh);
    node->attachObject(ent);
    updateProperties();
    updateBones();
}

void GfxBody::updateVisibility (void)
{
    if (ent==NULL) return;
    // avoid the draw entirely if faded out
    ent->setVisible(enabled && fade > 0.001);
    if (entEmissive) entEmissive->setVisible(enabled && fade > 0.001);

    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {
        GritEntity::Sub *se = ent->getSubEntity(i);
        // fading in/out (either stipple or alpha factor)
        se->setCustomParameter(0, Ogre::Vector4(fade,0,0,0));
    }

    if (entEmissive != NULL) {
        for (unsigned i=0 ; i<entEmissive->getNumSubEntities() ; ++i) {
            Ogre::SubEntity *se = entEmissive->getSubEntity(i);
            // fading in/out (either stipple or alpha factor)
            se->setCustomParameter(0, Ogre::Vector4(fade,0,0,0));
        }
    }

}

void GfxBody::updateEntEmissive (void)
{
    GFX_MAT_SYNC;
    if (ent==NULL) return;
    if (entEmissive != NULL) {
        // destroy it if we've already got one
        ogre_sm->destroyEntity(entEmissive);
        entEmissive = NULL;
    }
    bool needs_emissive = false;
    for (unsigned i=0 ; i<materials.size() ; ++i) {
        GfxMaterial *gfx_material = materials[i];
        if (!gfx_material->emissiveMat.isNull() && emissiveEnabled[i]) {
            needs_emissive = true;
        }
    }
    if (!needs_emissive) return;
    entEmissive = ogre_sm->createEntity(freshname(), mesh->getName());
    node->attachObject(entEmissive);
    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {
        GfxMaterial *gfx_material = materials[i];
        Ogre::SubEntity *se = entEmissive->getSubEntity(i);
        if (!gfx_material->emissiveMat.isNull() && emissiveEnabled[i]) {
            se->setMaterial(gfx_material->emissiveMat);
            switch (gfx_material->getSceneBlend()) {
                case GFX_MATERIAL_OPAQUE:
                se->setRenderQueueGroup(RQ_FORWARD_OPAQUE_EMISSIVE);
                break;
                case GFX_MATERIAL_ALPHA:
                se->setRenderQueueGroup(RQ_FORWARD_ALPHA_EMISSIVE);
                break;
                case GFX_MATERIAL_ALPHA_DEPTH:
                se->setRenderQueueGroup(RQ_FORWARD_ALPHA_DEPTH_EMISSIVE);
                break;
            }
        } else {
            se->setVisible(false);
        }
    }
    updateVisibility();
}

// Currently needs to be updated when these material properties change:
// getSceneBlend
// getStipple
// emissiveMat
void GfxBody::updateProperties (void)
{
    GFX_MAT_SYNC;

    if (ent==NULL) return;

    ent->setCastShadows(castShadows);

    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {

        GritEntity::Sub *se = ent->getSubEntity(i);

        GfxMaterial *gfx_material = materials[i];

        switch (gfx_material->getSceneBlend()) {
            case GFX_MATERIAL_OPAQUE:
            se->setRenderQueueGroup(RQ_GBUFFER_OPAQUE);
            break;
            case GFX_MATERIAL_ALPHA:
            se->setRenderQueueGroup(RQ_FORWARD_ALPHA);
            break;
            case GFX_MATERIAL_ALPHA_DEPTH:
            se->setRenderQueueGroup(RQ_FORWARD_ALPHA_DEPTH);
            break;
        }

        // materials
        /* TODO: include other criteria to pick a specific Ogre::Material:
         * bones: 1 2 3 4
         * vertex colours: false/true
         * vertex alpha: false/true
         * Fading: false/true
         * World: false/true
         */
        if (fade < 1 && !gfx_material->getStipple()) {
            se->setMaterial(gfx_material->fadingMat);
        } else {
            se->setMaterial(gfx_material->regularMat);
        }

        // car paint
        for (int k=0 ; k<4 ; ++k) {
            const GfxPaintColour &c = colours[k];
            se->setCustomParameter(2*k+1, Ogre::Vector4(c.diff.x, c.diff.y, c.diff.z, c.met));
            se->setCustomParameter(2*k+2, Ogre::Vector4(c.spec.x, c.spec.y, c.spec.z, 0.0f));
        }
    }

    updateEntEmissive();

    updateVisibility();
}

GfxBody::~GfxBody (void)
{
    if (!dead) destroy();
}

void GfxBody::destroy (void)
{
    if (dead) THROW_DEAD(className);
    if (ent) delete ent;
    ent = NULL;
    if (entEmissive) ogre_sm->destroyEntity(entEmissive);
    entEmissive = NULL;
    gfx_all_bodies.erase(this);
    GfxNode::destroy();
}

GfxMaterial *GfxBody::getMaterial (unsigned i)
{
    if (i >= materials.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return materials[i];
}

const std::string &GfxBody::getOriginalMaterialName (unsigned i)
{
    if (i >= materials.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return mesh->getSubMesh(i)->getMaterialName();
}

void GfxBody::setMaterial (unsigned i, GfxMaterial *m)
{
    if (i >= materials.size()) GRIT_EXCEPT("Submesh id out of range. ");
    if (materials[i] == m) return;
    materials[i] = m;
    updateProperties();
}

bool GfxBody::getEmissiveEnabled (unsigned i)
{
    if (i >= emissiveEnabled.size()) GRIT_EXCEPT("Submesh id out of range. ");
    return emissiveEnabled[i];
}

void GfxBody::setEmissiveEnabled (unsigned i, bool v)
{
    if (i >= emissiveEnabled.size()) GRIT_EXCEPT("Submesh id out of range. ");
    emissiveEnabled[i] = v;
    updateEntEmissive();
}

unsigned GfxBody::getBatches (void) const
{
    if (!hasGraphics()) return 0;
    return materials.size();
}

unsigned GfxBody::getBatchesWithChildren (void) const
{
    return getBatches() + GfxNode::getBatchesWithChildren();
}

unsigned GfxBody::getVertexes (void) const
{
    if (!hasGraphics()) return 0;
    unsigned total = 0;
    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {
        total += ent->getMesh()->sharedVertexData->vertexCount;
    }
    return total;
}

unsigned GfxBody::getVertexesWithChildren (void) const
{
    return getVertexes() + GfxNode::getVertexesWithChildren();
}

unsigned GfxBody::getTriangles (void) const
{
    if (!hasGraphics()) return 0;
    unsigned total = 0;
    for (unsigned i=0 ; i<ent->getNumSubEntities() ; ++i) {
        total += ent->getSubEntity(i)->getSubMesh()->indexData->indexCount/3;
    }
    return total;
}

unsigned GfxBody::getTrianglesWithChildren (void) const
{
    return getTriangles() + GfxNode::getTrianglesWithChildren();
}

float GfxBody::getFade (void)
{
    if (dead) THROW_DEAD(className);
    // It's convenient to just set fade for all nodes in the hierarchy
    //if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    return fade;
}
void GfxBody::setFade (float f)
{
    if (dead) THROW_DEAD(className);
    //if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    fade = f;
    updateVisibility();
}

bool GfxBody::getCastShadows (void)
{
    return castShadows;
}
void GfxBody::setCastShadows (bool v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    castShadows = v;
    ent->setCastShadows(v);
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
    updateProperties();
}

unsigned GfxBody::getNumBones (void)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    return manualBones.size();
}

unsigned GfxBody::getBoneId (const std::string name)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    if (!skel->hasBone(name)) GRIT_EXCEPT("GfxBody has no bone \""+name+"\"");
    return skel->getBone(name)->getHandle();
}

void GfxBody::checkBone (unsigned n)
{
    if (manualBones.size()==0) GRIT_EXCEPT("GfxBody has no skeleton");
    if (n >= manualBones.size()) {
        std::stringstream ss;
        ss << "Bone " << n << " out of range [0," << manualBones.size() << ")";
        GRIT_EXCEPT(ss.str());
    }
}

const std::string &GfxBody::getBoneName (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    return skel->getBone(n)->getName();
}

bool GfxBody::getBoneManuallyControlled (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    return manualBones[n];
}

void GfxBody::setBoneManuallyControlled (unsigned n, bool v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    manualBones[n] = v;
    updateBones();
}

void GfxBody::setAllBonesManuallyControlled (bool v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    for (unsigned i=0 ; i<manualBones.size() ; ++i) {
        manualBones[i] = v;
    }
    updateBones();
}

Vector3 GfxBody::getBoneInitialPosition (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getInitialPosition());
}

Vector3 GfxBody::getBoneWorldPosition (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->_getDerivedPosition());
}

Vector3 GfxBody::getBoneLocalPosition (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getPosition());
}

Quaternion GfxBody::getBoneInitialOrientation (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getInitialOrientation());
}

Quaternion GfxBody::getBoneWorldOrientation (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->_getDerivedOrientation());
}

Quaternion GfxBody::getBoneLocalOrientation (unsigned n)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    return from_ogre(bone->getOrientation());
}


void GfxBody::setBoneLocalPosition (unsigned n, const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    bone->setPosition(to_ogre(v));
}

void GfxBody::setBoneLocalOrientation (unsigned n, const Quaternion &v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    checkBone(n);
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::Bone *bone = skel->getBone(n);
    bone->setOrientation(to_ogre(v));
}

std::vector<std::string> GfxBody::getAnimationNames (void)
{
    std::vector<std::string> r;

    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");

    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    
    Ogre::AnimationStateIterator it = ent->getAllAnimationStates().getAnimationStateIterator();

    while (it.hasMoreElements()) r.push_back(it.getNext()->getAnimationName());

    return r;
}

static Ogre::AnimationState *get_anim_state (GritEntity *ent, const std::string &name)
{
    Ogre::Skeleton *skel = ent->getSkeleton();
    if (skel == NULL) GRIT_EXCEPT("GfxBody has no skeleton");
    Ogre::AnimationStateSet &anim_set = ent->getAllAnimationStates();
    Ogre::AnimationState *state = anim_set.getAnimationState(name);
    if (state == NULL) GRIT_EXCEPT("GfxBody has no animation called \""+name+"\"");
    return state;
}

float GfxBody::getAnimationLength (const std::string &name)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    return get_anim_state(ent, name)->getLength();
}

float GfxBody::getAnimationPos (const std::string &name)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    return get_anim_state(ent, name)->getTimePosition();
}

void GfxBody::setAnimationPos (const std::string &name, float v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    get_anim_state(ent, name)->setTimePosition(v);
}

float GfxBody::getAnimationMask (const std::string &name)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    Ogre::AnimationState *state = get_anim_state(ent, name);
    if (!state->getEnabled()) return 0.0f;
    return state->getWeight();
}

void GfxBody::setAnimationMask (const std::string &name, float v)
{
    if (dead) THROW_DEAD(className);
    if (!ent) GRIT_EXCEPT("GfxBody has no graphical representation");
    Ogre::AnimationState *state = get_anim_state(ent, name);
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
    updateVisibility();
}

const std::string &GfxBody::getMeshName (void)
{
    static std::string empty_string = "";
    return mesh.isNull() ? empty_string : mesh->getName();
}
