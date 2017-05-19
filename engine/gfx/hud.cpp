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

#include <centralised_log.h>
#include "../path_util.h"

#include "lua_wrappers_gfx.h"
#include "hud.h"
#include "gfx_internal.h"
#include "gfx_shader.h"

static GfxShader *shader_text, *shader_rect, *shader_stencil;
static GfxShaderBindings shader_text_binds, empty_binds;

static Vector2 win_size(0,0);

// {{{ CLASSES

static HudClassMap classes;

HudClass *hud_class_add (lua_State *L, const std::string& name)
{
    HudClass *ghc;
    HudClassMap::iterator i = classes.find(name);
    
    if (i!=classes.end()) {
        ghc = i->second;
        int index = lua_gettop(L);
        for (lua_pushnil(L) ; lua_next(L,index)!=0 ; lua_pop(L,1)) {
            ghc->set(L);
        }
    } else {
        // add it and return it
        ghc = new HudClass(L, name);
        classes[name] = ghc;
    }           
    return ghc;         
}

HudClass *hud_class_get (const std::string &name)
{
    HudClassMap::iterator i = classes.find(name);
    if (i==classes.end())
        GRIT_EXCEPT("GfXHudClass does not exist: "+name);
    return i->second;   
}

bool hud_class_has (const std::string &name)
{
    return classes.find(name)!=classes.end();
}

void hud_class_all (HudClassMap::iterator &begin, HudClassMap::iterator &end)
{
    begin = classes.begin();
    end = classes.end();
}

size_t hud_class_count (void)
{
    return classes.size();
}

// }}}


// {{{ BASE
static fast_erase_vector<HudBase*> root_elements;

/** Get a std::vector of root_elemenets that are HudObjects, and also inc
 * their ref counts.
 *
 * A common operation is to iterate over the hud objects and call a Lua
 * callback on some/all of them.  Since Lua callbacks can trigger the Lua
 * garbage collector, and thus call gc metamethods on hud objects, and thus dec
 * the reference count of hud objects, we must be careful that 1) we are not
 * iterating over root_elements directly since it will change, and 2) we are
 * not iterating over a simple copy of root_elements since some of those
 * pointers will be free'd in the middle of the iteration.  To avoid this, we
 * increment the reference counters for all these objects before the iteration,
 * as well as using a copied std::vector.
 */
static std::vector<HudObject *> get_all_hud_objects (const fast_erase_vector<HudBase*> &bases)
{
    std::vector<HudObject*> r;
    for (unsigned i=0 ; i<bases.size() ; ++i) {
        HudObject *o = dynamic_cast<HudObject*>(bases[i]);
        if (o == NULL) continue;
        r.push_back(o);
        o->incRefCount();
    }
    return r;
}

/** The companion to get_all_hud_objects.  This simply decs the reference count
 * for all the objects.
 */
static void dec_all_hud_objects (lua_State *L, std::vector<HudObject *> objs)
{
    for (unsigned i=0 ; i<objs.size() ; ++i) {
        objs[i]->decRefCount(L);
    }
}


void HudBase::registerRemove (void)
{
    //CVERB << "Hud element unregistering its existence: " << this << std::endl;
    if (parent != NULL) {
        parent->notifyChildRemove(this);
    } else {
        root_elements.erase(this);
    }
}

void HudBase::registerAdd (void)
{
    //CVERB << "Hud element registering its existence: " << this << std::endl;
    if (parent != NULL) {
        parent->notifyChildAdd(this);
    } else {
        root_elements.push_back(this);
    }
}

HudBase::HudBase (void)
  : aliveness(ALIVE), parent(NULL), zOrder(3),
    position(0,0), orientation(0), inheritOrientation(true), enabled(true), snapPixels(true)
{
    //CVERB << "Hud element created: " << this << std::endl;
    registerAdd();
}

HudBase::~HudBase (void)
{
    if (aliveness==ALIVE) destroy();
    //CVERB << "Hud element deleted: " << this << std::endl;
}

void HudBase::destroy (void)
{
    if (aliveness==DEAD) return;
    registerRemove();
    aliveness = DEAD;
    //CVERB << "Hud element destroyed: " << this << std::endl;
}

void HudBase::assertAlive (void) const
{
    if (aliveness == DEAD)
        GRIT_EXCEPT("Hud element destroyed.");
}

Radian HudBase::getDerivedOrientation (void) const
{
    assertAlive();
    if (inheritOrientation && parent!=NULL) {
        return parent->getDerivedOrientation() + orientation;
    } else {
        return orientation;
    }
}

Vector2 HudBase::getDerivedPosition (void) const
{
    assertAlive();
    if (parent!=NULL) {
        return parent->getDerivedPosition() + position.rotateBy(parent->getDerivedOrientation());
    } else {
        return position;
    }
}

Vector2 HudBase::getBounds (void)
{
    assertAlive();
    float s = gritsin(orientation);
    float c = gritcos(orientation);
    Vector2 size = getSize();
    float w = (fabs(c)*size.x + fabs(s)*size.y);
    float h = (fabs(s)*size.x + fabs(c)*size.y);
    return Vector2(w,h);
}

Vector2 HudBase::getDerivedBounds (void)
{
    assertAlive();
    float s = gritsin(getDerivedOrientation());
    float c = gritcos(getDerivedOrientation());
    Vector2 size = getSize();
    float w = (fabs(c)*size.x + fabs(s)*size.y);
    float h = (fabs(s)*size.x + fabs(c)*size.y);
    return Vector2(w,h);
}

// }}}


// {{{ OBJECTS

HudObject::HudObject (HudClass *hud_class)
  : HudBase(), hudClass(hud_class),
    uv1(0,0), uv2(1,1), cornered(false), size(32,32), sizeSet(false), colour(1,1,1), alpha(1),
    stencil(false),
    needsResizedCallbacks(false), needsParentResizedCallbacks(false), needsInputCallbacks(false),
    needsFrameCallbacks(false), refCount(0)
{
    shader_rect->populateMatEnv(false, texs, shaderBinds, matEnvRect);
    shader_stencil->populateMatEnv(false, stencilTexs, empty_binds, matEnvStencil);
}

void HudObject::incRefCount (void)
{
    refCount++;
    //CVERB << "Inc ref count to "<<refCount<<": " << this << std::endl;
}

