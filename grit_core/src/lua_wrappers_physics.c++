#include <limits>

#include "PhysicsWorld.h"
#include "CollisionMesh.h"

#include "lua_wrappers_primitives.h"
#include "lua_wrappers_physics.h"
#include "lua_wrappers_gritobj.h"

// Sweep Callback {{{

struct SweepResult { RigidBody *rb; float dist; Ogre::Vector3 n; };
typedef std::vector<SweepResult> SweepResults;

class LuaSweepCallback : public PhysicsWorld::SweepCallback {
    public:           
        LuaSweepCallback (bool ignore_dynamic)
              : ignoreDynamic(ignore_dynamic)
        { }
        virtual void result (RigidBody &rb, float dist, Ogre::Vector3 &n) {
                if (ignoreDynamic && rb.getMass()>0) return;
                if (blacklist.find(&rb)!=blacklist.end()) return;
                SweepResult r;
                r.rb = &rb;
                r.dist = dist;
                r.n = n;
                results.push_back(r);
        }
        std::set<RigidBody *> blacklist;
        SweepResults results;
        bool ignoreDynamic;
};

// }}}



// COLMESH ================================================================= {{{

static void push_colmesh (lua_State *L, const CollisionMeshPtr &self)
{
        if (self.isNull())
                lua_pushnil(L);
        else
                push(L,new CollisionMeshPtr(self),COLMESH_TAG);
}

static int colmesh_reload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(CollisionMeshPtr,self,1,COLMESH_TAG);
        self->reload();
        return 0;
TRY_END
}

static int colmesh_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(CollisionMeshPtr,self,1,COLMESH_TAG,0);
        delete self;
        return 0;
TRY_END
}

/*
static int colmesh_impulse (lua_State *L)
{
TRY_START
        GET_UD_MACRO(CollisionMeshPtr,self,1,COLMESH_TAG);
        return 0;
TRY_END
}
*/



TOSTRING_GETNAME_MACRO (colmesh,CollisionMeshPtr,->getName(),COLMESH_TAG)



static int colmesh_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(CollisionMeshPtr,self,1,COLMESH_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="mass") {
                lua_pushnumber(L,self->getMass());
        } else if (key=="reload") {
                push_cfunction(L,colmesh_reload);
        } else if (key=="ccdMotionThreshold") {
                lua_pushnumber(L,self->getCCDMotionThreshold());
        } else if (key=="ccdSweptSphereRadius") {
                lua_pushnumber(L,self->getCCDSweptSphereRadius());
        } else if (key=="linearDamping") {
                lua_pushnumber(L,self->getLinearDamping());
        } else if (key=="angularDamping") {
                lua_pushnumber(L,self->getAngularDamping());
        } else if (key=="linearSleepThreshold") {
                lua_pushnumber(L,self->getLinearSleepThreshold());
        } else if (key=="angularSleepThreshold") {
                lua_pushnumber(L,self->getAngularSleepThreshold());
        } else if (key=="name") {
                lua_pushstring(L,self->getName().c_str());
        } else {
                my_lua_error(L,"Not a readable CollisionMesh member: "+key);
        }
        return 1;
TRY_END
}

static int colmesh_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(CollisionMeshPtr,self,1,COLMESH_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="mass") {
                float v = luaL_checknumber(L,3);
                self->setMass(v);
        } else if (key=="ccdMotionThreshold") {
                float v = luaL_checknumber(L,3);
                self->setCCDMotionThreshold(v);
        } else if (key=="ccdSweptSphereRadius") {
                float v = luaL_checknumber(L,3);
                self->setCCDSweptSphereRadius(v);
        } else if (key=="linearDamping") {
                float v = luaL_checknumber(L,3);
                self->setLinearDamping(v);
        } else if (key=="angularDamping") {
                float v = luaL_checknumber(L,3);
                self->setAngularDamping(v);
        } else if (key=="linearSleepThreshold") {
                float v = luaL_checknumber(L,3);
                self->setLinearSleepThreshold(v);
        } else if (key=="angularSleepThreshold") {
                float v = luaL_checknumber(L,3);
                self->setAngularSleepThreshold(v);
        } else {
               my_lua_error(L,"Not a writeable CollisionMesh member: "+key);
        }
        return 0;
