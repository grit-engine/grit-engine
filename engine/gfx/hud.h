/* Copyright (c) The Grit Game Engine authors 2016
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

#include "../shared_ptr.h"

class HudBase;
class HudObject;
class HudText;

class HudClass;
typedef std::map<std::string,HudClass*> HudClassMap;

#ifndef GFX_HUD_h
#define GFX_HUD_h

extern "C" {
        #include <lua.h>
        #include <lauxlib.h>
        #include <lualib.h>
}

#include "../external_table.h"
#include "../grit_lua_util.h"
#include "../lua_ptr.h"
#include "../path_util.h"
#include "../vect_util.h"

#include "gfx_pipeline.h"
#include "gfx_font.h"
#include "gfx_shader.h"
#include "gfx_text_buffer.h"

#define GFX_HUD_ZORDER_MAX 15

class HudClass {

    public:

    HudClass (lua_State *L, const std::string &name_)
          : name(name_)
    {
        int index = lua_gettop(L);
        for (lua_pushnil(L) ; lua_next(L,index)!=0 ; lua_pop(L,1)) {
            const char *err = table.luaSet(L);
            if (err) my_lua_error(L, err);
        }
        lua_pop(L,1); // the table we just iterated through
    }

    void get (lua_State *L, lua_Number key)
    {
        const char *err = table.luaGet(L, key);
        if (err) my_lua_error(L,err);
    } 

    void set (lua_State *L, lua_Number key)
    {
        const char *err = table.luaSet(L, key);
        if (err) my_lua_error(L,err);
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
    
    void get (lua_State *L)
    {   
        const char *err = table.luaGet(L);
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

    const ExternalTable &getTable (void) const { return table; }
    ExternalTable &getTable (void) { return table; }

    protected:

    ExternalTable table;

};

HudClass *hud_class_add (lua_State *L, const std::string& name);

HudClass *hud_class_get (const std::string &name);
bool hud_class_has (const std::string &name);

void hud_class_all (HudClassMap::iterator &begin, HudClassMap::iterator &end);
size_t hud_class_count (void);



class HudBase : public fast_erase_index {

    protected:

    enum Aliveness { ALIVE, DYING, DEAD };

    Aliveness aliveness;

    HudObject *parent;

    unsigned char zOrder;

    Vector2 position;
    Radian orientation;
    bool inheritOrientation;
    bool enabled;

    public:
    bool snapPixels;
    protected:

    HudBase (void);

    // maintain tree structure
    void registerRemove (void);
    void registerAdd (void);

    public:

    virtual ~HudBase (void);

    virtual void destroy (void);
    virtual void destroy (lua_State *) { destroy(); }

    void assertAlive (void) const;

    bool destroyed (void) { return aliveness == DEAD; }

    void setEnabled (bool v) { assertAlive(); registerRemove() ; enabled = v; registerAdd(); }
    bool isEnabled (void) { assertAlive(); return enabled; }

    void setInheritOrientation (bool v) { assertAlive(); inheritOrientation = v; }
    bool getInheritOrientation (void) const { assertAlive(); return inheritOrientation; } 
    void setOrientation (Radian v) { assertAlive(); orientation = v; }
    Radian getOrientation (void) const { assertAlive(); return orientation; }
    Radian getDerivedOrientation (void) const;

    void setPosition (const Vector2 &v) { assertAlive(); position = v; }
    Vector2 getPosition (void) const { assertAlive(); return position; }
    Vector2 getDerivedPosition (void) const;

    /** Cannot make getSize const because would need to refactor the drawing
     * code out of HudText.  In other words, cannot know the size without
     * drawing the text, which requires modifying buffers.
     */
    virtual Vector2 getSize (void) = 0;

    Vector2 getBounds (void);

    Vector2 getDerivedBounds (void);

    void setParent (HudObject *v) { assertAlive(); registerRemove(); parent = v; registerAdd(); }
    HudObject *getParent (void) const { assertAlive(); return parent; }

    void setZOrder (unsigned char v) { assertAlive(); registerRemove(); zOrder = v; registerAdd(); }
    unsigned char getZOrder (void) const { assertAlive(); return zOrder; }

};

class HudObject : public HudBase {

    public:

    HudClass * const hudClass;
    LuaPtr table;

    protected:
    fast_erase_vector<HudBase*> children;

