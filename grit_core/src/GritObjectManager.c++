#include <OgreSceneManager.h>
#include <OgreException.h>

#include "GritObjectManager.h"
#include "Grit.h"

GritObjectManager::~GritObjectManager (void)
{
        if (!shutdown)
                APP_ERROR("destructing GritObjManager","not shutdown cleanly");
}

void GritObjectManager::doShutdown (lua_State *L)
{
        clearClasses(L);
        clearObjects(L);
        shutdown = true;
}

void GritObjectManager::clearClasses (lua_State *L)
{
        GritClassMap m = classes;
        for (GritClassMap::iterator i=m.begin(), i_=m.end() ; i!=i_ ; ++i) {
                deleteClass(L,i->second);
        }
}

void GritObjectManager::clearObjects (lua_State *L)
{
        GObjMap m = gObjs;
        for (GObjMap::iterator i=m.begin(), i_=m.end() ; i!=i_ ; ++i) {
                deleteObject(L,i->second);
        }
}


void GritObjectManager::setGFX (lua_State *L,
                                Ogre::SceneNode *gfx)
{
        GObjMap gObjs = this->gObjs;
        for (GObjMap::iterator i=gObjs.begin(), i_=gObjs.end() ; i!=i_ ; ++i) {
                i->second->deactivate(L, i->second);
        }
        this->gfx = gfx;
}

void GritObjectManager::setBounds (lua_State *L,
                                   const Ogre::AxisAlignedBox &bounds_)
{
        bounds = bounds_;
        setPhysics(L, PhysicsWorldPtr(new PhysicsWorld(bounds)));
}

void GritObjectManager::setPhysics (lua_State *L,
                                    const PhysicsWorldPtr &physics)
{
        GObjMap gObjs = this->gObjs;
        // don't need to clearPhysics() because it's a smart pointer
        this->physics = physics;
        for (GObjMap::iterator i=gObjs.begin(), i_=gObjs.end() ; i!=i_ ; ++i) {
                i->second->deactivate(L,i->second);
        }
}



GritClass *GritObjectManager::addClass (lua_State *L, const Ogre::String& name)
{
        if (classes.find(name)!=classes.end())
                OGRE_EXCEPT(Ogre::Exception::ERR_DUPLICATE_ITEM,
                            "GritClass already exists: "+name,
                            "GritObjectManager::addClass");
        // add it and return it
        GritClass *gcp = new GritClass(L,name);
        classes[name] = gcp;
        return gcp;
}

GritClass *GritObjectManager::getClass (const Ogre::String &name)
{
        GritClassMap::iterator i = classes.find(name);
        if (i==classes.end())
                OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND,
                            "GritClass does not exist: "+name,
                            "GritObjectManager::getClass");
        return i->second;
}


void GritObjectManager::eraseClass (const Ogre::String &name)
{
        // anything using this class keeps using it
        classes.erase(name);
        // class gets deleted properly when everything stops using it
}





GritObjectPtr GritObjectManager::addObject (
        lua_State *L,
        Ogre::String name,
        GritClass *grit_class)
{
        (void) L; // may need this again some day
        if (gfx==NULL) {
                OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR,
                            "No graphics engine set up, call setGFX()",
                            "GritObjectManager::addObject");
        }
        if (physics.isNull()) {
                OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR,
                            "No physics engine set up, call setPhysics()",
                            "GritObjectManager::addObject");
        }
        if (name=="") {
                do {
                        std::stringstream ss;
                        ss << "Unnamed:" << nameGenerationCounter++;
                        name = ss.str();
                } while (gObjs.find(name)!=gObjs.end());
        }
                        
        if (gObjs.find(name)!=gObjs.end())
                OGRE_EXCEPT(Ogre::Exception::ERR_DUPLICATE_ITEM,
                            "GritObject already exists: "+name,
                            "GritObjectManager::addObject");

        GritObjectPtr self = GritObjectPtr(new GritObject(name,grit_class));
        gObjs[name] = self;
        rs.add(self);

        return self;
}



const GritObjectPtr &GritObjectManager::getObject (const Ogre::String &name)
{
        GObjMap::iterator i = gObjs.find(name);
        if (i==gObjs.end())
                OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND,
                            "GritObject does not exist: "+name,
                            "GritObjectManager::getObject");

        return i->second;
}



// don't call this function directly since the object won't be deactivated
// call deleteObject instead
void GritObjectManager::eraseObject (const Ogre::String &name)
{
        GObjMap::iterator i = gObjs.find(name);
        if (i==gObjs.end()) 
                OGRE_EXCEPT(Ogre::Exception::ERR_DUPLICATE_ITEM,
                            "GritObject does not exist: "+name,
                            "GritObjectManager::eraseObject");

        gObjs.erase(name);
}

void GritObjectManager::deleteObject (lua_State *L, const GritObjectPtr &o)
{
        o->destroy(L,o);
        rs.remove(o);
        eraseObject(o->name);
}

void GritObjectManager::centre (lua_State *L,
                                Ogre::Real x, Ogre::Real y, Ogre::Real z,
                                Ogre::Real vF)
{
        Space::Cargo fnd;

        const Ogre::Real pF = prepareDistanceFactor;
        const Ogre::Real tpF = pF * vF; // prepare and visibility factors
        const Ogre::Real vF2 = vF * vF;

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
                 //note we use vF2 not vF
                Ogre::Real range2 = o->range2(x,y,z) / vF2;
                o->notifyRange2(L,o,range2);
                if (!o->getFar().isNull()) {
                        // update the far (perhaps for a second time this frame)
                        // to make sure it has picked up the fade imposed by o
                        const GritObjectPtr &f = o->getFar();
                        Ogre::Real range2 = f->range2(x,y,z) / vF2;
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
        rs.getPresent(x,y,z,stepSize,tpF,fnd);
        for (Space::Cargo::iterator i=fnd.begin(),i_=fnd.end() ; i!=i_ ; ++i) {
                const GritObjectPtr &o = *i;

                if (o->isActivated()) continue;

                // consider background loading
                if (o->queueBGPrepare(x,y,z)) {
                        // note 'loaded' includes things which have started
                        // but not finished loading...
                        loaded.push_back(o);
                }

                if (!o->backgroundPrepareComplete()) continue;

                Ogre::Real range2 = o->range2(x,y,z) / vF2;
                // not in range yet
                if (range2 > 1) continue;

                // if we get this far, we should be displayed but there might be
                // a near object in the way
                GritObjectPtr near = o->getNear();
                while (!near.isNull()) {
                        if (near->withinRange(x,y,z,vF * fadeOverlapFactor)) {
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

                o->notifyRange2(L,o,range2);

                skip:;
        }

        BackgroundMeshLoader::getSingleton().handleBastards();
        BackgroundMeshLoader::getSingleton().checkGPUUsage();
}

// vim: shiftwidth=8:tabstop=8:expandtab
