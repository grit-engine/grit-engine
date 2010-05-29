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

#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

#include <map>

#include <OgreSharedPtr.h>
#include <OgreException.h>

#include "CentralisedLog.h"

class PhysicsWorld;
typedef Ogre::SharedPtr<PhysicsWorld> PhysicsWorldPtr;

class RigidBody;
typedef Ogre::SharedPtr<RigidBody> RigidBodyPtr;

#ifndef PhysicsWorld_h
#define PhysicsWorld_h

#include <OgreAxisAlignedBox.h>

#include <btBulletDynamicsCommon.h>

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "TColParser.h"
#include "CollisionMesh.h"

#include "GritObject.h"

static inline Ogre::Vector3 to_ogre(const btVector3 &from)
{
        if (isnan(from.x()) || isnan(from.y()) || isnan(from.z())) {
                CERR << "NaN from bullet." << std::endl;
                return Ogre::Vector3(0,0,0);
        }
        return Ogre::Vector3 (from.x(), from.y(), from.z());
}

static inline Ogre::Quaternion to_ogre(const btQuaternion &from)
{
        if (isnan(from.w()) || isnan(from.x()) || isnan(from.y()) || isnan(from.z())) {
                CERR << "NaN from bullet." << std::endl;
                return Ogre::Quaternion(1,0,0,0);
        }
        return Ogre::Quaternion (from.w(), from.x(), from.y(), from.z());
}

static inline btVector3 to_bullet (const Ogre::Vector3 &from)
{
        if (isnan(from.x) || isnan(from.y) || isnan(from.z)) {
                CERR << "NaN from OGRE." << std::endl;
                return btVector3(0,0,0);
        }
        return btVector3(from.x,from.y,from.z);
}

static inline btQuaternion to_bullet (const Ogre::Quaternion &from)
{
        if (isnan(from.w) || isnan(from.x) || isnan(from.y) || isnan(from.z)) {
                CERR << "NaN from OGRE." << std::endl;
                return btQuaternion(0,0,0,1);
        }
        return btQuaternion(from.x, from.y, from.z, from.w);
}

class DynamicsWorld : public btDiscreteDynamicsWorld {
    public:
        DynamicsWorld (btCollisionDispatcher *colDisp,
                       btBroadphaseInterface *broadphase,
                       btConstraintSolver *conSolver,
                       btCollisionConfiguration *colConf)
              : btDiscreteDynamicsWorld(colDisp,broadphase,conSolver,colConf),
                stepSize(1.0/60),
                dirty(false)
        { }

        void step (void);
        void end (void);

        btScalar getStepSize (void) const { return stepSize; }
        void setStepSize (btScalar v) { stepSize = v; }

    protected:

        btScalar stepSize;
        bool dirty;
};

struct PhysicsMaterial {
        std::string name;
        float restitution;
        float friction;
        int id;
};

class PhysicsWorld {

        friend class CollisionMesh;
        friend class RigidBody;

    public:

        PhysicsWorld (const Ogre::AxisAlignedBox &bounds);

        ~PhysicsWorld ();

        int pump (lua_State *L, float time_step);

        void setGravity (const Ogre::Vector3 &);
        Ogre::Vector3 getGravity (void) const;

        CollisionMeshPtr createFromFile (
                const Ogre::String &fileName);

        bool hasMesh (const Ogre::String &name) const
        {
                return colMeshes.find(name) != colMeshes.end();
        }

        CollisionMeshPtr getMesh (const Ogre::String &name)
        {
                CollisionMeshMap::iterator i = colMeshes.find(name);
                if (i==colMeshes.end())
                        return createFromFile(name);
                return i->second;
        }

        void deleteMesh (const Ogre::String &name);

        void clearMeshes (void)
        {
                colMeshes.clear();
        }

        static const CollisionMeshMap &getMeshes (void) { return colMeshes; }

        // to be extended by lua wrapper or whatever

        class SweepCallback {
            public:
                virtual void result (RigidBody &body, float d,
                                     const Ogre::Vector3 &normal, int m) = 0;
        };

        void ray (const Ogre::Vector3 &start,
                  const Ogre::Vector3 &end,
                  SweepCallback &rcb,
                  float radius=0) const;


        void sweep (const CollisionMeshPtr &col_mesh,
                    const Ogre::Vector3 &startp,
                    const Ogre::Quaternion &startq,
                    const Ogre::Vector3 &endp,
                    const Ogre::Quaternion &endq,
                    SweepCallback &scb) const;


