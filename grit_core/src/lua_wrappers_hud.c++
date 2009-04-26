#include "HUD.h"

#include "lua_wrappers_hud.h"
#include "lua_wrappers_material.h"


static HUD::TreePtr *check_hud_tree (lua_State *L, int index)
{
TRY_START
        HUD::TreePtr *ptr = NULL;
        if (has_tag(L,index,PANE_TAG)) {
                ptr = *static_cast<HUD::TreePtr**>(lua_touserdata(L,index));
        } else if (has_tag(L,index,TEXT_TAG)) {
                ptr = *static_cast<HUD::TreePtr**>(lua_touserdata(L,index));
        }

        if (!ptr) {
                std::string acceptable_types;
                acceptable_types += PANE_TAG ", or ";
                acceptable_types += TEXT_TAG;
                luaL_typerror(L,index,acceptable_types.c_str());
        }
        return ptr;
TRY_END
}

static int tree_trigger_resize (lua_State *L)
{
TRY_START
        check_args(L,1);
        HUD::TreePtr& tp = *check_hud_tree(L,1);
        tp->_recalculate();
        return 0;
TRY_END
}


static int pane_add_child (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==2)
                lua_newtable(L); //opt = opt or {}
        if (lua_gettop(L)!=3) {
                std::stringstream msg;
                msg << "Wrong number of arguments: " << lua_gettop(L)
                    << " should be 2 or 3";
                my_lua_error(L,msg.str());
        }
        GET_UD_MACRO(HUD::PanePtr,pane,1,PANE_TAG);
        std::string type = luaL_checkstring(L,2);

        lua_getfield(L,3,"visible");
        bool visible = true;
        if (lua_isboolean(L,-1)) {
                visible = 0!=lua_toboolean(L,-1);
        }
        lua_pop(L,1);

        if (type=="Pane") {
                HUD::TreePtr tree = pane->createChild(type);
                tree->setVisible(visible);

                HUD::Pane *p = static_cast<HUD::Pane*>(&*tree);

                lua_getfield(L,3,"material");
                if (is_userdata(L,-1,MAT_TAG)) {
                        GET_UD_MACRO(Ogre::MaterialPtr,mat,-1,MAT_TAG);
                        p->setMaterial(mat->getName());
                }
                if (lua_isstring(L,-1)) {
                        const char *name = luaL_checkstring(L,-1);
                        p->setMaterial(name);
                }
                lua_pop(L,1);

                push(L,new HUD::TreePtr(tree),PANE_TAG);
        } else if (type=="Text") {
                HUD::TreePtr tree = pane->createChild(type);
                tree->setVisible(visible);

                HUD::Text *t = static_cast<HUD::Text*>(&*tree);

                lua_getfield(L,3,"font");
                if (lua_isstring(L,-1)) {
                        const char *str = lua_tostring(L,-1);
                        t->setFont(str);
                }
                lua_pop(L,1);

                lua_getfield(L,3,"charHeight");
                if (lua_isstring(L,-1)) {
                        lua_Number n = lua_tonumber(L,-1);
                        t->setCharHeight(n);
                }
                lua_pop(L,1);

                push(L,new HUD::TreePtr(tree),TEXT_TAG);
        } else {
                lua_getglobal(L,"Hud");
                if (lua_isnil(L,-1))
                        my_lua_error(L,"Hud table doesn't exist!");
                lua_getfield(L,-1,type.c_str());
                if (lua_isnil(L,-1))
                        my_lua_error(L,"Unknown type: \""+type+"\"");
                lua_getfield(L,-1,"new");
                if (lua_isnil(L,-1))
                        my_lua_error(L,"No constructor for type: \""+type+"\"");
                lua_pushvalue(L,1); // the PanePtr UD
                lua_pushvalue(L,3); // the optional table or nil
                check_stack(L,1);
                lua_call(L,2,1); // pushes the new object onto the stack
        }
        return 1;
TRY_END
}

static int pane_remove_child (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(HUD::PanePtr,pane,1,PANE_TAG);
        if (lua_istable(L,2)) {
                lua_getfield(L,2,"destroy");
                lua_pushvalue(L,2);
                lua_pushvalue(L,1);
                lua_call(L,2,0);
        } else {
                HUD::TreePtr& tp = *check_hud_tree(L,2);
                pane->removeChild(tp);
        }
        return 0;
TRY_END
}


