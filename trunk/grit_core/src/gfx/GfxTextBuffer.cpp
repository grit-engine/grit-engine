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

#include <unicode_util.h>

#include "GfxTextBuffer.h"

const unsigned VERT_FLOAT_SZ = 2+2+4;
const unsigned VERT_BYTE_SZ = VERT_FLOAT_SZ*sizeof(float);

GfxTextBuffer::GfxTextBuffer (GfxFont *font)
  : font(font), currentSize(0), currentDimensions(0,0), currentOffset(0,0), wrap(0,0), dirty(false)
{   
    APP_ASSERT(font != NULL);

    vBuf.setNull();
    iBuf.setNull();
    vData.vertexBufferBinding->setBinding(0, vBuf);
    vData.vertexStart = 0;
    vData.vertexCount = 0;

    iData.indexBuffer = iBuf;
    iData.indexStart = 0;
    iData.indexCount = 0;
    currentGPUCapacity = 0;


    op.useIndexes = true;
    op.vertexData = &vData;
    op.indexData = &iData;
    op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;


    unsigned vdecl_sz = 0;
    vdecl_sz += vData.vertexDeclaration->addElement(0, vdecl_sz, Ogre::VET_FLOAT2, Ogre::VES_POSITION, 0).getSize();
    vdecl_sz += vData.vertexDeclaration->addElement(0, vdecl_sz, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0).getSize(); // uv
    vdecl_sz += vData.vertexDeclaration->addElement(0, vdecl_sz, Ogre::VET_FLOAT4, Ogre::VES_TEXTURE_COORDINATES, 1).getSize(); // colour
    APP_ASSERT(vdecl_sz == VERT_BYTE_SZ);
}

void GfxTextBuffer::copyToGPUIfNeeded (void)
{
    if (!dirty) return;
    dirty = false;

    unsigned vertex_size = currentSize * 4; // 4 verts per quad
    unsigned index_size = currentSize * 6; // 2 triangles per quad

    if (currentGPUCapacity < currentSize) {
        // resize needed

        vBuf.setNull();
        iBuf.setNull();

        vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                        VERT_BYTE_SZ,
                        vertex_size,
                        Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

        iBuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
                        Ogre::HardwareIndexBuffer::IT_16BIT,
                        index_size,
                        Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

        vData.vertexBufferBinding->setBinding(0, vBuf);
        iData.indexBuffer = iBuf;

        currentGPUCapacity = currentSize;
    }

    if (currentSize > 0) {
        // copy the whole thing every time (could optimise this if needed)
        vBuf->writeData(0, vertex_size*VERT_BYTE_SZ, &rawVBuf[0], true);
        iBuf->writeData(0, index_size*sizeof(unsigned short), &rawIBuf[0], true);
    }
    
    vData.vertexCount = vertex_size;
    iData.indexCount = index_size;
}

unsigned GfxTextBuffer::wordFit (const std::vector<Char> &word)
{
    if (wrap.x == 0) {
        return word.size();
    }
    float curr_off = currentOffset.x;
    for (unsigned i=0 ; i<word.size() ; ++i) {
        GfxFont::codepoint_t cp = word[i].cp;
        GfxFont::CharRect uvs;
        bool r = font->getCodePointOrFail(cp, uvs);
        if (!r) EXCEPTEX << cp << ENDL;
        Vector2 tex_dim = font->getTextureDimensions();
        float width = fabsf(uvs.u2 - uvs.u1) * tex_dim.x;
        curr_off += width;

        if (curr_off > wrap.x) return i;
    }
    return word.size();
}

void GfxTextBuffer::addRawChar (const Char &c)
{

    GfxFont::CharRect uvs;
    bool r = font->getCodePointOrFail(c.cp, uvs);
    if (!r) EXCEPTEX << c.cp << ENDL;
    Vector2 tex_dim = font->getTextureDimensions();
    float width = fabsf(uvs.u2 - uvs.u1) * tex_dim.x;
    float height = fabsf(uvs.v2 - uvs.v1) * tex_dim.y;

    if (width == 0 || height == 0) {
        // invalid char rect in font -- indicates char should not be rendered
        return;
    }

    // use font, add to raw buf
    /* 0---1
       |  /|
       | / |
       |/  |
       2---3   indexes: 0 2 1  1 2 3
     */
    {
        rawVBuf.resize(rawVBuf.size() + VERT_FLOAT_SZ*4);
        float *base = &rawVBuf[currentSize * VERT_FLOAT_SZ*4];

        (*base++) = currentOffset.x;
        (*base++) = currentOffset.y;
        (*base++) = uvs.u1;
        (*base++) = uvs.v1;
        (*base++) = c.topColour.x;
        (*base++) = c.topColour.y;
        (*base++) = c.topColour.z;
        (*base++) = c.topAlpha;

        (*base++) = currentOffset.x + width;
        (*base++) = currentOffset.y;
        (*base++) = uvs.u2;
        (*base++) = uvs.v1;
        (*base++) = c.topColour.x;
        (*base++) = c.topColour.y;
        (*base++) = c.topColour.z;
        (*base++) = c.topAlpha;

        (*base++) = currentOffset.x;
        (*base++) = currentOffset.y - height;
        (*base++) = uvs.u1;
        (*base++) = uvs.v2;
        (*base++) = c.botColour.x;
        (*base++) = c.botColour.y;
        (*base++) = c.botColour.z;
        (*base++) = c.botAlpha;

        (*base++) = currentOffset.x + width;
        (*base++) = currentOffset.y - height;
        (*base++) = uvs.u2;
        (*base++) = uvs.v2;
        (*base++) = c.botColour.x;
        (*base++) = c.botColour.y;
        (*base++) = c.botColour.z;
        (*base++) = c.botAlpha;
    }

    {
        rawIBuf.resize(rawIBuf.size() + 6);
        uint16_t *base = &rawIBuf[currentSize * 6];
        (*base++) = currentSize*4 + 0;
        (*base++) = currentSize*4 + 2;
        (*base++) = currentSize*4 + 1;
        (*base++) = currentSize*4 + 1;
        (*base++) = currentSize*4 + 2;
        (*base++) = currentSize*4 + 3;
    }

    currentSize++;
    currentOffset.x += width;

    if (currentOffset.x > currentDimensions.x) currentDimensions.x = currentOffset.x;
    if (-currentOffset.y + font->getHeight() > currentDimensions.y) currentDimensions.y = -currentOffset.y + font->getHeight();

    dirty = true;
}

