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

#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>
#include <OgreMeshSerializer.h>
#include <OgreMeshFileFormat.h>

#include "gfx_disk_resource.h"
#include "gfx_internal.h"
#include "GfxMaterial.h"
#include "GfxBody.h"
#include "GfxSkyBody.h"


bool gfx_disk_resource_verbose_loads = false;

size_t gfx_gpu_ram_available (void)
{
    return gfx_option(GFX_RAM) * 1024 * 1024;
}

size_t gfx_gpu_ram_used (void)
{
    Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
    Ogre::MeshManager &mm = Ogre::MeshManager::getSingleton();
    return tm.getMemoryUsage() + mm.getMemoryUsage();
}


GfxMeshDiskResource::GfxMeshDiskResource (const std::string &name)
    : GfxGPUDiskResource(name)
{
    try {
        std::string ogre_name = name.substr(1);
        Ogre::HardwareBuffer::Usage u = Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY;
        rp = Ogre::MeshManager::getSingleton()
                .createOrRetrieve(ogre_name,RESGRP, false,0, 0, u,u,false,false).first;
    } catch (Ogre::Exception &e) { 
        GRIT_EXCEPT2(e.getFullDescription(), "Looking for a graphics resource");
    }       
}


// hack to get access to stuff
class MyMeshHack : public Ogre::Mesh {
    public:
    static Ogre::DataStreamPtr getData (const Ogre::MeshPtr &mp) 
    {
        return ((*mp).*&MyMeshHack::mFreshFromDisk);
    }
};

// get the material names only
class MyMeshDeserializer : public Ogre::Serializer {

    public:

    MyMeshDeserializer (const Ogre::DataStreamPtr stream, Ogre::MeshSerializerListener *listener)
    {
        mStream = stream;
        mListener = listener;
        mVersion = "[MeshSerializer_v1.8]";
    }
    
    void importMatNames (const Ogre::MeshPtr &pMesh, std::vector<std::string> &mat_names)
    {

        mStream->seek(0);

        determineEndianness(mStream);

        // Read header and determine the version
        // Read header ID
        unsigned short headerID;
        readShorts(mStream, &headerID, 1);

        // Read version
        std::string ver = readString(mStream);
        // Jump back to start
        mStream->seek(0);

        if (ver != mVersion) {
            GRIT_EXCEPT("Mesh \""+pMesh->getName()+"\" wrong version "+ver);
        }

        readFileHeader(mStream);

        unsigned short streamID = readChunk(mStream);
        APP_ASSERT(streamID == Ogre::M_MESH);

        bool skeletallyAnimated;
        readBools(mStream, &skeletallyAnimated, 1);

        for (streamID = readChunk(mStream) ; !mStream->eof() ; streamID = readChunk(mStream)) {
            bool broken = false;
            switch (streamID) {
                // ignore all of these
                case Ogre::M_ANIMATION: 
                case Ogre::M_ANIMATION_MORPH_KEYFRAME: 
                case Ogre::M_ANIMATION_POSE_KEYFRAME: 
                case Ogre::M_ANIMATION_POSE_REF: 
                case Ogre::M_ANIMATIONS: 
                case Ogre::M_ANIMATION_TRACK: 
                case Ogre::M_EDGE_GROUP: 
                case Ogre::M_EDGE_LIST_LOD: 
                case Ogre::M_EDGE_LISTS: 
                case Ogre::M_GEOMETRY: 
                case Ogre::M_GEOMETRY_VERTEX_BUFFER: 
                case Ogre::M_GEOMETRY_VERTEX_BUFFER_DATA: 
                case Ogre::M_GEOMETRY_VERTEX_DECLARATION: 
                case Ogre::M_GEOMETRY_VERTEX_ELEMENT: 
                case Ogre::M_HEADER: 
                case Ogre::M_MESH: 
                case Ogre::M_MESH_BONE_ASSIGNMENT: 
                case Ogre::M_MESH_BOUNDS: 
                case Ogre::M_MESH_LOD: 
                case Ogre::M_MESH_LOD_GENERATED: 
                case Ogre::M_MESH_LOD_MANUAL: 
                case Ogre::M_MESH_LOD_USAGE: 
                case Ogre::M_MESH_SKELETON_LINK: 
                case Ogre::M_POSE: 
                case Ogre::M_POSES: 
                case Ogre::M_POSE_VERTEX: 
                case Ogre::M_SUBMESH_BONE_ASSIGNMENT: 
                case Ogre::M_SUBMESH_NAME_TABLE: 
                case Ogre::M_SUBMESH_NAME_TABLE_ELEMENT: 
                case Ogre::M_SUBMESH_OPERATION: 
                case Ogre::M_SUBMESH_TEXTURE_ALIAS: 
                case Ogre::M_TABLE_EXTREMES: 
                break;
                // this is what we're looking for
                case Ogre::M_SUBMESH: {
                    size_t pos = mStream->tell();
                    std::string mat_name = readString(mStream);
                    mListener->processMaterialName(&*pMesh, &mat_name);
                    mat_names.push_back(mat_name);
                    mStream->seek(pos); // back to the 'overhead'
                } break;
                // error case if streamID is unknown
                default:
                    CERR << "Corrupted mesh, not loading textures: \""<<pMesh->getName()<<"\"" << std::endl;
                    mat_names.clear();
                    broken = true;
                    break;
            }
            if (broken) break;
            int stream_overhead_size = 6; // not exposed from Ogre
            mStream->skip(mCurrentstreamLen - stream_overhead_size); 
        }

        mStream->seek(0);

    }
    
