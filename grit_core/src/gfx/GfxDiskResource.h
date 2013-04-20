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
#include <OgreTexture.h>
#include <OgreMesh.h>

#include "../CentralisedLog.h"
#include "../BackgroundLoader.h"

/** Representation for Ogre resources.
 *
 * A loaded disk resource is only 'prepared' in Ogre parlance.  The actual Ogre
 * load, i.e. the movement of data from system memory to GPU memory, is done on
 * the render of the first frame where the resource is actually used.
 */
class GfxDiskResource : public DiskResource {

  public:

    /** Use disk_resource_get_or_make to create a new disk resource. */
    GfxDiskResource (const std::string &name);

    /** Returns true. */
    virtual bool isGPUResource (void) { return true; }

    /** Return Grit name for the resource, e.g. "/system/blah.dds". */
    virtual const std::string &getName (void) const { return name; }

    /** Return Ogre name for the resource, e.g. "system/blah.dds". */
    virtual std::string getOgreName (void) const = 0;

    /** Checks the dynamic type. */
    virtual bool isMesh (void) = 0;

  private:

    /** A cache of the Grit name, which is different to the Ogre name. */
    const std::string name;

    /** Load via Ogre (i.e. prepare it in Ogre terminology). */
    virtual void loadImpl (void) = 0;
    /** Reload from disk via Ogre calls. */
    virtual void reloadImpl (void) = 0;
    /** Unload via Ogre. */
    virtual void unloadImpl (void) = 0;

    friend class GfxBaseMaterial;
    friend class GfxMaterial;
    friend class GfxSkyMaterial;
};

/** Representation for textures.  Textures have no dependencies.
 */
class GfxTextureDiskResource : public GfxDiskResource {

  public:
    /** Use disk_resource_get_or_make to create a new disk resource. */
    GfxTextureDiskResource (const std::string &name);

    /** Return Ogre name for the resource, e.g. "system/blah.dds". */
    virtual std::string getOgreName (void) const { return rp->getName(); }

    /** Is it a texture or a mesh? */
    virtual bool isMesh (void) { return false; }

    /** Return the internal Ogre object. */
    const Ogre::TexturePtr &getOgreTexturePtr (void) { return rp; }

  private:
    /** The ogre representaiton. */
    Ogre::TexturePtr rp;

    /** Load via Ogre (i.e. prepare it in Ogre terminology). */
    virtual void loadImpl (void);
    /** Reload from disk via Ogre calls. */
    virtual void reloadImpl (void);
    /** Unload via Ogre. */
    virtual void unloadImpl (void);

};

/** Representation for meshes.
 * Meshes depend on textures via materials, so the material database is used
 * while loading meshes to determine the textures used.  For background
 * loading, this has concurrency implications.  Also, if the material changes
 * to use a different texture, the list of dependencies may become stale.  This
 * is not currently handled correctly.
 */
class GfxMeshDiskResource : public GfxDiskResource {

  public:
    /** Use disk_resource_get_or_make to create a new disk resource. */
    GfxMeshDiskResource (const std::string &name);

    /** Return Ogre name for the resource, e.g. "system/blah.dds". */
    virtual std::string getOgreName (void) const { return rp->getName(); }

    /** Is it a texture or a mesh? */
    virtual bool isMesh (void) { return false; }

    /** Return the internal Ogre object. */
    const Ogre::MeshPtr &getOgreMeshPtr (void) { return rp; }

  private:
    /** The ogre representaiton. */
    Ogre::MeshPtr rp;

    /** Load via Ogre (i.e. prepare it in Ogre terminology). */
    virtual void loadImpl (void);
    /** Reload from disk via Ogre calls. */
    virtual void reloadImpl (void);
    /** Unload via Ogre. */
    virtual void unloadImpl (void);

};

// Necessary modifications to children due to material changes
// * When a material changes texture, iterate through all meshes to find users of this material
// * For each mesh that uses the material:
// * * calculate: New set of children, old set of children
// * * new set of children are incremented, and loaded (foreground thread) if needed
// * * old set of children are decremented

/** How much RAM is available on the GPU.  This is actually a number provided by the user, via gfx_option(GFX_RAM). */
size_t gfx_gpu_ram_available (void);

/** How much RAM is used on the GPU. */
size_t gfx_gpu_ram_used (void);

/** Should additional log messages specifically about graphics resource loading/unloaded be emitted? */
extern bool gfx_disk_resource_verbose_loads;

#endif
