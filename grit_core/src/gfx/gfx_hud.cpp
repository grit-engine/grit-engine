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

#include <OgreHighLevelGpuProgramManager.h>
#include <OgreCgProgram.h>

#include "../CentralisedLog.h"
#include "../path_util.h"

#include "lua_wrappers_gfx.h"
#include "gfx_hud.h"
#include "gfx_internal.h"

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


// {{{ OBJECTS
static fast_erase_vector<GfxHudBase*> root_elements;

void GfxHudBase::registerRemove (void)
{
    //CVERB << "GfxHud element resetting its existing existence: " << this << std::endl;
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
  : dead(false), parent(NULL), zOrder(3),
    position(0,0), size(32,32), orientation(0), inheritOrientation(true), enabled(true)
{
    //CVERB << "GfxHud element created: " << this << std::endl;
    registerAdd();
}

GfxHudBase::~GfxHudBase (void)
{
    if (!dead) destroy();
}

void GfxHudBase::destroy (void)
{
    assertAlive();
    registerRemove();
    dead = true;
    //CVERB << "GfxHud element destroyed: " << this << std::endl;
}

void GfxHudBase::assertAlive (void)
{
    if (dead)
        GRIT_EXCEPT("Hud element destroyed.");
}

Radian GfxHudBase::getDerivedOrientation (void)
{
    assertAlive();
    if (inheritOrientation && parent!=NULL) {
        return parent->getDerivedOrientation() + orientation;
    } else {
        return orientation;
    }
}

Vector2 GfxHudBase::getDerivedPosition (void)
{
    assertAlive();
    if (parent!=NULL) {
        return parent->getDerivedPosition() + position.rotateBy(parent->orientation);
    } else {
        return position;
    }
}



GfxHudObject::GfxHudObject (GfxHudClass *hud_class)
  : GfxHudBase(), hudClass(hud_class), texture(NULL), uv1(0,0), uv2(1,1), colour(1,1,1), alpha(1), refCount(0)
{
}

void GfxHudObject::incRefCount (void)
{
    refCount++;
}

void GfxHudObject::decRefCount (lua_State *L)
{
    refCount--;
    if (refCount == 0) {
        destroy(L);
        if (refCount == 0) {
            // the destroy callback may increase the refcount by adding new
            // alisaes, in which case the object is still destroyed, i.e.
            // will throw exceptions when it is used in any way, but
            // will not actually be deleted until the ref count does reach zero
            delete this;
        }
    }
}


void GfxHudObject::destroy (lua_State *L)
{
    if (!dead) {
        triggerDestroy(L);
        table.setNil(L);
        while (children.size() != 0) {
            children[0]->setParent(parent);
        }
        GfxHudBase::destroy();
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
    pwd_push_dir(hudClass->dir);
    int status = lua_pcall(L,1,0,error_handler);
    pwd_pop();
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
    STACK_BASE;
    //stack is empty

    // error handler in case there is a problem during 
    // the callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    //stack: err

    // get the function
    hudClass->get(L,"parentResized");
    //stack: err,callback
    if (lua_isnil(L,-1)) {
        // no parentResized callback -- our work is done
        lua_pop(L,2);
        STACK_CHECK;
        return;
    }


    //stack: err,callback
    STACK_CHECK_N(2);

    push_gfxhudobj(L, this);
    push_v2(L, win_size);
    //stack: err,callback,object,size

    STACK_CHECK_N(4);

    // call (1 arg), pops function too
    pwd_push_dir(hudClass->dir);
    int status = lua_pcall(L,2,0,error_handler);
    pwd_pop();
    if (status) {
        STACK_CHECK_N(2);
        //stack: err,error
        // pop the error message since the error handler will
        // have already printed it out
        lua_pop(L,1);
        CERR << "Hud object of class: \"" << hudClass->name << "\" raised an error on parent resize, destroying it." << std::endl;
        // will call destroy callback
        destroy(L);
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
    pwd_push_dir(hudClass->dir);
    int status = lua_pcall(L,1,0,error_handler);
    pwd_pop();
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

void GfxHudObject::setTexture (GfxDiskResource *v)
{
    assertAlive();
    // maybe check it's valid at this point
    // although it can always become unloaaded before actual rendering time
    texture = v;
}

// }}}


// {{{ RENDERING

static Ogre::HighLevelGpuProgramPtr vp_tex, fp_tex;
static Ogre::HighLevelGpuProgramPtr vp_solid, fp_solid;
static Ogre::VertexData *quad_vdata; // Must be allocated later because constructor requires ogre to be initialised
static Ogre::HardwareVertexBufferSharedPtr quad_vbuf;
static unsigned quad_vdecl_size;

enum VertOrFrag { VERT, FRAG };

static Ogre::HighLevelGpuProgramPtr make_shader (VertOrFrag kind, const std::string &code)
{
    Ogre::StringVector vp_profs, fp_profs;
    vp_profs.push_back("vs_3_0"); fp_profs.push_back("ps_3_0"); // d3d9
    vp_profs.push_back("gpu_vp"); fp_profs.push_back("gp4fp"); // gl

    Ogre::HighLevelGpuProgramPtr prog = Ogre::HighLevelGpuProgramManager::getSingleton()
        .createProgram("hud_f", RESGRP, "cg", kind==FRAG ? Ogre::GPT_FRAGMENT_PROGRAM : Ogre::GPT_VERTEX_PROGRAM);
    APP_ASSERT(!prog.isNull());
    Ogre::CgProgram *tmp_prog = static_cast<Ogre::CgProgram*>(&*prog);
    prog->setSource(code);
    tmp_prog->setEntryPoint("main");
    tmp_prog->setProfiles(kind==FRAG ? fp_profs : vp_profs);
    tmp_prog->setCompileArguments("-I. -O3");
    prog->unload();
    prog->load();
    load_and_validate_shader(prog);
    return prog;
}

void gfx_hud_init (void)
{
    win_size = Vector2(ogre_win->getWidth(), ogre_win->getHeight());

    // Prepare vertex buffer
    quad_vdata = new Ogre::VertexData();
    quad_vdata->vertexStart = 0;
    quad_vdata->vertexCount = 6;

    // Non-instanced data
    quad_vdecl_size = 0;
    quad_vdecl_size += quad_vdata->vertexDeclaration->addElement(0, quad_vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_POSITION).getSize();
    quad_vdecl_size += quad_vdata->vertexDeclaration->addElement(0, quad_vdecl_size, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES,0).getSize();
    quad_vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            quad_vdecl_size, 6, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
    quad_vdata->vertexBufferBinding->setBinding(0, quad_vbuf);

    // Initialise vertex program

    vp_solid = make_shader(VERT, 
        "void main (\n"
        "    in float2 in_POSITION:POSITION,\n"
        "    out float4 out_POSITION:POSITION\n"
        ") {\n"
        "    out_POSITION = float4(in_POSITION,0,1);\n"
        "}\n"
    );

    fp_solid = make_shader(FRAG, 
        "void main (\n"
        "    uniform float3 colour,\n"
        "    uniform float alpha,\n"
        "    out float4 out_COLOR0:COLOR0\n"
        ") {\n"
        "    float4 texel = float4(1,1,1,1);\n"
        "    out_COLOR0 = float4(colour,alpha) * texel;\n"
        "}\n"
    );
    
    vp_tex = make_shader(VERT, 
        "void main (\n"
        "    in float2 in_POSITION:POSITION,\n"
        "    in float2 in_TEXCOORD0:TEXCOORD0,\n"
        "    out float4 out_POSITION:POSITION,\n"
        "    out float2 out_TEXCOORD0:TEXCOORD0\n"
        ") {\n"
        "    out_POSITION = float4(in_POSITION,0,1);\n"
        "    out_TEXCOORD0 = in_TEXCOORD0;\n"
        "}\n"
    );

    fp_tex = make_shader(FRAG, 
        "void main (\n"
        "    in float2 in_TEXCOORD0:TEXCOORD0,\n"
        "    uniform sampler2D texture,\n"
        "    uniform float3 colour,\n"
        "    uniform float alpha,\n"
        "    out float4 out_COLOR0:COLOR0\n"
        ") {\n"
        "    float4 texel = tex2D(texture,in_TEXCOORD0);\n"
        "    out_COLOR0 = float4(colour,alpha) * texel;\n"
        "}\n"
    );
    
}

void gfx_hud_shutdown (void)
{
    vp_solid.setNull();
    fp_solid.setNull();
    vp_tex.setNull();
    fp_tex.setNull();
    quad_vbuf.setNull();
    delete quad_vdata;
}

static void set_vertex_data (const Vector2 &position, const Vector2 &size, Radian orientation, const Vector2 &uv1, const Vector2 &uv2)
{
    struct Vertex { // strict alignment required here
        Vector2 position;
        Vector2 uv;
    };

    const Vector2 halfsize = size/2;

    float width = float(ogre_win->getWidth());
    float height = float(ogre_win->getHeight());

    Vertex top_left= {
        (position + (Vector2(-1,1) * halfsize).rotateBy(orientation)) / Vector2(width,height) * Vector2(2,2) - Vector2(1,1),
        uv1
    };

    Vertex top_right = {
        (position + halfsize.rotateBy(orientation)) / Vector2(width,height) * Vector2(2,2) - Vector2(1,1),
        Vector2(uv2.x,uv1.y)
    };

    Vertex bottom_left = {
        (position - halfsize.rotateBy(orientation)) / Vector2(width,height) * Vector2(2,2) - Vector2(1,1),
        Vector2(uv1.x,uv2.y)
    };

    Vertex bottom_right = {
        (position + (Vector2(1,-1) * halfsize).rotateBy(orientation)) / Vector2(width,height) * Vector2(2,2) - Vector2(1,1),
        uv2
    };

    Vertex vdata_raw[] = { bottom_left, bottom_right, top_left, top_left, bottom_right, top_right };

    quad_vbuf->writeData(quad_vdata->vertexStart, quad_vdata->vertexCount*quad_vdecl_size, vdata_raw);
    
}


void gfx_render_hud_one (GfxHudBase *base)
{
    if (!base->isEnabled()) return;

    GfxHudObject *obj = dynamic_cast<GfxHudObject*>(base);
    if (obj!=NULL) {
        //DiskResource *tex_ = disk_resource_get_or_make("/system/Crosshair.bmp");
        //GfxDiskResource *tex = dynamic_cast<GfxDiskResource*>(tex_);
        GfxDiskResource *tex = obj->getTexture();
        if (tex!=NULL && !tex->isLoaded()) {
            CERR << "Hud object using unloaded texture: \"" << (*tex) << "\"" << std::endl;
            obj->setTexture(NULL);
            tex = NULL;
        }

        const Ogre::HighLevelGpuProgramPtr &vp = tex == NULL ? vp_solid : vp_tex;
        const Ogre::HighLevelGpuProgramPtr &fp = tex == NULL ? fp_solid : fp_tex;
        const Ogre::GpuProgramParametersSharedPtr &vertex_params = vp->getDefaultParameters();
        const Ogre::GpuProgramParametersSharedPtr &fragment_params = fp->getDefaultParameters();

        //const Vector2 position(400,400);
        //const Vector2 size(4,4);
        //Degree orientation(0);
        //const Vector2 uv1(0.0, 0.0); // top left
        //const Vector2 uv2(1.0, 1.0); // bottom right
        //const Vector3 colour(1,1,1);
        //const float alpha = 1;

        Vector2 uv1 = obj->getUV1();
        Vector2 uv2 = obj->getUV2();

        if (tex != NULL && d3d9) {
            // Texel offsets for D3D rasterisation quirks, see http://msdn.microsoft.com/en-us/library/windows/desktop/bb219690(v=vs.85).aspx
            Ogre::TexturePtr ogre_tex = tex->getOgreResourcePtr();
            ogre_tex->load(); // otherwise width and height are 512!?
            Vector2 tex_size(ogre_tex->getWidth(), ogre_tex->getHeight());
            Vector2 uv_offset = Vector2(0.5, -0.5) / obj->getSize();
            uv_offset = uv_offset.rotateBy(- obj->getDerivedOrientation());
            uv1 += uv_offset;
            uv2 += uv_offset;
        }

        set_vertex_data(obj->getDerivedPosition(),
                        obj->getSize(),
                        obj->getDerivedOrientation(), uv1, uv2);

        // premultiply the colour by the alpha -- for convenience
        try_set_named_constant(fp, "colour", to_ogre(obj->getAlpha() * obj->getColour()));
        try_set_named_constant(fp, "alpha", obj->getAlpha());


        ogre_rs->_setCullingMode(Ogre::CULL_CLOCKWISE);
        ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);

        ogre_rs->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

        if (tex != NULL) {
            ogre_rs->_setTexture(0, true, tex->getOgreResourcePtr());
            Ogre::TextureUnitState::UVWAddressingMode addr_mode = {
                Ogre::TextureUnitState::TAM_WRAP,
                Ogre::TextureUnitState::TAM_WRAP,
                Ogre::TextureUnitState::TAM_WRAP
            };
            ogre_rs->_setTextureAddressingMode(0, addr_mode);
            ogre_rs->_setTextureUnitFiltering(0, Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_LINEAR);
        }


        APP_ASSERT(vp->_getBindingDelegate()!=NULL);
        APP_ASSERT(fp->_getBindingDelegate()!=NULL);

        // both programs must be bound before we bind the params, otherwise some params are 'lost' in gl
        ogre_rs->bindGpuProgram(vp->_getBindingDelegate());
        ogre_rs->bindGpuProgram(fp->_getBindingDelegate());

        ogre_rs->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, fragment_params, Ogre::GPV_ALL);
        ogre_rs->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, vertex_params, Ogre::GPV_ALL);

        // render the instances
        Ogre::RenderOperation op;
        op.useIndexes = false;
        op.vertexData = quad_vdata;
        //op.indexData = &idata;
        op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        //op.numberOfInstances = 1;
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

    } catch (const GritException &e) {
        CERR << "Rendering HUD, got: " << e.msg << std::endl;
    } catch (const Ogre::Exception &e) {
        CERR << "Rendering HUD, got: " << e.getDescription() << std::endl;
    }

    ogre_rs->_endFrame();

}

// when true, triggers resize callbacks at the root
static bool dirty_parent_size;

void gfx_hud_signal_window_resized (unsigned w, unsigned h)
{
    Vector2 new_win_size = Vector2(float(w),float(h));
    if (win_size == new_win_size) return;
    win_size = new_win_size;
    dirty_parent_size = true;
}

void gfx_hud_call_parent_resized (lua_State *L)
{
    // Do just the root bodies, this will recurse down the tree in each case

    if (!dirty_parent_size) return;

    dirty_parent_size = false;

    for (unsigned j=0 ; j<root_elements.size() ; ++j) {

        GfxHudBase *base = root_elements[j];

        GfxHudObject *obj = dynamic_cast<GfxHudObject*>(base);
        if (obj != NULL) obj->triggerParentResized(L);

    }
}