        float getDeactivationTime (void) const;
        void setDeactivationTime (float);

        float getContactBreakingThreshold (void) const;
        void setContactBreakingThreshold (float);

        float getSolverDamping (void) const;
        void setSolverDamping (float);

        int getSolverIterations (void) const;
        void setSolverIterations (int);

        float getSolverErp (void) const;
        void setSolverErp (float);

        float getSolverErp2 (void) const;
        void setSolverErp2 (float);

        float getSolverLinearSlop (void) const;
        void setSolverLinearSlop (float);

        float getSolverWarmStartingFactor (void) const;
        void setSolverWarmStartingFactor (float);

        bool getSolverRandomiseOrder (void) const;
        void setSolverRandomiseOrder (bool);

        bool getSolverFrictionSeparate (void) const;
        void setSolverFrictionSeparate (bool);

        bool getSolverUseWarmStarting (void) const;
        void setSolverUseWarmStarting (bool);

        bool getSolverCacheFriendly (void) const;
        void setSolverCacheFriendly (bool);

        bool getSolverSplitImpulse (void) const;
        void setSolverSplitImpulse (bool);

        float getSolverSplitImpulseThreshold (void) const;
        void setSolverSplitImpulseThreshold (float);

        btScalar getStepSize (void) const { return world->getStepSize(); }
        void setStepSize (btScalar v) { world->setStepSize(v); }

        btScalar getMaxSteps (void) const { return maxSteps; }
        void setMaxSteps (btScalar v) { maxSteps = v; }

        const PhysicsMaterial &getMaterial (const std::string &name) const {
                MaterialDB::const_iterator i = mdb.find(name);
                if (i==mdb.end()) 
                        OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND,
                                    "Physical Material \""+name+"\" does not exist.",
                                    "PhysicsWorld::getMaterial");
                return i->second;
        }

        const PhysicsMaterial &getMaterial (int material) const {
                return *mdb2[material];
        }

        void setMaterial (const std::string &name, float friction, float restitution) {
                PhysicsMaterial &m = mdb[name];
                m.name = name;
                m.friction = friction;
                m.restitution = restitution;
                m.id = mdb2.size();
                mdb2.push_back(&m);
        }

        void draw (void) { world->debugDrawWorld(); }

        bool verboseContacts;
        bool errorContacts;
        bool verboseCasts;
        bool errorCasts;
        bool bumpyTriangleMeshHack;
        bool useTriangleEdgeInfo;
        bool gimpactOneWayMeshHack;

        void updateGraphics (lua_State *L);

    protected:

        bool needsGraphicsUpdate;

        btDefaultCollisionConfiguration *colConf;
        btCollisionDispatcher *colDisp;

        btBroadphaseInterface *broadphase;

        btConstraintSolver *conSolver;

        DynamicsWorld *world;

        btScalar maxSteps;

        btScalar extrapolate;

        static CollisionMeshMap colMeshes;

        lua_State *last_L;

        typedef std::map<std::string, PhysicsMaterial> MaterialDB; // map string to material
        MaterialDB mdb;
        typedef std::vector<PhysicsMaterial*> MaterialDB2; // map id to material
        MaterialDB2 mdb2;
};

class RigidBody : public btMotionState {

        friend class PhysicsWorld;
        friend class CollisionMesh;

    public:

        RigidBody (const PhysicsWorldPtr &world,
                   const CollisionMeshPtr &col_mesh,
                   const Ogre::Vector3 &pos,
                   const Ogre::Quaternion &quat);


        ~RigidBody (void);

        void destroy (lua_State *L);

        void getWorldTransform (btTransform& into_here) const;

        void setWorldTransform (const btTransform& current_xform);

        Ogre::Vector3 getPosition (void) const
        {
                if (body==NULL) return Ogre::Vector3(0,0,0); // deactivated
                return to_ogre(body->getCenterOfMassPosition());
        }

        Ogre::Quaternion getOrientation (void) const
        {
                if (body==NULL) return Ogre::Quaternion(0,1,0,0); // deactivated
                return to_ogre(body->getOrientation());
        }

        void setPosition (const Ogre::Vector3 &v)
        {
                if (body==NULL) return; // deactivated
                body->setCenterOfMassTransform(
                        btTransform(body->getOrientation(), to_bullet(v)));
                body->activate();
        }

