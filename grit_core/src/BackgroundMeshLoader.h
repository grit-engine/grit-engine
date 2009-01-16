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


#include "app_error.h"


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
        volatile Ogre::Real mDist;
        volatile bool mProcessed;
};


class BackgroundMeshLoader : public Ogre::Singleton<BackgroundMeshLoader> {

    public:

        typedef std::set<Demand*> Demands;


        BackgroundMeshLoader (void);

        virtual ~BackgroundMeshLoader ();

        virtual void shutdown (void);

        virtual void add (Demand *d);

        virtual void remove (Demand *d);

        virtual void handleBastards (void);

        virtual size_t size () { return mDemands.size(); }

        virtual void setAllowance (Ogre::Real m);


        // background thread entry point
        virtual void operator() (void);

    protected:

        virtual bool nearestDemand (Demand * volatile &return_demand);

        OGRE_AUTO_MUTEX
        OGRE_THREAD_SYNCHRONISER(mCVar)

        ResourcePtrSet mBastards;
        volatile unsigned short mNumBastards;

        Demands mDemands;

        boost::thread *mThread;

        Demand * volatile mCurrent;
        volatile bool mQuit;

        Ogre::Real mAllowance;


    public:

        typedef std::set<Ogre::MaterialPtr> MaterialSet;

        void refreshResource (const Ogre::ResourcePtr &v)
        {
                mDeathRow.removeIfPresent(v);
        }
                

        virtual size_t getLRUQueueSize (void) const
        { return mDeathRow.size(); }

        virtual void finishedWith (const ResourcePtrSet &);
        virtual void finishedWith (const Ogre::MaterialPtr &);
        virtual void finishedWith (const Ogre::ResourcePtr &);

        virtual bool isGPUOversubscribed () const;
        virtual void checkGPUUsage ();

    protected:

        LRUQueue<Ogre::ResourcePtr> mDeathRow;

};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
