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

#include <vector>
#include <string>

#include "../SharedPtr.h"

class GfxSkyBody;
typedef SharedPtr<GfxSkyBody> GfxSkyBodyPtr;

#include "../vect_util.h"

extern fast_erase_vector<GfxSkyBody*> gfx_all_sky_bodies;


#ifndef SKY_BODY_H
#define SKY_BODY_H

#include <OgreEntity.h>
#include <OgreMesh.h>
#include <OgreSceneNode.h>

#include "../math_util.h"
#include "GfxDiskResource.h"
#include "GfxSkyMaterial.h"

class GfxSkyBody : public fast_erase_index {
  protected:
    static const std::string className;

    bool dead;
    bool enabled;

    unsigned char zOrder;

    Ogre::MeshPtr mesh;
    GfxSkyMaterials materials;

    GfxSkyBody (GfxDiskResource *gdr, short z_order);
  public:
    Ogre::Entity *ent;
    Ogre::SceneNode *node;

    ~GfxSkyBody (void);

    static GfxSkyBodyPtr make (const std::string &mesh_name, short z_order);

    unsigned getNumSubMeshes (void) { return materials.size(); }
    GfxSkyMaterial *getMaterial (unsigned i);

    void destroy (void);
    virtual bool destroyed (void) const { return dead; }

    Quaternion getOrientation (void);
    void setOrientation (const Quaternion &q);

    unsigned char getZOrder (void);
    void setZOrder (unsigned char v);

    bool isEnabled (void);
    void setEnabled (bool v);

    void updateProperties (void);

    friend class GfxSkyMaterial;

};

#endif
