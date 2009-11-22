class GritObjectManager;


#ifndef GritObjectManager_h
#define GritObjectManager_h

#include <map>

#include <OgreString.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}


#include "GritClass.h"
#include "GritObject.h"
#include "PhysicsWorld.h"
#include "CacheFriendlyRangeSpaceSIMD.h"

typedef std::map<Ogre::String,GritClass*> GritClassMap;

class GritObjectManager {

    public:

        GritObjectManager (void)
              : prepareDistanceFactor(1.3),
                fadeOutFactor(.7),
                fadeOverlapFactor(.7),
                visibility(1),
                stepSize(20000),
                gfx(NULL),
                bounds(-3000,-3000,-3000,3000,3000,3000),
                nameGenerationCounter(0), shutdown(false)
        {
        }

        virtual ~GritObjectManager (void);

        virtual void doShutdown (lua_State *L);

        virtual void clearClasses (lua_State *L);

        virtual void clearObjects (lua_State *L);


        // GLOBAL STUFF

        virtual void setGFX (lua_State *L, Ogre::SceneNode *gfx);
        virtual Ogre::SceneNode *getGFX (void) { return gfx; }

        virtual PhysicsWorldPtr getPhysics (void) { return physics; }

        virtual void setBounds (lua_State *L,
                                const Ogre::AxisAlignedBox &bounds_);

        virtual const Ogre::AxisAlignedBox &getBounds () { return bounds; }

        // CLASS STUFF

        virtual GritClass *addClass (lua_State *L, const Ogre::String& name);

        virtual GritClass *getClass (const Ogre::String &name);


        virtual bool hasClass (const Ogre::String &name)
        { return classes.find(name)!=classes.end(); }

        virtual void deleteClass (lua_State *L, GritClass *c)
        {
                eraseClass(c->name);
                c->release(L);
        }

        virtual void getClasses (GritClassMap::iterator &begin,
                                 GritClassMap::iterator &end)
        {
                begin = classes.begin();
                end = classes.end();
        }

        virtual void getClasses (GritClassMap::const_iterator &begin,
                                 GritClassMap::const_iterator &end) const
        {
                begin = classes.begin();
                end = classes.end();
        }

        virtual int numClasses (void) { return classes.size(); }


        // OBJECT STUFF

        virtual GritObjectPtr addObject (lua_State *L,
                                         Ogre::String name,
                                         GritClass *grit_class);

        virtual const GritObjectPtr &getObject (const Ogre::String &name);

        virtual void getObjects (GObjMap::iterator &begin,
                                 GObjMap::iterator &end)
        {
                begin = gObjs.begin();
                end = gObjs.end();
        }

        virtual void getObjects (GObjMap::const_iterator &begin,
                                 GObjMap::const_iterator &end) const
        {
                begin = gObjs.begin();
                end = gObjs.end();
        }

        virtual int numObjects (void) { return gObjs.size(); }

        virtual bool hasObject (const Ogre::String &name)
        { return gObjs.find(name)!=gObjs.end(); }

        virtual void deleteObject (lua_State *L, const GritObjectPtr &o);


        // ACTIVATION STUFF

        Ogre::Real prepareDistanceFactor;
        Ogre::Real fadeOutFactor;
        Ogre::Real fadeOverlapFactor;

        Ogre::Real visibility;

        virtual void centre (lua_State *L, Ogre::Real x, Ogre::Real y, Ogre::Real z);

        virtual inline void updateSphere (size_t index, Ogre::Real x,
                                          Ogre::Real y, Ogre::Real z,
                                          Ogre::Real d)
        {
                rs.updateSphere(index,x,y,z,d);
        }
        
        size_t stepSize;

        virtual void list (const GritObjectPtr &o)
        {
                GObjPtrs::iterator begin = activated.begin(), end = activated.end();
                GObjPtrs::iterator iter  = find(begin,end,o);
                if (iter!=end) return;
                activated.push_back(o);
        }

        virtual void unlist (const GritObjectPtr &o)
        {
                GObjPtrs::iterator begin = activated.begin(), end = activated.end();
                GObjPtrs::iterator iter  = find(begin,end,o);
                if (iter==end) return;
                size_t index = iter - begin;
                activated[index] = activated[activated.size()-1];
                activated.pop_back();
        }

        virtual int numActivated (void) { return activated.size(); }

        typedef CacheFriendlyRangeSpace<GritObjectPtr> Space;

    protected:

        virtual void eraseClass (const Ogre::String &name);

        virtual void eraseObject (const Ogre::String &);

        virtual void setPhysics (lua_State *L, const PhysicsWorldPtr &physics);

        GritClassMap classes;

        GObjMap gObjs;
        Space rs;
        GObjPtrs activated;
        GObjPtrs loaded;
        GObjPtrs fresh; // just been added - skip the queue for activation

        Ogre::SceneNode *gfx;

        PhysicsWorldPtr physics;

        Ogre::AxisAlignedBox bounds;

        size_t nameGenerationCounter;

        bool shutdown;


};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