    Ogre::MeshSerializerListener *mListener;
};


void GfxMeshDiskResource::loadImpl (void)
{
    APP_ASSERT(!isLoaded());
    try {
        if (!rp->isLoaded()) {
            // do as much as we can, given that this is a background thread
            if (gfx_disk_resource_verbose_loads)
                CVERB << "Preparing an Ogre::Resource: " << rp->getName() << std::endl;
            rp->prepare();

            // for meshes, scan the bytes from disk to extract materials, then work out
            // what textures we are depending on...
            Ogre::DataStreamPtr mesh_data = MyMeshHack::getData(rp);
            std::vector<std::string> mat_names;
            MyMeshDeserializer mmd(mesh_data, Ogre::MeshManager::getSingleton().getListener());
            mmd.importMatNames(rp, mat_names);

            GFX_MAT_SYNC;
            if (gfx_disk_resource_verbose_loads) {
                CVERB << "materials = [";
                for (unsigned i=0 ; i<mat_names.size() ; ++i) {
                    CLOG << (i==0?" ":", ") << mat_names[i];
                }
                CLOG << "]" << std::endl;
            }

            // sync with main thread to get texture names
            for (unsigned i=0 ; i<mat_names.size() ; ++i) {
                if (gfx_material_has_any(mat_names[i])) {
                    gfx_material_add_dependencies(mat_names[i], this);
                } else {
                    // Mesh references non-existing material, so silently ignore.
                    // When it is later used, an error will be raised then.
                    // This is actually better, as the error is raised at the point of use...
                    // Rather than asynchronously, in advance of use.
                }
            }
            
        } else {
            CVERB << "Internal warning: Loaded in OGRE, unloaded in GRIT: \"" << getName() << "\"" << std::endl;
        }
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT(e.getDescription());
    }
}

void GfxMeshDiskResource::reloadImpl(void)
{
    if (gfx_disk_resource_verbose_loads)
        CVERB << "OGRE: Reloading: " << rp->getName() << std::endl;
    try {
        rp->reload();
        if (rp->hasSkeleton()) rp->getSkeleton()->reload();
        for (unsigned long i=0 ; i<gfx_all_bodies.size() ; ++i) {
            GfxBody *b = gfx_all_bodies[i];
            if (b->mesh == rp) b->reinitialise();
        }
        for (unsigned long i=0 ; i<gfx_all_sky_bodies.size() ; ++i) {
            GfxSkyBody *b = gfx_all_sky_bodies[i];
            if (b->mesh == rp) b->reinitialise();
        }
    } catch (Ogre::Exception &e) {
        CERR << e.getFullDescription() << std::endl;
    }       
}  

