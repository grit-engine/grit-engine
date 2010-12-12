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

// GFXBODY ============================================================== {{{

void push_gfxbody (lua_State *L, const GfxBodyPtr &self)
{
        if (self.isNull())
                lua_pushnil(L);
        else
                push(L,new GfxBodyPtr(self),RBODY_TAG);
}

static int gfxbody_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(GfxBodyPtr,self,1,RBODY_TAG,0);
        delete self;
        return 0;
TRY_END
}

static int gfxbody_scatter (lua_State *L)
{
TRY_START
        check_args(L,11);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
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

static int gfxbody_ranged_scatter (lua_State *L)
{
TRY_START
        check_args(L,16);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
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

static int gfxbody_force (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==7) {
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_V3(force,2);
                GET_V3(rel_pos,5);
                const Vector3 &pos = self->getPosition();
                self->force(force,rel_pos-pos);
        } else if (lua_gettop(L)==4) {
                // local
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Vector3,force,2,VECTOR3_TAG);
                GET_UD_MACRO(Vector3,rel_pos,3,VECTOR3_TAG);
                bool world_orientation = check_bool(L,4);
                if (world_orientation) {
                        self->force(force,rel_pos);
                } else {
                        self->force(force,self->getOrientation()*rel_pos);
                }
        } else if (lua_gettop(L)==3) {
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Vector3,force,2,VECTOR3_TAG);
                GET_UD_MACRO(Vector3,wpos,3,VECTOR3_TAG);
                const Vector3 &pos = self->getPosition();
                self->force(force,wpos-pos);
        } else {
                check_args(L,2);
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Vector3,force,2,VECTOR3_TAG);
                self->force(force);
        }
        return 0;
TRY_END
}

static int gfxbody_impulse (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==7) {
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_V3(impulse,2);
                GET_V3(rel_pos,5);
                self->impulse(impulse,rel_pos);
        } else if (lua_gettop(L)==4) {
                // local
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Vector3,impulse,2,VECTOR3_TAG);
                GET_UD_MACRO(Vector3,rel_pos,3,VECTOR3_TAG);
                bool world_orientation = check_bool(L,4);
                if (world_orientation) {
                        self->impulse(impulse,rel_pos);
                } else {
                        self->impulse(impulse,self->getOrientation()*rel_pos);
                }
        } else if (lua_gettop(L)==3) {
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Vector3,impulse,2,VECTOR3_TAG);
                GET_UD_MACRO(Vector3,wpos,3,VECTOR3_TAG);
                const Vector3 &pos = self->getPosition();
                self->impulse(impulse,wpos - pos);
        } else {
                check_args(L,2);
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_UD_MACRO(Vector3,impulse,2,VECTOR3_TAG);
                self->impulse(impulse);
        }
        return 0;
TRY_END
}


static int gfxbody_torque_impulse (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        GET_V3(torque,2);
        self->torqueImpulse(torque);
        return 0;
TRY_END
}


static int gfxbody_torque (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        GET_V3(torque,2);
        self->torque(torque);
        return 0;
TRY_END
}

static int gfxbody_get_part_enabled (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        lua_pushboolean(L, self->getElementEnabled(i));
        return 1;
TRY_END
}

static int gfxbody_set_part_enabled (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        bool b = check_bool(L, 3);
        self->setElementEnabled(i, b);
        return 0;
TRY_END
}

static int gfxbody_get_part_position_initial (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Vector3 v = self->getElementPositionMaster(i);
        lua_pushnumber(L,v.x); lua_pushnumber(L,v.y); lua_pushnumber(L,v.z);
        return 3;
TRY_END
}

static int gfxbody_get_part_position_offset (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Vector3 v = self->getElementPositionOffset(i);
        lua_pushnumber(L,v.x); lua_pushnumber(L,v.y); lua_pushnumber(L,v.z);
        return 3;
TRY_END
}

static int gfxbody_set_part_position_offset (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        lua_Number x = luaL_checknumber(L,3);
        lua_Number y = luaL_checknumber(L,4);
        lua_Number z = luaL_checknumber(L,5);
        self->setElementPositionOffset(i, Vector3(x,y,z));
        return 0;
TRY_END
}

static int gfxbody_get_part_orientation_initial (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Quaternion q = self->getElementOrientationMaster(i);
        lua_pushnumber(L,q.w); lua_pushnumber(L,q.x); lua_pushnumber(L,q.y); lua_pushnumber(L,q.z);
        return 4;
TRY_END
}