void HudObject::decRefCount (lua_State *L)
{
    refCount--;
    //CVERB << "Dec ref count to "<<refCount<<": " << this << std::endl;
    if (refCount == 0) {
        destroy(L);
        if (refCount == 0) {
            // the destroy callback may increase the refcount by adding new
            // alisaes, in which case the object is still destroyed, i.e.
            // will throw exceptions when it is used in any way, but
            // will not actually be deleted until the ref count does reach zero
            delete this;
        } else {
            //CVERB << "Not deleting yet since destroy callback kept dangling link: " << this << std::endl;
        }
    }
}

void HudObject::setColour (const Vector3 &v)
{
    assertAlive();
    colour = v;
    shaderBinds["colour"] = v;
}

void HudObject::setAlpha (float v)
{
    assertAlive();
    alpha = v;
    shaderBinds["alpha"] = GfxGslParam::float1(v);
}

void HudObject::destroy (void)
{
    if (aliveness != DEAD) {
        texture = nullptr;
        HudBase::destroy();
    }
}

void HudObject::destroy (lua_State *L)
{
    if (aliveness == ALIVE) {
        aliveness = DYING;
        triggerDestroy(L);
        while (children.size() != 0) {
            HudBase *child = children[0];
            child->destroy(L);
        }
        table.setNil(L);
        destroy();
    }
}

void HudObject::triggerInit (lua_State *L)
{
    assertAlive();

    STACK_BASE;
    //stack is empty

    // error handler in case there is a problem during 
    // the callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    //stack: err

    // get the function
    hudClass->get(L,"init");
    //stack: err,callback
    if (lua_isnil(L,-1)) {
        // no destroy callback -- our work is done
        lua_pop(L,2);
        STACK_CHECK;
        return;
    }


    //stack: err,callback
    STACK_CHECK_N(2);

    push_hudobj(L, this);
    //stack: err,callback,object

    STACK_CHECK_N(3);

    // call (1 arg), pops function too
    int status = lua_pcall(L,1,0,error_handler);
    if (status) {
        STACK_CHECK_N(2);
        //stack: err,error
        // pop the error message since the error handler will
        // have already printed it out
        lua_pop(L,1);
        //stack: err
        STACK_CHECK_N(1);
    } else {
        //stack: err
        STACK_CHECK_N(1);
    }

    //stack: err
    STACK_CHECK_N(1);
    lua_pop(L,1);

    //stack is empty
    STACK_CHECK;
}

void HudObject::triggerResized (lua_State *L)
{
    assertAlive();

    if (!needsResizedCallbacks) return;

    STACK_BASE;
    //stack is empty

    // error handler in case there is a problem during 
    // the callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    //stack: err

    // get the function
    hudClass->get(L,"resizedCallback");
    //stack: err,callback
    if (lua_isnil(L,-1)) {
        // no resized callback -- our work is done
        lua_pop(L,2);
        CERR << "Hud object of class: \"" << hudClass->name
             << "\" has no resizedCallback, disabling callback." << std::endl;
        needsResizedCallbacks = false;
        STACK_CHECK;
        return;
    }


    //stack: err,callback
    STACK_CHECK_N(2);

    Vector2 size = getSize();

    push_hudobj(L, this);
    push_v2(L, size);
    //stack: err,callback,object,size

    STACK_CHECK_N(4);

    // call (1 arg), pops function too
    int status = lua_pcall(L,2,0,error_handler);
    if (status) {
        STACK_CHECK_N(2);
        //stack: err,error
        // pop the error message since the error handler will
        // have already printed it out
        lua_pop(L,1);
        CERR << "Hud object of class: \"" << hudClass->name
             << "\" raised an error on resizedCallback, disabling callback." << std::endl;
        // will call destroy callback
        needsResizedCallbacks = false;
        //stack: err
        STACK_CHECK_N(1);
    } else {
        //stack: err
        STACK_CHECK_N(1);
    }

    //stack: err
    STACK_CHECK_N(1);
    lua_pop(L,1);

    //stack is empty
    STACK_CHECK;

}

void HudObject::triggerParentResized (lua_State *L)
{
    assertAlive();

    if (!needsParentResizedCallbacks) return;

    STACK_BASE;
    //stack is empty

    // error handler in case there is a problem during 
    // the callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    //stack: err

    // get the function
    hudClass->get(L,"parentResizedCallback");
    //stack: err,callback
    if (lua_isnil(L,-1)) {
        // no parentResized callback -- our work is done
        lua_pop(L,2);
        CERR << "Hud object of class: \"" << hudClass->name << "\" has no parentResizedCallback, disabling callback." << std::endl;
        needsParentResizedCallbacks = false;
        STACK_CHECK;
        return;
    }


    //stack: err,callback
    STACK_CHECK_N(2);

    Vector2 parent_size = parent==NULL ? win_size : parent->getSize();

    push_hudobj(L, this);
    push_v2(L, parent_size);
    //stack: err,callback,object,size

    STACK_CHECK_N(4);

    // call (1 arg), pops function too
    int status = lua_pcall(L,2,0,error_handler);
    if (status) {
        STACK_CHECK_N(2);
        //stack: err,error
        // pop the error message since the error handler will
        // have already printed it out
        lua_pop(L,1);
        CERR << "Hud object of class: \"" << hudClass->name << "\" raised an error on parentResizeCallback, disabling callback." << std::endl;
        // will call destroy callback
        needsParentResizedCallbacks = false;
        //stack: err
        STACK_CHECK_N(1);
    } else {
        //stack: err
        STACK_CHECK_N(1);
    }

    //stack: err
    STACK_CHECK_N(1);
    lua_pop(L,1);

    //stack is empty
    STACK_CHECK;

}

void HudObject::setSize (lua_State *L, const Vector2 &v)
{
    assertAlive();
    sizeSet = true;
    size = v;

    // use local_children copy since callbacks can alter hierarchy
    std::vector<HudObject*> local_children = get_all_hud_objects(children);
    for (unsigned j=0 ; j<local_children.size() ; ++j) {
        HudObject *obj = local_children[j];
        if (!obj->destroyed()) obj->triggerParentResized(L);
    }
    dec_all_hud_objects(L, local_children);

    // Run this last because there is a common pattern where a parent positions its child
    // according to the child's size, and the child's size is controlled in the child's
    // parentResizedCallback as a function of the parent size.  This happens for example in a
    // FileExplorer IconFlow.  To make this work, we let the child know about he size change before
    // we call the parent's resizedCallback, which can then position the child properly.
    triggerResized(L);

}

