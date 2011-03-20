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

#include <limits>

#include <OgreSceneManager.h>

#include "PhysicsWorld.h"
#include "CollisionMesh.h"
#include "Clutter.h"

#include "lua_wrappers_primitives.h"
#include "lua_wrappers_physics.h"
#include "lua_wrappers_gritobj.h"
#include "lua_wrappers_mobj.h"
#include "lua_wrappers_mesh.h"
#include "lua_wrappers_scnmgr.h"

// Sweep Callback {{{

struct SweepResult { RigidBody *rb; float dist; Vector3 n; int material; };
typedef std::vector<SweepResult> SweepResults;

class LuaSweepCallback : public PhysicsWorld::SweepCallback {
    public:           
        LuaSweepCallback (bool ignore_dynamic)
              : ignoreDynamic(ignore_dynamic)
        { }
        virtual void result (RigidBody &rb, float dist, const Vector3 &n, int m) {
                if (ignoreDynamic && rb.getMass()>0) return;
                if (blacklist.find(&rb) != blacklist.end()) return;
                SweepResult r;
                r.rb = &rb;
                r.dist = dist;
                r.n = n;
                r.material = m;
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
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"mass")) {
                lua_pushnumber(L,self->getMass());
        } else if (!::strcmp(key,"ccdMotionThreshold")) {
                lua_pushnumber(L,self->getCCDMotionThreshold());
        } else if (!::strcmp(key,"ccdSweptSphereRadius")) {
                lua_pushnumber(L,self->getCCDSweptSphereRadius());
        } else if (!::strcmp(key,"linearDamping")) {
                lua_pushnumber(L,self->getLinearDamping());
        } else if (!::strcmp(key,"angularDamping")) {
                lua_pushnumber(L,self->getAngularDamping());
        } else if (!::strcmp(key,"linearSleepThreshold")) {
                lua_pushnumber(L,self->getLinearSleepThreshold());
        } else if (!::strcmp(key,"angularSleepThreshold")) {
                lua_pushnumber(L,self->getAngularSleepThreshold());
        } else if (!::strcmp(key,"name")) {
                lua_pushstring(L,self->getName().c_str());
        } else {
                my_lua_error(L,"Not a readable CollisionMesh member: "+std::string(key));
        }
        return 1;
TRY_END
}

