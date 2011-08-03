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

#include <vector>

class DiskResource;
typedef std::vector<DiskResource*> DiskResources;

#ifndef DiskResource_h
#define DiskResource_h

extern bool disk_resource_verbose_loads;
extern bool disk_resource_verbose_incs;

int disk_resource_num (void);
int disk_resource_num_loaded (void);
DiskResources disk_resource_all (void);
DiskResources disk_resource_all_loaded (void);
DiskResource *disk_resource_get_or_make (const std::string &rn);
DiskResource *disk_resource_get (const std::string &rn);

// each DiskResource can be in a loaded or unloaded state.
// the predicates 'users>0' and 'loaded' are unrelated by logic
// Example of users>0 && !loaded: demand placed in queue, not yet loaded
// Example of users==0 && loaded: resource in LRU queue, no pressure to unload yet
// !loaded => dependencies.size()==0
// loaded  => forall d in dependencies: d.loaded
// users>0 => forall d in dependencies: d.users > 0
class DiskResource {

    public:

        DiskResource (void) : loaded(false), users(0) { }

        virtual std::string getName (void) const = 0;

        bool isLoaded (void) { return loaded; }

        int getUsers (void) { return users; }

        bool noUsers() { return users == 0; }

    protected:

        virtual void load (void);

        virtual void unload (void);

        // if returns true, will get added to gpu death row list when no-longer required
        virtual bool isGPUResource (void)
        {
                return false;
        }

        // called as 'this' is loaded
        void addDependency (const std::string &name)
        {
                APP_ASSERT(name!="");
                DiskResource *dep = disk_resource_get_or_make(name);
                dependencies.push_back(dep);
                dep->increment();
                dep->load();
        }

    private:

        void increment (void)
        {
                if (disk_resource_verbose_incs)
                        CVERB << "++ " << getName() << std::endl;
                users++;
        }

        bool decrement (void)
        {
                if (disk_resource_verbose_incs)
                        CVERB << "-- " << getName() << std::endl;
                users--;
                // do not unload at this time, will be added to LRU queue by caller
                return users == 0;
        }

        DiskResources dependencies;

        bool loaded;
        int users;


        friend class Demand;
        friend class BackgroundLoader;
};

inline std::ostream &operator << (std::ostream &o, const DiskResource &dr)
{
        return o << dr.getName();
}

inline std::ostream &operator << (std::ostream &o, const DiskResources &dr)
{
        o << "[";
        for (unsigned i=0 ; i<dr.size() ; ++i) {
                o << (i==0?" ":", ") << (*dr[i]);
        }
        return o << (dr.size()==0?"]":" ]");
}

#endif