    DiskResourcePtr<GfxTextureDiskResource> texture;
    GfxShaderBindings shaderBinds;
    GfxTextureStateMap texs;
    GfxTextureStateMap stencilTexs;

    GfxGslMaterialEnvironment matEnvRect, matEnvStencil;
    Vector2 uv1, uv2;
    bool cornered;
    Vector2 size;
    bool sizeSet;
    Vector3 colour;
    float alpha;
    bool stencil;
    // If no texture, children will be visible only within the rect.
    // If texture, children will be visible only where texture alpha > 0.5
    DiskResourcePtr<GfxTextureDiskResource> stencilTexture;

    bool needsResizedCallbacks;
    bool needsParentResizedCallbacks;
    bool needsInputCallbacks;
    bool needsFrameCallbacks;

    public:

    HudObject (HudClass *hud_class);

    void incRefCount (void);
    void decRefCount (lua_State *L);
    // do as much as we can without using L
    void destroy (void);
    // destroy with L (callbacks etc)
    void destroy (lua_State *L);

    void triggerInit (lua_State *L);
    void triggerResized (lua_State *L);
    void triggerParentResized (lua_State *L);
    void triggerMouseMove (lua_State *L, const Vector2 &abs);
    void triggerButton (lua_State *L, const std::string &key);
    void triggerFrame (lua_State *L, float elapsed);
    void triggerDestroy (lua_State *L);

    void notifyChildRemove (HudBase *child);
    void notifyChildAdd (HudBase *child);

    float getAlpha (void) const { assertAlive(); return alpha; }
    void setAlpha (float v);
    
    Vector3 getColour (void) const { assertAlive(); return colour; }
    void setColour (const Vector3 &v);
    
    Vector2 getUV1 (void) const { assertAlive(); return uv1; }
    void setUV1 (const Vector2 &v) { assertAlive(); uv1 = v; }
    
    Vector2 getUV2 (void) const { assertAlive(); return uv2; }
    void setUV2 (const Vector2 &v) { assertAlive(); uv2 = v; }

    bool isCornered (void) const { assertAlive(); return cornered; }
    void setCornered (bool v) { assertAlive(); cornered = v; }

    GfxTextureDiskResource *getTexture (void) const { assertAlive(); return &*texture; }
    void setTexture (const DiskResourcePtr<GfxTextureDiskResource> &v);

    bool isStencil (void) const { assertAlive(); return stencil; }
    void setStencil (bool v) { assertAlive(); stencil = v; }

    GfxTextureDiskResource *getStencilTexture (void) const
    { assertAlive(); return &*stencilTexture; }
    void setStencilTexture (const DiskResourcePtr<GfxTextureDiskResource> &v);

    void setSize (lua_State *L, const Vector2 &v);
    Vector2 getSize (void) { assertAlive(); return size; }
    bool getSizeSet (void) { assertAlive(); return sizeSet; }

    void setParent (lua_State *L, HudObject *v);

    bool getNeedsResizedCallbacks (void) const { assertAlive(); return needsResizedCallbacks; }
    void setNeedsResizedCallbacks (bool v) { assertAlive(); needsResizedCallbacks = v; }

    bool getNeedsParentResizedCallbacks (void) const
    { assertAlive(); return needsParentResizedCallbacks; }
    void setNeedsParentResizedCallbacks (bool v)
    { assertAlive(); needsParentResizedCallbacks = v; }

    bool getNeedsInputCallbacks (void) const { assertAlive(); return needsInputCallbacks; }
    void setNeedsInputCallbacks (bool v) { assertAlive(); needsInputCallbacks = v; }

    bool getNeedsFrameCallbacks (void) const { assertAlive(); return needsFrameCallbacks; }
    void setNeedsFrameCallbacks (bool v) { assertAlive(); needsFrameCallbacks = v; }

    // Transform screen co-ordinates (e.g. from the mouse) to co-ordinates local to the HudObject.
    Vector2 screenToLocal (const Vector2 &screen_pos);

    /** Return NULL for a miss, otherwise return the object hit, which can be a child.  Must have needsInputCallbacks otherwise ignored. */
    HudObject *shootRay (const Vector2 &screen_pos);

    private:

    unsigned refCount;

    // internal function
    friend void gfx_render_hud_one (HudBase *, int);
    friend void gfx_render_hud_text (HudText *, const Vector3 &, const Vector2 &, int);
    
};

class HudText : public HudBase {

    public:

