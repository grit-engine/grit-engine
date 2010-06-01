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

#include <OgreSceneManager.h>
#include <OgreException.h>

#include "Streamer.h"
#include "Grit.h"

Streamer::~Streamer (void)
{
        if (!shutdown)
                CERR<<"Streamer: not shutdown cleanly"<<std::endl;
}

void Streamer::doShutdown (lua_State *L)
{
        // if lua has not been initialised yet then nothing to do
        if (L) {
                clearClasses(L);
                clearObjects(L);
        }
        shutdown = true;
}

void Streamer::clearClasses (lua_State *L)
{
        GritClassMap m = classes;
        for (GritClassMap::iterator i=m.begin(), i_=m.end() ; i!=i_ ; ++i) {
                deleteClass(L,i->second);
        }
}

void Streamer::clearObjects (lua_State *L)
{
        GObjMap m = gObjs;
        for (GObjMap::iterator i=m.begin(), i_=m.end() ; i!=i_ ; ++i) {
                deleteObject(L,i->second);
        }
}

void Streamer::clearAnonymousObjects (lua_State *L)
{
        GObjMap m = gObjs;
        for (GObjMap::iterator i=m.begin(), i_=m.end() ; i!=i_ ; ++i) {
                if (i->second->anonymous) deleteObject(L,i->second);
        }
}


void Streamer::setGFX (lua_State *L, Ogre::SceneNode *gfx)
{
        GObjMap gObjs = this->gObjs;
        for (GObjMap::iterator i=gObjs.begin(), i_=gObjs.end() ; i!=i_ ; ++i) {
                i->second->deactivate(L, i->second);
        }
        this->gfx = gfx;
}

void Streamer::setBounds (lua_State *L, const Ogre::AxisAlignedBox &bounds_)
{
        bounds = bounds_;
        setPhysics(L, PhysicsWorldPtr(new PhysicsWorld(bounds)));
}

void Streamer::setPhysics (lua_State *L, const PhysicsWorldPtr &physics)
{
        GObjMap gObjs = this->gObjs;
        // don't need to clearPhysics() because it's a smart pointer
        this->physics = physics;
        for (GObjMap::iterator i=gObjs.begin(), i_=gObjs.end() ; i!=i_ ; ++i) {
                i->second->deactivate(L,i->second);
        }
}



GritClass *Streamer::addClass (lua_State *L, const std::string& name)
{
        GritClass *gcp;
        GritClassMap::iterator i = classes.find(name);
        
        if (i!=classes.end()) {
                gcp = i->second;
                int index = lua_gettop(L);
                for (lua_pushnil(L) ; lua_next(L,index)!=0 ; lua_pop(L,1)) {
                        gcp->set(L);
                }
                lua_pop(L,1); // the table we just iterated through
                gcp->setParent(L);
        } else {
                // add it and return it
                gcp = new GritClass(L,name);
                classes[name] = gcp;
        }
        return gcp;
}

GritClass *Streamer::getClass (const std::string &name)
{
        GritClassMap::iterator i = classes.find(name);
        if (i==classes.end())
                GRIT_EXCEPT("GritClass does not exist: "+name);
        return i->second;
}


void Streamer::eraseClass (const std::string &name)
{
        // anything using this class keeps using it
        classes.erase(name);
        // class gets deleted properly when everything stops using it
}





GritObjectPtr Streamer::addObject (
        lua_State *L,
        std::string name,
        GritClass *grit_class)
{
        (void) L; // may need this again some day
        if (gfx==NULL) {
                GRIT_EXCEPT("No graphics engine set up, call setGFX()");
        }
        if (physics.isNull()) {
                GRIT_EXCEPT("No physics engine set up, call setPhysics()");
        }
        bool anonymous = false;
        if (name=="") {
                anonymous = true;
                do {
                        std::stringstream ss;
                        ss << "Unnamed:" << grit_class->name
                           << ":" << nameGenerationCounter++;
                        name = ss.str();
                } while (gObjs.find(name)!=gObjs.end());
        }
                        
        GObjMap::iterator i = gObjs.find(name);

        if (i!=gObjs.end()) {
                deleteObject(L,i->second);
        }

        GritObjectPtr self = GritObjectPtr(new GritObject(name,grit_class));
        self->anonymous = anonymous;
        gObjs[name] = self;
        rs.add(self);
        fresh.push_back(self);

        return self;
}