void HudObject::setParent (lua_State *L, HudObject *v)
{
    HudBase::setParent(v);

    triggerParentResized(L);
}

Vector2 HudObject::screenToLocal (const Vector2 &screen_pos)
{
    return (screen_pos - getDerivedPosition()).rotateBy(-getDerivedOrientation());
}

static HudObject *ray (const Vector2 &screen_pos)
{
    for (int i=GFX_HUD_ZORDER_MAX ; i>=0 ; --i) {
        for (unsigned j=0 ; j<root_elements.size() ; ++j) {
            HudBase *base = root_elements[j];
            if (base->destroyed()) continue;
            if (base->getZOrder() != i) continue;
            HudObject *obj = dynamic_cast<HudObject*>(base);
            if (obj == nullptr) continue;
            HudObject *hit = obj->shootRay(screen_pos);
            if (hit != NULL) return hit;
        }
    }

    return NULL;
}

HudObject *HudObject::shootRay (const Vector2 &screen_pos)
{
    if (!isEnabled()) return NULL; // can't hit any children either

    Vector2 local_pos = screenToLocal(screen_pos);
    bool inside = fabsf(local_pos.x) < getSize().x / 2
                && fabsf(local_pos.y) < getSize().y / 2;

    // children can still be hit, since they can be larger than parent, so do not return yet...
    //if (!inside) return NULL;
    
    // look at children, ensure not inside one of them
    for (int i=GFX_HUD_ZORDER_MAX ; i>=0 ; --i) {
        for (unsigned j=0 ; j<children.size() ; ++j) {
            HudBase *base = children[j];

            if (base->destroyed()) continue;
            if (base->getZOrder() != i) continue;

            HudObject *obj = dynamic_cast<HudObject*>(base);
            if (obj == NULL) continue;
            HudObject *hit = obj->shootRay(screen_pos);
            if (hit != NULL) return hit;
        }
    }
    
    // Child was not hit

    return getNeedsInputCallbacks() && inside ? this : NULL;
}

void HudObject::triggerMouseMove (lua_State *L, const Vector2 &screen_pos)
{
    assertAlive();

    if (!isEnabled()) return;

    do {

        if (!needsInputCallbacks) continue;

        STACK_BASE;
        //stack is empty

        // error handler in case there is a problem during 
        // the callback
        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // get the function
        hudClass->get(L,"mouseMoveCallback");
        //stack: err,callback
        if (lua_isnil(L,-1)) {
            // no parentResized callback -- our work is done
            lua_pop(L,2);
            STACK_CHECK;
            CERR << "Hud object of class: \"" << hudClass->name << "\" has no mouseMoveCallback function, disabling input callbacks." << std::endl;
            needsInputCallbacks = false;
            continue; // to children
        }


        //stack: err,callback
        STACK_CHECK_N(2);

        push_hudobj(L, this);
        Vector2 local_pos = screenToLocal(screen_pos);
        push_v2(L, local_pos);
        push_v2(L, screen_pos);
        HudObject *ray_hit_obj = ray(screen_pos);
        bool hit_us = ray_hit_obj == this;
        bool hit_child = false;
        for (HudObject *p = ray_hit_obj ; p != nullptr ; p = p->getParent()) {
            if (p == this) {
                hit_child = true;
                break;
            }
        }
        lua_pushboolean(L, hit_us);
        lua_pushboolean(L, hit_child);
        //stack: err,callback,object,local_pos,screen_pos,inside

        STACK_CHECK_N(7);

        // call (1 arg), pops function too
        int status = lua_pcall(L,5,0,error_handler);
        if (status) {
            STACK_CHECK_N(2);
            //stack: err,error
            // pop the error message since the error handler will
            // have already printed it out
            lua_pop(L,1);
            CERR << "Hud object of class: \"" << hudClass->name << "\" raised an error on mouseMoveCallback, disabling input callbacks." << std::endl;
            needsInputCallbacks = false;
            //stack: err
            STACK_CHECK_N(1);
        } else {
            //stack: err
            STACK_CHECK_N(1);
        }

        //stack: err
        STACK_CHECK_N(1);
        lua_pop(L,1);

        //stack is empty
        STACK_CHECK;

    } while (0);

    // note if we destroyed ourselves due to an error in the callback, children should be empty

    // use local_children copy since callbacks can alter hierarchy
    std::vector<HudObject*> local_children = get_all_hud_objects(children);
    for (unsigned j=0 ; j<local_children.size() ; ++j) {
        HudObject *obj = local_children[j];
        if (!obj->destroyed()) obj->triggerMouseMove(L, screen_pos);
    }
    dec_all_hud_objects(L, local_children);
}

void HudObject::triggerButton (lua_State *L, const std::string &name)
{
    assertAlive();

    if (!isEnabled()) return;

    do {
        if (!needsInputCallbacks) continue;

        STACK_BASE;
        //stack is empty

        // error handler in case there is a problem during 
        // the callback
        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // get the function
        hudClass->get(L,"buttonCallback");
        //stack: err,callback
        if (lua_isnil(L,-1)) {
            // no parentResized callback -- our work is done
            lua_pop(L,2);
            STACK_CHECK;
            CERR << "Hud object of class: \"" << hudClass->name << "\" has no buttonCallback function, disabling input callbacks." << std::endl;
            needsInputCallbacks = false;
            continue; // to children
        }


        //stack: err,callback
        STACK_CHECK_N(2);

        push_hudobj(L, this);
        push_string(L, name);
        //stack: err,callback,object,size

        STACK_CHECK_N(4);

        // call (2 args), pops function too
        int status = lua_pcall(L,2,0,error_handler);
        if (status) {
            STACK_CHECK_N(2);
            //stack: err,error
            // pop the error message since the error handler will
            // have already printed it out
            lua_pop(L,1);
            CERR << "Hud object of class: \"" << hudClass->name << "\" raised an error on buttonCallback, disabling input callbacks." << std::endl;
            needsInputCallbacks = false;
            //stack: err
            STACK_CHECK_N(1);
        } else {
            //stack: err
            STACK_CHECK_N(1);
        }

        //stack: err
        STACK_CHECK_N(1);
        lua_pop(L,1);

        //stack is empty
        STACK_CHECK;

    } while (0);

    // note if we destroyed ourselves due to an error in the callback, children should be empty

    // use local_children copy since callbacks can alter hierarchy
    std::vector<HudObject*> local_children = get_all_hud_objects(children);
    for (unsigned j=0 ; j<local_children.size() ; ++j) {
        HudObject *obj = local_children[j];
        if (!obj->destroyed()) obj->triggerButton(L, name);
    }
    dec_all_hud_objects(L, local_children);
}

