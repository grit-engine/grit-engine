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

#include <algorithm>

#include <OgreSimpleSpline.h>


#include "lua_wrappers_primitives.h"
#include "gfx.h"
#include "SplineTable.h"
#include "math_util.h"




// PLOT ==================================================================== {{{

typedef SplineTable<float> Plot;

int plot_make (lua_State *L)
{
TRY_START
        Plot *self = new Plot();
        check_args(L,1);
        int table = lua_gettop(L);
        if (!lua_istable(L,table))
                my_lua_error(L,"Parameter should be a table");
        for (lua_pushnil(L) ; lua_next(L,table)!=0 ; lua_pop(L,1)) {
                // the name is held in the object anyway
                float k = luaL_checknumber(L,-2);
                float v = luaL_checknumber(L,-1);
                self->addPoint(k,v);
        }
        self->commit();
        push(L,self,PLOT_TAG);
        return 1;
TRY_END
}

TOSTRING_ADDR_MACRO(plot,Plot,PLOT_TAG)

GC_MACRO(Plot,plot,PLOT_TAG)

static int plot_index(lua_State *L)
{
TRY_START
        typedef Plot::Map Map;
        typedef Plot::MI MI;
        check_args(L,2);
        GET_UD_MACRO(Plot,self,1,PLOT_TAG);
        if (lua_type(L,2)==LUA_TNUMBER) {
                float k = luaL_checknumber(L,2);
                lua_pushnumber(L,self[k]);
        } else {
                std::string key  = luaL_checkstring(L,2);
                if (key=="minX") {
                        lua_pushnumber(L,self.minX());
                } else if (key=="maxX") {
                        lua_pushnumber(L,self.maxX());
                } else if (key=="points") {
                        Map data = self.getPoints();
                        lua_createtable(L, data.size(), 0);
                        for (MI i=data.begin(), i_=data.end() ; i!=i_ ; ++i) {
                                lua_pushnumber(L,i->first);
                                lua_pushnumber(L,i->second);
                                lua_settable(L,-3);
                        }
                } else if (key=="tangents") {
                        Map data = self.getTangents();
                        lua_createtable(L, data.size(), 0);
                        for (MI i=data.begin(), i_=data.end() ; i!=i_ ; ++i) {
                                lua_pushnumber(L,i->first);
                                lua_pushnumber(L,i->second);
                                lua_settable(L,-3);
                        }
                } else {
                        my_lua_error(L,"Not a readable Plot member: "+key);
                }
        }
        return 1;
TRY_END
}

