#include <OgreException.h>

#include "PhysicsWorld.h"
#include "GritObject.h"
#include "Grit.h"
#include "lua_wrappers_physics.h"
#include "CentralisedLog.h"

//#include <GIMPACT/Bullet/btGImpactShape.h>
//#include <GIMPACT/Bullet/btGImpactCollisionAlgorithm.h>

#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"


void DynamicsWorld::step (void)
{
        if (!dirty) {
                saveKinematicState(stepSize);
                applyGravity();
                dirty = true;
        }
        internalSingleStepSimulation(stepSize);
}

void DynamicsWorld::synchronizeMotionStates (void)
{
        for (int i=0 ; i<m_collisionObjects.size() ; i++) {

                btCollisionObject* colObj = m_collisionObjects[i];
                
                btRigidBody* body = btRigidBody::upcast(colObj);

                if (body==NULL) continue;
                if (body->isStaticOrKinematicObject()) continue;
                if (body->getActivationState() == ISLAND_SLEEPING) continue;
                if (body->getMotionState()==NULL) continue;

                btTransform interpolatedTransform;

                btTransformUtil::integrateTransform(
                        body->getInterpolationWorldTransform(),
                        body->getInterpolationLinearVelocity(),
                        body->getInterpolationAngularVelocity(),
                        m_localTime*body->getHitFraction(),
                        interpolatedTransform);

                body->getMotionState()
                        ->setWorldTransform(interpolatedTransform);
        }
}


void DynamicsWorld::synchronizeMotionStatesNoInterpolation (void)
{
        for (int i=0 ; i<m_collisionObjects.size() ; i++) {

                btCollisionObject* colObj = m_collisionObjects[i];
                
                btRigidBody* body = btRigidBody::upcast(colObj);

                if (body==NULL) continue;
                if (body->isStaticOrKinematicObject()) continue;
                if (body->getActivationState() == ISLAND_SLEEPING) continue;
                if (body->getMotionState()==NULL) continue;

                body->getMotionState()->
                      setWorldTransform(body->getInterpolationWorldTransform());
        }
}


void DynamicsWorld::end (btScalar time_left)
{
        if (dirty) {
                if (time_left>0) {
                        m_localTime = time_left;
                        synchronizeMotionStates();
                } else {
                        synchronizeMotionStatesNoInterpolation();
                }
                        
                dirty = false;
        }
        clearForces();
}


/*
btVector3 get_face_normal (const btStridingMeshInterface *mesh, int face) {
        PHY_ScalarType vertexes_type, indexes_type;
        const unsigned char *vertexes;
        int num_vertexes;
        int vertexes_stride;
        const unsigned char *indexes;
        int num_faces;
        int face_stride;
        mesh->getLockedReadOnlyVertexIndexBase
                (&vertexes, num_vertexes, vertexes_type, vertexes_stride,
                 &indexes, face_stride, num_faces, indexes_type);
        APP_ASSERT(vertexes_type == PHY_FLOAT);
        APP_ASSERT(indexes_type == PHY_INTEGER);
        const Face *indexes2 =
                reinterpret_cast<const Face*>(indexes + face_stride*face);
        int i1=indexes2->v1, i2=indexes2->v2, i3=indexes2->v3;
        unsigned int flag = indexes2->flag;
        (void) flag; // TODO: do something useful with this
        btVector3 v1 = *reinterpret_cast<const btVector3 *>
                                (vertexes + vertexes_stride * i1);
        btVector3 v2 = *reinterpret_cast<const btVector3 *>
                                (vertexes + vertexes_stride * i2);
        btVector3 v3 = *reinterpret_cast<const btVector3 *>
                                (vertexes + vertexes_stride * i3);
        btVector3 r;
        r = (v2-v1).cross(v3-v1);
        r.normalize();
        return r;
}
*/

