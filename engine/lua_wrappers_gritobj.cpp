/* Copyright (c) The Grit Game Engine authors 2016
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

#include "grit_class.h"
#include "grit_object.h"
#include "lua_wrappers_core.h"
#include "lua_wrappers_gritobj.h"
#include "lua_wrappers_primitives.h"
#include "main.h"
#include "path_util.h"


// GRIT CLASS ============================================================= {{{

void push_gritcls (lua_State *L, GritClass *self)
{
    if (self == NULL) {
        lua_pushnil(L);
    } else {
        self->acquire();
        push(L, self, GRITCLS_TAG);
    }
}

static int gritcls_gc (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO_OFFSET(GritClass, self, 1, GRITCLS_TAG, 0);
    self->release(L);
    return 0;
TRY_END
}

TOSTRING_GETNAME_MACRO(gritcls, GritClass, .name, GRITCLS_TAG)



static int gritcls_index (lua_State *L)
{
TRY_START
    check_args(L, 2);
    GET_UD_MACRO(GritClass, self, 1, GRITCLS_TAG);
    std::string key = check_string(L, 2);
    if (key == "name") {
        lua_pushstring(L, self.name.c_str());
    } else if (key == "parent") {
        self.pushParent(L);
    } else if (key == "dump") {
        self.dump(L);
    } else {
        self.get(L, key);
    }
    return 1;
TRY_END
}

static int gritcls_newindex (lua_State *L)
{
TRY_START
    check_args(L, 3);
    GET_UD_MACRO(GritClass, self, 1, GRITCLS_TAG);
    std::string key = check_string(L, 2);

    if (key == "name") {
        my_lua_error(L, "Not a writeable GritClass member: " + key);
    } else if (key == "parent") {
        self.setParent(L);
    } else {
        self.set(L, key);
    }

    return 0;
TRY_END
}

EQ_PTR_MACRO(GritClass, gritcls, GRITCLS_TAG)

MT_MACRO_NEWINDEX(gritcls);

//}}}



// GRIT OBJECT ============================================================= {{{

void push_gritobj (lua_State *L, const GritObjectPtr &self)
{
    if (self.isNull()) {
        lua_pushnil(L);
    } else {
        push(L, new GritObjectPtr(self), GRITOBJ_TAG);
    }
}

static int gritobj_gc (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO_OFFSET(GritObjectPtr, self, 1, GRITOBJ_TAG, 0);
    delete self;
    return 0;
TRY_END
}

static int gritobj_destroy (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(GritObjectPtr, self, 1, GRITOBJ_TAG);
    object_del(L, self);
    return 0;
TRY_END
}

static int gritobj_activate (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(GritObjectPtr, self, 1, GRITOBJ_TAG);
    self->activate(L, self);
    return 0;
TRY_END
}

static int gritobj_add_disk_resource (lua_State *L)
{
TRY_START
    check_args(L, 2);
    GET_UD_MACRO(GritObjectPtr, self, 1, GRITOBJ_TAG);
    std::string name = check_path(L, 2);
    self->addDiskResource(name);
    return 0;
TRY_END
}

static int gritobj_reload_disk_resource (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(GritObjectPtr, self, 1, GRITOBJ_TAG);
    self->reloadDiskResources();
    return 0;
TRY_END
}

/*
static int gritobj_get_advance_prepare_hints (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(GritObjectPtr, self, 1, GRITOBJ_TAG);
    const GritObject::StringPairs &hints = self->getAdvanceHints();
    typedef GritObject::StringPairs::const_iterator I;
    int counter = 0;
    for (I i=hints.begin(), i_=hints.end() ; i != i_ ; ++i) {
        const std::string &type = i->first;
        const std::string &name = i->second;
        lua_pushstring(L, type.c_str());
        lua_pushstring(L, name.c_str());
        counter += 2;
    }
    return counter;
TRY_END
}
*/

static int gritobj_deactivate (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(GritObjectPtr, self, 1, GRITOBJ_TAG);
    self->deactivate(L, self);
    return 0;
TRY_END
}



TOSTRING_GETNAME_MACRO(gritobj, GritObjectPtr, ->name, GRITOBJ_TAG)



