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

#include <OgreHighLevelGpuProgramManager.h>
#include <OgreCgProgram.h>
#include <OgreGLSLProgram.h>

#include "../centralised_log.h"
#include "../path_util.h"

#include "lua_wrappers_gfx.h"
#include "gfx_hud.h"
#include "gfx_internal.h"
#include "gfx_shader.h"

static Vector2 win_size(0,0);

// {{{ CLASSES

static GfxHudClassMap classes;

GfxHudClass *gfx_hud_class_add (lua_State *L, const std::string& name)
{
    GfxHudClass *ghc;
    GfxHudClassMap::iterator i = classes.find(name);
    
    if (i!=classes.end()) {
        ghc = i->second;
        int index = lua_gettop(L);
        for (lua_pushnil(L) ; lua_next(L,index)!=0 ; lua_pop(L,1)) {
            ghc->set(L);
        }
    } else {
        // add it and return it
        ghc = new GfxHudClass(L,name);
        classes[name] = ghc;
    }           
    return ghc;         
}

GfxHudClass *gfx_hud_class_get (const std::string &name)
{
    GfxHudClassMap::iterator i = classes.find(name);
    if (i==classes.end())
        GRIT_EXCEPT("GfXHudClass does not exist: "+name);
    return i->second;   
}

bool gfx_hud_class_has (const std::string &name)
{
    return classes.find(name)!=classes.end();
}

void gfx_hud_class_all (GfxHudClassMap::iterator &begin, GfxHudClassMap::iterator &end)
{
    begin = classes.begin();
    end = classes.end();
}

size_t gfx_hud_class_count (void)
{
    return classes.size();
}

// }}}


// {{{ BASE
static fast_erase_vector<GfxHudBase*> root_elements;

/** Get a std::vector of root_elemenets that are GfxHudObjects, and also inc
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
static std::vector<GfxHudObject *> get_all_hud_objects (const fast_erase_vector<GfxHudBase*> &bases)
{
    std::vector<GfxHudObject*> r;
    for (unsigned i=0 ; i<bases.size() ; ++i) {
        GfxHudObject *o = dynamic_cast<GfxHudObject*>(bases[i]);
        if (o == NULL) continue;
        r.push_back(o);
        o->incRefCount();
    }
    return r;
}

/** The companion to get_all_hud_objects.  This simply decs the reference count
 * for all the objects.
 */
static void dec_all_hud_objects (lua_State *L, std::vector<GfxHudObject *> objs)
{
    for (unsigned i=0 ; i<objs.size() ; ++i) {
        objs[i]->decRefCount(L);
    }
}


void GfxHudBase::registerRemove (void)
{
    //CVERB << "GfxHud element unregistering its existence: " << this << std::endl;
    if (parent != NULL) {
        parent->notifyChildRemove(this);
    } else {
        root_elements.erase(this);
    }
}

void GfxHudBase::registerAdd (void)
{
    //CVERB << "GfxHud element registering its existence: " << this << std::endl;
    if (parent != NULL) {
        parent->notifyChildAdd(this);
    } else {
        root_elements.push_back(this);
    }
}

GfxHudBase::GfxHudBase (void)
  : aliveness(ALIVE), parent(NULL), zOrder(3),
    position(0,0), orientation(0), inheritOrientation(true), enabled(true), snapPixels(true)
{
    //CVERB << "GfxHud element created: " << this << std::endl;
    registerAdd();
}

GfxHudBase::~GfxHudBase (void)
{
    if (aliveness==ALIVE) destroy();
    //CVERB << "GfxHud element deleted: " << this << std::endl;
}

void GfxHudBase::destroy (void)
{
    if (aliveness==DEAD) return;
    registerRemove();
    aliveness = DEAD;
    //CVERB << "GfxHud element destroyed: " << this << std::endl;
}

void GfxHudBase::assertAlive (void) const
{
    if (aliveness == DEAD)
        GRIT_EXCEPT("Hud element destroyed.");
}

