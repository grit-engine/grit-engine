#include "GritObject.h"
#include "Grit.h"

#include "lua_wrappers_gritobj.h"
#include "lua_wrappers_physics.h"
#include "lua_wrappers_scnmgr.h"
#include "lua_wrappers_primitives.h"

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
        grit->getGOM().deleteObject(L,self);
        return 0;
TRY_END
}

static int gritobj_activate (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        self->activate(L, self,
                       grit->getGOM().getGFX(),
                       grit->getGOM().getPhysics());
        return 0;
TRY_END
}

static int gritobj_update_sphere (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        Ogre::Real x = luaL_checknumber(L,2);
        Ogre::Real y = luaL_checknumber(L,3);
        Ogre::Real z = luaL_checknumber(L,4);
        Ogre::Real r = luaL_checknumber(L,5);
        self->updateSphere(x,y,z,r);
        return 0;
TRY_END
}

static int gritobj_get_sphere (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        lua_pushnumber(L,self->getX());
        lua_pushnumber(L,self->getY());
        lua_pushnumber(L,self->getZ());
        lua_pushnumber(L,self->getR());
        return 4;
TRY_END
}

static int gritobj_hint_advance_prepare (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GritObjectPtr,self,1,GRITOBJ_TAG);
        Ogre::String type = luaL_checkstring(L,2);
        Ogre::String name = luaL_checkstring(L,3);
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
                const Ogre::String &type = i->first;
                const Ogre::String &name = i->second;
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
        } else if (key=="getSphere") {
                push_cfunction(L,gritobj_get_sphere);
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



// GRIT_OBJECT_MANAGER ===================================================== {{{

void push_gom (lua_State *L, GritObjectManager *self)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (self);
        luaL_getmetatable(L, GOM_TAG);
        lua_setmetatable(L, -2);
}

static int gom_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(GritObjectManager,self,1,GOM_TAG,0);
        if (self==NULL) return 0;
        return 0;
TRY_END
}


static int gom_get_bounds (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        const Ogre::AxisAlignedBox &aabb = self.getBounds();
        push(L,new Ogre::Vector3(aabb.getMinimum()),VECTOR3_TAG);
        push(L,new Ogre::Vector3(aabb.getMaximum()),VECTOR3_TAG);
        return 2;
TRY_END
}


static int gom_set_bounds (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        GET_UD_MACRO(Ogre::Vector3,min,2,VECTOR3_TAG);
        GET_UD_MACRO(Ogre::Vector3,max,3,VECTOR3_TAG);
        self.setBounds(L,Ogre::AxisAlignedBox(min,max));
        return 0;
TRY_END
}

static int gom_centre (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::Real x = luaL_checknumber(L,2);
        Ogre::Real y = luaL_checknumber(L,3);
        Ogre::Real z = luaL_checknumber(L,4);
        self.centre(L,x,y,z);
        return 0;
TRY_END
}

/*
static int gom_add_class (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        if (!lua_istable(L,2))
                my_lua_error(L,"Second parameter should be a table");
        lua_getfield(L,2,"name");
        if (lua_isnil(L,-1))
                my_lua_error(L,"Expected a \"name\" field.");
        if (lua_type(L,-1)!=LUA_TSTRING)
                my_lua_error(L,"The \"name\" field should be a string.");
        Ogre::String name = lua_tostring(L,-1);
        lua_pushnil(L);
        lua_setfield(L,2,"name");
        lua_pop(L,1);
        self.addClass(L, name);
        return 1;
TRY_END
}
*/

static int gom_add_class (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::String name = lua_tostring(L,2);
        if (!lua_istable(L,3))
                my_lua_error(L,"Second parameter should be a table");
        if (!lua_istable(L,4))
                my_lua_error(L,"Third parameter should be a table");
        self.addClass(L, name);
        return 1;
TRY_END
}

static int gom_get_class (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        push_gritcls(L,self.getClass(name));
        return 1;
TRY_END
}

static int gom_has_class (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        lua_pushboolean(L,self.hasClass(name));
        return 1;
TRY_END
}

static int gom_remove_class (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        self.deleteClass(L,self.getClass(name));
        return 0;
TRY_END
}

static int gom_add_object (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==3 || lua_gettop(L)==5)
                lua_newtable(L);
        if (lua_gettop(L)!=4)
                check_args(L,6);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::String className = luaL_checkstring(L,2);
        Ogre::Real x,y,z;
        if (lua_gettop(L)==4) {
                GET_UD_MACRO(Ogre::Vector3,val,3,VECTOR3_TAG);
                x = val.x;
                y = val.y;
                z = val.z;
        } else {
                x = luaL_checknumber(L,3);
                y = luaL_checknumber(L,4);
                z = luaL_checknumber(L,5);
        }
        int table_index = lua_gettop(L);
        if (!lua_istable(L,table_index))
                my_lua_error(L,"Last parameter should be a table");
        lua_getfield(L,table_index,"name");
        Ogre::String name;
        if (lua_isnil(L,-1)) {
                name = "";
        } else {
                if (lua_type(L,-1)==LUA_TSTRING) {
                        name = lua_tostring(L,-1);
                } else {
                        my_lua_error(L,"Name wasn't a string!");
                }
        }
        lua_pop(L,1);

        GritObjectPtr o = self.addObject(L,name,self.getClass(className));
        o->userValues.set("spawnPos", Ogre::Vector3(x,y,z));
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
        Ogre::Real r = lua_tonumber(L,-1);
        o->updateSphere(x,y,z,r);
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
                Ogre::String key = luaL_checkstring(L,-2);
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