static int gfxbody_get_part_orientation_offset (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        Quaternion q = self->getElementOrientationOffset(i);
        lua_pushnumber(L,q.w); lua_pushnumber(L,q.x); lua_pushnumber(L,q.y); lua_pushnumber(L,q.z);
        return 4;
TRY_END
}

static int gfxbody_set_part_orientation_offset (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        lua_Number w = luaL_checknumber(L,3);
        lua_Number x = luaL_checknumber(L,4);
        lua_Number y = luaL_checknumber(L,5);
        lua_Number z = luaL_checknumber(L,6);
        self->setElementOrientationOffset(i, Quaternion(w,x,y,z));
        return 0;
TRY_END
}

static int gfxbody_set_part_orientation_offset_angle (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        int i = check_int(L, 2, 0, self->getNumElements()-1);
        lua_Number a = luaL_checknumber(L,3);
        lua_Number x = luaL_checknumber(L,4);
        lua_Number y = luaL_checknumber(L,5);
        lua_Number z = luaL_checknumber(L,6);
        self->setElementOrientationOffset(i,Quaternion(Degree(a),Vector3(x,y,z)));
        return 0;
TRY_END
}


static void push_sweep_result (lua_State *L, const SweepResult &r, float len,
                               const PhysicsWorld &world)
{
        lua_pushnumber(L,r.dist * len);
        push_gfxbody(L,r.rb->getPtr());
        // normal is documented as being object space but is actually world space
        Vector3 normal = /*r.rb->getOrientation()* */r.n.normalisedCopy();
        PUT_V3(normal);
        lua_pushstring(L, world.getMaterial(r.material).name.c_str());
}

static int cast (lua_State *L)
{
TRY_START
        check_args_min(L,7);

        // use pointers because we want to assign them later
        GfxBody *body = NULL;
        PhysicsWorld *world;
        if (has_tag(L,1,RBODY_TAG)) {
                body = &***static_cast<GfxBodyPtr**>(lua_touserdata(L, 1));
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
                GET_UD_MACRO(Vector3,start_,3,VECTOR3_TAG);
                GET_UD_MACRO(Vector3,end_,4,VECTOR3_TAG);
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
                GET_UD_MACRO(Quaternion,startq_,6,QUAT_TAG);
                GET_UD_MACRO(Quaternion,endq_,7,QUAT_TAG);
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
                GET_UD_MACRO(GfxBodyPtr,black,base_line+i,RBODY_TAG);
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
                return 6;
        }
        return lcb.results.size() * 6;
TRY_END
}


static int gfxbody_world_position_xyz (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        const Vector3 &v = self->getPosition();
        lua_pushnumber(L,v.x);
        lua_pushnumber(L,v.y);
        lua_pushnumber(L,v.z);
        return 3;
TRY_END
}


static int gfxbody_world_orientation_wxyz (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        const Quaternion &q = self->getOrientation();
        lua_pushnumber(L,q.w);
        lua_pushnumber(L,q.x);
        lua_pushnumber(L,q.y);
        lua_pushnumber(L,q.z);
        return 4;
TRY_END
}


static int gfxbody_rotate_to_world (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) {
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_V3(a,2);
                Vector3 result = self->getOrientation() * a;
                PUT_V3(result);
                return 3;
        } else {
                check_args(L,5);
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_QUAT(a,2);
                Quaternion result = self->getOrientation() * a;
                PUT_QUAT(result);
                return 4;
        }
TRY_END
}

static int gfxbody_rotate_from_world (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) {
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_V3(a,2);
                Vector3 result = self->getOrientation().inverse() * a;
                PUT_V3(result);
                return 3;
        } else {
                check_args(L,5);
                GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
                GET_QUAT(a,2);
                Quaternion result = self->getOrientation().inverse() * a;
                PUT_QUAT(result);
                return 4;
        }
TRY_END
}

static int gfxbody_local_to_world (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        GET_V3(a,2);
        Vector3 result = self->getPosition() + self->getOrientation() * a;
        PUT_V3(result);
        return 3;
TRY_END
}

static int gfxbody_world_to_local (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        GET_V3(a,2);
        Vector3 result = self->getOrientation().inverse() * (a - self->getPosition());
        PUT_V3(result);
        return 3;
TRY_END
}


static int gfxbody_local_pos (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        GET_UD_MACRO(Vector3,local_pos,2,VECTOR3_TAG);
        Vector3 result = self->getPosition() + self->getOrientation() * local_pos;
        push(L, new Vector3(result), VECTOR3_TAG);
        return 1;
TRY_END
}

static int gfxbody_get_linear_velocity (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        PUT_V3(self->getLinearVelocity());
        return 3;
TRY_END
}

