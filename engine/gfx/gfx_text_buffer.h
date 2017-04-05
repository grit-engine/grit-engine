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

class GfxTextBuffer;

#ifndef GFX_TEXT_BUFFER_H
#define GFX_TEXT_BUFFER_H

#include <cstdint>

#include <math_util.h>

#include "gfx_font.h"

/** Encapsulate the code required to build GPU buffers for rendering text.*/
class GfxTextBuffer {

    struct ColouredChar {
        GfxFont::codepoint_t cp;
        /* We have to record the top/bottom colour anyway since ansi colour codes are not
        the only way to change the colour.  Therefore we may as well record all the
        colour.  Also, if we have the colour per character, it is possible to know the
        colour without looking back for the last colour code.
         */
        Vector3 topColour;
        float topAlpha;
        Vector3 bottomColour;
        float bottomAlpha;
        unsigned long left, top;
        ColouredChar (GfxFont::codepoint_t cp, const Vector3 &tc, float ta, const Vector3 &bc, float ba)
          : cp(cp), topColour(tc), topAlpha(ta), bottomColour(bc), bottomAlpha(ba)
        { }
    };
    std::vector<ColouredChar> colouredText;

    GfxFont *font;

    Ogre::VertexData vData;
    Ogre::IndexData iData;
    Ogre::HardwareVertexBufferSharedPtr vBuf;
    Ogre::HardwareIndexBufferSharedPtr iBuf;
    Ogre::RenderOperation op;
    unsigned currentGPUCapacity; // due to lazy update, lags behind

    std::vector<float> rawVBuf;
    std::vector<uint16_t> rawIBuf;

    Vector2 currentDrawnDimensions;

    unsigned long currentLeft, currentTop;

    long lastTop, lastBottom;
    unsigned long wrap;

    bool dirty;

    /** Fill in the ColouredChar::pos fields. */
    void recalculatePositions (unsigned long offset = 0);

    unsigned long binaryChopTop (unsigned long begin, unsigned long end, unsigned long top);

    unsigned long binaryChopBottom (unsigned long begin, unsigned long end, unsigned long bottom_top);

    public:

    GfxTextBuffer (GfxFont *font);

    ~GfxTextBuffer (void)
    {
        clear();
        vData.vertexDeclaration = NULL; // save OGRE from itself
    }

    /** High level interface: Add a string that can contain \n,\t and ansi terminal colours.
     * \param text The text in UTF8.
     * \param top_colour and friends: Initial colours, overriden by use of ansi terminal colours.
     */
    void addFormattedString (const std::string &text, const Vector3 &top_colour, float top_alpha, const Vector3 &bot_colour, float bot_alpha);

    /** Put the triangles in the vertex/index buffers and upload them.
     * \param no_scroll Do not use top/bottom to clip the buffer vertically, render the whole buffer.
     * \param top The top of the visible area, in pixels from the top of the buffer.  Can be negative to add extra space at top.
     * \param bottom The bottom of the visible area, in pixels from the top of the buffer.  Can be negative.
     */
    void updateGPU (bool no_scroll, long top, long bottom);

    /** Reset the buffer. */
    void clear (void)
    {
        colouredText.clear();
        recalculatePositions();
        dirty = true;
    }

    /** Return number of vertexes required to render the text. */
    unsigned getVertexes (void) const { return vData.vertexCount; }

    /** Return number of triangles required to render the text. */
    unsigned getTriangles (void) const { return iData.indexCount / 3; }

    /** Sets the font. */
    void setFont (GfxFont *v) { font = v; recalculatePositions(); }

    /** Returns the font. */
    GfxFont *getFont (void) const { return font; }

    /** Returns the size of the text rectangle in pixels.  Drawn part only, updated by updateGPU. */
    const Vector2 &getDrawnDimensions (void) const { return currentDrawnDimensions; }

    /** Returns the size of the text rectangle in pixels.  Entire buffer. */
    unsigned long getBufferHeight (void) const { return font->getHeight() + currentTop; }

    /** Set the max size.  This is used to wrap text during addFormattedString. */
    void setWrap (float v) { wrap = v; recalculatePositions(); }

    /** Returns the max size. \see setWrap */
    float getWrap (void) const { return wrap; }

    /** Get an operation that can be used to render this text buffer. */
    const Ogre::RenderOperation &getRenderOperation (void) const { return op; }

};

#endif