const GritObjectPtr &Streamer::getObject (const std::string &name)
{
        GObjMap::iterator i = gObjs.find(name);
        if (i==gObjs.end())
                GRIT_EXCEPT("GritObject does not exist: "+name);

        return i->second;
}


static void remove_if_exists (GObjPtrs &list, const GritObjectPtr &o)
{
        GObjPtrs::iterator iter = find(list.begin(),list.end(),o);
        if (iter!=list.end()) {
                size_t offset = iter - list.begin();
                list[offset] = list[list.size()-1];
                list.pop_back();
        }
}

void Streamer::deleteObject (lua_State *L, const GritObjectPtr &o)
{
        o->destroy(L,o);
        rs.remove(o);
        remove_if_exists(fresh, o);
        remove_if_exists(needFrameCallbacks, o);

        GObjMap::iterator i = gObjs.find(o->name);
        // Since object deactivation can trigger other objects to be destroyed,
        // sometimes when quitting, due to the order in which the objects are destroyed,
        // we destroy an object that is already dead...
        if (i!=gObjs.end()) gObjs.erase(o->name);
}

void Streamer::frameCallbacks (lua_State *L, float elapsed)
{
        GObjPtrs victims = needFrameCallbacks;
        typedef GObjPtrs::iterator I;
        for (I i=victims.begin(), i_=victims.end() ; i!=i_ ; ++i) {
                if (!(*i)->getNeedsFrameCallbacks()) continue;
                if (!(*i)->frameCallback(L, *i, elapsed)) {
                        (*i)->setNeedsFrameCallbacks(*i, false);
                }
        }
}

void Streamer::setNeedsFrameCallbacks (const GritObjectPtr & ptr, bool v)
{
        remove_if_exists(needFrameCallbacks, ptr);
        if (v) {
                if (ptr->isActivated())
                        needFrameCallbacks.push_back(ptr);
        }
}

