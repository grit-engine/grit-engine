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

#include <OgreMovableObject.h>
#include <OgreManualObject.h>
#include <OgreEntity.h>
#include <OgreInstancedGeometry.h>
#include <OgreStaticGeometry.h>
#include <OgreCamera.h>
#include <OgreMaterialManager.h>

#include "Clutter.h"
#include "Grit.h"
#include "LuaParticleSystem.h"

#include "lua_wrappers_mobj.h"
#include "lua_wrappers_scnmgr.h"
#include "lua_wrappers_primitives.h"
#include "lua_wrappers_material.h"
#include "lua_wrappers_mesh.h"


#include "lua_userdata_dependency_tracker_funcs.h"

// MOVABLE OBJECT ========================================================== {{{


Ogre::MovableObject *check_mobj(lua_State *L,int index)
{
        Ogre::MovableObject *ptr = NULL;
        if (has_tag(L,index,ENTITY_TAG)) {
                ptr = *static_cast<Ogre::Entity**>(lua_touserdata(L,index));
        } else if (has_tag(L,index,CAM_TAG)) {
                ptr = *static_cast<Ogre::Camera**>(lua_touserdata(L,index));
        } else if (has_tag(L,index,LIGHT_TAG)) {
                ptr = *static_cast<Ogre::Light**>(lua_touserdata(L,index));
        } else if (has_tag(L,index,CLUTTER_TAG)) {
                ptr = *static_cast<MovableClutter**>(lua_touserdata(L,index));
        } else if (has_tag(L,index,RCLUTTER_TAG)) {
                ptr = *static_cast<RangedClutter**>(lua_touserdata(L,index));
        } else if (has_tag(L,index,MANOBJ_TAG)) {
                ptr = *static_cast<Ogre::ManualObject**>(lua_touserdata(L,index));
        } else if (has_tag(L,index,PSYS_TAG)) {
                ptr = *static_cast<Ogre::ParticleSystem**>(lua_touserdata(L,index));
        }

        if (!ptr) {
                std::string acceptable_types;
                acceptable_types += ENTITY_TAG ", or ";
                acceptable_types += CAM_TAG ", or ";
                acceptable_types += LIGHT_TAG ", or ";
                acceptable_types += CLUTTER_TAG ", or ";
                acceptable_types += RCLUTTER_TAG ", or ";
                acceptable_types += MANOBJ_TAG ", or ";
                acceptable_types += PSYS_TAG;
                luaL_typerror(L,index,acceptable_types.c_str());
        }
        return ptr;
}


bool push_mobj (lua_State *L, Ogre::MovableObject *mobj)
{
        if (!mobj) {
                lua_pushnil(L);
                return true;
        } else if (dynamic_cast<Ogre::Entity*>(mobj)) {
                push_entity(L,static_cast<Ogre::Entity*>(mobj));
                return true;
        } else if (dynamic_cast<Ogre::Camera*>(mobj)) {
                push_cam(L,static_cast<Ogre::Camera*>(mobj));
                return true;
        } else if (dynamic_cast<Ogre::Light*>(mobj)) {
                push_light(L,static_cast<Ogre::Light*>(mobj));
                return true;
        } else if (dynamic_cast<MovableClutter*>(mobj)) {
                push_clutter(L,static_cast<MovableClutter*>(mobj));
                return true;
        } else if (dynamic_cast<RangedClutter*>(mobj)) {
                push_rclutter(L,static_cast<RangedClutter*>(mobj));
                return true;
        } else if (dynamic_cast<Ogre::ManualObject*>(mobj)) {
                push_manobj(L,static_cast<Ogre::ManualObject*>(mobj));
                return true;
        } else if (dynamic_cast<Ogre::ParticleSystem*>(mobj)) {
                push_psys(L,static_cast<Ogre::ParticleSystem*>(mobj));
                return true;
        }
        return false;
}

static bool mobj_index (lua_State *L, Ogre::MovableObject &self,
                        const std::string &key)
{
        if (key=="name") {
                lua_pushstring(L,self.getName().c_str());
        } else if (key=="movableType") {
                lua_pushstring(L,self.getMovableType().c_str());
        } else if (key == "parentSceneNode") {
                push_node(L,self.getParentSceneNode());
        } else if (key=="renderingDistance") {
                lua_pushnumber(L,self.getRenderingDistance());
        } else if (key=="castShadows") {
                lua_pushboolean(L,self.getCastShadows());
        } else if (key=="renderQueueGroup") {
                lua_pushnumber(L,self.getRenderQueueGroup());
        } else if (key=="visible") {
                lua_pushboolean(L,self.isVisible());
        } else {
                return false;
        }
        return true;
}

static bool mobj_newindex (lua_State *L, Ogre::MovableObject &self,
                           const std::string &key)
{
        if (key=="renderingDistance") {
                lua_Number n = luaL_checknumber(L,3);
                self.setRenderingDistance(n);
        } else if (key=="renderQueueGroup") {
                lua_Number n = check_t<unsigned char>(L,3);
                self.setRenderQueueGroup(n);
        } else if (key=="castShadows") {
                bool b = check_bool(L,3);
                self.setCastShadows(b);
        } else if (key=="visible") {
                bool b = check_bool(L,3);
                self.setVisible(b);
        } else {
                return false;
        }
        return true;
}

//}}}


// RCLUTTER =================================================================== {{{

void push_rclutter (lua_State *L, RangedClutter *self)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (self);
        luaL_getmetatable(L, RCLUTTER_TAG);
        lua_setmetatable(L, -2);
        Ogre::SceneManager *sm = self->_getManager();
        scnmgr_maps& maps = grit->getUserDataTables().scnmgrs[sm];
        maps.rangedClutters[self].push_back(ud);
}

static int rclutter_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(RangedClutter,self,1,RCLUTTER_TAG,0);
        if (self==NULL) return 0;
        Ogre::SceneManager *sm = self->_getManager();
        vec_nullify_remove(grit->getUserDataTables().scnmgrs[sm].rangedClutters[self],&self);
        return 0;
TRY_END
}

static int rclutter_update (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(RangedClutter,self,1,RCLUTTER_TAG);
        float x = check_float(L,2);
        float y = check_float(L,3);
        float z = check_float(L,4);
        self.update(x,y,z);
        return 0;
TRY_END
}

static int rclutter_get_utilisation (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(RangedClutter,self,1,RCLUTTER_TAG);
        size_t used = 0;
        size_t rendered = 0;
        size_t total = 0;
        self.getUtilisation(used,rendered,total);
        lua_pushnumber(L, used);
        lua_pushnumber(L, rendered);
        lua_pushnumber(L, total);
        return 3;
TRY_END
}

static int rclutter_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(RangedClutter,self,1,RCLUTTER_TAG);
        Ogre::SceneManager *sm = self._getManager();
        sm->destroyMovableObject(&self);
        map_nullify_remove(grit->getUserDataTables().scnmgrs[sm].rangedClutters,&self);
        return 0;
TRY_END
}

TOSTRING_ADDR_MACRO(rclutter,RangedClutter,RCLUTTER_TAG)

static int rclutter_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RangedClutter,self,1,RCLUTTER_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="destroy") {
                push_cfunction(L,rclutter_destroy);
        } else if (key=="update") {
                push_cfunction(L,rclutter_update);
        } else if (key=="getUtilisation") {
                push_cfunction(L,rclutter_get_utilisation);
        } else if (!mobj_index(L,self,key)) {
                my_lua_error(L,"Not a valid RangedClutter member: "+key);
        }
        return 1;
TRY_END
}

