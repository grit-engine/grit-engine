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

bool physics_verbose_contacts = false;

void process_contact (btManifoldPoint& cp, const PhysicsWorldPtr &world,
                      const btCollisionObject *colObj, bool gimpact)
{
        const btCollisionShape *shape = colObj->getCollisionShape();

        if (shape->getShapeType() != TRIANGLE_SHAPE_PROXYTYPE) return;
        const btTriangleShape *tshape =
               static_cast<const btTriangleShape*>(colObj->getCollisionShape());


        const btCollisionShape *parent = colObj->getRootCollisionShape();
        if (parent == NULL) return;

        btVector3 normal;
        tshape->calcNormal(normal);

        normal = colObj->getWorldTransform().getBasis() * normal;

        btScalar dot = normal.dot(cp.m_normalWorldOnB);


        if (gimpact) {
                if (parent->getShapeType()==GIMPACT_SHAPE_PROXYTYPE &&
                    world->gimpactOneWayMeshHack) {
                        if (dot > 0) {
                                cp.m_normalWorldOnB -= 2 * dot * normal;
                        }
                }
        } else {
                if (parent->getShapeType()==TRIANGLE_MESH_SHAPE_PROXYTYPE &&
                    world->bumpyTriangleMeshHack) {

                        btScalar magnitude = cp.m_normalWorldOnB.length();
                        normal *= dot > 0 ? magnitude : -magnitude;

                        cp.m_normalWorldOnB = normal;

                        // Reproject collision point along normal.
                        cp.m_positionWorldOnB =
                                cp.m_positionWorldOnA - cp.m_normalWorldOnB * cp.m_distance1;
                        cp.m_localPointB =
                                colObj->getWorldTransform().invXform(cp.m_positionWorldOnB);

                }
        }
}

static std::ostream &operator << (std::ostream &o, const btVector3 &v)
{
        return o << "("<<v.x()<<", "<<v.y()<<", "<<v.z()<<")";
}

static std::string shape_str (int s)
{
        switch (s) {
                case BOX_SHAPE_PROXYTYPE: return "box";
                case TRIANGLE_SHAPE_PROXYTYPE: return "tri";
                case CONVEX_HULL_SHAPE_PROXYTYPE: return "hul";
                case SPHERE_SHAPE_PROXYTYPE: return "sph";
                case CAPSULE_SHAPE_PROXYTYPE: return "cap";
                case CONE_SHAPE_PROXYTYPE: return "con";
                case CYLINDER_SHAPE_PROXYTYPE: return "cyl";
                case TRIANGLE_MESH_SHAPE_PROXYTYPE: return "sta";
                case GIMPACT_SHAPE_PROXYTYPE: return "gim";
                case STATIC_PLANE_PROXYTYPE: return "pla";
                case COMPOUND_SHAPE_PROXYTYPE: return "com";
                default:
                std::stringstream ss;
                ss << s;
                return ss.str();
        }
        return "";
}