static int gritobj_index (lua_State *L)
{
TRY_START
    check_args(L, 2);
    GET_UD_MACRO(GritObjectPtr, self, 1, GRITOBJ_TAG);
    std::string key = check_string(L, 2);
    if (key == "destroy") {
        push_cfunction(L, gritobj_destroy);
    } else if (key == "activated") {
        lua_pushboolean(L, self->isActivated());
    } else if (key == "near") {
        push_gritobj(L, self->getNearObj());
    } else if (key == "far") {
        push_gritobj(L, self->getFarObj());
    } else if (key == "fade") {
        lua_pushnumber(L, self->getFade());
    } else if (key == "pos") {
        push_v3(L, self->getPos());
    } else if (key == "renderingDistance") {
        lua_pushnumber(L, self->getR());
    } else if (key == "deactivate") {
        push_cfunction(L, gritobj_deactivate);
    } else if (key == "activate") {
        push_cfunction(L, gritobj_activate);
    } else if (key == "instance") {
        self->pushLuaTable(L);
    } else if (key == "addDiskResource") {
        push_cfunction(L, gritobj_add_disk_resource);
    } else if (key == "reloadDiskResources") {
        push_cfunction(L, gritobj_reload_disk_resource);
/*
    } else if (key == "getAdvancePrepareHints") {
        push_cfunction(L, gritobj_get_advance_prepare_hints);
*/
    } else if (key == "destroyed") {
        lua_pushboolean(L, self->getClass() == NULL);
    } else if (key == "class") {
        GritClass *c = self->getClass();
        if (c == NULL) my_lua_error(L, "GritObject destroyed");
        push_gritcls(L, c);
    } else if (key == "className") {
        GritClass *c = self->getClass();
        if (c == NULL) my_lua_error(L, "GritObject destroyed");
        lua_pushstring(L, c->name.c_str());
    } else if (key == "name") {
        lua_pushstring(L, self->name.c_str());
    } else if (key == "needsFrameCallbacks") {
        lua_pushboolean(L, self->getNeedsFrameCallbacks());
    } else if (key == "needsStepCallbacks") {
        lua_pushboolean(L, self->getNeedsStepCallbacks());
    } else if (key == "dump") {
        self->userValues.dump(L);
    } else {
        self->getField(L, key);
    }
    return 1;
TRY_END
}

static int gritobj_newindex (lua_State *L)
{
TRY_START
    check_args(L, 3);
    GET_UD_MACRO(GritObjectPtr, self, 1, GRITOBJ_TAG);
    std::string key = check_string(L, 2);

    if (key == "destroy") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "near") {
        if (lua_isnil(L, 3)) {
            self->setNearObj(self, GritObjectPtr());
        } else {
            GET_UD_MACRO(GritObjectPtr, v, 3, GRITOBJ_TAG);
            self->setNearObj(self, v);
        }
    } else if (key == "far") {
        if (lua_isnil(L, 3)) {
            self->setNearObj(self, GritObjectPtr());
        } else {
            GET_UD_MACRO(GritObjectPtr, v, 3, GRITOBJ_TAG);
            self->setFarObj(self, v);
        }
    } else if (key == "fade") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "pos") {
        self->updateSphere(check_v3(L, 3));
    } else if (key == "renderingDistance") {
        self->updateSphere(check_float(L, 3));
    } else if (key == "getSphere") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "activated") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "deactivate") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "activate") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "instance") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "hintAdvancePrepare") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "getAdvancePrepareHints") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "destroyed") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "class") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "className") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "name") {
        my_lua_error(L, "Not a writeable GritObject member: " + key);
    } else if (key == "needsFrameCallbacks") {
        self->setNeedsFrameCallbacks(self, check_bool(L, 3));
    } else if (key == "needsStepCallbacks") {
        self->setNeedsStepCallbacks(self, check_bool(L, 3));
    } else {
        GritClass *c = self->getClass();
        if (c == NULL) my_lua_error(L, "GritObject destroyed");
        const char *err = self->userValues.luaSet(L);
        if (err) my_lua_error(L, err);
    }

    return 0;
TRY_END
}

EQ_MACRO(GritObjectPtr, gritobj, GRITOBJ_TAG)

MT_MACRO_NEWINDEX(gritobj);

//}}}



// STREAMER ================================================================ {{{

static int global_streamer_centre (lua_State *L)
{
TRY_START
    check_args(L, 1);
    Vector3 pos = check_v3(L, 1);
    streamer_centre(L, pos, false);
    return 0;
TRY_END
}

static int global_streamer_centre_full (lua_State *L)
{
TRY_START
    check_args(L, 1);
    Vector3 pos = check_v3(L, 1);
    streamer_centre(L, pos, true);
    return 0;
TRY_END
}

static int global_class_add (lua_State *L)
{
TRY_START
    check_args(L, 3);
    std::string name = check_path(L, 1);
    if (!lua_istable(L, 2))
        my_lua_error(L, "Second parameter should be a table");
    if (!lua_istable(L, 3))
        my_lua_error(L, "Third parameter should be a table");
    class_add(L, name);
    return 1;
TRY_END
}