Radian GfxHudBase::getDerivedOrientation (void) const
{
    assertAlive();
    if (inheritOrientation && parent!=NULL) {
        return parent->getDerivedOrientation() + orientation;
    } else {
        return orientation;
    }
}

Vector2 GfxHudBase::getDerivedPosition (void) const
{
    assertAlive();
    if (parent!=NULL) {
        return parent->getDerivedPosition() + position.rotateBy(parent->getDerivedOrientation());
    } else {
        return position;
    }
}

Vector2 GfxHudBase::getBounds (void)
{
    assertAlive();
    float s = gritsin(orientation);
    float c = gritcos(orientation);
    Vector2 size = getSize();
    float w = (fabs(c)*size.x + fabs(s)*size.y);
    float h = (fabs(s)*size.x + fabs(c)*size.y);
    return Vector2(w,h);
}

Vector2 GfxHudBase::getDerivedBounds (void)
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

GfxHudObject::GfxHudObject (GfxHudClass *hud_class)
  : GfxHudBase(), hudClass(hud_class),
    uv1(0,0), uv2(1,1), cornered(false), size(32,32), sizeSet(false), colour(1,1,1), alpha(1),
    needsParentResizedCallbacks(false), needsInputCallbacks(false), needsFrameCallbacks(false),
    refCount(0)
{
}

void GfxHudObject::incRefCount (void)
{
    refCount++;
    //CVERB << "Inc ref count to "<<refCount<<": " << this << std::endl;
}

void GfxHudObject::decRefCount (lua_State *L)
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


void GfxHudObject::destroy (void)
{
    if (aliveness != DEAD) {
        texture = nullptr;
        GfxHudBase::destroy();
    }
}

void GfxHudObject::destroy (lua_State *L)
{
    if (aliveness == ALIVE) {
        aliveness = DYING;
        triggerDestroy(L);
        while (children.size() != 0) {
            GfxHudBase *child = children[0];
            child->destroy(L);
        }
        table.setNil(L);
        destroy();
    }
}

void GfxHudObject::triggerInit (lua_State *L)
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

    push_gfxhudobj(L, this);
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

void GfxHudObject::triggerParentResized (lua_State *L)
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

    push_gfxhudobj(L, this);
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

void GfxHudObject::setSize (lua_State *L, const Vector2 &v)
{
    assertAlive();
    sizeSet = true;
    size = v;

    // use local_children copy since callbacks can alter hierarchy
    std::vector<GfxHudObject*> local_children = get_all_hud_objects(children);
    for (unsigned j=0 ; j<local_children.size() ; ++j) {
        GfxHudObject *obj = local_children[j];
        if (!obj->destroyed()) obj->triggerParentResized(L);
    }
    dec_all_hud_objects(L, local_children);
}

void GfxHudObject::setParent (lua_State *L, GfxHudObject *v)
{
    GfxHudBase::setParent(v);

    triggerParentResized(L);
}

GfxHudObject *GfxHudObject::shootRay (const Vector2 &screen_pos)
{
    if (!isEnabled()) return NULL; // can't hit any children either

    Vector2 local_pos = (screen_pos - getDerivedPosition()).rotateBy(-getDerivedOrientation());
    bool inside = fabsf(local_pos.x) < getSize().x / 2
                && fabsf(local_pos.y) < getSize().y / 2;

    // children can still be hit, since they can be larger than parent, so do not return yet...
    //if (!inside) return NULL;
    
    // look at children, ensure not inside one of them
    for (unsigned j=0 ; j<children.size() ; ++j) {
        GfxHudBase *base = children[j];

        if (base->destroyed()) continue;

        GfxHudObject *obj = dynamic_cast<GfxHudObject*>(base);
        if (obj == NULL) continue;
        GfxHudObject *hit = obj->shootRay(screen_pos);
        if (hit != NULL) return hit;
    }
    
    // TODO: look at parent's z order > this one

    return getNeedsInputCallbacks() && inside ? this : NULL;
}