static int colmesh_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(CollisionMeshPtr,self,1,COLMESH_TAG);
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"mass")) {
                float v = luaL_checknumber(L,3);
                self->setMass(v);
        } else if (!::strcmp(key,"ccdMotionThreshold")) {
                float v = luaL_checknumber(L,3);
                self->setCCDMotionThreshold(v);
        } else if (!::strcmp(key,"ccdSweptSphereRadius")) {
                float v = luaL_checknumber(L,3);
                self->setCCDSweptSphereRadius(v);
        } else if (!::strcmp(key,"linearDamping")) {
                float v = luaL_checknumber(L,3);
                self->setLinearDamping(v);
        } else if (!::strcmp(key,"angularDamping")) {
                float v = luaL_checknumber(L,3);
                self->setAngularDamping(v);
        } else if (!::strcmp(key,"linearSleepThreshold")) {
                float v = luaL_checknumber(L,3);
                self->setLinearSleepThreshold(v);
        } else if (!::strcmp(key,"angularSleepThreshold")) {
                float v = luaL_checknumber(L,3);
                self->setAngularSleepThreshold(v);
        } else {
               my_lua_error(L,"Not a writeable CollisionMesh member: "+std::string(key));
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

static int rbody_scatter (lua_State *L)
{
TRY_START
        check_args(L,11);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        const char *mat = luaL_checkstring(L,2);
        Transform world_trans;
        world_trans.p = self->getPosition();
        world_trans.r = self->getOrientation();
        float density       = check_float(L,3);
        float min_slope     = check_float(L,4);
        float max_slope     = check_float(L,5);
        float min_elevation = check_float(L,6);
        float max_elevation = check_float(L,7);
        bool no_z           = check_bool(L,8);
        bool rotate         = check_bool(L,9);
        bool align_slope    = check_bool(L,10);
        unsigned seed       = check_t<unsigned>(L,11);

        std::vector<Transform> r;
        self->colMesh->scatter(self->world->getMaterial(mat).id,
                               world_trans, density, min_slope, max_slope, min_elevation,
                               max_elevation, no_z, rotate, align_slope, seed,
                               r);

        lua_newtable(L);
        for (size_t j=0 ; j<r.size(); ++j) {
                const Quaternion &q = r[j].r;
                const Vector3 &p = r[j].p;
                lua_pushnumber(L, p.x);
                lua_rawseti(L,-2,7*j+0+LUA_ARRAY_BASE);
                lua_pushnumber(L, p.y);
                lua_rawseti(L,-2,7*j+1+LUA_ARRAY_BASE);
                lua_pushnumber(L, p.z);
                lua_rawseti(L,-2,7*j+2+LUA_ARRAY_BASE);
                lua_pushnumber(L, q.w);
                lua_rawseti(L,-2,7*j+3+LUA_ARRAY_BASE);
                lua_pushnumber(L, q.x);
                lua_rawseti(L,-2,7*j+4+LUA_ARRAY_BASE);
                lua_pushnumber(L, q.y);
                lua_rawseti(L,-2,7*j+5+LUA_ARRAY_BASE);
                lua_pushnumber(L, q.z);
                lua_rawseti(L,-2,7*j+6+LUA_ARRAY_BASE);
        }

        return 1;
TRY_END
}

static int rbody_ranged_scatter (lua_State *L)
{
TRY_START
        check_args(L,16);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        GET_UD_MACRO(Ogre::SceneManager,sm,2,SCNMGR_TAG);
        const char *mat    = luaL_checkstring(L,3);
        const char *name    = luaL_checkstring(L,4);
        GET_UD_MACRO(Ogre::MeshPtr,mesh,5,MESH_TAG);
        float density       = check_float(L,6);
        float min_slope     = check_float(L,7);
        float max_slope     = check_float(L,8);
        float min_elevation = check_float(L,9);
        float max_elevation = check_float(L,10);
        bool no_z           = check_bool(L,11);
        bool rotate         = check_bool(L,12);
        bool align_slope    = check_bool(L,13);
        unsigned seed       = check_t<unsigned>(L,14);
        unsigned triangles  = check_int(L,15,0,65535);
        bool tangents       = check_bool(L,16);

        Transform world_trans;
        world_trans.p = self->getPosition();
        world_trans.r = self->getOrientation();

        std::stringstream triangles_;
        triangles_ << triangles;
        std::stringstream tangents_;
        tangents_ << (tangents?"true":"false");

        Ogre::NameValuePairList ps;
        ps["triangles"] = triangles_.str();
        ps["tangents"] = tangents_.str();
        RangedClutter *ranged_r = static_cast<RangedClutter*>(sm.createMovableObject(name, "RangedClutter", &ps));
        ranged_r->setNextMesh(mesh);
        self->colMesh->scatter(self->world->getMaterial(mat).id,
                               world_trans, density, min_slope, max_slope, min_elevation,
                               max_elevation, no_z, rotate, align_slope, seed,
                               *ranged_r);

        push_rclutter(L, ranged_r);
        return 1;
TRY_END
}

static int rbody_force (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        Vector3 force = check_v3(L, 2);
        Vector3 wpos = check_v3(L, 3);
        const Vector3 &pos = self->getPosition();
        self->force(force,wpos-pos);
        return 0;
TRY_END
}

static int rbody_impulse (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        Vector3 impulse = check_v3(L, 2);
        Vector3 wpos = check_v3(L, 3);
        const Vector3 &pos = self->getPosition();
        self->impulse(impulse, wpos-pos);
        return 0;
TRY_END
}


static int rbody_torque_impulse (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        Vector3 torque = check_v3(L, 2);
        self->torqueImpulse(torque);
        return 0;
TRY_END
}


static int rbody_torque (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        Vector3 torque = check_v3(L, 2);
        self->torque(torque);
        return 0;
TRY_END
}

static int rbody_get_part_enabled (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        lua_pushboolean(L, self->getElementEnabled(i));
        return 1;
TRY_END
}

static int rbody_set_part_enabled (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        bool b = check_bool(L, 3);
        self->setElementEnabled(i, b);
        return 0;
TRY_END
}

static int rbody_get_part_position_initial (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Vector3 v = self->getElementPositionMaster(i);
        lua_pushnumber(L,v.x); lua_pushnumber(L,v.y); lua_pushnumber(L,v.z);
        return 3;
TRY_END
}

static int rbody_get_part_position_offset (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Vector3 v = self->getElementPositionOffset(i);
        lua_pushnumber(L,v.x); lua_pushnumber(L,v.y); lua_pushnumber(L,v.z);
        return 3;
TRY_END
}

static int rbody_set_part_position_offset (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Vector3 v = check_v3(L,3);
        self->setElementPositionOffset(i, v);
        return 0;
TRY_END
}

static int rbody_get_part_orientation_initial (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Quaternion q = self->getElementOrientationMaster(i);
        lua_pushnumber(L,q.w); lua_pushnumber(L,q.x); lua_pushnumber(L,q.y); lua_pushnumber(L,q.z);
        return 4;
TRY_END
}

static int rbody_get_part_orientation_offset (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Quaternion q = self->getElementOrientationOffset(i);
        lua_pushnumber(L,q.w); lua_pushnumber(L,q.x); lua_pushnumber(L,q.y); lua_pushnumber(L,q.z);
        return 4;
TRY_END
}

static int rbody_set_part_orientation_offset (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Quaternion q = check_quat(L,3);
        self->setElementOrientationOffset(i, q);
        return 0;
TRY_END
}


static void push_sweep_result (lua_State *L, const SweepResult &r, float len,
                               const PhysicsWorld &world)
{
        lua_pushnumber(L,r.dist * len);
        push_rbody(L,r.rb->getPtr());
        // normal is documented as being object space but is actually world space
        Vector3 normal = /*r.rb->getOrientation()* */r.n.normalisedCopy();
        push_v3(L, normal);
        lua_pushstring(L, world.getMaterial(r.material).name.c_str());
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

        Vector3 start, end;
        {
                Vector3 start_ = check_v3(L,3);
                Vector3 end_ = check_v3(L,4);
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
        Quaternion startq(1,0,0,0);
        Quaternion endq(1,0,0,0);

        if (col_mesh) {
                check_args_min(L,9);
                base_line += 2;
                Quaternion startq_ = check_quat(L,6);
                Quaternion endq_ = check_quat(L,7);
                startq = startq_;
                endq = endq_;
        }

        bool nearest_only = check_bool(L,base_line+1);
        bool ignore_dynamic = check_bool(L,base_line+2);

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
                        push_sweep_result(L, r, len, *world);
                }
        }
        if (nearest_only) {
                // push nearest
                push_sweep_result(L, nearest, len, *world);
                return 4;
        }
        return lcb.results.size() * 4;
TRY_END
}




