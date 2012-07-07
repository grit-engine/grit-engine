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

#include "../gfx/Clutter.h"
#include "../lua_wrappers_gritobj.h"
#include "../main.h"
#include "../path_util.h"
#include "../lua_wrappers_disk_resource.h"

#include "../gfx/gfx.h"
#include "../gfx/lua_wrappers_scnmgr.h"
#include "../gfx/lua_wrappers_mesh.h"
#include "../gfx/lua_wrappers_mobj.h"

#include "PhysicsWorld.h"
#include "CollisionMesh.h"
#include "lua_wrappers_physics.h"


// Sweep Callback {{{

struct SweepResult { RigidBody *rb; float dist; Vector3 n; int material; };
typedef std::vector<SweepResult> SweepResults;

class LuaSweepCallback : public SweepCallback {
    public:           
        LuaSweepCallback (unsigned long flags)
              : ignoreDynamic((flags & 0x1) != 0)
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


#define RBODY_TAG "Grit/RigidBody"


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
        self->colMesh->scatter(phys_mats.getMaterial(mat)->id,
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
        unsigned triangles  = check_t<unsigned>(L,15);
        bool tangents       = check_bool(L,16);

        Transform world_trans;
        world_trans.p = self->getPosition();
        world_trans.r = self->getOrientation();

        std::stringstream triangles_;
        triangles_ << triangles;

        Ogre::NameValuePairList ps;
        ps["triangles"] = triangles_.str();
        ps["tangents"] = (tangents?"true":"false");
        RangedClutter *ranged_r =
                static_cast<RangedClutter*>(sm.createMovableObject(name, "RangedClutter", &ps));
        ranged_r->setNextMesh(mesh);
        self->colMesh->scatter(phys_mats.getMaterial(mat)->id,
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
        int i = (int)check_int(L, 2, 0, self->getNumElements()-1);
        lua_pushboolean(L, self->getElementEnabled(i));
        return 1;
TRY_END
}

static int rbody_set_part_enabled (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RigidBodyPtr,self,1,RBODY_TAG);
        int i = (int)check_int(L, 2, 0, self->getNumElements()-1);
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
        int i = (int)check_int(L, 2, 0, self->getNumElements()-1);
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
        int i = (int)check_int(L, 2, 0, self->getNumElements()-1);
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
        int i = (int)check_int(L, 2, 0, self->getNumElements()-1);
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
        int i = (int)check_int(L, 2, 0, self->getNumElements()-1);
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
        int i = (int)check_int(L, 2, 0, self->getNumElements()-1);
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
        int i = (int)check_int(L, 2, 0, self->getNumElements()-1);
        Quaternion q = check_quat(L,3);
        self->setElementOrientationOffset(i, q);
        return 0;
TRY_END
}


static void push_sweep_result (lua_State *L, const SweepResult &r)
{
        lua_pushnumber(L,r.dist);
        push_rbody(L,r.rb->getPtr());
        // normal is documented as being object space but is actually world space
        Vector3 normal = /*r.rb->getOrientation()* */r.n.normalisedCopy();
        push_v3(L, normal);
        lua_pushstring(L, phys_mats.getMaterial(r.material)->name.c_str());
}

static int push_cast_results (lua_State *L, const LuaSweepCallback &lcb, bool nearest_only)
{
        if (lcb.results.size()==0) return 0;

        SweepResult nearest;
        nearest.dist = FLT_MAX;
        if (!nearest_only) lua_checkstack(L, lcb.results.size()*4);
        for (size_t i=0 ; i<lcb.results.size() ; ++i) {
                const SweepResult &r = lcb.results[i];
                if (nearest_only) {
                        if (r.dist<nearest.dist) nearest = r;
                } else {
                        push_sweep_result(L, r);
                }
        }
        if (nearest_only) {
                // push nearest
                push_sweep_result(L, nearest);
                return 4;
        }
        return lcb.results.size() * 4;
}

static void init_cast_blacklist (lua_State *L, int base_line, LuaSweepCallback &lcb)
{
        int blacklist_number = lua_gettop(L) - base_line;
        for (int i=1 ; i<=blacklist_number ; ++i) {
                GET_UD_MACRO(RigidBodyPtr,black,base_line+i,RBODY_TAG);
                lcb.blacklist.insert(&*black);
        }
}

static int global_physics_cast_ray (lua_State *L)
{
TRY_START
        int base_line = 4;
        check_args_min(L, base_line);

        Vector3 start = check_v3(L,1);
        Vector3 ray = check_v3(L,2);
        bool nearest_only = check_bool(L,3);
        unsigned long flags = check_t<unsigned long>(L,4);

        LuaSweepCallback lcb(flags);
        init_cast_blacklist(L, base_line, lcb);

        physics_ray(start, start+ray, lcb);

        return push_cast_results(L, lcb, nearest_only);

TRY_END
}


static int global_physics_sweep_sphere (lua_State *L)
{
TRY_START
        int base_line = 5;
        check_args_min(L, base_line);

        float radius = (float)luaL_checknumber(L,1);
        Vector3 start = check_v3(L,2);
        Vector3 ray = check_v3(L,3);
        bool nearest_only = check_bool(L,4);
        unsigned long flags = check_t<unsigned long>(L,5);

        LuaSweepCallback lcb(flags);
        init_cast_blacklist(L, base_line, lcb);

        physics_sweep_sphere(start, start+ray, lcb, radius);

        return push_cast_results(L, lcb, nearest_only);

TRY_END
}


static int global_physics_sweep_box (lua_State *L)
{
TRY_START
        int base_line = 6;
        check_args_min(L, base_line);

        Vector3 size = check_v3(L,1);
        Quaternion q = check_quat(L,2);
        Vector3 start = check_v3(L,3);
        Vector3 ray = check_v3(L,4);
        bool nearest_only = check_bool(L,5);
        unsigned long flags = check_t<unsigned long>(L,6);

        LuaSweepCallback lcb(flags);
        init_cast_blacklist(L, base_line, lcb);

        physics_sweep_box(start, q, start+ray, lcb, size);

        return push_cast_results(L, lcb, nearest_only);

TRY_END
}


static int global_physics_sweep_cylinder (lua_State *L)
{
TRY_START
        int base_line = 7;
        check_args_min(L, base_line);

        float radius = (float)luaL_checknumber(L,1);
        float height = (float)luaL_checknumber(L,2);
        Quaternion q = check_quat(L,3);
        Vector3 start = check_v3(L,4);
        Vector3 ray = check_v3(L,5);
        bool nearest_only = check_bool(L,6);
        unsigned long flags = check_t<unsigned long>(L,7);

        LuaSweepCallback lcb(flags);
        init_cast_blacklist(L, base_line, lcb);

        physics_sweep_cylinder(start, q, start+ray, lcb, radius, height);

        return push_cast_results(L, lcb, nearest_only);

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
                        lua_pushstring(L, phys_mats.getMaterial(mats[j])->name.c_str());
                        lua_rawseti(L, -2, j+LUA_ARRAY_BASE);
                }
        } else if (!::strcmp(key,"scatter")) {
                push_cfunction(L,rbody_scatter);
        } else if (!::strcmp(key,"rangedScatter")) {
                push_cfunction(L,rbody_ranged_scatter);

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
                float v = (float)luaL_checknumber(L,3);
                self->setContactProcessingThreshold(v);
        } else if (!::strcmp(key,"linearDamping")) {
                float v = (float)luaL_checknumber(L,3);
                self->setLinearDamping(v);
        } else if (!::strcmp(key,"angularDamping")) {
                float v = (float)luaL_checknumber(L,3);
                self->setAngularDamping(v);
        } else if (!::strcmp(key,"linearSleepThreshold")) {
                float v = (float)luaL_checknumber(L,3);
                self->setLinearSleepThreshold(v);
        } else if (!::strcmp(key,"angularSleepThreshold")) {
                float v = (float)luaL_checknumber(L,3);
                self->setAngularSleepThreshold(v);
        } else if (!::strcmp(key,"mass")) {
                float v = (float)luaL_checknumber(L,3);
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

static int global_physics_draw (lua_State *L)
{
TRY_START
        check_args(L,0);
        physics_draw();
        return 0;
TRY_END
}

class LuaTestCallback : public TestCallback {
    public:

        void result (RigidBody *body, const Vector3 &pos, const Vector3 &wpos,
                     const Vector3 &normal, float penetration, int m)
        {
                resultMap[body].push_back(Result(pos,wpos,normal,penetration,m));
        }

        void pushResults (lua_State *L, int func_index, int err_handler)
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
                                lua_pushstring(L,phys_mats.getMaterial(j->m)->name.c_str());
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

static int global_physics_test (lua_State *L)
{
TRY_START
        LuaTestCallback lcb;
        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);
        if (lua_gettop(L)==5) {
                float radius = lua_tonumber(L,1);
                Vector3 pos = check_v3(L,2);
                bool only_dyn = check_bool(L,3);
                if (lua_type(L,4) != LUA_TFUNCTION)
                        my_lua_error(L,"Parameter 4 should be a function.");

                physics_test_sphere(radius,pos,only_dyn,lcb);
                lcb.pushResults(L, 4, error_handler);
        } else {
                check_args(L,6);
                std::string col_mesh_name = luaL_checkstring(L,1);
                DiskResource *col_mesh_ = disk_resource_get_or_make(col_mesh_name);
                CollisionMesh *col_mesh = dynamic_cast<CollisionMesh*>(col_mesh_);
                if (col_mesh==NULL) my_lua_error(L, "Not a collision mesh: \""+col_mesh_name+"\"");
                if (!col_mesh->isLoaded()) my_lua_error(L, "Not loaded: \""+col_mesh_name+"\"");
                Vector3 pos = check_v3(L,2);
                Quaternion quat = check_quat(L,3);
                bool only_dyn = check_bool(L,4);
                if (lua_type(L,5) != LUA_TFUNCTION)
                        my_lua_error(L,"Parameter 5 should be a function.");

                physics_test(col_mesh,pos,quat,only_dyn,lcb);
                lcb.pushResults(L, 5, error_handler);
        }
        lua_pop(L,1); // error handler
        return 0;
TRY_END
}

static int global_physics_pump (lua_State *L)
{
TRY_START
        check_args(L,1);
        float time_step = (float)luaL_checknumber(L,1);
        int iterations = physics_pump(L,time_step);
        lua_pushnumber(L,iterations);
        return 1;
TRY_END
}

static int global_physics_update_graphics (lua_State *L)
{
TRY_START
        check_args(L,0);
        physics_update_graphics(L);
        return 0;
TRY_END
}

static int global_physics_body_make (lua_State *L)
{
TRY_START
        check_args(L,3);
        std::string n = luaL_checkstring(L,1);
        Vector3 pos = check_v3(L,2);
        Quaternion quat = check_quat(L,3);
        RigidBodyPtr rbp = (new RigidBody(n,pos,quat))->getPtr();
        push_rbody(L,rbp);
        return 1;
TRY_END
}

static int global_physics_get_gravity (lua_State *L)
{
TRY_START
        check_args(L,0);
        Vector3 gravity = Vector3(physics_option(PHYSICS_GRAVITY_X), physics_option(PHYSICS_GRAVITY_Y), physics_option(PHYSICS_GRAVITY_Z));
        push_v3(L, gravity);
        return 1;
TRY_END
}


int global_physics_option (lua_State *L)
{
TRY_START
    if (lua_gettop(L)==2) {
        std::string opt = luaL_checkstring(L,1);
        int t;
        PhysicsBoolOption o0;
        PhysicsIntOption o1;
        PhysicsFloatOption o2;
        physics_option_from_string(opt, t, o0, o1, o2);
        switch (t) {
            case -1: my_lua_error(L,"Unrecognised physics option: \""+opt+"\"");
            case 0: physics_option(o0, check_bool(L,2)); break;
            case 1: physics_option(o1, check_t<int>(L,2)); break;
            case 2: physics_option(o2, (float)luaL_checknumber(L,2)); break;
            default: my_lua_error(L,"Unrecognised type from physics_option_from_string");
        }
        return 0;
    } else {
        check_args(L,1);
        std::string opt = luaL_checkstring(L,1);
        int t;
        PhysicsBoolOption o0;
        PhysicsIntOption o1;
        PhysicsFloatOption o2;
        physics_option_from_string(opt, t, o0, o1, o2);
        switch (t) {
            case -1: my_lua_error(L,"Unrecognised physics option: \""+opt+"\"");
            case 0: lua_pushboolean(L,physics_option(o0)); break;
            case 1: lua_pushnumber(L,physics_option(o1)); break;
            case 2: lua_pushnumber(L,physics_option(o2)); break;
            default: my_lua_error(L,"Unrecognised type from physics_option_from_string");
        }
        return 1;
    }
TRY_END
}


static int global_physics_set_material (lua_State *L)
{
TRY_START
        check_args(L,2);
        std::string name = luaL_checkstring(L,1);
        unsigned char interaction_group = check_t<unsigned char>(L,2);
        phys_mats.setMaterial(name, interaction_group);
        return 0;
TRY_END
}

static int global_physics_get_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        std::string name = luaL_checkstring(L,1);
        lua_pushnumber(L, phys_mats.getMaterial(name)->interactionGroup);
        return 1;
TRY_END
}

