#!/bin/sh

SOURCE='
static void push_NAME (lua_State *L, UNKNOWN_TYPE *self)
{
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (self);
        luaL_getmetatable(L, UNKNOWN_TAG);
        lua_setmetatable(L, -2);
        PARENT *p = self->getParent();
        TODO_maps& maps = grit->getUserDataTables().parents[parent];
        maps.UDNAME[self].push_back(ud);
}

static int NAME_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(UNKNOWN_TYPE,self,1,UNKNOWN_TAG,0);
        if (self==NULL) return 0;
        APP_VERBOSE("nullifying "UNKNOWN_TAG);
        vec_nullify_remove(grit->getUserDataTables().UDNAME[self],&self);
        return 0;
TRY_END
}

static int NAME_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(UNKNOWN_TYPE,self,1,UNKNOWN_TAG);
        APP_VERBOSE("destroying "UNKNOWN_TAG);
        self.destroySomehow();
        map_nullify_remove(grit->getUserDataTables().UDNAME,&self);
        return 0;
TRY_END
}


TOSTRING_ADDR_MACRO(NAME,UNKNOWN_TYPE,UNKNOWN_TAG)


static int NAME_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(UNKNOWN_TYPE,self,1,UNKNOWN_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="destroy") {
                lua_pushcfunction(L,NAME_destroy);
        } else if (key=="someField") {
                int v = 3;
                lua_pushnumber(L,v);
        } else {
                my_lua_error(L,"Not a valid USER_VISIBLE_NAME member: "+key);
        }
        return 1;
TRY_END
}

static int NAME_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(UNKNOWN_TYPE,self,1,UNKNOWN_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="someField") {
                bool v = lua_toboolean(L,3);
                self.setSomeField(v);
        } else {
                my_lua_error(L,"Not a valid USER_VISIBLE_NAME member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(UNKNOWN_TYPE,NAME,UNKNOWN_TAG)

MT_MACRO_NEWINDEX(NAME);'


echo "Type to be wrapped (e.g. Ogre::MaterialPtr)"
read TYPE
echo "Name (e.g. mat)"
read NAME
echo "Userdata TAG (e.g. MAT_TAG)"
read TAG
echo "Title in code (e.g. MATERIAL)"
read TITLE
echo "User-visible name (e.g. Material)"
read UVNAME
echo "Userdata table name (e.g. materials)"
read UDNAME

echo "// $TITLE ===================================================================================" | cut -b 1-80

echo "$SOURCE" | sed "s/UNKNOWN_TYPE/$TYPE/g" | sed "s/UNKNOWN_TAG/$TAG/g" | sed "s/USER_VISIBLE_NAME/$UVNAME/g" | sed "s/UDNAME/$UDNAME/g"  | sed "s/NAME/$NAME/g" 
