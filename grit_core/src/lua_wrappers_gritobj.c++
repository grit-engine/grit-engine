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

#include "GritObject.h"
#include "main.h"
#include "gfx.h"

#include "lua_wrappers_gritobj.h"
#include "lua_wrappers_physics.h"
#include "lua_wrappers_scnmgr.h"
#include "lua_wrappers_primitives.h"
#include "lua_wrappers_core.h"
#include "path_util.h"

// GRIT CLASS ============================================================= {{{

void push_gritcls (lua_State *L, GritClass *self)
{
        if (self == NULL) {
                lua_pushnil(L);
        } else {
                self->acquire();
                push(L,self,GRITCLS_TAG);
        }
}

static int gritcls_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(GritClass,self,1,GRITCLS_TAG,0);
        self->release(L);
        return 0;
TRY_END
}

TOSTRING_GETNAME_MACRO(gritcls,GritClass,.name,GRITCLS_TAG)



static int gritcls_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritClass,self,1,GRITCLS_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="name") {
                lua_pushstring(L,self.name.c_str());
        } else if (key=="parent") {
                self.pushParent(L);
        } else if (key=="dump") {
                self.dump(L);
        } else {
                self.get(L,key);
        }
        return 1;
TRY_END
}

static int gritcls_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GritClass,self,1,GRITCLS_TAG);
        std::string key = luaL_checkstring(L,2);

        if (key=="name") {
                my_lua_error(L,"Not a writeable GritClass member: "+key);
        } else if (key=="parent") {
                self.setParent(L);
        } else {
                self.set(L,key);
        }

        return 0;
TRY_END
}

EQ_PTR_MACRO(GritClass,gritcls,GRITCLS_TAG)

MT_MACRO_NEWINDEX(gritcls);

//}}}



// GRIT OBJECT ============================================================= {{{

void push_gritobj (lua_State *L, const GritObjectPtr &self)
{
        if (self.isNull()) {
                lua_pushnil(L);
        } else {
                push(L,new GritObjectPtr(self),GRITOBJ_TAG);
        }
}

static int gritobj_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(GritObjectPtr,self,1,GRITOBJ_TAG,0);
        delete self;
        return 0;
TRY_END
}

static int gritobj_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        streamer->deleteObject(L,self);
        return 0;
TRY_END
}

static int gritobj_activate (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        self->activate(L, self);
        return 0;
TRY_END
}

static int gritobj_update_sphere (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        Vector3 pos = check_v3(L,2);
        float r = luaL_checknumber(L,3);
        self->updateSphere(pos, r);
        return 0;
TRY_END
}

static int gritobj_hint_advance_prepare (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        std::string type = luaL_checkstring(L,2);
        std::string name = luaL_checkstring(L,3);
        self->hintPrepareInAdvance(type,name);
        return 0;
TRY_END
}

static int gritobj_get_advance_prepare_hints (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        const GritObject::StringPairs &hints = self->getAdvanceHints();
        typedef GritObject::StringPairs::const_iterator I;
        int counter = 0;
        for (I i=hints.begin(),i_=hints.end() ; i!=i_ ; ++i) {
                const std::string &type = i->first;
                const std::string &name = i->second;
                lua_pushstring(L,type.c_str());
                lua_pushstring(L,name.c_str());
                counter += 2;
        }
        return counter;
TRY_END
}

static int gritobj_deactivate (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        self->deactivate(L,self);
        return 0;
TRY_END
}



TOSTRING_GETNAME_MACRO(gritobj,GritObjectPtr,->name,GRITOBJ_TAG)



static int gritobj_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="destroy") {
                push_cfunction(L,gritobj_destroy);
        } else if (key=="activated") {
                lua_pushboolean(L,self->isActivated());
        } else if (key=="near") {
                push_gritobj(L,self->getNear());
        } else if (key=="far") {
                push_gritobj(L,self->getFar());
        } else if (key=="fade") {
                lua_pushnumber(L,self->getFade());
        } else if (key=="updateSphere") {
                push_cfunction(L,gritobj_update_sphere);
        } else if (key=="pos") {
                push_v3(L, self->getPos());
        } else if (key=="renderingDistance") {
                lua_pushnumber(L, self->getR());
        } else if (key=="deactivate") {
                push_cfunction(L,gritobj_deactivate);
        } else if (key=="activate") {
                push_cfunction(L,gritobj_activate);
        } else if (key=="instance") {
                self->pushLuaTable(L);
        } else if (key=="hintAdvancePrepare") {
                push_cfunction(L,gritobj_hint_advance_prepare);
        } else if (key=="getAdvancePrepareHints") {
                push_cfunction(L,gritobj_get_advance_prepare_hints);
        } else if (key=="destroyed") {
                lua_pushboolean(L,self->getClass()==NULL);
        } else if (key=="class") {
                GritClass *c = self->getClass();
                if (c==NULL) my_lua_error(L,"GritObject destroyed");
                push_gritcls(L,c);
        } else if (key=="className") {
                GritClass *c = self->getClass();
                if (c==NULL) my_lua_error(L,"GritObject destroyed");
                lua_pushstring(L,c->name.c_str());
        } else if (key=="name") {
                lua_pushstring(L,self->name.c_str());
        } else if (key=="needsFrameCallbacks") {
                lua_pushboolean(L,self->getNeedsFrameCallbacks());
        } else if (key=="dump") {
                self->userValues.dump(L);
        } else {
                GritClass *c = self->getClass();
                if (c==NULL) my_lua_error(L,"GritObject destroyed");
                const char *err = self->userValues.luaGet(L);
                if (err) my_lua_error(L, err);
                if (!lua_isnil(L,-1)) return 1;
                lua_pop(L,1);
                // try class instead
                c->get(L,key);
        }
        return 1;
