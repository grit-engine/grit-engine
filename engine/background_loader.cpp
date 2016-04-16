/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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


#include <sleep.h>

#include "gfx/gfx_disk_resource.h"

#include "background_loader.h"
#include "main.h"

#define SYNCHRONISED std::unique_lock<std::recursive_mutex> _scoped_lock(lock)
#define SYNCHRONISED2(bgl) std::unique_lock<std::recursive_mutex> _scoped_lock(bgl->lock)

bool Demand::requestLoad (float dist)
{
        // called by main thread only
        mDist = dist;
        {
                SYNCHRONISED2(bgl);
                if (mInBackgroundQueue) return false;
        }

        if (!incremented) {
                //CVERB << "Incrementing resources: " << resources << std::endl;
                for (unsigned i=0 ; i<resources.size() ; ++i) {
                        // increment all the resource counters so that we know they're in use
                        resources[i]->increment();
                }
                incremented = true;
        }

        // if all the resources are in a loaded state then return true
        if (!loaded()) {
                //CVERB << "Requesting background load: " << resources << std::endl;
                // else get the bgl to do the right thing and return false
                bgl->checkRAMHost();
                bgl->checkRAMGPU();
                bgl->add(this);
        }

        return true;
}

bool Demand::loaded (void)
{
        //CVERB << "Checking if resources are loaded: " << resources << std::endl;
        for (unsigned i=0 ; i<resources.size() ; ++i) {
                if (!resources[i]->isLoaded()) {
                        return false;
                }
        }
        return true;
}

void Demand::immediateLoad (void)
{
        //CVERB << "Immediate load" << std::endl;
        SYNCHRONISED2(bgl);
        if (!incremented) {
                for (unsigned i=0 ; i<resources.size() ; ++i) {
                        resources[i]->increment();
                }
                incremented = true;
        }
        for (unsigned i=0 ; i<resources.size() ; ++i) {
                if (!resources[i]->isLoaded()) resources[i]->load();
        }
}

void Demand::immediateReload (void)
{
        //CVERB << "Immediate load" << std::endl;
        SYNCHRONISED2(bgl);
        for (unsigned i=0 ; i<resources.size() ; ++i) {
                if (resources[i]->isLoaded()) resources[i]->reload();
        }
}

// called by main thread only
void Demand::finishedWith (void)
{
        //CVERB << "No longer need: " << resources << std::endl;

        // we could be called because a grit object is being destroyed (unusual)
        // in which case, the Demand may get destructed pretty soon

        // or we could also be called if the grit object is simply deactivating (hot path)

        // certainly do not bother continuing to load if a load is in progress
        bgl->remove(this);

        // consider putting the resources up for reclamation
        if (incremented) {
                for (unsigned i=0 ; i<resources.size() ; ++i) {
                        resources[i]->decrement();
                }
                incremented = false;
        }
}


BackgroundLoader::BackgroundLoader (void)
      : mNumBastards(0), mThread(NULL), mCurrent(NULL),
        mQuit(false), mAllowance(0)
{
        SYNCHRONISED;

        mThread = new std::thread((void (*)(BackgroundLoader*))thread_main, const_cast<BackgroundLoader*>(this));
}

BackgroundLoader::~BackgroundLoader (void)
{
        shutdown();
}

void BackgroundLoader::shutdown (void)
{
        if (mQuit) return;
        {
                SYNCHRONISED;
                mQuit = true;
                cVar.notify_one();
        }
        mThread->join();
        handleBastards();
        mDeathRowGPU.clear();
        mDeathRowHost.clear();
        mDemands.clear();
        delete mThread;
}

// called by main thread only
void BackgroundLoader::add (Demand *d)
{
        SYNCHRONISED;
        mDemands.push_back(d);
        d->mInBackgroundQueue = true;
        d->causedError = false;
        cVar.notify_one();
}

// called by main thread only
void BackgroundLoader::remove (Demand *d)
{
        SYNCHRONISED;
        if (!d->mInBackgroundQueue) return;
        mDemands.erase(d);
        //CVERB << "Retracted demand." << std::endl;
        if (mCurrent == d) {
                //CVERB << "making a bastard..." << std::endl;
                mCurrent = NULL;
        }
        d->mInBackgroundQueue = false;
}               

