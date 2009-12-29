#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>

#include "lua_wrappers_material.h"


static int mat_clone (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
        const char *name = luaL_checkstring(L,2);
        push(L,new Ogre::MaterialPtr(mat->clone(name)),MAT_TAG);
        return 1;
TRY_END
}

static int mat_load (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
        mat->load();
        return 0;
TRY_END
}

static int mat_unload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
        mat->unload();
        return 0;
TRY_END
}

static int mat_reload (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
        mat->reload();
        return 0;
TRY_END
}

static int mat_get_num_techniques (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
        lua_pushnumber(L,mat->getNumTechniques());
        return 1;
TRY_END
}

static int mat_create_technique (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
        mat->createTechnique();
        lua_pushnumber(L,mat->getNumTechniques()-1);
        return 1;
TRY_END
}

static int mat_remove_technique (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
        unsigned short n =
                check_t<unsigned short>(L,2,0,mat->getNumTechniques()-1);
        mat->removeTechnique(n);
        return 0;
TRY_END
}

static int mat_remove_all_techniques (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
        mat->removeAllTechniques();
        return 0;
TRY_END
}

static Ogre::Technique *mat_get_technique_unsafe (lua_State *L)
{
        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
        Ogre::Technique *t;
        if (lua_type(L,2)==LUA_TNUMBER) {
                unsigned short index = check_t<unsigned short>(L,2);
                if (index>=mat->getNumTechniques())
                        my_lua_error(L,"Technique out of range");
                t = mat->getTechnique(index);
        } else {
                const char *name = luaL_checkstring(L,2);
                t = mat->getTechnique(name);
        }
        if (t==NULL) my_lua_error(L,"Technique not found.");
        return t;
}

static Ogre::Technique *mat_get_technique (lua_State *L)
{
TRY_START
        Ogre::Technique *t = mat_get_technique_unsafe(L);
        // I tried the below but it's a pain when you want to just set
        // a texture name or something before the material is loaded.
        //if (!t->isSupported()) my_lua_error(L,"Technique not supported.");
        return t;
TRY_END
}

static int mat_is_technique_supported (lua_State *L)
{
TRY_START
        check_args(L,2);
        Ogre::Technique *t = mat_get_technique_unsafe(L);
        lua_pushboolean(L,t->isSupported());
        return 1;
TRY_END
}

static int mat_get_shadow_caster_material (lua_State *L)
{
TRY_START
        check_args(L,2);
        Ogre::Technique *t = mat_get_technique_unsafe(L);
        push(L,new Ogre::MaterialPtr(t->getShadowCasterMaterial()),MAT_TAG);
        return 1;
TRY_END
}

static int mat_set_shadow_caster_material (lua_State *L)
{
TRY_START
        check_args(L,2+1);
        Ogre::Technique *t = mat_get_technique_unsafe(L);
        GET_UD_MACRO(Ogre::MaterialPtr,mat,3,MAT_TAG);
        t->setShadowCasterMaterial(mat);
        return 0;
TRY_END
}

static int mat_get_num_passes (lua_State *L)
{
TRY_START
        check_args(L,2);
        Ogre::Technique *t = mat_get_technique(L);
        lua_pushnumber(L,t->getNumPasses());
        return 1;
TRY_END
}

static int mat_create_pass (lua_State *L)
{
TRY_START
        check_args(L,2);
        Ogre::Technique *t = mat_get_technique(L);
        t->createPass();
        lua_pushnumber(L,t->getNumPasses()-1);
        return 1;
TRY_END
}

static int mat_remove_pass (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Technique *t = mat_get_technique(L);
        unsigned short n = check_t<unsigned short>(L,3,0,t->getNumPasses()-1);
        t->removePass(n);
        return 0;
TRY_END
}

static int mat_remove_all_passes (lua_State *L)
{
TRY_START
        check_args(L,2);
        Ogre::Technique *t = mat_get_technique(L);
        t->removeAllPasses();
        return 0;
TRY_END
}

static Ogre::Pass *mat_get_pass (lua_State *L)
{
TRY_START
        Ogre::Technique *t = mat_get_technique(L);
        Ogre::Pass *p;
        if (lua_type(L,3)==LUA_TNUMBER) {
                unsigned short index = check_t<unsigned short>(L,3);
                if (index>=t->getNumPasses())
                        my_lua_error(L,"Pass out of range");
                p = t->getPass(index);
        } else {
                const char *name = luaL_checkstring(L,3);
                p = t->getPass(name);
        }
        if (p==NULL) my_lua_error(L,"Pass not found.");
        return p;
TRY_END
}

static int mat_create_texture_unit_state (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        p->createTextureUnitState();
        lua_pushnumber(L,p->getNumTextureUnitStates()-1);
        return 1;
TRY_END
}

static int mat_remove_texture_unit_state (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::Pass *p = mat_get_pass(L);
        unsigned short n =
              check_t<unsigned short>(L,4,0,p->getNumTextureUnitStates()-1);
        p->removeTextureUnitState(n);
        return 0;
TRY_END
}

static int mat_get_num_texture_unit_states (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushnumber(L,p->getNumTextureUnitStates());
        return 1;
TRY_END
}




static int mat_has_vertex_program (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->hasVertexProgram());
        return 1;
TRY_END
}

static int mat_has_fragment_program (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->hasFragmentProgram());
        return 1;
TRY_END
}

static int mat_get_vertex_program_name (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushstring(L,p->getVertexProgramName().c_str());
        return 1;
TRY_END
}

static int mat_get_fragment_program_name (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushstring(L,p->getFragmentProgramName().c_str());
        return 1;
TRY_END
}

static int mat_set_vertex_program (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        p->setVertexProgram(luaL_checkstring(L,4));
        return 0;
TRY_END
}

static int mat_set_fragment_program (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        p->setFragmentProgram(luaL_checkstring(L,4));
        return 0;
TRY_END
}

static int mat_get_ambient_vertex (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getVertexColourTracking() & Ogre::TVC_AMBIENT);
        return 1;
TRY_END
}
static int mat_get_diffuse_vertex (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getVertexColourTracking() & Ogre::TVC_DIFFUSE);
        return 1;
TRY_END
}
static int mat_get_specular_vertex (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getVertexColourTracking() & Ogre::TVC_SPECULAR);
        return 1;
TRY_END
}

static int mat_get_emissive_vertex (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getVertexColourTracking() & Ogre::TVC_EMISSIVE);
        return 1;
TRY_END
}

static int mat_get_emissive (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        const Ogre::ColourValue& cv = p->getSelfIllumination();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        return 3;
TRY_END
}

static int mat_get_ambient (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        const Ogre::ColourValue& cv = p->getAmbient();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        return 3;
TRY_END
}

static int mat_get_diffuse (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        const Ogre::ColourValue& cv = p->getDiffuse();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        lua_pushnumber(L,cv.a);
        return 4;
TRY_END
}

static int mat_get_specular (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        const Ogre::ColourValue& cv = p->getSpecular();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        lua_pushnumber(L,cv.a);
        lua_pushnumber(L,p->getShininess());
        return 5;
TRY_END
}