void HudObject::triggerFrame (lua_State *L, float elapsed)
{
    assertAlive();

    if (!isEnabled()) return;

    do {
        if (!needsFrameCallbacks) continue;

        STACK_BASE;
        //stack is empty

        // error handler in case there is a problem during 
        // the callback
        push_cfunction(L, my_lua_error_handler);
        int error_handler = lua_gettop(L);

        //stack: err

        // get the function
        hudClass->get(L,"frameCallback");
        //stack: err,callback
        if (lua_isnil(L,-1)) {
            // no parentResized callback -- our work is done
            lua_pop(L,2);
            STACK_CHECK;
            CERR << "Hud object of class: \"" << hudClass->name << "\" has no frameCallback function, disabling frame callbacks." << std::endl;
            needsFrameCallbacks = false;
            continue; // to children
        }


        //stack: err,callback
        STACK_CHECK_N(2);

        push_hudobj(L, this);
        lua_pushnumber(L, elapsed);
        //stack: err,callback,object,size

        STACK_CHECK_N(4);

        // call (1 arg), pops function too
        int status = lua_pcall(L,2,0,error_handler);
        if (status) {
            STACK_CHECK_N(2);
            //stack: err,error
            // pop the error message since the error handler will
            // have already printed it out
            lua_pop(L,1);
            CERR << "Hud object of class: \"" << hudClass->name << "\" raised an error on frameCallback, disabling frame callbacks." << std::endl;
            needsFrameCallbacks = false;
            //stack: err
            STACK_CHECK_N(1);
        } else {
            //stack: err
            STACK_CHECK_N(1);
        }

        //stack: err
        STACK_CHECK_N(1);
        lua_pop(L,1);

        //stack is empty
        STACK_CHECK;

    } while (0);

    // note if we destroyed ourselves due to an error in the callback, children should be empty

    // use local_children copy since callbacks can alter hierarchy
    std::vector<HudObject*> local_children = get_all_hud_objects(children);
    for (unsigned j=0 ; j<local_children.size() ; ++j) {
        HudObject *obj = local_children[j];
        if (!obj->destroyed()) obj->triggerFrame(L, elapsed);
    }
    dec_all_hud_objects(L, local_children);
}

void HudObject::notifyChildAdd (HudBase *child)
{
    children.push_back(child);
}

void HudObject::notifyChildRemove (HudBase *child)
{
    children.erase(child);
}


void HudObject::triggerDestroy (lua_State *L)
{
    assertAlive();
    STACK_BASE;
    //stack is empty

    // error handler in case there is a problem during 
    // the callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    //stack: err

    // get the function
    hudClass->get(L,"destroy");
    //stack: err,callback
    if (lua_isnil(L,-1)) {
        // no destroy callback -- our work is done
        lua_pop(L,2);
        STACK_CHECK;
        return;
    }


    //stack: err,callback
    STACK_CHECK_N(2);

    push_hudobj(L, this);
    //stack: err,callback,object

    STACK_CHECK_N(3);

    // call (1 arg), pops function too
    int status = lua_pcall(L,1,0,error_handler);
    if (status) {
        STACK_CHECK_N(2);
        //stack: err,error
        // pop the error message since the error handler will
        // have already printed it out
        lua_pop(L,1);
        //stack: err
        STACK_CHECK_N(1);
    } else {
        //stack: err
        STACK_CHECK_N(1);
    }

    //stack: err
    STACK_CHECK_N(1);
    lua_pop(L,1);

    //stack is empty
    STACK_CHECK;
}

void HudObject::setTexture (const DiskResourcePtr<GfxTextureDiskResource> &v)
{
    assertAlive();
    if (v != nullptr) {
        if (!v->isLoaded()) v->load();
    }
    texture = v;
    texs.clear();
    if (v != nullptr)
        texs["tex"] = gfx_texture_state_anisotropic(&*v);
    shader_rect->populateMatEnv(false, texs, shaderBinds, matEnvRect);
}

void HudObject::setStencilTexture (const DiskResourcePtr<GfxTextureDiskResource> &v)
{
    assertAlive();
    if (v != nullptr) {
        if (!v->isLoaded()) v->load();
    }
    stencilTexture = v;
    stencilTexs.clear();
    if (v != nullptr)
        stencilTexs["tex"] = gfx_texture_state_anisotropic(&*v);
    shader_stencil->populateMatEnv(false, stencilTexs, empty_binds, matEnvStencil);

}

// }}}


// {{{ TEXT

HudText::HudText (GfxFont *font)
  : buf(font), colour(1,1,1), alpha(1),
    letterTopColour(1,1,1), letterTopAlpha(1),
    letterBottomColour(1,1,1), letterBottomAlpha(1),
    wrap(0,0), scroll(0),
    shadow(0,0), shadowColour(0,0,0), shadowAlpha(1),
    refCount(0)
{
    shaderBinds["colour"] = colour;
    shaderBinds["alpha"] = GfxGslParam::float1(alpha);

    shaderBindsShadow["colour"] = shadowColour;
    shaderBindsShadow["alpha"] = GfxGslParam::float1(shadowAlpha);

    GfxTextureDiskResource *tex = font->getTexture();
    texs["tex"] = gfx_texture_state_anisotropic(tex);

    shader_text->populateMatEnv(false, texs, shaderBinds, matEnv);
    shader_text->populateMatEnv(false, texs, shaderBindsShadow, matEnvShadow);
}

void HudText::setAlpha (float v)
{
    assertAlive();
    alpha = v;
    shaderBinds["alpha"] = GfxGslParam::float1(alpha);
}

void HudText::setColour (const Vector3 &v)
{
    assertAlive();
    colour = v;
    shaderBinds["colour"] = colour;
}

void HudText::setShadowColour (const Vector3 &v)
{
    assertAlive();
    shadowColour = v;
    shaderBindsShadow["colour"] = shadowColour;
}

void HudText::setShadowAlpha (float v)
{
    assertAlive();
    shadowAlpha = v;
    shaderBindsShadow["alpha"] = GfxGslParam::float1(shadowAlpha);
}