static int rbody_local_to_world (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        Vector3 a = check_v3(L, 2);
        Vector3 result = self->getPosition() + self->getOrientation() * a;
        push_v3(L, result);
        return 1;
TRY_END
}

static int rbody_world_to_local (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        Vector3 a = check_v3(L, 2);
        Vector3 result = self->getOrientation().inverse() * (a - self->getPosition());
        push_v3(L, result);
        return 1;
TRY_END
}


static int rbody_local_pos (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        Vector3 local_pos = check_v3(L,2);
        Vector3 result = self->getPosition() + self->getOrientation() * local_pos;
        push_v3(L, result);
        return 1;
TRY_END
}

static int rbody_local_vel (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        Vector3 pos = check_v3(L, 2);
        bool world_space = check_bool(L,3);
        Vector3 local_pos = pos;
        if (world_space) {
                local_pos -= self->getPosition();
        }
        Vector3 local_vel = self->getLocalVelocity(local_pos);
        push_v3(L, local_vel);
        return 1;
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
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"force")) {
                push_cfunction(L,rbody_force);
        } else if (!::strcmp(key,"impulse")) {
                push_cfunction(L,rbody_impulse);
        } else if (!::strcmp(key,"torque")) {
                push_cfunction(L,rbody_torque);
        } else if (!::strcmp(key,"torqueImpulse")) {
                push_cfunction(L,rbody_torque_impulse);

        } else if (!::strcmp(key,"procObjMaterials")) {
                std::vector<int> mats;
                self->colMesh->getProcObjMaterials(mats);
                lua_createtable(L, mats.size(), 0);
                for (size_t j=0 ; j<mats.size(); ++j) {
                        lua_pushstring(L, self->world->getMaterial(mats[j]).name.c_str());
                        lua_rawseti(L, -2, j+LUA_ARRAY_BASE);
                }
        } else if (!::strcmp(key,"scatter")) {
                push_cfunction(L,rbody_scatter);
        } else if (!::strcmp(key,"rangedScatter")) {
                push_cfunction(L,rbody_ranged_scatter);

        } else if (!::strcmp(key,"cast")) {
                push_cfunction(L,cast);

        } else if (!::strcmp(key,"activate")) {
                push_cfunction(L,rbody_activate);
        } else if (!::strcmp(key,"deactivate")) {
                push_cfunction(L,rbody_deactivate);
        } else if (!::strcmp(key,"destroy")) {
                push_cfunction(L,rbody_destroy);

        } else if (!::strcmp(key,"linearSleepThreshold")) {
                lua_pushnumber(L,self->getLinearSleepThreshold());
        } else if (!::strcmp(key,"angularSleepThreshold")) {
                lua_pushnumber(L,self->getAngularSleepThreshold());

        } else if (!::strcmp(key,"worldPosition")) {
                push_v3(L, self->getPosition());
        } else if (!::strcmp(key,"worldOrientation")) {
                push_quat(L, self->getOrientation());

        } else if (!::strcmp(key,"localToWorld")) {
                push_cfunction(L,rbody_local_to_world);
        } else if (!::strcmp(key,"worldToLocal")) {
                push_cfunction(L,rbody_world_to_local);

        } else if (!::strcmp(key,"linearVelocity")) {
                push_v3(L, self->getLinearVelocity());
        } else if (!::strcmp(key,"angularVelocity")) {
                push_v3(L, self->getAngularVelocity());
        } else if (!::strcmp(key,"getLocalPosition")) {
                push_cfunction(L,rbody_local_pos);
        } else if (!::strcmp(key,"getLocalVelocity")) {
                push_cfunction(L,rbody_local_vel);

        } else if (!::strcmp(key,"contactProcessingThreshold")) {
                lua_pushnumber(L,self->getContactProcessingThreshold());

        } else if (!::strcmp(key,"linearDamping")) {
                lua_pushnumber(L,self->getLinearDamping());
        } else if (!::strcmp(key,"angularDamping")) {
                lua_pushnumber(L,self->getAngularDamping());

        } else if (!::strcmp(key,"mass")) {
                lua_pushnumber(L,self->getMass());
        } else if (!::strcmp(key,"inertia")) {
                push_v3(L, self->getInertia());
        } else if (!::strcmp(key,"ghost")) {
                lua_pushboolean(L,self->getGhost());

        } else if (!::strcmp(key,"numParts")) {
                lua_pushnumber(L, self->getNumElements());
        } else if (!::strcmp(key,"getPartEnabled")) {
                push_cfunction(L,rbody_get_part_enabled);
        } else if (!::strcmp(key,"setPartEnabled")) {
                push_cfunction(L,rbody_set_part_enabled);
        } else if (!::strcmp(key,"getPartPositionInitial")) {
                push_cfunction(L,rbody_get_part_position_initial);
        } else if (!::strcmp(key,"getPartPositionOffset")) {
                push_cfunction(L,rbody_get_part_position_offset);
        } else if (!::strcmp(key,"setPartPositionOffset")) {
                push_cfunction(L,rbody_set_part_position_offset);
        } else if (!::strcmp(key,"getPartOrientationInitial")) {
                push_cfunction(L,rbody_get_part_orientation_initial);
        } else if (!::strcmp(key,"getPartOrientationOffset")) {
                push_cfunction(L,rbody_get_part_orientation_offset);
        } else if (!::strcmp(key,"setPartOrientationOffset")) {
                push_cfunction(L,rbody_set_part_orientation_offset);

        } else if (!::strcmp(key,"mesh")) {
                push_colmesh(L,self->colMesh);
        } else if (!::strcmp(key,"world")) {
                push_pworld(L,self->world);

        } else if (!::strcmp(key,"owner")) {
                if (self->owner.isNull()) {
                        lua_pushnil(L);
                } else {
                        push_gritobj(L,self->owner);
                }

        } else if (!::strcmp(key,"updateCallback")) {
                self->updateCallbackPtr.push(L);
        } else if (!::strcmp(key,"stepCallback")) {
                self->stepCallbackPtr.push(L);
        } else if (!::strcmp(key,"collisionCallback")) {
                self->collisionCallbackPtr.push(L);
        } else if (!::strcmp(key,"stabiliseCallback")) {
                self->stabiliseCallbackPtr.push(L);
        } else {
                my_lua_error(L,"Not a readable RigidBody member: "+std::string(key));
        }
        return 1;