static int gom_get_object (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        GritObjectPtr o = self.getObject(name);
        if (o.isNull()) {
                lua_pushnil(L);
        } else {
                push_gritobj(L,o);
        }
        return 1;
TRY_END
}

static int gom_has_object (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        lua_pushboolean(L,self.hasObject(name));
        return 1;
TRY_END
}

static int gom_remove_object (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        self.deleteObject(L,self.getObject(name));
        return 0;
TRY_END
}



static int gom_clear_objects (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        self.clearObjects(L);
        return 0;
TRY_END
}



static int gom_clear_classes (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        self.clearClasses(L);
        return 0;
TRY_END
}

static int gom_deactivate_all (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        GObjMap::iterator i, i_;
        for (self.getObjects(i,i_) ; i!=i_ ; ++i) {
                const GritObjectPtr &o = i->second;
                o->deactivate(L,o);
        }       
        return 0;
TRY_END
}

static int gom_all_of_class (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        Ogre::String cls = luaL_checkstring(L,2);
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


TOSTRING_ADDR_MACRO (gom,GritObjectManager,GOM_TAG)


static int gom_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="gfx") {
                push_node(L,self.getGFX());
        } else if (key=="physics") {
                push_pworld(L,self.getPhysics());
        } else if (key=="stepSize") {
                lua_pushnumber(L,self.stepSize);
        } else if (key=="visibility") {
                lua_pushnumber(L,self.visibility);
        } else if (key=="centre") {
                push_cfunction(L,gom_centre);
        } else if (key=="getBounds") {
                push_cfunction(L,gom_get_bounds);
        } else if (key=="setBounds") {
                push_cfunction(L,gom_set_bounds);
        } else if (key=="addClass") {
                push_cfunction(L,gom_add_class);
        } else if (key=="getClass") {
                push_cfunction(L,gom_get_class);
        } else if (key=="hasClass") {
                push_cfunction(L,gom_has_class);
        } else if (key=="removeClass") {
                push_cfunction(L,gom_remove_class);
        } else if (key=="classes") {
                lua_newtable(L);
                unsigned int c = 0;
                GritClassMap::iterator i, i_;
                for (self.getClasses(i,i_) ; i!=i_ ; ++i) {
                        push_gritcls(L,i->second);
                        lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                        c++;                 
                }       
        } else if (key=="numClasses") {
                lua_pushnumber(L,self.numClasses());
        } else if (key=="addObject") {
                push_cfunction(L,gom_add_object);
        } else if (key=="getObject") {
                push_cfunction(L,gom_get_object);
        } else if (key=="hasObject") {
                push_cfunction(L,gom_has_object);
        } else if (key=="removeObject") {
                push_cfunction(L,gom_remove_object);
        } else if (key=="numObjects") {
                lua_pushnumber(L,self.numObjects());
        } else if (key=="numActivated") {
                lua_pushnumber(L,self.numActivated());
        } else if (key=="clearObjects") {
                push_cfunction(L,gom_clear_objects);
        } else if (key=="clearClasses") {
                push_cfunction(L,gom_clear_classes);
        } else if (key=="deactivateAll") {
                push_cfunction(L,gom_deactivate_all);
        } else if (key=="allOfClass") {
                push_cfunction(L,gom_all_of_class);
        } else if (key=="prepareDistanceFactor") {
                lua_pushnumber(L,self.prepareDistanceFactor);
        } else if (key=="fadeOverlapFactor") {
                lua_pushnumber(L,self.fadeOverlapFactor);
        } else if (key=="fadeOutFactor") {
                lua_pushnumber(L,self.fadeOutFactor);
        } else if (key=="objects") {
                lua_newtable(L);
                unsigned int c = 0;
                GObjMap::iterator i, i_;
                for (self.getObjects(i,i_) ; i!=i_ ; ++i) {
                        const GritObjectPtr &o = i->second;
                        push_gritobj(L,o);
                        lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                        c++;                 
                }       
        } else if (key=="activated") {
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
                my_lua_error(L,"Not a readable ObjectManager member: "+key);
        }
        return 1;
TRY_END
}

static int gom_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GritObjectManager,self,1,GOM_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="gfx") {
                GET_UD_MACRO_OFFSET(Ogre::SceneNode,node,3,NODE_TAG,0);
                self.setGFX(L,node);
        } else if (key=="stepSize") {
                size_t v = check_t<size_t>(L,3);
                self.stepSize = v;
        } else if (key=="visibility") {
                Ogre::Real v = luaL_checknumber(L,3);
                self.visibility = v;
        } else if (key=="prepareDistanceFactor") {
                Ogre::Real v = luaL_checknumber(L,3);
                self.prepareDistanceFactor = v;
        } else if (key=="fadeOverlapFactor") {
                Ogre::Real v = luaL_checknumber(L,3);
                self.fadeOverlapFactor = v;
        } else if (key=="fadeOutFactor") {
                Ogre::Real v = luaL_checknumber(L,3);
                self.fadeOutFactor = v;
        } else {
               my_lua_error(L,"Not a writeable ObjectManager member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(GritObjectManager,gom,GOM_TAG)

MT_MACRO_NEWINDEX(gom);

//}}}



// vim: shiftwidth=8:tabstop=8:expandtab