bool contact_added_callback (btManifoldPoint& cp,
                             const btCollisionObject* colObj0, int, int index0,
                             const btCollisionObject* colObj1, int, int index1)
{
        const btRigidBody *bbody0 = dynamic_cast<const btRigidBody*>(colObj0);
        const btRigidBody *bbody1 = dynamic_cast<const btRigidBody*>(colObj1);
        APP_ASSERT(bbody0!=NULL);
        APP_ASSERT(bbody1!=NULL);

        const RigidBody *body0 = static_cast<const RigidBody*>(bbody0->getMotionState());
        const RigidBody *body1 = static_cast<const RigidBody*>(bbody1->getMotionState());
        APP_ASSERT(body0!=NULL);
        APP_ASSERT(body1!=NULL);

        const PhysicsWorldPtr &world = body0->world;

        CollisionMeshPtr cmesh0 = body0->colMesh;
        CollisionMeshPtr cmesh1 = body1->colMesh;

        const btCollisionShape *shape0  = colObj0->getCollisionShape();
        const btCollisionShape *parent0 = colObj0->getRootCollisionShape();
        const btCollisionShape *shape1  = colObj1->getCollisionShape();
        const btCollisionShape *parent1 = colObj1->getRootCollisionShape();

        APP_ASSERT(parent0!=NULL);
        APP_ASSERT(parent1!=NULL);

        bool shit_has_hit_fan = false;

        physics_mat mat0, mat1;

        if (shape0->getShapeType()==TRIANGLE_SHAPE_PROXYTYPE ||
            shape0->getShapeType()==GIMPACT_SHAPE_PROXYTYPE) {
                int max = cmesh0->faceMaterials.size();
                int id = index0;
                if (id < 0 || id >= max) {
                        if (world->errorContacts) {
                                CERR << "index from bullet was garbage: " << id
                                     << " >= " << max << std::endl;
                                shit_has_hit_fan = true;
                        }
                        id = 0;
                }
                mat0 = cmesh0->getMaterialFromFace(id);
        } else {
                int max = cmesh0->partMaterials.size();
                int id = index0;
                //if (id==-1) id = 0;
                if (id < 0 || id >= max) {
                        if (world->errorContacts) {
                                CERR << "index from bullet was garbage: " << id
                                     << " >= " << max << std::endl;
                                shit_has_hit_fan = true;
                        }
                        id = 0;
                }
                mat0 = cmesh0->getMaterialFromPart(id);
        }

        if (shape1->getShapeType()==TRIANGLE_SHAPE_PROXYTYPE ||
            shape1->getShapeType()==GIMPACT_SHAPE_PROXYTYPE) {
                int max = cmesh1->faceMaterials.size();
                int id = index1;
                if (id < 0 || id >= max) {
                        if (world->errorContacts) {
                                CERR << "index from bullet was garbage: " << id
                                     << " >= " << max << std::endl;
                                shit_has_hit_fan = true;
                        }
                        id = 0;
                }
                mat1 = cmesh1->getMaterialFromFace(id);
        } else {
                int max = cmesh1->partMaterials.size();
                int id = index1;
                //if (id==-1) id = 0;
                if (id < 0 || id >= max) {
                        if (world->errorContacts) {
                                CERR << "index from bullet was garbage: " << id
                                     << " >= " << max << std::endl;
                                shit_has_hit_fan = true;
                        }
                        id = 0;
                }
                mat1 = cmesh1->getMaterialFromPart(id);
        }

        world->getInteraction(mat0, mat1, cp.m_combinedFriction, cp.m_combinedRestitution);

        if (shit_has_hit_fan || world->verboseContacts) {
                CLOG << mat0 << "[" << shape_str(shape0->getShapeType()) << "]"
                     << "(" << shape_str(parent0->getShapeType()) << ")"
                     << " " << index0 << " " << index0
                     << "  AGAINST  " << mat1 << "[" << shape_str(shape1->getShapeType()) << "]"
                     << "(" << shape_str(parent1->getShapeType()) << ")"
                     << " " << index1 << " " << index1 << std::endl;
                CLOG << cp.m_lifeTime << " " << cp.m_positionWorldOnA
                                      << " " << cp.m_positionWorldOnB
                     << " " << cp.m_normalWorldOnB << " " << cp.m_distance1
                     << " *" << cp.m_appliedImpulse << "* |" << cp.m_combinedFriction
                     << "| >" << cp.m_combinedRestitution << "<" << std::endl;
                /*
                bool    m_lateralFrictionInitialized
                btScalar        m_appliedImpulseLateral1
                btScalar        m_appliedImpulseLateral2
                btVector3       m_lateralFrictionDir1
                btVector3       m_lateralFrictionDir2
                */
        }
        
        // 0 is always the gimpact?
        // 1 is always the scenery?
        process_contact(cp, world, colObj0, true);
        process_contact(cp, world, colObj1, false);
        return true;
}

extern ContactAddedCallback gContactAddedCallback;

PhysicsWorld::PhysicsWorld (const Ogre::AxisAlignedBox &bounds)
      : verboseContacts(false), errorContacts(true),
        bumpyTriangleMeshHack(true), gimpactOneWayMeshHack(true), maxSteps(5)
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
        float step_size = world->getStepSize();
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

float PhysicsWorld::getDeactivationTime (void) const
{ return gDeactivationTime; }
void PhysicsWorld::setDeactivationTime (float v)
{ gDeactivationTime = v; }

float PhysicsWorld::getContactBreakingThreshold (void) const
{ return gContactBreakingThreshold; }
void PhysicsWorld::setContactBreakingThreshold (float v)
{ gContactBreakingThreshold = v; }

Ogre::Vector3 PhysicsWorld::getGravity (void) const
{ return to_ogre(world->getGravity()); }
void PhysicsWorld::setGravity (const Ogre::Vector3 &gravity)
{ world->setGravity(to_bullet(gravity)); }

float PhysicsWorld::getSolverDamping (void) const
{ return world->getSolverInfo().m_damping; }
void PhysicsWorld::setSolverDamping (float v)
{ world->getSolverInfo().m_damping = v; }

int PhysicsWorld::getSolverIterations (void) const
{ return world->getSolverInfo().m_numIterations; }
void PhysicsWorld::setSolverIterations (int v)
{ world->getSolverInfo().m_numIterations = v; }

float PhysicsWorld::getSolverErp (void) const
{ return world->getSolverInfo().m_erp; } 
void PhysicsWorld::setSolverErp (float v)
{ world->getSolverInfo().m_erp = v; } 

float PhysicsWorld::getSolverErp2 (void) const
{ return world->getSolverInfo().m_erp2; } 
void PhysicsWorld::setSolverErp2 (float v)
{ world->getSolverInfo().m_erp2 = v; } 

float PhysicsWorld::getSolverLinearSlop (void) const
{ return world->getSolverInfo().m_linearSlop; } 
void PhysicsWorld::setSolverLinearSlop (float v)
{ world->getSolverInfo().m_linearSlop = v; } 