void GfxHudObject::triggerMouseMove (lua_State *L, const Vector2 &screen_pos)
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

        push_gfxhudobj(L, this);
        Vector2 local_pos = (screen_pos - getDerivedPosition()).rotateBy(-getDerivedOrientation());
        push_v2(L, local_pos);
        push_v2(L, screen_pos);
        lua_pushboolean(L, this->shootRay(screen_pos)==this);
        //stack: err,callback,object,local_pos,screen_pos,inside

        STACK_CHECK_N(6);

        // call (1 arg), pops function too
        int status = lua_pcall(L,4,0,error_handler);
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
    std::vector<GfxHudObject*> local_children = get_all_hud_objects(children);
    for (unsigned j=0 ; j<local_children.size() ; ++j) {
        GfxHudObject *obj = local_children[j];
        if (!obj->destroyed()) obj->triggerMouseMove(L, screen_pos);
    }
    dec_all_hud_objects(L, local_children);
}

void GfxHudObject::triggerButton (lua_State *L, const std::string &name)
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

        push_gfxhudobj(L, this);
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
    std::vector<GfxHudObject*> local_children = get_all_hud_objects(children);
    for (unsigned j=0 ; j<local_children.size() ; ++j) {
        GfxHudObject *obj = local_children[j];
        if (!obj->destroyed()) obj->triggerButton(L, name);
    }
    dec_all_hud_objects(L, local_children);
}

void GfxHudObject::triggerFrame (lua_State *L, float elapsed)
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

        push_gfxhudobj(L, this);
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
    std::vector<GfxHudObject*> local_children = get_all_hud_objects(children);
    for (unsigned j=0 ; j<local_children.size() ; ++j) {
        GfxHudObject *obj = local_children[j];
        if (!obj->destroyed()) obj->triggerFrame(L, elapsed);
    }
    dec_all_hud_objects(L, local_children);
}

void GfxHudObject::notifyChildAdd (GfxHudBase *child)
{
    children.push_back(child);
}

void GfxHudObject::notifyChildRemove (GfxHudBase *child)
{
    children.erase(child);
}


void GfxHudObject::triggerDestroy (lua_State *L)
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

    push_gfxhudobj(L, this);
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

void GfxHudObject::setTexture (const DiskResourcePtr<GfxTextureDiskResource> &v)
{
    assertAlive();
    if (v != nullptr) {
        if (!v->isLoaded()) v->load();
    }
    texture = v;
}

// }}}


// {{{ TEXT

void GfxHudText::incRefCount (void)
{
    refCount++;
}

void GfxHudText::decRefCount (void)
{
    refCount--;
    if (refCount == 0) {
        destroy();
        delete this;
    }
}

void GfxHudText::destroy (void)
{
    if (aliveness==ALIVE) {
        buf.clear();
        GfxHudBase::destroy();
    }
}

void GfxHudText::clear (void)
{
    assertAlive();
    buf.clear();
}
void GfxHudText::append (const std::string &v)
{
    assertAlive();
    buf.addFormattedString(v, letterTopColour, letterTopAlpha, letterBottomColour, letterBottomAlpha);
}

std::string GfxHudText::getText (void) const
{
    assertAlive();
    // TODO: support this, turn unicode cps into UTF8.
    //return buf.getText();
    return "";
}
// }}}




// {{{ RENDERING


GfxShader *shader_text, *shader_tex, *shader_solid;
GfxShaderBindingsPtr shader_text_binds, shader_tex_binds, shader_solid_binds;

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

enum VertOrFrag { VERT, FRAG };