static int pane_remove_all (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(HUD::PanePtr,pane,1,PANE_TAG);
        pane->removeAllChildren();
        return 0;
TRY_END
}


static int pane_get_uv (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(HUD::PanePtr,pane,1,PANE_TAG);
        Ogre::Real u1;
        Ogre::Real v1;
        Ogre::Real u2;
        Ogre::Real v2;
        pane->getUV(u1,v1,u2,v2);
        lua_pushnumber(L,u1);
        lua_pushnumber(L,v1);
        lua_pushnumber(L,u2);
        lua_pushnumber(L,v2);
        return 4;
TRY_END
}

static int pane_set_uv (lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(HUD::PanePtr,pane,1,PANE_TAG);
        Ogre::Real u1 = luaL_checknumber(L,2);
        Ogre::Real v1 = luaL_checknumber(L,3);
        Ogre::Real u2 = luaL_checknumber(L,4);
        Ogre::Real v2 = luaL_checknumber(L,5);
        pane->setUV(u1,v1,u2,v2);
        return 0;
TRY_END
}


class LuaCallback : public HUD::Calculator {
        public:
        LuaCallback (lua_State *L) {
                this->L = L;
                func_key = luaL_ref(L,LUA_REGISTRYINDEX); //pops
                lua_pop(L,1); //pops
                ud_key = luaL_ref(L,LUA_REGISTRYINDEX); //pops
        }
        ~LuaCallback () {
                luaL_unref(L,LUA_REGISTRYINDEX,ud_key);
                luaL_unref(L,LUA_REGISTRYINDEX,func_key);
        }
        HUD::dim_t check_dim_t(int index, HUD::dim_t dfault, HUD::dim_t neg)
        {
                if (lua_isnil(L,index)) return dfault;
                if (lua_type(L,index)!=LUA_TNUMBER) {
                        std::stringstream msg;
                        msg<<"Return value #"<<5+index<<" was not a number: \""
                           <<lua_typename(L,lua_type(L,index))<<"\"";
                        my_lua_error(L,msg.str());
                }
                lua_Number n = lua_tonumber(L,index);
                n = floor(n);
                if (n<0) n += 1+neg;
                if (n>HUD::DIM_T_MAX) n = HUD::DIM_T_MAX;
                if (n<0) n = 0;
                return (HUD::dim_t)n;
        }
        void f(HUD::Tree *t, HUD::dim_t pw, HUD::dim_t ph,
               HUD::dim_t& x, HUD::dim_t& y, HUD::dim_t& w, HUD::dim_t& h)
        {
                check_stack(L,4);
                lua_State *L2 = lua_newthread(L); // the coroutine
                lua_rawgeti(L,LUA_REGISTRYINDEX,func_key);
                lua_pushnumber(L,pw);
                lua_pushnumber(L,ph);
                lua_rawgeti(L,LUA_REGISTRYINDEX,ud_key);
                lua_xmove(L,L2,4); // l has coro, l2 has 4 elements
                int result = lua_resume(L2,3);
                while (true) {
                        if (result!=LUA_YIELD && result!=0) {
                                lua_xmove(L2,L,1);
                                int r = my_lua_error_handler(L,L2,0);
                                lua_pop(L,r); // pop message
                                break;
                        }
                        // "l2" contains results of yield/return
                        lua_settop(L2,4);
                        lua_xmove(L2,L,4); // leaves l2 empty
                        w = check_dim_t(-2,pw,0);
                        h = check_dim_t(-1,ph,0);
                        x = check_dim_t(-4,0,pw-t->getWidth());
                        y = check_dim_t(-3,0,ph-t->getHeight());
                        lua_pop(L,4); // leaves l with just coro
                        if (result==0) break; // returned, didn't yield
                        lua_pushnumber(L,x);
                        lua_pushnumber(L,y);
                        lua_pushnumber(L,w);
                        lua_pushnumber(L,h);
                        lua_xmove(L,L2,4); // l has coro, l2 has the 4 vals
                        // don't need to pass the function this time
                        result = lua_resume(L2,4);
                }
                lua_pop(L,1); // the coroutine (matches lua_newthread)

        }
        protected:
        lua_State *L;
        int ud_key;
        int func_key;
};

TOSTRING_SMART_PTR_MACRO(pane,HUD::PanePtr,PANE_TAG)
TOSTRING_SMART_PTR_MACRO(text,HUD::TextPtr,TEXT_TAG)