TRY_END
}

EQ_MACRO(CollisionMeshPtr,colmesh,COLMESH_TAG)

MT_MACRO_NEWINDEX(colmesh);

//}}}


// RIGID_BODY ============================================================== {{{

void push_rbody (lua_State *L, const RigidBodyPtr &self)
{
        if (self.isNull())
                lua_pushnil(L);
        else
                push(L,new RigidBodyPtr(self),RBODY_TAG);
}

static int rbody_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(RigidBodyPtr,self,1,RBODY_TAG,0);
        delete self;
        return 0;
TRY_END
}

static int rbody_force (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) {
                // local
                GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Ogre::Vector3,force,2,VECTOR3_TAG);
                GET_UD_MACRO(Ogre::Vector3,rel_pos,3,VECTOR3_TAG);
                bool world_orientation = 0!=lua_toboolean(L,4);
                if (world_orientation) {
                        self->force(force,rel_pos);
                } else {
                        self->force(force,self->getOrientation()*rel_pos);
                }
        } else if (lua_gettop(L)==3) {
                GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Ogre::Vector3,force,2,VECTOR3_TAG);
                GET_UD_MACRO(Ogre::Vector3,wpos,3,VECTOR3_TAG);
                const Ogre::Vector3 &pos = self->getPosition();
                self->force(force,wpos-pos);
        } else {
                check_args(L,2);
                GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Ogre::Vector3,force,2,VECTOR3_TAG);
                self->force(force);
        }
        return 0;
TRY_END
}

static int rbody_impulse (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) {
                // local
                GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Ogre::Vector3,impulse,2,VECTOR3_TAG);
                GET_UD_MACRO(Ogre::Vector3,rel_pos,3,VECTOR3_TAG);
                bool world_orientation = 0!=lua_toboolean(L,4);
                if (world_orientation) {
                        self->impulse(impulse,rel_pos);
                } else {
                        self->impulse(impulse,self->getOrientation()*rel_pos);
                }
        } else if (lua_gettop(L)==3) {
                GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Ogre::Vector3,impulse,2,VECTOR3_TAG);
                GET_UD_MACRO(Ogre::Vector3,wpos,3,VECTOR3_TAG);
                const Ogre::Vector3 &pos = self->getPosition();
                self->impulse(impulse,wpos - pos);
        } else {
                check_args(L,2);
                GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Ogre::Vector3,impulse,2,VECTOR3_TAG);
                self->impulse(impulse);
        }
        return 0;
TRY_END
}


static int rbody_torque_impulse (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        GET_UD_MACRO(Ogre::Vector3,torque,2,VECTOR3_TAG);
        self->torqueImpulse(torque);
        return 0;
TRY_END
}


static int rbody_torque (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        GET_UD_MACRO(Ogre::Vector3,torque,2,VECTOR3_TAG);
        self->torque(torque);
        return 0;
TRY_END
}

static void push_sweep_result (lua_State *L,
                               const SweepResult &r, float len)
{
        lua_pushnumber(L,r.dist * len);
        push_rbody(L,r.rb->getPtr());
        // normal is documented as being object space but is actually world space
        push(L,new Ogre::Vector3(/*r.rb->getOrientation()* */r.n.normalisedCopy()),
                                 VECTOR3_TAG);
}

