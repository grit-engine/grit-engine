#include "lua_wrappers_common.h"


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
}

Ogre::MovableObject *check_mobj(lua_State *L,int index);

bool push_mobj (lua_State *L, Ogre::MovableObject *mobj);


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






// vim: shiftwidth=8:tabstop=8:expandtab
