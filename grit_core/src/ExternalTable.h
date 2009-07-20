class ExternalTable;


#ifndef ExternalTable_h
#define ExternalTable_h

#include <map>

#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreString.h>

extern "C" {
        #include <lua.h>
}


class ExternalTable {

    public:

        bool has (const Ogre::String &key)
        {
                return fields.find(key)!=fields.end();
        }

        void set (const Ogre::String &key, const Ogre::Real r)
        {
                fields[key].type = 0;
                fields[key].real = r;
        }

        void set (const Ogre::String &key, const Ogre::String &s)
        {
                fields[key].type = 1;
                fields[key].str = s;
        }

        void set (const Ogre::String &key, const Ogre::Vector3 &v3)
        {
                fields[key].type = 2;
                fields[key].v3 = v3;
        }

        void set (const Ogre::String &key, const Ogre::Quaternion &q)
        {
                fields[key].type = 3;
                fields[key].q = q;
        }

        void set (const Ogre::String &key, bool b)
        {
                fields[key].type = 4;
                fields[key].b = b;
        }

        void set (const Ogre::String &key,const std::vector<Ogre::String> &strs)
        {
                fields[key].type = 5;
                fields[key].strs = strs;
        }

        const char *luaGet (lua_State *L);

        const char *luaSet (lua_State *L);

        const char *luaGet (lua_State *L, const Ogre::String &key);

        const char *luaSet (lua_State *L, const Ogre::String &key);

        void unset (const Ogre::String &key)
        {
                fields.erase(key);
        }


        void dump (lua_State *L);
                

        struct Value {
                int type;
                Ogre::String str;
                Ogre::Real real;
                Ogre::Vector3 v3;
                Ogre::Quaternion q;
                bool b;
                std::vector<Ogre::String> strs;
        };

    protected:

        typedef std::map<Ogre::String,Value> ValueMap;
        ValueMap fields;

};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