static int cast (lua_State *L)
{
TRY_START
        check_args_min(L,7);

        // use pointers because we want to assign them later
        RigidBody *body = NULL;
        PhysicsWorld *world;
        if (has_tag(L,1,RBODY_TAG)) {
                body = &***static_cast<RigidBodyPtr**>(lua_touserdata(L, 1));
                world = &*(body->world);
        } else {
                GET_UD_MACRO(PhysicsWorldPtr,world_,1,PWORLD_TAG);
                world = &*world_;
        }
        // one or other of col_mesh or radius will be used
        CollisionMeshPtr *col_mesh = NULL;
        float radius = 0;
        if (lua_type(L,2) == LUA_TNUMBER) {
                radius = lua_tonumber(L,2);
        } else {
                GET_UD_MACRO(CollisionMeshPtr,col_mesh_,2,COLMESH_TAG);
                col_mesh = &col_mesh_;
        }

        Ogre::Vector3 start, end;
        {
                GET_UD_MACRO(Ogre::Vector3,start_,3,VECTOR3_TAG);
                GET_UD_MACRO(Ogre::Vector3,end_,4,VECTOR3_TAG);
                start = start_;
                end = end_;
        }
        float len = luaL_checknumber(L,5);

        if (len>0) {
                // 'end' is actually a direction
                end = start + len * end;
        } else {
                len = 1;
        }

        int base_line = 5;
        Ogre::Quaternion startq(1,0,0,0);
        Ogre::Quaternion endq(1,0,0,0);

        if (col_mesh) {
                check_args_min(L,9);
                base_line += 2;
                GET_UD_MACRO(Ogre::Quaternion,startq_,6,QUAT_TAG);
                GET_UD_MACRO(Ogre::Quaternion,endq_,7,QUAT_TAG);
                startq = startq_;
                endq = endq_;
        }

        bool nearest_only = 0!=lua_toboolean(L,base_line+1);
        bool ignore_dynamic = 0!=lua_toboolean(L,base_line+2);

        base_line += 2;

        if (body) {
                start = body->getOrientation()*start + body->getPosition();
                end = body->getOrientation()*end + body->getPosition();
                startq = body->getOrientation() * startq;
                endq = body->getOrientation() * endq;
        }

        int blacklist_number = lua_gettop(L) - base_line;

        LuaSweepCallback lcb(ignore_dynamic);
        for (int i=1 ; i<=blacklist_number ; ++i) {
                GET_UD_MACRO(RigidBodyPtr,black,base_line+i,RBODY_TAG);
                lcb.blacklist.insert(&*black);
        }

        if (col_mesh != NULL) {
                world->sweep(*col_mesh,start,startq,end,endq,lcb);
        } else {
                world->ray(start,end,lcb,radius);
        }
        if (lcb.results.size()==0) return 0;
        SweepResult nearest;
        nearest.dist = FLT_MAX;
        for (size_t i=0 ; i<lcb.results.size() ; ++i) {
                SweepResult &r = lcb.results[i];
                if (nearest_only) {
                        if (r.dist<nearest.dist) nearest = r;
                } else {
                        push_sweep_result(L, r, len);
                }
        }
        if (nearest_only) {
                // push nearest
                push_sweep_result(L, nearest, len);
                return 3;
        }
        return lcb.results.size() * 3;
TRY_END
}


static int rbody_world_position_xyz (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        const Ogre::Vector3 &v = self->getPosition();
        lua_pushnumber(L,v.x);
        lua_pushnumber(L,v.y);
        lua_pushnumber(L,v.z);
        return 3;
TRY_END
}


static int rbody_world_orientation_wxyz (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        const Ogre::Quaternion &q = self->getOrientation();
        lua_pushnumber(L,q.w);
        lua_pushnumber(L,q.x);
        lua_pushnumber(L,q.y);
        lua_pushnumber(L,q.z);
        return 4;
TRY_END
}


static int rbody_local_pos (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==3) {
                GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Ogre::Vector3,local_pos,2,VECTOR3_TAG);
                GET_UD_MACRO(Ogre::Vector3,result,3,VECTOR3_TAG);
                result = self->getPosition() + self->getOrientation() * local_pos;
                return 0;
        }
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        GET_UD_MACRO(Ogre::Vector3,local_pos,2,VECTOR3_TAG);
        Ogre::Vector3 result = self->getPosition() + self->getOrientation() * local_pos;
        push(L, new Ogre::Vector3(result), VECTOR3_TAG);
        return 1;
TRY_END
}


static int rbody_local_vel (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        GET_UD_MACRO(Ogre::Vector3,pos,2,VECTOR3_TAG);
        bool world_space = 0!=lua_toboolean(L,3);
        Ogre::Vector3 local_pos = pos;
        if (world_space) {
                local_pos -= self->getPosition();
        }
        const Ogre::Vector3 &local_vel = self->getLocalVelocity(local_pos);
        lua_pushnumber(L,local_vel.x);
        lua_pushnumber(L,local_vel.y);
        lua_pushnumber(L,local_vel.z);
        return 3;
TRY_END
}