void contact_added_callback_obj (btManifoldPoint& cp,
                                 const btCollisionObject* colObj,
                                 int partId, int index)
{
        (void) partId;
        (void) index;
        const btCollisionShape *shape = colObj->getCollisionShape();

        if (shape->getShapeType() != TRIANGLE_SHAPE_PROXYTYPE) return;
        const btTriangleShape *tshape =
               static_cast<const btTriangleShape*>(colObj->getCollisionShape());


        const btCollisionShape *parent = colObj->getRootCollisionShape();
        if (parent == NULL) return;
        switch (parent->getShapeType()) {
                case TRIANGLE_MESH_SHAPE_PROXYTYPE: {

                        btVector3 normal;
                        tshape->calcNormal(normal);

                        const btMatrix3x3 &orient =
                                colObj->getWorldTransform().getBasis();

                        normal = orient * normal;

                        btScalar dot = normal.dot(cp.m_normalWorldOnB);
                        btScalar magnitude = cp.m_normalWorldOnB.length();
                        normal *= dot > 0 ? magnitude : -magnitude;

                        cp.m_normalWorldOnB = normal;

                } ; break;
                case GIMPACT_SHAPE_PROXYTYPE: {

                        btVector3 normal;
                        tshape->calcNormal(normal);
                        normal *= -1;

                        const btMatrix3x3 &orient =
                                colObj->getWorldTransform().getBasis();

                        normal = orient * normal;

                        btScalar dot = normal.dot(cp.m_normalWorldOnB);
                        
                        if (dot < 0)
                                cp.m_normalWorldOnB -= 2 * dot * normal;


                } ; break;
        }
}

bool contact_added_callback (btManifoldPoint& cp,
                             const btCollisionObject* colObj0,
                             int partId0, int index0,
                             const btCollisionObject* colObj1,
                             int partId1, int index1)
{
        (void) colObj1;
        (void) partId1;
        (void) index1;
        contact_added_callback_obj(cp, colObj0, partId0, index0);
        //contact_added_callback_obj(cp, colObj1, partId1, index1);
        //std::cout << to_ogre(cp.m_normalWorldOnB) << std::endl;
        return true;
}       


extern ContactAddedCallback gContactAddedCallback;

PhysicsWorld::PhysicsWorld (const Ogre::AxisAlignedBox &bounds)
      : maxSteps(5)
{
        colConf = new btDefaultCollisionConfiguration();
        colDisp = new btCollisionDispatcher(colConf);

        // first 2 params specify the world AABB
        // last param is the max number of "proxies"
        broadphase = new btAxisSweep3(to_bullet(bounds.getMinimum()),
                                      to_bullet(bounds.getMaximum()),
                                      32766);

        conSolver = new btSequentialImpulseConstraintSolver();
        //conSolver = new btOdeQuickstepConstraintSolver();

        world = new DynamicsWorld(colDisp,broadphase,conSolver,colConf);


        btGImpactCollisionAlgorithm::registerAlgorithm(colDisp);

        gContactAddedCallback = contact_added_callback;
        
        world->setGravity(btVector3(0,0,-9.8));
}

PhysicsWorld::~PhysicsWorld ()
{

        // they should probably all be gone by now but just in case some
        // aren't linked to RigidBodies, this will clean them up
        for (int i=world->getNumCollisionObjects()-1 ; i>=0 ; --i) {

                btCollisionObject* victim = world->getCollisionObjectArray()[i];

                btRigidBody* victim2 = btRigidBody::upcast(victim);

                if (victim2)
                        world->removeRigidBody(victim2);

                delete victim;
        }


        delete world;
        delete conSolver;
        delete broadphase;
        delete colDisp;
        delete colConf;
}

int PhysicsWorld::pump (lua_State *L,
                         float elapsed)
{
        last_L = L;
        Ogre::Real step_size = world->getStepSize();
        int counter = 0;
        for (; counter<maxSteps ; counter++) {
                if (elapsed<step_size) break;
                elapsed -= step_size;
                world->step();
                for (int i=0 ; i<world->getNumCollisionObjects() ; ++i) {

                        btCollisionObject* victim =
                                world->getCollisionObjectArray()[i];
                        btRigidBody* victim2 = btRigidBody::upcast(victim);
                        if (victim2==NULL) continue;
                        RigidBody *rb =
                             static_cast<RigidBody*>(victim2->getMotionState());
                        rb->stepCallback(L);
                }
                for (int i=0 ; i<world->getNumCollisionObjects() ; ++i) {

                        btCollisionObject* victim =
                                world->getCollisionObjectArray()[i];
                        btRigidBody* victim2 = btRigidBody::upcast(victim);
                        if (victim2==NULL) continue;
                        RigidBody *rb =
                             static_cast<RigidBody*>(victim2->getMotionState());
                        rb->stabiliseCallback(L);
                }
        }
        // to handle errors raised by the lua callback
        push_cfunction(L, my_lua_error_handler);
        world->end(elapsed);
        lua_pop(L,1); // error handler
        return counter;
}

