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

#include <OgreParticleIterator.h>

extern "C" {
        #include "lua.h"
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "Grit.h"
#include "CentralisedLog.h"
#include "LuaParticleSystem.h"
#include "lua_util.h"


LuaParticleData::LuaParticleData (void)
      : Ogre::ParticleVisualData(),age(0),function(LUA_REFNIL),arg(LUA_REFNIL)
{ }

void LuaParticleData::setFunctionArg (lua_State *L)
{
        arg = luaL_ref(L,LUA_REGISTRYINDEX);
        function = luaL_ref(L,LUA_REGISTRYINDEX);
}

void LuaParticleData::clearFunctionArg (lua_State *L)
{
        luaL_unref(L,LUA_REGISTRYINDEX,function);
        luaL_unref(L,LUA_REGISTRYINDEX,arg);
}

void LuaParticleData::callCleanup (lua_State *L)
{
        // called to indicate a voluntary particle death
        // or particle death due to error in lua callback
        // or particle death due to reaching quota

        if (function == LUA_REFNIL) return;

        STACK_BASE;

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);
        // stack: err

        lua_rawgeti(L,LUA_REGISTRYINDEX,function);
        lua_rawgeti(L,LUA_REGISTRYINDEX,arg);
        // stack: err,func,arg

        int status = lua_pcall(L,1,0,error_handler);
        if (status) {
                // stack: err,msg
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                //stack: err
        }
        //stack: err

        lua_pop(L,1);
        //stack empty

        STACK_CHECK;

        clearFunctionArg(L);

        // particle should be removed from system at this point
}

bool LuaParticleData::call (lua_State *L,
                            float &x, float &y, float &z,
                            float &rot,
                            float &w, float &h,
                            float &r, float &g, float &b,
                            float &a,
                            float &dx, float &dy, float &dz,
                            float elapsed)
{
        age += elapsed;

        if (function == LUA_REFNIL) return false;

        lua_checkstack(L,20);

        STACK_BASE;

        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);
        // stack: err

        int stack_base = lua_gettop(L);

        lua_rawgeti(L,LUA_REGISTRYINDEX,function);
        lua_rawgeti(L,LUA_REGISTRYINDEX,arg);

        // stack: err,func,arg


        lua_pushnumber(L,x);
        lua_pushnumber(L,y);
        lua_pushnumber(L,z);

        lua_pushnumber(L,rot);

        lua_pushnumber(L,w);
        lua_pushnumber(L,h);

        lua_pushnumber(L,r);
        lua_pushnumber(L,g);
        lua_pushnumber(L,b);
        lua_pushnumber(L,a);

        lua_pushnumber(L,dx);
        lua_pushnumber(L,dy);
        lua_pushnumber(L,dz);

        lua_pushnumber(L,age);

        lua_pushnumber(L,elapsed);

        // stack: err,func,arg,16xfloat

        bool destroy = false;

        int status = lua_pcall(L,16,LUA_MULTRET,error_handler);
        if (status) {
                // stack: err,msg
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                //stack: err
                destroy = true;
        } else {
                // stack: err,nresults*float
                int nresults = lua_gettop(L) - stack_base;
                if (nresults>13) {
                        CERR << "Expected no more than 13 return values from "
                             << "particle lua callback function" << std::endl;
                        destroy = true;
                } else if (nresults==1 && lua_type(L,-1)==LUA_TBOOLEAN &&
                    !lua_toboolean(L,-1)) {
                        destroy = true;
                } else {
                        switch (nresults) {
                                case 13:
                                dz = luaL_checknumber(L,stack_base+13);
                                case 12:
                                dy = luaL_checknumber(L,stack_base+12);
                                case 11:
                                dx = luaL_checknumber(L,stack_base+11);
                                case 10:
                                a = luaL_checknumber(L,stack_base+10);
                                case 9:
                                b = luaL_checknumber(L,stack_base+9);
                                case 8:
                                g = luaL_checknumber(L,stack_base+8);
                                case 7:
                                r = luaL_checknumber(L,stack_base+7);
                                case 6:
                                h = luaL_checknumber(L,stack_base+6);
                                case 5:
                                w = luaL_checknumber(L,stack_base+5);
                                case 4:
                                rot = luaL_checknumber(L,stack_base+4);
                                case 3:
                                z = luaL_checknumber(L,stack_base+3);
                                case 2:
                                y = luaL_checknumber(L,stack_base+2);
                                case 1:
                                x = luaL_checknumber(L,stack_base+1);
                                case 0:;
                        }
                }
                lua_pop(L,nresults);
                //stack: err
        }
        //stack: err

        lua_pop(L,1);
        //stack empty

        //even if there was an error, we keep the callback function registered
        //so we can call it in cleanup mode

        STACK_CHECK;

        return destroy;
}



void LuaParticleAffector::_affectParticles (Ogre::ParticleSystem *ps,
                                            float)
{
        Ogre::ParticleIterator pi = ps->_getIterator();
        unsigned long long time = micros();
        float elapsed = (lastTime - time)/1E6;
        lastTime = time;
        while (!pi.end()) {
                Ogre::Particle *p = pi.getNext();
                LuaParticleData *pd =
                        static_cast<LuaParticleData*>(p->getVisualData());
                pd->affect(grit->getLuaState(), p, elapsed);
        }        
}

void LuaParticleData::affect (lua_State *L,
                              Ogre::Particle *p, float elapsed)
{
        float x = p->position.x;
        float y = p->position.y;
        float z = p->position.z;
        float rot = p->rotation.valueDegrees();
        float w = p->mWidth;
        float h = p->mHeight;
        float r = p->colour.r;
        float g = p->colour.g;
        float b = p->colour.b;
        float a = p->colour.a;
        float dx = p->direction.x;
        float dy = p->direction.y;
        float dz = p->direction.z;

        bool destroy = call(L, x,y,z, rot, w,h, r,g,b,a, dx,dx,dz, elapsed);

        if (destroy) {
                callCleanup(L);
                p->timeToLive = -1;
        } else {
                p->position = Ogre::Vector3(x,y,z);
                p->direction = Ogre::Vector3(dx,dy,dz);
                p->setRotation(Ogre::Degree(rot));
                p->setDimensions(w,h);
                p->colour = Ogre::ColourValue(r,g,b,a);
        }

}
// vim: ts=8:sw=8:expandtab:textwidth=80
