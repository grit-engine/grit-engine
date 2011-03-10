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
#include <string>

extern "C" {
        #include <lua.h>
        #include <lauxlib.h>
}

#include "math_util.h"
#include "SharedPtr.h"
#include "LuaPtr.h"

#include "SplineTable.h"

// Purpose of this class is to store lua data outside of lua so as to avoid
// putting stress on the garbage collector.  Only certain kinds of primitive data
// are supported.

class ExternalTable {

    public:

        ExternalTable (void) { }

        void destroy (lua_State *L);

        bool has (const std::string &key) { return fields.find(key)!=fields.end(); }
        bool has (lua_Number key) { return elements.find(key)!=elements.end(); }

        template<class U> void get (const std::string &key, U &val, const U &def)
        {
                if (!has(key) || !get(key,val)) val = def;
                
        }

        template<class U> void get (lua_Number key, U &val, const U &def)
        {
                if (!has(key) || !get(key,val)) val = def;
                
        }

        bool get (const std::string &key, lua_Number &v)
        {
                if (fields[key].type != 0) return false;
                v = fields[key].real;
                return true;
        }

        bool get (const std::string &key, std::string &v)
        {
                if (fields[key].type != 1) return false;
                v = fields[key].str;
                return true;
        }

        bool get (const std::string &key, Vector3 &v)
        {
                if (fields[key].type != 2) return false;
                v = fields[key].v3 ;
                return true;
        }

        bool get (const std::string &key, Quaternion &v)
        {
                if (fields[key].type != 3) return false;
                v = fields[key].q;
                return true;
        }

        bool get (const std::string &key, bool &v)
        {
                if (fields[key].type != 4) return false;
                v = fields[key].b;
                return true;
        }

        bool get (const std::string &key, SharedPtr<ExternalTable> &v)
        {
                if (fields[key].type != 5) return false;
                v = fields[key].t;
                return true;
        }

        bool get (const std::string &key, Plot &v)
        {
                if (fields[key].type != 6) return false;
                v = fields[key].plot;
                return true;
        }

        bool get (const std::string &key, PlotV3 &v)
        {
                if (fields[key].type != 7) return false;
                v = fields[key].plot_v3;
                return true;
        }

        bool get (lua_Number key, lua_Number &v)
        {
                if (elements[key].type != 0) return false;
                v = elements[key].real;
                return true;
        }

        bool get (lua_Number key, std::string &v)
        {
                if (elements[key].type != 1) return false;
                v = elements[key].str;
                return true;
        }

        bool get (lua_Number key, Vector3 &v)
        {
                if (elements[key].type != 2) return false;
                v = elements[key].v3 ;
                return true;
        }

        bool get (lua_Number key, Quaternion &v)
        {
                if (elements[key].type != 3) return false;
                v = elements[key].q;
                return true;
        }

        bool get (lua_Number key, bool &v)
        {
                if (elements[key].type != 4) return false;
                v = elements[key].b;
                return true;
        }

        bool get (lua_Number key, SharedPtr<ExternalTable> &v)
        {
                if (elements[key].type != 5) return false;
                v = elements[key].t;
                return true;
        }

        bool get (lua_Number key, Plot &v)
        {
                if (elements[key].type != 6) return false;
                v = elements[key].plot;
                return true;
        }

        bool get (lua_Number key, PlotV3 &v)
        {
                if (elements[key].type != 7) return false;
                v = elements[key].plot_v3;
                return true;
        }


        void set (const std::string &key, const lua_Number r)
        {
                fields[key].type = 0;
                fields[key].real = r;
        }

        void set (const std::string &key, const std::string &s)
        {
                fields[key].type = 1;
                fields[key].str = s;
        }

        void set (const std::string &key, const Vector3 &v3)
        {
                fields[key].type = 2;
                fields[key].v3 = v3;
        }

        void set (const std::string &key, const Quaternion &q)
        {
                fields[key].type = 3;
                fields[key].q = q;
        }

        void set (const std::string &key, bool b)
        {
                fields[key].type = 4;
                fields[key].b = b;
        }

        void set (const std::string &key, const SharedPtr<ExternalTable> &t)
        {
                fields[key].type = 5;
                fields[key].t = t;
        }

        void set (const std::string &key, Plot plot)
        {
                fields[key].type = 6;
                fields[key].plot = plot;
        }

        void set (const std::string &key, PlotV3 plot_v3)
        {
                fields[key].type = 7;
                fields[key].plot_v3 = plot_v3;
        }

        void set (lua_Number key, const lua_Number r)
        {
                elements[key].type = 0;
                elements[key].real = r;
        }

        void set (lua_Number key, const std::string &s)
        {
                elements[key].type = 1;
                elements[key].str = s;
        }

        void set (lua_Number key, const Vector3 &v3)
        {
                elements[key].type = 2;
                elements[key].v3 = v3;
        }

        void set (lua_Number &key, const Quaternion &q)
        {
                elements[key].type = 3;
                elements[key].q = q;
        }

        void set (lua_Number &key, bool b)
        {
                elements[key].type = 4;
                elements[key].b = b;
        }

        void set (lua_Number &key, const SharedPtr<ExternalTable> &t)
        {
                elements[key].type = 5;
                elements[key].t = t;
        }

        void set (lua_Number &key, const Plot &plot)
        {
                elements[key].type = 6;
                elements[key].plot = plot;
        }

        void set (lua_Number &key, const PlotV3 &plot_v3)
        {
                elements[key].type = 7;
                elements[key].plot_v3 = plot_v3;
        }

        const char *luaGet (lua_State *L);
        const char *luaSet (lua_State *L);

        const char *luaGet (lua_State *L, const std::string &key);
        const char *luaSet (lua_State *L, const std::string &key);

        const char *luaGet (lua_State *L, lua_Number key);
        const char *luaSet (lua_State *L, lua_Number key);

        void unset (const std::string &key) { fields.erase(key); }
        void unset (lua_Number key) { elements.erase(key); }

        void clear (lua_State *L);

        void dump (lua_State *L);
        void takeTableFromLuaStack (lua_State *L, int tab);

        template<typename T> class UniquePtr {
                T *ptr;
            public:
                UniquePtr () : ptr(NULL) { }
                ~UniquePtr (void) { delete ptr; }
                T *operator-> (void) const { return ptr; }
                T &operator* (void) const { return *ptr; }
                T *operator= (T *ptr_) { delete ptr; return ptr=ptr_; }
        };

        struct Value {
                int type;
                std::string str;
                lua_Number real;
                Vector3 v3;
                Quaternion q;
                bool b;
                SharedPtr<ExternalTable> t;
                Plot plot;
                PlotV3 plot_v3;
                LuaPtr func;
        };

    protected:

        typedef std::map<std::string,Value> StringMap;
        StringMap fields;

        typedef std::map<lua_Number,Value> NumberMap;
        NumberMap elements;

};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