TRY_END
}

static int gritobj_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        std::string key = luaL_checkstring(L,2);

        if (key=="destroy") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="near") {
                if (lua_isnil(L,3)) {
                        self->setNear(self,GritObjectPtr());
                } else {
                        GET_UD_MACRO(GritObjectPtr,v,3,GRITOBJ_TAG);
                        self->setNear(self,v);
                }
        } else if (key=="far") {
                if (lua_isnil(L,3)) {
                        self->setNear(self,GritObjectPtr());
                } else {
                        GET_UD_MACRO(GritObjectPtr,v,3,GRITOBJ_TAG);
                        self->setFar(self,v);
                }
        } else if (key=="fade") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="updateSphere") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="pos") {
                self->updateSphere(check_v3(L,3));
        } else if (key=="renderingDistance") {
                self->updateSphere(luaL_checknumber(L,3));
        } else if (key=="getSphere") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="activated") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="deactivate") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="activate") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="instance") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="hintAdvancePrepare") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="getAdvancePrepareHints") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="destroyed") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="class") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="className") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="name") {
                my_lua_error(L,"Not a writeable GritObject member: "+key);
        } else if (key=="needsFrameCallbacks") {
                self->setNeedsFrameCallbacks(self, check_bool(L,3));
        } else {
                GritClass *c = self->getClass();
                if (c==NULL) my_lua_error(L,"GritObject destroyed");
                const char *err = self->userValues.luaSet(L);
                if (err) my_lua_error(L, err);
        }

        return 0;
TRY_END
}

EQ_MACRO(GritObjectPtr,gritobj,GRITOBJ_TAG)

MT_MACRO_NEWINDEX(gritobj);

//}}}



// STREAMER ================================================================ {{{

void push_streamer (lua_State *L, Streamer *self)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (self);
        luaL_getmetatable(L, STREAMER_TAG);
        lua_setmetatable(L, -2);
}

static int streamer_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Streamer,self,1,STREAMER_TAG,0);
        if (self==NULL) return 0;
        return 0;
TRY_END
}


static int streamer_centre (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        Vector3 pos = check_v3(L,2);
        self.centre(L, pos);
        return 0;
TRY_END
}

static int streamer_frame_callbacks (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        float t = luaL_checknumber(L,2);
        self.frameCallbacks(L,t);
        return 0;
TRY_END
}

static int streamer_add_class (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        const char *name = lua_tostring(L,2);
        if (name==NULL)
                my_lua_error(L,"Could not process the name of the new class");
        std::string fqname = pwd_full(L, name);
        if (!lua_istable(L,3))
                my_lua_error(L,"Second parameter should be a table");
        if (!lua_istable(L,4))
                my_lua_error(L,"Third parameter should be a table");
        self.addClass(L, fqname);
        return 1;
TRY_END
}

static int streamer_get_class (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        std::string name = pwd_full(L, luaL_checkstring(L,2));
        push_gritcls(L,self.getClass(name));
        return 1;
TRY_END
}

static int streamer_has_class (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        std::string name = pwd_full(L, luaL_checkstring(L,2));
        lua_pushboolean(L,self.hasClass(name));
        return 1;
TRY_END
}

static int streamer_remove_class (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        std::string name = pwd_full(L, luaL_checkstring(L,2));
        self.deleteClass(L,self.getClass(name));
        return 0;
TRY_END
}

