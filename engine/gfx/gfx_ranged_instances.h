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

#include "../shared_ptr.h"

class GfxRangedInstances;
typedef SharedPtr<GfxRangedInstances> GfxRangedInstancesPtr;


#ifndef GFX_RANGED_INSTANCES_H
#define GFX_RANGED_INSTANCES_H

#include "../streamer.h"
#include "../cache_friendly_range_space_simd.h"

#include "gfx_instances.h"

class GfxRangedInstances : public GfxInstances, public StreamerCallback {

    protected:

    static const std::string className;

    GfxRangedInstances (const DiskResourcePtr<GfxMeshDiskResource> &mesh, const GfxNodePtr &par_);
    ~GfxRangedInstances ();

    struct Item {
        GfxRangedInstances *parent;
        int index; // maintained by the RangeSpace class
        Vector3 pos;
        Quaternion quat;
        bool activated;
        int activatedIndex;
        unsigned ticket;
        float renderingDistance;
        float lastFade;
        void updateSphere (const Vector3 &pos_, float r_)
        {
            renderingDistance = r_;
            pos = pos_;
            parent->mSpace.updateSphere(index, pos.x, pos.y, pos.z, r_);
        }
        void updateIndex (int index_) { index = index_; }
        float range2 (const Vector3 &new_pos) const
        {
            return (new_pos-pos).length2() / renderingDistance / renderingDistance;
        }
        float calcFade (float range2);
    };
    typedef std::vector<Item> Items;

    typedef CacheFriendlyRangeSpace<Item*> RS;
    typedef RS::Cargo Cargo;
    RS mSpace;
    Items items;
    Cargo activated;


    float mItemRenderingDistance;
    float mVisibility;
    float mStepSize;


    public:
    static GfxRangedInstancesPtr make (const std::string &mesh, const GfxNodePtr &par_=GfxNodePtr(NULL));

    void push_back (const SimpleTransform &t);
    void reserve (size_t s) {
        items.reserve(s);
        mSpace.reserve(s);
    };  
    size_t size (void) { return items.size(); }

    void registerMe (void);
    void unregisterMe (void);
    void update (const Vector3 &new_pos);
    void update (unsigned int inst, const Vector3 &pos, const Quaternion &q, float fade)
    { this->GfxInstances::update(inst, pos, q, fade); }

    public:

    virtual const Ogre::String& getMovableType (void) const
    {
        return className;
    }

    friend class SharedPtr<GfxRangedInstances>;
};

#endif
