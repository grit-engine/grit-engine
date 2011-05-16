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

#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>

#include "GfxDiskResource.h"

size_t gfx_gpu_ram_available (void)
{
        Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
        Ogre::MeshManager &mm = Ogre::MeshManager::getSingleton();
        return tm.getMemoryBudget() + mm.getMemoryBudget();
}

size_t gfx_gpu_ram_used (void)
{
        Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
        Ogre::MeshManager &mm = Ogre::MeshManager::getSingleton();
        return tm.getMemoryUsage() + mm.getMemoryUsage();
}


GfxDiskResource::GfxDiskResource (std::string name, const std::string &ext)
{
    name = name.substr(1);
    try {
        if (ext == "mesh") {
            Ogre::HardwareBuffer::Usage u = Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY;
            rp = Ogre::MeshManager::getSingleton()
                    .createOrRetrieve(name,"GRIT", false,0, 0, u,u,false,false).first;
        } else {
            rp = Ogre::TextureManager::getSingleton().createOrRetrieve(name,"GRIT").first;
        }
    } catch (Ogre::Exception &e) { 
        GRIT_EXCEPT2(e.getFullDescription(), "Looking for a graphics resource");
    }       
}

void GfxDiskResource::load (void)
{
    try {
        // do as much as we can, given that this is a background thread
        rp->prepare();
        DiskResource::load();
    } catch (Ogre::Exception &e) {
        CERR << e.getFullDescription() << std::endl;
    }       
}

void GfxDiskResource::unload(void)
{
    try {
        rp->unload();
        DiskResource::unload();
    } catch (Ogre::Exception &e) {
        CERR << e.getFullDescription() << std::endl;
    }       
}  

bool GfxDiskResource::isGPUResource (void)
{
    return true;
}

std::string GfxDiskResource::getName (void) const
{
    return "/"+rp->getName();
}
