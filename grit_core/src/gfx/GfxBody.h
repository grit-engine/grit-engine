/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

#include "../SharedPtr.h"

class GfxBody;
typedef SharedPtr<GfxBody> GfxBodyPtr;

extern fast_erase_vector<GfxBody*> gfx_all_bodies;

#ifndef GfxBody_h
#define GfxBody_h

#include "gfx.h"
#include "GfxMaterial.h"
#include <OgreMesh.h>

// this should rarely need to be used by users of this API
class GfxNode {
    protected:
    static const std::string className;
    Vector3 pos; Quaternion quat; Vector3 scl;
    GfxBodyPtr par;
    public: // HACK
    Ogre::SceneNode *node;
    protected:
    bool dead;

    GfxNode (const GfxBodyPtr &par_);
    virtual ~GfxNode ();

    void notifyParentDead (void);
    void ensureNotChildOf (GfxBody *leaf) const;

    public:

    const GfxBodyPtr &getParent (void) const;
    virtual void setParent (const GfxBodyPtr &par_);

    Vector3 transformPositionParent (const Vector3 &v);
    Quaternion transformOrientationParent (const Quaternion &v);
    Vector3 transformScaleParent (const Vector3 &v);
    Vector3 transformPosition (const Vector3 &v);
    Quaternion transformOrientation (const Quaternion &v);
    Vector3 transformScale (const Vector3 &v);

    const Vector3 &getLocalPosition (void);
    void setLocalPosition (const Vector3 &p);
    Vector3 getWorldPosition (void);

    const Quaternion &getLocalOrientation (void);
    void setLocalOrientation (const Quaternion &q);
    Quaternion getWorldOrientation (void);

    const Vector3 &getLocalScale (void);
    void setLocalScale (const Vector3 &s);
    Vector3 getWorldScale (void);

    virtual void destroy (void);
    virtual bool destroyed (void) const { return dead; }

    friend class GfxBody; // otherwise it cannot access our protected stuff
};

class GfxBody : public GfxNode, public fast_erase_index {
    protected:
    static const std::string className;
    std::vector<GfxNode*> children; // caution!
    public: // HACK
    Ogre::MeshPtr mesh;
    Ogre::Entity *ent;
    Ogre::Entity *entEmissive;
    protected:
    float fade;
    GfxMaterials materials;
    std::vector<bool> emissiveEnabled;
    GfxPaintColour colours[4];
    bool enabled;
    bool castShadows;
    std::vector<bool> manualBones;
    GfxStringMap initialMaterialMap;

    GfxBody (GfxDiskResource *gdr, const GfxStringMap &sm, const GfxBodyPtr &par_);
    GfxBody (const GfxBodyPtr &par_);
    ~GfxBody ();


    public:
    static GfxBodyPtr make (const std::string &mesh_name,
                            const GfxStringMap &sm=gfx_empty_string_map,
                            const GfxBodyPtr &par_=GfxBodyPtr(NULL));

    static GfxBodyPtr make (const GfxBodyPtr &par_=GfxBodyPtr(NULL))
    { return GfxBodyPtr(new GfxBody(par_)); }

    GfxMaterial *getMaterial (unsigned i);
    const std::string &getOriginalMaterialName (unsigned i);
    unsigned getSubMeshByOriginalMaterialName (const std::string &n);
    void setMaterial (unsigned i, GfxMaterial *m);
    bool getEmissiveEnabled (unsigned i);
    void setEmissiveEnabled (unsigned i, bool v);
    unsigned getNumSubMeshes (void) { return materials.size(); }

    void notifyLostChild (GfxNode *child);
    void notifyGainedChild (GfxNode *child);
    void setParent (const GfxBodyPtr &par_);

    protected:
    void updateEntEmissive (void);
    void updateVisibility (void);
    void updateMaterials (void);
    void updateBones (void);
    void checkBone (unsigned n);
    public:
    void updateProperties (void);
    void reinitialise (void);

    unsigned getBatches (void) const;
    unsigned getBatchesWithChildren (void) const;

    unsigned getTriangles (void) const;
    unsigned getTrianglesWithChildren (void) const;

    unsigned getVertexes (void) const;
    unsigned getVertexesWithChildren (void) const;

    float getFade (void);
    void setFade (float f);

    bool getCastShadows (void);
    void setCastShadows (bool v);

    GfxPaintColour getPaintColour (int i);
    void setPaintColour (int i, const GfxPaintColour &c);

    unsigned getNumBones (void);
    unsigned getBoneId (const std::string name);
    const std::string &getBoneName (unsigned n);

    bool getBoneManuallyControlled (unsigned n);
    void setBoneManuallyControlled (unsigned n, bool v);
    void setAllBonesManuallyControlled (bool v);

    Vector3 getBoneInitialPosition (unsigned n);
    Vector3 getBoneWorldPosition (unsigned n);
    Vector3 getBoneLocalPosition (unsigned n);
    Quaternion getBoneInitialOrientation (unsigned n);
    Quaternion getBoneWorldOrientation (unsigned n);
    Quaternion getBoneLocalOrientation (unsigned n);

    void setBoneLocalPosition (unsigned n, const Vector3 &v);
    void setBoneLocalOrientation (unsigned n, const Quaternion &v);

    bool isEnabled (void);
    void setEnabled (bool v);

    void destroy (void);

    bool hasGraphics (void) const { return ent!=NULL; }


    friend class SharedPtr<GfxBody>;
    friend class GfxDiskResource;
};

#endif