static int streamer_add_object (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==3) lua_newtable(L);
        check_args(L,4);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        std::string className = pwd_full(L, luaL_checkstring(L,2));
        Vector3 spawnPos = check_v3(L, 3);
        int table_index = lua_gettop(L);
        if (!lua_istable(L,table_index)) my_lua_error(L,"Last parameter should be a table");
        lua_getfield(L,table_index,"name");
        std::string name;
        if (lua_isnil(L,-1)) {
                name = "";
        } else {
                if (lua_type(L,-1)!=LUA_TSTRING) my_lua_error(L,"Name wasn't a string!");
                name = lua_tostring(L,-1);
        }
        lua_pop(L,1);

        GritObjectPtr o = self.addObject(L,name,self.getClass(className));
        o->userValues.set("spawnPos", spawnPos);
        o->getClass()->get(L,"renderingDistance");
        if (lua_isnil(L,-1)) {
                self.deleteObject(L,o);
                my_lua_error(L,"no renderingDistance in class \""
                               +className+"\"");
        }
        if (lua_type(L,-1)!=LUA_TNUMBER) {
                self.deleteObject(L,o);
                my_lua_error(L,"renderingDistance not a number in class \""
                               +className+"\"");
        }
        float r = lua_tonumber(L,-1);
        o->updateSphere(spawnPos, r);
        lua_pop(L,1);

        lua_getfield(L,table_index,"near");
        if (!lua_isnil(L,-1)) {
                if (is_userdata(L,-1,GRITOBJ_TAG)) {
                        GET_UD_MACRO(GritObjectPtr,near,-1,GRITOBJ_TAG);
                        o->setNear(o, near);
                } else {
                        my_lua_error(L,"Field 'near' must be a grit object.");
                }
        }
        lua_pop(L,1);

        lua_getfield(L,table_index,"far");
        if (!lua_isnil(L,-1)) {
                if (is_userdata(L,-1,GRITOBJ_TAG)) {
                        GET_UD_MACRO(GritObjectPtr,far,-1,GRITOBJ_TAG);
                        o->setFar(o, far);
                } else {
                        my_lua_error(L,"Field 'far' must be a grit object.");
                }
        }
        lua_pop(L,1);

        // scan through table adding lua data to o
        for (lua_pushnil(L) ; lua_next(L,table_index)!=0 ; lua_pop(L,1)) {
                if (lua_type(L,-2)!=LUA_TSTRING) {
                        self.deleteObject(L,o);
                        my_lua_error(L,"user value key was not a string");
                }
                std::string key = luaL_checkstring(L,-2);
                // the name is held in the object anyway
                if (key=="name") continue;
                if (key=="near") continue;
                if (key=="far") continue;
                const char *err = o->userValues.luaSet(L);
                if (err) {
                        self.deleteObject(L,o);
                        my_lua_error(L, err);
                }
        }
        o->init(L,o);
        push_gritobj(L,o);
        return 1;
TRY_END
}

static int streamer_get_object (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        std::string name = luaL_checkstring(L,2);
        GritObjectPtr o = self.getObject(name);
        if (o.isNull()) {
                lua_pushnil(L);
        } else {
                push_gritobj(L,o);
        }
        return 1;
TRY_END
}

static int streamer_has_object (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        std::string name = luaL_checkstring(L,2);
        lua_pushboolean(L,self.hasObject(name));
        return 1;
TRY_END
}

static int streamer_remove_object (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        std::string name = luaL_checkstring(L,2);
        self.deleteObject(L,self.getObject(name));
        return 0;
TRY_END
}



static int streamer_clear_objects (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        self.clearObjects(L);
        return 0;
TRY_END
}

static int streamer_clear_anonymous_objects (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        self.clearAnonymousObjects(L);
        return 0;
TRY_END
}



static int streamer_clear_classes (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        self.clearClasses(L);
        return 0;
TRY_END
}

static int streamer_deactivate_all (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        GObjMap::iterator i, i_;
        for (self.getObjects(i,i_) ; i!=i_ ; ++i) {
                const GritObjectPtr &o = i->second;
                o->deactivate(L,o);
        }       
        return 0;
TRY_END
}

static int streamer_all_of_class (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        std::string cls = luaL_checkstring(L,2);
        lua_newtable(L);
        unsigned int c = 0;
        GObjMap::iterator i, i_;
        for (self.getObjects(i,i_) ; i!=i_ ; ++i) {
                const GritObjectPtr &o = i->second;
                GritClass *oc = o->getClass();
                if (oc==NULL) continue; // object destroyed
                if (oc->name != cls) continue; // wrong class
                push_gritobj(L,o);
                lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                c++;                 
        }       
        return 1;
TRY_END
}


TOSTRING_ADDR_MACRO (streamer,Streamer,STREAMER_TAG)


