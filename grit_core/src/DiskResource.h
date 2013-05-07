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
#include <set>

class DiskResource;
typedef std::vector<DiskResource*> DiskResources;

#ifndef DiskResource_h
#define DiskResource_h

#include <string>

#include "CentralisedLog.h"

/** \file
 *
 * Grit's resource subsystem allows disk data to transition between a loaded
 * and unloaded state.  The intention is to manage memory usage by only keeping
 * a necessary subset of all the disk resources loaded simultaneously.  There
 * are many kinds of disk resources, including meshes, textures, audio files,
 * collision meshes, etc.  In each case, 'unloaded' means existing only on
 * disk, and 'loaded' means ready for use.  There is also a system for
 * registering one's need for a particular resource, so that automatic memory
 * management can avoid unloading resources that are currently in use.
 *
 * In addition to this, one can reload resources, which is useful for
 * development of games, where e.g. textures can be tweaked on disk and
 * reloaded in the game for instant preview.
 */

/** Whether or not disk resource (un)loads trigger a log message. */
extern bool disk_resource_verbose_loads;

/** Whether or not disk resource inc/decrements trigger a log message. */
extern bool disk_resource_verbose_incs;

/** Number of disk resources in existence. */
unsigned long disk_resource_num (void);

/** Number of disk resources that are loaded. */
int disk_resource_num_loaded (void);

/** A vector of every disk resource in existence. */
DiskResources disk_resource_all (void);

/** A vector of every disk resource in existence that is also loaded. */
DiskResources disk_resource_all_loaded (void);

/** Either retrieve the disk resource of the given name, or create a new one.
 * The disk resource will begin life unloaded.
 */
DiskResource *disk_resource_get_or_make (const std::string &rn);

/** Retrieve the existing disk resource or NULL if it does not exist. */
DiskResource *disk_resource_get (const std::string &rn);

/** Test if a disk resource of the given name exists. */
bool disk_resource_has (const std::string &rn);

/** Represents some data on disk that must be loaded before use.
 *
 * Subclasses of DiskResource define different kinds of data, such as audio,
 * textures, etc.  Each disk resource can be in a loaded or unloaded state.  In
 * addition to this, each resource keeps a counter of the number of 'users' it
 * has, who register/unregister their need for the resource.  Loaded resources
 * with no users are subject to be automatically unloaded if system memory
 * becomes tight.
 *
 * Note that users>0 and loaded are not logically related.  Just because a
 * resource is loaded, does not mean it is still in use, as resources are
 * cached according to a least-recently-used policy.  Just because a resource
 * has registered users, does not mean it is loaded, as one can explicitly
 * unload a resource, or a resource can be marked as being used but not be
 * loaded yet.
 *
 * Each resource can depend on any number of other resources to form an acylic
 * graph.  This is e.g. for meshes that use materials (and thus textures), and
 * also for collision meshes that have procedural vegetation. The dependencies
 * are discovered when loading the resource, and loading a resource will load
 * its dependencies automatically.  Therefore the set of dependencies of an
 * unloaded resource is always empty.  Also, if users>0 on a resource, then the
 * dependencies automatically have a user registered for the depending resource
 * so one only has to register use of the top-level resource.
 */
class DiskResource {

    public:

    /** A callback to tell you if this resource is reloaded. */
    struct ReloadWatcher {
        virtual ~ReloadWatcher (void) { };
        virtual void notifyReloaded (DiskResource *dr) = 0;
    };

    /** Do not use this, call the disk_resource_get function instead. */
    DiskResource (void) : loaded(false), users(0) { }

    /** The filename, as an absolute unix-style path from the root of the
     * game directory. */
    virtual const std::string &getName (void) const = 0;

    /** Is the resource loaded and therefore ready for use? */
    bool isLoaded (void) { return loaded; }

    /** Number of users of this resource. */
    int getUsers (void) { return users; }

    /** Are there no users? */
    bool noUsers() { return users == 0; }

    /** Register a callback for discovering when a resource is reloaded. */
    void registerReloadWatcher (ReloadWatcher *u) { reloadWatchers.insert(u); }

    /** Unregister a callback for discovering when a resource is reloaded. */
    void unregisterReloadWatcher (ReloadWatcher *u) { reloadWatchers.erase(u); }

    /** Call all the reload watchers.  This is called automatically so you
     * almost certainly don't need to call it yourself. */
    void callReloadWatchers ();

    /** Is this resource to be unloaded if there is GPU memory pressure?  There
     * are two kinds of memory, GPU memory (on the graphics card) and system
     * memory.  Textures and meshes take up both kinds, everything else only takes
     * system memory.  Therefore if there is GPU memory pressure, unloading a
     * collsion mesh will not help.
    */
    virtual bool isGPUResource (void)
    {
        return false;
    }

    /** Register yourself as a user.  This stops the resource being unloaded while you're using it. */
    void increment (void)
    {
        if (disk_resource_verbose_incs)
            CVERB << "++ " << getName() << std::endl;
        users++;
    }

    /** Inform that you are no-longer using this resource. */
    bool decrement (void)
    {
        if (disk_resource_verbose_incs)
            CVERB << "-- " << getName() << std::endl;
        users--;
        // do not unload at this time, will be added to LRU queue by caller
        return users == 0;
    }

    /** Update internal state from disk.  The resource is never actually unloaded at any point. */
    void reload (void);

    /** Load from disk. */
    void load (void);

    /** Erase from memory, the only copy will be on disk. */
    void unload (void);

    /** Subclasses should register dependencies at load time. */
    void addDependency (const std::string &name)
    {
        APP_ASSERT(name!="");
        DiskResource *dep = disk_resource_get_or_make(name);
        addDependency(dep);
    }

    /** Subclasses should register dependencies at load time. */
    void addDependency (DiskResource *dep)
    {
        dependencies.push_back(dep);
        dep->increment();
        if (!dep->isLoaded())
            dep->load();
    }

    protected:

    /** Subclasses override to implement reloading. */
    virtual void reloadImpl (void) { unloadImpl(); loaded = false; loadImpl(); loaded = true; }

    /** Subclasses override to implement loading. */
    virtual void loadImpl (void) { }

    /** Subclasses override to implement unloading. */
    virtual void unloadImpl (void) { }

    private:

    /** The dependencies of this disk resource that must be loaded when it is. */
    DiskResources dependencies;

    /** Store loaded state. */
    bool loaded;

    /** Store number of users (like a reference counter). */
    int users;

    /** Type for storage of reload callbacks. */
    typedef std::set<ReloadWatcher*> ReloadWatcherSet;

    /** Storage for reload callbacks. */
    ReloadWatcherSet reloadWatchers;

    friend class Demand;
    friend class BackgroundLoader;
};

/** Allow writing a disk resource to a stream, printing its name. */
inline std::ostream &operator << (std::ostream &o, const DiskResource &dr)
{
    return o << dr.getName();
}

/** Allow writing a vector of disk resources to a stream, printing a list of their names. */
inline std::ostream &operator << (std::ostream &o, const DiskResources &dr)
{
    o << "[";
    for (unsigned i=0 ; i<dr.size() ; ++i) {
        o << (i==0?" ":", ") << (*dr[i]);
    }
    return o << (dr.size()==0?"]":" ]");
}

#endif