Ogre::Real PhysicsWorld::getDeactivationTime (void) const
{ return gDeactivationTime; }
void PhysicsWorld::setDeactivationTime (Ogre::Real v)
{ gDeactivationTime = v; }

Ogre::Real PhysicsWorld::getContactBreakingThreshold (void) const
{ return gContactBreakingThreshold; }
void PhysicsWorld::setContactBreakingThreshold (Ogre::Real v)
{ gContactBreakingThreshold = v; }

Ogre::Vector3 PhysicsWorld::getGravity (void) const
{ return to_ogre(world->getGravity()); }
void PhysicsWorld::setGravity (const Ogre::Vector3 &gravity)
{ world->setGravity(to_bullet(gravity)); }

Ogre::Real PhysicsWorld::getSolverDamping (void) const
{ return world->getSolverInfo().m_damping; }
void PhysicsWorld::setSolverDamping (Ogre::Real v)
{ world->getSolverInfo().m_damping = v; }

int PhysicsWorld::getSolverIterations (void) const
{ return world->getSolverInfo().m_numIterations; }
void PhysicsWorld::setSolverIterations (int v)
{ world->getSolverInfo().m_numIterations = v; }

Ogre::Real PhysicsWorld::getSolverErp (void) const
{ return world->getSolverInfo().m_erp; } 
void PhysicsWorld::setSolverErp (Ogre::Real v)
{ world->getSolverInfo().m_erp = v; } 

Ogre::Real PhysicsWorld::getSolverErp2 (void) const
{ return world->getSolverInfo().m_erp2; } 
void PhysicsWorld::setSolverErp2 (Ogre::Real v)
{ world->getSolverInfo().m_erp2 = v; } 

Ogre::Real PhysicsWorld::getSolverLinearSlop (void) const
{ return world->getSolverInfo().m_linearSlop; } 
void PhysicsWorld::setSolverLinearSlop (Ogre::Real v)
{ world->getSolverInfo().m_linearSlop = v; } 

Ogre::Real PhysicsWorld::getSolverWarmStartingFactor (void) const
{ return world->getSolverInfo().m_warmstartingFactor; } 
void PhysicsWorld::setSolverWarmStartingFactor (Ogre::Real v)
{ world->getSolverInfo().m_warmstartingFactor = v; } 

bool PhysicsWorld::getSolverSplitImpulse (void) const
{ return 0!=world->getSolverInfo().m_splitImpulse; } 
void PhysicsWorld::setSolverSplitImpulse (bool v)
{ world->getSolverInfo().m_splitImpulse = v; } 

Ogre::Real PhysicsWorld::getSolverSplitImpulseThreshold (void) const
{ return world->getSolverInfo().m_splitImpulsePenetrationThreshold; } 
void PhysicsWorld::setSolverSplitImpulseThreshold (Ogre::Real v)
{ world->getSolverInfo().m_splitImpulsePenetrationThreshold = v; } 

static inline int set_flag (int &var, int flag, bool val)
{ return var = (var & ~flag) | (val?flag:0); }

bool PhysicsWorld::getSolverRandomiseOrder (void) const
{ return 0!=(world->getSolverInfo().m_solverMode & SOLVER_RANDMIZE_ORDER); } 
void PhysicsWorld::setSolverRandomiseOrder (bool v)
{ set_flag(world->getSolverInfo().m_solverMode,SOLVER_RANDMIZE_ORDER,v); } 

bool PhysicsWorld::getSolverFrictionSeparate (void) const
{ return 0!=(world->getSolverInfo().m_solverMode & SOLVER_FRICTION_SEPARATE); } 
void PhysicsWorld::setSolverFrictionSeparate (bool v)
{ set_flag(world->getSolverInfo().m_solverMode,SOLVER_FRICTION_SEPARATE,v); } 