static int mat_set_lighting (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        p->setLightingEnabled(b);
        return 0;
TRY_END
}

static int mat_get_lighting (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getLightingEnabled());
        return 1;
TRY_END
}

static const char *
shading_mode_to_string (lua_State *L, Ogre::ShadeOptions sm)
{
        switch (sm) {
        case Ogre::SO_FLAT: return "FLAG";
        case Ogre::SO_GOURAUD: return "GOURAUD";
        case Ogre::SO_PHONG: return "PHONG";
        default:
                my_lua_error(L,"Unrecognised shading mode.");
                return "never happens";
        }
}
static Ogre::ShadeOptions
shading_mode_from_string (lua_State *L, const std::string& t)
{
        if (t=="FLAT") {
                return Ogre::SO_FLAT;
        } else if (t=="GOURAUD") {
                return Ogre::SO_GOURAUD;
        } else if (t=="PHONG") {
                return Ogre::SO_PHONG;
        } else {
                my_lua_error(L,"Unrecognised shading mode: "+t);
                return Ogre::SO_FLAT; //never happens
        }
}

static int mat_set_shading_mode (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        p->setShadingMode(shading_mode_from_string(L,luaL_checkstring(L,4)));
        return 0;
TRY_END
}

static int mat_get_shading_mode (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushstring(L,shading_mode_to_string(L,p->getShadingMode()));
        return 1;
TRY_END
}


const char * polygon_mode_to_string (lua_State *L, Ogre::PolygonMode pm)
{
        switch (pm) {
        case Ogre::PM_POINTS: return "POINTS";
        case Ogre::PM_WIREFRAME: return "WIREFRAME";
        case Ogre::PM_SOLID: return "SOLID";
        default:
                my_lua_error(L,"Unrecognised polygon mode.");
                return "never happens";
        }
}
Ogre::PolygonMode polygon_mode_from_string (lua_State *L, const std::string& t)
{
        if (t=="POINTS") {
                return Ogre::PM_POINTS;
        } else if (t=="WIREFRAME") {
                return Ogre::PM_WIREFRAME;
        } else if (t=="SOLID") {
                return Ogre::PM_SOLID;
        } else {
                my_lua_error(L,"Unrecognised polygon mode: "+t);
                return Ogre::PM_POINTS; //never happens
        }
}

static int mat_set_polygon_mode (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        p->setPolygonMode(polygon_mode_from_string(L,luaL_checkstring(L,4)));
        return 0;
TRY_END
}

static int mat_get_polygon_mode (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushstring(L,polygon_mode_to_string(L,p->getPolygonMode()));
        return 1;
TRY_END
}


static int mat_set_emissive_vertex (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        p->setVertexColourTracking ((p->getVertexColourTracking() &
                                        ~Ogre::TVC_EMISSIVE)
                                    | (b?Ogre::TVC_EMISSIVE:0));
        return 0;
TRY_END
}

static int mat_set_ambient_vertex (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        p->setVertexColourTracking ((p->getVertexColourTracking() &
                                        ~Ogre::TVC_AMBIENT)
                                    | (b?Ogre::TVC_AMBIENT:0));
        return 0;
TRY_END
}

static int mat_set_diffuse_vertex (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        p->setVertexColourTracking ((p->getVertexColourTracking() &
                                        ~Ogre::TVC_DIFFUSE)
                                    | (b?Ogre::TVC_DIFFUSE:0));
        return 0;
TRY_END
}

static int mat_set_specular_vertex (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        p->setVertexColourTracking ((p->getVertexColourTracking() &
                                        ~Ogre::TVC_SPECULAR)
                                    | (b?Ogre::TVC_SPECULAR:0));
        return 0;
TRY_END
}


