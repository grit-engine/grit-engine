#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

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

#include "CollisionMesh.h"
#include "TColParser.h"

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
        void end (btScalar time_left);

        btScalar getStepSize (void) const { return stepSize; }
        void setStepSize (btScalar v) { stepSize = v; }

        void synchronizeMotionStates (void);

        void synchronizeMotionStatesNoInterpolation (void);

    protected:

        btScalar stepSize;
        bool dirty;

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

        CollisionMeshPtr getMesh (const Ogre::String &name) const
        {
                CollisionMeshMap::iterator i = colMeshes.find(name);
                if (i==colMeshes.end())
                        OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND,
                                    "CollisionMesh does not exist: "+name,
                                    "PhysicsWorld::getMesh");
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
                virtual void result (RigidBody &body,
                                     Ogre::Real d,
                                     Ogre::Vector3 &normal) = 0;
        };

        void ray (const Ogre::Vector3 &start,
                  const Ogre::Vector3 &end,
                  SweepCallback &rcb,
                  Ogre::Real radius=0) const;


        void sweep (const CollisionMeshPtr &col_mesh,
                    const Ogre::Vector3 &startp,
                    const Ogre::Quaternion &startq,
                    const Ogre::Vector3 &endp,
                    const Ogre::Quaternion &endq,
                    SweepCallback &scb) const;


        Ogre::Real getDeactivationTime (void) const;
        void setDeactivationTime (Ogre::Real);

        Ogre::Real getContactBreakingThreshold (void) const;
        void setContactBreakingThreshold (Ogre::Real);

        Ogre::Real getSolverDamping (void) const;
        void setSolverDamping (Ogre::Real);

        int getSolverIterations (void) const;
        void setSolverIterations (int);

        Ogre::Real getSolverErp (void) const;
        void setSolverErp (Ogre::Real);

        Ogre::Real getSolverErp2 (void) const;
        void setSolverErp2 (Ogre::Real);

        Ogre::Real getSolverLinearSlop (void) const;
        void setSolverLinearSlop (Ogre::Real);

        Ogre::Real getSolverWarmStartingFactor (void) const;
        void setSolverWarmStartingFactor (Ogre::Real);

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

        Ogre::Real getSolverSplitImpulseThreshold (void) const;
        void setSolverSplitImpulseThreshold (Ogre::Real);

        btScalar getStepSize (void) const { return world->getStepSize(); }
        void setStepSize (btScalar v) { world->setStepSize(v); }

        btScalar getMaxSteps (void) const { return maxSteps; }
        void setMaxSteps (btScalar v) { maxSteps = v; }

    protected:

        btDefaultCollisionConfiguration *colConf;
        btCollisionDispatcher *colDisp;

        btBroadphaseInterface *broadphase;

        btConstraintSolver *conSolver;

        DynamicsWorld *world;

        btScalar maxSteps;

        static CollisionMeshMap colMeshes;

        lua_State *last_L;
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

        Ogre::Real getLinearDamping (void) const;
        void setLinearDamping (Ogre::Real r);

        Ogre::Real getAngularDamping (void) const;
        void setAngularDamping (Ogre::Real r);

        Ogre::Real getLinearSleepThreshold (void) const;
        void setLinearSleepThreshold (Ogre::Real r);

        Ogre::Real getAngularSleepThreshold (void) const;
        void setAngularSleepThreshold (Ogre::Real r);

        Ogre::Vector3 getLinearVelocity (void) const;
        void setLinearVelocity (const Ogre::Vector3 &v);

        Ogre::Vector3 getAngularVelocity (void) const;
        void setAngularVelocity (const Ogre::Vector3 &v);

        Ogre::Vector3 getLocalVelocity (const Ogre::Vector3 &) const;

        Ogre::Real getFriction (void) const;
        void setFriction (Ogre::Real r);

        Ogre::Real getRestitution (void) const;
        void setRestitution (Ogre::Real r);

        Ogre::Real getMass (void) const;
        void setMass (Ogre::Real r);

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

    protected:

        btTransform lastXform;

        int updateCallbackIndex;
        int stepCallbackIndex;
        int stabiliseCallbackIndex;

        btRigidBody * body;

        // yes, it's stupid, but it must be done
        RigidBodyPtr self;

};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