GC_MACRO(HUD::PanePtr,pane,PANE_TAG)
GC_MACRO(HUD::TextPtr,text,TEXT_TAG)

static bool tree_index(lua_State *L, HUD::Tree *tree, const std::string& key)
{
TRY_START
        if (key=="x") {
                lua_pushnumber(L,tree->getPosX());
        } else if (key=="y") {
                lua_pushnumber(L,tree->getPosY());
        } else if (key=="width") {
                lua_pushnumber(L,tree->getWidth());
        } else if (key=="height") {
                lua_pushnumber(L,tree->getHeight());
        } else if (key=="visible") {
                lua_pushboolean(L,tree->isVisible());
        } else if (key=="type") {
                if (typeid(*tree)==typeid(HUD::Root)) {
                        lua_pushstring(L,"Root");
                } else if (typeid(*tree)==typeid(HUD::Pane)) {
                        lua_pushstring(L,"Pane");
                } else if (typeid(*tree)==typeid(HUD::Text)) {
                        lua_pushstring(L,"Text");
                } else {
                        my_lua_error(L,"Unrecognised HUD element.");
                }
        } else if (key=="triggerResize") {
                push_cfunction(L,tree_trigger_resize);
        } else {
                return false;
        }
        return true;
TRY_END
}

static int pane_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(HUD::PanePtr,pane,1,PANE_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (tree_index(L,&*pane,key)) return 1;
        if (key=="addChild") {
                push_cfunction(L,pane_add_child);
        } else if (key=="removeChild") {
                push_cfunction(L,pane_remove_child);
        } else if (key=="removeAllChildren") {
                push_cfunction(L,pane_remove_all);
        } else if (key=="getUV") {
                push_cfunction(L,pane_get_uv);
        } else if (key=="setUV") {
                push_cfunction(L,pane_set_uv);
        } else if (key=="children") {
                HUD::TreePtrs trees = pane->getChildren();
                lua_newtable(L);
                for (HUD::TreePtrs::iterator i=trees.begin() ;
                     i < trees.end() ; i++) {
                        if (typeid(**i)==typeid(HUD::Pane)) {
                                push(L,new HUD::TreePtr(*i),PANE_TAG);
                        } else if (typeid(**i)==typeid(HUD::Text)) {
                                push(L,new HUD::TreePtr(*i),TEXT_TAG);
                        } else {
                                my_lua_error(L,"Unrecognised HUD element.");
                        }
                        lua_rawseti(L,-2,1+(i-trees.begin()));
                }
        } else if (key=="material") {
                Ogre::MaterialPtr mptr = pane->getMaterial();
                if (mptr.isNull()) {
                        lua_pushnil(L);
                } else {
                        push(L,new Ogre::MaterialPtr(mptr),MAT_TAG);
                }
        } else {
                my_lua_error(L, "Not a valid HUD::Pane member: "+key);
        }
        return 1;
TRY_END
}

static int text_reset(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(HUD::TextPtr,text,1,TEXT_TAG);
        text->resetText();
        return 0;
TRY_END
}

static int text_append(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(HUD::TextPtr,text,1,TEXT_TAG);
        const char *str = luaL_checkstring(L,2);
        text->appendText(str);
        return 0;
TRY_END
}

static int text_commit(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(HUD::TextPtr,text,1,TEXT_TAG);
        text->commitText();
        return 0;
TRY_END
}

static int text_set_colour_top(lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(HUD::TextPtr,text,1,TEXT_TAG);
        Ogre::Real r = luaL_checknumber(L,2);
        Ogre::Real g = luaL_checknumber(L,3);
        Ogre::Real b = luaL_checknumber(L,4);
        Ogre::Real a = luaL_checknumber(L,5);
        text->setColourTop(Ogre::ColourValue(r,g,b,a));
        return 0;
TRY_END
}

static int text_get_colour_top(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(HUD::TextPtr,text,1,TEXT_TAG);
        Ogre::ColourValue cv = text->getColourTop();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        lua_pushnumber(L,cv.a);
        return 4;
TRY_END
}

static int text_set_colour_bottom(lua_State *L)
{
TRY_START
        check_args(L,5);
        GET_UD_MACRO(HUD::TextPtr,text,1,TEXT_TAG);
        Ogre::Real r = luaL_checknumber(L,2);
        Ogre::Real g = luaL_checknumber(L,3);
        Ogre::Real b = luaL_checknumber(L,4);
        Ogre::Real a = luaL_checknumber(L,5);
        text->setColourBottom(Ogre::ColourValue(r,g,b,a));
        return 0;
TRY_END
}