static int mat_set_emissive (lua_State *L)
{
TRY_START
        check_args(L,3+3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_Number r = luaL_checknumber(L,4);
        lua_Number g = luaL_checknumber(L,5);
        lua_Number b = luaL_checknumber(L,6);
        p->setSelfIllumination(r,g,b);
        return 0;
TRY_END
}

static int mat_set_ambient (lua_State *L)
{
TRY_START
        check_args(L,3+3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_Number r = luaL_checknumber(L,4);
        lua_Number g = luaL_checknumber(L,5);
        lua_Number b = luaL_checknumber(L,6);
        p->setAmbient(r,g,b);
        return 0;
TRY_END
}

static int mat_set_diffuse (lua_State *L)
{
TRY_START
        check_args(L,3+4);
        Ogre::Pass *p = mat_get_pass(L);
        lua_Number r = luaL_checknumber(L,4);
        lua_Number g = luaL_checknumber(L,5);
        lua_Number b = luaL_checknumber(L,6);
        lua_Number a = luaL_checknumber(L,7);
        p->setDiffuse(r,g,b,a);
        return 0;
TRY_END
}

static int mat_set_specular (lua_State *L)
{
TRY_START
        check_args(L,3+5);
        Ogre::Pass *p = mat_get_pass(L);
        lua_Number r = luaL_checknumber(L,4);
        lua_Number g = luaL_checknumber(L,5);
        lua_Number b = luaL_checknumber(L,6);
        lua_Number a = luaL_checknumber(L,7);
        lua_Number s = luaL_checknumber(L,8);
        p->setSpecular(r,g,b,a);
        p->setShininess(s);
        return 0;
TRY_END
}

static Ogre::TextureUnitState *mat_get_texture_unit_state(lua_State *L)
{
TRY_START
        Ogre::Pass *p = mat_get_pass(L);
        Ogre::TextureUnitState *tex;
        if (lua_type(L,4)==LUA_TNUMBER) {
                unsigned short index = check_t<unsigned short>(L,4);
                if (index>=p->getNumTextureUnitStates())
                        my_lua_error(L,"Texture unit out of range");
                tex = p->getTextureUnitState(index);
        } else {
                const char *name = luaL_checkstring(L,4);
                tex = p->getTextureUnitState(name);
        }
        if (tex==NULL) my_lua_error(L,"Texture unit not found.");
        return tex;
TRY_END
}

static const char *
envmap_type_to_string (lua_State *L, Ogre::TextureUnitState::EnvMapType t)
{
        switch (t) {
        case Ogre::TextureUnitState::ENV_PLANAR: return "PLANAR";
        case Ogre::TextureUnitState::ENV_CURVED: return "CURVED";
        case Ogre::TextureUnitState::ENV_REFLECTION: return "REFLECTION";
        case Ogre::TextureUnitState::ENV_NORMAL: return "NORMAL";
        default:
                my_lua_error(L,"Unrecognised environment map type.");
                return "never happens";
        }
}
static Ogre::TextureUnitState::EnvMapType
envmap_type_from_string (lua_State *L, const std::string& t)
{
        if (t=="PLANAR") {
                return Ogre::TextureUnitState::ENV_PLANAR;
        } else if (t=="CURVED") {
                return Ogre::TextureUnitState::ENV_CURVED;
        } else if (t=="REFLECTION") {
                return Ogre::TextureUnitState::ENV_REFLECTION;
        } else if (t=="NORMAL") {
                return Ogre::TextureUnitState::ENV_NORMAL;
        } else {
                my_lua_error(L,"Unrecognised environment map type: "+t);
                return Ogre::TextureUnitState::ENV_PLANAR;
        }
}

static int mat_get_env_map (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        Ogre::TextureUnitState::EffectMap map = tex->getEffects();
        Ogre::TextureUnitState::EffectMap::iterator i =
                map.find(Ogre::TextureUnitState::ET_ENVIRONMENT_MAP);
        if (i==map.end()) {
                lua_pushboolean(L,false);
                return 1;
        }
        lua_pushboolean(L,true);
        Ogre::TextureUnitState::EnvMapType t =
                (Ogre::TextureUnitState::EnvMapType)i->second.subtype;
        lua_pushstring(L,envmap_type_to_string(L,t));
        return 2;
TRY_END
}
static int mat_set_env_map (lua_State *L)
{
TRY_START
        check_args(L,4+2);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        bool b = check_bool(L,5);
        std::string str = luaL_checkstring(L,6);
        tex->setEnvironmentMap(b, envmap_type_from_string(L,str));
        return 0;
TRY_END
}

static int mat_get_mipmap_bias (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);

        lua_pushnumber(L,tex->getTextureMipmapBias());
        return 1;
TRY_END
}
static int mat_set_mipmap_bias (lua_State *L)
{
TRY_START
        check_args(L,4+1);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        lua_Number n = luaL_checknumber(L,5);
        tex->setTextureMipmapBias(n);
        return 0;
TRY_END
}

static const char *
filter_options_to_string (lua_State *L, Ogre::FilterOptions fo)
{
        switch ( fo) {
        case Ogre::FO_NONE: return "NONE";
        case Ogre::FO_POINT: return "POINT";
        case Ogre::FO_LINEAR: return "LINEAR";
        case Ogre::FO_ANISOTROPIC: return "ANISOTROPIC";
        default:
                my_lua_error(L,"Unrecognised filter option.");
                return "never happens";
        }
}
static Ogre::FilterOptions
filter_options_from_string (lua_State *L, const std::string& t)
{
        if (t=="NONE") {
                return Ogre::FO_NONE;
        } else if (t=="POINT") {
                return Ogre::FO_POINT;
        } else if (t=="LINEAR") {
                return Ogre::FO_LINEAR;
        } else if (t=="ANISOTROPIC") {
                return Ogre::FO_ANISOTROPIC;
        } else {
                my_lua_error(L,"Unrecognised filter option: "+t);
                return Ogre::FO_NONE; //never happens
        }
}


static int mat_set_texture_anisotropy (lua_State *L)
{
TRY_START
        check_args(L,4+1);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        tex->setTextureAnisotropy(check_t<unsigned short>(L,5));
        return 0;
TRY_END
}

static int mat_get_texture_anisotropy (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        lua_pushnumber(L,tex->getTextureAnisotropy());
        return 1;
TRY_END
}


static int mat_set_texture_filtering (lua_State *L)
{
TRY_START
        check_args(L,4+3);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        tex->setTextureFiltering(
                filter_options_from_string(L,luaL_checkstring(L,5)),
                filter_options_from_string(L,luaL_checkstring(L,6)),
                filter_options_from_string(L,luaL_checkstring(L,7)));
        return 0;
TRY_END
}

static int mat_get_texture_filtering (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        lua_pushstring(L,filter_options_to_string
                                (L,tex->getTextureFiltering(Ogre::FT_MIN)));
        lua_pushstring(L,filter_options_to_string
                                (L,tex->getTextureFiltering(Ogre::FT_MAG)));
        lua_pushstring(L,filter_options_to_string
                                (L,tex->getTextureFiltering(Ogre::FT_MIP)));
        return 3;
TRY_END
}


static int mat_get_texture_name (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        lua_pushstring(L,tex->getTextureName().c_str());
        return 1;
TRY_END
}
static int mat_set_texture_name (lua_State *L)
{
TRY_START
        check_args(L,4+1);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        std::string str = luaL_checkstring(L,5);
        tex->setTextureName(str);
        return 0;
TRY_END
}
static int mat_set_cubic_texture_name (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==6) {
                check_args(L,4+2);
                Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
                std::string str = luaL_checkstring(L,5);
                bool b = check_bool(L,6);
                tex->setCubicTextureName(str,b);
        } else {
                check_args(L,4+7);
                Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
                std::string str1 = luaL_checkstring(L,5);
                std::string str2 = luaL_checkstring(L,6);
                std::string str3 = luaL_checkstring(L,7);
                std::string str4 = luaL_checkstring(L,8);
                std::string str5 = luaL_checkstring(L,9);
                std::string str6 = luaL_checkstring(L,10);
                bool b = check_bool(L,11);
                std::string arr[] = {str1,str2,str3,str4,str5,str6};
                tex->setCubicTextureName(arr,b);
        }
        return 0;
TRY_END
}

static int mat_get_is_shadow (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        lua_pushboolean(L,tex->getContentType()==Ogre::TextureUnitState::CONTENT_SHADOW);
        return 1;
TRY_END
}
static int mat_set_is_shadow (lua_State *L)
{
TRY_START
        check_args(L,4+1);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        bool b = check_bool(L,5);
        tex->setContentType(b ?  Ogre::TextureUnitState::CONTENT_SHADOW :
                                 Ogre::TextureUnitState::CONTENT_NAMED);
        return 0;
TRY_END
}

static Ogre::SceneBlendFactor factor_from_string (lua_State *L,
                                                  const std::string& factor)
{
        Ogre::SceneBlendFactor r;
        if (factor=="ONE") {
                r = Ogre::SBF_ONE;
        } else if (factor=="ZERO") {
                r = Ogre::SBF_ZERO;
        } else if (factor=="DEST_COLOUR") {
                r = Ogre::SBF_DEST_COLOUR;
        } else if (factor=="SRC_COLOUR") {
                r = Ogre::SBF_SOURCE_COLOUR;
        } else if (factor=="ONE_MINUS_DEST_COLOUR") {
                r = Ogre::SBF_ONE_MINUS_DEST_COLOUR;
        } else if (factor=="ONE_MINUS_SRC_COLOUR") {
                r = Ogre::SBF_ONE_MINUS_SOURCE_COLOUR;
        } else if (factor=="DEST_ALPHA") {
                r = Ogre::SBF_DEST_ALPHA;
        } else if (factor=="SRC_ALPHA") {
                r = Ogre::SBF_SOURCE_ALPHA;
        } else if (factor=="ONE_MINUS_DEST_ALPHA") {
                r = Ogre::SBF_ONE_MINUS_DEST_ALPHA;
        } else if (factor=="ONE_MINUS_SRC_ALPHA") {
                r = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
        } else {
                std::string msg = "Unknown scene blend factor: ";
                msg += factor;
                my_lua_error(L,msg);
                r = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA; // never happens
        }
        return r;
}