static int rclutter_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(RangedClutter,self,1,RCLUTTER_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="autoUpdate") {
                if (check_bool(L,3)) {
                        self.registerMe();
                } else {
                        self.unregisterMe();
                }
        } else if (!mobj_newindex(L,self,key)) {
                my_lua_error(L,"Not a writeable RangedClutter member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(RangedClutter,rclutter,RCLUTTER_TAG)

MT_MACRO_NEWINDEX(rclutter);

//}}}


// CLUTTER =================================================================== {{{

void push_clutter (lua_State *L, MovableClutter *self)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (self);
        luaL_getmetatable(L, CLUTTER_TAG);
        lua_setmetatable(L, -2);
        Ogre::SceneManager *sm = self->_getManager();
        scnmgr_maps& maps = grit->getUserDataTables().scnmgrs[sm];
        maps.movableClutters[self].push_back(ud);
}

static int clutter_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(MovableClutter,self,1,CLUTTER_TAG,0);
        if (self==NULL) return 0;
        Ogre::SceneManager *sm = self->_getManager();
        vec_nullify_remove(grit->getUserDataTables().scnmgrs[sm].movableClutters[self],&self);
        return 0;
TRY_END
}

static int clutter_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(MovableClutter,self,1,CLUTTER_TAG);
        Ogre::SceneManager *sm = self._getManager();
        sm->destroyMovableObject(&self);
        map_nullify_remove(grit->getUserDataTables().scnmgrs[sm].movableClutters,&self);
        return 0;
TRY_END
}

float randf (void) { return float(rand())/RAND_MAX*2 - 1; }
float prandf (void) { return float(rand())/RAND_MAX; }
float prandf2 (void) { float x = prandf(); return 0.5*tan(x * 3.142f/2) + sqrtf(x); }

static int clutter_fuck_with (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_UD_MACRO(MovableClutter,self,1,CLUTTER_TAG);
        std::string matname = luaL_checkstring(L,2);
        float radius = luaL_checknumber(L,3);
        float angle = luaL_checknumber(L,4);
        float sz = luaL_checknumber(L,5);
        float vis = luaL_checknumber(L,6);
        Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().getByName(matname, "GRIT");
        if (m.isNull()) {
                my_lua_error(L, "Could not find material \""+matname+"\"");
        }
        for (int i=0 ; i<100 ; ++i) {
                Ogre::Vector3 centre(randf(), randf(), 0);
                centre = radius * prandf2() * centre.normalisedCopy();
                Ogre::Quaternion rot(Ogre::Degree(180*randf()),Ogre::Vector3(0,0,1));
                rot = rot * Ogre::Quaternion(Ogre::Degree(angle*randf()),Ogre::Vector3(1,0,0));

                ClutterBuffer::QTicket t = self.reserveQuad(m);
                Ogre::Vector3 pos[4] = { centre + rot * Ogre::Vector3(-sz, 0, sz),
                                         centre + rot * Ogre::Vector3( sz, 0, sz),
                                         centre + rot * Ogre::Vector3(-sz, 0, 0),
                                         centre + rot * Ogre::Vector3( sz, 0, 0) };
                Ogre::Vector3 norm[4] = { rot * Ogre::Vector3(0, -1, 0),
                                          rot * Ogre::Vector3(0, -1, 0),
                                          rot * Ogre::Vector3(0, -1, 0),
                                          rot * Ogre::Vector3(0, -1, 0) };
                Ogre::Vector2 uv[4] = { Ogre::Vector2(.0,0),
                                        Ogre::Vector2(.5,0),
                                        Ogre::Vector2(.0,.32),
                                        Ogre::Vector2(.5,.32) };
                Ogre::Vector3 tang[4] = { rot * Ogre::Vector3(1, 0, 0),
                                          rot * Ogre::Vector3(1, 0, 0),
                                          rot * Ogre::Vector3(1, 0, 0),
                                          rot * Ogre::Vector3(1, 0, 0) };
                self.updateQuad(t, pos, norm, uv, &tang, vis);
        }
        return 0;
TRY_END
}

static int clutter_fuck_with2 (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_UD_MACRO(MovableClutter,self,1,CLUTTER_TAG);
        std::string meshname = luaL_checkstring(L,2);
        float radius = luaL_checknumber(L,3);
        float angle = luaL_checknumber(L,4);
        float vis = luaL_checknumber(L,5);
        Ogre::MeshPtr m = Ogre::MeshManager::getSingleton().load(meshname, "GRIT");
        if (m.isNull()) {
                my_lua_error(L, "Could not find mesh \""+meshname+"\"");
        }
        for (int i=0 ; i<1 ; ++i) {
                Ogre::Vector3 centre(randf(), randf(), 0);
                centre = radius * prandf2() * centre.normalisedCopy();
                Ogre::Quaternion rot(Ogre::Degree(180*randf()),Ogre::Vector3(0,0,1));
                rot = rot * Ogre::Quaternion(Ogre::Degree(angle*randf()),Ogre::Vector3(1,0,0));

                ClutterBuffer::MTicket t = self.reserveGeometry(m);
                self.updateGeometry(t, centre, rot, vis);
        }
        return 0;
TRY_END
}

TOSTRING_ADDR_MACRO(clutter,MovableClutter,CLUTTER_TAG)

static int clutter_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(MovableClutter,self,1,CLUTTER_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="destroy") {
                push_cfunction(L,clutter_destroy);
        } else if (key=="fuckWith2") {
                push_cfunction(L,clutter_fuck_with2);
        } else if (key=="fuckWith") {
                push_cfunction(L,clutter_fuck_with);
        } else if (!mobj_index(L,self,key)) {
                my_lua_error(L,"Not a valid MovableClutter member: "+key);
        }
        return 1;
TRY_END
}

static int clutter_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(MovableClutter,self,1,CLUTTER_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="donothing") {
                float v = luaL_checknumber(L,3);
                (void) v;
        } else if (!mobj_newindex(L,self,key)) {
                my_lua_error(L,"Not a writeable MovableClutter member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(MovableClutter,clutter,CLUTTER_TAG)

MT_MACRO_NEWINDEX(clutter);

//}}}


// LIGHT =================================================================== {{{

void push_light (lua_State *L, Ogre::Light *self)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (self);
        luaL_getmetatable(L, LIGHT_TAG);
        lua_setmetatable(L, -2);
        Ogre::SceneManager *sm = self->_getManager();
        scnmgr_maps& maps = grit->getUserDataTables().scnmgrs[sm];
        maps.lights[self].push_back(ud);
}

static int light_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::Light,self,1,LIGHT_TAG,0);
        if (self==NULL) return 0;
        Ogre::SceneManager *sm = self->_getManager();
        vec_nullify_remove(grit->getUserDataTables().scnmgrs[sm].lights[self],&self);
        return 0;
TRY_END
}

static int light_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        Ogre::SceneManager *sm = self._getManager();
        sm->destroyLight(&self);
        map_nullify_remove(grit->getUserDataTables().scnmgrs[sm].lights,&self);
        return 0;
TRY_END
}


static int light_get_diffuse_colour (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        Ogre::ColourValue cv = self.getDiffuseColour();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        return 3;
TRY_END
}

static int light_set_diffuse_colour (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        float r = luaL_checknumber(L,2);
        float g = luaL_checknumber(L,3);
        float b = luaL_checknumber(L,4);
        self.setDiffuseColour(r,g,b);
        return 0;
TRY_END
}


static int light_get_specular_colour (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        Ogre::ColourValue cv = self.getSpecularColour();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        return 3;
TRY_END
}

static int light_set_specular_colour (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        float r = luaL_checknumber(L,2);
        float g = luaL_checknumber(L,3);
        float b = luaL_checknumber(L,4);
        self.setSpecularColour(r,g,b);
        return 0;
TRY_END
}