float PhysicsWorld::getSolverWarmStartingFactor (void) const
{ return world->getSolverInfo().m_warmstartingFactor; } 
void PhysicsWorld::setSolverWarmStartingFactor (float v)
{ world->getSolverInfo().m_warmstartingFactor = v; } 

bool PhysicsWorld::getSolverSplitImpulse (void) const
{ return 0!=world->getSolverInfo().m_splitImpulse; } 
void PhysicsWorld::setSolverSplitImpulse (bool v)
{ world->getSolverInfo().m_splitImpulse = v; } 

float PhysicsWorld::getSolverSplitImpulseThreshold (void) const
{ return world->getSolverInfo().m_splitImpulsePenetrationThreshold; } 
void PhysicsWorld::setSolverSplitImpulseThreshold (float v)
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
        // from float to btScalar.
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
                        float radius) const
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

        colMesh->registerUser(this);
        body = NULL;
        addToWorld();

        self = RigidBodyPtr(this);
        // Make the self pointer a weak reference, otherwise
        // the object will be kept alive until destroy() is called!
        // We instead want the object to print a warning if it is destructed
        // without first being destroyed.
        // RigidBody must only be constructed as follows:
        // RigidBodyPtr blah = (new RigidBody(...))->getPtr()
        (*self.useCountPointer())--;
}

/* The intention is that the object is always in the world.  The exceptions are
 * when it is created, after it has been destroyed, and in the brief moments
 * when it is pulled out and put back in again in response to a CollisionMesh
 * reload. */

void RigidBody::addToWorld (void)
{
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
        info.m_friction = 0;
        info.m_restitution = 0;
        info.m_linearSleepingThreshold = colMesh->getLinearSleepThreshold();
        info.m_angularSleepingThreshold = colMesh->getAngularSleepThreshold();

        APP_ASSERT(body==NULL);
        body = new btRigidBody(info);

        world->world->addRigidBody(body);

        // Only do CCD if speed of body exceeeds this
        body->setCcdMotionThreshold(colMesh->getCCDMotionThreshold());
        body->setCcdSweptSphereRadius(colMesh->getCCDSweptSphereRadius());

        body->setCollisionFlags(body->getCollisionFlags() |
                                btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}

void RigidBody::removeFromWorld (void)
{
        lastXform = body->getCenterOfMassTransform();
        world->world->removeRigidBody(body);
        delete body;
        body = NULL;
}

void RigidBody::destroy (lua_State *L)
{
        colMesh->unregisterUser(this);
        if (body==NULL) return;
        removeFromWorld();
        luaL_unref(L,LUA_REGISTRYINDEX,updateCallbackIndex);
        luaL_unref(L,LUA_REGISTRYINDEX,stepCallbackIndex);
        // the next line prevents the RigidBody being destructed prematurely
        (*self.useCountPointer())++;
        self.setNull();
}

RigidBody::~RigidBody (void)
{
        colMesh->unregisterUser(this);
        if (body==NULL) return;
        CERR << "destructing RigidBody: destroy() was not called" << std::endl;
        removeFromWorld();
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
        float x=pos.x(), y=pos.y(), z=pos.z();
        float qw=quat.w(), qx=quat.x(), qy=quat.y(), qz=quat.z();
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

float RigidBody::getContactProcessingThreshold (void) const
{
        if (body==NULL) return 0;
        return body->getContactProcessingThreshold();
}

void RigidBody::setContactProcessingThreshold (float r)
{
        if (body==NULL) return;
        body->setContactProcessingThreshold(r);
}

float RigidBody::getLinearDamping (void) const
{
        if (body==NULL) return 0;
        return body->getLinearDamping();
}

void RigidBody::setLinearDamping (float r)
{
        if (body==NULL) return;
        body->setDamping(r,getAngularDamping());
}

float RigidBody::getAngularDamping (void) const
{
        if (body==NULL) return 0;
        return body->getAngularDamping();
}

void RigidBody::setAngularDamping (float r)
{
        if (body==NULL) return;
        body->setDamping(getLinearDamping(),r);
}


float RigidBody::getLinearSleepThreshold (void) const
{
        if (body==NULL) return 0;
        return body->getLinearSleepingThreshold();
}

void RigidBody::setLinearSleepThreshold (float r)
{
        if (body==NULL) return;
        body->setSleepingThresholds(r,getAngularSleepThreshold());
}

float RigidBody::getAngularSleepThreshold (void) const
{
        if (body==NULL) return 0;
        return body->getAngularSleepingThreshold();
}

void RigidBody::setAngularSleepThreshold (float r)
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

static inline float invert0 (float v)
{
        return v==0 ? 0 : 1/v;
}

static inline Ogre::Vector3 invert0 (const Ogre::Vector3 &v)
{
        return Ogre::Vector3(invert0(v.x),invert0(v.y),invert0(v.z));
}

float RigidBody::getMass (void) const
{
        if (body==NULL) return 0;
        return invert0(body->getInvMass());
}

void RigidBody::setMass (float r)
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