static int text_get_colour_bottom(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(HUD::TextPtr,text,1,TEXT_TAG);
        Ogre::ColourValue cv = text->getColourBottom();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        lua_pushnumber(L,cv.a);
        return 4;
TRY_END
}


static int text_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(HUD::TextPtr,text,1,TEXT_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (tree_index(L,&*text,key)) return 1;
        if (key=="reset") {
                push_cfunction(L,text_reset);
        } else if (key=="append") {
                push_cfunction(L,text_append);
        } else if (key=="commit") {
                push_cfunction(L,text_commit);
        } else if (key=="text") {
                lua_pushstring(L,text->getText().asUTF8_c_str());
        } else if (key=="font") {
                lua_pushstring(L,text->getFont().c_str());
        } else if (key=="charHeight") {
                lua_pushnumber(L,text->getCharHeight());
        } else if (key=="setColourTop") {
                push_cfunction(L,text_set_colour_top);
        } else if (key=="getColourTop") {
                push_cfunction(L,text_get_colour_top);
        } else if (key=="setColourBottom") {
                push_cfunction(L,text_set_colour_bottom);
        } else if (key=="getColourBottom") {
                push_cfunction(L,text_get_colour_bottom);
        } else {
                std::string s("Not a valid HUD::Text member: ");
                s = s + key;
                my_lua_error(L,s.c_str());
        }
        return 1;
TRY_END
}

static int pane_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(HUD::PanePtr,pane,1,PANE_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="visible") {
                bool val = 0!=lua_toboolean(L,3);
                pane->setVisible(val);
        } else if (key=="material") {
                if (lua_isstring(L,3)) {
                        std::string name = luaL_checkstring(L,3);
                        pane->setMaterial(name);
                } else {
                        GET_UD_MACRO(Ogre::MaterialPtr,mat,1,MAT_TAG);
                        pane->setMaterial(mat);
                }
        } else if (key=="resize") {
                luaL_checktype(L,3,LUA_TFUNCTION);
                lua_getglobal(L,"MAIN_STATE");
                lua_State *lbase = lua_tothread(L,4);
                if (lbase==NULL) {
                        CERR<<"accessing MAIN_STATE: null pointer!"<<std::endl;
                } else {
                        lua_pop(L,1);
                        lua_xmove(L,lbase,3);
                        // pop all 3 params:
                        HUD::CalculatorPtr cp(new LuaCallback(lbase));
                        pane->setCalculator(cp); // frees old if it exists
                }
        } else {
                my_lua_error(L,"Not a valid HUD::Pane field: " + key);
        }
        return 0;

TRY_END
}

static int text_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(HUD::TextPtr,text,1,TEXT_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="visible") {
                bool val = 0!=lua_toboolean(L,3);
                text->setVisible(val);
        } else if (key=="font") {
                std::string name = luaL_checkstring(L,3);
                text->setFont(name);
        } else if (key=="text") {
                const char *str = luaL_checkstring(L,3);
                text->resetText();
                text->appendText(str);
                text->commitText();
        } else if (key=="charHeight") {
                lua_Number n = luaL_checknumber(L,3);
                text->setCharHeight(n);
        } else if (key=="resize") {
                luaL_checktype(L,3,LUA_TFUNCTION);
                lua_getglobal(L,"MAIN_STATE");
                lua_State *lbase = lua_tothread(L,4);
                if (lbase==NULL) {
                        CERR<<"accessing MAIN_STATE: null pointer!"<<std::endl;
                } else {
                        lua_pop(L,1);
                        lua_xmove(L,lbase,3);
                        // pops all 3 params
                        HUD::CalculatorPtr cp(
                                new LuaCallback(lbase));
                        // frees old if it exists
                        text->setCalculator(cp);
                }
        } else {
                my_lua_error(L,"Not a valid HUD::Text field: " + key);
        }
        return 0;

TRY_END
}

EQ_MACRO(HUD::PanePtr,pane,PANE_TAG)
EQ_MACRO(HUD::TextPtr,text,TEXT_TAG)


MT_MACRO_NEWINDEX(pane);
MT_MACRO_NEWINDEX(text);

// vim: shiftwidth=8:tabstop=8:expandtab