bool PhysicsWorld::getSolverUseWarmStarting (void) const
{ return 0!=(world->getSolverInfo().m_solverMode & SOLVER_USE_WARMSTARTING); } 
void PhysicsWorld::setSolverUseWarmStarting (bool v)
{ set_flag(world->getSolverInfo().m_solverMode,SOLVER_USE_WARMSTARTING,v); } 

bool PhysicsWorld::getSolverCacheFriendly (void) const
{ return 0!=(world->getSolverInfo().m_solverMode & SOLVER_CACHE_FRIENDLY); } 
void PhysicsWorld::setSolverCacheFriendly (bool v)
{ set_flag(world->getSolverInfo().m_solverMode,SOLVER_CACHE_FRIENDLY,v); } 


static inline void check_name(const PhysicsWorld &pw, const Ogre::String &name,
                              const char *func_name)
{
        if (pw.hasMesh(name))
                OGRE_EXCEPT(Ogre::Exception::ERR_DUPLICATE_ITEM,
                            "Collision mesh \""+name+"\" already exists.",
                            func_name);
}

CollisionMeshPtr PhysicsWorld::createFromFile (const Ogre::String &name)
{
        check_name(*this,name,"PhysicsWorld::createFromFile");
        CollisionMeshPtr cmp = CollisionMeshPtr(new CollisionMesh(name));
        // Note: this only works because both ogre and bullet are using float
        // if this situation changes it will be necessary to convert
        // from Ogre::Real to btScalar.
        Ogre::DataStreamPtr file =
                Ogre::ResourceGroupManager::getSingleton()
                        .openResource(name,"GRIT");
        cmp->importFromFile(file);
        colMeshes[name] = cmp;
        return cmp;
}

void PhysicsWorld::deleteMesh (const Ogre::String &name)
{
        CollisionMeshMap::iterator i = colMeshes.find(name);
        if (i==colMeshes.end())
                OGRE_EXCEPT(Ogre::Exception::ERR_DUPLICATE_ITEM,
                            "Collision mesh \""+name+"\" doesn't exist.",
                            "PhysicsWorld::deleteByName");
        colMeshes.erase(i);
}

class BulletRayCallback : public btCollisionWorld::RayResultCallback {
    public:
        BulletRayCallback (PhysicsWorld::SweepCallback &scb_) : scb(scb_) { }
        virtual btScalar addSingleResult (btCollisionWorld::LocalRayResult&r,
                                          bool)
        {
                btRigidBody *body = btRigidBody::upcast(r.m_collisionObject);
                if (body == NULL) return r.m_hitFraction;
                RigidBody *rb= dynamic_cast<RigidBody*>(body->getMotionState());
                if (rb == NULL) return r.m_hitFraction;
                Ogre::Vector3 normal = to_ogre(r.m_hitNormalLocal);
                // TODO other data from r
                scb.result(*rb, r.m_hitFraction, normal);
                return r.m_hitFraction;
        }
    protected:
        PhysicsWorld::SweepCallback &scb;
};

class BulletSweepCallback : public btCollisionWorld::ConvexResultCallback {
    public:
        BulletSweepCallback (PhysicsWorld::SweepCallback &scb_) : scb(scb_) { }
        virtual btScalar addSingleResult (btCollisionWorld::LocalConvexResult&r,
                                          bool)
        {
                btRigidBody *body = btRigidBody::upcast(r.m_hitCollisionObject);
                if (body == NULL) return r.m_hitFraction;
                RigidBody *rb= dynamic_cast<RigidBody*>(body->getMotionState());
                if (rb == NULL) return r.m_hitFraction;
                Ogre::Vector3 normal = to_ogre(r.m_hitNormalLocal);
                // TODO other data from r
                scb.result(*rb, r.m_hitFraction, normal);
                return r.m_hitFraction;
        }
    protected:
        PhysicsWorld::SweepCallback &scb;
};

void PhysicsWorld::ray (const Ogre::Vector3 &start,
                        const Ogre::Vector3 &end,
                        SweepCallback &scb,
                        Ogre::Real radius) const
{
        if (radius<0) {
                BulletRayCallback brcb(scb);
                world->rayTest(to_bullet(start),to_bullet(end),brcb);
        } else {
                BulletSweepCallback bscb(scb);
                btSphereShape tmpSphere(radius);
                btTransform from(btQuaternion(0,0,0,1),to_bullet(start));
                btTransform to(btQuaternion(0,0,0,1),to_bullet(end));
                world->convexSweepTest(&tmpSphere,from,to,bscb);
        }
}