static int light_get_spotlight_range (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        lua_pushnumber(L,self.getSpotlightInnerAngle().valueDegrees());
        lua_pushnumber(L,self.getSpotlightOuterAngle().valueDegrees());
        lua_pushnumber(L,self.getSpotlightFalloff());
        return 3;
TRY_END
}

static int light_set_spotlight_range (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        float i = luaL_checknumber(L,2);
        float o = luaL_checknumber(L,3);
        float f = luaL_checknumber(L,4);
        self.setSpotlightRange(Ogre::Degree(i),Ogre::Degree(o),f);
        return 0;
TRY_END
}


static int light_get_attenuation (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        lua_pushnumber(L,self.getAttenuationRange());
        lua_pushnumber(L,self.getAttenuationConstant());
        lua_pushnumber(L,self.getAttenuationLinear());
        lua_pushnumber(L,self.getAttenuationQuadric());
        return 4;
TRY_END
}

static int light_set_attenuation (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        float r = luaL_checknumber(L,2);
        float c = luaL_checknumber(L,3);
        float l = luaL_checknumber(L,4);
        float q = luaL_checknumber(L,5);
        self.setAttenuation(r,c,l,q);
        return 0;
TRY_END
}


TOSTRING_ADDR_MACRO(light,Ogre::Light,LIGHT_TAG)

std::string light_type_to_string (lua_State *L, const Ogre::Light::LightTypes t)
{
        switch (t) {
        case Ogre::Light::LT_POINT: return "POINT";
        case Ogre::Light::LT_DIRECTIONAL: return "DIRECTIONAL";
        case Ogre::Light::LT_SPOTLIGHT: return "SPOTLIGHT";
        default:
                my_lua_error(L,"Unrecognised light type.");
        }
        return "";
}

Ogre::Light::LightTypes light_type_from_string (lua_State *L,
                                                const std::string &t)
{
        if (t=="POINT") return Ogre::Light::LT_POINT;
        if (t=="DIRECTIONAL") return Ogre::Light::LT_DIRECTIONAL;
        if (t=="SPOTLIGHT") return Ogre::Light::LT_SPOTLIGHT;
        my_lua_error(L,"Unrecognised light type: "+t);
        return Ogre::Light::LT_POINT; // never gets here
}

static int light_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="destroy") {
                push_cfunction(L,light_destroy);
        } else if (key=="castShadows") {
                lua_pushboolean(L,self.getCastShadows());
        } else if (key=="position") {
                push(L,new Ogre::Vector3(self.getPosition()),VECTOR3_TAG);
        } else if (key=="direction") {
                push(L,new Ogre::Vector3(self.getDirection()),VECTOR3_TAG);
        } else if (key=="getDiffuseColour") {
                push_cfunction(L,light_get_diffuse_colour);
        } else if (key=="setDiffuseColour") {
                push_cfunction(L,light_set_diffuse_colour);
        } else if (key=="getSpecularColour") {
                push_cfunction(L,light_get_specular_colour);
        } else if (key=="setSpecularColour") {
                push_cfunction(L,light_set_specular_colour);
        } else if (key=="getSpotlightRange") {
                push_cfunction(L,light_get_spotlight_range);
        } else if (key=="setSpotlightRange") {
                push_cfunction(L,light_set_spotlight_range);
        } else if (key=="getAttenuation") {
                push_cfunction(L,light_get_attenuation);
        } else if (key=="setAttenuation") {
                push_cfunction(L,light_set_attenuation);
        } else if (key=="powerScale") {
                lua_pushnumber(L, self.getPowerScale());
        } else if (key=="type") {
                lua_pushstring(L,
                               light_type_to_string(L,self.getType()).c_str());
        } else if (!mobj_index(L,self,key)) {
                my_lua_error(L,"Not a valid Light member: "+key);
        }
        return 1;
TRY_END
}

static int light_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::Light,self,1,LIGHT_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="powerScale") {
                float v = luaL_checknumber(L,3);
                self.setPowerScale(v);
        } else if (key=="castShadows") {
                bool b = check_bool(L,3);
                self.setCastShadows(b);
        } else if (key=="position") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self.setPosition(v);
        } else if (key=="direction") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self.setDirection(v);
        } else if (key=="type") {
                self.setType
                        (light_type_from_string(L,luaL_checkstring(L,3)));
        } else if (!mobj_newindex(L,self,key)) {
                my_lua_error(L,"Not a valid Light member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::Light,light,LIGHT_TAG)

MT_MACRO_NEWINDEX(light);

//}}}


// PSYS =================================================================== {{{

Ogre::ParticleSystem *make_psys (Ogre::SceneManager *sm,
                                 const std::string &name)
{
        Ogre::ParticleSystem *psys = sm->createParticleSystem(name);
        psys->setRenderer("LuaParticleRenderer");
        psys->addAffector("LuaParticleAffector");
        psys->setSpeedFactor(0);
        return psys;
}

void push_psys (lua_State *L, Ogre::ParticleSystem *self)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (self);
        luaL_getmetatable(L, PSYS_TAG);
        lua_setmetatable(L, -2);
        Ogre::SceneManager *sm = self->_getManager();
        scnmgr_maps& maps = grit->getUserDataTables().scnmgrs[sm];
        maps.psyss[self].push_back(ud);
}

static int psys_add_particle_ex (lua_State *L, bool err)
{
TRY_START
        check_args(L,17);
        GET_UD_MACRO(Ogre::ParticleSystem,self,1,PSYS_TAG);
        float x = luaL_checknumber(L,2);
        float y = luaL_checknumber(L,3);
        float z = luaL_checknumber(L,4);
        float rot = luaL_checknumber(L,5);
        float w = luaL_checknumber(L,6);
        float h = luaL_checknumber(L,7);
        float r = luaL_checknumber(L,8);
        float g = luaL_checknumber(L,9);
        float b = luaL_checknumber(L,10);
        float a = luaL_checknumber(L,11);
        float dx = luaL_checknumber(L,12);
        float dy = luaL_checknumber(L,13);
        float dz = luaL_checknumber(L,14);
        luaL_checktype(L,15,LUA_TFUNCTION);
        lua_pushvalue(L,15); //func
        lua_pushvalue(L,16); //arg
        float advance = luaL_checknumber(L,17);
        Ogre::Particle *p = self.createParticle();
        if (p==NULL) {
                if (err) {
                        my_lua_error(L, "No more particles");
                } else {
                        return 0;
                }
        }
        p->position = Ogre::Vector3(x,y,z);
        p->direction = Ogre::Vector3(dx,dy,dz);
        p->setRotation(Ogre::Degree(rot));
        p->setDimensions(w,h);
        p->colour = Ogre::ColourValue(r,g,b,a);
        p->particleType = Ogre::Particle::Visual;
        p->timeToLive = 1;
        LuaParticleData *pd = static_cast<LuaParticleData*>(p->getVisualData());
        pd->age = 0;
        pd->setFunctionArg(L);
        (void) advance;
        return 0;
TRY_END
}

static int psys_add_particle (lua_State *L)
{
TRY_START
        return psys_add_particle_ex(L, true);
TRY_END
}

static int psys_try_add_particle (lua_State *L)
{
TRY_START
        return psys_add_particle_ex(L, false);
TRY_END
}

static int psys_update (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::ParticleSystem,self,1,PSYS_TAG);
        float t = luaL_checknumber(L,2);
        self._update(t);
        return 0;
TRY_END
}

static int psys_clear (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::ParticleSystem,self,1,PSYS_TAG);
        self.clear();
        return 0;
TRY_END
}

static int psys_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::ParticleSystem,self,1,PSYS_TAG,0);
        if (self==NULL) return 0;
        Ogre::SceneManager *sm = self->_getManager();
        vec_nullify_remove(grit->getUserDataTables().scnmgrs[sm].psyss[self],&self);
        return 0;