    protected:
    GfxTextBuffer buf;
    Vector3 colour;
    float alpha;
    Vector3 letterTopColour;
    float letterTopAlpha;
    Vector3 letterBottomColour;
    float letterBottomAlpha;
    Vector2 wrap;
    long scroll;
    Vector2 shadow;
    Vector3 shadowColour;
    float shadowAlpha;
    GfxShaderBindings shaderBinds, shaderBindsShadow;
    GfxTextureStateMap texs;
    GfxGslMaterialEnvironment matEnv, matEnvShadow;


    unsigned refCount;

    public:

    HudText (GfxFont *font);

    ~HudText (void) { }

    void incRefCount (void);
    void decRefCount (void);
    void destroy (void);
    void destroy (lua_State *) { destroy(); }

    void clear (void);
    void append (const std::string &v);

    void setFont (GfxFont *v) { assertAlive() ; buf.setFont(v); }
    GfxFont *getFont (void) { assertAlive() ; return buf.getFont(); }

    std::string getText (void) const;

    long getScroll (void)
    {
        assertAlive();
        return scroll;
    }

    void setScroll (long v)
    {
        assertAlive();
        scroll = v;
    }

    void setTextWrap (const Vector2 &v)
    {
        assertAlive();
        wrap = v;
        buf.setWrap(wrap.x);
    }
    Vector2 getTextWrap (void) const {
        assertAlive();
        return wrap;
    }

    Vector2 getSize (void)
    {
        assertAlive();
        if (wrap == Vector2(0,0)) {
            buf.updateGPU(true, scroll, scroll+wrap.y);
            return buf.getDrawnDimensions();
        }
        return wrap;
    }

    unsigned long getBufferHeight (void) const
    {
        assertAlive();
        return buf.getBufferHeight();
    }

    float getAlpha (void) const { assertAlive(); return alpha; }
    void setAlpha (float v);
    
    Vector3 getColour (void) const { assertAlive(); return colour; }
    void setColour (const Vector3 &v);

    Vector2 getShadow (void) const { assertAlive(); return shadow; }
    void setShadow (const Vector2 &v) { assertAlive(); shadow = v; }

    Vector3 getShadowColour (void) const { assertAlive(); return shadowColour; }
    void setShadowColour (const Vector3 &v);

    float getShadowAlpha (void) const { assertAlive(); return shadowAlpha; }
    void setShadowAlpha (float v);

    Vector3 getLetterTopColour (void) const { assertAlive(); return letterTopColour; }
    void setLetterTopColour (const Vector3 &v) { assertAlive(); letterTopColour = v; }
    Vector3 getLetterBottomColour (void) const { assertAlive(); return letterBottomColour; }
    void setLetterBottomColour (const Vector3 &v) { assertAlive(); letterBottomColour = v; }
    float getLetterTopAlpha (void) const { assertAlive(); return letterTopAlpha; }
    void setLetterTopAlpha (float v) { assertAlive(); letterTopAlpha = v; }
    float getLetterBottomAlpha (void) const { assertAlive(); return letterBottomAlpha; }
    void setLetterBottomAlpha (float v) { assertAlive(); letterBottomAlpha = v; }
    

    // internal function
    friend void gfx_render_hud_one (HudBase *, int);
    friend void gfx_render_hud_text (HudText *, bool, const Vector2 &, int);
};

/** Called in the frame loop by the graphics code to render the HUD on top of the 3d graphics. */
void hud_render (Ogre::Viewport *vp);

/** Set up internal state. */
void hud_init (void);

/** Called as the game engine exits to clean up internal state. */
void hud_shutdown (lua_State *L);

/** Return the hud element at the given coordinates. */
HudObject *hud_ray (int x, int y);

/** Notify the hud system of the mouse location (called on a mouse move event). */
void hud_signal_mouse_move (lua_State *L, const Vector2 &abs);

/** Notify the hud system of a mouse button event. */
void hud_signal_button (lua_State *L, const std::string &key);

/** Notify the hud objects of a window resize. */
void hud_signal_window_resized (unsigned w, unsigned h);

/** Notify the hud objects that they will not be getting key presses for a while. */
void hud_signal_flush (lua_State *L);

/** To be called just before gfx_render, to notify hud objects of any parent
 * resize events that may be pending.  The point is that this function has a lua state param, whereas the gfx_render call does not.
 */
void hud_call_per_frame_callbacks (lua_State *L, float elapsed);

#endif