TRY_END
}

static int rbody_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"linearVelocity")) {
                Vector3 v = check_v3(L, 3);
                self->setLinearVelocity(v);
        } else if (!::strcmp(key,"angularVelocity")) {
                Vector3 v = check_v3(L, 3);
                self->setAngularVelocity(v);
        } else if (!::strcmp(key,"worldPosition")) {
                Vector3 v = check_v3(L, 3);
                self->setPosition(v);
        } else if (!::strcmp(key,"worldOrientation")) {
                Quaternion v = check_quat(L, 3);
                self->setOrientation(v);
        } else if (!::strcmp(key,"contactProcessingThreshold")) {
                float v = luaL_checknumber(L,3);
                self->setContactProcessingThreshold(v);
        } else if (!::strcmp(key,"linearDamping")) {
                float v = luaL_checknumber(L,3);
                self->setLinearDamping(v);
        } else if (!::strcmp(key,"angularDamping")) {
                float v = luaL_checknumber(L,3);
                self->setAngularDamping(v);
        } else if (!::strcmp(key,"linearSleepThreshold")) {
                float v = luaL_checknumber(L,3);
                self->setLinearSleepThreshold(v);
        } else if (!::strcmp(key,"angularSleepThreshold")) {
                float v = luaL_checknumber(L,3);
                self->setAngularSleepThreshold(v);
        } else if (!::strcmp(key,"mass")) {
                float v = luaL_checknumber(L,3);
                self->setMass(v);
        } else if (!::strcmp(key,"ghost")) {
                bool v = check_bool(L,3);
                self->setGhost(v);
        } else if (!::strcmp(key,"updateCallback")) {
                self->updateCallbackPtr.set(L);
        } else if (!::strcmp(key,"stepCallback")) {
                self->stepCallbackPtr.set(L);
        } else if (!::strcmp(key,"collisionCallback")) {
                self->collisionCallbackPtr.set(L);
        } else if (!::strcmp(key,"stabiliseCallback")) {
                self->stabiliseCallbackPtr.set(L);
        } else if (!::strcmp(key,"inertia")) {
                Vector3 v = check_v3(L, 3);
                self->setInertia(v);
        } else if (!::strcmp(key,"owner")) {
                if (lua_isnil(L,3)) {
                        self->owner.setNull();
                } else {
                        GET_UD_MACRO(GritObjectPtr,v,3,GRITOBJ_TAG);
                        self->owner = v;
                }

        } else {
               my_lua_error(L,"Not a writeable RigidBody member: "+std::string(key));
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
        check_args(L,0);
        push_pworld (L, PhysicsWorldPtr(new PhysicsWorld()));
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


static int pworld_draw (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        self->draw();
        return 0;
TRY_END
}

class LuaTestCallback : public PhysicsWorld::TestCallback {
    public:

        void result (RigidBody *body, const Vector3 &pos, const Vector3 &wpos,
                     const Vector3 &normal, float penetration, int m)
        {
                resultMap[body].push_back(Result(pos,wpos,normal,penetration,m));
        }

        void pushResults (lua_State *L, const PhysicsWorldPtr &world,
                          int func_index, int err_handler)
        {
                for (ResultMap::iterator i=resultMap.begin(),i_=resultMap.end() ; i!=i_ ; ++i) {
                        RigidBody *body = i->first;
                        Results &results = i->second;
                        for (Results::iterator j=results.begin(),j_=results.end() ; j!=j_ ; ++j) {
                                if (body->destroyed()) continue;
                                lua_pushvalue(L,func_index);
                                push_rbody(L,body->getPtr());
                                lua_pushnumber(L,results.size());
                                push_v3(L,j->pos);
                                push_v3(L,j->wpos);
                                push_v3(L,j->normal);
                                lua_pushnumber(L,j->penetration);
                                lua_pushstring(L,world->getMaterial(j->m).name.c_str());
                                int status = lua_pcall(L,7,0,err_handler);
                                if (status) {
                                        lua_pop(L,1); // error message, already printed
                                        break;
                                }
                        }
                }
        }

    protected:

        struct Result {
                Result (Vector3 pos_, Vector3 wpos_,
                        Vector3 normal_, float penetration_, int m_)
                      : pos(pos_), wpos(wpos_), normal(normal_),
                        penetration(penetration_), m(m_)
                { }
                Vector3 pos;
                Vector3 wpos;
                Vector3 normal;
                float penetration;
                int m;
        };

        typedef std::vector<Result> Results;
        typedef std::map<RigidBody*,Results > ResultMap;
        ResultMap resultMap;
};

static int pworld_test (lua_State *L)
{
TRY_START
        LuaTestCallback lcb;
        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);
        if (lua_gettop(L)==6) {
                GET_UD_MACRO(PhysicsWorldPtr,world,1,PWORLD_TAG);
                float radius = lua_tonumber(L,2);
                Vector3 pos = check_v3(L,3);
                bool only_dyn = check_bool(L,4);
                if (lua_type(L,5) != LUA_TFUNCTION)
                        my_lua_error(L,"Parameter 4 should be a function.");

                world->testSphere(radius,pos,only_dyn,lcb);
                lcb.pushResults(L, world, 5, error_handler);
        } else {
                check_args(L,7);
                GET_UD_MACRO(PhysicsWorldPtr,world,1,PWORLD_TAG);
                GET_UD_MACRO(CollisionMeshPtr,col_mesh,2,COLMESH_TAG);
                Vector3 pos = check_v3(L,3);
                Quaternion quat = check_quat(L,4);
                bool only_dyn = check_bool(L,5);
                if (lua_type(L,6) != LUA_TFUNCTION)
                        my_lua_error(L,"Parameter 5 should be a function.");

                world->test(col_mesh,pos,quat,only_dyn,lcb);
                lcb.pushResults(L, world, 6, error_handler);
        }
        lua_pop(L,1); // error handler
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



static int pworld_update_graphics (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        self->updateGraphics(L);
        return 0;
TRY_END
}



static int pworld_add_mesh (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        CollisionMeshPtr cmp;
        std::string name = luaL_checkstring(L,2);
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
        std::string name = luaL_checkstring(L,2);
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
        std::string name = luaL_checkstring(L,2);
        self->deleteMesh(name);
        return 0;
TRY_END
}



static int pworld_reload_mesh_by_name (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        std::string name = luaL_checkstring(L,2);
        CollisionMeshPtr cmp = self->getMesh(name);
        cmp->reload(self->getMaterialDB());
        return 0;
TRY_END
}

static int pworld_reload_mesh (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        GET_UD_MACRO(CollisionMeshPtr,cmp,2,COLMESH_TAG);
        cmp->reload(self->getMaterialDB());
        return 0;
TRY_END
}

static int pworld_has_mesh (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        std::string name = luaL_checkstring(L,2);
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


static int pworld_get_gravity (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        push_v3(L, self->getGravity());
        return 1;
TRY_END
}

static int pworld_set_gravity (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        Vector3 gravity = check_v3(L, 2);
        self->setGravity(gravity);
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
                std::string n = luaL_checkstring(L,2);
                cmp = self->getMesh(n);
        } else {
                GET_UD_MACRO(CollisionMeshPtr,cmp2,2,COLMESH_TAG);
                cmp = cmp2;
        }
        Vector3 pos = check_v3(L,3);
        Quaternion quat = check_quat(L,4);
        RigidBodyPtr rbp = (new RigidBody(self,cmp,pos,quat))->getPtr();
        push_rbody(L,rbp);
        return 1;
TRY_END
}

static int pworld_set_material (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        std::string name = luaL_checkstring(L,2);
        unsigned char interaction_group = check_t<unsigned char>(L,3);
        self->setMaterial(name, interaction_group);
        return 0;
TRY_END
}

static int pworld_get_material (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        std::string name = luaL_checkstring(L,2);
        const PhysicsMaterial &m = self->getMaterial(name);
        lua_pushnumber(L, m.interactionGroup);
        return 1;
TRY_END
}

static int pworld_set_interaction_groups (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        if (!lua_istable(L,2))
                my_lua_error(L,"Second parameter should be a table");
        if (!lua_istable(L,3))
                my_lua_error(L,"Third parameter should be a table");

        int counter1 = 0;
        for (lua_pushnil(L) ; lua_next(L,2)!=0 ; lua_pop(L,1)) {
                counter1++;
        }
        int counter2 = 0;
        for (lua_pushnil(L) ; lua_next(L,3)!=0 ; lua_pop(L,1)) {
                counter2++;
        }

        if (counter1 != counter2) {
                my_lua_error(L,"Tables were of different sizes");
        }

        int counter = counter1;

        int num = int(sqrtf(counter)+0.5f);
        if (num*num != counter) {
                my_lua_error(L,"Table was not a square (e.g. 4, 16, 25, etc, elements)");
        }
        
        std::vector<std::pair<float,float> > v;
        v.resize(counter);
        counter = 0;
        for (lua_pushnil(L) ; lua_next(L,2)!=0 ; lua_pop(L,1)) {
                float f = (float)luaL_checknumber(L,-1);
                v[counter].first = f;
                counter++;
        }
        counter = 0;
        for (lua_pushnil(L) ; lua_next(L,3)!=0 ; lua_pop(L,1)) {
                float f = (float)luaL_checknumber(L,-1);
                v[counter].second = f;
                counter++;
        }

        // reflect the other half of the square into place
        for (int ig0=0 ; ig0<num ; ++ig0) {
                for (int ig1=ig0+1 ; ig1<num ; ++ig1) {
                        int code_from = ig1*num + ig0;
                        int code_to = ig0*num + ig1;
                        v[code_to].first = v[code_from].first;
                        v[code_to].second = v[code_from].second;
                }
        }

        self->setInteractionGroups(num, v);

        return 0;
TRY_END
}




TOSTRING_SMART_PTR_MACRO(pworld,PhysicsWorldPtr,PWORLD_TAG)

static int pworld_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"cast")) {
                push_cfunction(L,cast);
        } else if (!::strcmp(key,"test")) {
                push_cfunction(L,pworld_test);
        } else if (!::strcmp(key,"pump")) {
                push_cfunction(L,pworld_pump);
        } else if (!::strcmp(key,"updateGraphics")) {
                push_cfunction(L,pworld_update_graphics);
        } else if (!::strcmp(key,"addMesh")) {
                push_cfunction(L,pworld_add_mesh);
        } else if (!::strcmp(key,"getMesh")) {
                push_cfunction(L,pworld_get_mesh);
        } else if (!::strcmp(key,"removeMesh")) {
                push_cfunction(L,pworld_remove_mesh);
        } else if (!::strcmp(key,"hasMesh")) {
                push_cfunction(L,pworld_has_mesh);
        } else if (!::strcmp(key,"reloadMeshByName")) {
                push_cfunction(L,pworld_reload_mesh_by_name);
        } else if (!::strcmp(key,"reloadMesh")) {
                push_cfunction(L,pworld_reload_mesh);
        } else if (!::strcmp(key,"clearMeshes")) {
                push_cfunction(L,pworld_clear_meshes);
        } else if (!::strcmp(key,"meshes")) {
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
        } else if (!::strcmp(key,"addRigidBody")) {
                push_cfunction(L,pworld_add_rigid_body);
        } else if (!::strcmp(key,"gravity")) {
                push_v3(L, self->getGravity());
        } else if (!::strcmp(key,"setGravity")) {
                push_cfunction(L,pworld_set_gravity);
        } else if (!::strcmp(key,"getGravity")) {
                push_cfunction(L,pworld_get_gravity);
        } else if (!::strcmp(key,"stepSize")) {
                lua_pushnumber(L,self->getStepSize());
        } else if (!::strcmp(key,"maxSteps")) {
                lua_pushnumber(L,self->getMaxSteps());
        } else if (!::strcmp(key,"contactBreakingThreshold")) {
                lua_pushnumber(L,self->getContactBreakingThreshold());
        } else if (!::strcmp(key,"deactivationTime")) {
                lua_pushnumber(L,self->getDeactivationTime());
        } else if (!::strcmp(key,"gimpactOneWayMeshHack")) {
                lua_pushboolean(L,self->gimpactOneWayMeshHack);
        } else if (!::strcmp(key,"bumpyTriangleMeshHack")) {
                lua_pushboolean(L,self->bumpyTriangleMeshHack);
        } else if (!::strcmp(key,"useTriangleEdgeInfo")) {
                lua_pushboolean(L,self->useTriangleEdgeInfo);
        } else if (!::strcmp(key,"verboseContacts")) {
                lua_pushboolean(L,self->verboseContacts);
        } else if (!::strcmp(key,"verboseCasts")) {
                lua_pushboolean(L,self->verboseCasts);
        } else if (!::strcmp(key,"errorContacts")) {
                lua_pushboolean(L,self->errorContacts);
        } else if (!::strcmp(key,"setMaterial")) {
                push_cfunction(L,pworld_set_material);
        } else if (!::strcmp(key,"getMaterial")) {
                push_cfunction(L,pworld_get_material);
        } else if (!::strcmp(key,"setInteractionGroups")) {
                push_cfunction(L,pworld_set_interaction_groups);
        } else if (!::strcmp(key,"solverDamping")) {
                lua_pushnumber(L,self->getSolverDamping());
        } else if (!::strcmp(key,"solverIterations")) {
                lua_pushnumber(L,self->getSolverIterations());
        } else if (!::strcmp(key,"solverErp")) {
                lua_pushnumber(L,self->getSolverErp());
        } else if (!::strcmp(key,"solverErp2")) {
                lua_pushnumber(L,self->getSolverErp2());
        } else if (!::strcmp(key,"solverSplitImpulseThreshold")) {
                lua_pushnumber(L,self->getSolverSplitImpulseThreshold());
        } else if (!::strcmp(key,"solverLinearSlop")) {
                lua_pushnumber(L,self->getSolverLinearSlop());
        } else if (!::strcmp(key,"solverWarmStartingFactor")) {
                lua_pushnumber(L,self->getSolverWarmStartingFactor());
        } else if (!::strcmp(key,"solverSplitImpulse")) {
                lua_pushboolean(L,self->getSolverSplitImpulse());
        } else if (!::strcmp(key,"solverRandomiseOrder")) {
                lua_pushboolean(L,self->getSolverRandomiseOrder());
        } else if (!::strcmp(key,"solverFrictionSeparate")) {
                lua_pushboolean(L,self->getSolverFrictionSeparate());
        } else if (!::strcmp(key,"solverUseWarmStarting")) {
                lua_pushboolean(L,self->getSolverUseWarmStarting());
        } else if (!::strcmp(key,"solverCacheFriendly")) {
                lua_pushboolean(L,self->getSolverCacheFriendly());
        } else if (!::strcmp(key,"draw")) {
                push_cfunction(L,pworld_draw);
        } else {
                my_lua_error(L,"Not a valid PhysicsWorld member: "+std::string(key));
        }
        return 1;