TRY_END
}

static int psys_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::ParticleSystem,self,1,PSYS_TAG);
        Ogre::SceneManager *sm = self._getManager();
        sm->destroyParticleSystem(&self);
        map_nullify_remove(grit->getUserDataTables().scnmgrs[sm].psyss,&self);
        return 0;
TRY_END
}




TOSTRING_ADDR_MACRO(psys,Ogre::ParticleSystem,PSYS_TAG)

static int psys_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::ParticleSystem,self,1,PSYS_TAG);
        LuaParticleRenderer *renderer=
                static_cast<LuaParticleRenderer*>(self.getRenderer());
        std::string key = luaL_checkstring(L,2);
        if (key=="destroy") {
                push_cfunction(L,psys_destroy);
        } else if (key=="quota") {
                lua_pushnumber(L, self.getParticleQuota());
        } else if (key=="activeParticles") {
                lua_pushnumber(L, self.getNumParticles());
        } else if (key=="sortingEnabled") {
                lua_pushboolean(L, self.getSortingEnabled());
        } else if (key=="accurateFacing") {
                lua_pushboolean(L, renderer->getUseAccurateFacing());
        } else if (key=="pointRenderingEnabled") {
                lua_pushboolean(L, renderer->isPointRenderingEnabled());
        } else if (key=="rotateVertexes") {
                bool b = renderer->getBillboardRotationType()==Ogre::BBR_VERTEX;
                lua_pushboolean(L, b);
        } else if (key=="materialName") {
                lua_pushstring(L, self.getMaterialName().c_str());
        } else if (key=="addParticle") {
                push_cfunction(L, psys_add_particle);
        } else if (key=="tryAddParticle") {
                push_cfunction(L, psys_try_add_particle);
        } else if (key=="clear") {
                push_cfunction(L, psys_clear);
        } else if (key=="update") {
                push_cfunction(L, psys_update);
        } else if (!mobj_index(L,self,key)) {
                my_lua_error(L,"Not a readable ParticleSystem member: "+key);
        }
        return 1;
TRY_END
}

static int psys_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::ParticleSystem,self,1,PSYS_TAG);
        LuaParticleRenderer *renderer=
                static_cast<LuaParticleRenderer*>(self.getRenderer());
        std::string key = luaL_checkstring(L,2);
        if (key=="quota") {
                float v = luaL_checknumber(L,3);
                self.setParticleQuota(v);
        } else if (key=="sortingEnabled") {
                bool b = check_bool(L,3);
                self.setSortingEnabled(b);
        } else if (key=="accurateFacing") {
                bool b = check_bool(L,3);
                renderer->setUseAccurateFacing(b);
        } else if (key=="pointRenderingEnabled") {
                bool b = check_bool(L,3);
                renderer->setPointRenderingEnabled(b);
        } else if (key=="rotateVertexes") {
                bool b = check_bool(L,3);
                renderer->setBillboardRotationType(b?Ogre::BBR_VERTEX
                                                    :Ogre::BBR_TEXCOORD);
        } else if (key=="materialName") {
                std::string v = luaL_checkstring(L,3);
                self.setMaterialName(v);
        } else if (!mobj_newindex(L,self,key)) {
                my_lua_error(L,"Not a writeable ParticleSystem member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::ParticleSystem,psys,PSYS_TAG)

MT_MACRO_NEWINDEX(psys);

//}}}


// MANUAL OBJECT =========================================================== {{{

void push_manobj(lua_State *L, Ogre::ManualObject *man)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (man);
        Ogre::SceneManager *scnmgr = man->_getManager();
        luaL_getmetatable(L, MANOBJ_TAG);
        lua_setmetatable(L, -2);
        grit->getUserDataTables().scnmgrs[scnmgr].manobjs[man].push_back(ud);
}


static int manobj_get_polygon_mode_overrideable (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::ManualObject,self,1,MANOBJ_TAG);
        unsigned n = check_t<unsigned>(L,2);
        Ogre::ManualObject::ManualObjectSection *s = self.getSection(n);
        lua_pushboolean(L,s->getPolygonModeOverrideable());
        return 1;
TRY_END
}

static int manobj_set_polygon_mode_overrideable (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::ManualObject,self,1,MANOBJ_TAG);
        unsigned n = check_t<unsigned>(L,2);
        bool b = check_bool(L,3);
        Ogre::ManualObject::ManualObjectSection *s = self.getSection(n);
        s->setPolygonModeOverrideable(b);
        return 0;
TRY_END
}


static int manobj_get_material (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::ManualObject,self,1,MANOBJ_TAG);
        unsigned n = check_t<unsigned>(L,2);
        Ogre::ManualObject::ManualObjectSection *s = self.getSection(n);
        push(L,new Ogre::MaterialPtr(s->getMaterial()),MAT_TAG);
        return 1;
TRY_END
}

static int manobj_set_material (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::ManualObject,self,1,MANOBJ_TAG);
        unsigned n = check_t<unsigned>(L,2);
        std::string name = luaL_checkstring(L,3);
        Ogre::ManualObject::ManualObjectSection *s = self.getSection(n);
        s->setMaterialName(name);
        return 0;
TRY_END
}


static int manobj_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::ManualObject,self,1,MANOBJ_TAG);
        Ogre::SceneManager *scnmgr = self._getManager();
        scnmgr->destroyManualObject(&self);
        map_nullify_remove(grit->getUserDataTables().scnmgrs[scnmgr].manobjs,
                           &self);
        return 0;
TRY_END
}


TOSTRING_GETNAME_MACRO(manobj,Ogre::ManualObject,.getName(),MANOBJ_TAG)

static int manobj_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::ManualObject,ent,1,MANOBJ_TAG,0);
        if (ent==NULL) return 0;
        Ogre::SceneManager *scnmgr = ent->_getManager();
        vec_nullify_remove(
                       grit->getUserDataTables().scnmgrs[scnmgr].manobjs[ent],
                           &ent);
        return 0;
TRY_END
}

static int manobj_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::ManualObject,self,1,MANOBJ_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key == "numSubSections") {
                lua_pushnumber(L,self.getNumSections());
        } else if (key == "getPolygonModeOverrideable") {
                push_cfunction(L,manobj_get_polygon_mode_overrideable);
        } else if (key == "setPolygonModeOverrideable") {
                push_cfunction(L,manobj_set_polygon_mode_overrideable);
        } else if (key == "getMaterial") {
                push_cfunction(L,manobj_get_material);
        } else if (key == "setMaterial") {
                push_cfunction(L,manobj_set_material);
        } else if (key == "destroy") {
                push_cfunction(L,manobj_destroy);
        } else if (!mobj_index(L,self,key)) {
                my_lua_error(L,"Not a valid ManualObject member: " + key);
        }
        return 1;
TRY_END
}

static int manobj_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::ManualObject,self,1,MANOBJ_TAG);
        std::string key = luaL_checkstring(L,2);
        if (false) {
        } else if (!mobj_newindex(L,self,key)) {
                my_lua_error(L,"Not a valid ManualObject member: " + key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::ManualObject,manobj,MANOBJ_TAG)

MT_MACRO_NEWINDEX(manobj);

//}}}


// ENTITY ================================================================== {{{

void push_entity(lua_State *L, Ogre::Entity *ent)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (ent);
        Ogre::SceneManager *scnmgr = ent->_getManager();
        luaL_getmetatable(L, ENTITY_TAG);
        lua_setmetatable(L, -2);
        grit->getUserDataTables().scnmgrs[scnmgr].entities[ent].push_back(ud);
}


