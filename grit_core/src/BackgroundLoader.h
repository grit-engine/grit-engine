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
class DiskResource {

    public:

        DiskResource (void) : loaded(false), users(0) { }

        virtual std::string getName (void) const = 0;

    protected:

        virtual void load (void) 
        {
                loaded = true;
        }

        virtual void unload (void)
        {
                APP_ASSERT(noUsers());
                loaded = false;
        }

        // if returns true, will get added to gpu death row list when no-longer required
        virtual bool isGPUResource (void)
        {
                return false;
        }

    private:

        bool noUsers() { return users == 0; }

        void increment (void)
        {
                users++;
        }

        bool decrement (void)
        {
                users--;
                // do not unload at this time, will be added to LRU queue by caller
                return users == 0;
        }

        bool isLoaded (void) { return loaded; }

        bool loaded;
        int users;

        friend class Demand;
        friend class BackgroundLoader;
};

inline std::ostream &operator << (std::ostream &o, const DiskResource &dr)
{
        return o << dr.getName();
}

DiskResource *disk_resource_get_or_make (const std::string &rn);

class Demand : public fast_erase_index {
    public:
        Demand() : mBeingLoaded(false) { }

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

        bool requestLoad (float dist);
        bool isBeingLoaded (void) { return mBeingLoaded; }
        void finishedWith (void);

    private:
        volatile bool mBeingLoaded; // is it in the bgl's queue
        DiskResources resources;
        volatile float mDist;
        friend class BackgroundLoader;
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

    protected:

        bool nearestDemand (Demand * volatile &return_demand);

        boost::recursive_mutex lock;
        boost::condition cVar;

        DiskResources mBastards;
        volatile unsigned short mNumBastards;

        Demands mDemands;

        boost::thread *mThread;

        Demand * volatile mCurrent;
        volatile bool mQuit;

        float mAllowance;


    public:

/*
        void refreshDiskResource (DiskResource *v)
        {
                mDeathRow.removeIfPresent(v);
        }
*/
                

        size_t getLRUQueueSizeGPU (void) const
        { return mDeathRowGPU.size(); }
        size_t getLRUQueueSizeHost (void) const
        { return mDeathRowHost.size(); }

        void finishedWith (const DiskResources &);
        void finishedWith (DiskResource *);

        void checkRAMHost (void);
        void checkRAMGPU (void);

    protected:

        LRUQueue<DiskResource*> mDeathRowGPU;
        LRUQueue<DiskResource*> mDeathRowHost;

};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