static void push_string_from_factor (lua_State *L,Ogre::SceneBlendFactor factor)
{
        switch (factor) {
                case Ogre::SBF_ONE:
                lua_pushstring(L,"ONE");
                break;
                case Ogre::SBF_ZERO:
                lua_pushstring(L,"ZERO");
                break;
                case Ogre::SBF_DEST_COLOUR:
                lua_pushstring(L,"DEST_COLOUR");
                break;
                case Ogre::SBF_SOURCE_COLOUR:
                lua_pushstring(L,"SRC_COLOUR");
                break;
                case Ogre::SBF_ONE_MINUS_DEST_COLOUR:
                lua_pushstring(L,"ONE_MINUS_DEST_COLOUR");
                break;
                case Ogre::SBF_ONE_MINUS_SOURCE_COLOUR:
                lua_pushstring(L,"ONE_MINUS_SRC_COLOUR");
                break;
                case Ogre::SBF_DEST_ALPHA:
                lua_pushstring(L,"DEST_ALPHA");
                break;
                case Ogre::SBF_SOURCE_ALPHA:
                lua_pushstring(L,"SRC_ALPHA");
                break;
                case Ogre::SBF_ONE_MINUS_DEST_ALPHA:
                lua_pushstring(L,"ONE_MINUS_DEST_ALPHA");
                break;
                case Ogre::SBF_ONE_MINUS_SOURCE_ALPHA:
                lua_pushstring(L,"ONE_MINUS_SRC_ALPHA");
                break;
                default:
                my_lua_error(L,"Unknown scene blend factor");
        }
}

static int mat_get_scene_blending (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        Ogre::SceneBlendFactor f1 = p->getSourceBlendFactor();
        Ogre::SceneBlendFactor f2 = p->getDestBlendFactor();
        push_string_from_factor(L,f1);
        push_string_from_factor(L,f2);
        return 2;
TRY_END
}
static int mat_set_scene_blending (lua_State *L)
{
TRY_START
        check_args(L,3+2);
        Ogre::Pass *p = mat_get_pass(L);
        const char *f1 = luaL_checkstring(L,4);
        const char *f2 = luaL_checkstring(L,5);
        p->setSceneBlending(factor_from_string(L,f1), factor_from_string(L,f2));
        return 0;
TRY_END
}

static const char *compare_func_to_string (lua_State *L,
                                           Ogre::CompareFunction cf)
{
        switch (cf) {
                case Ogre::CMPF_ALWAYS_FAIL: return "FALSE";
                case Ogre::CMPF_ALWAYS_PASS: return "TRUE";
                case Ogre::CMPF_LESS: return "<";
                case Ogre::CMPF_LESS_EQUAL: return "<=";
                case Ogre::CMPF_EQUAL: return "==";
                case Ogre::CMPF_NOT_EQUAL: return "!=";
                case Ogre::CMPF_GREATER_EQUAL: return ">=";
                case Ogre::CMPF_GREATER: return ">";
                default:
                my_lua_error(L,"Unknown compare function");
                return "never happens";
        }
}

static Ogre::CompareFunction compare_func_from_string (lua_State *L,
                                                       const std::string &str)
{
        if (str=="FALSE") return Ogre::CMPF_ALWAYS_FAIL;
        if (str=="TRUE") return Ogre::CMPF_ALWAYS_PASS;
        if (str=="<") return Ogre::CMPF_LESS;
        if (str=="<=") return Ogre::CMPF_LESS_EQUAL;
        if (str=="==") return Ogre::CMPF_EQUAL;
        if (str=="!=") return Ogre::CMPF_NOT_EQUAL;
        if (str==">=") return Ogre::CMPF_GREATER_EQUAL;
        if (str==">") return Ogre::CMPF_GREATER;
        my_lua_error(L,"Unknown compare function: "+str);
        return Ogre::CMPF_ALWAYS_FAIL;
}



static int mat_get_depth_bias (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushnumber(L,p->getDepthBiasConstant());
        lua_pushnumber(L,p->getDepthBiasSlopeScale());
        return 2;
TRY_END
}
static int mat_set_depth_bias (lua_State *L)
{
TRY_START
        check_args(L,3+2);
        Ogre::Pass *p = mat_get_pass(L);
        p->setDepthBias(luaL_checknumber(L,4),luaL_checknumber(L,5));
        return 0;
TRY_END
}


static int mat_get_alpha_rejection (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushstring(L,compare_func_to_string(L,p->getAlphaRejectFunction()));
        lua_pushnumber(L,p->getAlphaRejectValue());
        return 2;
TRY_END
}
static int mat_set_alpha_rejection (lua_State *L)
{
TRY_START
        check_args(L,3+2);
        Ogre::Pass *p = mat_get_pass(L);
        p->setAlphaRejectFunction
                (compare_func_from_string(L,luaL_checkstring(L,4)));
        p->setAlphaRejectValue(check_t<unsigned char>(L,5));
        return 0;
TRY_END
}


static int mat_get_alpha_to_coverage (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->isAlphaToCoverageEnabled());
        return 1;
TRY_END
}
static int mat_set_alpha_to_coverage (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        p->setAlphaToCoverageEnabled(b);
        return 0;
TRY_END
}


static int mat_get_depth_function (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushstring(L,compare_func_to_string(L,p->getDepthFunction()));
        return 1;
TRY_END
}
static int mat_set_depth_function (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        p->setDepthFunction(compare_func_from_string(L,luaL_checkstring(L,4)));
        return 0;
TRY_END
}


static int mat_get_depth_check_enabled (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getDepthCheckEnabled());
        return 1;
TRY_END
}
static int mat_set_depth_check_enabled (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        p->setDepthCheckEnabled(b);
        return 0;
TRY_END
}


static int mat_get_depth_write_enabled (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getDepthWriteEnabled());
        return 1;
TRY_END
}
static int mat_set_depth_write_enabled (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        p->setDepthWriteEnabled(b);
        return 0;
TRY_END
}


static int mat_get_cull (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        Ogre::CullingMode m = p->getCullingMode();
        if (m==Ogre::CULL_CLOCKWISE) {
                lua_pushboolean(L,true);
        } else if (m==Ogre::CULL_NONE) {
                lua_pushboolean(L,false);
        } else {
                my_lua_error(L,"culling mode not clockwise or none");
        }
        return 1;
TRY_END
}
static int mat_set_cull (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        p->setCullingMode(b?Ogre::CULL_CLOCKWISE:Ogre::CULL_NONE);
        return 0;
TRY_END
}


static int mat_get_manual_cull (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        Ogre::ManualCullingMode m = p->getManualCullingMode();
        if (m==Ogre::MANUAL_CULL_BACK) {
                lua_pushboolean(L,true);
        } else if (m==Ogre::MANUAL_CULL_NONE) {
                lua_pushboolean(L,false);
        } else {
                my_lua_error(L,"culling mode not clockwise or none");
        }
        return 1;
TRY_END
}
static int mat_set_manual_cull (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool b = check_bool(L,4);
        if (b) {
                p->setManualCullingMode(Ogre::MANUAL_CULL_BACK);
        } else {
                p->setManualCullingMode(Ogre::MANUAL_CULL_NONE);
        }
        return 0;
TRY_END
}