static int entity_get_polygon_mode_overrideable (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        unsigned n = check_t<unsigned>(L,2);
        Ogre::SubEntity *se = self.getSubEntity(n);
        lua_pushboolean(L,se->getPolygonModeOverrideable());
        return 1;
TRY_END
}

static int entity_set_polygon_mode_overrideable (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        unsigned n = check_t<unsigned>(L,2);
        bool b = check_bool(L,3);
        Ogre::SubEntity *se = self.getSubEntity(n);
        se->setPolygonModeOverrideable(b);
        return 0;
TRY_END
}


static int entity_get_material (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        unsigned n = check_t<unsigned>(L,2);
        Ogre::SubEntity *se = self.getSubEntity(n);
        push(L,new Ogre::MaterialPtr(se->getMaterial()),MAT_TAG);
        return 1;
TRY_END
}

static int entity_get_material_name (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        unsigned n = check_t<unsigned>(L,2);
        Ogre::SubEntity *se = self.getSubEntity(n);
        lua_pushstring(L,se->getMaterialName().c_str());
        return 1;
TRY_END
}

static int entity_set_material (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        unsigned n = check_t<unsigned>(L,2);
        std::string name = luaL_checkstring(L,3);
        Ogre::SubEntity *se = self.getSubEntity(n);
        se->setMaterialName(name);
        return 0;
TRY_END
}


static int entity_set_custom_parameter (lua_State *L)
{
TRY_START
        check_args(L,7);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        unsigned n = check_t<unsigned>(L,2);
        unsigned varindex = check_t<unsigned>(L,3);
        float v1 = luaL_checknumber(L,4);
        float v2 = luaL_checknumber(L,5);
        float v3 = luaL_checknumber(L,6);
        float v4 = luaL_checknumber(L,7);
        Ogre::SubEntity *se = self.getSubEntity(n);
        se->setCustomParameter(varindex,Ogre::Vector4(v1,v2,v3,v4));
        return 0;
TRY_END
}

struct Hack : public Ogre::SubEntity {
        bool hasCustomParameter (size_t index);
};

inline bool Hack::hasCustomParameter (size_t index)
{
        CustomParameterMap::const_iterator i = mCustomParameters.find(index);
        return i != mCustomParameters.end();
}

static int entity_set_custom_parameter_all (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) {
                GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
                unsigned varindex = check_t<unsigned>(L,2);
                unsigned elindex = check_t<unsigned>(L,3,0,3);
                float v = luaL_checknumber(L,4);
                for (size_t i=0 ; i<self.getNumSubEntities() ; ++i) {
                        Ogre::SubEntity *se = self.getSubEntity(i);
                        // Undefined behaviour here
                        Ogre::Vector4 cp = static_cast<Hack*>(se)->hasCustomParameter(varindex) ? se->getCustomParameter(varindex) : Ogre::Vector4(0,0,0,0);
                        switch (elindex) {
                                case 0: cp.x = v; break;
                                case 1: cp.y = v; break;
                                case 2: cp.z = v; break;
                                case 3: cp.w = v; break;
                        }
                        se->setCustomParameter(varindex,cp);
                }
        } else {
                check_args(L,6);
                GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
                unsigned varindex = check_t<unsigned>(L,2);
                float v1 = luaL_checknumber(L,3);
                float v2 = luaL_checknumber(L,4);
                float v3 = luaL_checknumber(L,5);
                float v4 = luaL_checknumber(L,6);
                for (size_t i=0 ; i<self.getNumSubEntities() ; ++i) {
                        Ogre::SubEntity *se = self.getSubEntity(i);
                        se->setCustomParameter(varindex,Ogre::Vector4(v1,v2,v3,v4));
                }
        }
        return 0;
TRY_END
}


static int entity_get_custom_parameter (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        unsigned n = check_t<unsigned>(L,2);
        unsigned varindex = check_t<unsigned>(L,3);
        Ogre::SubEntity *se = self.getSubEntity(n);
        const Ogre::Vector4 &cp = se->getCustomParameter(varindex);
        lua_pushnumber(L,cp[0]);
        lua_pushnumber(L,cp[1]);
        lua_pushnumber(L,cp[2]);
        lua_pushnumber(L,cp[3]);
        return 4;
TRY_END
}


static int entity_get_bone_derived_position_orientation (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Vector3 pos = skel->getBone(n)->_getDerivedPosition();
        Ogre::Quaternion quat = skel->getBone(n)->_getDerivedOrientation();
        lua_pushnumber(L,pos.x);
        lua_pushnumber(L,pos.y);
        lua_pushnumber(L,pos.z);
        lua_pushnumber(L,quat.w);
        lua_pushnumber(L,quat.x);
        lua_pushnumber(L,quat.y);
        lua_pushnumber(L,quat.z);
        return 7;
TRY_END
}

static int entity_get_bone_position (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Vector3 pos = skel->getBone(n)->getPosition();
        lua_pushnumber(L,pos.x);
        lua_pushnumber(L,pos.y);
        lua_pushnumber(L,pos.z);
        return 3;
TRY_END
}

static int entity_get_bone_initial_position (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Vector3 pos = skel->getBone(n)->getInitialPosition();
        lua_pushnumber(L,pos.x);
        lua_pushnumber(L,pos.y);
        lua_pushnumber(L,pos.z);
        return 3;
TRY_END
}

static int entity_set_bone_position (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        float x = luaL_checknumber(L,3);
        float y = luaL_checknumber(L,4);
        float z = luaL_checknumber(L,5);
        skel->getBone(n)->setPosition(x,y,z);
        return 0;
TRY_END
}

static int entity_set_bone_position_offset (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Vector3 ip = skel->getBone(n)->getInitialPosition();
        float x = luaL_checknumber(L,3);
        float y = luaL_checknumber(L,4);
        float z = luaL_checknumber(L,5);
        skel->getBone(n)->setPosition(ip + Ogre::Vector3(x,y,z));
        return 0;
TRY_END
}

static int entity_get_bone_orientation (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Quaternion quat = skel->getBone(n)->getOrientation();
        lua_pushnumber(L,quat.w);
        lua_pushnumber(L,quat.x);
        lua_pushnumber(L,quat.y);
        lua_pushnumber(L,quat.z);
        return 4;
TRY_END
}

static int entity_get_bone_initial_orientation (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Quaternion quat = skel->getBone(n)->getInitialOrientation();
        lua_pushnumber(L,quat.w);
        lua_pushnumber(L,quat.x);
        lua_pushnumber(L,quat.y);
        lua_pushnumber(L,quat.z);
        return 4;
TRY_END
}

static int entity_set_bone_orientation (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        float qw = luaL_checknumber(L,3);
        float qx = luaL_checknumber(L,4);
        float qy = luaL_checknumber(L,5);
        float qz = luaL_checknumber(L,6);
        skel->getBone(n)->setOrientation(qw,qx,qy,qz);
        return 0;
TRY_END
}

static int entity_set_bone_orientation_offset (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Quaternion quat = skel->getBone(n)->getInitialOrientation();
        float qw = luaL_checknumber(L,3);
        float qx = luaL_checknumber(L,4);
        float qy = luaL_checknumber(L,5);
        float qz = luaL_checknumber(L,6);
        skel->getBone(n)->setOrientation(quat*Ogre::Quaternion(qw,qx,qy,qz));
        return 0;
TRY_END
}

static int entity_get_bone_orientation_offset (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Quaternion parent = skel->getBone(n)->getInitialOrientation();
        Ogre::Quaternion world = skel->getBone(n)->getOrientation();
        PUT_QUAT(parent.Inverse() * world);
        return 4;
TRY_END
}