static int gfxbody_set_linear_velocity (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        GET_V3(v,2);
        self->setLinearVelocity(v);
        return 0;
TRY_END
}

static int gfxbody_get_angular_velocity (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        PUT_V3(self->getAngularVelocity());
        return 3;
TRY_END
}

static int gfxbody_set_angular_velocity (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        GET_V3(v,2);
        self->setAngularVelocity(v);
        return 0;
TRY_END
}


static int gfxbody_local_vel (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        GET_V3(pos,2);
        bool world_space = check_bool(L,5);
        Vector3 local_pos = pos;
        if (world_space) {
                local_pos -= self->getPosition();
        }
        Vector3 local_vel = self->getLocalVelocity(local_pos);
        PUT_V3(local_vel);
        return 3;
TRY_END
}


static int gfxbody_activate (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        self->activate();
        return 0;
TRY_END
}


static int gfxbody_deactivate (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        self->deactivate();
        return 0;
TRY_END
}


static int gfxbody_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        self->destroy(L);
        return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxbody,GfxBodyPtr,RBODY_TAG)


static int gfxbody_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"force")) {
                push_cfunction(L,gfxbody_force);
        } else if (!::strcmp(key,"impulse")) {
                push_cfunction(L,gfxbody_impulse);
        } else if (!::strcmp(key,"torque")) {
                push_cfunction(L,gfxbody_torque);
        } else if (!::strcmp(key,"torqueImpulse")) {
                push_cfunction(L,gfxbody_torque_impulse);

        } else if (!::strcmp(key,"procObjMaterials")) {
                std::vector<int> mats;
                self->colMesh->getProcObjMaterials(mats);
                lua_createtable(L, mats.size(), 0);
                for (size_t j=0 ; j<mats.size(); ++j) {
                        lua_pushstring(L, self->world->getMaterial(mats[j]).name.c_str());
                        lua_rawseti(L, -2, j+LUA_ARRAY_BASE);
                }
        } else if (!::strcmp(key,"scatter")) {
                push_cfunction(L,gfxbody_scatter);
        } else if (!::strcmp(key,"rangedScatter")) {
                push_cfunction(L,gfxbody_ranged_scatter);

        } else if (!::strcmp(key,"cast")) {
                push_cfunction(L,cast);

        } else if (!::strcmp(key,"activate")) {
                push_cfunction(L,gfxbody_activate);
        } else if (!::strcmp(key,"deactivate")) {
                push_cfunction(L,gfxbody_deactivate);
        } else if (!::strcmp(key,"destroy")) {
                push_cfunction(L,gfxbody_destroy);

        } else if (!::strcmp(key,"linearSleepThreshold")) {
                lua_pushnumber(L,self->getLinearSleepThreshold());
        } else if (!::strcmp(key,"angularSleepThreshold")) {
                lua_pushnumber(L,self->getAngularSleepThreshold());

        } else if (!::strcmp(key,"worldPosition")) {
                push(L, new Vector3(self->getPosition()), VECTOR3_TAG);
        } else if (!::strcmp(key,"getWorldPositionXYZ")) {
                push_cfunction(L,gfxbody_world_position_xyz);
        } else if (!::strcmp(key,"worldOrientation")) {
                push(L, new Quaternion(self->getOrientation()), QUAT_TAG);
        } else if (!::strcmp(key,"getWorldOrientationWXYZ")) {
                push_cfunction(L,gfxbody_world_orientation_wxyz);

        } else if (!::strcmp(key,"rotateToWorld")) {
                push_cfunction(L,gfxbody_rotate_to_world);
        } else if (!::strcmp(key,"rotateFromWorld")) {
                push_cfunction(L,gfxbody_rotate_from_world);
        } else if (!::strcmp(key,"localToWorld")) {
                push_cfunction(L,gfxbody_local_to_world);
        } else if (!::strcmp(key,"worldToLocal")) {
                push_cfunction(L,gfxbody_world_to_local);

        } else if (!::strcmp(key,"getAngularVelocity")) {
                push_cfunction(L,gfxbody_get_angular_velocity);
        } else if (!::strcmp(key,"getLinearVelocity")) {
                push_cfunction(L,gfxbody_get_linear_velocity);
        } else if (!::strcmp(key,"setAngularVelocity")) {
                push_cfunction(L,gfxbody_set_angular_velocity);
        } else if (!::strcmp(key,"setLinearVelocity")) {
                push_cfunction(L,gfxbody_set_linear_velocity);
        } else if (!::strcmp(key,"linearVelocity")) {
                push(L, new Vector3(self->getLinearVelocity()),
                        VECTOR3_TAG);
        } else if (!::strcmp(key,"angularVelocity")) {
                push(L, new Vector3(self->getAngularVelocity()),
                        VECTOR3_TAG);
        } else if (!::strcmp(key,"getLocalPosition")) {
                push_cfunction(L,gfxbody_local_pos);
        } else if (!::strcmp(key,"getLocalVelocity")) {
                push_cfunction(L,gfxbody_local_vel);

        } else if (!::strcmp(key,"contactProcessingThreshold")) {
                lua_pushnumber(L,self->getContactProcessingThreshold());

        } else if (!::strcmp(key,"linearDamping")) {
                lua_pushnumber(L,self->getLinearDamping());
        } else if (!::strcmp(key,"angularDamping")) {
                lua_pushnumber(L,self->getAngularDamping());

        } else if (!::strcmp(key,"mass")) {
                lua_pushnumber(L,self->getMass());
        } else if (!::strcmp(key,"inertia")) {
                push(L,new Vector3(self->getInertia()),VECTOR3_TAG);

        } else if (!::strcmp(key,"numParts")) {
                lua_pushnumber(L, self->getNumElements());
        } else if (!::strcmp(key,"getPartEnabled")) {
                push_cfunction(L,gfxbody_get_part_enabled);
        } else if (!::strcmp(key,"setPartEnabled")) {
                push_cfunction(L,gfxbody_set_part_enabled);
        } else if (!::strcmp(key,"getPartPositionInitial")) {
                push_cfunction(L,gfxbody_get_part_position_initial);
        } else if (!::strcmp(key,"getPartPositionOffset")) {
                push_cfunction(L,gfxbody_get_part_position_offset);
        } else if (!::strcmp(key,"setPartPositionOffset")) {
                push_cfunction(L,gfxbody_set_part_position_offset);
        } else if (!::strcmp(key,"getPartOrientationInitial")) {
                push_cfunction(L,gfxbody_get_part_orientation_initial);
        } else if (!::strcmp(key,"getPartOrientationOffset")) {
                push_cfunction(L,gfxbody_get_part_orientation_offset);
        } else if (!::strcmp(key,"setPartOrientationOffset")) {
                push_cfunction(L,gfxbody_set_part_orientation_offset);
        } else if (!::strcmp(key,"setPartOrientationOffsetAngle")) {
                push_cfunction(L,gfxbody_set_part_orientation_offset_angle);

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
                self->pushUpdateCallback(L);
        } else if (!::strcmp(key,"stepCallback")) {
                self->pushStepCallback(L);
        } else if (!::strcmp(key,"stabiliseCallback")) {
                self->pushStabiliseCallback(L);
        } else {
                my_lua_error(L,"Not a readable GfxBody member: "+std::string(key));
        }
        return 1;