void HudText::incRefCount (void)
{
    refCount++;
}

void HudText::decRefCount (void)
{
    refCount--;
    if (refCount == 0) {
        destroy();
        delete this;
    }
}

void HudText::destroy (void)
{
    if (aliveness==ALIVE) {
        buf.clear();
        HudBase::destroy();
    }
}

void HudText::clear (void)
{
    assertAlive();
    buf.clear();
}
void HudText::append (const std::string &v)
{
    assertAlive();
    buf.addFormattedString(v, letterTopColour, letterTopAlpha, letterBottomColour, letterBottomAlpha);
}

std::string HudText::getText (void) const
{
    assertAlive();
    // TODO: support this, turn unicode cps into UTF8.
    //return buf.getText();
    return "";
}

// }}}




// {{{ RENDERING


GfxGslMeshEnvironment simple_mesh_env;

// vdata/idata be allocated later because constructor requires ogre to be initialised
static Ogre::VertexData *quad_vdata;
static Ogre::HardwareVertexBufferSharedPtr quad_vbuf;
static unsigned quad_vdecl_size;

// vdata/idata be allocated later because constructor requires ogre to be initialised
static Ogre::VertexData *cornered_quad_vdata;
static Ogre::IndexData *cornered_quad_idata;
static Ogre::HardwareVertexBufferSharedPtr cornered_quad_vbuf;
static Ogre::HardwareIndexBufferSharedPtr cornered_quad_ibuf;
static unsigned cornered_quad_vdecl_size;

void hud_init (void)
{
    win_size = Vector2(ogre_win->getWidth(), ogre_win->getHeight());

    // Prepare vertex buffers
    quad_vdata = OGRE_NEW Ogre::VertexData();
    quad_vdata->vertexStart = 0;
    quad_vdata->vertexCount = 6;
    quad_vdecl_size = 0;
    quad_vdecl_size += quad_vdata->vertexDeclaration->addElement(0, quad_vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_POSITION).getSize();
    quad_vdecl_size += quad_vdata->vertexDeclaration->addElement(0, quad_vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES,0).getSize();
    quad_vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            quad_vdecl_size, 6, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
    quad_vdata->vertexBufferBinding->setBinding(0, quad_vbuf);

    cornered_quad_vdata = OGRE_NEW Ogre::VertexData();
    cornered_quad_vdata->vertexStart = 0;
    cornered_quad_vdata->vertexCount = 16;
    cornered_quad_vdecl_size = 0;
    cornered_quad_vdecl_size += cornered_quad_vdata->vertexDeclaration->addElement(0, cornered_quad_vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_POSITION).getSize();
    cornered_quad_vdecl_size += cornered_quad_vdata->vertexDeclaration->addElement(0, cornered_quad_vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES,0).getSize();
    cornered_quad_vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            cornered_quad_vdecl_size, 16, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
    cornered_quad_vdata->vertexBufferBinding->setBinding(0, cornered_quad_vbuf);

    unsigned cornered_quad_indexes = 6*9;
    cornered_quad_ibuf =
        Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
            Ogre::HardwareIndexBuffer::IT_16BIT,
            cornered_quad_indexes,
            Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

    cornered_quad_idata = OGRE_NEW Ogre::IndexData();
    cornered_quad_idata->indexStart = 0;
    cornered_quad_idata->indexCount = cornered_quad_indexes;
    cornered_quad_idata->indexBuffer = cornered_quad_ibuf;

    /* c d e f
     * 8 9 a b
     * 4 5 6 7    d c
     * 0 1 2 3    a b
     */
    #define QUAD(a,b,c,d) a, b, d,  c, d, b
    uint16_t cornered_quad_idata_raw[] = {
        QUAD( 0, 1, 5, 4), QUAD( 1, 2, 6, 5), QUAD( 2, 3, 7, 6),
        QUAD( 4, 5, 9, 8), QUAD( 5, 6,10, 9), QUAD( 6, 7,11,10),
        QUAD( 8, 9,13,12), QUAD( 9,10,14,13), QUAD(10,11,15,14),
    };
    #undef QUAD

    cornered_quad_ibuf->writeData(
        0, cornered_quad_indexes*sizeof(uint16_t), &cornered_quad_idata_raw[0], true);

    GfxGslRunParams shader_rect_params = {
        {"colour", GfxGslParam::float3(1, 1, 1)},
        {"alpha", GfxGslParam::float1(1.0f)},
        {"tex", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)}
    };
    GfxGslRunParams shader_stencil_params = {
        {"tex", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)}
    };
    GfxGslRunParams shader_text_params = {
        {"colour", GfxGslParam::float3(1, 1, 1)},
        {"alpha", GfxGslParam::float1(1.0f)},
        {"tex", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1)}
    };

    std::string vertex_code =
        "out.position = transform_to_world(Float3(vert.position.xy, 0));\n";

    // Note this never discards, even if alpha == 0.  This is important because we use it
    // to populate the stencil buffer to mask children at the bounds of the object.
    std::string rect_colour_code =
        "var texel = sample(mat.tex, vert.coord0.xy);\n"
        "out.colour = texel.rgb * mat.colour;\n"
        "out.alpha = texel.a * mat.alpha;\n"
        "out.colour = out.colour * out.alpha;\n";

    std::string stencil_colour_code =
        "var texel = sample(mat.tex, vert.coord0.xy);\n"
        "if (texel.a < 0.5) discard;\n";

    std::string text_colour_code =
        "var texel = sample(mat.tex, vert.coord0.xy);\n"
        "out.colour = texel.rgb * vert.coord1.rgb * mat.colour;\n"
        "out.alpha = texel.a * vert.coord1.a * mat.alpha;\n"
        "out.colour = out.colour * out.alpha;\n";

    gfx_shader_check(
        "/system/HudRect", vertex_code, "", rect_colour_code, shader_rect_params, false);
    gfx_shader_check(
        "/system/HudStencil", vertex_code, "", stencil_colour_code, shader_stencil_params, false);
    gfx_shader_check(
        "/system/HudText", vertex_code, "", text_colour_code, shader_text_params, false);

    shader_rect = gfx_shader_make_or_reset(
        "/system/HudRect", vertex_code, "", rect_colour_code, shader_rect_params, false);

    shader_stencil = gfx_shader_make_or_reset(
        "/system/HudStencil", vertex_code, "", stencil_colour_code, shader_stencil_params, false);
    
    shader_text = gfx_shader_make_or_reset(
        "/system/HudText", vertex_code, "", text_colour_code, shader_text_params, false);

}

