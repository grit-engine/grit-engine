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

#include <OgreVector3.h>
#include <OgreVector4.h>

#include <OgreCgProgram.h>
#include <OgreGLSLProgram.h>
#include <OgreGLSLGpuProgram.h>
#ifdef WIN32
#include <OgreD3D9HLSLProgram.h>
#endif

#include "lua_wrappers_gpuprog.h"

// GPUPROG ================================================================= {{{

void push_gpuprog (lua_State *L, const Ogre::HighLevelGpuProgramPtr &self)
{
        if (self.isNull())
                lua_pushnil(L);
        else
                push(L,new Ogre::HighLevelGpuProgramPtr(self),GPUPROG_TAG);
}

static int gpuprog_load (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        self->load();
        return 0;
TRY_END
}

static int gpuprog_unload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        self->unload();
        return 0;
TRY_END
}

static int gpuprog_reload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        self->reload();
        return 0;
TRY_END
}

static int gpuprog_set_constant_float (lua_State *L)
{
TRY_START
        int args = lua_gettop(L);
        switch (args) {
                case 3: {
                        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
                        std::string name = luaL_checkstring(L,2);
                        float v = luaL_checknumber(L,3);
                        self->getDefaultParameters()->setNamedConstant(name,v);
                } break;
                case 5: {
                        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
                        std::string name = luaL_checkstring(L,2);
                        Ogre::Vector3 v;
                        v.x = luaL_checknumber(L,3);
                        v.y = luaL_checknumber(L,4);
                        v.z = luaL_checknumber(L,5);
                        self->getDefaultParameters()->setNamedConstant(name,v);
                } break;
                case 6: {
                        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
                        std::string name = luaL_checkstring(L,2);
                        Ogre::Vector4 v;
                        v.x = luaL_checknumber(L,3);
                        v.y = luaL_checknumber(L,4);
                        v.z = luaL_checknumber(L,5);
                        v.w = luaL_checknumber(L,6);
                        self->getDefaultParameters()->setNamedConstant(name,v);
                } break;
                default:
                check_args(L,3);
                break;
        }
        return 0;
TRY_END
}

static int gpuprog_set_constant_int (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        std::string name = luaL_checkstring(L,2);
        int v = check_t<int>(L,3);
        self->getDefaultParameters()->setNamedConstant(name,v);
        return 0;
TRY_END
}

typedef Ogre::GpuProgramParameters::AutoConstantType ACT;