static int entity_get_bone_position_offset (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Vector3 parent = skel->getBone(n)->getInitialPosition();
        Ogre::Vector3 world = skel->getBone(n)->getPosition();
        PUT_V3(world - parent);
        return 3;
TRY_END
}

static int entity_set_bone_orientation_offset_angle (lua_State *L)
{
TRY_START
        check_args(L,6);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Quaternion quat = skel->getBone(n)->getInitialOrientation();
        float qw = luaL_checknumber(L,3);
        float qx = luaL_checknumber(L,4);
        float qy = luaL_checknumber(L,5);
        float qz = luaL_checknumber(L,6);
        skel->getBone(n)->setOrientation(
                quat*Ogre::Quaternion(Ogre::Degree(qw),Ogre::Vector3(qx,qy,qz)));
        return 0;
TRY_END
}

static int entity_get_bone_position_orientation (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Vector3 pos = skel->getBone(n)->getPosition();
        Ogre::Quaternion quat = skel->getBone(n)->getOrientation();
        lua_pushnumber(L,pos.x);
        lua_pushnumber(L,pos.y);
        lua_pushnumber(L,pos.z);
        lua_pushnumber(L,quat.w);
        lua_pushnumber(L,quat.x);
        lua_pushnumber(L,quat.y);
        lua_pushnumber(L,quat.z);
        return 7;
TRY_END
}

static int entity_get_bone_initial_position_orientation (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        Ogre::Vector3 pos = skel->getBone(n)->getInitialPosition();
        Ogre::Quaternion quat = skel->getBone(n)->getInitialOrientation();
        lua_pushnumber(L,pos.x);
        lua_pushnumber(L,pos.y);
        lua_pushnumber(L,pos.z);
        lua_pushnumber(L,quat.w);
        lua_pushnumber(L,quat.x);
        lua_pushnumber(L,quat.y);
        lua_pushnumber(L,quat.z);
        return 7;
TRY_END
}

static int entity_set_bone_position_orientation (lua_State *L)
{
TRY_START
        check_args(L,9);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        float x = luaL_checknumber(L,3);
        float y = luaL_checknumber(L,4);
        float z = luaL_checknumber(L,5);
        float qw = luaL_checknumber(L,6);
        float qx = luaL_checknumber(L,7);
        float qy = luaL_checknumber(L,8);
        float qz = luaL_checknumber(L,9);
        skel->getBone(n)->setPosition(x,y,z);
        skel->getBone(n)->setOrientation(qw,qx,qy,qz);
        return 0;
TRY_END
}

static int entity_get_bone_id (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        std::string name = luaL_checkstring(L,2);
        if (!skel->hasBone(name)) my_lua_error(L, "No such bone: "+name);
        unsigned n = skel->getBone(name)->getHandle();
        lua_pushnumber(L,n);
        return 1;
TRY_END
}

static int entity_get_bone_manually_controlled (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        lua_pushboolean(L,skel->getBone(n)->isManuallyControlled());
        return 1;
TRY_END
}

static int entity_set_bone_manually_controlled (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        unsigned n = check_int(L,2,0,skel->getNumBones()-1);
        bool b = check_bool(L,3);
        skel->getBone(n)->setManuallyControlled(b);
        skel->_notifyManualBonesDirty(); // HACK!
        return 0;
TRY_END
}

static int entity_set_all_bones_manually_controlled (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SkeletonInstance *skel = self.getSkeleton();
        if (skel==NULL) my_lua_error(L, "This mesh does not have a skeleton.");
        bool b = check_bool(L,2);
        for (int i=0 ; i<skel->getNumBones() ; ++i) {
                skel->getBone(i)->setManuallyControlled(b);
        }
        skel->_notifyManualBonesDirty(); // HACK!
        return 0;
TRY_END
}


static int entity_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        Ogre::SceneManager *scnmgr = self._getManager();
        scnmgr->destroyEntity(&self);
        map_nullify_remove(grit->getUserDataTables().scnmgrs[scnmgr].entities,
                           &self);
        return 0;
TRY_END
}


TOSTRING_GETNAME_MACRO(entity,Ogre::Entity,.getName(),ENTITY_TAG)

static int entity_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::Entity,ent,1,ENTITY_TAG,0);
        if (ent==NULL) return 0;
        Ogre::SceneManager *scnmgr = ent->_getManager();
        vec_nullify_remove(
                       grit->getUserDataTables().scnmgrs[scnmgr].entities[ent],
                           &ent);
        return 0;
TRY_END
}

static int entity_index(lua_State *L)
{
TRY_START
        // this was determined (via profiling) to be hot code
        // so is appropriately "massaged"
        check_args(L,2);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        const char *key = luaL_checkstring(L,2);
        if (0==strcmp(key,"displaySkeleton")) {
                lua_pushboolean(L,self.getDisplaySkeleton());
        } else if (0==strcmp(key,"setCustomParameterAll")) {
                push_cfunction(L,entity_set_custom_parameter_all);
        } else if (0==strcmp(key,"setCustomParameter")) {
                push_cfunction(L,entity_set_custom_parameter);
        } else if (0==strcmp(key,"getCustomParameter")) {
                push_cfunction(L,entity_get_custom_parameter);
        } else if (0==strcmp(key,"setBonePosition")) {
                push_cfunction(L,entity_set_bone_position);
        } else if (0==strcmp(key,"setBonePositionOffset")) {
                push_cfunction(L,entity_set_bone_position_offset);
        } else if (0==strcmp(key,"setBoneOrientation")) {
                push_cfunction(L,entity_set_bone_orientation);
        } else if (0==strcmp(key,"setBoneOrientationOffset")) {
                push_cfunction(L,entity_set_bone_orientation_offset);
        } else if (0==strcmp(key,"setBoneOrientationOffsetAngle")) {
                push_cfunction(L,entity_set_bone_orientation_offset_angle);
        } else if (0==strcmp(key,"setBonePositionOrientation")) {
                push_cfunction(L,entity_set_bone_position_orientation);
        } else if (0==strcmp(key,"getBoneDerivedPositionOrientation")) {
                push_cfunction(L,entity_get_bone_derived_position_orientation);
        } else if (0==strcmp(key,"getBoneInitialPositionOrientation")) {
                push_cfunction(L,entity_get_bone_initial_position_orientation);
        } else if (0==strcmp(key,"getBonePositionOrientation")) {
                push_cfunction(L,entity_get_bone_position_orientation);
        } else if (0==strcmp(key,"getBoneInitialPosition")) {
                push_cfunction(L,entity_get_bone_initial_position);
        } else if (0==strcmp(key,"getBoneInitialOrientation")) {
                push_cfunction(L,entity_get_bone_initial_orientation);
        } else if (0==strcmp(key,"getBonePosition")) {
                push_cfunction(L,entity_get_bone_position);
        } else if (0==strcmp(key,"getBoneOrientation")) {
                push_cfunction(L,entity_get_bone_orientation);
        } else if (0==strcmp(key,"getBonePositionOffset")) {
                push_cfunction(L,entity_get_bone_position_offset);
        } else if (0==strcmp(key,"getBoneOrientationOffset")) {
                push_cfunction(L,entity_get_bone_orientation_offset);
        } else if (0==strcmp(key,"setAllBonesManuallyControlled")) {
                push_cfunction(L,entity_set_all_bones_manually_controlled);
        } else if (0==strcmp(key,"setBoneManuallyControlled")) {
                push_cfunction(L,entity_set_bone_manually_controlled);
        } else if (0==strcmp(key,"getBoneManuallyControlled")) {
                push_cfunction(L,entity_get_bone_manually_controlled);
        } else if (0==strcmp(key,"getBoneId")) {
                push_cfunction(L,entity_get_bone_id);
        } else if (0==strcmp(key,"hasSkeleton")) {
                lua_pushboolean(L,self.getSkeleton()!=NULL);
        } else if (0==strcmp(key,"numBones")) {
                lua_pushnumber(L,self.getSkeleton()->getNumBones());
        } else if (0==strcmp(key,"getMaterial")) {
                push_cfunction(L,entity_get_material);
        } else if (0==strcmp(key,"getMaterialName")) {
                push_cfunction(L,entity_get_material_name);
        } else if (0==strcmp(key,"setMaterial")) {
                push_cfunction(L,entity_set_material);
        } else if (0==strcmp(key,"numSubEntities")) {
                lua_pushnumber(L,self.getNumSubEntities());
        } else if (0==strcmp(key,"mesh")) {
                push_mesh(L,self.getMesh());
        } else if (0==strcmp(key,"getPolygonModeOverrideable")) {
                push_cfunction(L,entity_get_polygon_mode_overrideable);
        } else if (0==strcmp(key,"setPolygonModeOverrideable")) {
                push_cfunction(L,entity_set_polygon_mode_overrideable);
        } else if (0==strcmp(key,"destroy")) {
                push_cfunction(L,entity_destroy);
        } else if (!mobj_index(L,self,key)) {
                my_lua_error(L,"Not a readable Entity member: " + std::string(key));
        }
        return 1;
TRY_END
}

