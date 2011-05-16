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

#include "lua_wrappers_common.h"

#include <OgreLight.h>

extern "C" {
#include "lua.h"
}

namespace Ogre {
        class MovableObject;
        class InstancedGeometry;
        class StaticGeometry;
        class Camera;
        class Light;
        class Entity;
        class ManualObject;
        class ParticleSystem;
}

class RangedClutter;
class MovableClutter;

Ogre::MovableObject *check_mobj(lua_State *L,int index);

bool push_mobj (lua_State *L, Ogre::MovableObject *mobj);

Ogre::Light::LightTypes light_type_from_string (lua_State *L,
                                                const std::string &t);

#define MANOBJ_TAG "Grit/ManualObject"
MT_MACRO_DECLARE(manobj);
void push_manobj (lua_State *, Ogre::ManualObject *);

#define ENTITY_TAG "Grit/Entity"
MT_MACRO_DECLARE(entity);
void push_entity (lua_State *, Ogre::Entity *);

#define STATGEO_TAG "Grit/StaticGeometry"
MT_MACRO_DECLARE(statgeo);
void push_statgeo (lua_State *, Ogre::SceneManager *, Ogre::StaticGeometry *);


#define INSTGEO_TAG "Grit/InstancedGeometry"
MT_MACRO_DECLARE(instgeo);
void push_instgeo (lua_State *, Ogre::SceneManager *, Ogre::InstancedGeometry*);

#define CAM_TAG "Grit/Camera"
MT_MACRO_DECLARE(cam);
void push_cam (lua_State *, Ogre::Camera *);

#define LIGHT_TAG "Grit/Light"
MT_MACRO_DECLARE(light);
void push_light (lua_State *, Ogre::Light *);

#define CLUTTER_TAG "Grit/MovableClutter"
MT_MACRO_DECLARE(clutter);
void push_clutter (lua_State *, MovableClutter *);

#define RCLUTTER_TAG "Grit/RangedClutter"
MT_MACRO_DECLARE(rclutter);
void push_rclutter (lua_State *, RangedClutter *);





// vim: shiftwidth=8:tabstop=8:expandtab