static int plot_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Plot,self,1,PLOT_TAG);
        (void) self;
        std::string key  = luaL_checkstring(L,2);
        if (false) {
        } else {
                my_lua_error(L,"Not a writeable Plot member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Plot,plot,PLOT_TAG)

MT_MACRO_NEWINDEX(plot);

// }}}


// STRINGDB ================================================================ {{{

typedef std::vector<std::string> StringDB;


int stringdb_make (lua_State *L)
{
TRY_START
        StringDB self;
        if (lua_gettop(L)==1) {
                int table = lua_gettop(L);
                if (!lua_istable(L,table))
                        my_lua_error(L,"Parameter should be a table");
                for (lua_pushnil(L) ; lua_next(L,table)!=0 ; lua_pop(L,1)) {
                        // the name is held in the object anyway
                        std::string str = luaL_checkstring(L,-1);
                        self.push_back(str);
                }
        } else {
                check_args(L,0);
        }
        push(L,new StringDB(self),STRINGDB_TAG);
        return 1;
TRY_END
}

std::ostream &operator<<(std::ostream &o, StringDB &db)
{
        o << "{";
        for (size_t i=0 ; i<db.size() ; ++i) {
                if (i>0) o << ", ";
                o << db[i];
        }
        o << "}";
        return o;
}
        


static int stringdb_add (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(StringDB,self,1,STRINGDB_TAG);
        self.push_back(luaL_checkstring(L,2));
        return 0;
TRY_END
}


static int stringdb_clear (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(StringDB,self,1,STRINGDB_TAG);
        self.clear();
        return 0;
TRY_END
}


TOSTRING_MACRO(stringdb,StringDB,STRINGDB_TAG)

GC_MACRO(StringDB,stringdb,STRINGDB_TAG)

static int stringdb_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(StringDB,self,1,STRINGDB_TAG);
        if (lua_type(L,2)==LUA_TNUMBER) {
                if (self.size()==0) {
                        my_lua_error(L,"Empty stringdb");
                }
                unsigned short index=check_t<unsigned short>(L,2,1,self.size());
                lua_pushstring(L,self[index-1].c_str());
        } else {
                std::string key  = luaL_checkstring(L,2);
                if (key=="add") {
                        push_cfunction(L,stringdb_add);
                } else if (key=="clear") {
                        push_cfunction(L,stringdb_clear);
                } else if (key=="table") {
                        lua_createtable(L, self.size(), 0);
                        for (unsigned int i=0 ; i<self.size() ; i++) {
                                lua_pushnumber(L,i+LUA_ARRAY_BASE);
                                lua_pushstring(L,self[i].c_str());
                                lua_settable(L,-3);
                        }
                } else {
                        my_lua_error(L,"Not a readable StringDB member: "+key);
                }
        }
        return 1;
TRY_END
}

static int stringdb_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(StringDB,self,1,STRINGDB_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (lua_type(L,2)==LUA_TNUMBER) {
                if (self.size()==0) {
                        my_lua_error(L,"Empty stringdb");
                }
                unsigned short index=check_t<unsigned short>(L,2,1,self.size());
                std::string v = luaL_checkstring(L,3);
                self[index-1] = v;
        } else {
                if (key=="value") {
                        GET_UD_MACRO(StringDB,v,3,STRINGDB_TAG);
                        self = v;
                } else {
                        my_lua_error(L,"Not a writeable StringDB member: "+key);
                }
        }
        return 0;
TRY_END
}

static int stringdb_len(lua_State *L)
{
TRY_START
        check_args(L,2); // a
        GET_UD_MACRO(StringDB,self,1,STRINGDB_TAG);
        lua_pushnumber(L,self.size());
        return 1;
TRY_END
}

EQ_PTR_MACRO(StringDB,stringdb,STRINGDB_TAG)

MT_MACRO_LEN_NEWINDEX(stringdb);

// }}}


// SPLINE ================================================================== {{{


int spline_make (lua_State *L)
{
TRY_START
        // TODO: take table of vectors to initialise with
        push(L,new Ogre::SimpleSpline(),SPLINE_TAG);
        return 1;
TRY_END
}


static int spline_add (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SimpleSpline,self,1,SPLINE_TAG);
        GET_UD_MACRO(Vector3,v,2,VECTOR3_TAG);
        self.addPoint(to_ogre(v));
        return 0;
TRY_END
}


static int spline_clear (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SimpleSpline,self,1,SPLINE_TAG);
        self.clear();
        return 0;
TRY_END
}


TOSTRING_ADDR_MACRO(spline,Ogre::SimpleSpline,SPLINE_TAG)

GC_MACRO(Ogre::SimpleSpline,spline,SPLINE_TAG)

static int spline_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SimpleSpline,self,1,SPLINE_TAG);
        if (lua_type(L,2)==LUA_TNUMBER) {
                if (self.getNumPoints()==0) {
                        my_lua_error(L,"Empty spline");
                }
                float index = lua_tonumber(L,2);
                index = std::max(index,float(0));
                index = std::min(index,self.getNumPoints()-float(1));
                push(L,new Vector3(from_ogre(self.interpolate(index))),VECTOR3_TAG);
        } else {
                std::string key  = luaL_checkstring(L,2);
                if (key=="add") {
                        push_cfunction(L,spline_add);
                } else if (key=="clear") {
                        push_cfunction(L,spline_clear);
                } else {
                        my_lua_error(L,"Not a readable Spline member: "+key);
                }
        }
        return 1;
TRY_END
}

static int spline_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::SimpleSpline,self,1,SPLINE_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (lua_type(L,2)==LUA_TNUMBER) {
                if (self.getNumPoints()==0) {
                        my_lua_error(L,"Empty spline");
                }
                unsigned short index =
                        check_t<unsigned short>(L,2,0,self.getNumPoints()-1);
                GET_UD_MACRO(Vector3,v,3,VECTOR3_TAG);
                self.updatePoint(index,to_ogre(v));
        } else {
                if (key=="value") {
                        GET_UD_MACRO(Ogre::SimpleSpline,v,3,SPLINE_TAG);
                        self = v;
                } else {
                        my_lua_error(L,"Not a writeable Spline member: "+key);
                }
        }
        return 0;
TRY_END
}

static int spline_len(lua_State *L)
{
TRY_START
        check_args(L,2); // a
        GET_UD_MACRO(Ogre::SimpleSpline,self,1,SPLINE_TAG);
        lua_pushnumber(L,self.getNumPoints());
        return 1;

TRY_END
}

