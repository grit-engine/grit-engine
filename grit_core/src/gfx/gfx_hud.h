/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

#include <string>
#include <map>

#include "../SharedPtr.h"

class GfxHudBase;
class GfxHudObject;
class GfxHudText;

class GfxHudClass;
typedef std::map<std::string,GfxHudClass*> GfxHudClassMap;

#ifndef GFX_HUD_h
#define GFX_HUD_h

extern "C" {
        #include <lua.h>
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "../ExternalTable.h"
#include "../lua_util.h"

#include "../path_util.h"
#include "../vect_util.h"

#include "GfxPipeline.h"

class GfxHudClass {

    public:

    GfxHudClass (lua_State *L, const std::string &name_)
          : name(name_), dir(grit_dirname(name_))
    {
        int index = lua_gettop(L);
        for (lua_pushnil(L) ; lua_next(L,index)!=0 ; lua_pop(L,1)) {
            const char *err = table.luaSet(L);
            if (err) my_lua_error(L, err);
        }
        lua_pop(L,1); // the table we just iterated through
    }

    void get (lua_State *L, const std::string &key)
    {
        const char *err = table.luaGet(L, key);
        if (err) my_lua_error(L,err);
    } 

    void set (lua_State *L, const std::string &key)
    {
        const char *err = table.luaSet(L, key);
        if (err) my_lua_error(L,err);
    }
    
    void set (lua_State *L)
    {   
        const char *err = table.luaSet(L);
        if (err) my_lua_error(L,err);
    }

    void dump (lua_State *L)
    {
        table.dump(L);
    }

    const std::string name;
    const std::string dir;

    const ExternalTable &getTable (void) const { return table; }
    ExternalTable &getTable (void) { return table; }

    protected:

    ExternalTable table;

};

GfxHudClass *gfx_hud_class_add (lua_State *L, const std::string& name);

GfxHudClass *gfx_hud_class_get (const std::string &name);
bool gfx_hud_class_has (const std::string &name);

void gfx_hud_class_all (GfxHudClassMap::iterator &begin, GfxHudClassMap::iterator &end);
size_t gfx_hud_class_count (void);



class GfxHudBase : public fast_erase_index {

    protected:

    bool dead;

    GfxHudObject *parent;

    unsigned char zOrder;

    Vector2 position, size;
    Radian orientation;
    bool enabled;

    GfxHudBase (void);

    public:

    virtual ~GfxHudBase (void);

    void destroy (void);

    void assertAlive (void);

    bool destroyed (void) { return dead; }

    void setEnabled (bool v) { assertAlive(); enabled = v; }
    bool isEnabled (void) { assertAlive(); return enabled; }

    void setOrientation (Radian v) { assertAlive(); orientation = v; }
    Radian getOrientation (void) { assertAlive(); return orientation; }

    void setPosition (const Vector2 &v) { assertAlive(); position = v; }
    Vector2 getPosition (void) { assertAlive(); return position; }

    void setSize (const Vector2 &v) { assertAlive(); size = v; }
    Vector2 getSize (void) { assertAlive(); return size; }

    unsigned char getZOrder (void) { assertAlive(); return zOrder; }
    void setZOrder (unsigned char v) { assertAlive(); zOrder = v; }
};

class GfxHudObject : public GfxHudBase {

    public:

    GfxHudClass * const hudClass;
    ExternalTable userValues;

    protected:
    std::vector<GfxHudBase*> children;

    GfxDiskResource *texture;
    Vector2 uv1, uv2;
    Vector3 colour;
    float alpha;

    public:

    GfxHudObject (GfxHudClass *hud_class)
      : GfxHudBase(), hudClass(hud_class), texture(NULL), uv1(0,1), uv2(1,0), colour(1,1,1), alpha(1), refCount(0)
    { }

    void incRefCount (void);
    void decRefCount (lua_State *L);
    void destroy (lua_State *L);

    void triggerInit (lua_State *L);
    void triggerParentResized (lua_State *L, const Vector2 &psize);
    void triggerDestroy (lua_State *L);


    float getAlpha (void) { assertAlive(); return alpha; }
    void setAlpha (float v) { assertAlive(); alpha = v; }
    
    Vector3 getColour (void) { assertAlive(); return colour; }
    void setColour (const Vector3 &v) { assertAlive(); colour = v; }
    
    Vector2 getUV1 (void) { assertAlive(); return uv1; }
    void setUV1 (const Vector2 &v) { assertAlive(); uv1 = v; }
    
    Vector2 getUV2 (void) { assertAlive(); return uv2; }
    void setUV2 (const Vector2 &v) { assertAlive(); uv2 = v; }

    GfxDiskResource *getTexture (void) { assertAlive(); return texture; }
    void setTexture (GfxDiskResource *v);


    private:

    unsigned refCount;
    
};

class GfxHudText : public GfxHudBase {

};

/** Called in the frame loop by the graphics code to render the HUD on top of the 3d graphics. */
void gfx_hud_render (Ogre::Viewport *vp);

/** Set up internal state. */
void gfx_hud_init (void);

/** Called as the game engine exits to clean up internal state. */
void gfx_hud_shutdown (void);

/** Notify the hud objects of a window resize. */
void gfx_hud_window_resized (unsigned w, unsigned h);

/** Call the parentResized callbacks on the whole tree of hud elements.  This
 * is to be called after a window resize, but before the gfx_hud_render call.
 * */
void gfx_hud_call_parent_resized (lua_State *L);

#endif
