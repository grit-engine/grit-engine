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

//#include <GIMPACT/Bullet/btGImpactShape.h>
//#include <GIMPACT/Bullet/btGImpactCollisionAlgorithm.h>

#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>

#include "PhysicsWorld.h"
#include "GritObject.h"
#include "main.h"
#include "lua_wrappers_physics.h"
#include "CentralisedLog.h"


void DynamicsWorld::step (void)
{
        if (!dirty) {
                saveKinematicState(stepSize);
                applyGravity();
                dirty = true;
        }
        internalSingleStepSimulation(stepSize);
}


void DynamicsWorld::end ()
{
        dirty = false;
        clearForces();
}

bool physics_verbose_contacts = false;

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

/* bullet does not support putting a gimpact triangle mesh in a compound shape so I am hacking the parent in this case to be the triangle mesh instead of the compound shape
*/
static void fix_parent (const btCollisionShape *shape, const btCollisionShape *&parent)
{
        APP_ASSERT(shape!=NULL);
        APP_ASSERT(parent!=NULL);
        if (shape->getShapeType()!=TRIANGLE_SHAPE_PROXYTYPE) return;
        if (parent->getShapeType()!=COMPOUND_SHAPE_PROXYTYPE) return;
        // iterate through children of compound shape searching for the gimpact shape

        // assume there is exactly 1 gimpact shape in the compound if the child shape
        // is a triangle

        const btCompoundShape *compound = static_cast<const btCompoundShape*>(parent);
        const btCollisionShape *new_parent = NULL;

        for (int i=0,i_=compound->getNumChildShapes() ; i<i_ ; ++i) {
                const btCollisionShape *child = compound->getChildShape(i);
                if (child->getShapeType()==GIMPACT_SHAPE_PROXYTYPE) {
                        new_parent = child;
                        break;
                }
                if (child->getShapeType()==TRIANGLE_MESH_SHAPE_PROXYTYPE) {
                        new_parent = child;
                        break;
                }
        }

        // this can happen with static triangle meshes in a compound
        //if (new_parent==NULL) return;
        APP_ASSERT(new_parent!=NULL);

        parent = new_parent;
}

int get_material (const CollisionMeshPtr &cmesh, const btCollisionShape *shape,
                          int part, int id, bool *err, bool verb)
{
        (void) part;
        // * when one gimpact shape hits another (not in compounds), we don't get the triangle
        // we get the whole gimpact shape for some reason
        // * when casting rays, we get the whole shape in the case of static meshes
        if (shape->getShapeType()==TRIANGLE_SHAPE_PROXYTYPE
            || shape->getShapeType()==GIMPACT_SHAPE_PROXYTYPE
            || shape->getShapeType()==TRIANGLE_MESH_SHAPE_PROXYTYPE) {
                int max = cmesh->faceMaterials.size();
                if (id < 0 || id >= max) {
                        if (verb) {
                                CERR << "index from bullet was garbage: " << id
                                     << " >= " << max << std::endl;
                                if (err) *err = true;
                        }
                        id = 0;
                }
                return cmesh->getMaterialFromFace(id);
        } else {
                int max = cmesh->partMaterials.size();
                if (id < 0 || id >= max) {
                        if (verb) {
                                CERR << "index from bullet was garbage: " << id
                                     << " >= " << max << std::endl;
                                if (err) *err = true;
                        }
                        id = 0;
                }
                return cmesh->getMaterialFromPart(id);
        }
}

void get_shape_and_parent(const btCollisionObject* colObj,
                          const btCollisionShape *&shape, const btCollisionShape *&parent)
{
        shape  = colObj->getCollisionShape();
        parent = colObj->getRootCollisionShape();
        fix_parent(shape, parent);
        APP_ASSERT(shape!=NULL);
        APP_ASSERT(parent!=NULL);
}