static int global_class_get (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    push_gritcls(L, class_get(name));
    return 1;
TRY_END
}

static int global_class_has (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    lua_pushboolean(L, class_has(name));
    return 1;
TRY_END
}

static int global_class_del (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_path(L, 1);
    class_del(L, class_get(name));
    return 0;
TRY_END
}

static int global_class_all_del (lua_State *L)
{
TRY_START
    check_args(L, 0);
    class_all_del(L);
    return 0;
TRY_END
}

static int global_class_count (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushnumber(L, class_count());
    return 1;
TRY_END
}

static int global_class_all (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_newtable(L);
    unsigned int c = 0;
    GritClassMap::iterator i, i_;
    for (class_all(i, i_) ; i != i_ ; ++i) {
        push_gritcls(L, i->second);
        lua_rawseti(L, -2, c + 1);
        c++;         
    }       
    return 1;
TRY_END
}


static int global_object_add (lua_State *L)
{
TRY_START
    if (lua_gettop(L) == 2) lua_newtable(L);
    check_args(L, 3);
    std::string className = check_path(L, 1);
    Vector3 spawnPos = check_v3(L, 2);
    int table_index = lua_gettop(L);
    if (!lua_istable(L, table_index)) my_lua_error(L, "Last parameter should be a table");
    lua_getfield(L, table_index, "name");
    std::string name;
    if (lua_isnil(L, -1)) {
        name = "";
    } else {
        if (lua_type(L, -1) != LUA_TSTRING) my_lua_error(L, "Name wasn't a string!");
        name = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    GritObjectPtr o = object_add(L, name, class_get(className));
    o->userValues.set("spawnPos", spawnPos);
    lua_getfield(L, table_index, "renderingDistance");
    // Use renderingDistance from table if provided
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        // Otherwise, use class.
        o->getClass()->get(L, "renderingDistance");
        if (lua_isnil(L, -1)) {
            object_del(L, o);
            my_lua_error(L, "no renderingDistance in class \""
                            + className + "\"");
        }
        if (lua_type(L, -1) != LUA_TNUMBER) {
            object_del(L, o);
            my_lua_error(L, "renderingDistance not a number in class \""
                            + className + "\"");
        }
    } else {
        if (lua_type(L, -1) != LUA_TNUMBER) {
            object_del(L, o);
            my_lua_error(L, "renderingDistance not a number in object \""
                            + name + "\"");
        }
    }
    float r = lua_tonumber(L, -1);
    o->updateSphere(spawnPos, r);
    lua_pop(L, 1);

    // TODO move near and far into the loop below ('name' must remain above)
    lua_getfield(L, table_index, "near");
    if (!lua_isnil(L, -1)) {
        if (is_userdata(L, -1, GRITOBJ_TAG)) {
            GET_UD_MACRO(GritObjectPtr, the_near, -1, GRITOBJ_TAG);
            o->setNearObj(o, the_near);
        } else {
            my_lua_error(L, "Field 'near' must be a grit object.");
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, table_index, "far");
    if (!lua_isnil(L, -1)) {
        if (is_userdata(L, -1, GRITOBJ_TAG)) {
            GET_UD_MACRO(GritObjectPtr, the_far, -1, GRITOBJ_TAG);
            o->setFarObj(o, the_far);
        } else {
            my_lua_error(L, "Field 'far' must be a grit object.");
        }
    }
    lua_pop(L, 1);

    // scan through table adding lua data to o
    for (lua_pushnil(L) ; lua_next(L, table_index) != 0 ; lua_pop(L, 1)) {
        if (lua_type(L, -2) != LUA_TSTRING) {
            object_del(L, o);
            my_lua_error(L, "user value key was not a string");
        }
        std::string key = check_string(L, -2);
        // the name is held in the object anyway
        if (key == "name") continue;
        if (key == "near") continue;
        if (key == "far") continue;
        if (key == "renderingDistance") continue;
        const char *err = o->userValues.luaSet(L);
        if (err) {
            object_del(L, o);
            my_lua_error(L, err);
        }
    }
    o->init(L, o);
    push_gritobj(L, o);
    return 1;
TRY_END
}

static int global_object_del (lua_State *L)
{
TRY_START
    check_args(L, 1);
    GET_UD_MACRO(GritObjectPtr, self, 1, GRITOBJ_TAG);
    object_del(L, self);
    return 0;
TRY_END
}



static int global_object_get (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_string(L, 1);
    GritObjectPtr o = object_get(name);
    if (o.isNull()) {
        lua_pushnil(L);
    } else {
        push_gritobj(L, o);
    }
    return 1;
TRY_END
}

static int global_object_has (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string name = check_string(L, 1);
    lua_pushboolean(L, object_has(name));
    return 1;
TRY_END
}



static int global_object_all (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_newtable(L);
    unsigned int c = 0;
    GObjMap::iterator i, i_;
    for (object_all(i, i_) ; i != i_ ; ++i) {
        const GritObjectPtr &o = i->second;
        push_gritobj(L, o);
        lua_rawseti(L, -2, c + 1);
        c++;         
    }       
    return 1;
TRY_END
}

static int global_object_all_del (lua_State *L)
{
TRY_START
    check_args(L, 0);
    object_all_del(L);
    return 0;
TRY_END
}

static int global_object_count (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushnumber(L, object_count());
    return 1;
TRY_END
}


static int global_object_all_deactivate (lua_State *L)
{
TRY_START
    check_args(L, 0);
    GObjMap::iterator i, i_;
    for (object_all(i, i_) ; i != i_ ; ++i) {
        const GritObjectPtr &o = i->second;
        o->deactivate(L, o);
    }       
    return 0;
TRY_END
}

static int global_object_all_activated (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_newtable(L);
    unsigned int c = 0;
    GObjPtrs::iterator i, i_;
    for (streamer_object_activated(i, i_) ; i != i_ ; ++i) {
        const GritObjectPtr &o = *i;
        push_gritobj(L, o);
        lua_rawseti(L, -2, c + 1);
        c++;         
    }       
    return 1;
TRY_END
}

static int global_object_count_activated (lua_State *L)
{
TRY_START
    check_args(L, 0);
    lua_pushnumber(L, streamer_object_activated_count());
    return 1;
TRY_END
}

static int global_object_all_of_class (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string cls = check_path(L, 1);
    lua_newtable(L);
    unsigned int c = 0;
    GObjMap::iterator i, i_;
    for (object_all(i, i_) ; i != i_ ; ++i) {
        const GritObjectPtr &o = i->second;
        GritClass *oc = o->getClass();
        if (oc == NULL) continue; // object destroyed
        if (oc->name != cls) continue; // wrong class
        push_gritobj(L, o);
        lua_rawseti(L, -2, c + 1);
        c++;         
    }       
    return 1;
TRY_END
}

static int global_object_count_of_class (lua_State *L)
{
TRY_START
    check_args(L, 1);
    std::string cls = check_path(L, 1);
    unsigned counter = 0;
    GObjMap::iterator i, i_;
    for (object_all(i, i_) ; i != i_ ; ++i) {
        const GritObjectPtr &o = i->second;
        GritClass *oc = o->getClass();
        if (oc == NULL) continue; // object destroyed
        if (oc->name != cls) continue; // wrong class
        counter++;
    }       
    return 1;
TRY_END
}

static int global_object_do_frame_callbacks (lua_State *L)
{
TRY_START
    check_args(L, 1);
    float elapsed = check_float(L, 1);
    object_do_frame_callbacks(L, elapsed);
    return 0;
TRY_END
}

static int global_object_do_step_callbacks (lua_State *L)
{
TRY_START
    check_args(L, 1);
    float elapsed = check_float(L, 1);
    object_do_step_callbacks(L, elapsed);
    return 0;
TRY_END
}



static const luaL_reg global[] = {
    {"streamer_centre", global_streamer_centre},
    {"streamer_centre_full", global_streamer_centre_full},
    {"class_add", global_class_add},
    {"class_del", global_class_del},
    {"class_all_del", global_class_all_del},
    {"class_get", global_class_get},
    {"class_has", global_class_has},
    {"class_all", global_class_all},
    {"class_count", global_class_count},
    {"object_add", global_object_add},
    {"object_del", global_object_del},
    {"object_all_del", global_object_all_del},
    {"object_get", global_object_get},
    {"object_has", global_object_has},
    {"object_all", global_object_all},
    {"object_count", global_object_count},
    {"object_all_activated", global_object_all_activated},
    {"object_count_activated", global_object_count_activated},
    {"object_all_of_class", global_object_all_of_class},
    {"object_count_of_class", global_object_count_of_class},
    {"object_all_deactivate", global_object_all_deactivate},
    {"object_do_frame_callbacks", global_object_do_frame_callbacks},
    {"object_do_step_callbacks", global_object_do_step_callbacks},
    {NULL, NULL}
};

void gritobj_lua_init (lua_State *L)
{
    ADD_MT_MACRO(gritcls, GRITCLS_TAG);
    ADD_MT_MACRO(gritobj, GRITOBJ_TAG);
    register_lua_globals(L, global);
}
