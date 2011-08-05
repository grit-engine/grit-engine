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
#include <string>

#include "SharedPtr.h"

class GritObject;
typedef SharedPtr<GritObject> GritObjectPtr;
typedef std::map<std::string,GritObjectPtr> GObjMap;
typedef std::vector<GritObjectPtr> GObjPtrs;
typedef std::set<GritObjectPtr> GObjSet;


#ifndef GritObject_h
#define GritObject_h

#include "Streamer.h"
#include "ExternalTable.h"
#include "BackgroundLoader.h"

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

class GritObject {

    public:

        GritObject (const std::string &name_, GritClass *gritClass_);

        ~GritObject (void) { }

        void destroy (lua_State *L, const GritObjectPtr &self);

        bool deactivate (lua_State *L, const GritObjectPtr &self);

        void init (lua_State *L, const GritObjectPtr &self);

        void activate (lua_State *L, const GritObjectPtr &self);

        float calcFade (const float range2, bool &overlap);

        void notifyRange2 (lua_State *L, const GritObjectPtr &self, const float range2)
        {
                if (gritClass==NULL) GRIT_EXCEPT("Object destroyed");
                bool overlap = false;
                float fade = calcFade(range2, overlap);
                // if fade is sufficiently different
                if (fade!=lastFade) {
                        notifyFade(L, self, fade);
                        lastFade = fade;
                }
        }

        bool frameCallback (lua_State *L,
                            const GritObjectPtr &self,
                            const float time);

        void notifyFade (lua_State *L,
                         const GritObjectPtr &self,
                         const float fade);

        bool isActivated (void) const { return lua!=LUA_NOREF; }

        void pushLuaTable (lua_State *L)
        {
                 lua_rawgeti(L,LUA_REGISTRYINDEX,lua);
        }


        void addDiskResource (const std::string &name)
        { demand.addDiskResource(name); }

        void clearDiskResources (void)
        { demand.clearDiskResources(); }

        bool requestLoad (const Vector3 &cam_pos)
        {
                return demand.requestLoad((cam_pos - pos).length2());
        }
        void tryUnloadResources(void)
        {
                demand.finishedWith();
        }
        bool isInBackgroundQueue (void)
        {
                return demand.isInBackgroundQueue();
        }
        bool backgroundLoadingCausedError (void)
        {
                return demand.errorOnLoad();
        }



        GritClass *getClass (void) { return gritClass; }

        float getFade (void) const { return lastFade; }

        bool getNeedsFrameCallbacks (void) const { return needsFrameCallbacks; }
        void setNeedsFrameCallbacks (const GritObjectPtr &self, bool v);


        const std::string name;

        inline void updateIndex (int index_)
        {
                index = index_;
        }

        void updateSphere (const Vector3 &pos, float r_);
        void updateSphere (const Vector3 &pos);
        void updateSphere (float r_);

        Vector3 getPos() const { return pos; }
        float getR() const { return r; }
                
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

        float range2 (const Vector3 &cam_pos) const
        {
                return (cam_pos - pos).length2() / (r*r);
        }

        bool withinRange (const Vector3 &cam_pos, float factor) const
        {
                const float rad = r*factor;
                return (cam_pos - pos).length2() < rad*rad;
        }

        float getImposedFarFade (void) const { return imposedFarFade; }

        ExternalTable userValues;

        bool anonymous;

    protected:

        Vector3 pos;
        float r;
                
        GritClass *gritClass;

        int lua;

        int index;

        bool needsFrameCallbacks;

        GritObjectPtr near, far;
                
        bool demandRegistered;
        Demand demand;


        float imposedFarFade;
        float lastFade;
};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