const char* fog_mode_to_string (lua_State *L, Ogre::FogMode fm)
{
        switch (fm) {
                case Ogre::FOG_NONE: return "NONE";
                case Ogre::FOG_EXP: return "EXP";
                case Ogre::FOG_EXP2: return "EXP2";
                case Ogre::FOG_LINEAR: return "LINEAR";
                default: my_lua_error(L,"Unknown fog mode");
        }
        return "never happens";
}

Ogre::FogMode fog_mode_from_string (lua_State *L, const std::string& fm)
{
        if (fm == "NONE") {
                return Ogre::FOG_NONE;
        } else if (fm == "EXP") {
                return Ogre::FOG_EXP;
        } else if (fm == "EXP2") {
                return Ogre::FOG_EXP2;
        } else if (fm == "LINEAR") {
                return Ogre::FOG_LINEAR;
        } else {
                my_lua_error(L,"Unknown fog mode: "+fm);
                return Ogre::FOG_NONE;
        }
}

static int mat_get_fog (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getFogOverride());
        lua_pushstring(L,fog_mode_to_string(L,p->getFogMode()));
        const Ogre::ColourValue &cv = p->getFogColour();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        lua_pushnumber(L,p->getFogDensity());
        lua_pushnumber(L,p->getFogStart());
        lua_pushnumber(L,p->getFogEnd());
        return 8;
TRY_END
}

static int mat_set_fog (lua_State *L)
{
TRY_START
        check_args(L,3+8);
        Ogre::Pass *p = mat_get_pass(L);
        bool override = check_bool(L,4);
        Ogre::FogMode fm = fog_mode_from_string(L,luaL_checkstring(L,5));
        Ogre::Real r = luaL_checknumber(L,6);
        Ogre::Real g = luaL_checknumber(L,7);
        Ogre::Real b = luaL_checknumber(L,8);
        Ogre::Real density = luaL_checknumber(L,9);
        Ogre::Real start = luaL_checknumber(L,10);
        Ogre::Real end = luaL_checknumber(L,11);
        p->setFog(override,fm,Ogre::ColourValue(r,g,b),density,start,end);
        return 0;
TRY_END
}

static int mat_get_transparent_sorting (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getTransparentSortingEnabled());
        return 1;
TRY_END
}

static int mat_set_transparent_sorting (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool v = check_bool(L,4);
        p->setTransparentSortingEnabled(v);
        return 0;
TRY_END
}

static int mat_get_transparent_sorting_forced (lua_State *L)
{
TRY_START
        check_args(L,3);
        Ogre::Pass *p = mat_get_pass(L);
        lua_pushboolean(L,p->getTransparentSortingForced());
        return 1;
TRY_END
}

static int mat_set_transparent_sorting_forced (lua_State *L)
{
TRY_START
        check_args(L,3+1);
        Ogre::Pass *p = mat_get_pass(L);
        bool v = check_bool(L,4);
        p->setTransparentSortingForced(v);
        return 0;
TRY_END
}

static int mat_set_fragment_constant_float (lua_State *L)
{
TRY_START
        check_args(L,3+2);
        Ogre::Pass *p = mat_get_pass(L);
        if (!p->hasFragmentProgram())
                my_lua_error(L,"This pass does not have a fragment program.");
        Ogre::String name = luaL_checkstring(L,4);
        Ogre::Real r = luaL_checknumber(L,5);
        p->getFragmentProgramParameters()->setNamedConstant(name,r);
        return 0;
TRY_END
}

static int mat_set_vertex_constant_float (lua_State *L)
{
TRY_START
        check_args(L,3+2);
        Ogre::Pass *p = mat_get_pass(L);
        if (!p->hasVertexProgram())
                my_lua_error(L,"This pass does not have a vertex program.");
        Ogre::String name = luaL_checkstring(L,4);
        Ogre::Real r = luaL_checknumber(L,5);
        p->getVertexProgramParameters()->setNamedConstant(name,r);
        return 0;
TRY_END
}



static Ogre::LayerBlendOperationEx lbo_from_string (lua_State *L,
                                                    const std::string& blend_op)
{
        if (blend_op=="SOURCE1") {
                return Ogre::LBX_SOURCE1;
        } else if (blend_op=="SOURCE2") {
                return Ogre::LBX_SOURCE2;
        } else if (blend_op=="MODULATE") {
                return Ogre::LBX_MODULATE;
        } else if (blend_op=="MODULATE_X2") {
                return Ogre::LBX_MODULATE_X2;
        } else if (blend_op=="MODULATE_X4") {
                return Ogre::LBX_MODULATE_X4;
        } else if (blend_op=="ADD") {
                return Ogre::LBX_ADD;
        } else if (blend_op=="ADD_SIGNED") {
                return Ogre::LBX_ADD_SIGNED;
        } else if (blend_op=="ADD_SMOOTH") {
                return Ogre::LBX_ADD_SMOOTH;
        } else if (blend_op=="SUBTRACT") {
                return Ogre::LBX_SUBTRACT;
        } else if (blend_op=="BLEND_DIFFUSE_ALPHA") {
                return Ogre::LBX_BLEND_DIFFUSE_ALPHA;
        } else if (blend_op=="BLEND_TEXTURE_ALPHA") {
                return Ogre::LBX_BLEND_TEXTURE_ALPHA;
        } else if (blend_op=="BLEND_CURRENT_ALPHA") {
                return Ogre::LBX_BLEND_CURRENT_ALPHA;
        } else if (blend_op=="BLEND_MANUAL") {
                return Ogre::LBX_BLEND_MANUAL;
        } else if (blend_op=="DOTPRODUCT") {
                return Ogre::LBX_DOTPRODUCT;
        } else if (blend_op=="BLEND_DIFFUSE_COLOUR") {
                return Ogre::LBX_BLEND_DIFFUSE_COLOUR;
        } else {
                my_lua_error(L,"Unknown layer blend operation: "+blend_op);
                return Ogre::LBX_BLEND_DIFFUSE_COLOUR; // never happens
        }
}

static void push_string_from_lbo (lua_State *L,
                                  Ogre::LayerBlendOperationEx blend_op)
{
        switch (blend_op) {
                case Ogre::LBX_SOURCE1:
                lua_pushstring(L,"SOURCE1");
                break;
                case Ogre::LBX_SOURCE2:
                lua_pushstring(L,"SOURCE2");
                break;
                case Ogre::LBX_MODULATE:
                lua_pushstring(L,"MODULATE");
                break;
                case Ogre::LBX_MODULATE_X2:
                lua_pushstring(L,"MODULATE_X2");
                break;
                case Ogre::LBX_MODULATE_X4:
                lua_pushstring(L,"MODULATE_X4");
                break;
                case Ogre::LBX_ADD:
                lua_pushstring(L,"ADD");
                break;
                case Ogre::LBX_ADD_SIGNED:
                lua_pushstring(L,"ADD_SIGNED");
                break;
                case Ogre::LBX_ADD_SMOOTH:
                lua_pushstring(L,"ADD_SMOOTH");
                break;
                case Ogre::LBX_SUBTRACT:
                lua_pushstring(L,"SUBTRACT");
                break;
                case Ogre::LBX_BLEND_DIFFUSE_ALPHA:
                lua_pushstring(L,"BLEND_DIFFUSE_ALPHA");
                break;
                case Ogre::LBX_BLEND_TEXTURE_ALPHA:
                lua_pushstring(L,"BLEND_TEXTURE_ALPHA");
                break;
                case Ogre::LBX_BLEND_CURRENT_ALPHA:
                lua_pushstring(L,"BLEND_CURRENT_ALPHA");
                break;
                case Ogre::LBX_BLEND_MANUAL:
                lua_pushstring(L,"BLEND_MANUAL");
                break;
                case Ogre::LBX_DOTPRODUCT:
                lua_pushstring(L,"BLEND_DOTPRODUCT");
                break;
                case Ogre::LBX_BLEND_DIFFUSE_COLOUR:
                lua_pushstring(L,"BLEND_DIFFUSE_COLOUR");
                break;
                default:
                my_lua_error(L,"Unknown layer blend operation");
        }

}