static Ogre::HighLevelGpuProgramPtr make_cg_shader (const std::string &name, VertOrFrag kind, const std::string &code)
{
    CVERB << "Compiling HUD CG shader: " << name << std::endl;
    Ogre::StringVector vp_profs, fp_profs;
    vp_profs.push_back("vs_3_0"); fp_profs.push_back("ps_3_0"); // d3d9
    vp_profs.push_back("gpu_vp"); fp_profs.push_back("gp4fp"); // gl

    auto kind2 = kind==FRAG ? Ogre::GPT_FRAGMENT_PROGRAM : Ogre::GPT_VERTEX_PROGRAM;
    Ogre::HighLevelGpuProgramPtr prog = Ogre::HighLevelGpuProgramManager::getSingleton()
        .createProgram(name, RESGRP, "cg", kind2);
    APP_ASSERT(!prog.isNull());
    Ogre::CgProgram *tmp_prog = static_cast<Ogre::CgProgram*>(&*prog);
    prog->setSource(code);
    tmp_prog->setEntryPoint("main");
    tmp_prog->setProfiles(kind==FRAG ? fp_profs : vp_profs);
    tmp_prog->setCompileArguments("-I. -O3");
    prog->unload();
    prog->load();
    return prog;
}

static Ogre::HighLevelGpuProgramPtr make_glsl_shader (const std::string &name, VertOrFrag kind, const std::string &code)
{
    CVERB << "Compiling HUD GLSL shader: " << name << std::endl;

    auto kind2 = kind==FRAG ? Ogre::GPT_FRAGMENT_PROGRAM : Ogre::GPT_VERTEX_PROGRAM;
    Ogre::HighLevelGpuProgramPtr prog = Ogre::HighLevelGpuProgramManager::getSingleton()
        .createProgram(name, RESGRP, "glsl", kind2);
    APP_ASSERT(!prog.isNull());
    std::string uniforms;
    if (kind == VERT) {
        uniforms += "uniform mat4 global_worldViewProj;\n";
        uniforms += "in vec4 vertex;\n";
        uniforms += "in vec4 uv0;\n";
        uniforms += "in vec4 uv1;\n";
        uniforms += "out vec4 trans0;\n";
        uniforms += "out vec4 trans1;\n";
    } else {
        uniforms += "uniform sampler2D mat_tex;\n";
        uniforms += "uniform vec3 mat_colour;\n";
        uniforms += "uniform float mat_alpha;\n";
        uniforms += "in vec4 trans0;\n";
        uniforms += "in vec4 trans1;\n";
        uniforms += "out vec4 frag_colour;\n";
    }
    std::string code2 = "#version 130\n" + uniforms + code;
    prog->setSource(code2);
    prog->unload();
    prog->load();


    return prog;
}