static int rbody_activate (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        self->activate();
        return 0;
TRY_END
}


static int rbody_deactivate (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        self->deactivate();
        return 0;
TRY_END
}


static int rbody_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        self->destroy(L);
        return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (rbody,RigidBodyPtr,RBODY_TAG)


static int rbody_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="force") {
                push_cfunction(L,rbody_force);
        } else if (key=="impulse") {
                push_cfunction(L,rbody_impulse);
        } else if (key=="torque") {
                push_cfunction(L,rbody_torque);
        } else if (key=="torqueImpulse") {
                push_cfunction(L,rbody_torque_impulse);

        } else if (key=="cast") {
                push_cfunction(L,cast);

        } else if (key=="activate") {
                push_cfunction(L,rbody_activate);
        } else if (key=="deactivate") {
                push_cfunction(L,rbody_deactivate);
        } else if (key=="destroy") {
                push_cfunction(L,rbody_destroy);

        } else if (key=="linearSleepThreshold") {
                lua_pushnumber(L,self->getLinearSleepThreshold());
        } else if (key=="angularSleepThreshold") {
                lua_pushnumber(L,self->getAngularSleepThreshold());

        } else if (key=="worldPosition") {
                push(L, new Ogre::Vector3(self->getPosition()), VECTOR3_TAG);
        } else if (key=="getWorldPositionXYZ") {
                push_cfunction(L,rbody_world_position_xyz);
        } else if (key=="worldOrientation") {
                push(L, new Ogre::Quaternion(self->getOrientation()), QUAT_TAG);
        } else if (key=="getWorldOrientationWXYZ") {
                push_cfunction(L,rbody_world_orientation_wxyz);

        } else if (key=="linearVelocity") {
                push(L, new Ogre::Vector3(self->getLinearVelocity()),
                        VECTOR3_TAG);
        } else if (key=="angularVelocity") {
                push(L, new Ogre::Vector3(self->getAngularVelocity()),
                        VECTOR3_TAG);
        } else if (key=="getLocalPosition") {
                push_cfunction(L,rbody_local_pos);
        } else if (key=="getLocalVelocity") {
                push_cfunction(L,rbody_local_vel);

        } else if (key=="contactProcessingThreshold") {
                lua_pushnumber(L,self->getContactProcessingThreshold());

        } else if (key=="linearDamping") {
                lua_pushnumber(L,self->getLinearDamping());
        } else if (key=="angularDamping") {
                lua_pushnumber(L,self->getAngularDamping());

        } else if (key=="mass") {
                lua_pushnumber(L,self->getMass());
        } else if (key=="inertia") {
                push(L,new Ogre::Vector3(self->getInertia()),VECTOR3_TAG);

        } else if (key=="mesh") {
                push_colmesh(L,self->colMesh);
        } else if (key=="world") {
                push_pworld(L,self->world);

        } else if (key=="owner") {
                if (self->owner.isNull()) {
                        lua_pushnil(L);
                } else {
                        push_gritobj(L,self->owner);
                }

        } else if (key=="updateCallback") {
                self->pushUpdateCallback(L);
        } else if (key=="stepCallback") {
                self->pushStepCallback(L);
        } else if (key=="stabiliseCallback") {
                self->pushStabiliseCallback(L);
        } else {
                my_lua_error(L,"Not a readable RigidBody member: "+key);
        }
        return 1;
TRY_END
}

