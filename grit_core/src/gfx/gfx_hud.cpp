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

#include "gfx_hud.h"
#include "gfx_internal.h"

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



void GfxHudObject::init (lua_State *L, const GfxHudObjectPtr &ptr)
{
    (void) L; (void) ptr;
}

void GfxHudObject::parentResized (lua_State *L, const GfxHudObjectPtr &ptr)
{
    (void) L; (void) ptr;
}

void GfxHudObject::setTexture (GfxDiskResource *v)
{
    assertAlive();
    // maybe check it's valid at this point
    // although it can always become unloaaded before actual rendering time
    texture = v;
}


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

    float width = ogre_win->getWidth();
    float height = ogre_win->getHeight();

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

static fast_erase_vector<GfxHudBase*> all_bodies;

GfxHudBase::GfxHudBase (void)
  : dead(false), parent(NULL), zOrder(127), position(0,0), size(32,32), orientation(0), enabled(true)
{
    all_bodies.push_back(this);
}

GfxHudBase::~GfxHudBase (void)
{
    if (!dead) destroy();
}

void GfxHudBase::assertAlive (void)
{
    if (dead) GRIT_EXCEPT("Hud element destroyed.");
}


void GfxHudBase::destroy (void)
{
    APP_ASSERT(!dead);
    dead = true;
    all_bodies.erase(this);
    // do something with parents and children
}

void gfx_hud_render (void)
{
    ogre_rs->_beginFrame();

    try {

        for (unsigned i=0 ; i<all_bodies.size() ; ++i) {

            GfxHudBase *base = all_bodies[i];
            if (!base->isEnabled()) continue;

            GfxHudObject *body = dynamic_cast<GfxHudObject*>(base);
            if (body!=NULL) {
                //DiskResource *tex_ = disk_resource_get_or_make("/system/Crosshair.bmp");
                //GfxDiskResource *tex = dynamic_cast<GfxDiskResource*>(tex_);
                GfxDiskResource *tex = body->getTexture();
                if (tex!=NULL && !tex->isLoaded()) {
                    CERR << "Hud body using unloaded texture: \"" << (*tex) << "\"" << std::endl;
                    body->setTexture(NULL);
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

                set_vertex_data(body->getPosition(), body->getSize(), body->getOrientation(), body->getUV1(), body->getUV2());

                // premultiply the colour by the alpha -- for convenience
                try_set_named_constant(fp, "colour", to_ogre(body->getAlpha() * body->getColour()));
                try_set_named_constant(fp, "alpha", body->getAlpha());


                ogre_rs->_setCullingMode(Ogre::CULL_CLOCKWISE);
                ogre_rs->_setDepthBufferParams(false, false, Ogre::CMPF_LESS_EQUAL);

                ogre_rs->_setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

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
            }

            GfxHudText *text = dynamic_cast<GfxHudText*>(base);
            if (text!=NULL) {
            }
        }

    } catch (const GritException &e) {
        CERR << "Rendering HUD, got: " << e.msg << std::endl;
    } catch (const Ogre::Exception &e) {
        CERR << "Rendering HUD, got: " << e.getDescription() << std::endl;
    }

    ogre_rs->_endFrame();

}