TRY_END
}

static int pworld_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(PhysicsWorldPtr,self,1,PWORLD_TAG);
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"gravity")) {
                Vector3 v = check_v3(L,3);
                self->setGravity(v);
        } else if (!::strcmp(key,"maxSteps")) {
                int v = check_t<int>(L,3);
                self->setMaxSteps(v);
        } else if (!::strcmp(key,"stepSize")) {
                float v = luaL_checknumber(L,3);
                self->setStepSize(v);
        } else if (!::strcmp(key,"contactBreakingThreshold")) {
                float v = luaL_checknumber(L,3);
                self->setContactBreakingThreshold(v);
        } else if (!::strcmp(key,"deactivationTime")) {
                float v = luaL_checknumber(L,3);
                self->setDeactivationTime(v);
        } else if (!::strcmp(key,"gimpactOneWayMeshHack")) {
                bool v = check_bool(L,3);
                self->gimpactOneWayMeshHack = v;
        } else if (!::strcmp(key,"bumpyTriangleMeshHack")) {
                bool v = check_bool(L,3);
                self->bumpyTriangleMeshHack = v;
        } else if (!::strcmp(key,"useTriangleEdgeInfo")) {
                bool v = check_bool(L,3);
                self->useTriangleEdgeInfo = v;
        } else if (!::strcmp(key,"verboseContacts")) {
                bool v = check_bool(L,3);
                self->verboseContacts = v;
        } else if (!::strcmp(key,"verboseCasts")) {
                bool v = check_bool(L,3);
                self->verboseCasts = v;
        } else if (!::strcmp(key,"errorContacts")) {
                bool v = check_bool(L,3);
                self->errorContacts = v;
        } else if (!::strcmp(key,"solverDamping")) {
                float v = luaL_checknumber(L,3);
                self->setSolverDamping(v);
        } else if (!::strcmp(key,"solverIterations")) {
                int v = check_t<int>(L,3);
                self->setSolverIterations(v);
        } else if (!::strcmp(key,"solverErp")) {
                float v = luaL_checknumber(L,3);
                self->setSolverErp(v);
        } else if (!::strcmp(key,"solverErp2")) {
                float v = luaL_checknumber(L,3);
                self->setSolverErp2(v);
        } else if (!::strcmp(key,"solverSplitImpulseThreshold")) {
                float v = luaL_checknumber(L,3);
                self->setSolverSplitImpulseThreshold(v);
        } else if (!::strcmp(key,"solverLinearSlop")) {
                float v = luaL_checknumber(L,3);
                self->setSolverLinearSlop(v);
        } else if (!::strcmp(key,"solverWarmStartingFactor")) {
                float v = luaL_checknumber(L,3);
                self->setSolverWarmStartingFactor(v);
        } else if (!::strcmp(key,"solverSplitImpulse")) {
                bool v = check_bool(L,3);
                self->setSolverSplitImpulse(v);
        } else if (!::strcmp(key,"solverRandomiseOrder")) {
                bool v = check_bool(L,3);
                self->setSolverRandomiseOrder(v);
        } else if (!::strcmp(key,"solverFrictionSeparate")) {
                bool v = check_bool(L,3);
                self->setSolverFrictionSeparate(v);
        } else if (!::strcmp(key,"solverUseWarmStarting")) {
                bool v = check_bool(L,3);
                self->setSolverUseWarmStarting(v);
        } else if (!::strcmp(key,"solverCacheFriendly")) {
                bool v = check_bool(L,3);
                self->setSolverCacheFriendly(v);
        } else {
                my_lua_error(L,"Not a valid PhysicsWorld member: "+std::string(key));
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(PhysicsWorld,pworld,PWORLD_TAG)

MT_MACRO_NEWINDEX(pworld);

//}}}



// vim: shiftwidth=8:tabstop=8:expandtab