EQ_PTR_MACRO(Ogre::SimpleSpline,spline,SPLINE_TAG)

MT_MACRO_LEN_NEWINDEX(spline);

// }}}


// VECTOR3 ================================================================= {{{


int vector3_make (lua_State *L)
{
TRY_START
        float x = 0, y = 0, z = 0;
        if (lua_gettop(L)!=0) {
                check_args(L,3);
                x = luaL_checknumber(L,1);
                y = luaL_checknumber(L,2);
                z = luaL_checknumber(L,3);
        }
        push(L,new Vector3(x,y,z),VECTOR3_TAG);
        return 1;
TRY_END
}


static int vector3_xyz (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) {
                GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
                float x = luaL_checknumber(L,2);
                float y = luaL_checknumber(L,3);
                float z = luaL_checknumber(L,4);
                v.x = x;
                v.y = y;
                v.z = z;
                return 0;
        } else if (lua_gettop(L)==2) {
                GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
                GET_UD_MACRO(Vector3,v2,2,VECTOR3_TAG);
                v = v2;
                return 0;
        }
                
        check_args(L,1);
        GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
        lua_pushnumber(L,v.x);
        lua_pushnumber(L,v.y);
        lua_pushnumber(L,v.z);
        return 3;
TRY_END
}

static int vector3_scale (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
        lua_Number n = luaL_checknumber(L,2);
        v *= n;
        return 0;
TRY_END
}


static int vector3_append (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) {
                GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
                float x = luaL_checknumber(L,2);
                float y = luaL_checknumber(L,3);
                float z = luaL_checknumber(L,4);
                v.x+=x;
                v.y+=y;
                v.z+=z;
                return 0;
        }
        check_args(L,2);
        GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,v2,2,VECTOR3_TAG);
        v += v2;
        return 0;
TRY_END
}


static int vector3_remove (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) {
                GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
                float x = luaL_checknumber(L,2);
                float y = luaL_checknumber(L,3);
                float z = luaL_checknumber(L,4);
                v.x-=x;
                v.y-=y;
                v.z-=z;
                return 0;
        }
        check_args(L,2);
        GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,v2,2,VECTOR3_TAG);
        v -= v2;
        return 0;
TRY_END
}


static int vector3_remove_component (lua_State *L)
{
TRY_START
        Vector3 *self;
        Vector3 comp;
        if (lua_gettop(L)==4) {
                GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
                float x = luaL_checknumber(L,2);
                float y = luaL_checknumber(L,3);
                float z = luaL_checknumber(L,4);
                self = &v;
                comp = Vector3(x,y,z);
        } else {
                check_args(L,2);
                GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
                GET_UD_MACRO(Vector3,v2,2,VECTOR3_TAG);
                self = &v;
                comp = v2;
        }
        *self -= self->dot(comp)*comp;
        return 0;
TRY_END
}


static int vector3_rotate (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==5) {
                GET_UD_MACRO(Vector3,self,1,VECTOR3_TAG);
                float angle = luaL_checknumber(L,2);
                float x = luaL_checknumber(L,3);
                float y = luaL_checknumber(L,4);
                float z = luaL_checknumber(L,5);
                Vector3 axis(x,y,z);
                axis.normalise();
                self = Quaternion(Degree(angle), axis) * self;
                return 0;
        } else if (lua_gettop(L)==3) {
                GET_UD_MACRO(Vector3,self,1,VECTOR3_TAG);
                float angle = luaL_checknumber(L,2);
                GET_UD_MACRO(Vector3,a,3,VECTOR3_TAG);
                Vector3 axis = a.normalisedCopy();
                self = Quaternion(Degree(angle), axis) * self;
                return 0;
        }
        check_args(L,2);
        GET_UD_MACRO(Vector3,self,1,VECTOR3_TAG);
        GET_UD_MACRO(Quaternion,q,2,QUAT_TAG);
        self = q * self;
        return 0;
TRY_END
}


static int vector3_normalise (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
        lua_pushnumber(L,v.normalise());
        return 1;
TRY_END
}


static int vector3_distance_to (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        lua_pushnumber(L,a.distance(b));
        return 1;
TRY_END
}


static int vector3_squared_distance_to (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        lua_pushnumber(L,a.distance2(b));
        return 1;
TRY_END
}