void hud_shutdown (lua_State *L)
{
    quad_vbuf.setNull();
    OGRE_DELETE quad_vdata;
    cornered_quad_vbuf.setNull();
    OGRE_DELETE cornered_quad_vdata;

    // Not all destroy callbacks actually destroy their children.  Orphaned
    // children end up being adopted by grandparents, and ultimately the root.  If
    // hud elements are left at the root until Lua state destruction, the gc
    // callbacks are called in an arbitrary order. That can cause undefined
    // behaviour on the c++ side (access of deleted objects).  So we aggressively
    // destroy all the root elements before the actual lua shutdown.  This causes
    // the callbacks to run in the correct order, and avoid segfaults.

    while (root_elements.size() > 0) {
        // First clean up all the non Lua ones.  This is the easy part.

        // Copy into an intermediate buffer, since calling destroy changes root_elements.
        std::vector<HudBase*> all_non_lua;
        for (unsigned j=0 ; j<root_elements.size() ; ++j) {
            HudBase *base = root_elements[j];
            if (dynamic_cast<HudObject*>(base) == NULL) all_non_lua.push_back(base);
        }
        for (unsigned j=0 ; j<all_non_lua.size() ; ++j) {
            all_non_lua[j]->destroy();
        }

        // Now kill the ones with lua callbacks.
        std::vector<HudObject*> local_root_objects = get_all_hud_objects(root_elements);
        for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
            HudObject *obj = local_root_objects[j];
            if (obj->destroyed()) continue;
            obj->destroy(L);
        }

        dec_all_hud_objects(L, local_root_objects);
    }
}

static inline Vector2 maybe_snap (bool snap, Vector2 pos)
{
    if (snap) {
        pos.x = ::floorf(pos.x);
        pos.y = ::floorf(pos.y);
    }
    return pos;
}

static void set_vertex_data (const Vector2 &size,
                             const Vector2 &uv1, const Vector2 &uv2)
{
    // strict alignment required here
    struct Vertex { Vector2 position; Vector2 uv; };

    float left   = -size.x / 2;
    float right  =  size.x / 2;
    float bottom = -size.y / 2;
    float top    =  size.y / 2;

    Vertex bottom_left = { Vector2(left, bottom), Vector2(uv1.x,uv2.y) };
    Vertex bottom_right = { Vector2(right, bottom), uv2 };
    Vertex top_left = { Vector2(left, top), uv1 };
    Vertex top_right = { Vector2(right, top), Vector2(uv2.x,uv1.y) };

    Vertex vdata_raw[] = {
        bottom_left, bottom_right, top_left,
        top_left, bottom_right, top_right
    };

    quad_vbuf->writeData(quad_vdata->vertexStart, quad_vdata->vertexCount*quad_vdecl_size,
                         vdata_raw, true);
}

static void set_cornered_vertex_data (GfxTextureDiskResource *tex,
                                      const Vector2 &size,
                                      const Vector2 &uv1, const Vector2 &uv2)
{
    // strict alignment required here
    struct Vertex { Vector2 position; Vector2 uv; };

    const Ogre::TexturePtr &texptr = tex->getOgreTexturePtr();
    texptr->load();
    const Vector2 whole_tex_size(texptr->getWidth(), texptr->getHeight());
    
    const Vector2 tex_size = whole_tex_size * Vector2(::fabsf(uv2.x-uv1.x), ::fabsf(uv2.y-uv1.y));
    const Vector2 uvm = (uv2+uv1)/2;

    const Vector2 diag0 = -size/2;
    const Vector2 diag3 = size/2;
    const Vector2 diag1 = diag0 + tex_size/2;
    const Vector2 diag2 = diag3 - tex_size/2;

    /* c d e f
     * 8 9 a b
     * 4 5 6 7
     * 0 1 2 3
     */

    Vertex vdata_raw[] = {
        { Vector2(diag0.x, diag0.y), Vector2(uv1.x, uv2.y) },
        { Vector2(diag1.x, diag0.y), Vector2(uvm.x, uv2.y) },
        { Vector2(diag2.x, diag0.y), Vector2(uvm.x, uv2.y) },
        { Vector2(diag3.x, diag0.y), Vector2(uv2.x, uv2.y) },

        { Vector2(diag0.x, diag1.y), Vector2(uv1.x, uvm.y) },
        { Vector2(diag1.x, diag1.y), Vector2(uvm.x, uvm.y) },
        { Vector2(diag2.x, diag1.y), Vector2(uvm.x, uvm.y) },
        { Vector2(diag3.x, diag1.y), Vector2(uv2.x, uvm.y) },

        { Vector2(diag0.x, diag2.y), Vector2(uv1.x, uvm.y) },
        { Vector2(diag1.x, diag2.y), Vector2(uvm.x, uvm.y) },
        { Vector2(diag2.x, diag2.y), Vector2(uvm.x, uvm.y) },
        { Vector2(diag3.x, diag2.y), Vector2(uv2.x, uvm.y) },

        { Vector2(diag0.x, diag3.y), Vector2(uv1.x, uv1.y) },
        { Vector2(diag1.x, diag3.y), Vector2(uvm.x, uv1.y) },
        { Vector2(diag2.x, diag3.y), Vector2(uvm.x, uv1.y) },
        { Vector2(diag3.x, diag3.y), Vector2(uv2.x, uv1.y) },
    };

    cornered_quad_vbuf->writeData(cornered_quad_vdata->vertexStart,
                                  cornered_quad_vdata->vertexCount*cornered_quad_vdecl_size,
                                  vdata_raw, true);
}

