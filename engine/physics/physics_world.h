/* Copyright (c) The Grit Game Engine authors 2016
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

#include <centralised_log.h>
#include "../shared_ptr.h"

class RigidBody;

#ifndef physics_h
#define physics_h

enum PhysicsBoolOption {
    PHYSICS_AUTOUPDATE,

    PHYSICS_GIMPACT_ONE_WAY_MESH_HACK,
    PHYSICS_BUMPY_TRIANGLE_MESH_HACK,
    PHYSICS_USE_TRIANGLE_EDGE_INFO,
    PHYSICS_VERBOSE_CONTACTS,
    PHYSICS_VERBOSE_CASTS,
    PHYSICS_ERROR_CONTACTS,
    PHYSICS_ERROR_CASTS,
    PHYSICS_SOLVER_SPLIT_IMPULSE,
    PHYSICS_SOLVER_RANDOMISE_ORDER,
    PHYSICS_SOLVER_FRICTION_SEPARATE,
    PHYSICS_SOLVER_USE_WARM_STARTING,
    PHYSICS_SOLVER_CACHE_FRIENDLY,

    PHYSICS_DEBUG_WIREFRAME,
    PHYSICS_DEBUG_AABB,
    PHYSICS_DEBUG_FEATURES_TEXT,
    PHYSICS_DEBUG_CONTACT_POINTS,
    PHYSICS_DEBUG_CONSTRAINTS,
    PHYSICS_DEBUG_CONSTRAINTS_LIMITS,

    PHYSICS_DEBUG_NO_DEACTIVATION,
    PHYSICS_DEBUG_NO_HELP_TEXT,
    PHYSICS_DEBUG_DRAW_TEXT,
    PHYSICS_DEBUG_PROFILE_TIMINGS,
    PHYSICS_DEBUG_ENABLE_SAT_COMPARISON,
    PHYSICS_DEBUG_DISABLE_BULLET_LCP,
    PHYSICS_DEBUG_ENABLE_CCD,
    PHYSICS_DEBUG_FAST_WIREFRAME

};

enum PhysicsIntOption {
    PHYSICS_SOLVER_ITERATIONS
};

enum PhysicsFloatOption {
    PHYSICS_GRAVITY_X,
    PHYSICS_GRAVITY_Y,
    PHYSICS_GRAVITY_Z,
    PHYSICS_STEP_SIZE,
    PHYSICS_CONTACT_BREAKING_THRESHOLD,
    PHYSICS_DEACTIVATION_TIME,
    PHYSICS_SOLVER_DAMPING,
    PHYSICS_SOLVER_ERP,
    PHYSICS_SOLVER_ERP2,
    PHYSICS_SOLVER_SPLIT_IMPULSE_THRESHOLD,
    PHYSICS_SOLVER_LINEAR_SLOP,
    PHYSICS_SOLVER_WARM_STARTING_FACTOR
};

std::string physics_option_to_string (PhysicsBoolOption o);
std::string physics_option_to_string (PhysicsIntOption o);
std::string physics_option_to_string (PhysicsFloatOption o);

// set's t to either 0,1,2 and fills in the approriate argument
void physics_option_from_string (const std::string &s,
                                 int &t,
                                 PhysicsBoolOption &o0,
                                 PhysicsIntOption &o1,
                                 PhysicsFloatOption &o2);

void physics_option (PhysicsBoolOption o, bool v);
bool physics_option (PhysicsBoolOption o);
void physics_option (PhysicsIntOption o, int v);
int physics_option (PhysicsIntOption o);
void physics_option (PhysicsFloatOption o, float v);
float physics_option (PhysicsFloatOption o);

void physics_option_reset (void);


#include <btBulletDynamicsCommon.h>

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#include <math_util.h>

#include "tcol_parser.h"
#include "collision_mesh.h"

#include "../grit_object.h"

#include "../lua_ptr.h"

#include "physical_material.h"


// a class that extends a bullet class
class DynamicsWorld;

void physics_update (lua_State *L);

// to be extended by lua wrapper or whatever
class SweepCallback {
    public:
    virtual void result (RigidBody &body, float d, const Vector3 &normal, int m) = 0;
};

void physics_ray (const Vector3 &start,
                  const Vector3 &end,
                  SweepCallback &rcb);

void physics_sweep_sphere (const Vector3 &start, const Vector3 &end,
                           SweepCallback &scb, float radius);

void physics_sweep_cylinder (const Vector3 &start, const Quaternion &startq,
                             const Vector3 &end,
                             SweepCallback &scb, float radius, float height);


void physics_sweep_box (const Vector3 &start, const Quaternion &startq,
                        const Vector3 &end,
                        SweepCallback &scb, const Vector3 &size);

void physics_sweep_colmesh (const Vector3 &startp,
                            const Quaternion &startq,
                            const Vector3 &endp,
                            const Quaternion &endq,
                            SweepCallback &scb,
                            const CollisionMesh *col_mesh);

class TestCallback {
    public:
    virtual void result (RigidBody *body, const Vector3 &pos, const Vector3 &wpos,
                 const Vector3 &normal, float penetration, int m) = 0;
};

void physics_test (const CollisionMesh *col_mesh, const Vector3 &pos, const Quaternion &quat,
                   bool dyn_only, TestCallback &cb);

void physics_test_sphere (float rad, const Vector3 &pos, bool dyn_only, TestCallback &cb);

void physics_draw (void);

void physics_update_graphics (lua_State *L, float extrapolate);


class RigidBody : public btMotionState, public CollisionMesh::ReloadWatcher {

    friend class CollisionMesh;

    public:

    RigidBody (const std::string &col_mesh,
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

    void stepCallback (lua_State *L, float step_size);
    void collisionCallback (lua_State *L, int lifetime, float impulse,
                RigidBody *other,
                int m, int m2, float penetration,
                const Vector3 &pos, const Vector3 &pos2, const Vector3 &wnormal);
    void stabiliseCallback (lua_State *L, float elapsed);
    void updateGraphicsCallback (lua_State *L, float extrapolate);

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

    void incRefCount (void);
    void decRefCount (lua_State *L);

    CollisionMesh * colMesh;

    GritObjectPtr owner;

    void notifyReloaded (const DiskResource *r)
    {
        (void) r;
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

    unsigned refCount;

    struct CompElement {
        bool enabled;
        btTransform offset;
    };
    btAlignedObjectArray<CompElement> localChanges; // to the master compound

    void updateCollisionFlags (void);
};

void physics_init (void);
void physics_shutdown (void);

#endif

// vim: shiftwidth=4:tabstop=4:expandtab
