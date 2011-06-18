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

#include <cmath>

#include <OgreException.h>
#include <OgreSceneNode.h>
#include <OgreMeshManager.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreTexture.h>
#include <OgreSubMesh.h>

#include "main.h"
#include "GritObject.h"

#include "lua_util.h"
#include "lua_wrappers_gritobj.h"

GritObject::GritObject (const std::string &name_,
                        GritClass *gritClass_)
      : name(name_), 
        anonymous(false),
        gritClass(gritClass_),
        lua(LUA_NOREF),
        needsFrameCallbacks(false),
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
        userValues.destroy(L);
}       




void GritObject::notifyFade (lua_State *L,
                             const GritObjectPtr &self,
                             const float fade)
{
        if (gritClass==NULL) GRIT_EXCEPT("Object destroyed");

        if (!isActivated()) return;

        STACK_BASE;
        //stack is empty

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // call into lua...
        push_gritcls(L,gritClass);
        //stack: err,class
        gritClass->get(L,"setFade");
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

        push_gritobj(L,self); // persistent grit obj
        lua_pushnumber(L,fade); // fade
        //stack: err,class,callback,persistent, fade
        int status = lua_pcall(L,2,0,error_handler);
        if (status) {
                //stack: err,class,msg
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                streamer->deleteObject(L,self);
                //stack: err,class
        }
        //stack: err,class

        lua_pop(L,2);
        //stack is empty
        STACK_CHECK;

}

void GritObject::activate (lua_State *L,
                           const GritObjectPtr &self)
{
        if (isActivated()) return;

        // can call in from lua after destroyed by deleteObject
        if (gritClass==NULL) GRIT_EXCEPT("Object destroyed");

        STACK_BASE;
        //stack is empty

        // error handler in case there is a problem during 
        // the callback
        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // get the activate function
        push_gritcls(L,gritClass);
        //stack: err,class
        gritClass->get(L,"activate");
        //stack: err,class,callback
        if (lua_isnil(L,-1)) {
                // don't activate it as class does not have activate function
                // pop both the error handler and the nil activate function
                // and the table
                lua_pop(L,3);
                CERR << "activating object: \""<<name<<"\": "
                     << "class \""<<gritClass->name<<"\" "
                     << "does not have activate function" << std::endl;
                streamer->deleteObject(L,self);
                STACK_CHECK;
                return;
        }

        //stack: err,class,callback
        STACK_CHECK_N(3);

        // we now have the callback to play with

        // push 4 args
        lua_checkstack(L,5);
        //stack: err,class,callback
        push_gritobj(L,self); // persistent
        //stack: err,class,callback,persistent
        lua_newtable(L); // instance
        //stack: err,class,callback,persistent,instance
        lua_pushvalue(L,-1);
        //stack: err,class,callback,persistent,instance,instance
        lua = luaL_ref(L,LUA_REGISTRYINDEX); // set up the lua ref to the new instance while we're at it
        //stack: err,class,callback,persistent,instance
        STACK_CHECK_N(5);

        // call (2 args), pops function too
        int status = lua_pcall(L,2,0,error_handler);
        if (status) {
                STACK_CHECK_N(3);
                //stack: err,class,error
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                CERR << "Object: \"" << name << "\" raised an error on activation, so destroying it." << std::endl;
                // will deactivate us
                streamer->deleteObject(L,self);
                //stack: err,class
                STACK_CHECK_N(2);
        } else {
                STACK_CHECK_N(2);
                //stack: err,class,object
                streamer->list(self);
                // pop and store the new object returned       
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

float GritObject::calcFade (const float range2, bool &overlap)
{

        const GritObjectPtr &near = getNear();
        const GritObjectPtr &far = getFar();

        const float out = streamer->fadeOutFactor;

        const float over = streamer->fadeOverlapFactor;


        float range = ::sqrtf(range2);

        float fade = 1.0;
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
                const float overmid = (over+1)/2;
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
        if (gritClass==NULL) GRIT_EXCEPT("Object destroyed");

        if (!isActivated()) return false;

        bool killme = false;

        streamer->unlist(self);

        STACK_BASE;
        //stack is empty

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // call into lua...
        push_gritcls(L,gritClass);
        //stack: err,class
        gritClass->get(L,"deactivate");
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

        push_gritobj(L,self); // persistent grit obj
        //stack: err,class,callback
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
        if (gritClass==NULL) GRIT_EXCEPT("Object destroyed");

        STACK_BASE;
        //stack is empty

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // could get the init function from the object itself, but
        // this would only lead to pulling the function from the class
        // indirectly which is a waste of time, may as well get it straight
        // from the class
        push_gritcls(L,gritClass);
        //stack: err,class
        gritClass->get(L,"init");
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
        //stack: err,class,callback,persistent
        int status = lua_pcall(L,1,0,error_handler);
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

bool GritObject::frameCallback (lua_State *L, const GritObjectPtr &self, float elapsed)
{
        if (gritClass==NULL) GRIT_EXCEPT("Object destroyed");

        STACK_BASE;
        //stack is empty

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // could get the init function from the object itself, but
        // this would only lead to pulling the function from the class
        // indirectly which is a waste of time, may as well get it straight
        // from the class
        push_gritcls(L,gritClass);
        //stack: err,class
        gritClass->get(L,"frameCallback");
        //stack: err,class,callback
        if (lua_isnil(L,-1)) {
                lua_pop(L,3);
                //stack is empty
                STACK_CHECK;
                return false;
        }

        //stack: err,class,callback
        // we now have the callback to play with

        lua_checkstack(L,2);
        push_gritobj(L,self); // persistent grit obj
        lua_pushnumber(L, elapsed); // time since last frame
        //stack: err,class,callback,instance,elapsed
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

        return status==0;
}

void GritObject::updateSphere (const Vector3 &pos_, float r_)
{
        if (index==-1) return;
        pos = pos_;
        r = r_;
        streamer->updateSphere(index,pos,r);
}

void GritObject::updateSphere (const Vector3 &pos_)
{
        updateSphere(pos_, r);
}

void GritObject::updateSphere (float r_)
{
        updateSphere(pos,r_);
}

void GritObject::setNeedsFrameCallbacks (const GritObjectPtr &self, bool v)
{
        needsFrameCallbacks = v;
        streamer->setNeedsFrameCallbacks(self, v);
}

// vim: shiftwidth=8:tabstop=8:expandtab