static int rbody_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="linearVelocity") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self->setLinearVelocity(v);
        } else if (key=="angularVelocity") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self->setAngularVelocity(v);
        } else if (key=="worldPosition") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self->setPosition(v);
        } else if (key=="worldOrientation") {
                GET_UD_MACRO(Ogre::Quaternion,v,3,QUAT_TAG);
                self->setOrientation(v);
        } else if (key=="contactProcessingThreshold") {
                float v = luaL_checknumber(L,3);
                self->setContactProcessingThreshold(v);
        } else if (key=="linearDamping") {
                float v = luaL_checknumber(L,3);
                self->setLinearDamping(v);
        } else if (key=="angularDamping") {
                float v = luaL_checknumber(L,3);
                self->setAngularDamping(v);
        } else if (key=="linearSleepThreshold") {
                float v = luaL_checknumber(L,3);
                self->setLinearSleepThreshold(v);
        } else if (key=="angularSleepThreshold") {
                float v = luaL_checknumber(L,3);
                self->setAngularSleepThreshold(v);
        } else if (key=="mass") {
                float v = luaL_checknumber(L,3);
                self->setMass(v);
        } else if (key=="updateCallback") {
                self->setUpdateCallback(L);
        } else if (key=="stepCallback") {
                self->setStepCallback(L);
        } else if (key=="stabiliseCallback") {
                self->setStabiliseCallback(L);
        } else if (key=="inertia") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self->setInertia(v);

        } else if (key=="owner") {
                if (lua_isnil(L,3)) {
                        self->owner.setNull();
                } else {
                        GET_UD_MACRO(GritObjectPtr,v,3,GRITOBJ_TAG);
                        self->owner = v;
                }

        } else {
               my_lua_error(L,"Not a writeable RigidBody member: "+key);
        }
        return 0;
TRY_END
}

EQ_MACRO(RigidBodyPtr,rbody,RBODY_TAG)

MT_MACRO_NEWINDEX(rbody);

//}}}


// PHYSICS WORLD =========================================================== {{{

void push_pworld (lua_State *L, const PhysicsWorldPtr &self)
{
        if (self.isNull())
                lua_pushnil(L);
        else
                push(L,new PhysicsWorldPtr(self),PWORLD_TAG);
}

int pworld_make(lua_State *L)
{
TRY_START
        check_args(L,6);
        float mX = luaL_checknumber(L,1);
        float mY = luaL_checknumber(L,2);
        float mZ = luaL_checknumber(L,3);
        float MX = luaL_checknumber(L,4);
        float MY = luaL_checknumber(L,5);
        float MZ = luaL_checknumber(L,6);
        push_pworld (L, PhysicsWorldPtr(new PhysicsWorld
                (Ogre::AxisAlignedBox(mX,mY,mZ,MX,MY,MZ))));
        return 1;
TRY_END
}

static int pworld_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(PhysicsWorldPtr,self,1,PWORLD_TAG,0);
        delete self;
        return 0;
TRY_END
}


static int pworld_pump (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        float time_step = luaL_checknumber(L,2);
        int iterations = self->pump(L,time_step);
        lua_pushnumber(L,iterations);
        return 1;
TRY_END
}



static int pworld_add_mesh (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        CollisionMeshPtr cmp;
        Ogre::String name = luaL_checkstring(L,2);
        cmp = self->createFromFile(name);
        push_colmesh(L,cmp);
        return 1;
TRY_END
}



static int pworld_get_mesh (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        CollisionMeshPtr cmp = self->getMesh(name);
        push_colmesh(L,cmp);
        return 1;
TRY_END
}



static int pworld_remove_mesh (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        self->deleteMesh(name);
        return 0;
TRY_END
}



static int pworld_has_mesh (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        lua_pushboolean(L,self->hasMesh(name));
        return 1;
TRY_END
}

static int pworld_clear_meshes (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        self->clearMeshes();
        return 0;
TRY_END
}


static int pworld_add_rigid_body (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        CollisionMeshPtr cmp;
        if (lua_type(L,2) == LUA_TSTRING) {
                Ogre::String n = luaL_checkstring(L,2);
                cmp = self->getMesh(n);
        } else {
                GET_UD_MACRO(CollisionMeshPtr,cmp2,2,COLMESH_TAG);
                cmp = cmp2;
        }
        GET_UD_MACRO(Ogre::Vector3,pos,3,VECTOR3_TAG);
        GET_UD_MACRO(Ogre::Quaternion,quat,4,QUAT_TAG);
        RigidBodyPtr rbp = (new RigidBody(self,cmp,pos,quat))->getPtr();
        push_rbody(L,rbp);
        return 1;
TRY_END
}