void Streamer::centre (lua_State *L, float x, float y, float z)
{
        Space::Cargo fnd = fresh;
        fresh.clear();

        const float pF = prepareDistanceFactor;
        const float tpF = pF * visibility; // prepare and visibility factors
        const float vis2 = visibility * visibility;

        typedef GObjPtrs::iterator I;

        // iterate through, deactivating things


        ////////////////////////////////////////////////////////////////////////
        // DEACTIVATE DISTANT GRIT OBJECTS /////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        // use victims because deactivate() changes the 'activated' list
        // and so does notifyRange2 if the callback raises an error
        GObjPtrs victims = activated;
        for (I i=victims.begin(), i_=victims.end() ; i!=i_ ; ++i) {
                const GritObjectPtr &o = *i;
                 //note we use vis2 not visibility
                float range2 = o->range2(x,y,z) / vis2;
                o->notifyRange2(L,o,range2);
                if (!o->getFar().isNull()) {
                        // update the far (perhaps for a second time this frame)
                        // to make sure it has picked up the fade imposed by o
                        const GritObjectPtr &f = o->getFar();
                        float range2 = f->range2(x,y,z) / vis2;
                        f->notifyRange2(L,f,range2);
                }
                if (range2 > 1) {
                        // now out of range
                        const GritObjectPtr &o = *i;
                        const GritObjectPtr &far = o->getFar();
                        bool killme = o->deactivate(L,o);
                        if (!far.isNull()) {
                                // we're deactivating and we have a far,
                                // so make sure it gets considered this frame
                                fnd.push_back(far);
                        }
                        if (killme) {
                                deleteObject(L,o);
                        }
                }
        }

        ////////////////////////////////////////////////////////////////////////
        // UNLOAD RESOURCES FOR VERY DISTANT GRIT OBJECTS //////////////////////
        ////////////////////////////////////////////////////////////////////////
        for (size_t i=0, i_=loaded.size(); i<i_ ;) {
                // Iteration should be fast for removal of significant number of
                // elements.  Don't do this if it ever stops being a vector.
                const GritObjectPtr &o = loaded[i];
                if (!o->withinRange(x,y,z,tpF)) {
                        // unregister demand...
                        // we deactivated first so this should
                        // unload any resources we were using
                        o->tryUnloadResources();
                        loaded[i] = loaded[i_-1];
                        loaded.pop_back();
                        --i_;
                } else {
                        ++i;
                }
        }


        ////////////////////////////////////////////////////////////////////////
        // LOAD RESOURCES FOR APPROACHING GRIT OBJECTS /////////////////////////
        // AND... ACTIVATE ARRIVING GRIT OBJECTS ///////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        // note: since fnd is prepopulated by new objects and the lods of deactivated objects
        // it may have duplicates after the rangespace has gone through
        rs.getPresent(x,y,z,stepSize,tpF,fnd);
        for (Space::Cargo::iterator i=fnd.begin(),i_=fnd.end() ; i!=i_ ; ++i) {
                const GritObjectPtr &o = *i;

                // this can happen if there is a duplicate in the list and it gets destroyed
                // the first time due to an error in the activation function
                // but is eventually processed a second time after being destroyed
                if (o->getClass()==NULL) continue;
                if (o->isActivated()) continue;

                // consider background loading
                if (o->queueBGPrepare(x,y,z)) {
                        // note 'loaded' includes things which have started
                        // but not finished loading...
                        loaded.push_back(o);
                }

                if (!o->backgroundPrepareComplete()) continue;

                float range2 = o->range2(x,y,z) / vis2;
                // not in range yet
                if (range2 > 1) continue;

                // if we get this far, we should be displayed but there might be
                // a near object in the way
                GritObjectPtr near = o->getNear();
                while (!near.isNull()) {
                        if (near->withinRange(x,y,z,visibility * fadeOverlapFactor)) {
                                if (near->isActivated()) {
                                        o->deactivate(L,o);
                                        // don't activate, near gobj is
                                        // in the way
                                        goto skip;
                                }
                        }
                        near = near->getNear();
                }
      

                // ok there wasn't so activate
                o->activate(L,o,gfx,physics);

                // activation can result in a lua error which triggers the destruction of the
                // object 'o' so we test for that here before doing more stuff
                if (o->getClass()==NULL) continue;

                o->notifyRange2(L,o,range2);

                skip:;
        }

        BackgroundMeshLoader::getSingleton().handleBastards();
        BackgroundMeshLoader::getSingleton().checkGPUUsage();
}

void Streamer::list (const GritObjectPtr &o)
{
        GObjPtrs::iterator begin = activated.begin(), end = activated.end();
        GObjPtrs::iterator iter  = find(begin,end,o);
        if (iter!=end) return;
        activated.push_back(o);
        if (o->getNeedsFrameCallbacks())
                needFrameCallbacks.push_back(o);
}

void Streamer::unlist (const GritObjectPtr &o)
{
        GObjPtrs::iterator begin = activated.begin(), end = activated.end();
        GObjPtrs::iterator iter  = find(begin,end,o);
        if (iter==end) return;
        size_t index = iter - begin;
        activated[index] = activated[activated.size()-1];
        activated.pop_back();
        if (o->getNeedsFrameCallbacks())
                remove_if_exists(needFrameCallbacks, o);
}

// vim: shiftwidth=8:tabstop=8:expandtab
