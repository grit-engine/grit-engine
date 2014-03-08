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
#include "vect_util.h"

class BackgroundLoader;
class Demand;
typedef fast_erase_vector<Demand*> Demands;

#ifndef BACKGROUNDLOADER_H
#define BACKGROUNDLOADER_H

#include <list>
#include <algorithm>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/recursive_mutex.hpp>


#include "centralised_log.h"
#include "disk_resource.h"

/** A least recently used queue, used to select resources to unload in the
 * event of memory pressure.  The basic idea is you push to the queue when you
 * no-longer need something, and you pop if you need to free space.  If you
 * start using something, you call removeIfPresent to mark it as being used
 * again.
 */
template<typename T> class LRUQueue {

    protected:

        typedef std::list<T> Queue;

        typedef typename Queue::size_type size_type;


    public:

        /** Create an empty queue. */
        LRUQueue () : mSize(0) { }

        /** Destructor. */
        ~LRUQueue () { }

        /** If v is in the queue, remove it, otherwise a no-op. */
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

        /** Add a new object v to the queue.  This means you're not intending to use it anymore, but it is the most recently used thing. */
        inline void push (const T &v)
        {
                //it only gets pushed if it's not referenced anywhere
                //so the following line is a waste of time
                //removeIfPresent(v);

                //add to young end of queue
                mQueue.push_front(v);
                mSize++;
        }

        /** Retrieve the least-recently used thing, and remove it from the queue. */
        inline T pop ()
        {
                //remove from old end of queue
                T v = mQueue.back();
                mQueue.pop_back();
                mSize--;
                return v;
        }

        /** Return the number of elements in the queue.  This is the number of
         * resources that are loaded but not being used (i.e. it's a cache in case they
         * need to be used again. */
        inline size_type size () const { return mSize; }

        /** Make the queue empty. */
        inline void clear () { mQueue.clear() ; mSize=0; }


    protected:

        /** Cache of the number of elements in the queue. */
        size_t mSize;

        /** The queue itself, a linked list. */
        Queue mQueue;
};


/** When a GritObject wants to load something, it registers a 'demand' with the
 * BackgroundLoader.  This acts as a channel of communication.  The  object can
 * communicate its changing position (the distance to the demand is used by the
 * background loader to prioritise loads), and the list of resources needed.
 * The BackgroundLoader communicates back when the loading is completed and
 * whether any errors occured.
 *
 * Typical mode of operation is to have a Demand inline in your object,
 * addDiskReource many times to build up the list of needed resources, call
 * requestLoad to start the background loading, waiting for isInBackgroundQueue
 * to return false.
 *
 * Note that when resources are loaded in this fashion, the loading and the
 * increment/decrement aspects of the DiskResource are handled automatically.
 */
class Demand : public fast_erase_index {

    public:

    Demand (void)
        : mInBackgroundQueue(false), mDist(0.0f), incremented(false), causedError(false)
    { }

    /** Add a required disk resource (by absolute path to the file). */
    void addDiskResource (const std::string &rn)
    {
        DiskResource *dr = disk_resource_get_or_make(rn);
        if (dr == NULL) return;
        resources.push_back(dr);
    }

    /** Clear out the list, useful for reusing a Demand object. */
    void clearDiskResources (void)
    {
        resources.clear();
    }

    /** Called by the main thread to get resources loaded prior to activation
     * of objects.  Repeated calls update the distance of the object, used for
     * prioritising the queue of demands.  Returns !isInBackgroundQueue() for
     * convenience.
     */
    bool requestLoad (float dist);

    /** Is the demand registered to be procesed by the background thread?  If
     * true, the background thread will (eventually,) process this demand.  If
     * false, either 1) the demand has never been registered, 2) it was registered
     * and the loading has already completed, or 3) it was registered and all the
     * resources were actually loaded and there was no background loading to be
     * done.  Either way, a return value of false following a requestLoad call
     * means that the resources are loaded.
     */
    bool isInBackgroundQueue (void) { return mInBackgroundQueue; }

    /* Cancel a requestLoad call, or indicate the resources are no-longer
     * required.  They may be unloaded if necessary according to memory pressure.
     * May be called when isInBackgroundQueue()==true in which case the demand is
     * removed from the background queue and any partially completed loading is
     * correctly undone.
     */
    void finishedWith (void);

    /** Load resources in the foreground thread, block until complete.  Called
     * usually as a result of debug commands from the console.  Obviously, this
     * will cause a frame stall (a drop in fps) so is not advised for normal use.
     */
    void immediateLoad (void);

    /** Reload resources in the foreground thread, block until complete called
     * usually as a result of debug commands from the console Obviously, this will
     * cause a frame rate stall.
     */
    void immediateReload (void);

    /** Are the resources in this Demand loaded yet?  Simply tests each one's loaded status. */
    bool loaded (void);

    /** Did the background loading throw an exception at all?  If so, not all
     * the disk resources are available.  It is suggested to call finishedWith()
     * and proceed no further.  The error (probably a file not found or other I/O
     * error) will already have been reported to the user in the console.
     */
    bool errorOnLoad (void) { return causedError; }

    private:

    /** Did we add to the background queue yet? */
    bool mInBackgroundQueue;

    /** The vector of resources that are required. */
    DiskResources resources;

    /** Distance from the player to the user of these resources. */
    volatile float mDist;

    /** Have we called increment on the resources yet? */
    bool incremented;

    /** Did an error occur in the background thread? */
    bool causedError;

    friend class BackgroundLoader;
};


/** Singleton class that managest the background loading of DiskResources using
 * a system thread to block on the I/O involved.  The intent is to avoid stalls
 * in the frame loop due to loading from disk.  Resources are loaded ahead of
 * when they are needed, and when multiple resources are needed, the closest to
 * the player is loaded first. */
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

// vim: shiftwidth=4:tabstop=4:expandtab