static int vector3_get_rotation_to (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        push(L,new Quaternion(a.getRotationTo(b)),QUAT_TAG);
        return 1;
TRY_END
}


static int vector3_reflect (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        push(L,new Vector3(a.reflect(b)),VECTOR3_TAG);
        return 1;
TRY_END
}

/*
static int vector3_random_deviant (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        Degree angle(luaL_checknumber(L,2));
        push(L,new Vector3(a.randomDeviant(angle)),VECTOR3_TAG);
        return 1;
TRY_END
}
*/

static int vector3_mid_point (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        push(L,new Vector3(a.midPoint(b)),VECTOR3_TAG);
        return 1;
TRY_END
}

static int vector3_cross_product (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        push(L,new Vector3(a.cross(b)),VECTOR3_TAG);
        return 1;
TRY_END
}

static int vector3_set_as_cross_product (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Vector3,self,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,a,2,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,3,VECTOR3_TAG);
        self = a.cross(b);
        return 0;
TRY_END
}

static int vector3_dot (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        lua_pushnumber(L,a.dot(b));
        return 1;
TRY_END
}


TOSTRING_MACRO(vector3,Vector3,VECTOR3_TAG)

GC_MACRO(Vector3,vector3,VECTOR3_TAG)

static int vector3_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="x") {
                lua_pushnumber(L,v.x);
        } else if (key=="y") {
                lua_pushnumber(L,v.y);
        } else if (key=="z") {
                lua_pushnumber(L,v.z);
        } else if (key=="xyz") {
                push_cfunction(L,vector3_xyz);
        } else if (key=="append") {
                push_cfunction(L,vector3_append);
        } else if (key=="remove") {
                push_cfunction(L,vector3_remove);
        } else if (key=="removeComponent") {
                push_cfunction(L,vector3_remove_component);
        } else if (key=="scale") {
                push_cfunction(L,vector3_scale);
        } else if (key=="rotate") {
                push_cfunction(L,vector3_rotate);
        } else if (key=="normalise") {
                push_cfunction(L,vector3_normalise);
        } else if (key=="squaredLength") {
                lua_pushnumber(L,v.length2());
        } else if (key=="distanceTo") {
                push_cfunction(L,vector3_distance_to);
        } else if (key=="squaredDistanceTo") {
                push_cfunction(L,vector3_squared_distance_to);
        } else if (key=="dot") {
                push_cfunction(L,vector3_dot);
        } else if (key=="cross") {
                push_cfunction(L,vector3_cross_product);
        } else if (key=="setAsCross") {
                push_cfunction(L,vector3_set_as_cross_product);
        } else if (key=="midPoint") {
                push_cfunction(L,vector3_mid_point);
        } else if (key=="normalised") {
                push(L,new Vector3(v.normalisedCopy()),VECTOR3_TAG);
/*
        } else if (key=="perpendicular") {
                push(L,new Vector3(v.perpendicular()),VECTOR3_TAG);
        } else if (key=="randomDeviant") {
                push_cfunction(L,vector3_random_deviant);
*/
        } else if (key=="getRotationTo") {
                push_cfunction(L,vector3_get_rotation_to);
        } else if (key=="reflect") {
                push_cfunction(L,vector3_reflect);
        } else {
                my_lua_error(L,"Not a valid Vector3 member: "+key);
        }
        return 1;
TRY_END
}

static int vector3_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="x") {
                lua_Number val = luaL_checknumber(L,3);
                v.x = val;
        } else if (key=="y") {
                lua_Number val = luaL_checknumber(L,3);
                v.y = val;
        } else if (key=="z") {
                lua_Number val = luaL_checknumber(L,3);
                v.z = val;
        } else if (key=="value") {
                GET_UD_MACRO(Vector3,v2,3,VECTOR3_TAG);
                v = v2;
        } else {
                my_lua_error(L,"Not a valid Vector3 member: "+key);
        }
        return 0;
TRY_END
}

static int vector3_len(lua_State *L)
{
TRY_START
        check_args(L,2); // a
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        lua_pushnumber(L,a.length());
        return 1;

TRY_END
}

static int vector3_div(lua_State *L)
{
TRY_START
        check_args(L,2); // a / b

        if (lua_type(L, 1)==LUA_TNUMBER) {
                lua_Number a = luaL_checknumber(L,1);
                GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
                push(L,new Vector3(a/b),VECTOR3_TAG);
                return 1;
        }

        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);

        if (lua_type(L,2)==LUA_TNUMBER) {
                lua_Number b = luaL_checknumber(L,2);
                push(L,new Vector3(a/b),VECTOR3_TAG);
        } else {
                GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
                push(L,new Vector3(a/b),VECTOR3_TAG);
        }

        return 1;
