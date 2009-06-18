#include <cmath>

#include "PhysicsWorld.h"
#include "GritObject.h"

#include "lua_util.h"
#include "lua_wrappers_gritobj.h"
#include "lua_wrappers_physics.h"
#include "lua_wrappers_scnmgr.h"
#include "Grit.h"

GritObject::GritObject (const Ogre::String &name_,
                        GritClass *gritClass_)
      : name(name_), 
        gritClass(gritClass_),
        lua(LUA_NOREF),
        demandRegistered(false),
        imposedFarFade(1.0),
        lastFade(-1)
{       
        gritClass->acquire();
}       

void GritObject::destroy (lua_State *L, const GritObjectPtr &self)
{
        if (gritClass==NULL) return;;
        setNear(self,GritObjectPtr());
        setFar(self,GritObjectPtr());
        deactivate(L,self);
        gritClass->release(L);
        gritClass = NULL;
        tryUnloadResources();
}       


void GritObject::doQueueBGPrepare (Ogre::Real x, Ogre::Real y, Ogre::Real z)
{
        std::vector<Ogre::ResourcePtr> &rs = demand.rPtrs;
        rs.reserve(advanceResources.size());
        bool background_prepare_needed = false;
        for (StringPairs::iterator i=advanceResources.begin(),
                                   i_=advanceResources.end() ;
             i!=i_ ; ++i) {
                const Ogre::String &type = i->first;
                const Ogre::String &name = i->second;
                if (type=="Mesh") {
                        Ogre::HardwareBuffer::Usage u =
                                Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY;
                        Ogre::MeshPtr mp = Ogre::MeshManager::getSingleton()
                                .createOrRetrieve(name,"GRIT",
                                                  false,0,
                                                  0,
                                                  u,u,false,false)
                                        .first;
                        rs.push_back(mp);
                        if (!mp->isPrepared() && !mp->isLoaded()) {
                                background_prepare_needed = true;
                        }
                } else if (type=="Texture") {
                        Ogre::TexturePtr tp =
                                Ogre::TextureManager::getSingleton()
                                        .createOrRetrieve(name,"GRIT")
                                                .first;
                        rs.push_back(tp);
                        if (!tp->isPrepared() && !tp->isLoaded()) {
                                background_prepare_needed = true;
                        }
                } else if (type=="Col") {
                        // do nothing at present, Col files loaded at startup.
                }
        }
        if (background_prepare_needed) {
                updateDemand(x,y,z);
                demand.mProcessed = false;
                BackgroundMeshLoader::getSingleton().checkGPUUsage();
                BackgroundMeshLoader::getSingleton().add(&demand);
        } else {
                demand.mProcessed = true;
        }
        demandRegistered = true;
}


void finished_with_mesh(Ogre::Mesh *m)
{
        Ogre::Mesh::SubMeshIterator i = m->getSubMeshIterator();

        while (i.hasMoreElements()) {
                Ogre::SubMesh *sm = i.getNext();

                Ogre::String matname = sm->getMaterialName();

                Ogre::MaterialPtr mat;

                mat = Ogre::MaterialManager::getSingleton().getByName(matname);
                BackgroundMeshLoader::getSingleton().finishedWith(mat);

                matname += "'";
                mat = Ogre::MaterialManager::getSingleton().getByName(matname);
                BackgroundMeshLoader::getSingleton().finishedWith(mat);
        }
}



void GritObject::tryUnloadResources(void)
{
        BackgroundMeshLoader::getSingleton().remove(&demand);
        ResourcePtrs &rps = demand.rPtrs;
        ResourcePtrSet s;
        for (ResourcePtrs::iterator i=rps.begin(),i_=rps.end() ; i!=i_ ; ++i) {
                const Ogre::ResourcePtr &rp = *i;
                s.insert(rp);
                Ogre::Mesh *m = dynamic_cast<Ogre::Mesh*>(&*rp);
                if (m!=NULL) finished_with_mesh(m);
        }
        demand.rPtrs.clear();
        BackgroundMeshLoader::getSingleton().finishedWith(s);
        demandRegistered = false;
}


