/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include "grit_class.h"

static GritClassMap classes;

GritClass *class_add (lua_State *L, const std::string& name)
{
    GritClass *gcp;
    GritClassMap::iterator i = classes.find(name);
    
    if (i!=classes.end()) {
        gcp = i->second;
        int index = lua_gettop(L);
        for (lua_pushnil(L) ; lua_next(L,index)!=0 ; lua_pop(L,1)) {
            gcp->set(L);
        }       
        lua_pop(L,1); // the table we just iterated through
        gcp->setParent(L);
    } else {
        // add it and return it
        gcp = new GritClass(L,name);
        classes[name] = gcp;
    }
    return gcp;
}


void class_del (lua_State *L, GritClass *c)
{
    // anything using this class keeps using it
    classes.erase(c->name);
    // class gets deleted properly when everything stops using it
    c->release(L);
}


GritClass *class_get (const std::string &name)
{
    GritClassMap::iterator i = classes.find(name);
    if (i==classes.end())
        GRIT_EXCEPT("GritClass does not exist: "+name);
    return i->second;
}

bool class_has (const std::string &name)
{
    return classes.find(name)!=classes.end();
}

        
void class_all_del (lua_State *L)
{
    GritClassMap m = classes;
    for (GritClassMap::iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
        class_del(L, i->second);
    }       
}

void class_all (GritClassMap::iterator &begin, GritClassMap::iterator &end)
{
    begin = classes.begin();
    end = classes.end();
}

size_t class_count (void)
{
    return classes.size();
}
