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

        void unset (const Ogre::String &key) { fields.erase(key); }

        void clear (void) { fields.clear(); }


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
