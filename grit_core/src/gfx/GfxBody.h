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

#include "../SharedPtr.h"

class GfxBody;
class GritEntity;
typedef SharedPtr<GfxBody> GfxBodyPtr;

extern fast_erase_vector<GfxBody*> gfx_all_bodies;

#ifndef GfxBody_h
#define GfxBody_h

#include <OgreMesh.h>

#include "GfxNode.h"
#include "GfxMaterial.h"

class GfxBody : public GfxNode, public fast_erase_index, public Ogre::MovableObject {

    public:

    class Sub : public Ogre::Renderable {

        protected:

        GfxBody *parent;


        Ogre::SubMesh *subMesh;

        public:

        Sub (GfxBody *parent, Ogre::SubMesh *sub_mesh)
            : Renderable(), parent(parent), subMesh(sub_mesh)
        { }
        ~Sub() { }

        Ogre::SubMesh* getSubMesh(void) const { return subMesh; }

        GfxMaterial *material;

        const Ogre::MaterialPtr& getMaterial(void) const;


        void getRenderOperation(Ogre::RenderOperation& op)
        { subMesh->_getRenderOperation(op, 0); }

        void getWorldTransforms(Ogre::Matrix4* xform) const;
        unsigned short getNumWorldTransforms(void) const;
        Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
        const Ogre::LightList& getLights(void) const;

        bool getCastsShadows(void) const;

    };


    const Ogre::AxisAlignedBox& getBoundingBox (void) const;
    float getBoundingRadius (void) const;

    void _updateRenderQueue (Ogre::RenderQueue* queue);
    void visitRenderables (Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

    const std::string& getMovableType (void) const { return className; }

    void _updateAnimation (void);

    protected:

    typedef std::vector<Sub*> SubList;
    SubList subList;

    Ogre::AnimationStateSet animationState;

    Ogre::Matrix4 *boneWorldMatrices;
    Ogre::Matrix4 *boneMatrices;
    unsigned short numBoneMatrices;
    Ogre::SkeletonInstance* skeleton;



    protected:
    static const std::string className;
    public: // HACK
    Ogre::MeshPtr mesh;
    protected:
    float fade;
    Ogre::MaterialPtr renderMaterial;
    GfxMaterials materials;
    std::vector<bool> emissiveEnabled;
    GfxPaintColour colours[4];
    bool enabled;
    bool castShadows;
    bool wireframe;
    std::vector<bool> manualBones;
    GfxStringMap initialMaterialMap;

    GfxBody (GfxMeshDiskResource *gdr, const GfxStringMap &sm, const GfxNodePtr &par_);
    ~GfxBody ();


    public:
    static GfxBodyPtr make (const std::string &mesh_name,
                            const GfxStringMap &sm=gfx_empty_string_map,
                            const GfxNodePtr &par_=GfxNodePtr(NULL));

    GfxMaterial *getMaterial (unsigned i);
    const std::string &getOriginalMaterialName (unsigned i);
    unsigned getSubMeshByOriginalMaterialName (const std::string &n);
    void setMaterial (unsigned i, GfxMaterial *m);
    bool getEmissiveEnabled (unsigned i);
    void setEmissiveEnabled (unsigned i, bool v);
    unsigned getNumSubMeshes (void) { return materials.size(); }

    protected:
    void destroyGraphics (void);
    void updateVisibility (void);
    void updateMaterials (void);
    void updateBones (void);
    void checkBone (unsigned n);
    Ogre::AnimationState *getAnimState (const std::string &name);
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

    bool getWireframe (void);
    void setWireframe (bool v);

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

    std::vector<std::string> getAnimationNames (void);
    float getAnimationLength (const std::string &name);
    float getAnimationPos (const std::string &name);
    void setAnimationPos (const std::string &name, float v);
    float getAnimationMask (const std::string &name);
    void setAnimationMask (const std::string &name, float v);

    bool isEnabled (void);
    void setEnabled (bool v);

    void destroy (void);

    bool hasGraphics (void) const { return true; }

    const std::string &getMeshName (void);

    friend class SharedPtr<GfxBody>;
    friend class GfxMeshDiskResource;
};

#endif
