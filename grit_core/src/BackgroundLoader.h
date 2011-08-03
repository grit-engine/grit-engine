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
typedef fast_erase_vector<Demand*> Demands;

#ifndef BackgroundLoader_h
#define BackgroundLoader_h

#include <list>
#include <algorithm>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/recursive_mutex.hpp>


#include "CentralisedLog.h"
#include "DiskResource.h"


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
