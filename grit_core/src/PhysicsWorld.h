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

#include "CentralisedLog.h"
#include "SharedPtr.h"

class PhysicsWorld;
typedef SharedPtr<PhysicsWorld> PhysicsWorldPtr;

class RigidBody;
typedef SharedPtr<RigidBody> RigidBodyPtr;

#ifndef PhysicsWorld_h
#define PhysicsWorld_h

#include <btBulletDynamicsCommon.h>

extern "C" {
        #include <lua.h>
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "TColParser.h"
#include "CollisionMesh.h"

#include "GritObject.h"
#include "math_util.h"

static inline btVector3 check_nan_ (const btVector3 &v, const char *file, int line)
{
        if (isnan(v.x()) || isnan(v.y()) || isnan(v.z())) {
                CLog(file,line,true) << "Vect3 NaN from Bullet." << std::endl;
                return btVector3(0,0,0);
        }
        return v;
}

static inline Vector3 check_nan_ (const Vector3 &v, const char *file, int line)
{
        if (isnan(v.x) || isnan(v.y) || isnan(v.z)) {
                CLog(file,line,true) << "Vect3 NaN from Grit." << std::endl;
                return Vector3(0,0,0);
        }
        return v;
}

static inline btQuaternion check_nan_ (const btQuaternion &v, const char *file, int line)
{
        if (isnan(v.w()) || isnan(v.x()) || isnan(v.y()) || isnan(v.z())) {
                CLog(file,line,true) << "Quat NaN from Bullet." << std::endl;
                return btQuaternion(0,0,0,1);
        }
        return v;
}

static inline Quaternion check_nan_ (const Quaternion &v, const char *file, int line)
{
        if (isnan(v.w) || isnan(v.x) || isnan(v.y) || isnan(v.z)) {
                CLog(file,line,true) << "Quat NaN from Grit." << std::endl;
                return Quaternion(1,0,0,0);
        }
        return v;
}

static inline Vector3 from_bullet (const btVector3 &from)
{ return Vector3 (from.x(), from.y(), from.z()); }

static inline Quaternion from_bullet (const btQuaternion &from)
{ return Quaternion (from.w(), from.x(), from.y(), from.z()); }

static inline btVector3 to_bullet (const Vector3 &from)
{ return btVector3(from.x,from.y,from.z); }

static inline btQuaternion to_bullet (const Quaternion &from)
{ return btQuaternion(from.x, from.y, from.z, from.w); }

#define check_nan(x) check_nan_(x,__FILE__,__LINE__)

class DynamicsWorld : public btDiscreteDynamicsWorld {
    public:
        DynamicsWorld (btCollisionDispatcher *colDisp,
                       btBroadphaseInterface *broadphase,
                       btConstraintSolver *conSolver,
                       btCollisionConfiguration *colConf)
              : btDiscreteDynamicsWorld(colDisp,broadphase,conSolver,colConf),
                stepSize(1.0f/60.0f),
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

class PhysicsWorld {

        friend class CollisionMesh;
        friend class RigidBody;

    public:

        PhysicsWorld (const Vector3 &bounds_min, const Vector3 &bounds_max);

        ~PhysicsWorld ();

        int pump (lua_State *L, float time_step);

        void setGravity (const Vector3 &);
        Vector3 getGravity (void) const;

        CollisionMeshPtr createFromFile (
                const std::string &fileName);

        bool hasMesh (const std::string &name) const
        { return colMeshes.find(name) != colMeshes.end(); }

        CollisionMeshPtr getMesh (const std::string &name)
        {
                CollisionMeshMap::iterator i = colMeshes.find(name);
                if (i==colMeshes.end())
                        return createFromFile(name);
                return i->second;
        }

        void deleteMesh (const std::string &name);

        void clearMeshes (void)
        { colMeshes.clear(); }

        static const CollisionMeshMap &getMeshes (void) { return colMeshes; }

        const PhysicsMaterial &getMaterial (const std::string &name) const
        { return mdb.getMaterial(name); }

        const PhysicsMaterial &getMaterial (int material) const
        { return mdb.getMaterial(material); }

        void setMaterial (const std::string &name, int interaction_group)
        { mdb.setMaterial(name, interaction_group); }

        void setInteractionGroups (unsigned groups,
                                   const std::vector<std::pair<float,float> > &interactions_)
        {
                APP_ASSERT(groups * groups == interactions_.size());
                numInteractions = groups;
                interactions = interactions_;
        }

        void getFrictionRestitution (int mat0, int mat1, float &f, float &r)
        {
                unsigned char ig0 = getMaterial(mat0).interactionGroup;
                unsigned char ig1 = getMaterial(mat1).interactionGroup;
                if (ig0 > numInteractions) {
                        CERR<<"Invalid interaction group "<<ig0<<" in material \""
                            <<getMaterial(mat0).name<<"\""<<std::endl;
                        f = 0; r = 0; return;
                }
                if (ig1 > numInteractions) {
                        CERR<<"Invalid interaction group "<<ig1<<" in material \""
                            <<getMaterial(mat1).name<<"\""<<std::endl;
                        f = 0; r = 0; return;
                }
                int code = ig0*numInteractions + ig1;
                f = interactions[code].first;
                r = interactions[code].second;
        }

        // to be extended by lua wrapper or whatever
        class SweepCallback {
            public:
                virtual void result (RigidBody &body, float d,
                                     const Vector3 &normal, int m) = 0;
        };

        void ray (const Vector3 &start,
                  const Vector3 &end,
                  SweepCallback &rcb,
                  float radius=0) const;


        void sweep (const CollisionMeshPtr &col_mesh,
                    const Vector3 &startp,
                    const Quaternion &startq,
                    const Vector3 &endp,
                    const Quaternion &endq,
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

        void draw (void) { world->debugDrawWorld(); }

        bool verboseContacts;
        bool errorContacts;
        bool verboseCasts;
        bool errorCasts;
        bool bumpyTriangleMeshHack;
        bool useTriangleEdgeInfo;
        bool gimpactOneWayMeshHack;

        void updateGraphics (lua_State *L);

        const MaterialDB &getMaterialDB (void) { return mdb; }

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

        MaterialDB mdb;

        std::vector<std::pair<float, float> > interactions;
        unsigned numInteractions;
};

class RigidBody : public btMotionState {

        friend class PhysicsWorld;
        friend class CollisionMesh;

    public:

        RigidBody (const PhysicsWorldPtr &world,
                   const CollisionMeshPtr &col_mesh,
                   const Vector3 &pos,
                   const Quaternion &quat);


        ~RigidBody (void);

        void destroy (lua_State *L);

        void getWorldTransform (btTransform& into_here) const;

        void setWorldTransform (const btTransform& current_xform);

        Vector3 getPosition (void) const
        {
                if (body==NULL) return Vector3(0,0,0); // deactivated
                return from_bullet(body->getCenterOfMassPosition());
        }

        Quaternion getOrientation (void) const
        {
                if (body==NULL) return Quaternion(0,1,0,0); // deactivated
                return from_bullet(body->getOrientation());
        }

        void setPosition (const Vector3 &v)
        {
                if (body==NULL) return; // deactivated
                body->setCenterOfMassTransform(
                    btTransform(body->getOrientation(), to_bullet(v)));
                body->activate();
        }

        void setOrientation (const Quaternion &q)
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

        void force (const Vector3 &force);
        void force (const Vector3 &force,
                    const Vector3 &rel_pos);
        void impulse (const Vector3 &impulse);
        void impulse (const Vector3 &impulse,
                      const Vector3 &rel_pos);
        void torque (const Vector3 &torque);
        void torqueImpulse (const Vector3 &torque);

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

        Vector3 getLinearVelocity (void) const;
        void setLinearVelocity (const Vector3 &v);

        Vector3 getAngularVelocity (void) const;
        void setAngularVelocity (const Vector3 &v);

        Vector3 getLocalVelocity (const Vector3 &) const;

        float getMass (void) const;
        void setMass (float r);

        Vector3 getInertia (void) const;
        void setInertia (const Vector3 &v);

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

        Vector3 getElementPositionMaster (int i);
        void setElementPositionOffset (int i, const Vector3 &v);
        Vector3 getElementPositionOffset (int i);
        Quaternion getElementOrientationMaster (int i);
        void setElementOrientationOffset (int i, const Quaternion &q);
        Quaternion getElementOrientationOffset (int i);
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
