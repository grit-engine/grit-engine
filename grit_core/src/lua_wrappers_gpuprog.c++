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
        check_args(L,3);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        Ogre::Real v = luaL_checknumber(L,3);
        self->getDefaultParameters()->setNamedConstant(name,v);
        return 0;
TRY_END
}

static int gpuprog_set_constant_int (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
        Ogre::String name = luaL_checkstring(L,2);
        int v = check_t<int>(L,3);
        self->getDefaultParameters()->setNamedConstant(name,v);
        return 0;
TRY_END
}

typedef Ogre::GpuProgramParameters::AutoConstantType ACT;

static ACT act_from_str (lua_State *L, const std::string& str)
{
        if (str=="WORLD_MATRIX") return Ogre::GpuProgramParameters::ACT_WORLD_MATRIX;
        else if (str=="INVERSE_WORLD_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_WORLD_MATRIX;
        else if (str=="TRANSPOSE_WORLD_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_WORLD_MATRIX;
        else if (str=="INVERSE_TRANSPOSE_WORLD_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX;
        else if (str=="WORLD_MATRIX_ARRAY_3x4") return Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4;
        else if (str=="WORLD_MATRIX_ARRAY") return Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY;
        else if (str=="VIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_VIEW_MATRIX;
        else if (str=="INVERSE_VIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX;
        else if (str=="TRANSPOSE_VIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_VIEW_MATRIX;
        else if (str=="INVERSE_TRANSPOSE_VIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_VIEW_MATRIX;
        else if (str=="PROJECTION_MATRIX") return Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX;
        else if (str=="INVERSE_PROJECTION_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX;
        else if (str=="TRANSPOSE_PROJECTION_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_PROJECTION_MATRIX;
        else if (str=="INVERSE_TRANSPOSE_PROJECTION_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX;
        else if (str=="VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_VIEWPROJ_MATRIX;
        else if (str=="INVERSE_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_VIEWPROJ_MATRIX;
        else if (str=="TRANSPOSE_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_VIEWPROJ_MATRIX;
        else if (str=="INVERSE_TRANSPOSE_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX;
        else if (str=="WORLDVIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_WORLDVIEW_MATRIX;
        else if (str=="INVERSE_WORLDVIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_WORLDVIEW_MATRIX;
        else if (str=="TRANSPOSE_WORLDVIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEW_MATRIX;
        else if (str=="INVERSE_TRANSPOSE_WORLDVIEW_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX;
        else if (str=="WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX;
        else if (str=="INVERSE_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_WORLDVIEWPROJ_MATRIX;
        else if (str=="TRANSPOSE_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX;
        else if (str=="INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX;
        else if (str=="RENDER_TARGET_FLIPPING") return Ogre::GpuProgramParameters::ACT_RENDER_TARGET_FLIPPING;
        else if (str=="FOG_COLOUR") return Ogre::GpuProgramParameters::ACT_FOG_COLOUR;
        else if (str=="FOG_PARAMS") return Ogre::GpuProgramParameters::ACT_FOG_PARAMS;
        else if (str=="SURFACE_AMBIENT_COLOUR") return Ogre::GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR;
        else if (str=="SURFACE_DIFFUSE_COLOUR") return Ogre::GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR;
        else if (str=="SURFACE_SPECULAR_COLOUR") return Ogre::GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR;
        else if (str=="SURFACE_EMISSIVE_COLOUR") return Ogre::GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR;
        else if (str=="SURFACE_SHININESS") return Ogre::GpuProgramParameters::ACT_SURFACE_SHININESS;
        else if (str=="LIGHT_COUNT") return Ogre::GpuProgramParameters::ACT_LIGHT_COUNT;
        else if (str=="AMBIENT_LIGHT_COLOUR") return Ogre::GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR;
        else if (str=="LIGHT_DIFFUSE_COLOUR") return Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR;
        else if (str=="LIGHT_SPECULAR_COLOUR") return Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR;
        else if (str=="LIGHT_ATTENUATION") return Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION;
        else if (str=="SPOTLIGHT_PARAMS") return Ogre::GpuProgramParameters::ACT_SPOTLIGHT_PARAMS;
        else if (str=="LIGHT_POSITION") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION;
        else if (str=="LIGHT_POSITION_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE;
        else if (str=="LIGHT_POSITION_VIEW_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE;
        else if (str=="LIGHT_DIRECTION") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION;
        else if (str=="LIGHT_DIRECTION_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_OBJECT_SPACE;
        else if (str=="LIGHT_DIRECTION_VIEW_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE;
        else if (str=="LIGHT_DISTANCE_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_LIGHT_DISTANCE_OBJECT_SPACE;
        else if (str=="LIGHT_POWER_SCALE") return Ogre::GpuProgramParameters::ACT_LIGHT_POWER_SCALE;
        else if (str=="LIGHT_DIFFUSE_COLOUR_POWER_SCALED") return Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED;
        else if (str=="LIGHT_SPECULAR_COLOUR_POWER_SCALED") return Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED;
        else if (str=="LIGHT_DIFFUSE_COLOUR_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_ARRAY;
        else if (str=="LIGHT_SPECULAR_COLOUR_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_ARRAY;
        else if (str=="LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY;
        else if (str=="LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY;
        else if (str=="LIGHT_ATTENUATION_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION_ARRAY;
        else if (str=="LIGHT_POSITION_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_ARRAY;
        else if (str=="LIGHT_POSITION_OBJECT_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY;
        else if (str=="LIGHT_POSITION_VIEW_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY;
        else if (str=="LIGHT_DIRECTION_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_ARRAY;
        else if (str=="LIGHT_DIRECTION_OBJECT_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY;
        else if (str=="LIGHT_DIRECTION_VIEW_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY;
        else if (str=="LIGHT_DISTANCE_OBJECT_SPACE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY;
        else if (str=="LIGHT_POWER_SCALE_ARRAY") return Ogre::GpuProgramParameters::ACT_LIGHT_POWER_SCALE_ARRAY;
        else if (str=="SPOTLIGHT_PARAMS_ARRAY") return Ogre::GpuProgramParameters::ACT_SPOTLIGHT_PARAMS_ARRAY;
        else if (str=="DERIVED_AMBIENT_LIGHT_COLOUR") return Ogre::GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR;
        else if (str=="DERIVED_SCENE_COLOUR") return Ogre::GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR;
        else if (str=="DERIVED_LIGHT_DIFFUSE_COLOUR") return Ogre::GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR;
        else if (str=="DERIVED_LIGHT_SPECULAR_COLOUR") return Ogre::GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR;
        else if (str=="DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY") return Ogre::GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY;
        else if (str=="DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY") return Ogre::GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY;
        else if (str=="LIGHT_NUMBER") return Ogre::GpuProgramParameters::ACT_LIGHT_NUMBER;
        else if (str=="LIGHT_CASTS_SHADOWS") return Ogre::GpuProgramParameters::ACT_LIGHT_CASTS_SHADOWS;
        else if (str=="SHADOW_EXTRUSION_DISTANCE") return Ogre::GpuProgramParameters::ACT_SHADOW_EXTRUSION_DISTANCE;
        else if (str=="CAMERA_POSITION") return Ogre::GpuProgramParameters::ACT_CAMERA_POSITION;
        else if (str=="CAMERA_POSITION_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE;
        else if (str=="TEXTURE_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX;
        else if (str=="TEXTURE_VIEWPROJ_MATRIX_ARRAY") return Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY;
        else if (str=="TEXTURE_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX;
        else if (str=="TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY") return Ogre::GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY;
        else if (str=="SPOTLIGHT_VIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX;
        else if (str=="SPOTLIGHT_WORLDVIEWPROJ_MATRIX") return Ogre::GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX;
        else if (str=="CUSTOM") return Ogre::GpuProgramParameters::ACT_CUSTOM;
        else if (str=="TIME") return Ogre::GpuProgramParameters::ACT_TIME;
        else if (str=="TIME_0_X") return Ogre::GpuProgramParameters::ACT_TIME_0_X;
        else if (str=="COSTIME_0_X") return Ogre::GpuProgramParameters::ACT_COSTIME_0_X;
        else if (str=="SINTIME_0_X") return Ogre::GpuProgramParameters::ACT_SINTIME_0_X;
        else if (str=="TANTIME_0_X") return Ogre::GpuProgramParameters::ACT_TANTIME_0_X;
        else if (str=="TIME_0_X_PACKED") return Ogre::GpuProgramParameters::ACT_TIME_0_X_PACKED;
        else if (str=="TIME_0_1") return Ogre::GpuProgramParameters::ACT_TIME_0_1;
        else if (str=="COSTIME_0_1") return Ogre::GpuProgramParameters::ACT_COSTIME_0_1;
        else if (str=="SINTIME_0_1") return Ogre::GpuProgramParameters::ACT_SINTIME_0_1;
        else if (str=="TANTIME_0_1") return Ogre::GpuProgramParameters::ACT_TANTIME_0_1;
        else if (str=="TIME_0_1_PACKED") return Ogre::GpuProgramParameters::ACT_TIME_0_1_PACKED;
        else if (str=="TIME_0_2PI") return Ogre::GpuProgramParameters::ACT_TIME_0_2PI;
        else if (str=="COSTIME_0_2PI") return Ogre::GpuProgramParameters::ACT_COSTIME_0_2PI;
        else if (str=="SINTIME_0_2PI") return Ogre::GpuProgramParameters::ACT_SINTIME_0_2PI;
        else if (str=="TANTIME_0_2PI") return Ogre::GpuProgramParameters::ACT_TANTIME_0_2PI;
        else if (str=="TIME_0_2PI_PACKED") return Ogre::GpuProgramParameters::ACT_TIME_0_2PI_PACKED;
        else if (str=="FRAME_TIME") return Ogre::GpuProgramParameters::ACT_FRAME_TIME;
        else if (str=="FPS") return Ogre::GpuProgramParameters::ACT_FPS;
        else if (str=="VIEWPORT_WIDTH") return Ogre::GpuProgramParameters::ACT_VIEWPORT_WIDTH;
        else if (str=="VIEWPORT_HEIGHT") return Ogre::GpuProgramParameters::ACT_VIEWPORT_HEIGHT;
        else if (str=="INVERSE_VIEWPORT_WIDTH") return Ogre::GpuProgramParameters::ACT_INVERSE_VIEWPORT_WIDTH;
        else if (str=="INVERSE_VIEWPORT_HEIGHT") return Ogre::GpuProgramParameters::ACT_INVERSE_VIEWPORT_HEIGHT;
        else if (str=="VIEWPORT_SIZE") return Ogre::GpuProgramParameters::ACT_VIEWPORT_SIZE;
        else if (str=="VIEW_DIRECTION") return Ogre::GpuProgramParameters::ACT_VIEW_DIRECTION;
        else if (str=="VIEW_SIDE_VECTOR") return Ogre::GpuProgramParameters::ACT_VIEW_SIDE_VECTOR;
        else if (str=="VIEW_UP_VECTOR") return Ogre::GpuProgramParameters::ACT_VIEW_UP_VECTOR;
        else if (str=="FOV") return Ogre::GpuProgramParameters::ACT_FOV;
        else if (str=="NEAR_CLIP_DISTANCE") return Ogre::GpuProgramParameters::ACT_NEAR_CLIP_DISTANCE;
        else if (str=="FAR_CLIP_DISTANCE") return Ogre::GpuProgramParameters::ACT_FAR_CLIP_DISTANCE;
        else if (str=="PASS_NUMBER") return Ogre::GpuProgramParameters::ACT_PASS_NUMBER;
        else if (str=="PASS_ITERATION_NUMBER") return Ogre::GpuProgramParameters::ACT_PASS_ITERATION_NUMBER;
        else if (str=="ANIMATION_PARAMETRIC") return Ogre::GpuProgramParameters::ACT_ANIMATION_PARAMETRIC;
        else if (str=="TEXEL_OFFSETS") return Ogre::GpuProgramParameters::ACT_TEXEL_OFFSETS;
        else if (str=="SCENE_DEPTH_RANGE") return Ogre::GpuProgramParameters::ACT_SCENE_DEPTH_RANGE;
        else if (str=="SHADOW_SCENE_DEPTH_RANGE") return Ogre::GpuProgramParameters::ACT_SHADOW_SCENE_DEPTH_RANGE;
        else if (str=="SHADOW_COLOUR") return Ogre::GpuProgramParameters::ACT_SHADOW_COLOUR;
        else if (str=="TEXTURE_SIZE") return Ogre::GpuProgramParameters::ACT_TEXTURE_SIZE;
        else if (str=="INVERSE_TEXTURE_SIZE") return Ogre::GpuProgramParameters::ACT_INVERSE_TEXTURE_SIZE;
        else if (str=="PACKED_TEXTURE_SIZE") return Ogre::GpuProgramParameters::ACT_PACKED_TEXTURE_SIZE;
        else if (str=="TEXTURE_MATRIX") return Ogre::GpuProgramParameters::ACT_TEXTURE_MATRIX;
        else if (str=="LOD_CAMERA_POSITION") return Ogre::GpuProgramParameters::ACT_LOD_CAMERA_POSITION;
        else if (str=="LOD_CAMERA_POSITION_OBJECT_SPACE") return Ogre::GpuProgramParameters::ACT_LOD_CAMERA_POSITION_OBJECT_SPACE;
        else if (str=="LIGHT_CUSTOM") return Ogre::GpuProgramParameters::ACT_LIGHT_CUSTOM;

        else {
                my_lua_error(L,"Unknown auto-constant: "+str);
                return Ogre::GpuProgramParameters::ACT_WORLD_MATRIX; // never happens
        }
}

static int gpuprog_set_auto_constant_int (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==5) {
                GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
                Ogre::String name = luaL_checkstring(L,2);
                Ogre::String t = luaL_checkstring(L,3);
                Ogre::uint16 v1 = check_t<Ogre::uint16>(L,4);
                Ogre::uint16 v2 = check_t<Ogre::uint16>(L,5);
                self->getDefaultParameters()->setNamedAutoConstant(name, act_from_str(L,t), v1, v2);
        } else if (lua_gettop(L)==4) {
                GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
                Ogre::String name = luaL_checkstring(L,2);
                Ogre::String t = luaL_checkstring(L,3);
                size_t v = check_t<size_t>(L,4);
                self->getDefaultParameters()->setNamedAutoConstant(name, act_from_str(L,t), v);
        } else {
                check_args(L,3);
                GET_UD_MACRO(Ogre::HighLevelGpuProgramPtr,self,1,GPUPROG_TAG);
                Ogre::String name = luaL_checkstring(L,2);
                Ogre::String t = luaL_checkstring(L,3);
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
        Ogre::String name = luaL_checkstring(L,2);
        Ogre::String t = luaL_checkstring(L,3);
        Ogre::Real v = luaL_checknumber(L,4);
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
                #ifdef WIN32
                Ogre::D3D9HLSLProgram *hlsl = dynamic_cast<Ogre::D3D9HLSLProgram*>(&*self);
                if (hlsl) {
                        lua_pushstring(L,hlsl->getEntryPoint().c_str());
                } else
                #endif
                        lua_pushstring(L,"unknown");
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
        } else if (key=="entryPoint") {
                std::string v = luaL_checkstring(L, 3);
                #ifdef WIN32
                Ogre::D3D9HLSLProgram *hlsl = dynamic_cast<Ogre::D3D9HLSLProgram*>(&*self);
                if (hlsl) {
                        hlsl->setEntryPoint(v);
                }
                #endif
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