static int streamer_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        const char *key = luaL_checkstring(L,2);
        if (!::strcmp(key,"physics")) {
                push_pworld(L,self.getPhysics());
        } else if (!::strcmp(key,"stepSize")) {
                lua_pushnumber(L,self.stepSize);
        } else if (!::strcmp(key,"visibility")) {
                lua_pushnumber(L,self.visibility);
        } else if (!::strcmp(key,"centre")) {
                push_cfunction(L,streamer_centre);
        } else if (!::strcmp(key,"frameCallbacks")) {
                push_cfunction(L,streamer_frame_callbacks);
        } else if (!::strcmp(key,"addClass")) {
                push_cfunction(L,streamer_add_class);
        } else if (!::strcmp(key,"getClass")) {
                push_cfunction(L,streamer_get_class);
        } else if (!::strcmp(key,"hasClass")) {
                push_cfunction(L,streamer_has_class);
        } else if (!::strcmp(key,"removeClass")) {
                push_cfunction(L,streamer_remove_class);
        } else if (!::strcmp(key,"classes")) {
                lua_newtable(L);
                unsigned int c = 0;
                GritClassMap::iterator i, i_;
                for (self.getClasses(i,i_) ; i!=i_ ; ++i) {
                        push_gritcls(L,i->second);
                        lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                        c++;                 
                }       
        } else if (!::strcmp(key,"numClasses")) {
                lua_pushnumber(L,self.numClasses());
        } else if (!::strcmp(key,"addObject")) {
                push_cfunction(L,streamer_add_object);
        } else if (!::strcmp(key,"getObject")) {
                push_cfunction(L,streamer_get_object);
        } else if (!::strcmp(key,"hasObject")) {
                push_cfunction(L,streamer_has_object);
        } else if (!::strcmp(key,"removeObject")) {
                push_cfunction(L,streamer_remove_object);
        } else if (!::strcmp(key,"numObjects")) {
                lua_pushnumber(L,self.numObjects());
        } else if (!::strcmp(key,"numActivated")) {
                lua_pushnumber(L,self.numActivated());
        } else if (!::strcmp(key,"clearObjects")) {
                push_cfunction(L,streamer_clear_objects);
        } else if (!::strcmp(key,"clearAnonymousObjects")) {
                push_cfunction(L,streamer_clear_anonymous_objects);
        } else if (!::strcmp(key,"clearClasses")) {
                push_cfunction(L,streamer_clear_classes);
        } else if (!::strcmp(key,"deactivateAll")) {
                push_cfunction(L,streamer_deactivate_all);
        } else if (!::strcmp(key,"allOfClass")) {
                push_cfunction(L,streamer_all_of_class);
        } else if (!::strcmp(key,"prepareDistanceFactor")) {
                lua_pushnumber(L,self.prepareDistanceFactor);
        } else if (!::strcmp(key,"fadeOverlapFactor")) {
                lua_pushnumber(L,self.fadeOverlapFactor);
        } else if (!::strcmp(key,"fadeOutFactor")) {
                lua_pushnumber(L,self.fadeOutFactor);
        } else if (!::strcmp(key,"objects")) {
                lua_newtable(L);
                unsigned int c = 0;
                GObjMap::iterator i, i_;
                for (self.getObjects(i,i_) ; i!=i_ ; ++i) {
                        const GritObjectPtr &o = i->second;
                        push_gritobj(L,o);
                        lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                        c++;                 
                }       
        } else if (!::strcmp(key,"activated")) {
                lua_newtable(L);
                unsigned int c = 0;
                GObjMap::iterator i, i_;
                for (self.getObjects(i,i_) ; i!=i_ ; ++i) {
                        const GritObjectPtr &o = i->second;
                        if (!o->isActivated()) continue;
                        push_gritobj(L,o);
                        lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                        c++;                 
                }       
        } else {
                my_lua_error(L,"Not a readable Streamer member: "+std::string(key));
        }
        return 1;
TRY_END
}

static int streamer_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Streamer,self,1,STREAMER_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="stepSize") {
                size_t v = check_t<size_t>(L,3);
                self.stepSize = v;
        } else if (key=="visibility") {
                float v = luaL_checknumber(L,3);
                self.visibility = v;
        } else if (key=="prepareDistanceFactor") {
                float v = luaL_checknumber(L,3);
                self.prepareDistanceFactor = v;
        } else if (key=="fadeOverlapFactor") {
                float v = luaL_checknumber(L,3);
                self.fadeOverlapFactor = v;
        } else if (key=="fadeOutFactor") {
                float v = luaL_checknumber(L,3);
                self.fadeOutFactor = v;
        } else {
               my_lua_error(L,"Not a writeable Streamer member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Streamer,streamer,STREAMER_TAG)

MT_MACRO_NEWINDEX(streamer);

//}}}



// vim: shiftwidth=8:tabstop=8:expandtab