static int global_physics_set_interaction_groups (lua_State *L)
{
TRY_START
        check_args(L,2);
        if (!lua_istable(L,1))
                my_lua_error(L,"Second parameter should be a table");
        if (!lua_istable(L,2))
                my_lua_error(L,"Third parameter should be a table");

        int counter1 = 0;
        for (lua_pushnil(L) ; lua_next(L,1)!=0 ; lua_pop(L,1)) {
                counter1++;
        }
        int counter2 = 0;
        for (lua_pushnil(L) ; lua_next(L,2)!=0 ; lua_pop(L,1)) {
                counter2++;
        }

        if (counter1 != counter2) {
                my_lua_error(L,"Tables were of different sizes");
        }

        int counter = counter1;

        int num = int(sqrtf(float(counter))+0.5f);
        if (num*num != counter) {
                my_lua_error(L,"Table was not a square (e.g. 4, 16, 25, etc, elements)");
        }
        
        Interactions v;
        v.resize(counter);
        counter = 0;
        for (lua_pushnil(L) ; lua_next(L,1)!=0 ; lua_pop(L,1)) {
                float f = (float)luaL_checknumber(L,-1);
                v[counter].friction = f;
                counter++;
        }
        counter = 0;
        for (lua_pushnil(L) ; lua_next(L,2)!=0 ; lua_pop(L,1)) {
                float f = (float)luaL_checknumber(L,-1);
                v[counter].restitution = f;
                counter++;
        }

        // reflect the other half of the square into place
        for (int ig0=0 ; ig0<num ; ++ig0) {
                for (int ig1=ig0+1 ; ig1<num ; ++ig1) {
                        int code_from = ig1*num + ig0;
                        int code_to = ig0*num + ig1;
                        v[code_to].friction = v[code_from].friction;
                        v[code_to].restitution = v[code_from].restitution;
                }
        }

        phys_mats.setInteractionGroups(num, v);

        return 0;
TRY_END
}