static int pworld_set_material_interaction (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        if (!lua_istable(L,2))
                my_lua_error(L,"Second parameter should be a table");
        if (!lua_istable(L,3))
                my_lua_error(L,"Third parameter should be a table");
        float friction = luaL_checknumber(L,4);
        float restitution = luaL_checknumber(L,5);

        // scan through table adding lua data to o
        for (lua_pushnil(L) ; lua_next(L,2)!=0 ; lua_pop(L,1)) {
                physics_mat m1 = check_t<physics_mat>(L,-1);
                for (lua_pushnil(L) ; lua_next(L,3)!=0 ; lua_pop(L,1)) {
                        physics_mat m2 = check_t<physics_mat>(L,-1);
                        self->setInteraction(m1, m2, friction, restitution);
                }
        }

        return 0;
TRY_END
}

static int pworld_get_material_interaction (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        physics_mat m1 = check_t<physics_mat>(L,2);
        physics_mat m2 = check_t<physics_mat>(L,3);
        float friction;
        float restitution;
        self->getInteraction(m1, m2, friction, restitution);
        lua_pushnumber(L, friction);
        lua_pushnumber(L, restitution);
        return 2;
TRY_END
}



TOSTRING_SMART_PTR_MACRO(pworld,PhysicsWorldPtr,PWORLD_TAG)

static int pworld_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="cast") {
                push_cfunction(L,cast);
        } else if (key=="pump") {
                push_cfunction(L,pworld_pump);
        } else if (key=="addMesh") {
                push_cfunction(L,pworld_add_mesh);
        } else if (key=="getMesh") {
                push_cfunction(L,pworld_get_mesh);
        } else if (key=="removeMesh") {
                push_cfunction(L,pworld_remove_mesh);
        } else if (key=="hasMesh") {
                push_cfunction(L,pworld_has_mesh);
        } else if (key=="clearMeshes") {
                push_cfunction(L,pworld_clear_meshes);
        } else if (key=="meshes") {
                // meshes are actually program-wide rather than
                // per-physics-world but there doesn't seem much point in
                // reflecting this fact on the lua side
                lua_newtable(L);
                unsigned int c = 0;
                const CollisionMeshMap &ms = PhysicsWorld::getMeshes();
                typedef CollisionMeshMap::const_iterator I;
                for (I i=ms.begin(), i_=ms.end() ; i!=i_ ; ++i) {
                        const CollisionMeshPtr &cm = i->second;
                        push_colmesh(L,cm);
                        lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                        c++;
                }
        } else if (key=="addRigidBody") {
                push_cfunction(L,pworld_add_rigid_body);
        } else if (key=="gravity") {
                push(L,new Ogre::Vector3(self->getGravity()),VECTOR3_TAG);
        } else if (key=="stepSize") {
                lua_pushnumber(L,self->getStepSize());
        } else if (key=="maxSteps") {
                lua_pushnumber(L,self->getMaxSteps());
        } else if (key=="contactBreakingThreshold") {
                lua_pushnumber(L,self->getContactBreakingThreshold());
        } else if (key=="deactivationTime") {
                lua_pushnumber(L,self->getDeactivationTime());
        } else if (key=="gimpactOneWayMeshHack") {
                lua_pushboolean(L,self->gimpactOneWayMeshHack);
        } else if (key=="bumpyTriangleMeshHack") {
                lua_pushboolean(L,self->bumpyTriangleMeshHack);
        } else if (key=="verboseContacts") {
                lua_pushboolean(L,self->verboseContacts);
        } else if (key=="errorContacts") {
                lua_pushboolean(L,self->errorContacts);
        } else if (key=="setMaterialInteraction") {
                push_cfunction(L,pworld_set_material_interaction);
        } else if (key=="getMaterialInteraction") {
                push_cfunction(L,pworld_get_material_interaction);
        } else if (key=="solverDamping") {
                lua_pushnumber(L,self->getSolverDamping());
        } else if (key=="solverIterations") {
                lua_pushnumber(L,self->getSolverIterations());
        } else if (key=="solverErp") {
                lua_pushnumber(L,self->getSolverErp());
        } else if (key=="solverErp2") {
                lua_pushnumber(L,self->getSolverErp2());
        } else if (key=="solverSplitImpulseThreshold") {
                lua_pushnumber(L,self->getSolverSplitImpulseThreshold());
        } else if (key=="solverLinearSlop") {
                lua_pushnumber(L,self->getSolverLinearSlop());
        } else if (key=="solverWarmStartingFactor") {
                lua_pushnumber(L,self->getSolverWarmStartingFactor());
        } else if (key=="solverSplitImpulse") {
                lua_pushboolean(L,self->getSolverSplitImpulse());
        } else if (key=="solverRandomiseOrder") {
                lua_pushboolean(L,self->getSolverRandomiseOrder());
        } else if (key=="solverFrictionSeparate") {
                lua_pushboolean(L,self->getSolverFrictionSeparate());
        } else if (key=="solverUseWarmStarting") {
                lua_pushboolean(L,self->getSolverUseWarmStarting());
        } else if (key=="solverCacheFriendly") {
                lua_pushboolean(L,self->getSolverCacheFriendly());
        } else {
                my_lua_error(L,"Not a valid PhysicsWorld member: "+key);
        }
        return 1;