void GritObject::notifyFade (lua_State *L,
                             const GritObjectPtr &self,
                             const Ogre::Real fade,
                             const int transition)
{
        if (gritClass==NULL)
                OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
                            "Object destroyed",
                            "GritObject::notifyFade");

        if (!isActivated()) return;

        STACK_BASE;
        //stack is empty

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // call into lua...
        gritClass->pushLuaTable(L);
        //stack: err,class
        lua_getfield(L, -1, "setFade");
        //stack: err,class,callback
        if (lua_isnil(L,-1)) {
                // no setFade function, do nothing
                lua_pop(L,3);
                //stack is empty
                STACK_CHECK;
                return;
        }

        //stack: err,class,callback
        // we now have the callback to play with

        pushLuaTable(L); // the instance
        lua_pushnumber(L,fade);
        if (transition<0) {
                lua_pushnil(L);
        } else {
                lua_pushnumber(L, transition);
        }
        //stack: err,class,callback,object
        int status = lua_pcall(L,3,0,error_handler);
        if (status) {
                //stack: err,class,msg
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                grit->getGOM().deleteObject(L,self);
                //stack: err,class
        }
        //stack: err,class

        lua_pop(L,2);
        //stack is empty
        STACK_CHECK;

}

void GritObject::activate (lua_State *L,
                           const GritObjectPtr &self,
                           Ogre::SceneNode *root,
                           const PhysicsWorldPtr &physics)
{
        if (isActivated()) return;

        // can call in from lua after destroyed by deleteObject
        if (gritClass==NULL)
                OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
                            "Object destroyed",
                            "GritObject::activate");

        STACK_BASE;
        //stack is empty

        // error handler in case there is a problem during 
        // the callback
        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // get the activate function
        gritClass->pushLuaTable(L);
        //stack: err,class
        lua_getfield(L, -1, "activate");
        //stack: err,class,callback
        if (lua_isnil(L,-1)) {
                // don't activate it as class does not have activate function
                // pop both the error handler and the nil activate function
                // and the table
                lua_pop(L,3);
                CERR << "activating object: \""<<name<<"\": "
                     << "class \""<<gritClass->name<<"\" "
                     << "does not have activate function" << std::endl;
                grit->getGOM().deleteObject(L,self);
                STACK_CHECK;
                return;
        }

        //stack: err,class,callback
        STACK_CHECK_N(3);

        // we now have the callback to play with

        // push 4 args
        lua_checkstack(L,4);
        gritClass->pushLuaTable(L); // the class
        push_gritobj(L,self); // this
        push_node(L,root); // the graphics
        push_pworld(L,physics); // the physics
        //stack: err,class,callback,class,persistent,gfx,physics
        STACK_CHECK_N(7);

        // call (4 args, returns the new object);
        int status = lua_pcall(L,4,1,error_handler);
        STACK_CHECK_N(3);
        if (status) {
                //stack: err,class,error
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                grit->getGOM().deleteObject(L,self);
                //stack: err,class
                STACK_CHECK_N(2);
        } else {
                //stack: err,class,object
                grit->getGOM().list(self);
                // pop and store the new object returned       
                lua = luaL_ref(L,LUA_REGISTRYINDEX);
                lastFade = -1;
                //stack: err,class
                STACK_CHECK_N(2);
        }
        //stack: err,class

        STACK_CHECK_N(2);
        lua_pop(L,2);
        //stack is empty
        STACK_CHECK;
}