static const luaL_reg global[] = {
        {"physics_get_material" ,global_physics_get_material},
        {"physics_set_material" ,global_physics_set_material},
        {"physics_set_interaction_groups" ,global_physics_set_interaction_groups},
        {"physics_draw" ,global_physics_draw},
        {"physics_test" ,global_physics_test},
        {"physics_pump" ,global_physics_pump},
        {"physics_update_graphics" ,global_physics_update_graphics},
        {"physics_body_make" ,global_physics_body_make},
        {"physics_get_gravity" ,global_physics_get_gravity},
        {"physics_option" ,global_physics_option},
        {"physics_cast", global_physics_cast_ray},
        {"physics_sweep_sphere", global_physics_sweep_sphere},
        {"physics_sweep_cylinder", global_physics_sweep_cylinder},
        {"physics_sweep_box", global_physics_sweep_box},
        {NULL, NULL}
};



void physics_lua_init (lua_State *L)
{
        #define ADD_MT_MACRO(name,tag) do {\
        luaL_newmetatable(L, tag); \
        luaL_register(L, NULL, name##_meta_table); \
        lua_pop(L,1); } while(0)

        ADD_MT_MACRO(rbody,RBODY_TAG);

        luaL_register(L, "_G", global);

}

// vim: shiftwidth=8:tabstop=8:expandtab