TRY_END
}

static int vector3_mul(lua_State *L)
{
TRY_START
        check_args(L,2); // a * b

        if (lua_type(L, 1)==LUA_TNUMBER) { // n*v
                lua_Number a = luaL_checknumber(L,1);
                GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
                push(L,new Vector3(a*b),VECTOR3_TAG);
                return 1;
        }

        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);

        if (lua_type(L,2)==LUA_TNUMBER) { // v*n
                lua_Number b = luaL_checknumber(L,2);
                push(L,new Vector3(a*b),VECTOR3_TAG);
        } else if (has_tag(L,2,QUAT_TAG)) { // v*q
                GET_UD_MACRO(Quaternion,b,2,QUAT_TAG);
                push(L,new Vector3(b*a),VECTOR3_TAG);
        } else {
                GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
                push(L,new Vector3(a*b),VECTOR3_TAG);
        }
        return 1;
TRY_END
}

static int vector3_add(lua_State *L)
{
TRY_START
        check_args(L,2); // a + b
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        push(L,new Vector3(a+b),VECTOR3_TAG);
        return 1;

TRY_END
}

static int vector3_sub(lua_State *L)
{
TRY_START
        check_args(L,2); // a - b
        GET_UD_MACRO(Vector3,a,1,VECTOR3_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        push(L,new Vector3(a-b),VECTOR3_TAG);
        return 1;

TRY_END
}

static int vector3_unm(lua_State *L)
{
TRY_START
        check_args(L,2); // -v
        GET_UD_MACRO(Vector3,v,1,VECTOR3_TAG);

        push(L,new Vector3(-v),VECTOR3_TAG);
        return 1;

TRY_END
}

EQ_MACRO(Vector3,vector3,VECTOR3_TAG)

MT_MACRO_ARITH_NEWINDEX(vector3);

// }}}


// QUAT ==================================================================== {{{



int quat_make (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==2) {
                lua_Number degrees = luaL_checknumber(L,1);
                GET_UD_MACRO(Vector3,v,2,VECTOR3_TAG);
                push(L,new Quaternion(Degree(degrees),v),QUAT_TAG);
        } else {
                float w = 1, x = 0, y = 0, z = 0;
                if (lua_gettop(L)!=0) {
                        check_args(L,4);
                        w = luaL_checknumber(L,1);
                        x = luaL_checknumber(L,2);
                        y = luaL_checknumber(L,3);
                        z = luaL_checknumber(L,4);
                }
                push(L,new Quaternion(w,x,y,z),QUAT_TAG);
        }
        return 1;
TRY_END
}

static int quat_xyzw (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==5) {
                GET_UD_MACRO(Quaternion,q,1,QUAT_TAG);
                float w = luaL_checknumber(L,2);
                float x = luaL_checknumber(L,3);
                float y = luaL_checknumber(L,4);
                float z = luaL_checknumber(L,5);
                q.w = w;
                q.x = x;
                q.y = y;
                q.z = z;
                return 0;
        }
        check_args(L,1);
        GET_UD_MACRO(Quaternion,q,1,QUAT_TAG);
        lua_pushnumber(L,q.w);
        lua_pushnumber(L,q.x);
        lua_pushnumber(L,q.y);
        lua_pushnumber(L,q.z);
        return 4;
TRY_END
}


static int quat_invert (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Quaternion,q,1,QUAT_TAG);
        q = q.inverse();
        return 0;
TRY_END
}


static int quat_normalise (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Quaternion,q,1,QUAT_TAG);
        lua_pushnumber(L,q.normalise());
        return 1;
TRY_END
}


static int quat_tostring(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Quaternion,q,1,QUAT_TAG);
        std::stringstream ss;
        ss << "Quat("<<q.w<<","<<q.x<<","<<q.y<<","<<q.z<<")";
        lua_pushstring(L, ss.str().c_str());
        return 1;
TRY_END 
}


GC_MACRO(Quaternion,quat,QUAT_TAG)