static Ogre::LayerBlendSource lbs_from_string (lua_State *L,
                                               const std::string& lbs)
{
        Ogre::LayerBlendSource r;
        if (lbs=="CURRENT") {
                r = Ogre::LBS_CURRENT;
        } else if (lbs=="TEXTURE") {
                r = Ogre::LBS_TEXTURE;
        } else if (lbs=="DIFFUSE") {
                r = Ogre::LBS_DIFFUSE;
        } else if (lbs=="SPECULAR") {
                r = Ogre::LBS_SPECULAR;
        } else if (lbs=="MANUAL") {
                r = Ogre::LBS_MANUAL;
        } else {
                my_lua_error(L,"Unknown layer blend source: "+lbs);
                r = Ogre::LBS_MANUAL; // never happens
        }
        return r;
}

static void push_string_from_lbs (lua_State *L,Ogre::LayerBlendSource lbs)
{
        switch (lbs) {
                case Ogre::LBS_CURRENT:
                lua_pushstring(L,"CURRENT");
                break;
                case Ogre::LBS_TEXTURE:
                lua_pushstring(L,"TEXTURE");
                break;
                case Ogre::LBS_DIFFUSE:
                lua_pushstring(L,"DIFFUSE");
                break;
                case Ogre::LBS_SPECULAR:
                lua_pushstring(L,"SPECULAR");
                break;
                case Ogre::LBS_MANUAL:
                lua_pushstring(L,"MANUAL");
                break;
                default:
                my_lua_error(L,"Unknown layer blend source");
        }

}

static int mat_get_colour_operation (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        Ogre::LayerBlendModeEx bm = tex->getColourBlendMode();
        push_string_from_lbo(L,bm.operation);
        push_string_from_lbs(L,bm.source1);
        push_string_from_lbs(L,bm.source2);
        lua_pushnumber(L,bm.factor);
        Ogre::ColourValue rgb = bm.colourArg1;
        lua_pushnumber(L,rgb.r);
        lua_pushnumber(L,rgb.g);
        lua_pushnumber(L,rgb.b);
        rgb = bm.colourArg2;
        lua_pushnumber(L,rgb.r);
        lua_pushnumber(L,rgb.g);
        lua_pushnumber(L,rgb.b);
        return 10;
TRY_END
}
static int mat_set_colour_operation (lua_State *L)
{
TRY_START
        check_args(L,4+10);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        Ogre::LayerBlendOperationEx op=lbo_from_string(L,luaL_checkstring(L,5));
        Ogre::LayerBlendSource s1 = lbs_from_string(L,luaL_checkstring(L,6));
        Ogre::LayerBlendSource s2 = lbs_from_string(L,luaL_checkstring(L,7));
        Ogre::ColourValue c1;
        c1.r = luaL_checknumber(L,9);
        c1.g = luaL_checknumber(L,10);
        c1.b = luaL_checknumber(L,11);
        Ogre::ColourValue c2;
        c2.r = luaL_checknumber(L,12);
        c2.g = luaL_checknumber(L,13);
        c2.b = luaL_checknumber(L,14);
        tex->setColourOperationEx(op,s1,s2,c1,c2,luaL_checknumber(L,8));
        return 0;
TRY_END
}


static int mat_get_alpha_operation (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        Ogre::LayerBlendModeEx bm = tex->getAlphaBlendMode();
        push_string_from_lbo(L,bm.operation);
        push_string_from_lbs(L,bm.source1);
        push_string_from_lbs(L,bm.source2);
        lua_pushnumber(L,bm.factor);
        lua_pushnumber(L,bm.alphaArg1);
        lua_pushnumber(L,bm.alphaArg2);
        return 6;
TRY_END
}
static int mat_set_alpha_operation (lua_State *L)
{
TRY_START
        check_args(L,4+6);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        Ogre::LayerBlendOperationEx op=lbo_from_string(L,luaL_checkstring(L,5));
        Ogre::LayerBlendSource s1 = lbs_from_string(L,luaL_checkstring(L,6));
        Ogre::LayerBlendSource s2 = lbs_from_string(L,luaL_checkstring(L,7));
        Ogre::Real a1 = luaL_checknumber(L,9);
        Ogre::Real a2 = luaL_checknumber(L,10);
        tex->setAlphaOperation(op,s1,s2,a1,a2,luaL_checknumber(L,8));
        return 0;
TRY_END
}

static int mat_get_colour_operation_multipass_fallback (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        Ogre::SceneBlendFactor f1 = tex->getColourBlendFallbackSrc();
        Ogre::SceneBlendFactor f2 = tex->getColourBlendFallbackDest();
        push_string_from_factor(L,f1);
        push_string_from_factor(L,f2);
        return 2;
TRY_END
}
static int mat_set_colour_operation_multipass_fallback (lua_State *L)
{
TRY_START
        check_args(L,4+2);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        const char *f1 = luaL_checkstring(L,4);
        const char *f2 = luaL_checkstring(L,5);
        tex->setColourOpMultipassFallback(factor_from_string(L,f1),
                                          factor_from_string(L,f2));
        return 0;
TRY_END
}


static int mat_get_texture_scroll (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        lua_pushnumber(L,tex->getTextureUScroll());
        lua_pushnumber(L,tex->getTextureVScroll());
        return 2;
TRY_END
}
static int mat_set_texture_scroll (lua_State *L)
{
TRY_START
        check_args(L,4+2);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        Ogre::Real u = luaL_checknumber(L,5);
        Ogre::Real v = luaL_checknumber(L,6);
        tex->setTextureScroll(u,v);
        return 0;
TRY_END
}


static int mat_get_texture_scale (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        lua_pushnumber(L,tex->getTextureUScale());
        lua_pushnumber(L,tex->getTextureVScale());
        return 2;
TRY_END
}
static int mat_set_texture_scale (lua_State *L)
{
TRY_START
        check_args(L,4+2);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        Ogre::Real u = luaL_checknumber(L,5);
        Ogre::Real v = luaL_checknumber(L,6);
        tex->setTextureScale(u,v);
        return 0;
TRY_END
}