void PhysicsWorld::sweep (const CollisionMeshPtr &col_mesh,
                          const Ogre::Vector3 &startp,
                          const Ogre::Quaternion &startq,
                          const Ogre::Vector3 &endp,
                          const Ogre::Quaternion &endq,
                          SweepCallback &scb) const
{
        BulletSweepCallback bscb(scb);
        btTransform start(to_bullet(startq),to_bullet(startp));
        btTransform end(to_bullet(endq),to_bullet(endp));
        btConvexShape *conv =dynamic_cast<btConvexShape*>(col_mesh->getShape());
        if (conv==NULL) return;
        world->convexSweepTest(conv,start,end,bscb);
}

CollisionMeshMap PhysicsWorld::colMeshes;



RigidBody::RigidBody (const PhysicsWorldPtr &world_,
                      const CollisionMeshPtr &col_mesh,
                      const Ogre::Vector3 &pos,
                      const Ogre::Quaternion &quat)
      : world(world_), colMesh(col_mesh),
        lastXform(to_bullet(quat),to_bullet(pos))
{
        // first, peel callback from lua stack...
        updateCallbackIndex = LUA_NOREF;
        stepCallbackIndex = LUA_NOREF;
        stabiliseCallbackIndex = LUA_NOREF;

        btCollisionShape *shape = colMesh->getShape();
        float mass = colMesh->getMass();

        // TODO: move this to collision mesh init?
        btVector3 local_inertia(0,0,0);
        if (mass != 0)
                shape->calculateLocalInertia(mass,local_inertia);

        btRigidBody::btRigidBodyConstructionInfo
                info(mass, this, shape, local_inertia);

        info.m_linearDamping = colMesh->getLinearDamping();
        info.m_angularDamping = colMesh->getAngularDamping();
        info.m_friction = colMesh->getFriction();
        info.m_restitution = colMesh->getRestitution();
        info.m_linearSleepingThreshold = colMesh->getLinearSleepThreshold();
        info.m_angularSleepingThreshold = colMesh->getAngularSleepThreshold();

        body = new btRigidBody(info);

        world->world->addRigidBody(body);

        // Only do CCD if speed of body exceeeds this
        body->setCcdMotionThreshold(colMesh->getCCDMotionThreshold());
        body->setCcdSweptSphereRadius(colMesh->getCCDSweptSphereRadius());

        body->setCollisionFlags(body->getCollisionFlags() |
                                btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);


        self = RigidBodyPtr(this);
        // Make the self pointer a weak reference, otherwise
        // the object will be kept alive until destroy() is called!
        // We instead want the object to print a warning if it is destructed
        // without first being destroyed.
        // RigidBody must only be constructed as follows:
        // RigidBodyPtr blah = (new RigidBody(...))->getPtr()
        (*self.useCountPointer())--;
}

void RigidBody::destroy (lua_State *L)
{
        if (body==NULL) return;
        world->world->removeRigidBody(body);
        delete body;
        body = NULL;
        luaL_unref(L,LUA_REGISTRYINDEX,updateCallbackIndex);
        luaL_unref(L,LUA_REGISTRYINDEX,stepCallbackIndex);
        // the next line prevents the RigidBody being destructed prematurely
        (*self.useCountPointer())++;
        self.setNull();
}

RigidBody::~RigidBody (void)
{
        if (body==NULL) return;
        CERR << "destructing RigidBody: destroy() was not called" << std::endl;
        world->world->removeRigidBody(body);
        delete body;
        body = NULL;
        // the next line will cause the counter to go negative but should cause
        // no problem
        self.setNull();
}

void RigidBody::getWorldTransform (btTransform& into_here) const
{
        into_here = lastXform;
}