bool contact_added_callback (btManifoldPoint& cp,
                             const btCollisionObject* colObj0, int part0, int index0,
                             const btCollisionObject* colObj1, int part1, int index1)
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

        CollisionMeshPtr cmesh0 = body0->colMesh, cmesh1 = body1->colMesh;

        const btCollisionShape *shape0, *parent0, *shape1, *parent1;

        get_shape_and_parent(colObj0, shape0, parent0);
        get_shape_and_parent(colObj1, shape1, parent1);

        bool err = false;
        bool verb = world->errorContacts;

        int mat0 = get_material(cmesh0, shape0, part0, index0, &err, verb);
        int mat1 = get_material(cmesh1, shape1, part1, index1, &err, verb);

        world->getFrictionRestitution(mat0, mat1, cp.m_combinedFriction, cp.m_combinedRestitution);

        if (err || world->verboseContacts) {
                CLOG << mat0 << "[" << shape_str(shape0->getShapeType()) << "]"
                     << "(" << shape_str(parent0->getShapeType()) << ")"
                     << " " << part0 << " " << index0
                     << "  AGAINST  " << mat1 << "[" << shape_str(shape1->getShapeType()) << "]"
                     << "(" << shape_str(parent1->getShapeType()) << ")"
                     << " " << part1 << " " << index1 << std::endl;
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
        
        if (parent0->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE ||
            parent1->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE) {

                const btRigidBody *sta_body;
                const btRigidBody *dyn_body;

                if (parent0->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE) {
                        sta_body = bbody0;
                        dyn_body = bbody1;
                } else {
                        dyn_body = bbody0;
                        sta_body = bbody1;
                }


                if (world->useTriangleEdgeInfo) {
                        btAdjustInternalEdgeContacts(cp,sta_body, dyn_body, part1,index1);
                        if (world->verboseContacts) {
                                CLOG << cp.m_lifeTime << " " << cp.m_positionWorldOnA
                                                      << " " << cp.m_positionWorldOnB
                                     << " " << cp.m_normalWorldOnB << " " << cp.m_distance1
                                     << " *" << cp.m_appliedImpulse << "* |" << cp.m_combinedFriction
                                     << "| >" << cp.m_combinedRestitution << "<   (CORRECTION)" << std::endl;
                                /*
                                bool    m_lateralFrictionInitialized
                                btScalar        m_appliedImpulseLateral1
                                btScalar        m_appliedImpulseLateral2
                                btVector3       m_lateralFrictionDir1
                                btVector3       m_lateralFrictionDir2
                                */
                        }
                }

                if (world->bumpyTriangleMeshHack) {
                        const btTriangleShape *tshape =
                               static_cast<const btTriangleShape*>(sta_body->getCollisionShape());

                        btVector3 normal;
                        tshape->calcNormal(normal);

                        normal = sta_body->getWorldTransform().getBasis() * normal;

                        btScalar dot = normal.dot(cp.m_normalWorldOnB);

                        btScalar magnitude = cp.m_normalWorldOnB.length();
                        normal *= dot > 0 ? magnitude : -magnitude;

                        cp.m_normalWorldOnB = normal;

                        // Reproject collision point along normal.
                        cp.m_positionWorldOnB =
                                cp.m_positionWorldOnA - cp.m_normalWorldOnB * cp.m_distance1;
                        cp.m_localPointB =
                                sta_body->getWorldTransform().invXform(cp.m_positionWorldOnB);
                }

        }

        if (parent0->getShapeType() == GIMPACT_SHAPE_PROXYTYPE ||
            parent1->getShapeType() == GIMPACT_SHAPE_PROXYTYPE) {

                const btRigidBody *gim_body;

                if (parent0->getShapeType() == GIMPACT_SHAPE_PROXYTYPE) {
                        gim_body = bbody0;
                } else {
                        gim_body = bbody1;
                }

                if (world->gimpactOneWayMeshHack) {
                        const btTriangleShape *tshape =
                               static_cast<const btTriangleShape*>(gim_body->getCollisionShape());

                        btVector3 normal;
                        tshape->calcNormal(normal);

                        normal = gim_body->getWorldTransform().getBasis() * normal;

                        btScalar dot = normal.dot(cp.m_normalWorldOnB) * (gim_body == bbody0 ? 1 : -1);

                        if (dot > 0) {
                                cp.m_normalWorldOnB -= 2 * dot * normal;
                        }
                }
        }

        return true;
}

