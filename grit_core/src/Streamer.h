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

class Streamer;


#ifndef Streamer_h
#define Streamer_h

#include <map>

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}


#include "GritClass.h"
#include "GritObject.h"
#include "PhysicsWorld.h"
#include "CacheFriendlyRangeSpaceSIMD.h"

typedef std::map<std::string,GritClass*> GritClassMap;

class Streamer {

    public:

        Streamer (void);

        ~Streamer (void);

        void doShutdown (lua_State *L);

        void clearClasses (lua_State *L);

        void clearObjects (lua_State *L);
        void clearAnonymousObjects (lua_State *L);


        // GLOBAL STUFF

        PhysicsWorldPtr getPhysics (void) { return physics; }

        void setBounds (lua_State *L, const Vector3 &bounds_min, const Vector3 &bounds_max);

        // CLASS STUFF

        GritClass *addClass (lua_State *L, const std::string& name);

        GritClass *getClass (const std::string &name);


        bool hasClass (const std::string &name) { return classes.find(name)!=classes.end(); }

        void deleteClass (lua_State *L, GritClass *c)
        {
                eraseClass(c->name);
                c->release(L);
        }

        void getClasses (GritClassMap::iterator &begin,
                         GritClassMap::iterator &end)
        {
                begin = classes.begin();
                end = classes.end();
        }

        void getClasses (GritClassMap::const_iterator &begin,
                         GritClassMap::const_iterator &end) const
        {
                begin = classes.begin();
                end = classes.end();
        }

        int numClasses (void) { return classes.size(); }


        // OBJECT STUFF

        GritObjectPtr addObject (lua_State *L,
                                 std::string name,
                                 GritClass *grit_class);

        const GritObjectPtr &getObject (const std::string &name);

        void getObjects (GObjMap::iterator &begin,
                         GObjMap::iterator &end)
        {
                begin = gObjs.begin();
                end = gObjs.end();
        }

        void getObjects (GObjMap::const_iterator &begin,
                         GObjMap::const_iterator &end) const
        {
                begin = gObjs.begin();
                end = gObjs.end();
        }

        int numObjects (void) { return gObjs.size(); }

        bool hasObject (const std::string &name)
        { return gObjs.find(name)!=gObjs.end(); }

        void deleteObject (lua_State *L, const GritObjectPtr &o);


        // FRAME CALLBACK

        void frameCallbacks (lua_State *L, float elapsed);
        void setNeedsFrameCallbacks (const GritObjectPtr &ptr, bool v);

        
        // ACTIVATION STUFF

        float prepareDistanceFactor;
        float fadeOutFactor;
        float fadeOverlapFactor;

        float visibility;

        void centre (lua_State *L, const Vector3 &new_pos);

        inline void updateSphere (size_t index, const Vector3 &pos, float d)
        {
                rs.updateSphere(index,pos.x,pos.y,pos.z,d);
        }
        
        size_t stepSize;

        void list (const GritObjectPtr &o);

        void unlist (const GritObjectPtr &o);

        int numActivated (void) { return activated.size(); }

        typedef CacheFriendlyRangeSpace<GritObjectPtr> Space;

        // ACTIVATION UPDATE CALLBACK

        struct UpdateHook {
                virtual void update(const Vector3 &new_pos) = 0;
        };

        void registerUpdateHook (UpdateHook *rc);

        void unregisterUpdateHook (UpdateHook *rc);

    protected:

        void eraseClass (const std::string &name);

        GritClassMap classes;

        GObjMap gObjs;
        Space rs;
        GObjPtrs activated;
        GObjPtrs needFrameCallbacks;
        GObjPtrs loaded;
        GObjPtrs fresh; // just been added - skip the queue for activation

        PhysicsWorldPtr physics;

        size_t nameGenerationCounter;

        bool shutdown;

        typedef std::vector<UpdateHook*> UpdateHooks;
        UpdateHooks updateHooks;

};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