void gfx_hud_init (void)
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

    cornered_quad_ibuf->writeData(0, cornered_quad_indexes*sizeof(uint16_t), &cornered_quad_idata_raw[0], true);

    // Initialise vertex program
    GfxShaderParamMap shader_solid_params = {
        {"colour", GfxShaderParam(Vector3(1, 1, 1))},
        {"alpha", GfxShaderParam(1.0f)},
    };

    GfxShaderParamMap shader_tex_params = {
        {"colour", GfxShaderParam(Vector3(1, 1, 1))},
        {"alpha", GfxShaderParam(1.0f)},
        {"tex", GfxShaderParam(GFX_GSL_FLOAT_TEXTURE2, Vector4(1, 1, 1, 1))}
    };

    GfxShaderParamMap shader_text_params = {
        {"colour", GfxShaderParam(Vector3(1, 1, 1))},
        {"alpha", GfxShaderParam(1.0f)},
        {"tex", GfxShaderParam(GFX_GSL_FLOAT_TEXTURE2, Vector4(1, 1, 1, 1))}
    };

    if (gfx_d3d9()) {
        shader_solid = gfx_shader_make_from_existing(
            "/system/HudSolid",
            make_cg_shader("vp_solid", VERT, 
                "void main (\n"
                "    in float2 in_POSITION:POSITION,\n"
                "    uniform float4x4 global_worldViewProj,\n"
                "    out float4 out_POSITION:POSITION\n"
                ") {\n"
                "    out_POSITION = mul(global_worldViewProj, float4(in_POSITION,0,1));\n"
                "}\n"
            ),
            make_cg_shader("fp_solid", FRAG, 
                "void main (\n"
                "    uniform float3 mat_colour,\n"
                "    uniform float mat_alpha,\n"
                "    out float4 out_COLOR0:COLOR0\n"
                ") {\n"
                "    float4 texel = float4(1,1,1,1);\n"
                "    out_COLOR0 = float4(mat_colour, mat_alpha) * texel;\n"
                "}\n"
            ),
            shader_solid_params);
        
        shader_tex = gfx_shader_make_from_existing(
            "/system/HudTex",
            make_cg_shader("vp_tex", VERT, 
                "void main (\n"
                "    in float2 in_POSITION:POSITION,\n"
                "    in float4 in_TEXCOORD0:TEXCOORD0,\n"
                "    uniform float4x4 global_worldViewProj,\n"
                "    out float4 out_POSITION:POSITION,\n"
                "    out float4 out_TEXCOORD0:TEXCOORD0\n"
                ") {\n"
                "    out_POSITION = mul(global_worldViewProj, float4(in_POSITION,0,1));\n"
                "    out_TEXCOORD0.xy = in_TEXCOORD0.xy;\n"
                "}\n"
            ),
            make_cg_shader("fp_tex", FRAG, 
                "void main (\n"
                "    in float4 in_TEXCOORD0:TEXCOORD0,\n"
                "    uniform sampler2D mat_tex : TEXUNIT0,\n"
                "    uniform float3 mat_colour,\n"
                "    uniform float mat_alpha,\n"
                "    out float4 out_COLOR0:COLOR0\n"
                ") {\n"
                "    float4 texel = tex2D(mat_tex, in_TEXCOORD0.xy);\n"
                "    out_COLOR0 = float4(mat_colour, mat_alpha) * texel;\n"
                "}\n"
            ),
            shader_tex_params);
        
        shader_text = gfx_shader_make_from_existing(
            "/system/HudText",
            make_cg_shader("vp_text", VERT, 
                "void main (\n"
                "    in float2 in_POSITION:POSITION,\n"
                "    in float4 in_TEXCOORD0:TEXCOORD0,\n"
                "    in float4 in_TEXCOORD1:TEXCOORD1,\n"
                "    uniform float4x4 global_worldViewProj,\n"
                "    out float4 out_POSITION:POSITION,\n"
                "    out float4 out_TEXCOORD0:TEXCOORD0,\n"
                "    out float4 out_TEXCOORD1:TEXCOORD1\n"
                ") {\n"
                "    out_POSITION = mul(global_worldViewProj, float4(in_POSITION,0,1));\n"
                "    out_TEXCOORD0.xy = in_TEXCOORD0.xy;\n"
                "    out_TEXCOORD1 = in_TEXCOORD1;\n"
                "}\n"
            ),
            make_cg_shader("fp_text", FRAG, 
                "void main (\n"
                "    in float4 in_TEXCOORD0:TEXCOORD0,\n"
                "    in float4 in_TEXCOORD1:TEXCOORD1,\n"
                "    uniform sampler2D mat_tex : TEXUNIT0,\n"
                "    uniform float3 mat_colour,\n"
                "    uniform float mat_alpha,\n"
                "    out float4 out_COLOR0:COLOR0\n"
                ") {\n"
                "    float4 texel = tex2D(mat_tex, in_TEXCOORD0.xy);\n"
                "    out_COLOR0 = float4(mat_colour, mat_alpha) * in_TEXCOORD1 * texel;\n"
                "}\n"
            ),
            shader_text_params);

    } else {

        shader_solid = gfx_shader_make_from_existing(
            "/system/HudSolid",
            make_glsl_shader("vp_solid", VERT, 
                "void main (void) {\n"
                "    gl_Position = global_worldViewProj * vec4(vertex.xy, 0, 1);\n"
                "}\n"
            ),
            make_glsl_shader("fp_solid", FRAG, 
                "void main (void) {\n"
                "    frag_colour = vec4(mat_colour, mat_alpha);\n"
                "}\n"
            ),
            shader_solid_params);
        
        shader_tex = gfx_shader_make_from_existing(
            "/system/HudTex",
            make_glsl_shader("vp_tex", VERT, 
                "void main (void) {\n"
                "    gl_Position = global_worldViewProj * vec4(vertex.xy, 0, 1);\n"
                "    trans0.xy = uv0.xy;\n"
                "}\n"
            ),
            make_glsl_shader("fp_tex", FRAG, 
                "void main (void) {\n"
                "    vec4 texel = texture(mat_tex, trans0.xy);\n"
                "    frag_colour = vec4(mat_colour, mat_alpha) * texel;\n"
                "}\n"
            ),
            shader_tex_params);
        
        shader_text = gfx_shader_make_from_existing(
            "/system/HudText",
            make_glsl_shader("vp_text", VERT, 
                "void main (void) {\n"
                "    gl_Position = global_worldViewProj * vec4(vertex.xy, 0, 1);\n"
                "    trans0.xy = uv0.xy;\n"
                "    trans1 = uv1;\n"
                "}\n"
            ),
            make_glsl_shader("fp_text", FRAG, 
                "void main (void) {\n"
                "    vec4 texel = texture(mat_tex, trans0.xy);\n"
                "    frag_colour = vec4(mat_colour, mat_alpha) * trans1 * texel;\n"
                "}\n"
            ),
            shader_text_params);
    }

    shader_solid->validate();
    shader_tex->validate();
    shader_text->validate();

    shader_solid_binds = shader_solid->makeBindings();
    shader_tex_binds = shader_tex->makeBindings();
    shader_text_binds = shader_text->makeBindings();

}