void GfxTextBuffer::addTab (void)
{
    //TODO: wrap
    GfxFont::CharRect uvs;
    bool r = font->getCodePointOrFail(' ', uvs);
    if (!r) return;
    Vector2 tex_dim = font->getTextureDimensions();
    float width = fabsf(uvs.u2 - uvs.u1) * tex_dim.x;

    int tab_width = width*9;
    currentOffset.x = (currentOffset.x + (tab_width-1))/tab_width*tab_width;
}

void GfxTextBuffer::addRawWord (const std::vector<Char> &word)
{
    if (word.size() == 0) return;
    if (wrap.y != 0 && -currentOffset.y + font->getHeight() > wrap.y) return;
    unsigned letters = wordFit(word);
    if (letters == word.size()) {
        for (unsigned j=0 ; j<letters ; ++j) {
            addRawChar(word[j]);
        }
    } else {
        if (currentOffset.x == 0) {
            // no point wrapping, just add what we can
            for (unsigned j=0 ; j<letters ; ++j) {
                addRawChar(word[j]);
            }
            std::vector<Char> rest;
            for (unsigned j=letters ; j<word.size() ; ++j) {
                rest.push_back(word[j]);
            }
            addRawWord(rest);
        } else {
            endLine();
            addRawWord(word);
        }
    }
}

void GfxTextBuffer::endLine (void)
{
    currentOffset.x = 0;
    currentOffset.y -= font->getHeight();
}

void GfxTextBuffer::addFormattedString (const std::string &text,
                                        const Vector3 top_colour, float top_alpha,
                                        const Vector3 bot_colour, float bot_alpha)
{
    Char c;
    c.topColour = top_colour;
    c.topAlpha = top_alpha;
    c.botColour = bot_colour;
    c.botAlpha = bot_alpha;

    Vector3 ansi_colour[] = {
        Vector3(0,0,0), Vector3(.8,0,0), Vector3(0,.8,0), Vector3(.8,.8,0),
        Vector3(0,0,.93), Vector3(.8,0,.8), Vector3(0,.8,.8), Vector3(.75,.75,.75),
    };
    Vector3 ansi_bold_colour[] = {
        Vector3(0.5,0.5,0.5), Vector3(1,0,0), Vector3(0,1,0), Vector3(1,1,0),
        Vector3(.36,.36,1), Vector3(1,0,1), Vector3(0,1,1), Vector3(1,1,1),
    };

    bool bold = false;
    unsigned last_colour = 0;
    std::vector<Char> word;
    for (size_t i=0 ; i<text.length() ; ++i) {
        GfxFont::codepoint_t cp = decode_utf8(text, i);
        if (cp == '\n') {
            addRawWord(word);
            word.clear();
            endLine();
            continue;
        } else if (cp == ' ') {
            addRawWord(word);
            word.clear();
            c.cp = cp;
            // use (abuse) addRawWord because it handles wrapping...
            std::vector<Char> chars;
            chars.push_back(c);
            addRawWord(chars);
            continue;
        } else if (cp == '\t') {
            addRawWord(word);
            word.clear();
            addTab();
            continue;
        } else if (cp == UNICODE_ESCAPE_CODEPOINT) {
            cp = decode_utf8(text, ++i);
            if (cp == '[') {
                // continue until 'm'
                unsigned code = 0;
                do {
                    cp = decode_utf8(text, ++i);
                    if (cp == 'm' || cp == ';') {
                        // act on code
                        if (code == 0) {
                            bold = false;
                            last_colour = 0;
                            c.topColour = top_colour;
                            c.botColour = bot_colour;
                        } else if (code == 1) {
                            bold = true;
                            Vector3 col = bold ? ansi_bold_colour[last_colour] : ansi_colour[last_colour];
                            c.topColour = col;
                            c.botColour = col;
                        } else if (code == 22) {
                            bold = false;
                            Vector3 col = bold ? ansi_bold_colour[last_colour] : ansi_colour[last_colour];
                            c.topColour = col;
                            c.botColour = col;
                        } else if (code >= 30 && code <= 37) {
                            last_colour = code - 30;
                            Vector3 col = bold ? ansi_bold_colour[last_colour] : ansi_colour[last_colour];
                            c.topColour = col;
                            c.botColour = col;
                        }
                        code = 0;
                    }
                    if (cp == 'm') break;
                    if (cp >= '0' && cp <= '9') {
                        unsigned digit = cp - '0';
                        code = 10*code + digit;
                    }
                } while (true);
                if (cp == 'm') continue;
            }
        }

        // printable char

        if (font->hasCodePoint(cp)) {
            c.cp = cp;
            word.push_back(c);
        } else if (font->hasCodePoint(UNICODE_ERROR_CODEPOINT)) { // draw an error char
            c.cp = UNICODE_ERROR_CODEPOINT;
            word.push_back(c);
        } else if (font->hasCodePoint(' ')) { // try a space...
            c.cp = ' ';
            word.push_back(c);
        } else {
            // just display nothing
        }
    }
    addRawWord(word);
    word.clear();
}