void BackgroundLoader::handleBastards (void)
{
        // access volatile field without taking lock first
        // worst case we return early, i.e. will pick up bastards next time
        if (mNumBastards==0) return;
        DiskResources s;
        {
                SYNCHRONISED;
                if (mNumBastards==0) return;
                s = mBastards;
                mBastards.clear();
                mNumBastards = 0;
        }

        for (unsigned i=0 ; i<s.size() ; ++i) {
                finishedWith(s[i]);
        }
}


void BackgroundLoader::thread_main (BackgroundLoader *self)
{
        self->thread_main();
}

void BackgroundLoader::thread_main (void)
{
        //APP_VERBOSE("BackgroundLoader: thread started");
        DiskResources pending;
        bool caused_error = false;
        while (!mQuit) {
                {
                        SYNCHRONISED;
                        if (mCurrent) {
                                // Usual case:
                                // demand was not retracted while we were
                                // processing it
                                mCurrent->mInBackgroundQueue = false;
                                mCurrent->causedError = caused_error;
                                // cache in d to suppress compiler error
                                Demand *d = mCurrent;
                                mDemands.erase(d);
                                mCurrent = NULL;
                        } else {
                                // demand was retracted, and we actually
                                // loaded stuff
                                // (this can also get called before the first wait,
                                // in which case pending is empty)
                                typedef DiskResources::const_iterator DRCI;
                                for (DRCI i=pending.begin(), i_=pending.end() ; i!=i_ ; ++i) {
                                        //CVERB << "Poor bastard: " << (*i)->getName() << " (" << (*i)->getUsers() << ")" << std::endl;
                                        mBastards.push_back(*i);
                                        mNumBastards = mBastards.size();
                                }
                               //asynchronously call sm.finishedWith(resource);
                        }
                        pending.clear();
                        if (mAllowance <= 0 || !nearestDemand(mCurrent)) {
                                cVar.wait(_scoped_lock);
                                continue;        
                        }
                        pending = mCurrent->resources;
                }
                //APP_VERBOSE("BackgroundLoader: loading: "+name);
                caused_error = false;
                for (DiskResources::iterator i=pending.begin(), i_=pending.end() ; i!=i_ ; ++i) {
                        DiskResource *rp = *i;
                        try {
                                if (!rp->isLoaded()) {
                                        rp->load();
                                        mAllowance--;
                                        //CVERB << "Loaded a resource: " << *rp << std::endl;
                                }
                        } catch (Exception &e) {
                                CERR << e << std::endl;
                                caused_error = true;
                        }
                }

                //mysleep(100000);
        }
        //APP_VERBOSE("BackgroundLoader: thread terminated");
}


bool BackgroundLoader::nearestDemand (Demand * volatile &return_demand)
{
        if (mDemands.size() == 0) return false;

        float closest_dist = mDemands[0]->mDist;
        Demand *rd = mDemands[0];

        for (unsigned i=1 ; i<mDemands.size() ; ++i) {

                Demand *d = mDemands[i];
                
                float this_dist = d->mDist;
                
                if (this_dist<closest_dist) {
                        closest_dist = this_dist;
                        rd = d;
                }       

        }       

        return_demand = rd;
        return true;
}

void BackgroundLoader::setAllowance (float m)
{
        SYNCHRONISED;
        mAllowance = std::max(mAllowance+m, m);
        cVar.notify_one();
}


// unloading of resources /////////////////////////////////////////////////////////

void BackgroundLoader::finishedWith (DiskResource *de)
{
        if (mQuit) return;
        if (de->noUsers()) {
                if (de->isGPUResource()) {
                       mDeathRowGPU.push(de); 
                } else {
                        mDeathRowHost.push(de); 
                }
        }
}

void BackgroundLoader::checkRAMGPU ()
{

        double budget = gfx_gpu_ram_available();

        while (true) {

                double usage = gfx_gpu_ram_used();

                if (usage < budget || mDeathRowGPU.size() == 0) break;

                DiskResource *r = mDeathRowGPU.pop();

                if (r->noUsers() && r->isLoaded()) r->unload();
        }

}

void BackgroundLoader::checkRAMHost ()
{

        double budget = host_ram_available();

        while (true) {

                double usage = host_ram_used();

                if (usage < budget || mDeathRowHost.size() == 0) break;

                DiskResource *r = mDeathRowHost.pop();

                if (r->noUsers() && r->isLoaded())
                        r->unload();
        }

}


// vim: shiftwidth=8:tabstop=8:expandtab