void gfx_hud_shutdown (lua_State *L)
{
    quad_vbuf.setNull();
    OGRE_DELETE quad_vdata;
    cornered_quad_vbuf.setNull();
    OGRE_DELETE cornered_quad_vdata;

    // Not all destroy callbacks actually destroy their children.  Orphaned
    // children end up being adopted by grandparents, and ultimately the root.  If
    // hud elements are left at the root until Lua state destruction, the gc
    // callbacks are called in an arbitrary order. That can cause undefined
    // behvaiour on the c++ side (access of deleted objects).  So we aggressively
    // destroy all the root elements before the actual lua shutdown.  This causes
    // the callbacks to run in the correct order, and avoid segfaults.

    while (root_elements.size() > 0) {
        // First clean up all the non Lua ones.  This is the easy part.

        // Copy into an intermediate buffer, since calling destroy changes root_elements.
        std::vector<GfxHudBase*> all_non_lua;
        for (unsigned j=0 ; j<root_elements.size() ; ++j) {
            GfxHudBase *base = root_elements[j];
            if (dynamic_cast<GfxHudObject*>(base) == NULL) all_non_lua.push_back(base);
        }
        for (unsigned j=0 ; j<all_non_lua.size() ; ++j) {
            all_non_lua[j]->destroy();
        }

        // Now kill the ones with lua callbacks.
        std::vector<GfxHudObject*> local_root_objects = get_all_hud_objects(root_elements);
        for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
            GfxHudObject *obj = local_root_objects[j];
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

void gfx_render_hud_text (GfxHudText *text, const Vector3 &colour, float alpha, const Vector2 &offset)
{
    GfxFont *font = text->getFont();
    GfxTextureDiskResource *tex = font->getTexture();

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

    shader_text->bindShader();
    shader_text->bindGlobals(matrix, I, I, win_size, render_target_flipping);
    shader_text_binds->setBinding("colour", colour);
    shader_text_binds->setBinding("alpha", alpha);
    shader_text->bind(shader_text_binds);
    shader_text->bindShaderParams();

    ogre_rs->_setCullingMode(Ogre::CULL_CLOCKWISE);
    ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);

    ogre_rs->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

    ogre_rs->_setTexture(0, true, tex->getOgreTexturePtr());
    Ogre::TextureUnitState::UVWAddressingMode addr_mode = {
        Ogre::TextureUnitState::TAM_CLAMP,
        Ogre::TextureUnitState::TAM_CLAMP,
        Ogre::TextureUnitState::TAM_CLAMP
    };
    ogre_rs->_setTextureAddressingMode(0, addr_mode);
    ogre_rs->_setTextureUnitFiltering(0, Ogre::FO_ANISOTROPIC, Ogre::FO_ANISOTROPIC, Ogre::FO_LINEAR);
    ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
    ogre_rs->setStencilCheckEnabled(false);
    ogre_rs->_setDepthBias(0, 0);

    if (text->buf.getRenderOperation().indexData->indexCount > 0) {
        ogre_rs->_render(text->buf.getRenderOperation());
    }

    ogre_rs->_disableTextureUnit(0);
}


void gfx_render_hud_one (GfxHudBase *base)
{
    if (!base->isEnabled()) return;
    if (base->destroyed()) return;

    GfxHudObject *obj = dynamic_cast<GfxHudObject*>(base);
    if (obj!=NULL) {
        GfxTextureDiskResource *tex = obj->getTexture();
        if (tex!=NULL && !tex->isLoaded()) {
            CERR << "Hud object using unloaded texture: \"" << (*tex) << "\"" << std::endl;
            obj->setTexture(DiskResourcePtr<GfxTextureDiskResource>());
            tex = nullptr;
        }

        bool is_cornered = obj->isCornered() && tex != NULL;

        GfxShader *shader = tex == NULL ? shader_solid : shader_tex;
        const GfxShaderBindingsPtr &binds = tex == NULL ? shader_solid_binds : shader_tex_binds;

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

        if (is_cornered) {
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
        matrix_scale.setScale(Ogre::Vector3(2/win_size.x, 2/win_size.y ,1));

        // TODO: Is there no render target flipping?
        // I guess we never rendered HUD to a texture on GL?
        bool render_target_flipping = false;
        Ogre::Matrix4 matrix = matrix_scale * matrix_d3d_offset * matrix_trans * matrix_spin;

        shader->bindShader();
        shader->bindGlobals(matrix, I, I, win_size, render_target_flipping);
        binds->setBinding("colour", obj->getColour());
        binds->setBinding("alpha", obj->getAlpha());
        shader->bind(binds);
        shader->bindShaderParams();

        ogre_rs->_setCullingMode(Ogre::CULL_NONE);
        ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);

        ogre_rs->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

        if (tex != NULL) {
            ogre_rs->_setTexture(0, true, tex->getOgreTexturePtr());
            Ogre::TextureUnitState::UVWAddressingMode addr_mode = {
                Ogre::TextureUnitState::TAM_WRAP,
                Ogre::TextureUnitState::TAM_WRAP,
                Ogre::TextureUnitState::TAM_WRAP
            };
            ogre_rs->_setTextureAddressingMode(0, addr_mode);
            ogre_rs->_setTextureUnitFiltering(0, Ogre::FO_ANISOTROPIC, Ogre::FO_ANISOTROPIC, Ogre::FO_LINEAR);
        }

        ogre_rs->_setDepthBias(0, 0);
        ogre_rs->_setPolygonMode(Ogre::PM_SOLID);
        ogre_rs->setStencilCheckEnabled(false);

        // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl

        // render the rectangle
        Ogre::RenderOperation op;
        if (is_cornered) {
            op.useIndexes = true;
            op.vertexData = cornered_quad_vdata;
            op.indexData = cornered_quad_idata;
        } else {
            op.useIndexes = false;
            op.vertexData = quad_vdata;
        }
        op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        ogre_rs->_render(op);

        if (tex != NULL) {
            ogre_rs->_disableTextureUnit(0);
        }

        for (unsigned i=0 ; i<=GFX_HUD_ZORDER_MAX ; ++i) {
            for (unsigned j=0 ; j<obj->children.size() ; ++j) {
                GfxHudBase *child = obj->children[j];
                if (child->getZOrder() != i) continue;
                gfx_render_hud_one(child);
            }
        }
    }

    GfxHudText *text = dynamic_cast<GfxHudText*>(base);
    if (text!=NULL) {

        text->buf.updateGPU(text->wrap == Vector2(0,0), text->scroll, text->scroll+text->wrap.y);

        if (text->getShadow() != Vector2(0,0)) {
            gfx_render_hud_text(text, text->getShadowColour(), text->getShadowAlpha(), text->getShadow());
        }
        gfx_render_hud_text(text, text->getColour(), text->getAlpha(), Vector2(0,0));
    }
}

void gfx_hud_render (Ogre::Viewport *vp)
{
    ogre_rs->_setViewport(vp);

    ogre_rs->_beginFrame();

    try {

        for (unsigned i=0 ; i<=GFX_HUD_ZORDER_MAX ; ++i) {
            for (unsigned j=0 ; j<root_elements.size() ; ++j) {
                GfxHudBase *el = root_elements[j];
                if (el->getZOrder() != i) continue;
                gfx_render_hud_one(el);
            }
        }

    } catch (const Exception &e) {
        CERR << "Rendering HUD, got: " << e << std::endl;
    } catch (const Ogre::Exception &e) {
        CERR << "Rendering HUD, got: " << e.getDescription() << std::endl;
    }

    ogre_rs->_endFrame();

}

// }}}


// {{{ RESIZE

// when true, triggers resize callbacks at the root
static bool window_size_dirty;

void gfx_hud_signal_window_resized (unsigned w, unsigned h)
{
    Vector2 new_win_size = Vector2(float(w),float(h));
    if (win_size == new_win_size) return;
    win_size = new_win_size;
    window_size_dirty = true;
}

void gfx_hud_call_per_frame_callbacks (lua_State *L, float elapsed)
{

    std::vector<GfxHudObject*> local_root_objects = get_all_hud_objects(root_elements);

    if (window_size_dirty) {
        window_size_dirty = false;
        for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
            GfxHudObject *obj = local_root_objects[j];
            if (obj->destroyed()) continue;
            obj->triggerParentResized(L);
        }
    }

    for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
        GfxHudObject *obj = local_root_objects[j];
        if (obj->destroyed()) continue;
        obj->triggerFrame(L, elapsed);
    }

    dec_all_hud_objects(L, local_root_objects);
}

