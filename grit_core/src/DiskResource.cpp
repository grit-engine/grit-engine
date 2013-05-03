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


#include "gfx/gfx_disk_resource.h"
#include "audio/AudioDiskResource.h"
#include "main.h"

bool disk_resource_verbose_loads = false;
bool disk_resource_verbose_incs = false;

// should map nothing to NULL
typedef std::map<std::string, DiskResource*> DiskResourceMap;
DiskResourceMap disk_resource_map;

bool disk_resource_has (const std::string &n)
{
    DiskResourceMap::iterator it = disk_resource_map.find(n);
    if (it == disk_resource_map.end()) return false;
    return true;
}

unsigned long disk_resource_num (void)
{
    return disk_resource_map.size();
}

int disk_resource_num_loaded (void)
{
    int r = 0;
    DiskResourceMap &m = disk_resource_map;
    for (DiskResourceMap::iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
        if (i->second->isLoaded()) r++;
    }
    return r;
}

DiskResources disk_resource_all (void)
{
    DiskResources r;
    DiskResourceMap &m = disk_resource_map;
    for (DiskResourceMap::iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
        r.push_back(i->second);
    }
    return r;
}

DiskResources disk_resource_all_loaded (void)
{
    DiskResources r;
    DiskResourceMap &m = disk_resource_map;
    for (DiskResourceMap::iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
        if (i->second->isLoaded())
            r.push_back(i->second);
    }
    return r;
}
 
DiskResource *disk_resource_get (const std::string &rn)
{
    if (!disk_resource_has(rn)) return NULL;
    return disk_resource_map[rn];
}

static bool ends_with (const std::string &str, const std::string &snippet)
{
    if (snippet.length() > str.length()) return false;
    return str.substr(str.length() - snippet.length()) == snippet;
}

DiskResource *disk_resource_get_or_make (const std::string &rn)
{
    DiskResource *dr = disk_resource_get(rn);
    if (dr != NULL) return dr;

    size_t pos = rn.rfind('.');
    if (pos == rn.npos) {
        GRIT_EXCEPT("Ignoring resource \""+rn+"\" as "
                    "it does not have a file extension.");
    }
    std::string suffix(rn, pos+1);

    if (suffix == "mesh") {
        dr = new GfxMeshDiskResource(rn);
    } else if (suffix == "tcol" || suffix == "gcol" || suffix == "bcol") {
        dr = new CollisionMesh(rn);
    } else if (suffix == "wav" || suffix == "ogg" || suffix == "mp3") {
        dr = new AudioDiskResource(rn);
    } else if (suffix == "wav" || suffix == "ogg" || suffix == "mp3") {
        dr = new AudioDiskResource(rn);
    } else if (ends_with(rn, ".envcube.tiff")) {
        dr = new GfxEnvCubeDiskResource(rn);
    } else if (ends_with(rn, ".lut.png")) {
        dr = new GfxColourGradeLUTDiskResource(rn);
    } else if (ends_with(rn, ".lut.tiff")) {
        dr = new GfxColourGradeLUTDiskResource(rn);
    } else {
        const char *texture_formats[] = { "jpeg", "png", "tga", "tiff", "hdr", "dds" };
        unsigned num_texture_formats = sizeof(texture_formats)/sizeof(*texture_formats);
        for (unsigned i=0 ; i<num_texture_formats ; ++i) {
            if (suffix == texture_formats[i]) {
                dr = new GfxTextureDiskResource(rn);
                break;
            }
        }
    }
    if (dr == NULL) {
        GRIT_EXCEPT("Ignoring resource \""+rn+"\" as "
                    "its file extension was not recognised.");
    }
    disk_resource_map[rn] = dr;
    return dr;
}

void DiskResource::callReloadWatchers (void)
{
    typedef ReloadWatcherSet::iterator I;
    for (I i=reloadWatchers.begin(),i_=reloadWatchers.end() ; i!=i_ ; ++i) {
        (*i)->notifyReloaded(this);
    }
}


void DiskResource::reload (void)
{
    APP_ASSERT(loaded);
    reloadImpl();
    callReloadWatchers();
}

void DiskResource::load (void)
{
    APP_ASSERT(!loaded);

    loadImpl();

    if (disk_resource_verbose_loads)
            CVERB << "LOAD " << getName() << std::endl;
    loaded = true;

    callReloadWatchers();
}

void DiskResource::unload (void)
{
    APP_ASSERT(loaded);
    APP_ASSERT(noUsers());

    if (disk_resource_verbose_loads)
        CVERB << "FREE " << getName() << std::endl;
    for (unsigned i=0 ; i<dependencies.size() ; ++i) {
        dependencies[i]->decrement();
        bgl->finishedWith(dependencies[i]);
    }
    dependencies.clear();
    unloadImpl();
    loaded = false;
}



