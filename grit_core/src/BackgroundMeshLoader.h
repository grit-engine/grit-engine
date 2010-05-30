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

class BackgroundMeshLoader;
struct Demand;

#ifndef BackgroundMeshLoader_h
#define BackgroundMeshLoader_h

#include <set>
#include <list>
#include <algorithm>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

#include <OgrePrerequisites.h>
#include <OgreSingleton.h>
#include <OgreAtomicWrappers.h>


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


typedef std::vector<Ogre::ResourcePtr> ResourcePtrs;
typedef std::set<Ogre::ResourcePtr> ResourcePtrSet;

struct Demand {
        Demand() : mProcessed(false) { }
        ResourcePtrs rPtrs;
        volatile float mDist;
        volatile bool mProcessed;
};


class BackgroundMeshLoader : public Ogre::Singleton<BackgroundMeshLoader> {

    public:

        typedef std::set<Demand*> Demands;


        BackgroundMeshLoader (void);

        ~BackgroundMeshLoader ();

        void shutdown (void);

        void add (Demand *d);

        void remove (Demand *d);

        void handleBastards (void);

        size_t size () { return mDemands.size(); }

        void setAllowance (float m);


        // background thread entry point
        void operator() (void);

    protected:

        bool nearestDemand (Demand * volatile &return_demand);

        OGRE_AUTO_MUTEX
        OGRE_THREAD_SYNCHRONISER(mCVar)

        ResourcePtrSet mBastards;
        volatile unsigned short mNumBastards;

        Demands mDemands;

        boost::thread *mThread;

        Demand * volatile mCurrent;
        volatile bool mQuit;

        float mAllowance;


    public:

        typedef std::set<Ogre::MaterialPtr> MaterialSet;

        void refreshResource (const Ogre::ResourcePtr &v)
        {
                mDeathRow.removeIfPresent(v);
        }
                

        size_t getLRUQueueSize (void) const
        { return mDeathRow.size(); }

        void finishedWith (const ResourcePtrSet &);
        void finishedWith (const Ogre::MaterialPtr &);
        void finishedWith (const Ogre::ResourcePtr &);

        bool isGPUOversubscribed () const;
        void checkGPUUsage ();

    protected:

        LRUQueue<Ogre::ResourcePtr> mDeathRow;

};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