// }}}


// {{{ INPUT

GfxHudObject *gfx_hud_ray (int x, int y)
{
    Vector2 screen_pos(x,y);

    for (unsigned j=0 ; j<root_elements.size() ; ++j) {
        GfxHudBase *base = root_elements[j];
        if (base->destroyed()) continue;
        GfxHudObject *obj = dynamic_cast<GfxHudObject*>(base);
        GfxHudObject *hit = obj->shootRay(screen_pos);
        if (hit != NULL) return hit;
    }

    return NULL;
}

static Vector2 last_mouse_abs;
void gfx_hud_signal_mouse_move (lua_State *L, const Vector2 &abs)
{
    // make local copy because callbacks can destroy elements
    std::vector<GfxHudObject*> local_root_objects = get_all_hud_objects(root_elements);

    for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
        GfxHudObject *obj = local_root_objects[j];
        if (obj->destroyed()) continue;
        obj->triggerMouseMove(L, abs);
    }

    dec_all_hud_objects(L, local_root_objects);
    last_mouse_abs = abs;
}

void gfx_hud_signal_button (lua_State *L, const std::string &key)
{
    // make local copy because callbacks can destroy elements
    std::vector<GfxHudObject*> local_root_objects = get_all_hud_objects(root_elements);

    for (unsigned j=0 ; j<local_root_objects.size() ; ++j) {
        GfxHudObject *obj = local_root_objects[j];
        if (obj->destroyed()) continue;
        obj->triggerMouseMove(L, last_mouse_abs);
        obj->triggerButton(L, key);
    }

    dec_all_hud_objects(L, local_root_objects);
}

void gfx_hud_signal_flush (lua_State *L)
{
    (void) L;
    // TODO: issue 'fake' key up for any keys that are still down.
}
// }}}