static int mat_get_texture_rotate (lua_State *L)
{
TRY_START
        check_args(L,4);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        lua_pushnumber(L,tex->getTextureRotate().valueDegrees());
        return 1;
TRY_END
}
static int mat_set_texture_rotate (lua_State *L)
{
TRY_START
        check_args(L,4+1);
        Ogre::TextureUnitState *tex = mat_get_texture_unit_state(L);
        tex->setTextureRotate(Ogre::Degree(luaL_checknumber(L,5)));
        return 0;
TRY_END
}


TOSTRING_GETNAME_MACRO(mat,Ogre::MaterialPtr,->getName(),MAT_TAG)

GC_MACRO(Ogre::MaterialPtr,mat,MAT_TAG)

static int mat_index(lua_State *L) {
        check_args(L,2);
        GET_UD_MACRO(Ogre::MaterialPtr,self,1,MAT_TAG);
        const char *key  = luaL_checkstring(L,2);
        if (!::strcmp(key,"clone")) {
                push_cfunction(L,mat_clone);

        } else if (!::strcmp(key,"load")) {
                push_cfunction(L,mat_load);
        } else if (!::strcmp(key,"unload")) {
                push_cfunction(L,mat_unload);
        } else if (!::strcmp(key,"reload")) {
                push_cfunction(L,mat_reload);
        } else if (!::strcmp(key,"isLoaded")) {
                lua_pushboolean(L,self->isLoaded());
        } else if (!::strcmp(key,"isPrepared")) {
                lua_pushboolean(L,self->isPrepared());

        } else if (!::strcmp(key,"receiveShadows")) {
                lua_pushboolean(L,self->getReceiveShadows());
        } else if (!::strcmp(key,"transparencyCastsShadows")) {
                lua_pushboolean(L,self->getTransparencyCastsShadows());

        } else if (!::strcmp(key,"createTechnique")) {
                push_cfunction(L,mat_create_technique);
        } else if (!::strcmp(key,"createPass")) {
                push_cfunction(L,mat_create_pass);
        } else if (!::strcmp(key,"createTextureUnitState")) {
                push_cfunction(L,mat_create_texture_unit_state);
        } else if (!::strcmp(key,"isTechniqueSupported")) {
                push_cfunction(L,mat_is_technique_supported);
        } else if (!::strcmp(key,"getShadowCasterMaterial")) {
                push_cfunction(L,mat_get_shadow_caster_material);
        } else if (!::strcmp(key,"setShadowCasterMaterial")) {
                push_cfunction(L,mat_set_shadow_caster_material);

        } else if (!::strcmp(key,"removeTechnique")) {
                push_cfunction(L,mat_remove_technique);
        } else if (!::strcmp(key,"removeAllTechniques")) {
                push_cfunction(L,mat_remove_all_techniques);
        } else if (!::strcmp(key,"removePass")) {
                push_cfunction(L,mat_remove_pass);
        } else if (!::strcmp(key,"removeAllPasses")) {
                push_cfunction(L,mat_remove_all_passes);
        } else if (!::strcmp(key,"removeTextureUnitState")) {
                push_cfunction(L,mat_remove_texture_unit_state);

        } else if (!::strcmp(key,"getNumTechniques")) {
                push_cfunction(L,mat_get_num_techniques);
        } else if (!::strcmp(key,"getNumPasses")) {
                push_cfunction(L,mat_get_num_passes);
        } else if (!::strcmp(key,"getNumTextureUnits")) {
                push_cfunction(L,mat_get_num_texture_unit_states);

        } else if (!::strcmp(key,"hasVertexProgram")) {
                push_cfunction(L,mat_has_vertex_program);
        } else if (!::strcmp(key,"hasFragmentProgram")) {
                push_cfunction(L,mat_has_fragment_program);
        } else if (!::strcmp(key,"getVertexProgramName")) {
                push_cfunction(L,mat_get_vertex_program_name);
        } else if (!::strcmp(key,"getFragmentProgramName")) {
                push_cfunction(L,mat_get_fragment_program_name);
        } else if (!::strcmp(key,"setVertexProgram")) {
                push_cfunction(L,mat_set_vertex_program);
        } else if (!::strcmp(key,"setFragmentProgram")) {
                push_cfunction(L,mat_set_fragment_program);
        } else if (!::strcmp(key,"getEmissiveVertex")) {
                push_cfunction(L,mat_get_emissive_vertex);
        } else if (!::strcmp(key,"getAmbientVertex")) {
                push_cfunction(L,mat_get_ambient_vertex);
        } else if (!::strcmp(key,"getDiffuseVertex")) {
                push_cfunction(L,mat_get_diffuse_vertex);
        } else if (!::strcmp(key,"getSpecularVertex")) {
                push_cfunction(L,mat_get_specular_vertex);
        } else if (!::strcmp(key,"getEmissive")) {
                push_cfunction(L,mat_get_emissive);
        } else if (!::strcmp(key,"getAmbient")) {
                push_cfunction(L,mat_get_ambient);
        } else if (!::strcmp(key,"getDiffuse")) {
                push_cfunction(L,mat_get_diffuse);
        } else if (!::strcmp(key,"getSpecular")) {
                push_cfunction(L,mat_get_specular);
        } else if (!::strcmp(key,"setEmissiveVertex")) {
                push_cfunction(L,mat_set_emissive_vertex);
        } else if (!::strcmp(key,"setAmbientVertex")) {
                push_cfunction(L,mat_set_ambient_vertex);
        } else if (!::strcmp(key,"setDiffuseVertex")) {
                push_cfunction(L,mat_set_diffuse_vertex);
        } else if (!::strcmp(key,"setSpecularVertex")) {
                push_cfunction(L,mat_set_specular_vertex);
        } else if (!::strcmp(key,"setEmissive")) {
                push_cfunction(L,mat_set_emissive);
        } else if (!::strcmp(key,"setAmbient")) {
                push_cfunction(L,mat_set_ambient);
        } else if (!::strcmp(key,"setDiffuse")) {
                push_cfunction(L,mat_set_diffuse);
        } else if (!::strcmp(key,"setSpecular")) {
                push_cfunction(L,mat_set_specular);
        } else if (!::strcmp(key,"getSceneBlending")) {
                push_cfunction(L,mat_get_scene_blending);
        } else if (!::strcmp(key,"setSceneBlending")) {
                push_cfunction(L,mat_set_scene_blending);
        } else if (!::strcmp(key,"getCull")) {
                push_cfunction(L,mat_get_cull);
        } else if (!::strcmp(key,"setCull")) {
                push_cfunction(L,mat_set_cull);
        } else if (!::strcmp(key,"getManualCull")) {
                push_cfunction(L,mat_get_manual_cull);
        } else if (!::strcmp(key,"setManualCull")) {
                push_cfunction(L,mat_set_manual_cull);
        } else if (!::strcmp(key,"getAlphaRejection")) {
                push_cfunction(L,mat_get_alpha_rejection);
        } else if (!::strcmp(key,"setAlphaRejection")) {
                push_cfunction(L,mat_set_alpha_rejection);
        } else if (!::strcmp(key,"getAlphaToCoverage")) {
                push_cfunction(L,mat_get_alpha_to_coverage);
        } else if (!::strcmp(key,"setAlphaToCoverage")) {
                push_cfunction(L,mat_set_alpha_to_coverage);
        } else if (!::strcmp(key,"getDepthFunction")) {
                push_cfunction(L,mat_get_depth_function);
        } else if (!::strcmp(key,"setDepthFunction")) {
                push_cfunction(L,mat_set_depth_function);
        } else if (!::strcmp(key,"getDepthCheckEnabled")) {
                push_cfunction(L,mat_get_depth_check_enabled);
        } else if (!::strcmp(key,"setDepthCheckEnabled")) {
                push_cfunction(L,mat_set_depth_check_enabled);
        } else if (!::strcmp(key,"getDepthWriteEnabled")) {
                push_cfunction(L,mat_get_depth_write_enabled);
        } else if (!::strcmp(key,"setDepthWriteEnabled")) {
                push_cfunction(L,mat_set_depth_write_enabled);
        } else if (!::strcmp(key,"getLightingEnabled")) {
                push_cfunction(L,mat_get_lighting);
        } else if (!::strcmp(key,"setLightingEnabled")) {
                push_cfunction(L,mat_set_lighting);
        } else if (!::strcmp(key,"getShadingMode")) {
                push_cfunction(L,mat_get_shading_mode);
        } else if (!::strcmp(key,"setShadingMode")) {
                push_cfunction(L,mat_set_shading_mode);
        } else if (!::strcmp(key,"getPolygonMode")) {
                push_cfunction(L,mat_get_polygon_mode);
        } else if (!::strcmp(key,"setPolygonMode")) {
                push_cfunction(L,mat_set_polygon_mode);
        } else if (!::strcmp(key,"getDepthBias")) {
                push_cfunction(L,mat_get_depth_bias);
        } else if (!::strcmp(key,"setDepthBias")) {
                push_cfunction(L,mat_set_depth_bias);
        } else if (!::strcmp(key,"getFog")) {
                push_cfunction(L,mat_get_fog);
        } else if (!::strcmp(key,"setFog")) {
                push_cfunction(L,mat_set_fog);
        } else if (!::strcmp(key,"getTransparentSortingEnabled")) {
                push_cfunction(L,mat_get_transparent_sorting);
        } else if (!::strcmp(key,"setTransparentSortingEnabled")) {
                push_cfunction(L,mat_set_transparent_sorting);
        } else if (!::strcmp(key,"getTransparentSortingForced")) {
                push_cfunction(L,mat_get_transparent_sorting_forced);
        } else if (!::strcmp(key,"setTransparentSortingForced")) {
                push_cfunction(L,mat_set_transparent_sorting_forced);
/* not supported it seems
        } else if (!::strcmp(key,"getVertexConstantFloat")) {
                push_cfunction(L,mat_get_vertex_constant_float);
*/
        } else if (!::strcmp(key,"setVertexConstantFloat")) {
                push_cfunction(L,mat_set_vertex_constant_float);
/* not supported it seems
        } else if (!::strcmp(key,"getFragmentConstantFloat")) {
                push_cfunction(L,mat_get_fragment_constant_float);
*/
        } else if (!::strcmp(key,"setFragmentConstantFloat")) {
                push_cfunction(L,mat_set_fragment_constant_float);

        } else if (!::strcmp(key,"getTextureName")) {
                push_cfunction(L,mat_get_texture_name);
        } else if (!::strcmp(key,"setTextureName")) {
                push_cfunction(L,mat_set_texture_name);
        } else if (!::strcmp(key,"getIsShadow")) {
                push_cfunction(L,mat_get_is_shadow);
        } else if (!::strcmp(key,"setIsShadow")) {
                push_cfunction(L,mat_set_is_shadow);
        } else if (!::strcmp(key,"setCubicTextureName")) {
                push_cfunction(L,mat_set_cubic_texture_name);
        } else if (!::strcmp(key,"getEnvMap")) {
                push_cfunction(L,mat_get_env_map);
        } else if (!::strcmp(key,"setEnvMap")) {
                push_cfunction(L,mat_set_env_map);
        } else if (!::strcmp(key,"getTextureFiltering")) {
                push_cfunction(L,mat_get_texture_filtering);
        } else if (!::strcmp(key,"setTextureFiltering")) {
                push_cfunction(L,mat_set_texture_filtering);
        } else if (!::strcmp(key,"getTextureAnisotropy")) {
                push_cfunction(L,mat_get_texture_anisotropy);
        } else if (!::strcmp(key,"setTextureAnisotropy")) {
                push_cfunction(L,mat_set_texture_anisotropy);
        } else if (!::strcmp(key,"getMipmapBias")) {
                push_cfunction(L,mat_get_mipmap_bias);
        } else if (!::strcmp(key,"setMipmapBias")) {
                push_cfunction(L,mat_set_mipmap_bias);
        } else if (!::strcmp(key,"getColourOperationMultipassFallback")) {
               push_cfunction(L,mat_get_colour_operation_multipass_fallback);
        } else if (!::strcmp(key,"setColourOperationMultipassFallback")) {
               push_cfunction(L,mat_set_colour_operation_multipass_fallback);
        } else if (!::strcmp(key,"getColourOperation")) {
                push_cfunction(L,mat_get_colour_operation);
        } else if (!::strcmp(key,"setColourOperation")) {
                push_cfunction(L,mat_set_colour_operation);
        } else if (!::strcmp(key,"getAlphaOperation")) {
                push_cfunction(L,mat_get_alpha_operation);
        } else if (!::strcmp(key,"setAlphaOperation")) {
                push_cfunction(L,mat_set_alpha_operation);
        } else if (!::strcmp(key,"getTextureScale")) {
                push_cfunction(L,mat_get_texture_scale);
        } else if (!::strcmp(key,"setTextureScale")) {
                push_cfunction(L,mat_set_texture_scale);
        } else if (!::strcmp(key,"getTextureScroll")) {
                push_cfunction(L,mat_get_texture_scroll);
        } else if (!::strcmp(key,"setTextureScroll")) {
                push_cfunction(L,mat_set_texture_scroll);
        } else if (!::strcmp(key,"getTextureRotate")) {
                push_cfunction(L,mat_get_texture_rotate);
        } else if (!::strcmp(key,"setTextureRotate")) {
                push_cfunction(L,mat_set_texture_rotate);

        } else if (!::strcmp(key,"name")) {
                lua_pushstring(L,self->getName().c_str());
        } else if (!::strcmp(key,"num_techniques")) {
                lua_pushnumber(L,self->getNumTechniques());
        } else {
                my_lua_error(L,"Not a valid Material member: " + std::string(key));
        }
        return 1;

}

EQ_MACRO(Ogre::MaterialPtr,mat,MAT_TAG)

static int mat_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::MaterialPtr,self,1,MAT_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="receiveShadows") {
                bool v = check_bool(L,3);
                self->setReceiveShadows(v);
        } else if (key=="transparencyCastsShadows") {
                bool v = check_bool(L,3);
                self->setTransparencyCastsShadows(v);
        } else {
                my_lua_error(L,"Not a writeable Material member: "+key);
        }
        return 0;
TRY_END
}

MT_MACRO_NEWINDEX(mat);


// vim: shiftwidth=8:tabstop=8:expandtab
