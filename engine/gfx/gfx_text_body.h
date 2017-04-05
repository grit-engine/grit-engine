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

#include "../shared_ptr.h"

class GfxTextBody;
typedef SharedPtr<GfxTextBody> GfxTextBodyPtr;

#ifndef GfxTextBody_h
#define GfxTextBody_h

#include "gfx_fertile_node.h"
#include "gfx_text_buffer.h"
#include "gfx_material.h"

// Must extend Ogre::MovableObject so that we can become attached to a node and
// rendered by the regular Ogre scenemanager-based pipeline.
class GfxTextBody : public GfxFertileNode, public Ogre::MovableObject {

    protected:

    class Sub : public Ogre::Renderable {

        protected:

        GfxTextBody *parent;

        public:

        Sub (GfxTextBody *parent)
            : parent(parent)
        { }
        ~Sub() { }

        const Ogre::MaterialPtr& getMaterial(void) const;


        void getRenderOperation(Ogre::RenderOperation& op)
        { op = parent->textBuffer.getRenderOperation(); }

        void getWorldTransforms(Ogre::Matrix4* xform) const;
        unsigned short getNumWorldTransforms(void) const;
        Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
        const Ogre::LightList& getLights(void) const;

        bool getCastShadows(void) const;
    };


	void recomputeBounds (void);

    const Ogre::AxisAlignedBox& getBoundingBox (void) const
	{ return bounds; }
	
    float getBoundingRadius (void) const;

    void _updateRenderQueue (Ogre::RenderQueue* queue);
    void visitRenderables (Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

    const std::string& getMovableType (void) const { return className; }


    static const std::string className;

    Sub sub;
 
    GfxMaterial *material;

    bool emissiveEnabled;
    float fade;
    // Hack to pass material into queue->addRenderable
    Ogre::MaterialPtr renderMaterial;
    GfxTextBuffer textBuffer;
	// Ogre API requires us to return a reference to this.
	Ogre::AxisAlignedBox bounds;
    bool enabled;
    bool castShadows;

    GfxTextBody (GfxFont *font, GfxMaterial *mat, const GfxNodePtr &par_);
    ~GfxTextBody ();

    public:
    static GfxTextBodyPtr make (GfxFont *font, GfxMaterial *mat,
                            const GfxNodePtr &par_=GfxNodePtr(NULL));

    void append (const std::string &text, const Vector3 &top_colour, float top_alpha,
                 const Vector3 &bot_colour, float bot_alpha) {
        textBuffer.addFormattedString(text, top_colour, top_alpha, bot_colour, bot_alpha);
    }

    void updateGpu (void) {
        textBuffer.updateGPU(true, 0, 0);
		recomputeBounds();
    }

    void clear (void) {
        textBuffer.clear();
    }
    GfxFont *getFont (void) const {
        return textBuffer.getFont();
    }
    void setFont (GfxFont *font) {
        textBuffer.setFont(font);
    }


    GfxMaterial *getMaterial (void) const;
    void setMaterial (GfxMaterial *m);

    bool getEmissiveEnabled (void) const;
    void setEmissiveEnabled (bool v);

    unsigned getBatches (void) const;
    unsigned getBatchesWithChildren (void) const;

    unsigned getTriangles (void) const;
    unsigned getTrianglesWithChildren (void) const;

    unsigned getVertexes (void) const;
    unsigned getVertexesWithChildren (void) const;

    float getFade (void) const;
    void setFade (float f);

    bool getCastShadows (void) const;
    void setCastShadows (bool v);

    bool isEnabled (void) const;
    void setEnabled (bool v);

    void destroy (void);

    bool hasGraphics (void) const { return true; }

    friend class SharedPtr<GfxTextBody>;
};

#endif