static int entity_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::Entity,self,1,ENTITY_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key == "displaySkeleton") {
                bool b = check_bool(L,3);
                self.setDisplaySkeleton(b);
        } else if (!mobj_newindex(L,self,key)) {
                my_lua_error(L,"Not a writeable Entity member: " + key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::Entity,entity,ENTITY_TAG)

MT_MACRO_NEWINDEX(entity);

//}}}


// INSTANCED GEOM ========================================================== {{{


void push_instgeo (lua_State *L, Ogre::SceneManager *scnmgr,
                         Ogre::InstancedGeometry *g)
{
        if (!g) {
                lua_pushnil(L);
                return;
        }
        void **ud = static_cast<void**>(lua_newuserdata(L, 2*sizeof(*ud)));
        ud[0] = static_cast<void*> (g);
        ud[1] = static_cast<void*> (scnmgr);
        luaL_getmetatable(L, INSTGEO_TAG);
        lua_setmetatable(L, -2);
        APP_ASSERT(scnmgr!=NULL);
        APP_ASSERT(g!=NULL);
        APP_ASSERT(ud!=NULL);
        grit->getUserDataTables().scnmgrs[scnmgr].instgeoms[g].push_back(ud);
}


static int instgeo_reset (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::InstancedGeometry,self,1,INSTGEO_TAG);
        self.reset();
        return 0;
TRY_END
}

static int instgeo_build (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::InstancedGeometry,self,1,INSTGEO_TAG);
        self.build();
        return 0;
TRY_END
}

static int instgeo_add_entity (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(Ogre::InstancedGeometry,self,1,INSTGEO_TAG);
        GET_UD_MACRO(Ogre::Entity,ent,2,ENTITY_TAG);
        GET_UD_MACRO(Ogre::Vector3,pos,3,VECTOR3_TAG);
        GET_UD_MACRO(Ogre::Quaternion,q,4,QUAT_TAG);
        GET_UD_MACRO(Ogre::Vector3,scale,5,VECTOR3_TAG);

        self.addEntity(&ent,pos,q,scale);
        return 0;
TRY_END
}

static int instgeo_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::InstancedGeometry,self,1,INSTGEO_TAG);
        GET_UD_MACRO_OFFSET(Ogre::SceneManager,scnmgr,1,INSTGEO_TAG,1);
        scnmgr->destroyInstancedGeometry(&self);
        map_nullify_remove(grit->getUserDataTables().scnmgrs[scnmgr].instgeoms,
                           &self);

        return 0;
TRY_END
}



TOSTRING_GETNAME_MACRO(instgeo,Ogre::InstancedGeometry,.getName(),INSTGEO_TAG)

static int instgeo_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::InstancedGeometry,self,1,INSTGEO_TAG,0);
        if (self==NULL) return 0;
        GET_UD_MACRO_OFFSET(Ogre::SceneManager,scnmgr,1,INSTGEO_TAG,1);
        vec_nullify_remove(
                     grit->getUserDataTables().scnmgrs[scnmgr].instgeoms[self],
                           &self);
        return 0;
TRY_END
}

static int instgeo_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::InstancedGeometry,self,1,INSTGEO_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="reset") {
                push_cfunction(L,instgeo_reset);
        } else if (key=="build") {
                push_cfunction(L,instgeo_build);
        } else if (key=="renderingDistance") {
                lua_pushnumber(L,self.getRenderingDistance());
        } else if (key=="squaredRenderingDistance") {
                lua_pushnumber(L,self.getSquaredRenderingDistance());
        } else if (key=="regionDimensions") {
                push(L,new Ogre::Vector3(self.getBatchInstanceDimensions()),
                       VECTOR3_TAG);
        } else if (key=="origin") {
                push(L,new Ogre::Vector3(self.getOrigin()),VECTOR3_TAG);

        } else if (key=="addEntity") {
                push_cfunction(L,instgeo_add_entity);
        } else if (key=="name") {
                lua_pushstring(L,self.getName().c_str());
        } else if (key=="destroy") {
                push_cfunction(L,instgeo_destroy);
        } else {
                std::string s("Not a valid Instanced Geometry member: ");
                my_lua_error(L,s+key);
        }
        return 1;
TRY_END
}

static int instgeo_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::InstancedGeometry,self,1,INSTGEO_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="renderingDistance") {
                lua_Number n = luaL_checknumber(L,3);
                self.setRenderingDistance(n);
        } else if (key=="regionDimensions") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self.setBatchInstanceDimensions(v);
        } else if (key=="origin") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self.setOrigin(v);
        } else {
                std::string s("Not a valid Instanced Geometry member: ");
                my_lua_error(L,s+key);
        }
        return 1;
TRY_END
}

EQ_PTR_MACRO(Ogre::InstancedGeometry,instgeo,INSTGEO_TAG)


MT_MACRO_NEWINDEX(instgeo);

//}}}


// STATIC GEOM ============================================================= {{{

void push_statgeo (lua_State *L, Ogre::SceneManager *scnmgr,
                         Ogre::StaticGeometry *g)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, 2*sizeof(*ud)));
        ud[0] = static_cast<void*> (g);
        ud[1] = static_cast<void*> (scnmgr);
        luaL_getmetatable(L, STATGEO_TAG);
        lua_setmetatable(L, -2);
        APP_ASSERT(scnmgr!=NULL);
        APP_ASSERT(g!=NULL);
        APP_ASSERT(ud!=NULL);
        grit->getUserDataTables().scnmgrs[scnmgr].statgeoms[g].push_back(ud);
}


static int statgeo_reset (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::StaticGeometry,self,1,STATGEO_TAG);
        self.reset();
        return 0;
TRY_END
}

static int statgeo_build (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::StaticGeometry,self,1,STATGEO_TAG);
        self.build();
        return 0;
TRY_END
}

static int statgeo_add_entity (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(Ogre::StaticGeometry,self,1,STATGEO_TAG);
        GET_UD_MACRO(Ogre::Entity,ent,2,ENTITY_TAG);
        GET_UD_MACRO(Ogre::Vector3,pos,3,VECTOR3_TAG);
        GET_UD_MACRO(Ogre::Quaternion,q,4,QUAT_TAG);
        GET_UD_MACRO(Ogre::Vector3,scale,5,VECTOR3_TAG);

        self.addEntity(&ent,pos,q,scale);
        return 0;
TRY_END
}

