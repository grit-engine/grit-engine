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

class GfxTextBuffer;

#ifndef GFX_TEXT_BUFFER_H
#define GFX_TEXT_BUFFER_H

#include <cstdint>

#include <OgreHardwareBufferManager.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>

#include "../math_util.h"
#include "GfxFont.h"

/** Encapsulate the code required to build GPU buffers for rendering text.*/
class GfxTextBuffer {

    GfxFont *font;

    Ogre::VertexDeclaration vDecl;
    Ogre::VertexData vData;
    Ogre::IndexData iData;
    Ogre::HardwareVertexBufferSharedPtr vBuf;
    Ogre::HardwareIndexBufferSharedPtr iBuf;
    unsigned currentGPUCapacity; // due to lazy update, lags behind

    std::vector<float> rawVBuf;
    std::vector<uint16_t> rawIBuf;
    unsigned currentSize;
    unsigned currentCapacity;

    Vector2 currentOffset;

    bool dirty;

    public:

    GfxTextBuffer (GfxFont *font);

    ~GfxTextBuffer (void) { }

    /** Reset the buffer. */
    void clear (void)
    {
        rawVBuf.clear();
        rawIBuf.clear();
        currentOffset = Vector2(0,0);
    }


    /** Basic interface: add a character. */
    bool addRawChar (GfxFont::codepoint_t cp, Vector3 topColour, float topAlpha, Vector3 botColour, float botAlpha);

    /** Basic interface: end of line. */
    void endLine (float line_height);

    /** High level interface: Add a string that can contain \n,\t and ansi terminal colours.
     * \param text The text in UTF8.
     */
    void addFormattedString (const std::string &text);

    protected:

    /** Change the amount of space used for data on the host. */
    void reserve (unsigned new_capacity);

    /** Copy the buffer to the GPU, enlargeing GPU buffers if necessary. */
    void copyToGPUIfNeeded ();

};

#endif