void RigidBody::setWorldTransform (const btTransform& current_xform)
{
        lua_State *L = world->last_L;

        if (updateCallbackIndex==LUA_NOREF) return;
        if (updateCallbackIndex==LUA_REFNIL) return;

        int error_handler = lua_gettop(L);

        STACK_BASE;

        // get callback
        lua_rawgeti(L,LUA_REGISTRYINDEX,updateCallbackIndex);

        // args
        const btVector3 &pos = current_xform.getOrigin();
        btQuaternion quat;
        current_xform.getBasis().getRotation(quat);
        Ogre::Real x=pos.x(), y=pos.y(), z=pos.z();
        Ogre::Real qw=quat.w(), qx=quat.x(), qy=quat.y(), qz=quat.z();
        if (isnan(x) || isnan(y) || isnan(z) || isnan(qw) || isnan(qx) || isnan(qy) || isnan(qz)) {
                CERR << "NaN from physics engine." << std::endl;
                x = 0; y = 0; z = 0;
                qw = 1; qx = 0; qy = 0; qz = 0;
                // TODO: move the object back into the real world
        }
        lua_pushnumber(L,x); // arg 1
        lua_pushnumber(L,y); // arg 2
        lua_pushnumber(L,z); // arg 3
        lua_pushnumber(L,qw); // arg 4
        lua_pushnumber(L,qx); // arg 5
        lua_pushnumber(L,qy); // arg 6
        lua_pushnumber(L,qz); // arg 7

        // call callback (7 args, no return values)
        int status = lua_pcall(L,7,0,error_handler);
        if (status) {
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                luaL_unref(L,LUA_REGISTRYINDEX,updateCallbackIndex);
                updateCallbackIndex = LUA_NOREF;
        }

        STACK_CHECK;
}

void RigidBody::stepCallback (lua_State *L)
{
        if (stepCallbackIndex==LUA_NOREF) return;
        if (stepCallbackIndex==LUA_REFNIL) return;

        STACK_BASE;

        // to handle errors raised by the lua callback
        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        // get callback
        lua_rawgeti(L,LUA_REGISTRYINDEX,stepCallbackIndex);

        // call callback (no args, no return values)
        int status = lua_pcall(L,0,0,error_handler);
        if (status) {
                lua_pop(L,1);
                luaL_unref(L,LUA_REGISTRYINDEX,stepCallbackIndex);
                stepCallbackIndex = LUA_NOREF;
        }

        lua_pop(L,1); // error handler

        STACK_CHECK;
}

void RigidBody::stabiliseCallback (lua_State *L)
{
        if (stabiliseCallbackIndex==LUA_NOREF) return;
        if (stabiliseCallbackIndex==LUA_REFNIL) return;

        STACK_BASE;

        // to handle errors raised by the lua callback
        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        // get callback
        lua_rawgeti(L,LUA_REGISTRYINDEX,stabiliseCallbackIndex);

        // call callback (no args, no return values)
        int status = lua_pcall(L,0,0,error_handler);
        if (status) {
                lua_pop(L,1);
                luaL_unref(L,LUA_REGISTRYINDEX,stabiliseCallbackIndex);
                stabiliseCallbackIndex = LUA_NOREF;
        }

        lua_pop(L,1); // error handler

        STACK_CHECK;
}

void RigidBody::force (const Ogre::Vector3 &force)
{
        if (body==NULL) return;
        body->applyCentralImpulse(to_bullet(force*world->getStepSize()));
        body->activate();
}

void RigidBody::force (const Ogre::Vector3 &force,
                       const Ogre::Vector3 &rel_pos)
{
        if (body==NULL) return;
        body->applyImpulse(to_bullet(force*world->getStepSize()),
                           to_bullet(rel_pos));
        body->activate();
}

void RigidBody::impulse (const Ogre::Vector3 &impulse)
{
        if (body==NULL) return;
        body->applyCentralImpulse(to_bullet(impulse));
        body->activate();
}

void RigidBody::impulse (const Ogre::Vector3 &impulse,
                         const Ogre::Vector3 &rel_pos)
{
        if (body==NULL) return;
        body->applyImpulse(to_bullet(impulse),to_bullet(rel_pos));
        body->activate();
}

void RigidBody::torque (const Ogre::Vector3 &torque)
{
        if (body==NULL) return;
        body->applyTorqueImpulse(to_bullet(torque*world->getStepSize()));
        body->activate();
}