        void setOrientation (const Ogre::Quaternion &q)
        {
                if (body==NULL) return; // deactivated
                body->setCenterOfMassTransform(
                     btTransform(to_bullet(q),body->getCenterOfMassPosition()));
                body->activate();
        }

        void stepCallback (lua_State *L);
        void stabiliseCallback (lua_State *L);
        void updateGraphicsCallback (lua_State *L);

        void activate (void);
        void deactivate (void);

        void force (const Ogre::Vector3 &force);
        void force (const Ogre::Vector3 &force,
                    const Ogre::Vector3 &rel_pos);
        void impulse (const Ogre::Vector3 &impulse);
        void impulse (const Ogre::Vector3 &impulse,
                      const Ogre::Vector3 &rel_pos);
        void torque (const Ogre::Vector3 &torque);
        void torqueImpulse (const Ogre::Vector3 &torque);

        float getContactProcessingThreshold (void) const;
        void setContactProcessingThreshold (float v);

        float getLinearDamping (void) const;
        void setLinearDamping (float r);

        float getAngularDamping (void) const;
        void setAngularDamping (float r);

        float getLinearSleepThreshold (void) const;
        void setLinearSleepThreshold (float r);

        float getAngularSleepThreshold (void) const;
        void setAngularSleepThreshold (float r);

        Ogre::Vector3 getLinearVelocity (void) const;
        void setLinearVelocity (const Ogre::Vector3 &v);

        Ogre::Vector3 getAngularVelocity (void) const;
        void setAngularVelocity (const Ogre::Vector3 &v);

        Ogre::Vector3 getLocalVelocity (const Ogre::Vector3 &) const;

        float getMass (void) const;
        void setMass (float r);

        Ogre::Vector3 getInertia (void) const;
        void setInertia (const Ogre::Vector3 &v);

        RigidBodyPtr getPtr (void) const { return self; }

        const PhysicsWorldPtr world;

        const CollisionMeshPtr colMesh;

        void pushUpdateCallback (lua_State *L)
        {
                // pushes nil if index is LUA_NOREF
                lua_rawgeti(L,LUA_REGISTRYINDEX,updateCallbackIndex);
        }

        void setUpdateCallback (lua_State *L)
        {
                // unref if not already
                luaL_unref(L,LUA_REGISTRYINDEX,updateCallbackIndex);
                updateCallbackIndex = luaL_ref(L,LUA_REGISTRYINDEX);
        }

        void pushStepCallback (lua_State *L)
        {
                // pushes nil if index is LUA_NOREF
                lua_rawgeti(L,LUA_REGISTRYINDEX,stepCallbackIndex);
        }

        void setStepCallback (lua_State *L)
        {
                // unref if not already
                luaL_unref(L,LUA_REGISTRYINDEX,stepCallbackIndex);
                stepCallbackIndex = luaL_ref(L,LUA_REGISTRYINDEX);
        }

        void pushStabiliseCallback (lua_State *L)
        {
                // pushes nil if index is LUA_NOREF
                lua_rawgeti(L,LUA_REGISTRYINDEX,stabiliseCallbackIndex);
        }

        void setStabiliseCallback (lua_State *L)
        {
                // unref if not already
                luaL_unref(L,LUA_REGISTRYINDEX,stabiliseCallbackIndex);
                stabiliseCallbackIndex = luaL_ref(L,LUA_REGISTRYINDEX);
        }

        GritObjectPtr owner;

        void notifyMeshReloaded (void)
        {
                removeFromWorld();
                addToWorld();
        }

        void addToWorld (void);
        void removeFromWorld (void);

        void setElementEnabled (int i, bool v);
        bool getElementEnabled (int i);

        Ogre::Vector3 getElementPositionMaster (int i);
        void setElementPositionOffset (int i, const Ogre::Vector3 &v);
        Ogre::Vector3 getElementPositionOffset (int i);
        Ogre::Quaternion getElementOrientationMaster (int i);
        void setElementOrientationOffset (int i, const Ogre::Quaternion &q);
        Ogre::Quaternion getElementOrientationOffset (int i);
        int getNumElements (void) { return localChanges.size(); };

    protected:

        btTransform lastXform;

        int updateCallbackIndex;
        int stepCallbackIndex;
        int stabiliseCallbackIndex;

        btRigidBody *body;
        btCompoundShape *shape;

        // yes, it's stupid, but it must be done
        RigidBodyPtr self;

        struct CompElement {
                bool enabled;
                btTransform offset;
        };
        btAlignedObjectArray<CompElement> localChanges; // to the master compound
};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
