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

#ifndef GFX_DISK_RESOURCE_H
#define GFX_DISK_RESOURCE_H

#include <OgreResource.h>

#include "../CentralisedLog.h"
#include "../BackgroundLoader.h"

class GfxDiskResource : public DiskResource {

  public:
    GfxDiskResource (std::string name, const std::string &ext);

    virtual void load (void);

    virtual void unload (void);

    virtual bool isGPUResource (void);

    virtual std::string getName (void) const;

  private:
    Ogre::ResourcePtr rp;
    // have a vector of other DiskResource, increment them when this one is incremented
    // decrement them when this one is decremented
    // if a material is changed to use a different texture then
    // clearly this vector will be wrong
    // however the most harm this will do is to cause a loading stutter
    // It may also be possible to update this record when a material is changed
    // Even better: link to the materials from here, not the resources
    // then, we can do the increment thing by reading from the materials the correct value
    // The material will have a link to a DiskResource for each texture, it will not incrememnt it
    // if the material changes texture, it will have to cause the users of that material to decrement it
};

// When the resource is created, it is not known what other resources it depends on.

// When the resource is incremented, it is still not known.

// Dependent resources are only known when the resource is loaded.  In the case
// of meshes and textures, once the mesh is loaded (i.e. prepared in Ogre
// terminology) we sync with the main thread to get the textures for its
// material.  There is an issue of what happens when the material changes and
// the dependencies are stale.

// upon load, get vector<DiskResource>
// upon any change to dependencies, ensure 
// increment them all
// load them all if not already loaded

// upon unload
// decrement them all
// 

size_t gfx_gpu_ram_available (void);
size_t gfx_gpu_ram_used (void);

#endif
