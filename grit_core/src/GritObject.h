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
#include "GritObjectManager.h"
#include "BackgroundMeshLoader.h"
#include "lua_wrappers_primitives.h"

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

        bool hasLuaValue (const Ogre::String &key)
        {
                return luaValues.find(key)!=luaValues.end();
        }

        void setLuaValue (const Ogre::String &key, const Ogre::Real r)
        {
                luaValues[key].type = 0;
                luaValues[key].real = r;
        }

        void setLuaValue (const Ogre::String &key,
                                  const Ogre::String &s)
        {
                luaValues[key].type = 1;
                luaValues[key].str = s;
        }

        void setLuaValue (const Ogre::String &key,
                                  const Ogre::Vector3 &v3)
        {
                luaValues[key].type = 2;
                luaValues[key].v3 = v3;
        }

        void setLuaValue (const Ogre::String &key,
                                  const Ogre::Quaternion &q)
        {
                luaValues[key].type = 3;
                luaValues[key].q = q;
        }

        void pushLuaValue (lua_State *L, const Ogre::String &key)
        {
                if (!hasLuaValue(key)) {
                        lua_pushnil(L);
                } else {
                        const LuaValue &v = luaValues[key];
                        switch (v.type) {
                                case 0:
                                lua_pushnumber(L,v.real);
                                break;
                                case 1:
                                lua_pushstring(L,v.str.c_str());
                                break;
                                case 2:
                                push(L,new Ogre::Vector3(v.v3),VECTOR3_TAG);
                                break;
                                case 3:
                                push(L,new Ogre::Quaternion(v.q),QUAT_TAG);
                                break;
                        }
                }
        }

        void clearLuaValue (const Ogre::String &key)
        {
                luaValues.erase(key);
        }

        GritClass *getClass (void) { return gritClass; }


        Ogre::Real getFade (void) const { return lastFade; }


        const Ogre::String name;

        inline void updateIndex (int index_)
        {
                index = index_;
        }

        void updateSphere (Ogre::Real x_, Ogre::Real y_, Ogre::Real z_,
                                   Ogre::Real r_);

        Ogre::Real getX() const { return x; }
        Ogre::Real getY() const { return y; }
        Ogre::Real getZ() const { return y; }
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


    protected:

        Ogre::Real x, y, z, r;
                
        GritClass *gritClass;

        int lua;

        int index;

        struct LuaValue {
                int type;
                Ogre::String str;
                Ogre::Real real;
                Ogre::Vector3 v3;
                Ogre::Quaternion q;
        };

        GritObjectPtr near, far;
                
        StringPairs advanceResources;

        typedef std::map<Ogre::String,LuaValue> LuaValueMap;
        LuaValueMap luaValues;

        bool demandRegistered;
        Demand demand;

        Ogre::Real imposedFarFade;
        Ogre::Real lastFade;
};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