TRY_END
}

static int pworld_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="gravity") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self->setGravity(v);
        } else if (key=="maxSteps") {
                int v = check_t<int>(L,3);
                self->setMaxSteps(v);
        } else if (key=="stepSize") {
                float v = luaL_checknumber(L,3);
                self->setStepSize(v);
        } else if (key=="contactBreakingThreshold") {
                float v = luaL_checknumber(L,3);
                self->setContactBreakingThreshold(v);
        } else if (key=="deactivationTime") {
                float v = luaL_checknumber(L,3);
                self->setDeactivationTime(v);
        } else if (key=="gimpactOneWayMeshHack") {
                bool v = 0!=lua_toboolean(L,3);
                self->gimpactOneWayMeshHack = v;
        } else if (key=="bumpyTriangleMeshHack") {
                bool v = 0!=lua_toboolean(L,3);
                self->bumpyTriangleMeshHack = v;
        } else if (key=="verboseContacts") {
                bool v = 0!=lua_toboolean(L,3);
                self->verboseContacts = v;
        } else if (key=="errorContacts") {
                bool v = 0!=lua_toboolean(L,3);
                self->errorContacts = v;
        } else if (key=="solverDamping") {
                float v = luaL_checknumber(L,3);
                self->setSolverDamping(v);
        } else if (key=="solverIterations") {
                int v = check_t<int>(L,3);
                self->setSolverIterations(v);
        } else if (key=="solverErp") {
                float v = luaL_checknumber(L,3);
                self->setSolverErp(v);
        } else if (key=="solverErp2") {
                float v = luaL_checknumber(L,3);
                self->setSolverErp2(v);
        } else if (key=="solverSplitImpulseThreshold") {
                float v = luaL_checknumber(L,3);
                self->setSolverSplitImpulseThreshold(v);
        } else if (key=="solverLinearSlop") {
                float v = luaL_checknumber(L,3);
                self->setSolverLinearSlop(v);
        } else if (key=="solverWarmStartingFactor") {
                float v = luaL_checknumber(L,3);
                self->setSolverWarmStartingFactor(v);
        } else if (key=="solverSplitImpulse") {
                bool v = 0!=lua_toboolean(L,3);
                self->setSolverSplitImpulse(v);
        } else if (key=="solverRandomiseOrder") {
                bool v = 0!=lua_toboolean(L,3);
                self->setSolverRandomiseOrder(v);
        } else if (key=="solverFrictionSeparate") {
                bool v = 0!=lua_toboolean(L,3);
                self->setSolverFrictionSeparate(v);
        } else if (key=="solverUseWarmStarting") {
                bool v = 0!=lua_toboolean(L,3);
                self->setSolverUseWarmStarting(v);
        } else if (key=="solverCacheFriendly") {
                bool v = 0!=lua_toboolean(L,3);
                self->setSolverCacheFriendly(v);
        } else {
                my_lua_error(L,"Not a valid PhysicsWorld member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(PhysicsWorld,pworld,PWORLD_TAG)

MT_MACRO_NEWINDEX(pworld);

//}}}



// vim: shiftwidth=8:tabstop=8:expandtab