static ACT act_from_str (lua_State *L, const std::string& str)
{
        if (str=="WORLD_MATRIX") return Ogre::GpuProgramParameters::ACT_WORLD_MATRIX;
        if (str=="INVERSE_WORLD_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_WORLD_MATRIX;
        if (str=="TRANSPOSE_WORLD_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_WORLD_MATRIX;
        if (str=="INVERSE_TRANSPOSE_WORLD_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX;
        if (str=="WORLD_MATRIX_ARRAY_3x4") return Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4;
        if (str=="WORLD_MATRIX_ARRAY") return Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY;
        if (str=="VIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_VIEW_MATRIX;
        if (str=="INVERSE_VIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX;
        if (str=="TRANSPOSE_VIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_VIEW_MATRIX;
        if (str=="INVERSE_TRANSPOSE_VIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_VIEW_MATRIX;
        if (str=="PROJECTION_MATRIX") return Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX;
        if (str=="INVERSE_PROJECTION_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX;
        if (str=="TRANSPOSE_PROJECTION_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_PROJECTION_MATRIX;
        if (str=="INVERSE_TRANSPOSE_PROJECTION_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX;
        if (str=="VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_VIEWPROJ_MATRIX;
        if (str=="INVERSE_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_VIEWPROJ_MATRIX;
        if (str=="TRANSPOSE_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_VIEWPROJ_MATRIX;
        if (str=="INVERSE_TRANSPOSE_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX;
        if (str=="WORLDVIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_WORLDVIEW_MATRIX;
        if (str=="INVERSE_WORLDVIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_WORLDVIEW_MATRIX;
        if (str=="TRANSPOSE_WORLDVIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEW_MATRIX;
        if (str=="INVERSE_TRANSPOSE_WORLDVIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX;
        if (str=="WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX;
        if (str=="INVERSE_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_WORLDVIEWPROJ_MATRIX;
        if (str=="TRANSPOSE_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX;
        if (str=="INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX;
        if (str=="RENDER_TARGET_FLIPPING") return Ogre::GpuProgramParameters::ACT_RENDER_TARGET_FLIPPING;
        if (str=="FOG_COLOUR") return Ogre::GpuProgramParameters::ACT_FOG_COLOUR;
        if (str=="FOG_PARAMS") return Ogre::GpuProgramParameters::ACT_FOG_PARAMS;
        if (str=="SURFACE_AMBIENT_COLOUR") return Ogre::GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR;
        if (str=="SURFACE_DIFFUSE_COLOUR") return Ogre::GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR;
        if (str=="SURFACE_SPECULAR_COLOUR") return Ogre::GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR;
        if (str=="SURFACE_EMISSIVE_COLOUR") return Ogre::GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR;
        if (str=="SURFACE_SHININESS") return Ogre::GpuProgramParameters::ACT_SURFACE_SHININESS;
        if (str=="LIGHT_COUNT") return Ogre::GpuProgramParameters::ACT_LIGHT_COUNT;
        if (str=="AMBIENT_LIGHT_COLOUR") return Ogre::GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR;
        if (str=="LIGHT_DIFFUSE_COLOUR") return Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR;
        if (str=="LIGHT_SPECULAR_COLOUR") return Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR;
        if (str=="LIGHT_ATTENUATION") return Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION;
        if (str=="SPOTLIGHT_PARAMS") return Ogre::GpuProgramParameters::ACT_SPOTLIGHT_PARAMS;
        if (str=="LIGHT_POSITION") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION;
        if (str=="LIGHT_POSITION_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE;
        if (str=="LIGHT_POSITION_VIEW_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE;
        if (str=="LIGHT_DIRECTION") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION;
        if (str=="LIGHT_DIRECTION_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_OBJECT_SPACE;
        if (str=="LIGHT_DIRECTION_VIEW_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE;
        if (str=="LIGHT_DISTANCE_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_DISTANCE_OBJECT_SPACE;
        if (str=="LIGHT_POWER_SCALE") return Ogre::GpuProgramParameters::ACT_LIGHT_POWER_SCALE;
        if (str=="LIGHT_DIFFUSE_COLOUR_POWER_SCALED") return Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED;
        if (str=="LIGHT_SPECULAR_COLOUR_POWER_SCALED") return Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED;
        if (str=="LIGHT_DIFFUSE_COLOUR_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_ARRAY;
        if (str=="LIGHT_SPECULAR_COLOUR_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_ARRAY;
        if (str=="LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY;
        if (str=="LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY;
        if (str=="LIGHT_ATTENUATION_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION_ARRAY;
        if (str=="LIGHT_POSITION_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_ARRAY;
        if (str=="LIGHT_POSITION_OBJECT_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY;
        if (str=="LIGHT_POSITION_VIEW_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY;
        if (str=="LIGHT_DIRECTION_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_ARRAY;
        if (str=="LIGHT_DIRECTION_OBJECT_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY;
        if (str=="LIGHT_DIRECTION_VIEW_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY;
        if (str=="LIGHT_DISTANCE_OBJECT_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY;
        if (str=="LIGHT_POWER_SCALE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_POWER_SCALE_ARRAY;
        if (str=="SPOTLIGHT_PARAMS_ARRAY") return Ogre::GpuProgramParameters::ACT_SPOTLIGHT_PARAMS_ARRAY;
        if (str=="DERIVED_AMBIENT_LIGHT_COLOUR") return Ogre::GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR;
        if (str=="DERIVED_SCENE_COLOUR") return Ogre::GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR;
        if (str=="DERIVED_LIGHT_DIFFUSE_COLOUR") return Ogre::GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR;
        if (str=="DERIVED_LIGHT_SPECULAR_COLOUR") return Ogre::GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR;
        if (str=="DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY") return Ogre::GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY;
        if (str=="DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY") return Ogre::GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY;
        if (str=="LIGHT_NUMBER") return Ogre::GpuProgramParameters::ACT_LIGHT_NUMBER;
        if (str=="LIGHT_CASTS_SHADOWS") return Ogre::GpuProgramParameters::ACT_LIGHT_CASTS_SHADOWS;
        if (str=="SHADOW_EXTRUSION_DISTANCE") return Ogre::GpuProgramParameters::ACT_SHADOW_EXTRUSION_DISTANCE;
        if (str=="CAMERA_POSITION") return Ogre::GpuProgramParameters::ACT_CAMERA_POSITION;
        if (str=="CAMERA_POSITION_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE;
        if (str=="TEXTURE_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX;
        if (str=="TEXTURE_VIEWPROJ_MATRIX_ARRAY") return Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY;
        if (str=="TEXTURE_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX;
        if (str=="TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY") return Ogre::GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY;
        if (str=="SPOTLIGHT_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX;
        if (str=="SPOTLIGHT_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX;
        if (str=="CUSTOM") return Ogre::GpuProgramParameters::ACT_CUSTOM;
        if (str=="TIME") return Ogre::GpuProgramParameters::ACT_TIME;
        if (str=="TIME_0_X") return Ogre::GpuProgramParameters::ACT_TIME_0_X;
        if (str=="COSTIME_0_X") return Ogre::GpuProgramParameters::ACT_COSTIME_0_X;
        if (str=="SINTIME_0_X") return Ogre::GpuProgramParameters::ACT_SINTIME_0_X;
        if (str=="TANTIME_0_X") return Ogre::GpuProgramParameters::ACT_TANTIME_0_X;
        if (str=="TIME_0_X_PACKED") return Ogre::GpuProgramParameters::ACT_TIME_0_X_PACKED;
        if (str=="TIME_0_1") return Ogre::GpuProgramParameters::ACT_TIME_0_1;
        if (str=="COSTIME_0_1") return Ogre::GpuProgramParameters::ACT_COSTIME_0_1;
        if (str=="SINTIME_0_1") return Ogre::GpuProgramParameters::ACT_SINTIME_0_1;
        if (str=="TANTIME_0_1") return Ogre::GpuProgramParameters::ACT_TANTIME_0_1;
        if (str=="TIME_0_1_PACKED") return Ogre::GpuProgramParameters::ACT_TIME_0_1_PACKED;
        if (str=="TIME_0_2PI") return Ogre::GpuProgramParameters::ACT_TIME_0_2PI;
        if (str=="COSTIME_0_2PI") return Ogre::GpuProgramParameters::ACT_COSTIME_0_2PI;
        if (str=="SINTIME_0_2PI") return Ogre::GpuProgramParameters::ACT_SINTIME_0_2PI;
        if (str=="TANTIME_0_2PI") return Ogre::GpuProgramParameters::ACT_TANTIME_0_2PI;
        if (str=="TIME_0_2PI_PACKED") return Ogre::GpuProgramParameters::ACT_TIME_0_2PI_PACKED;
        if (str=="FRAME_TIME") return Ogre::GpuProgramParameters::ACT_FRAME_TIME;
        if (str=="FPS") return Ogre::GpuProgramParameters::ACT_FPS;
        if (str=="VIEWPORT_WIDTH") return Ogre::GpuProgramParameters::ACT_VIEWPORT_WIDTH;
        if (str=="VIEWPORT_HEIGHT") return Ogre::GpuProgramParameters::ACT_VIEWPORT_HEIGHT;
        if (str=="INVERSE_VIEWPORT_WIDTH") return Ogre::GpuProgramParameters::ACT_INVERSE_VIEWPORT_WIDTH;
        if (str=="INVERSE_VIEWPORT_HEIGHT") return Ogre::GpuProgramParameters::ACT_INVERSE_VIEWPORT_HEIGHT;
        if (str=="VIEWPORT_SIZE") return Ogre::GpuProgramParameters::ACT_VIEWPORT_SIZE;
        if (str=="VIEW_DIRECTION") return Ogre::GpuProgramParameters::ACT_VIEW_DIRECTION;
        if (str=="VIEW_SIDE_VECTOR") return Ogre::GpuProgramParameters::ACT_VIEW_SIDE_VECTOR;
        if (str=="VIEW_UP_VECTOR") return Ogre::GpuProgramParameters::ACT_VIEW_UP_VECTOR;
        if (str=="FOV") return Ogre::GpuProgramParameters::ACT_FOV;
        if (str=="NEAR_CLIP_DISTANCE") return Ogre::GpuProgramParameters::ACT_NEAR_CLIP_DISTANCE;
        if (str=="FAR_CLIP_DISTANCE") return Ogre::GpuProgramParameters::ACT_FAR_CLIP_DISTANCE;
        if (str=="PASS_NUMBER") return Ogre::GpuProgramParameters::ACT_PASS_NUMBER;
        if (str=="PASS_ITERATION_NUMBER") return Ogre::GpuProgramParameters::ACT_PASS_ITERATION_NUMBER;
        if (str=="ANIMATION_PARAMETRIC") return Ogre::GpuProgramParameters::ACT_ANIMATION_PARAMETRIC;
        if (str=="TEXEL_OFFSETS") return Ogre::GpuProgramParameters::ACT_TEXEL_OFFSETS;
        if (str=="SCENE_DEPTH_RANGE") return Ogre::GpuProgramParameters::ACT_SCENE_DEPTH_RANGE;
        if (str=="SHADOW_SCENE_DEPTH_RANGE") return Ogre::GpuProgramParameters::ACT_SHADOW_SCENE_DEPTH_RANGE;
        if (str=="SHADOW_COLOUR") return Ogre::GpuProgramParameters::ACT_SHADOW_COLOUR;
        if (str=="TEXTURE_SIZE") return Ogre::GpuProgramParameters::ACT_TEXTURE_SIZE;
        if (str=="INVERSE_TEXTURE_SIZE") return Ogre::GpuProgramParameters::ACT_INVERSE_TEXTURE_SIZE;
        if (str=="PACKED_TEXTURE_SIZE") return Ogre::GpuProgramParameters::ACT_PACKED_TEXTURE_SIZE;
        if (str=="TEXTURE_MATRIX") return Ogre::GpuProgramParameters::ACT_TEXTURE_MATRIX;
        if (str=="LOD_CAMERA_POSITION") return Ogre::GpuProgramParameters::ACT_LOD_CAMERA_POSITION;
        if (str=="LOD_CAMERA_POSITION_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_LOD_CAMERA_POSITION_OBJECT_SPACE;
        if (str=="LIGHT_CUSTOM") return Ogre::GpuProgramParameters::ACT_LIGHT_CUSTOM;

        my_lua_error(L,"Unknown auto-constant: "+str);
        return Ogre::GpuProgramParameters::ACT_WORLD_MATRIX; // never happens
}

static int gpuprog_set_auto_constant_int (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==5) {
                GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
                std::string name = luaL_checkstring(L,2);
                std::string t = luaL_checkstring(L,3);
                Ogre::uint16 v1 = check_t<Ogre::uint16>(L,4);
                Ogre::uint16 v2 = check_t<Ogre::uint16>(L,5);
                self->getDefaultParameters()->setNamedAutoConstant(name, act_from_str(L,t), v1, v2);
        } else if (lua_gettop(L)==4) {
                GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
                std::string name = luaL_checkstring(L,2);
                std::string t = luaL_checkstring(L,3);
                size_t v = check_t<size_t>(L,4);
                self->getDefaultParameters()->setNamedAutoConstant(name, act_from_str(L,t), v);
        } else {
                check_args(L,3);
                GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
                std::string name = luaL_checkstring(L,2);
                std::string t = luaL_checkstring(L,3);
                self->getDefaultParameters()->setNamedAutoConstant(name, act_from_str(L,t), 0);
        }
        return 0;
TRY_END
}

static int gpuprog_set_auto_constant_float (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        std::string name = luaL_checkstring(L,2);
        std::string t = luaL_checkstring(L,3);
        float v = luaL_checknumber(L,4);
        self->getDefaultParameters()->setNamedAutoConstantReal(name, act_from_str(L,t), v);
        return 0;
TRY_END
}

static int gpuprog_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG,0);
        if (self==NULL) return 0;
        return 0;
TRY_END
}

static int gpuprog_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        delete &self;
        return 0;
TRY_END
}



TOSTRING_GETNAME_MACRO(gpuprog,Ogre::HighLevelGpuProgramPtr,->getName(),GPUPROG_TAG)



static int gpuprog_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="destroy") {
                push_cfunction(L,gpuprog_destroy);
        } else if (key=="load") {
                push_cfunction(L,gpuprog_load);
        } else if (key=="unload") {
                push_cfunction(L,gpuprog_unload);
        } else if (key=="reload") {
                push_cfunction(L,gpuprog_reload);
        } else if (key=="isLoaded") {
                lua_pushboolean(L,self->isLoaded());
        } else if (key=="isPrepared") {
                lua_pushboolean(L,self->isPrepared());
        } else if (key=="useCount") {
                lua_pushnumber(L,self.useCount());
        } else if (key=="sourceFile") {
                lua_pushstring(L,self->getSourceFile().c_str());
        } else if (key=="source") {
                lua_pushstring(L,self->getSource().c_str());
        } else if (key=="asm") {
                if (self->_getBindingDelegate() == NULL)
                        lua_pushstring(L,"// Not compiled yet.");
                else
                        lua_pushstring(L,self->_getBindingDelegate()->getSource().c_str());
        } else if (key=="syntaxCode") {
                lua_pushstring(L,self->getSyntaxCode().c_str());
        } else if (key=="language") {
                lua_pushstring(L,self->getSyntaxCode().c_str());
        } else if (key=="setConstantFloat") {
                lua_pushcfunction(L,gpuprog_set_constant_float);
        } else if (key=="setConstantInt") {
                lua_pushcfunction(L,gpuprog_set_constant_int);
        } else if (key=="setAutoConstantFloat") {
                lua_pushcfunction(L,gpuprog_set_auto_constant_float);
        } else if (key=="setAutoConstantInt") {
                lua_pushcfunction(L,gpuprog_set_auto_constant_int);
        } else if (key=="skeletalAnimationIncluded") {
                lua_pushboolean(L,self->isSkeletalAnimationIncluded());
        } else if (key=="morphAnimationIncluded") {
                lua_pushboolean(L,self->isMorphAnimationIncluded());
        } else if (key=="poseAnimationIncluded") {
                lua_pushboolean(L,self->isPoseAnimationIncluded());
        } else if (key=="defines") {
                Ogre::GLSLProgram *glsl = dynamic_cast<Ogre::GLSLProgram*>(&*self);
                if (glsl) {
                        lua_pushstring(L,glsl->getPreprocessorDefines().c_str());
                } else {
                        #ifdef WIN32
                        Ogre::D3D9HLSLProgram *hlsl = dynamic_cast<Ogre::D3D9HLSLProgram*>(&*self);
                        if (hlsl) {
                                lua_pushstring(L,hlsl->getPreprocessorDefines().c_str());
                        } else
                        #endif
                                my_lua_error(L,"This kind of program does not support defines");
                }
        } else if (key=="target") {
                #ifdef WIN32
                Ogre::D3D9HLSLProgram *hlsl = dynamic_cast<Ogre::D3D9HLSLProgram*>(&*self);
                if (hlsl) {
                        lua_pushstring(L,hlsl->getTarget().c_str());
                } else
                #endif
                        lua_pushstring(L,"unknown");
        } else if (key=="entryPoint") {
                Ogre::CgProgram *cg = dynamic_cast<Ogre::CgProgram*>(&*self);
                if (cg) {
                        lua_pushstring(L,cg->getEntryPoint().c_str());
                } else {
                        #ifdef WIN32
                        Ogre::D3D9HLSLProgram *hlsl = dynamic_cast<Ogre::D3D9HLSLProgram*>(&*self);
                        if (hlsl) {
                                lua_pushstring(L,hlsl->getEntryPoint().c_str());
                        } else
                        #endif
                                lua_pushstring(L,"unknown");
                }
        } else if (key=="compileArguments") {
                Ogre::CgProgram *cg = dynamic_cast<Ogre::CgProgram*>(&*self);
                if (cg) {
                        lua_pushstring(L,cg->getCompileArguments().c_str());
                } else {
                        lua_pushstring(L,"unknown");
                }
        } else if (key=="profiles") {
                Ogre::CgProgram *cg = dynamic_cast<Ogre::CgProgram*>(&*self);
                if (cg) {
                        Ogre::StringVector profs = cg->getProfiles();

                        lua_createtable(L, profs.size(), 0);
                        for (unsigned int i=0 ; i<profs.size() ; i++) {
                                const char *key = profs[i].c_str();
                                lua_pushnumber(L,i+LUA_ARRAY_BASE);
                                lua_pushstring(L,key);
                                lua_settable(L,-3);
                        }

                } else {
                        lua_createtable(L, 0, 0);
                }
        } else {
                my_lua_error(L,"Not a readable GPUProg member: "+key);
        }
        return 1;
TRY_END
}

static int gpuprog_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="sourceFile") {
                std::string v = luaL_checkstring(L, 3);
                self->setSourceFile(v);
        } else if (key=="source") {
                std::string v = luaL_checkstring(L, 3);
                self->setSource(v);
        } else if (key=="syntaxCode") {
                std::string v = luaL_checkstring(L, 3);
                self->setSyntaxCode(v);
        } else if (key=="defines") {
                std::string v = luaL_checkstring(L, 3);
                Ogre::GLSLProgram *glsl = dynamic_cast<Ogre::GLSLProgram*>(&*self);
                if (glsl) {
                        glsl->setPreprocessorDefines(v);
                } else {
                        #ifdef WIN32
                        Ogre::D3D9HLSLProgram *hlsl = dynamic_cast<Ogre::D3D9HLSLProgram*>(&*self);
                        if (hlsl) {
                                hlsl->setPreprocessorDefines(v);
                        } else
                        #endif
                                my_lua_error(L,"This kind of program does not support defines");
                }
        } else if (key=="target") {
                std::string v = luaL_checkstring(L, 3);
                #ifdef WIN32
                Ogre::D3D9HLSLProgram *hlsl = dynamic_cast<Ogre::D3D9HLSLProgram*>(&*self);
                if (hlsl) {
                        hlsl->setTarget(v);
                }
                #endif
        } else if (key=="compileArguments") {
                std::string v = luaL_checkstring(L, 3);
                Ogre::CgProgram *cg = dynamic_cast<Ogre::CgProgram*>(&*self);
                if (cg) {
                        cg->setCompileArguments(v);
                }
        } else if (key=="entryPoint") {
                std::string v = luaL_checkstring(L, 3);
                Ogre::CgProgram *cg = dynamic_cast<Ogre::CgProgram*>(&*self);
                if (cg) {
                        cg->setEntryPoint(v);
                } else {
                        #ifdef WIN32
                        Ogre::D3D9HLSLProgram *hlsl = dynamic_cast<Ogre::D3D9HLSLProgram*>(&*self);
                        if (hlsl) {
                                hlsl->setEntryPoint(v);
                        }
                        #endif
                }
        } else if (key=="profiles") {
                if (!lua_istable(L,3))
                        my_lua_error(L,"New value should be a table");
                Ogre::CgProgram *cg = dynamic_cast<Ogre::CgProgram*>(&*self);
                if (cg) {
                        Ogre::StringVector profs;
                        for (lua_pushnil(L) ; lua_next(L,3)!=0 ; lua_pop(L,1)) {
                                if (!lua_isstring(L,-1))
                                        my_lua_error(L,"Table should only contain strings");
                                profs.push_back(lua_tostring(L,-1));
                        }
                        cg->setProfiles(profs);
                }
        } else if (key=="skeletalAnimationIncluded") {
                self->setSkeletalAnimationIncluded(check_bool(L,3));
        } else if (key=="morphAnimationIncluded") {
                self->setMorphAnimationIncluded(check_bool(L,3));
        } else if (key=="poseAnimationIncluded") {
                self->setPoseAnimationIncluded(check_bool(L,3));
        } else {
                my_lua_error(L,"Not a writeable GPUProg member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::GpuProgram,gpuprog,GPUPROG_TAG)

MT_MACRO_NEWINDEX(gpuprog);

// }}}

// vim: shiftwidth=8:tabstop=8:expandtab