void gfx_render_hud_text (HudText *text, bool shadow, const Vector2 &offset,
                          int parent_stencil_ref)
{
    GfxFont *font = text->getFont();
    GfxTextureDiskResource *tex = font->getTexture();

    // In case the font has changed texture.
    // Note that we do not have to rebuild the GfxGasolineMaterialEnvironments because
    // they only care about whether a texture was bound, not which one.
    text->texs["tex"] = gfx_texture_state_anisotropic(tex);

    Vector2 pos = text->getDerivedPosition();
    if (text->snapPixels) {
        if (int(text->getDerivedBounds().x + 0.5) % 2 == 1)
            pos.x += 0.5f;
        if (int(text->getDerivedBounds().y + 0.5) % 2 == 1)
            pos.y += 0.5f;
        pos.x = ::floorf(pos.x);
        pos.y = ::floorf(pos.y);
        if (int(text->getDerivedBounds().x + 0.5) % 2 == 1)
            pos.x -= 0.5f;
        if (int(text->getDerivedBounds().y + 0.5) % 2 == 1)
            pos.y -= 0.5f;
    }
    pos += offset;

    const Ogre::Matrix4 &I = Ogre::Matrix4::IDENTITY;

    Ogre::Matrix4 matrix_centre = I;
    // move origin to center (for rotation)
    matrix_centre.setTrans(Ogre::Vector3(-text->getSize().x/2, text->getSize().y/2, 0));

    const Degree &orientation = text->getDerivedOrientation();
    Ogre::Matrix4 matrix_spin(Ogre::Quaternion(to_ogre(orientation), Ogre::Vector3(0,0,-1)));

    Ogre::Matrix4 matrix_trans = I;
    matrix_trans.setTrans(Ogre::Vector3(pos.x, pos.y, 0));

    Ogre::Matrix4 matrix_d3d_offset = I;
    if (d3d9) {
        // offsets for D3D rasterisation quirks, see http://msdn.microsoft.com/en-us/library/windows/desktop/bb219690(v=vs.85).aspx
        matrix_d3d_offset.setTrans(Ogre::Vector3(-0.5-win_size.x/2, 0.5-win_size.y/2, 0));
    } else {
        matrix_d3d_offset.setTrans(Ogre::Vector3(-win_size.x/2, -win_size.y/2, 0));
    }

    Ogre::Matrix4 matrix_scale = I;
    matrix_scale.setScale(Ogre::Vector3(2/win_size.x, 2/win_size.y ,1));

    // TODO: render target flipping (see comment in text rendering, above)
    bool render_target_flipping = false;
    Ogre::Matrix4 matrix = matrix_scale * matrix_d3d_offset * matrix_trans * matrix_spin * matrix_centre;

    Vector3 zv(0,0,0);
    GfxShaderGlobals globs = {
        zv, I, I, I, zv, zv, zv, zv, win_size, render_target_flipping, nullptr
    };

    if (shadow) {
        shader_text->bindShader(GFX_GSL_PURPOSE_HUD, text->matEnvShadow, simple_mesh_env,
                                globs, matrix, nullptr, 0, 1, text->texs, text->shaderBindsShadow);
    } else {
        shader_text->bindShader(GFX_GSL_PURPOSE_HUD, text->matEnv, simple_mesh_env,
                                globs, matrix, nullptr, 0, 1, text->texs, text->shaderBinds);
    }


    ogre_rs->setStencilBufferParams(Ogre::CMPF_EQUAL, parent_stencil_ref);


    if (text->buf.getRenderOperation().indexData->indexCount > 0) {
        ogre_rs->_render(text->buf.getRenderOperation());
    }

    ogre_rs->_disableTextureUnit(0);
}


void gfx_render_hud_one (HudBase *base, int parent_stencil_ref)
{
    if (!base->isEnabled()) return;
    if (base->destroyed()) return;

    HudObject *obj = dynamic_cast<HudObject*>(base);
    if (obj != nullptr) {

        bool is_cornered = obj->isCornered();

        Vector2 uv1 = obj->getUV1();
        Vector2 uv2 = obj->getUV2();

        Vector2 pos = obj->getDerivedPosition();
        if (obj->snapPixels) {
            if (int(obj->getDerivedBounds().x + 0.5) % 2 == 1)
                pos.x += 0.5f;
            if (int(obj->getDerivedBounds().y + 0.5) % 2 == 1)
                pos.y += 0.5f;
            pos.x = ::floorf(pos.x);
            pos.y = ::floorf(pos.y);
            if (int(obj->getDerivedBounds().x + 0.5) % 2 == 1)
                pos.x -= 0.5f;
            if (int(obj->getDerivedBounds().y + 0.5) % 2 == 1)
                pos.y -= 0.5f;
        }

        GfxTextureDiskResource *tex = obj->getTexture();
        if (is_cornered && tex != nullptr) {
            set_cornered_vertex_data(tex, obj->getSize(), uv1, uv2);
        } else {
            set_vertex_data(obj->getSize(), uv1, uv2);
        }

        const Degree &orientation = obj->getDerivedOrientation();
        Ogre::Matrix4 matrix_spin(Ogre::Quaternion(to_ogre(orientation), Ogre::Vector3(0,0,-1)));

        const Ogre::Matrix4 &I = Ogre::Matrix4::IDENTITY;

        Ogre::Matrix4 matrix_trans = I;
        matrix_trans.setTrans(Ogre::Vector3(pos.x, pos.y, 0));

        Ogre::Matrix4 matrix_d3d_offset = I;
        if (d3d9) {
            // offsets for D3D rasterisation quirks, see http://msdn.microsoft.com/en-us/library/windows/desktop/bb219690(v=vs.85).aspx
            matrix_d3d_offset.setTrans(Ogre::Vector3(-0.5-win_size.x/2, 0.5-win_size.y/2, 0));
        } else {
            matrix_d3d_offset.setTrans(Ogre::Vector3(-win_size.x/2, -win_size.y/2, 0));
        }

        Ogre::Matrix4 matrix_scale = I;
        matrix_scale.setScale(Ogre::Vector3(2/win_size.x, 2/win_size.y, 1));

        // TODO: Is there no render target flipping?
        // I guess we never rendered HUD to a texture on GL?
        bool render_target_flipping = false;
        Ogre::Matrix4 matrix = matrix_scale * matrix_d3d_offset * matrix_trans * matrix_spin;

        Vector3 zv(0,0,0);
        GfxShaderGlobals globs = { zv, I, I, I, zv, zv, zv, zv, win_size, render_target_flipping,
                                   nullptr };

        Ogre::RenderOperation op;
        if (is_cornered && tex != nullptr) {
            op.useIndexes = true;
            op.vertexData = cornered_quad_vdata;
            op.indexData = cornered_quad_idata;
        } else {
            op.useIndexes = false;
            op.vertexData = quad_vdata;
        }
        op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;

        shader_rect->bindShader(GFX_GSL_PURPOSE_HUD, obj->matEnvRect, simple_mesh_env,
                                globs, matrix, nullptr, 0, 1, obj->texs, obj->shaderBinds);

        // First only draw our colour if we fit inside our parent.
        ogre_rs->setStencilBufferParams(
            Ogre::CMPF_EQUAL, parent_stencil_ref, 0xffffffff, 0xffffffff,
            Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_KEEP);

        ogre_rs->_render(op);

        int child_stencil_ref = parent_stencil_ref;
        if (obj->isStencil()) {

            GfxTextureDiskResource *stencil_tex = obj->getStencilTexture();
            if (is_cornered && stencil_tex != nullptr) {
                set_cornered_vertex_data(stencil_tex, obj->getSize(), uv1, uv2);
            } else {
                set_vertex_data(obj->getSize(), uv1, uv2);
            }


            // Paint a rectangle in the stencil buffer to mask our children, but we
            // ourselves are still masked by our parent.
            child_stencil_ref += 1;

            ogre_rs->setStencilBufferParams(
                Ogre::CMPF_EQUAL, parent_stencil_ref, 0xffffffff, 0xffffffff,
                Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_INCREMENT);

            ogre_rs->_setColourBufferWriteEnabled(false, false, false, false);
            shader_stencil->bindShader(GFX_GSL_PURPOSE_HUD, obj->matEnvStencil, simple_mesh_env,
                                       globs, matrix, nullptr, 0, 1, obj->stencilTexs,
                                       empty_binds);

            ogre_rs->_render(op);
            ogre_rs->_setColourBufferWriteEnabled(true, true, true, true);
        }

        if (tex != NULL) {
            ogre_rs->_disableTextureUnit(0);
        }

        for (unsigned i=0 ; i<=GFX_HUD_ZORDER_MAX ; ++i) {
            for (unsigned j=0 ; j<obj->children.size() ; ++j) {
                // Draw in reverse order, for consistency with ray priority
                HudBase *child = obj->children[obj->children.size() - j - 1];
                if (child->getZOrder() != i) continue;
                gfx_render_hud_one(child, child_stencil_ref);
            }
        }

        if (obj->isStencil()) {
            ogre_rs->setStencilBufferParams(
                Ogre::CMPF_LESS_EQUAL, parent_stencil_ref, 0xffffffff, 0xffffffff,
                Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_REPLACE);

            ogre_rs->_setColourBufferWriteEnabled(false, false, false, false);
            shader_stencil->bindShader(GFX_GSL_PURPOSE_HUD, obj->matEnvStencil, simple_mesh_env,
                                       globs, matrix, nullptr, 0, 1, obj->stencilTexs,
                                       empty_binds);

            ogre_rs->_render(op);
            ogre_rs->_setColourBufferWriteEnabled(true, true, true, true);
        }
    }

    HudText *text = dynamic_cast<HudText*>(base);
    if (text != nullptr) {

        text->buf.updateGPU(text->wrap == Vector2(0, 0), text->scroll, text->scroll+text->wrap.y);

        if (text->getShadow() != Vector2(0, 0)) {
            gfx_render_hud_text(text, true, text->getShadow(), parent_stencil_ref);
        }
        gfx_render_hud_text(text, false, Vector2(0, 0), parent_stencil_ref);
    }
}

