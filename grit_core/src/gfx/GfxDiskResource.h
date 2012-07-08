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

class GfxDiskResource;

#ifndef GFX_DISK_RESOURCE_H
#define GFX_DISK_RESOURCE_H

#include <OgreResource.h>

#include "../CentralisedLog.h"
#include "../BackgroundLoader.h"

class GfxDiskResource : public DiskResource {

  public:
    GfxDiskResource (const std::string &name, const std::string &ext);

    virtual bool isGPUResource (void);

    virtual const std::string &getName (void) const { return name; }

    const Ogre::ResourcePtr &getOgreResourcePtr (void) { return rp; }

  private:
    Ogre::ResourcePtr rp;

    const std::string name;

    virtual void loadImpl (void);
    virtual void reloadImpl (void);
    virtual void unloadImpl (void);

    friend class GfxBaseMaterial;
    friend class GfxMaterial;
    friend class GfxSkyMaterial;

};

// Dependent resources are only known when the resource is loaded.  In the case
// of meshes and textures, once the mesh is loaded (i.e. prepared in Ogre
// terminology) we sync with the main thread to get the textures for its
// material.  There is an issue of what happens when the material changes and
// the dependencies are stale.

// Modifications to children at load or unload:

// * at load, will set children from empty set to the correct set, and increment
// * at unload, will set children to empty set, and decrement

// Modifications to children due to material changes

// * When a material changes texture, iterate through all meshes to find users of this material
// * For each mesh that uses the material:
// * * calculate: New set of children, old set of children
// * * new set of children are incremented, and loaded (foreground thread) if needed
// * * old set of children are decremented

size_t gfx_gpu_ram_available (void);
size_t gfx_gpu_ram_used (void);

extern bool gfx_disk_resource_verbose_loads;

#endif