Ogre::Real GritObject::calcFade (const Ogre::Real range2, bool &overlap)
{
        const GritObjectManager &gom = grit->getGOM();

        const GritObjectPtr &near = getNear();
        const GritObjectPtr &far = getFar();

        const Ogre::Real out = gom.fadeOutFactor;

        const Ogre::Real over = gom.fadeOverlapFactor;


        Ogre::Real range = ::sqrt(range2);

        Ogre::Real fade = 1.0;
        // if near is not activated, farfade will be out of date
        if (!near.isNull() && near->isActivated()) {
                fade = near->getImposedFarFade();
                if (fade<1) overlap = true;
        }
        if (far.isNull()) {
                if (range > out) {
                        fade = (1-range) / (1-out);
                }
                // doesn't actually do anything as there is no far
                imposedFarFade = 1.0;
        } else {
                //TODO: generalise hte following 2 options together
                const Ogre::Real overmid = (over+1)/2;
                if (range > overmid) {
                        fade = (1-range) / (1-overmid);
                        imposedFarFade = 1;
                } else if (range > over) {
                        imposedFarFade = 1  -  (overmid-range) / (overmid-over);
                } else {
                        imposedFarFade = 0;
                }
/*
                if (range > over) {
                        fade = (1-range) / (1-over);
                        imposedFarFade = 1-fade;
                } else {
                        imposedFarFade = 0;
                }
*/
        }
        if (fade<0) fade = 0;
        if (imposedFarFade<0) imposedFarFade = 0;
        return fade;
}

bool GritObject::deactivate (lua_State *L, const GritObjectPtr &self)
{
        if (gritClass==NULL)
                OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
                            "Object destroyed",
                            "GritObject::deactivate");

        if (!isActivated()) return false;

        bool killme = false;

        grit->getGOM().unlist(self);

        STACK_BASE;
        //stack is empty

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // call into lua...
        gritClass->pushLuaTable(L);
        //stack: err,class
        lua_getfield(L, -1, "deactivate");
        //stack: err,class,callback
        if (lua_isnil(L,-1)) {
                lua_pop(L,3);
                //stack is empty
                CERR << "deactivating object: \""<<name<<"\": "
                     << "class \""<<gritClass->name<<"\" "
                     << "does not have deactivate function" << std::endl;
                luaL_unref(L,LUA_REGISTRYINDEX,lua);
                lua = LUA_NOREF;

                STACK_CHECK;
                // returning true indicates the object will be erased
                // to prevent the error reoccuring
                return true;
        }

        //stack: err,class,callback
        // we now have the callback to play with

        pushLuaTable(L); // the instance
        //stack: err,class,callback,object
        int status = lua_pcall(L,1,1,error_handler);
        if (status) {
                //stack: err,class,msg
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                killme = true;
                //stack: err,class
        } else {
                //stack: err,class,killme
                killme = 0!=lua_toboolean(L,-1);
                lua_pop(L,1);
                //stack: err,class
        }
        //stack: err,class

        luaL_unref(L,LUA_REGISTRYINDEX,lua);
        lua = LUA_NOREF;

        lua_pop(L,2);
        //stack is empty
        STACK_CHECK;

        return killme;
}


void GritObject::init (lua_State *L, const GritObjectPtr &self)
{
        if (gritClass==NULL)
                OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
                            "Object destroyed",
                            "GritObject::init");

        STACK_BASE;
        //stack is empty

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // could get the init function from the object itself, but
        // this would only lead to pulling the function from the class
        // indirectly which is a waste of time, may as well get it straight
        // from the class
        gritClass->pushLuaTable(L);
        //stack: err,class
        lua_getfield(L, -1, "init");
        //stack: err,class,callback
        if (lua_isnil(L,-1)) {
                lua_pop(L,3);
                //stack is empty
                STACK_CHECK;
                return;
        }

        //stack: err,class,callback
        // we now have the callback to play with

        lua_checkstack(L,2);
        push_gritobj(L,self); // persistent grit obj
        gritClass->pushLuaTable(L); // the class
        //stack: err,class,callback,instance
        int status = lua_pcall(L,2,0,error_handler);
        if (status) {
                //stack: err,class,msg
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                //stack: err,class
        }
        //stack: err,class

        lua_pop(L,2);
        //stack is empty
        STACK_CHECK;

}

void GritObject::updateSphere (Ogre::Real x_, Ogre::Real y_,
                               Ogre::Real z_, Ogre::Real r_)
{
        if (index==-1) return;
        x = x_;
        y = y_;
        z = z_;
        r = r_;
        grit->getGOM().updateSphere(index,x_,y_,z_,r_);
}


// vim: shiftwidth=8:tabstop=8:expandtab
