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

#include <map>
#include <vector>
#include <set>

#include <OgreString.h>
#include <OgreSharedPtr.h>

class GritObject;
typedef Ogre::SharedPtr<GritObject> GritObjectPtr;
typedef std::map<Ogre::String,GritObjectPtr> GObjMap;
typedef std::vector<GritObjectPtr> GObjPtrs;
typedef std::set<GritObjectPtr> GObjSet;


#ifndef GritObject_h
#define GritObject_h

#include <OgreException.h>
#include <OgreSceneNode.h>

#include "PhysicsWorld.h"
#include "Streamer.h"
#include "BackgroundMeshLoader.h"
#include "ExternalTable.h"

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

class GritObject {

    public:

        typedef std::pair<Ogre::String,Ogre::String> StringPair;
        typedef std::vector<StringPair> StringPairs;

        GritObject (const Ogre::String &name_, GritClass *gritClass_);

        virtual ~GritObject (void) { }

        void destroy (lua_State *L, const GritObjectPtr &self);

        bool deactivate (lua_State *L, const GritObjectPtr &self);

        void init (lua_State *L, const GritObjectPtr &self);

        void activate (lua_State *L,
                       const GritObjectPtr &self,
                       Ogre::SceneNode *gfxNode,
                       const PhysicsWorldPtr &physics);

        Ogre::Real calcFade (const Ogre::Real range2, bool &overlap);

        void notifyRange2 (lua_State *L, const GritObjectPtr &self,
                           const Ogre::Real range2)
        {
                bool overlap = false;
                volatile Ogre::Real fade = calcFade(range2, overlap);
                // if fade is sufficiently different
                if (fade!=lastFade) {
                        int transition;
                        if (((lastFade<1) == (fade<1)) && (lastFade != -1))
                                transition = -1;
                        else if (fade>=1)
                                transition = 0;
                        else if (overlap)
                                transition = 1;
                        else
                                transition = 2;
                        notifyFade(L, self, fade, transition);
                        lastFade = fade;
                }
        }

        bool frameCallback (lua_State *L,
                            const GritObjectPtr &self,
                            const Ogre::Real time);

        void notifyFade (lua_State *L,
                         const GritObjectPtr &self,
                         const Ogre::Real fade,
                         const int transition);

        bool isActivated (void) const { return lua!=LUA_NOREF; }

        void pushLuaTable (lua_State *L)
        {
                 lua_rawgeti(L,LUA_REGISTRYINDEX,lua);
        }

        void hintPrepareInAdvance (const Ogre::String &type,
                                           const Ogre::String &name)
        { advanceResources.push_back(StringPair(type,name)); }

        void clearAdvanceHints (void)
        { advanceResources.clear(); }

        const StringPairs &getAdvanceHints (void) const
        { return advanceResources; }

        bool queueBGPrepare (Ogre::Real x, Ogre::Real y, Ogre::Real z)
        {
                if (!demandRegistered) {
                        doQueueBGPrepare(x,y,z);
                        return true;
                } else {
                        updateDemand(x,y,z);
                        return false;
                }
        }

        void updateDemand(Ogre::Real x_, Ogre::Real y_, Ogre::Real z_) {
                const Ogre::Real dx=x-x_, dy=y-y_, dz=z-z_;
                demand.mDist = dx*dx + dy*dy + dz*dz;
        }

        void doQueueBGPrepare (Ogre::Real, Ogre::Real, Ogre::Real);

        void tryUnloadResources (void);

        bool backgroundPrepareComplete (void)
        {
                return demand.mProcessed;
        }

        GritClass *getClass (void) { return gritClass; }


        Ogre::Real getFade (void) const { return lastFade; }

        bool getNeedsFrameCallbacks (void) const { return needsFrameCallbacks; }
        void setNeedsFrameCallbacks (const GritObjectPtr &self, bool v);


        const Ogre::String name;

        inline void updateIndex (int index_)
        {
                index = index_;
        }

        void updateSphere (Ogre::Real x_, Ogre::Real y_, Ogre::Real z_,
                                   Ogre::Real r_);

        Ogre::Real getX() const { return x; }
        Ogre::Real getY() const { return y; }
        Ogre::Real getZ() const { return z; }
        Ogre::Real getR() const { return r; }
                
        const GritObjectPtr &getNear (void) const { return near; }
        void setNear (const GritObjectPtr &self, const GritObjectPtr &v)
        {
                // this function is recursive and mutually recursive
                // but always terminates due to the early return below
                if (near==v) return;
                if (near.isNull()) {
                        // note that v cannot be null since near!=v
                        // adding a near
                        near = v;
                        near->setFar(near,self);
                } else {
                        if (v.isNull()) {
                                // removing near
                                GritObjectPtr tmp = near;
                                near.setNull();
                                tmp->setFar(tmp,near);
                        } else {
                                // changing near
                                // first set to null
                                GritObjectPtr tmp = near;
                                near.setNull();
                                tmp->setFar(tmp,near);
                                // then set to new value
                                setNear(self,v);
                        }
                }
        }

        const GritObjectPtr &getFar (void) const { return far; }
        void setFar (const GritObjectPtr &self, const GritObjectPtr &v)
        {
                // this function is recursive and mutually recursive
                // but always terminates due to the early return below
                if (far==v) return;
                if (far.isNull()) {
                        // adding a far
                        far = v;
                        far->setNear(far,self);
                } else {
                        if (v.isNull()) {
                                // removing far
                                GritObjectPtr tmp = far;
                                far.setNull();
                                tmp->setNear(tmp,far);
                        } else {
                                // changing far
                                // first set to null
                                GritObjectPtr tmp = far;
                                far.setNull();
                                tmp->setNear(tmp,far);
                                // then set to new value
                                setFar(self,v);
                        }
                }
        }

        Ogre::Real range2 (Ogre::Real x_, Ogre::Real y_, Ogre::Real z_) const
        {
                const Ogre::Real dx=x-x_, dy=y-y_, dz=z-z_;
                return (dx*dx + dy*dy + dz*dz) / (r*r);
        }

        bool withinRange (Ogre::Real x_, Ogre::Real y_,
                          Ogre::Real z_, Ogre::Real factor) const
        {
                const Ogre::Real dx=x-x_, dy=y-y_, dz=z-z_;
                const Ogre::Real rad = r*factor;
                return dx*dx + dy*dy + dz*dz < rad*rad;
        }

        Ogre::Real getImposedFarFade (void) const { return imposedFarFade; }

        ExternalTable userValues;


    protected:

        Ogre::Real x, y, z, r;
                
        GritClass *gritClass;

        int lua;

        int index;

        bool needsFrameCallbacks;

        GritObjectPtr near, far;
                
        StringPairs advanceResources;

        bool demandRegistered;
        Demand demand;


        Ogre::Real imposedFarFade;
        Ogre::Real lastFade;
};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