void hud_render (Ogre::Viewport *vp)
{
    ogre_rs->_setViewport(vp);

    ogre_rs->_beginFrame();


    ogre_rs->clearFrameBuffer(Ogre::FBT_STENCIL);

    ogre_rs->_setDepthBias(0, 0);
    ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
    ogre_rs->setStencilCheckEnabled(true);
    ogre_rs->_setCullingMode(Ogre::CULL_NONE);
    ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);
    ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

    try {

        for (unsigned i=0 ; i<=GFX_HUD_ZORDER_MAX ; ++i) {
            for (unsigned j=0 ; j<root_elements.size() ; ++j) {
                HudBase *el = root_elements[root_elements.size() - j - 1];
                if (el->getZOrder() != i) continue;
                gfx_render_hud_one(el, 0);
            }
        }

    } catch (const Exception &e) {
        CERR << "Rendering HUD, got: " << e << std::endl;
    } catch (const Ogre::Exception &e) {
        CERR << "Rendering HUD, got: " << e.getDescription() << std::endl;
    }

    ogre_rs->setStencilCheckEnabled(false);

    ogre_rs->_endFrame();

}

// }}}


// {{{ RESIZE

// when true, triggers resize callbacks at the root
static bool window_size_dirty;

void hud_signal_window_resized (unsigned w, unsigned h)
{
    Vector2 new_win_size = Vector2(float(w),float(h));
    if (win_size == new_win_size) return;
    win_size = new_win_size;
    window_size_dirty = true;
}

void hud_call_per_frame_callbacks (lua_State *L, float elapsed)
{

    std::vector<HudObject*> local_root_objects = get_all_hud_objects(root_elements);

    if (window_size_dirty) {
        window_size_dirty = false;
        for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
            HudObject *obj = local_root_objects[j];
            if (obj->destroyed()) continue;
            obj->triggerParentResized(L);
        }
    }

    for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
        HudObject *obj = local_root_objects[j];
        if (obj->destroyed()) continue;
        obj->triggerFrame(L, elapsed);
    }

    dec_all_hud_objects(L, local_root_objects);
}

// }}}


// {{{ INPUT

HudObject *hud_ray (int x, int y)
{
    Vector2 screen_pos(x,y);
    return ray(screen_pos);
}

static Vector2 last_mouse_abs;
void hud_signal_mouse_move (lua_State *L, const Vector2 &abs)
{
    // make local copy because callbacks can destroy elements
    std::vector<HudObject*> local_root_objects = get_all_hud_objects(root_elements);

    for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
        HudObject *obj = local_root_objects[j];
        if (obj->destroyed()) continue;
        obj->triggerMouseMove(L, abs);
    }

    dec_all_hud_objects(L, local_root_objects);
    last_mouse_abs = abs;
}

void hud_signal_button (lua_State *L, const std::string &key)
{
    // make local copy because callbacks can destroy elements
    std::vector<HudObject*> local_root_objects = get_all_hud_objects(root_elements);

    for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
        HudObject *obj = local_root_objects[j];
        if (obj->destroyed()) continue;
        obj->triggerMouseMove(L, last_mouse_abs);
        obj->triggerButton(L, key);
    }

    dec_all_hud_objects(L, local_root_objects);
}

void hud_signal_flush (lua_State *L)
{
    (void) L;
    // TODO: issue 'fake' key up for any keys that are still down.
}
// }}}
