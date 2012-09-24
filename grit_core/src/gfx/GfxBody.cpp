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

const std::string GfxNode::className = "GfxNode";

GfxNode::GfxNode (const GfxBodyPtr &par_)
{
    dead = false;
    pos = Vector3(0,0,0);
    scl = Vector3(1,1,1);
    quat = Quaternion(1,0,0,0);
    node = ogre_sm->createSceneNode();
    par = GfxBodyPtr(NULL); setParent(par_);
}

GfxNode::~GfxNode (void)
{
    if (!dead) destroy();
}
    
void GfxNode::destroy (void)
{       
    if (!par.isNull()) par->notifyLostChild(this);
    ogre_sm->destroySceneNode(node->getName());
    node = NULL; 
    par = GfxBodyPtr(NULL);
    dead = true;
}       
    
void GfxNode::notifyParentDead (void)
{
    if (dead) THROW_DEAD(className);
    APP_ASSERT(!par.isNull());
    par->node->removeChild(node);
    par = GfxBodyPtr(NULL);
    ogre_root_node->addChild(node);
}   
    
const GfxBodyPtr &GfxNode::getParent (void) const
{   
    if (dead) THROW_DEAD(className);
    return par;
}   
    
void GfxNode::setParent (const GfxBodyPtr &par_)
{
    if (dead) THROW_DEAD(className);
    if (!par.isNull()) {
        par->node->removeChild(node);
        par->notifyLostChild(this);
    } else {
        ogre_root_node->removeChild(node);
    }
    par = par_;
    if (!par.isNull()) {
        par->notifyGainedChild(this);
        par->node->addChild(node);
    } else {
        ogre_root_node->addChild(node);
    }
}

void GfxNode::ensureNotChildOf (GfxBody *leaf) const
{
    if (leaf==this) GRIT_EXCEPT("Parenthood must be acyclic.");
    if (par.isNull()) return;
    par->ensureNotChildOf(leaf);
}

void GfxNode::setLocalPosition (const Vector3 &p)
{
    if (dead) THROW_DEAD(className);
    pos = p;
    node->setPosition(to_ogre(p));
}

void GfxNode::setLocalOrientation (const Quaternion &q)
{
    if (dead) THROW_DEAD(className);
    quat = q;
    node->setOrientation(to_ogre(q));
}

void GfxNode::setLocalScale (const Vector3 &s)
{
    if (dead) THROW_DEAD(className);
    scl = s;
    node->setScale(to_ogre(s));
}

Vector3 GfxNode::transformPosition (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return getWorldPosition() + getWorldScale()*(getWorldOrientation()*v);
}
Quaternion GfxNode::transformOrientation (const Quaternion &v)
{
    if (dead) THROW_DEAD(className);
    return getWorldOrientation()*v;
}
Vector3 GfxNode::transformScale (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return getWorldScale()*v;
}

Vector3 GfxNode::transformPositionParent (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return par.isNull() ? v : par->transformPosition(v);
}
Quaternion GfxNode::transformOrientationParent (const Quaternion &v)
{
    if (dead) THROW_DEAD(className);
    return par.isNull() ? v : par->transformOrientation(v);
}
Vector3 GfxNode::transformScaleParent (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return par.isNull() ? v : par->transformScale(v);
}

const Vector3 &GfxNode::getLocalPosition (void)
{
    if (dead) THROW_DEAD(className);
    return pos;
}
Vector3 GfxNode::getWorldPosition (void)
{
    if (dead) THROW_DEAD(className);
    return transformPositionParent(pos);
}

const Quaternion &GfxNode::getLocalOrientation (void)
{
    if (dead) THROW_DEAD(className);
    return quat;
}
Quaternion GfxNode::getWorldOrientation (void)
{
    if (dead) THROW_DEAD(className);
    return transformOrientationParent(quat);
}