extern ContactAddedCallback gContactAddedCallback;

PhysicsWorld::PhysicsWorld (const Vector3 &bounds_min, const Vector3 &bounds_max)
      : verboseContacts(false), errorContacts(true), verboseCasts(false), errorCasts(true),
        bumpyTriangleMeshHack(false), useTriangleEdgeInfo(true),
        gimpactOneWayMeshHack(true), needsGraphicsUpdate(false), maxSteps(5), extrapolate(0)
{
        colConf = new btDefaultCollisionConfiguration();
        colDisp = new btCollisionDispatcher(colConf);

        // first 2 params specify the world AABB
        // last param is the max number of "proxies"
        broadphase = new btAxisSweep3(to_bullet(bounds_min),
                                      to_bullet(bounds_max),
                                      32766);

        conSolver = new btSequentialImpulseConstraintSolver();
        //conSolver = new btOdeQuickstepConstraintSolver();

        world = new DynamicsWorld(colDisp,broadphase,conSolver,colConf);


        btGImpactCollisionAlgorithm::registerAlgorithm(colDisp);

        gContactAddedCallback = contact_added_callback;
        
        world->setGravity(btVector3(0.0f,0.0f,-9.8f));

        world->setDebugDrawer(debug_drawer);
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

int PhysicsWorld::pump (lua_State *L, float elapsed)
{
        last_L = L;
        float step_size = world->getStepSize();
        int counter = 0;
        for (; counter<maxSteps ; counter++) {
                if (elapsed<step_size) break;
                elapsed -= step_size;
                world->step();
                needsGraphicsUpdate = true;
                extrapolate = elapsed;
                std::vector<RigidBody*> nan_bodies;
                for (int i=0 ; i<world->getNumCollisionObjects() ; ++i) {

                        btCollisionObject* victim =
                                world->getCollisionObjectArray()[i];
                        btRigidBody* victim2 = btRigidBody::upcast(victim);
                        if (victim2==NULL) continue;
                        RigidBody *rb =
                             static_cast<RigidBody*>(victim2->getMotionState());

                        const btTransform &current_xform = victim2->getWorldTransform();
                        const btVector3 &pos = current_xform.getOrigin();
                        btQuaternion quat;
                        current_xform.getBasis().getRotation(quat);
                        float x=pos.x(), y=pos.y(), z=pos.z();
                        float qw=quat.w(), qx=quat.x(), qy=quat.y(), qz=quat.z();
                        if (isnan(x) || isnan(y) || isnan(z) || isnan(qw) || isnan(qx) || isnan(qy) || isnan(qz)) {
                                CERR << "NaN from physics engine position update." << std::endl;
                                nan_bodies.push_back(rb);
                        } else {
                                rb->stepCallback(L);
                        }
                }
                for (unsigned int i=0 ; i<nan_bodies.size() ; ++i) {
                        nan_bodies[i]->removeFromWorld();
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
        world->end();
        return counter;
}

void PhysicsWorld::updateGraphics (lua_State *L)
{
        if (!needsGraphicsUpdate) return;
        needsGraphicsUpdate = false;

        // to handle errors raised by the lua callback
        push_cfunction(L, my_lua_error_handler);

        for (int i=0 ; i<world->getNumCollisionObjects() ; ++i) {
                btCollisionObject* victim = world->getCollisionObjectArray()[i];
                btRigidBody* victim2 = btRigidBody::upcast(victim);
                if (victim2==NULL) continue;
                RigidBody *rb = static_cast<RigidBody*>(victim2->getMotionState());
                rb->updateGraphicsCallback(L);
        }

        lua_pop(L,1); // error handler
}

float PhysicsWorld::getDeactivationTime (void) const
{ return gDeactivationTime; }
void PhysicsWorld::setDeactivationTime (float v)
{ gDeactivationTime = v; }

float PhysicsWorld::getContactBreakingThreshold (void) const
{ return gContactBreakingThreshold; }
void PhysicsWorld::setContactBreakingThreshold (float v)
{ gContactBreakingThreshold = v; }

Vector3 PhysicsWorld::getGravity (void) const
{ return from_bullet(world->getGravity()); }
void PhysicsWorld::setGravity (const Vector3 &gravity)
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


CollisionMeshPtr PhysicsWorld::createFromFile (const std::string &name)
{
        if (hasMesh(name))
                GRIT_EXCEPT("Collision mesh \""+name+"\" already exists.");
        CollisionMeshPtr cmp = CollisionMeshPtr(new CollisionMesh(name));
        // Note: this only works because both ogre and bullet are using float
        // if this situation changes it will be necessary to convert
        // from float to btScalar.
        Ogre::DataStreamPtr file =
                Ogre::ResourceGroupManager::getSingleton().openResource(name,"GRIT");
        cmp->importFromFile(file, mdb);
        colMeshes[name] = cmp;
        return cmp;
}

void PhysicsWorld::deleteMesh (const std::string &name)
{
        CollisionMeshMap::iterator i = colMeshes.find(name);
        if (i==colMeshes.end())
                GRIT_EXCEPT("Collision mesh \""+name+"\" doesn't exist.");
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
                APP_ASSERT(r.m_localShapeInfo!=NULL);
                int part, index;
                bool err = false;
                if (r.m_localShapeInfo) {
                        part = r.m_localShapeInfo->m_shapePart;
                        index = r.m_localShapeInfo->m_triangleIndex;
                } else {
                        err = true;
                        part = 0;
                        index = 0;
                }

                const btCollisionShape *shape, *parent;
                get_shape_and_parent(body, shape, parent);

                bool verb = rb->world->errorCasts;
                int m = get_material(rb->colMesh, shape, part, index, &err, verb);

                if (err || rb->world->verboseCasts) {
                        CLOG << "RAY HIT  " << m << "[" << shape_str(shape->getShapeType()) << "]"
                             << "(" << shape_str(parent->getShapeType()) << ")"
                             << " " << part << " " << index << std::endl;
                        CLOG << r.m_hitFraction << " " << r.m_hitNormalLocal << std::endl;
                }


                scb.result(*rb, r.m_hitFraction, from_bullet(r.m_hitNormalLocal), m);
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
                APP_ASSERT(r.m_localShapeInfo!=NULL);
                int part, index;
                bool err = false;
                if (r.m_localShapeInfo) {
                        part = r.m_localShapeInfo->m_shapePart;
                        index = r.m_localShapeInfo->m_triangleIndex;
                } else {
                        part = 0;
                        index = 0;
                        err = true;
                }

                const btCollisionShape *shape, *parent;
                get_shape_and_parent(body, shape, parent);

                bool verb = rb->world->errorCasts;
                int m = get_material(rb->colMesh, shape, part, index, &err, verb);

                if (err || rb->world->verboseCasts) {
                        CLOG << "SWEEP HIT  " << m << "[" << shape_str(shape->getShapeType()) << "]"
                             << "(" << shape_str(parent->getShapeType()) << ")"
                             << " " << part << " " << index << std::endl;
                        CLOG << r.m_hitFraction << " " << r.m_hitNormalLocal << std::endl;
                }
                scb.result(*rb, r.m_hitFraction, from_bullet(r.m_hitNormalLocal), m);
                return r.m_hitFraction;
        }
    protected:
        PhysicsWorld::SweepCallback &scb;
};

void PhysicsWorld::ray (const Vector3 &start,
                        const Vector3 &end,
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
                          const Vector3 &startp,
                          const Quaternion &startq,
                          const Vector3 &endp,
                          const Quaternion &endq,
                          SweepCallback &scb) const
{
        BulletSweepCallback bscb(scb);
        btTransform start(to_bullet(startq),to_bullet(startp));
        btTransform end(to_bullet(endq),to_bullet(endp));
        btConvexShape *conv =dynamic_cast<btConvexShape*>(col_mesh->getMasterShape());
        if (conv==NULL) return;
        world->convexSweepTest(conv,start,end,bscb);
}

CollisionMeshMap PhysicsWorld::colMeshes;



RigidBody::RigidBody (const PhysicsWorldPtr &world_,
                      const CollisionMeshPtr &col_mesh,
                      const Vector3 &pos,
                      const Quaternion &quat)
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
        // the object will not be destructed until after its own destructor...
        // We instead want the object to print a warning if it is destructed
        // without first being destroy()ed.
        // RigidBody must only be constructed as follows:
        // RigidBodyPtr blah = (new RigidBody(...))->getPtr()
        self.useCount()--;
}


static btCompoundShape *clone_compound (btCompoundShape *master)
{
        btCompoundShape *slave = new btCompoundShape();
        for (int i=0 ; i<master->getNumChildShapes() ; ++i) {
                btCollisionShape *s = master->getChildShape(i);
                btTransform &t = master->getChildTransform(i);
                slave->addChildShape(t,s);
        }
        return slave;
}

/* The intention is that the object is always in the world.  The exceptions are when it is created,
 * after it has been destroyed, and in the brief moments when it is pulled out and put back in again
 * in response to a CollisionMesh reload. */

void RigidBody::addToWorld (void)
{
        shape = clone_compound(colMesh->getMasterShape());
        localChanges.resize(shape->getNumChildShapes());
        // by default, turn everything on, leave it transformed as found in the master copy
        for (int i=0 ; i<localChanges.size() ; ++i) {
                localChanges[i].enabled = true;
                localChanges[i].offset.setIdentity();
        }

        btRigidBody::btRigidBodyConstructionInfo
                info(colMesh->getMass(), this, shape, to_bullet(colMesh->getInertia()));

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
        delete shape;
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
        self.useCount()++;
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

void RigidBody::setWorldTransform (const btTransform& )
{
}

void RigidBody::updateGraphicsCallback (lua_State *L)
{
        if (updateCallbackIndex==LUA_NOREF) return;
        if (updateCallbackIndex==LUA_REFNIL) return;

        btTransform current_xform;

        btTransformUtil::integrateTransform(
                body->getInterpolationWorldTransform(),
                body->getInterpolationLinearVelocity(),
                body->getInterpolationAngularVelocity(),
                world->extrapolate*body->getHitFraction(),
                current_xform);


        STACK_BASE;

        // args
        const btVector3 &pos = check_nan(current_xform.getOrigin());
        btQuaternion quat;
        current_xform.getBasis().getRotation(quat);
        quat = check_nan(quat);
        float x=pos.x(), y=pos.y(), z=pos.z();
        float qw=quat.w(), qx=quat.x(), qy=quat.y(), qz=quat.z();

        int error_handler = lua_gettop(L);

        // get callback
        lua_rawgeti(L,LUA_REGISTRYINDEX,updateCallbackIndex);

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
        if (body->getInvMass()==0) {
                btTransform after;
                btTransformUtil::integrateTransform(body->getCenterOfMassTransform(),
                                                    body->getLinearVelocity(),
                                                    body->getInterpolationAngularVelocity(),
                                                    world->getStepSize(),
                                                    after);
                body->proceedToTransform(after);
        }
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

void RigidBody::force (const Vector3 &force)
{
        if (body==NULL) return;
        body->applyCentralImpulse(to_bullet(force*world->getStepSize()));
        body->activate();
}

void RigidBody::force (const Vector3 &force,
                       const Vector3 &rel_pos)
{
        if (body==NULL) return;
        body->applyImpulse(to_bullet(force*world->getStepSize()),
                           to_bullet(rel_pos));
        body->activate();
}

void RigidBody::impulse (const Vector3 &impulse)
{
        if (body==NULL) return;
        body->applyCentralImpulse(to_bullet(impulse));
        body->activate();
}

void RigidBody::impulse (const Vector3 &impulse,
                         const Vector3 &rel_pos)
{
        if (body==NULL) return;
        body->applyImpulse(to_bullet(impulse),to_bullet(rel_pos));
        body->activate();
}

void RigidBody::torque (const Vector3 &torque)
{
        if (body==NULL) return;
        body->applyTorqueImpulse(to_bullet(torque*world->getStepSize()));
        body->activate();
}

void RigidBody::torqueImpulse (const Vector3 &torque)
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

Vector3 RigidBody::getLinearVelocity (void) const
{
        if (body==NULL) return Vector3(0,0,0);
        return from_bullet(body->getLinearVelocity());
}

void RigidBody::setLinearVelocity (const Vector3 &v)
{
        if (body==NULL) return;
        body->setLinearVelocity(to_bullet(v));
        body->activate();
}

Vector3 RigidBody::getAngularVelocity (void) const
{
        if (body==NULL) return Vector3(0,0,0);
        return from_bullet(body->getAngularVelocity());
}

Vector3 RigidBody::getLocalVelocity (const Vector3 &v) const
{
        if (body==NULL) return Vector3(0,0,0);
        return from_bullet(body->getVelocityInLocalPoint(to_bullet(v)));
}

void RigidBody::setAngularVelocity (const Vector3 &v)
{
        if (body==NULL) return;
        body->setAngularVelocity(to_bullet(v));
        body->activate();
}

static inline float invert0 (float v)
{
        return v==0 ? 0 : 1/v;
}

static inline Vector3 invert0 (const Vector3 &v)
{
        return Vector3(invert0(v.x),invert0(v.y),invert0(v.z));
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


Vector3 RigidBody::getInertia (void) const
{
        if (body==NULL) return Vector3(0,0,0);
        return invert0(from_bullet(body->getInvInertiaDiagLocal()));
}

void RigidBody::setInertia (const Vector3 &v)
{
        if (body==NULL) return;
        body->setMassProps(getMass(),to_bullet(v));
}

static int get_child_index(btCompoundShape* c, btCollisionShape* shape)
{
        for (int i=0 ; i<c->getNumChildShapes() ; ++i) {
                if(c->getChildShape(i) == shape)
                        return i;
        }
        CERR << "Could not find child shape in compound." << std::endl;
        return 0;
}


void RigidBody::setElementEnabled (int i, bool v)
{
        if (getElementEnabled(i)==v) return;
        localChanges[i].enabled = v;
        if (v) {
                btCollisionShape *s = colMesh->getMasterShape()->getChildShape(i);
                btTransform &t = colMesh->getMasterShape()->getChildTransform(i);
                t = t * localChanges[i].offset;
                shape->addChildShape(t,s);
        } else {
                int i2 = get_child_index(shape, colMesh->getMasterShape()->getChildShape(i));
                world->world->getBroadphase()->getOverlappingPairCache()
                        ->cleanProxyFromPairs(body->getBroadphaseHandle(), world->world->getDispatcher());
                shape->removeChildShapeByIndex(i2);
        }
}

bool RigidBody::getElementEnabled (int i)
{
        APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
        return localChanges[i].enabled;
}

Vector3 RigidBody::getElementPositionMaster (int i)
{
        APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
        return from_bullet(colMesh->getMasterShape()->getChildTransform(i).getOrigin());
}

void RigidBody::setElementPositionOffset (int i, const Vector3 &v)
{
        APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
        localChanges[i].offset.setOrigin(to_bullet(v));
        if (localChanges[i].enabled) {
                int i2 = get_child_index(shape, colMesh->getMasterShape()->getChildShape(i));
                shape->updateChildTransform(i2, colMesh->getMasterShape()->getChildTransform(i) * localChanges[i].offset);
        }
}

Vector3 RigidBody::getElementPositionOffset (int i)
{
        APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
        return from_bullet(localChanges[i].offset.getOrigin());
}

Quaternion RigidBody::getElementOrientationMaster (int i)
{
        APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
        return from_bullet(colMesh->getMasterShape()->getChildTransform(i).getRotation());
}

void RigidBody::setElementOrientationOffset (int i, const Quaternion &q)
{
        APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
        localChanges[i].offset.setRotation(to_bullet(q));
        if (localChanges[i].enabled) {
                int i2 = get_child_index(shape, colMesh->getMasterShape()->getChildShape(i));
                shape->updateChildTransform(i2, colMesh->getMasterShape()->getChildTransform(i) * localChanges[i].offset);
        }
}

Quaternion RigidBody::getElementOrientationOffset (int i)
{
        APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
        return from_bullet(localChanges[i].offset.getRotation());
}

// vim: shiftwidth=8:tabstop=8:expandtab:tw=100