TRY_END
}

static int gfxbody_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxBodyPtr,self,1,RBODY_TAG);
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"linearVelocity")) {
                GET_UD_MACRO(Vector3,v,3,VECTOR3_TAG);
                self->setLinearVelocity(v);
        } else if (!::strcmp(key,"angularVelocity")) {
                GET_UD_MACRO(Vector3,v,3,VECTOR3_TAG);
                self->setAngularVelocity(v);
        } else if (!::strcmp(key,"worldPosition")) {
                GET_UD_MACRO(Vector3,v,3,VECTOR3_TAG);
                self->setPosition(v);
        } else if (!::strcmp(key,"worldOrientation")) {
                GET_UD_MACRO(Quaternion,v,3,QUAT_TAG);
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
        } else if (!::strcmp(key,"updateCallback")) {
                self->setUpdateCallback(L);
        } else if (!::strcmp(key,"stepCallback")) {
                self->setStepCallback(L);
        } else if (!::strcmp(key,"stabiliseCallback")) {
                self->setStabiliseCallback(L);
        } else if (!::strcmp(key,"inertia")) {
                GET_UD_MACRO(Vector3,v,3,VECTOR3_TAG);
                self->setInertia(v);
        } else if (!::strcmp(key,"owner")) {
                if (lua_isnil(L,3)) {
                        self->owner.setNull();
                } else {
                        GET_UD_MACRO(GritObjectPtr,v,3,GRITOBJ_TAG);
                        self->owner = v;
                }

        } else {
               my_lua_error(L,"Not a writeable GfxBody member: "+std::string(key));
        }
        return 0;
TRY_END
}

EQ_MACRO(GfxBodyPtr,gfxbody,RBODY_TAG)

MT_MACRO_NEWINDEX(gfxbody);

//}}}





// vim: shiftwidth=8:tabstop=8:expandtab
