/* Copyright (c) The Grit Game Engine authors 2016
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef GfxTracerBody_h
#define GfxTracerBody_h

class GfxPipeline;

#include <utility>

#include <math_util.h>

#include "../vect_util.h"
#include "../shared_ptr.h"

#include "gfx.h"
#include "gfx_disk_resource.h"
#include "gfx_node.h"
#include "gfx_shader.h"

class GfxTracerBody : public GfxNode {

    protected:
    static const std::string className;
    bool enabled;
    DiskResourcePtr<GfxTextureDiskResource> texture;
    unsigned length;

    Vector3 diffuseColour;
    Vector3 emissiveColour;
    float fade;
    float alpha;
    float size;

    struct Element {
        Vector3 diffuseColour;
        Vector3 emissiveColour;
        float alpha;
        float size;
        Vector3 pos;
        Element (void) { }
        Element (const Vector3 &diffuse_colour, const Vector3 &emissive_colour,
                 float alpha, float size, const Vector3 &pos)
          : diffuseColour(diffuse_colour), emissiveColour(emissive_colour), alpha(alpha),
            size(size), pos(pos)
        { }
    };

    /** Handles the GPU stuff -- geometry, rendering, etc. */
    class Buffer {
        const GfxTracerBody * const body;

        Ogre::HardwareVertexBufferSharedPtr vertexBuffer;
        Ogre::RenderOperation renderOp;
        Ogre::VertexData vertexData;
        Ogre::IndexData indexData;
        unsigned elementVertexSize;

        float *vertexPtr, *vertexPtr0;
        uint16_t *indexPtr, *indexPtr0;
        unsigned maxElements;
        unsigned counter;

        public:

        Buffer (const GfxTracerBody *body);

        void beginTrace (unsigned elements);

        void addTraceElement (const Vector3 &cam_pos, const Element &element);

        void endTrace (void);

        const Ogre::RenderOperation &getRenderOperation (void) { return renderOp; }

    };

    Buffer buffer;

    std::vector<Element> elements;

    GfxTracerBody (const GfxNodePtr &par_);
    ~GfxTracerBody (void);

    public:
    static SharedPtr<GfxTracerBody> make (const GfxNodePtr &par_=GfxNodePtr(NULL))
    { return SharedPtr<GfxTracerBody>(new GfxTracerBody(par_)); }

    bool isEnabled (void) const;
    void setEnabled (bool v);

    Vector3 getDiffuseColour (void) const;
    void setDiffuseColour (const Vector3 &f);

    Vector3 getEmissiveColour (void) const;
    void setEmissiveColour (const Vector3 &f);

    float getAlpha (void) const;
    void setAlpha (float f);

    float getFade (void) const;
    void setFade (float f);

    float getSize (void) const;
    void setSize (float f);

    unsigned getLength (void) const;
    void setLength (unsigned v);

    GfxTextureDiskResource *getTexture (void) const;
    void setTexture (const DiskResourcePtr<GfxTextureDiskResource> &v);

    void render (const GfxPipeline *pipe, const GfxShaderGlobals &g);

    void pump (void);

    void destroy (void);

    void assertAlive (void) const;

    friend class SharedPtr<GfxTracerBody>;
};

// Called every frame.
void gfx_tracer_body_render (GfxPipeline *p);

void gfx_tracer_body_pump (void);

void gfx_tracer_body_init (void);

#endif
