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

#include <vector>
#include "vect_util.h"

class BackgroundLoader;
class Demand;
class DiskResource;
typedef std::vector<DiskResource*> DiskResources;
typedef fast_erase_vector<Demand*> Demands;

#ifndef BackgroundLoader_h
#define BackgroundLoader_h

#include <list>
#include <algorithm>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/recursive_mutex.hpp>


#include "CentralisedLog.h"


extern bool disk_resource_verbose_loads;
extern bool disk_resource_verbose_incs;

int disk_resource_num (void);
int disk_resource_num_loaded (void);
DiskResources disk_resource_all (void);
DiskResources disk_resource_all_loaded (void);
DiskResource *disk_resource_get_or_make (const std::string &rn);
DiskResource *disk_resource_get (const std::string &rn);

template<typename T> class LRUQueue {

    protected:

        typedef std::list<T> Queue;

        typedef typename Queue::size_type size_type;


    public:

        LRUQueue () : mSize(0) { }

        ~LRUQueue () { }

        inline void removeIfPresent (const T &v)
        {
                //if already in queue, remove
                typename Queue::iterator end = mQueue.end();
                typename Queue::iterator i = std::find(mQueue.begin(), end, v);
                if (i!=end) {
                        mQueue.erase(i);
                        mSize--;
                }
        }

        inline void push (const T &v)
        {
                //it only gets pushed if it's not referenced anywhere
                //so the following line is a waste of time
                //removeIfPresent(v);

                //add to young end of queue
                mQueue.push_front(v);
                mSize++;
        }

        inline T pop ()
        {
                //remove from old end of queue
                T v = mQueue.back();
                mQueue.pop_back();
                mSize--;
                return v;
        }

        inline size_type size () const { return mSize; }

        inline void clear () { mQueue.clear() ; mSize=0; }


    protected:

        size_t mSize;

        Queue mQueue;
};



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

class Demand : public fast_erase_index {
    public:
        Demand() : mInBackgroundQueue(false), incremented(false) { }

        void addDiskResource (const std::string &rn)
        {
                DiskResource *dr = disk_resource_get_or_make(rn);
                if (dr == NULL) return;
                resources.push_back(dr);
        }

        void clearDiskResources (void)
        {
                resources.clear();
        }

        // called by the main thread to get resources loaded prior
        // to activation of objects
        // repeated calls update the distance of the object, used for prioritising
        // the queue of demands
        // as a convenience, returns !isInBackgroundQueue()
        bool requestLoad (float dist);

        // is the demand registered to be procesed by the background thread
        // if returns true after a call to requestLoad, the resources should be loaded
        bool isInBackgroundQueue (void) { return mInBackgroundQueue; }

        // the object nolonger requires its resources, they may be unloaded if necessary
        // may be called when isInBackgroundQueue()==true in which case the demand
        // is removed from the background queue (may also be partially loaded at this point)
        void finishedWith (void);

    private:
        volatile bool mInBackgroundQueue; // is it in the bgl's queue
        DiskResources resources;
        volatile float mDist;
        friend class BackgroundLoader;
        bool incremented; // have we called increment() on required resources?
};


class BackgroundLoader {

    public:


        BackgroundLoader (void);

        ~BackgroundLoader (void);

        void shutdown (void);

        void add (Demand *d);

        void remove (Demand *d);

        void handleBastards (void);

        size_t size (void) { return mDemands.size(); }

        void setAllowance (float m);


        // background thread entry point
        void operator() (void);

        boost::recursive_mutex lock;
        boost::condition cVar;

    protected:

        bool nearestDemand (Demand * volatile &return_demand);

        DiskResources mBastards;
        volatile unsigned short mNumBastards;

        Demands mDemands;

        boost::thread *mThread;

        Demand * volatile mCurrent;
        volatile bool mQuit;

        float mAllowance;


    public:

        size_t getLRUQueueSizeGPU (void) const
        { return mDeathRowGPU.size(); }
        size_t getLRUQueueSizeHost (void) const
        { return mDeathRowHost.size(); }

        void finishedWith (DiskResource *);

        void checkRAMHost (void);
        void checkRAMGPU (void);

    protected:

        LRUQueue<DiskResource*> mDeathRowGPU;
        LRUQueue<DiskResource*> mDeathRowHost;

};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