const Vector3 &GfxNode::getLocalScale (void)
{
    if (dead) THROW_DEAD(className);
    return scl;
}
Vector3 GfxNode::getWorldScale (void)
{
    if (dead) THROW_DEAD(className);
    return transformScaleParent(scl);
}


// }}}


// {{{ GFX_BODY

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
                          const GfxBodyPtr &par_)
{
    DiskResource *dr = disk_resource_get(mesh_name);
    if (dr==NULL) GRIT_EXCEPT("No such resource: \""+mesh_name+"\"");

    if (!dr->isLoaded()) GRIT_EXCEPT("Resource not yet loaded: \""+mesh_name+"\"");

    GfxDiskResource *gdr = dynamic_cast<GfxDiskResource*>(dr);
    if (gdr==NULL) GRIT_EXCEPT("Resource is not a mesh: \""+mesh_name+"\"");

    return GfxBodyPtr(new GfxBody(gdr, sm, par_));
}

GfxBody::GfxBody (GfxDiskResource *gdr, const GfxStringMap &sm, const GfxBodyPtr &par_)
  : GfxNode(par_), initialMaterialMap(sm)
{
    const Ogre::ResourcePtr &rp = gdr->getOgreResourcePtr();

    mesh = rp;

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

    if (ent!=NULL) ogre_sm->destroyEntity(ent);
    ent = ogre_sm->createEntity(freshname(), mesh->getName());
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
        Ogre::SubEntity *se = ent->getSubEntity(i);
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
            se->setRenderQueueGroup(RQ_FORWARD_EMISSIVE);
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

        Ogre::SubEntity *se = ent->getSubEntity(i);

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

GfxBody::GfxBody (const GfxBodyPtr &par_)
  : GfxNode(par_)
{
    memset(colours, 0, sizeof(colours));

    mesh.setNull();

    ent = NULL;
    entEmissive = NULL;
    fade = 1;
    enabled = true;

    gfx_all_bodies.push_back(this);
}

GfxBody::~GfxBody (void)
{
    if (!dead) destroy();
}

void GfxBody::destroy (void)
{
    if (dead) THROW_DEAD(className);
    for (unsigned i=0 ; i<children.size() ; ++i) {
        children[i]->notifyParentDead();
    }
    children.clear();
    if (ent) ogre_sm->destroyEntity(ent);
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

void GfxBody::notifyLostChild (GfxNode *child)
{
    unsigned ci = 0;
    bool found_child = false;
    for (unsigned i=0 ; i<children.size() ; ++i) {
        if (children[i]==child) {
            ci = i;
            found_child = true;
            break;
        }
    }
    APP_ASSERT(found_child);
    children[ci] = children[children.size()-1];
    children.pop_back();
}

void GfxBody::notifyGainedChild (GfxNode *child)
{
    children.push_back(child);
}

void GfxBody::setParent (const GfxBodyPtr &par_)
{
    if (dead) THROW_DEAD(className);
    if (!par_.isNull()) par_->ensureNotChildOf(this); // check that we are not a parent of par_
    GfxNode::setParent(par_);
}

unsigned GfxBody::getBatches (void) const
{
    if (!hasGraphics()) return 0;
    return materials.size();
}

unsigned GfxBody::getBatchesWithChildren (void) const
{
    unsigned total = getBatches();
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxBody *child = dynamic_cast<GfxBody*>(children[i]);
        if (child) {
            total += child->getBatchesWithChildren();
        }
    }
    return total;
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
    unsigned total = getVertexes();
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxBody *child = dynamic_cast<GfxBody*>(children[i]);
        if (child) {
            total += child->getVertexesWithChildren();
        }
    }
    return total;
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
    unsigned total = getTriangles();
    for (unsigned i=0 ; i<children.size() ; ++i) {
        GfxBody *child = dynamic_cast<GfxBody*>(children[i]);
        if (child) {
            total += child->getTrianglesWithChildren();
        }
    }
    return total;
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

// }}}