void GfxMeshDiskResource::unloadImpl(void)
{
    if (gfx_disk_resource_verbose_loads)
        CVERB << "OGRE: Unloading: " << rp->getName() << std::endl;
    try {
        rp->unload();
    } catch (Ogre::Exception &e) {
        CERR << e.getFullDescription() << std::endl;
    }       
}  



GfxTextureDiskResource::GfxTextureDiskResource (const std::string &name)
    : GfxGPUDiskResource(name)
{
    try {
        std::string ogre_name = name.substr(1);
        rp = Ogre::TextureManager::getSingleton().createOrRetrieve(ogre_name,RESGRP).first;
    } catch (Ogre::Exception &e) { 
        GRIT_EXCEPT2(e.getFullDescription(), "Looking for a graphics resource");
    }       
}

void GfxTextureDiskResource::loadImpl (void)
{
    APP_ASSERT(!isLoaded());
    try {
        if (!rp->isLoaded()) {
            // do as much as we can, given that this is a background thread
            if (gfx_disk_resource_verbose_loads)
                CVERB << "Preparing an Ogre::Resource: " << rp->getName() << std::endl;
            rp->prepare();

        } else {
            CVERB << "Internal warning: Loaded in OGRE, unloaded in GRIT: \"" << getName() << "\"" << std::endl;
        }
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT(e.getDescription());
    }
}

void GfxTextureDiskResource::reloadImpl(void)
{
    if (gfx_disk_resource_verbose_loads)
        CVERB << "OGRE: Reloading: " << rp->getName() << std::endl;
    try {
        rp->reload();
    } catch (Ogre::Exception &e) {
        CERR << e.getFullDescription() << std::endl;
    }       
}  

void GfxTextureDiskResource::unloadImpl(void)
{
    if (gfx_disk_resource_verbose_loads)
        CVERB << "OGRE: Unloading: " << rp->getName() << std::endl;
    try {
        rp->unload();
    } catch (Ogre::Exception &e) {
        CERR << e.getFullDescription() << std::endl;
    }       
}  




GfxEnvCubeDiskResource::GfxEnvCubeDiskResource (const std::string &name)
    : GfxGPUDiskResource(name)
{
    APP_ASSERT(name.length()>0 && name[0]=='/');
    try {
        std::string ogre_name = name.substr(1);
        rp = Ogre::TextureManager::getSingleton().createManual(ogre_name, RESGRP, Ogre::TEX_TYPE_CUBE_MAP, 512, 512, Ogre::MIP_DEFAULT, Ogre::PF_UNKNOWN);
    } catch (Ogre::Exception &e) { 
        GRIT_EXCEPT2(e.getFullDescription(), "Creating an env cube");
    }       
}


/* This is a procedural tiny cube if we ever need it:
        // Prepare texture (6(1x1) RGB8) 
        uint8_t raw_tex_stack[] = { 
            0xff, 0x7f, 0x7f, 
            0x00, 0x7f, 0x7f, 
            0x7f, 0xff, 0x7f, 
            0x7f, 0x00, 0x7f, 
            0x7f, 0x7f, 0xff, 
            0x7f, 0x7f, 0x00 
        }; 
        // Load raw byte array into an Image 
        img.loadDynamicImage(raw_tex_stack, 1, 1, 1, Ogre::PF_B8G8R8, false, 6, 0); 
*/

