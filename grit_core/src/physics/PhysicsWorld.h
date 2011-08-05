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

#include "../CentralisedLog.h"
#include "../SharedPtr.h"

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

#include "../GritObject.h"
#include "../math_util.h"

#include "../LuaPtr.h"

#include "PhysicalMaterial.h"


// a class that extends a bullet class
class DynamicsWorld;

class PhysicsWorld {

    friend class RigidBody;

    public:

    PhysicsWorld (void);

    ~PhysicsWorld (void);

    int pump (lua_State *L, float time_step);

    void setGravity (const Vector3 &);
    Vector3 getGravity (void) const;

    // to be extended by lua wrapper or whatever
    class SweepCallback {
        public:
        virtual void result (RigidBody &body, float d, const Vector3 &normal, int m) = 0;
    };

    void ray (const Vector3 &start,
          const Vector3 &end,
          SweepCallback &rcb,
          float radius=0) const;


    void sweep (const CollisionMesh *col_mesh,
                const Vector3 &startp,
                const Quaternion &startq,
                const Vector3 &endp,
                const Quaternion &endq,
                SweepCallback &scb) const;

    class TestCallback {
        public:
        virtual void result (RigidBody *body, const Vector3 &pos, const Vector3 &wpos,
                     const Vector3 &normal, float penetration, int m) = 0;
    };

    void test (const CollisionMesh *col_mesh, const Vector3 &pos, const Quaternion &quat,
               bool dyn_only, TestCallback &cb);

    void testSphere (float rad, const Vector3 &pos, bool dyn_only, TestCallback &cb);

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

    btScalar getStepSize (void) const;
    void setStepSize (btScalar v);

    btScalar getMaxSteps (void) const;
    void setMaxSteps (btScalar v);

    void draw (void);

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

    lua_State *last_L;

};

class RigidBody : public btMotionState, public CollisionMesh::ReloadWatcher {

    friend class PhysicsWorld;
    friend class CollisionMesh;

    public:

    RigidBody (const PhysicsWorldPtr &world,
               const std::string &col_mesh,
               const Vector3 &pos,
               const Quaternion &quat);


    ~RigidBody (void);

    void destroy (lua_State *L);

    bool destroyed (void) const { return body==NULL; }

    void getWorldTransform (btTransform& into_here) const;

    void setWorldTransform (const btTransform& current_xform);

    Vector3 getPosition (void) const;

    Quaternion getOrientation (void) const;

    void setPosition (const Vector3 &v);

    void setOrientation (const Quaternion &q);

    void stepCallback (lua_State *L);
    void collisionCallback (lua_State *L, int lifetime, float impulse,
                const RigidBodyPtr &other,
                int m, int m2, float penetration,
                const Vector3 &pos, const Vector3 &pos2, const Vector3 &wnormal);
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

    CollisionMesh * colMesh;

    GritObjectPtr owner;

    void notifyReloaded (void)
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

    bool getGhost (void) const { return ghost; }
    void setGhost (bool v) { ghost = v; updateCollisionFlags(); }

    protected:

    float mass;
    bool ghost;

    btTransform lastXform;

    public:
    LuaPtr updateCallbackPtr;
    LuaPtr stepCallbackPtr;
    LuaPtr collisionCallbackPtr;
    LuaPtr stabiliseCallbackPtr;

    protected:
    btRigidBody *body;
    btCompoundShape *shape;

    // yes, it's stupid, but it must be done
    RigidBodyPtr self;

    struct CompElement {
        bool enabled;
        btTransform offset;
    };
    btAlignedObjectArray<CompElement> localChanges; // to the master compound

    void updateCollisionFlags (void);
};


#endif

// vim: shiftwidth=4:tabstop=4:expandtab