static int quat_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Quaternion,q,1,QUAT_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="x") {
                lua_pushnumber(L,q.x);
        } else if (key=="y") {
                lua_pushnumber(L,q.y);
        } else if (key=="z") {
                lua_pushnumber(L,q.z);
        } else if (key=="w") {
                lua_pushnumber(L,q.w);
        } else if (key=="axis") {
                Vector3 axis(q.x, q.y, q.z);
                axis.normalise();
                push(L,new Vector3(axis),VECTOR3_TAG);
        } else if (key=="angle") {
                float r = 2*Ogre::Math::ACos(q.w).valueDegrees();
                lua_pushnumber(L,r);
        } else if (key=="wxyz") {
                push_cfunction(L,quat_xyzw);
        } else if (key=="invert") {
                push_cfunction(L,quat_invert);
        } else if (key=="inverse") {
                push(L,new Quaternion(q.inverse()),QUAT_TAG);
        } else if (key=="unitInverse") {
                push(L,new Quaternion(q.unitInverse()),QUAT_TAG);
        } else if (key=="normalise") {
                push_cfunction(L,quat_normalise);
        } else if (key=="normalised") {
                Quaternion *v = new Quaternion(q);
                v->normalise();
                push(L,v,QUAT_TAG);
        } else {
                my_lua_error(L,"Not a valid Quat member: "+key);
        }
        return 1;
TRY_END
}

static int quat_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Quaternion,q,1,QUAT_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="x") {
                lua_Number v = luaL_checknumber(L,3);
                q.x = v;
        } else if (key=="y") {
                lua_Number v = luaL_checknumber(L,3);
                q.y = v;
        } else if (key=="z") {
                lua_Number v = luaL_checknumber(L,3);
                q.z = v;
        } else if (key=="w") {
                lua_Number v = luaL_checknumber(L,3);
                q.w = v;
        } else if (key=="value") {
                GET_UD_MACRO(Quaternion,v,3,QUAT_TAG);
                q = v;
        } else {
                my_lua_error(L,"Not a valid Quat member: "+key);
        }
        return 0;
TRY_END
}

static int quat_len(lua_State *L)
{
TRY_START
        check_args(L,2); // a
        GET_UD_MACRO(Quaternion,q,1,QUAT_TAG);
        lua_pushnumber(L,q.length());
        return 1;
TRY_END
}

static int quat_div(lua_State *L)
{
TRY_START
        check_args(L,2); // a / b
        GET_UD_MACRO(Quaternion,a,1,QUAT_TAG);
        GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
        push(L,new Vector3(a.inverse()*b),VECTOR3_TAG);
        return 1;
TRY_END
}

static int quat_mul(lua_State *L)
{
TRY_START
        check_args(L,2); // a * b

        if (lua_type(L, 1)==LUA_TNUMBER) { // n*q
                lua_Number a = luaL_checknumber(L,1);
                GET_UD_MACRO(Quaternion,b,2,QUAT_TAG);
                push(L,new Quaternion(a*b),QUAT_TAG);
                return 1;
        }

        GET_UD_MACRO(Quaternion,a,1,QUAT_TAG);

        if (has_tag(L,2,VECTOR3_TAG)) { // q*v
                GET_UD_MACRO(Vector3,b,2,VECTOR3_TAG);
                push(L,new Vector3(a*b),VECTOR3_TAG);
        } else { // q*q
                GET_UD_MACRO(Quaternion,b,2,QUAT_TAG);
                push(L,new Quaternion(a*b),QUAT_TAG);
        }
        return 1;

TRY_END
}

static int quat_add(lua_State *L)
{
TRY_START
        check_args(L,2); // a + b
        GET_UD_MACRO(Quaternion,a,1,QUAT_TAG);
        GET_UD_MACRO(Quaternion,b,2,QUAT_TAG);
        push(L,new Quaternion(a+b),QUAT_TAG);
        return 1;
TRY_END
}

static int quat_sub(lua_State *L)
{
TRY_START
        check_args(L,2); // a - b
        GET_UD_MACRO(Quaternion,a,1,QUAT_TAG);
        GET_UD_MACRO(Quaternion,b,2,QUAT_TAG);
        push(L,new Quaternion(a-b),QUAT_TAG);
        return 1;
TRY_END
}

static int quat_unm(lua_State *L)
{
TRY_START
        check_args(L,2); // -a
        GET_UD_MACRO(Quaternion,a,1,QUAT_TAG);
        push(L,new Quaternion(-a),QUAT_TAG);
        return 1;
TRY_END
}

EQ_MACRO(Quaternion,quat,QUAT_TAG)

MT_MACRO_ARITH_NEWINDEX(quat);



// }}}


// vim: shiftwidth=8:tabstop=8:expandtab