void GfxEnvCubeDiskResource::loadImpl (void)
{
    APP_ASSERT(!isLoaded());
    uint8_t *raw_tex = NULL;
    try {
        const std::string &ogre_name = rp->getName();

        if (gfx_disk_resource_verbose_loads)
            CVERB << "Loading env cube: " << ogre_name << std::endl;

        if (rp->isLoaded()) {
            CERR << "WARNING: env cube "<<ogre_name<<" should not be loaded in Ogre" << std::endl;
            rp->unload();
        }

        Ogre::Image disk;
        disk.load (ogre_name, RESGRP);
        if (disk.getWidth() != disk.getHeight()*6) {
            GRIT_EXCEPT("Environment map has incorrect dimensions: "+ogre_name);
        }
        unsigned sz = disk.getHeight();
        if (sz & (sz-1) ) {
            GRIT_EXCEPT("Environment map size not a power of 2: "+ogre_name);
        }
        raw_tex = new uint8_t[disk.getSize()];

        Ogre::Image img;
        img.loadDynamicImage(raw_tex, sz, sz, 1, disk.getFormat(), true, 6, 0);
        // copy faces across
        Ogre::PixelBox from = disk.getPixelBox();
        for (unsigned face=0 ; face<6 ; ++face) {
            Ogre::PixelBox to = img.getPixelBox(face,0);
            for (unsigned y=0 ; y<sz ; ++y) {
                for (unsigned x=0 ; x<sz ; ++x) {
                    to.setColourAt(from.getColourAt(face*sz + x, y, 0), x, y, 0);
                }
            }
        }


        rp->loadImage(img);

    } catch (Ogre::Exception &e) {

        GRIT_EXCEPT(e.getDescription());
    }
}

void GfxEnvCubeDiskResource::unloadImpl(void)
{
    if (gfx_disk_resource_verbose_loads)
        CVERB << "OGRE: Unloading env cube: " << rp->getName() << std::endl;
    try {
        rp->unload();
    } catch (Ogre::Exception &e) {
        CERR << e.getFullDescription() << std::endl;
    }       
}  




GfxColourGradeLUTDiskResource::GfxColourGradeLUTDiskResource (const std::string &name)
    : GfxGPUDiskResource(name)
{
    APP_ASSERT(name.length()>0 && name[0]=='/');
    try {
        std::string ogre_name = name.substr(1);
        rp = Ogre::TextureManager::getSingleton().createManual(ogre_name, RESGRP, Ogre::TEX_TYPE_3D, 32, 32, 32, Ogre::MIP_DEFAULT, Ogre::PF_UNKNOWN);
    } catch (Ogre::Exception &e) { 
        GRIT_EXCEPT2(e.getFullDescription(), "Creating an env cube");
    }       
}

void GfxColourGradeLUTDiskResource::loadImpl (void)
{
    APP_ASSERT(!isLoaded());
    uint8_t *raw_tex = NULL;
    try {
        const std::string &ogre_name = rp->getName();

        if (gfx_disk_resource_verbose_loads)
            CVERB << "Loading colour grade LUT: " << ogre_name << std::endl;

        if (rp->isLoaded()) {
            CERR << "WARNING: colour grade "<<ogre_name<<" should not be loaded in Ogre" << std::endl;
            rp->unload();
        }

        Ogre::Image disk;
        disk.load (ogre_name, RESGRP);
        unsigned sz = disk.getHeight();
        if (disk.getWidth() != sz*sz) {
            GRIT_EXCEPT("Colour grade LUT has incorrect dimensions: "+ogre_name);
        }
        if (sz & (sz-1) ) {
            GRIT_EXCEPT("Colour grade LUT size not a power of 2: "+ogre_name);
        }
        raw_tex = new uint8_t[disk.getSize()];

        Ogre::Image img;
        img.loadDynamicImage(raw_tex, sz, sz, sz, disk.getFormat(), true, 1, 0);
        // copy faces across
        Ogre::PixelBox from = disk.getPixelBox();
        Ogre::PixelBox to = img.getPixelBox();
        for (unsigned z=0 ; z<sz ; ++z) {
            for (unsigned y=0 ; y<sz ; ++y) {
                for (unsigned x=0 ; x<sz ; ++x) {
                    to.setColourAt(from.getColourAt(z*sz + x, y, 0), x, y, z);
                }
            }
        }


        rp->loadImage(img);

    } catch (Ogre::Exception &e) {

        GRIT_EXCEPT(e.getDescription());
    }
}

void GfxColourGradeLUTDiskResource::unloadImpl(void)
{
    if (gfx_disk_resource_verbose_loads)
        CVERB << "OGRE: Unloading colour grade LUT: " << rp->getName() << std::endl;
    try {
        rp->unload();
    } catch (Ogre::Exception &e) {
        CERR << e.getFullDescription() << std::endl;
    }       
}  

