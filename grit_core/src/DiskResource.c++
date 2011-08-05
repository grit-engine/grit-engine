/* Copyright (c) David Cunningham and the Grit Game Engine project 2011
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


#include "gfx/GfxDiskResource.h"
#include "main.h"

bool disk_resource_verbose_loads = false;
bool disk_resource_verbose_incs = false;

typedef std::map<std::string, DiskResource*> DiskResourceMap;
DiskResourceMap disk_resource_map;

int disk_resource_num(void)
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
    return disk_resource_map[rn];
}

DiskResource *disk_resource_get_or_make (const std::string &rn)
{
    DiskResource *dr = disk_resource_map[rn];
    if (dr != NULL) return dr;

    const char *texture_formats[] = { "bmp", "jpg", "jpeg", "png", "tga", "targa",
                                      "tif", "tiff", "gif", "hdr", "dds" };
    unsigned num_texture_formats = sizeof(texture_formats)/sizeof(*texture_formats);
    size_t pos = rn.rfind('.');
    if (pos == rn.npos) {
        CERR << "Ignoring resource \""<<rn<<"\" as "
             << "it does not have a file extension." << std::endl;
        return dr;
    }
    std::string suffix(rn, pos+1);

    if (suffix == "mesh") {
        dr = new GfxDiskResource(rn,suffix);
    } else if (suffix == "tcol" || suffix == "gcol" || suffix == "bcol") {
        dr = new CollisionMesh(rn);
    } else if (suffix == "wav" || suffix == "ogg" || suffix == "mp3") {
        // TODO: sound
    } else {
        for (unsigned i=0 ; i<num_texture_formats ; ++i) {
            if (suffix == texture_formats[i]) {
                dr = new GfxDiskResource(rn,suffix);
                break;
            }
        }
    }
    if (dr == NULL) {
        CERR << "Ignoring resource \""<<rn<<"\" as "
             << "its file extension was not recognised." << std::endl;
        return dr;
    }
    disk_resource_map[rn] = dr;
    return dr;
}


void DiskResource::load (void)
{
    APP_ASSERT(!loaded);

    if (disk_resource_verbose_loads)
            CVERB << "LOAD " << getName() << std::endl;
    loaded = true;

    typedef ReloadWatcherSet::iterator I;
    for (I i=reloadWatchers.begin(),i_=reloadWatchers.end() ; i!=i_ ; ++i) {
        (*i)->notifyReloaded();
    }
}

void DiskResource::unload (void)
{
    APP_ASSERT(loaded);

    if (disk_resource_verbose_loads)
        CVERB << "FREE " << getName() << std::endl;
    APP_ASSERT(noUsers());
    for (unsigned i=0 ; i<dependencies.size() ; ++i) {
        dependencies[i]->decrement();
        bgl->finishedWith(dependencies[i]);
    }
    dependencies.clear();
    loaded = false;
}



