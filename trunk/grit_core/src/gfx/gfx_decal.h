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

class GfxDecal;
typedef SharedPtr<GfxDecal> GfxDecalPtr;

#ifndef GFX_DECAL_H
#define GFX_DECAL_H

#include "gfx.h"
#include "gfx_fertile_node.h"
#include "gfx_material.h"
#include "gfx_particle_system.h"

class GfxDecal : public GfxNode {

    protected:
    static const std::string className;
    bool enabled;
    float fade;
    Quaternion aim;
    GfxMaterial *material;
    
    protected:

    GfxDecal (GfxMaterial *material, const GfxNodePtr &par_);
    ~GfxDecal ();

    public:
    static GfxDecalPtr make (GfxMaterial *material, const GfxNodePtr &par_=GfxNodePtr(NULL))
    { return GfxDecalPtr(new GfxDecal(material, par_)); }
    
    bool isEnabled (void);
    void setEnabled (bool v);

    float getFade (void);
    void setFade (float f);

    GfxMaterial *getMaterial (void);
    void setMaterial (GfxMaterial *m);

    void render (const GfxShaderGlobals &g);

    void destroy (void);

    friend class SharedPtr<GfxDecal>;
};

// called every frame
void gfx_decal_render (GfxPipeline *p);

void gfx_decal_init (void);
void gfx_decal_shutdown (void);

#endif