static int statgeo_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::StaticGeometry,self,1,STATGEO_TAG);
        GET_UD_MACRO_OFFSET(Ogre::SceneManager,scnmgr,1,STATGEO_TAG,1);
        scnmgr->destroyStaticGeometry(&self);
        map_nullify_remove(grit->getUserDataTables().scnmgrs[scnmgr].statgeoms,
                           &self);

        return 0;
TRY_END
}



TOSTRING_GETNAME_MACRO(statgeo,Ogre::StaticGeometry,.getName(),STATGEO_TAG)

static int statgeo_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::StaticGeometry,self,1,STATGEO_TAG,0);
        if (self==NULL) return 0;
        GET_UD_MACRO_OFFSET(Ogre::SceneManager,scnmgr,1,STATGEO_TAG,1);
        vec_nullify_remove(
                     grit->getUserDataTables().scnmgrs[scnmgr].statgeoms[self],
                           &self);
        return 0;
TRY_END
}

static int statgeo_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::StaticGeometry,self,1,STATGEO_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="reset") {
                push_cfunction(L,statgeo_reset);
        } else if (key=="build") {
                push_cfunction(L,statgeo_build);
        } else if (key=="renderingDistance") {
                lua_pushnumber(L,self.getRenderingDistance());
        } else if (key=="squaredRenderingDistance") {
                lua_pushnumber(L,self.getSquaredRenderingDistance());
        } else if (key=="regionDimensions") {
                push(L,new Ogre::Vector3(self.getRegionDimensions()),
                       VECTOR3_TAG);
        } else if (key=="origin") {
                push(L,new Ogre::Vector3(self.getOrigin()),VECTOR3_TAG);

        } else if (key=="addEntity") {
                push_cfunction(L,statgeo_add_entity);
        } else if (key=="name") {
                lua_pushstring(L,self.getName().c_str());
        } else if (key=="destroy") {
                push_cfunction(L,statgeo_destroy);
        } else {
                std::string s("Not a valid Static Geometry member: ");
                my_lua_error(L,s+key);
        }
        return 1;
TRY_END
}

static int statgeo_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::StaticGeometry,self,1,STATGEO_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="renderingDistance") {
                lua_Number n = luaL_checknumber(L,3);
                self.setRenderingDistance(n);
        } else if (key=="regionDimensions") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self.setRegionDimensions(v);
        } else if (key=="origin") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self.setOrigin(v);
        } else {
                std::string s("Not a valid Static Geometry member: ");
                my_lua_error(L,s+key);
        }
        return 1;
TRY_END
}

EQ_PTR_MACRO(Ogre::StaticGeometry,statgeo,STATGEO_TAG)

MT_MACRO_NEWINDEX(statgeo);

//}}}


// CAMERA ================================================================== {{{



void push_cam (lua_State *L, Ogre::Camera *cam)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (cam);
        Ogre::SceneManager *scnmgr = cam->getSceneManager();
        luaL_getmetatable(L, CAM_TAG);
        lua_setmetatable(L, -2);
        grit->getUserDataTables().scnmgrs[scnmgr].cameras[cam].push_back(ud);
}

static int cam_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::Camera,self,1,CAM_TAG);
        Ogre::SceneManager *scnmgr = self.getSceneManager();
        scnmgr->destroyCamera(&self);
        map_nullify_remove(grit->getUserDataTables().scnmgrs[scnmgr].cameras,
                           &self);
        return 0;
TRY_END
}



TOSTRING_MACRO(cam,Ogre::Camera,CAM_TAG)
//TOSTRING_GETNAME_MACRO(cam,Ogre::Camera,.getName(),CAM_TAG)

static int cam_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::Camera,self,1,CAM_TAG,0);
        if (self==NULL) return 0;
        Ogre::SceneManager *scnmgr = self->getSceneManager();
        vec_nullify_remove(
                       grit->getUserDataTables().scnmgrs[scnmgr].cameras[self],
                           &self);
        return 0;
TRY_END
}

static int cam_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::Camera,self,1,CAM_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="position") {
                push(L,new Ogre::Vector3(self.getPosition()),VECTOR3_TAG);
        } else if (key=="orientation") {
                push(L,new Ogre::Quaternion(self.getOrientation()),QUAT_TAG);
        } else if (key=="derivedPosition") {
                push(L,new Ogre::Vector3(self.getDerivedPosition()),
                       VECTOR3_TAG);
        } else if (key=="derivedOrientation") {
                push(L,new Ogre::Quaternion(self.getDerivedOrientation()),
                       QUAT_TAG);
        } else if (key=="polygonMode") {
                std::string s = polygon_mode_to_string(L,self.getPolygonMode());
                lua_pushstring(L,s.c_str());
        } else if (key=="focalLength") {
                lua_pushnumber(L,self.getFocalLength());
        } else if (key=="frustumOffsetX") {
                lua_pushnumber(L,self.getFrustumOffset().x);
        } else if (key=="frustumOffsetY") {
                lua_pushnumber(L,self.getFrustumOffset().y);
        } else if (key=="autoAspectRatio") {
                lua_pushboolean(L,self.getAutoAspectRatio());
        } else if (key=="FOV") {
                lua_pushnumber(L,self.getFOVy().valueDegrees());
        } else if (key=="farClipDistance") {
                lua_pushnumber(L,self.getFarClipDistance());
        } else if (key=="nearClipDistance") {
                lua_pushnumber(L,self.getNearClipDistance());
        } else if (key=="useRenderingDistance") {
                lua_pushboolean(L,self.getUseRenderingDistance());
        } else if (key=="destroy") {
                push_cfunction(L,cam_destroy);
        } else if (!mobj_index(L,self,key)) {
                my_lua_error(L,"Not a valid Camera member: " + key);
        }
        return 1;
TRY_END
}

static int cam_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::Camera,self,1,CAM_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="position") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self.setPosition(v);
        } else if (key=="orientation") {
                GET_UD_MACRO(Ogre::Quaternion,q,3,QUAT_TAG);
                self.setOrientation(q);
        } else if (key=="polygonMode") {
                std::string s = luaL_checkstring(L,3);
                self.setPolygonMode(polygon_mode_from_string(L,s));
        } else if (key=="focalLength") {
                lua_Number n = luaL_checknumber(L,3);
                self.setFocalLength(n);
        } else if (key=="frustumOffsetX") {
                Ogre::Vector2 v = self.getFrustumOffset();
                v.x = luaL_checknumber(L,3);
                self.setFrustumOffset(v);
        } else if (key=="frustumOffsetY") {
                Ogre::Vector2 v = self.getFrustumOffset();
                v.y = luaL_checknumber(L,3);
                self.setFrustumOffset(v);
        } else if (key=="autoAspectRatio") {
                bool v = check_bool(L,3);
                self.setAutoAspectRatio(v);
        } else if (key=="FOV") {
                lua_Number n = luaL_checknumber(L,3);
                if (n<=0) {
                        my_lua_error(L,"FOV must be greater than 0.");
                } else if (n>=180) {
                        my_lua_error(L,"FOV must be less than 180.");
                }
                self.setFOVy(Ogre::Degree(n));
        } else if (key=="farClipDistance") {
                lua_Number n = luaL_checknumber(L,3);
                self.setFarClipDistance(n);
        } else if (key=="nearClipDistance") {
                lua_Number n = luaL_checknumber(L,3);
                self.setNearClipDistance(n);
        } else if (key=="useRenderingDistance") {
                bool b = check_bool(L,3);
                self.setUseRenderingDistance(b);
        } else {
                my_lua_error(L,"Not a valid Camera member: " + key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::Camera,cam,CAM_TAG)

MT_MACRO_NEWINDEX(cam);

//}}}



// vim: shiftwidth=8:tabstop=8:expandtab