void RigidBody::torqueImpulse (const Ogre::Vector3 &torque)
{
        if (body==NULL) return;
        body->applyTorqueImpulse(to_bullet(torque));
        body->activate();
}

Ogre::Real RigidBody::getLinearDamping (void) const
{
        if (body==NULL) return 0;
        return body->getLinearDamping();
}

void RigidBody::setLinearDamping (Ogre::Real r)
{
        if (body==NULL) return;
        body->setDamping(r,getAngularDamping());
}

Ogre::Real RigidBody::getAngularDamping (void) const
{
        if (body==NULL) return 0;
        return body->getAngularDamping();
}

void RigidBody::setAngularDamping (Ogre::Real r)
{
        if (body==NULL) return;
        body->setDamping(getLinearDamping(),r);
}


Ogre::Real RigidBody::getLinearSleepThreshold (void) const
{
        if (body==NULL) return 0;
        return body->getLinearSleepingThreshold();
}

void RigidBody::setLinearSleepThreshold (Ogre::Real r)
{
        if (body==NULL) return;
        body->setSleepingThresholds(r,getAngularSleepThreshold());
}

Ogre::Real RigidBody::getAngularSleepThreshold (void) const
{
        if (body==NULL) return 0;
        return body->getAngularSleepingThreshold();
}

void RigidBody::setAngularSleepThreshold (Ogre::Real r)
{
        if (body==NULL) return;
        body->setSleepingThresholds(getLinearSleepThreshold(),r);
}


void RigidBody::activate (void)
{
        if (body==NULL) return;
        body->activate();
}

void RigidBody::deactivate (void)
{
        body->setActivationState( WANTS_DEACTIVATION );
}

Ogre::Vector3 RigidBody::getLinearVelocity (void) const
{
        if (body==NULL) return Ogre::Vector3(0,0,0);
        return to_ogre(body->getLinearVelocity());
}

void RigidBody::setLinearVelocity (const Ogre::Vector3 &v)
{
        if (body==NULL) return;
        body->setLinearVelocity(to_bullet(v));
        body->activate();
}

Ogre::Vector3 RigidBody::getAngularVelocity (void) const
{
        if (body==NULL) return Ogre::Vector3(0,0,0);
        return to_ogre(body->getAngularVelocity());
}

Ogre::Vector3 RigidBody::getLocalVelocity (const Ogre::Vector3 &v) const
{
        if (body==NULL) return Ogre::Vector3(0,0,0);
        return to_ogre(body->getVelocityInLocalPoint(to_bullet(v)));
}

void RigidBody::setAngularVelocity (const Ogre::Vector3 &v)
{
        if (body==NULL) return;
        body->setAngularVelocity(to_bullet(v));
        body->activate();
}

Ogre::Real RigidBody::getFriction (void) const
{
        if (body==NULL) return 0;
        return body->getFriction();
}

void RigidBody::setFriction (Ogre::Real r)
{
        if (body==NULL) return;
        body->setFriction(r);
        body->activate();
}

Ogre::Real RigidBody::getRestitution (void) const
{
        if (body==NULL) return 0;
        return body->getRestitution();
}

void RigidBody::setRestitution (Ogre::Real r)
{
        if (body==NULL) return;
        body->setRestitution(r);
}

static inline Ogre::Real invert0 (Ogre::Real v)
{
        return v==0 ? 0 : 1/v;
}

static inline Ogre::Vector3 invert0 (const Ogre::Vector3 &v)
{
        return Ogre::Vector3(invert0(v.x),invert0(v.y),invert0(v.z));
}

Ogre::Real RigidBody::getMass (void) const
{
        if (body==NULL) return 0;
        return invert0(body->getInvMass());
}

void RigidBody::setMass (Ogre::Real r)
{
        if (body==NULL) return;
        body->setMassProps(r,to_bullet(getInertia()));
}


Ogre::Vector3 RigidBody::getInertia (void) const
{
        if (body==NULL) return Ogre::Vector3(0,0,0);
        return invert0(to_ogre(body->getInvInertiaDiagLocal()));
}

void RigidBody::setInertia (const Ogre::Vector3 &v)
{
        if (body==NULL) return;
        body->setMassProps(getMass(),to_bullet(v));
}

// vim: shiftwidth=8:tabstop=8:expandtab
