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

#include <unicode_util.h>

#include "gfx_text_buffer.h"

const unsigned VERT_FLOAT_SZ = 2+2+4;
const unsigned VERT_BYTE_SZ = VERT_FLOAT_SZ*sizeof(float);

/** If the font doesn't have the codepoint, try some alternatives. */
static GfxFont::codepoint_t override_cp (GfxFont *font, GfxFont::codepoint_t cp) 
{
    if (font->hasCodePoint(cp)) return cp;
    if (font->hasCodePoint(UNICODE_ERROR_CODEPOINT)) return UNICODE_ERROR_CODEPOINT;
    if (font->hasCodePoint(' ')) return ' ';
    if (font->hasCodePoint('E')) return 'E';
    return cp;
}

static bool is_whitespace (GfxFont::codepoint_t cp)
{
    return cp==' ' || cp=='\n' || cp=='\t';
}

GfxTextBuffer::GfxTextBuffer (GfxFont *font)
  : font(font), currentDrawnDimensions(0,0), currentLeft(0), currentTop(0),
    lastTop(0), lastBottom(0), wrap(0), dirty(false)
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

// searching for the i such that cT[i-1].top < top && cT[i].top >= top, or 0 if cT[0] >= top
unsigned long GfxTextBuffer::binaryChopTop (unsigned long begin, unsigned long end, unsigned long top)
{
    unsigned long middle = (end + begin + 1) / 2;
    if (middle == 0) return middle;
    if (middle == colouredText.size() - 1) return middle+1;
    const ColouredChar &c = colouredText[middle];
    const ColouredChar &b = colouredText[middle-1];
    if (c.top < top) {
        // too close to the beginning of the text, look right
        APP_ASSERT(begin != middle); // this leads to infinite recursion
        return binaryChopTop(middle, end, top);
    } else if (b.top < top) {
        // found it
        return middle;
    } else {
        // too close to the end of the text, look left
        return binaryChopTop(begin, middle-1, top);
    }
}

// searching for the i such that cT[i].top <= top && cT[i+1].top > top, or sz-1 if cT[sz-1] < top
unsigned long GfxTextBuffer::binaryChopBottom (unsigned long begin, unsigned long end, unsigned long bottom_top)
{   
    unsigned long middle = (end + begin) / 2;
    if (middle == colouredText.size() - 1) return middle;
    const ColouredChar &c = colouredText[middle];
    const ColouredChar &d = colouredText[middle+1];
    if (c.top > bottom_top) {
        // too close to the end of the text, look left
        APP_ASSERT(middle != end); // this leads to infinite recursion
        return binaryChopBottom(begin, middle, bottom_top);
    } else if (d.top > bottom_top) {
        // found it
        return middle;
    } else {
        // too close to the beginning of the text, look right
        return binaryChopBottom(middle+1, end, bottom_top);
    }
}

void GfxTextBuffer::updateGPU (bool no_scroll, long top, long bottom)
{
    if (lastTop != top || lastBottom != bottom) dirty = true;
    if (!dirty) return;

    currentDrawnDimensions = Vector2(0,0);
    rawVBuf.clear();
    rawIBuf.clear();

    unsigned long start_index = 0;
    unsigned long stop_index = colouredText.size();
    float zero = 0;

    if (!no_scroll && colouredText.size() > 0) {
        long bottom_top = bottom - long(font->getHeight());
        /* find start and stop */
        if (bottom_top < 0) {
            start_index = 0;
            stop_index = 0;
        } else {
            start_index = binaryChopTop(0, colouredText.size() - 1, std::max(0l, top));
            stop_index = 1+binaryChopBottom(0, colouredText.size() - 1, std::max(0l, bottom_top));
            zero = top;
        }
    }

    unsigned long current_size = 0;

    for (unsigned long i=start_index ; i<stop_index ; ++i) {
        const ColouredChar &c = colouredText[i];
        if (is_whitespace(c.cp)) {
            continue;
        }

        GfxFont::CharRect uvs;
        GfxFont::codepoint_t cp = override_cp(font, c.cp);
        bool r = font->getCodePointOrFail(cp, uvs);
        if (!r) continue;
        Vector2 tex_dim = font->getTextureDimensions();
        float width = uvs.u2 - uvs.u1;
        float height = uvs.v2 - uvs.v1;

        if (width == 0 || height == 0) {
            // invalid char rect in font -- indicates char should not be rendered
            continue;
        }

        // use font, add to raw buf
        /* 0---1
           |  /|
           | / |
           |/  |
           2---3   indexes: 0 2 1  1 2 3
         */
        {
            // 4 because there are 4 vertexes per letter
            rawVBuf.resize(rawVBuf.size() + VERT_FLOAT_SZ*4);
            float *base = &rawVBuf[current_size * VERT_FLOAT_SZ*4];

            (*base++) = c.left;
            (*base++) = -(c.top - zero);
            (*base++) = uvs.u1 / tex_dim.x;
            (*base++) = uvs.v1 / tex_dim.y;
            (*base++) = c.topColour.x;
            (*base++) = c.topColour.y;
            (*base++) = c.topColour.z;
            (*base++) = c.topAlpha;

            (*base++) = c.left + width;
            (*base++) = -(c.top - zero);
            (*base++) = uvs.u2 / tex_dim.x;
            (*base++) = uvs.v1 / tex_dim.y;
            (*base++) = c.topColour.x;
            (*base++) = c.topColour.y;
            (*base++) = c.topColour.z;
            (*base++) = c.topAlpha;

            (*base++) = c.left;
            (*base++) = -(c.top - zero + height);
            (*base++) = uvs.u1 / tex_dim.x;
            (*base++) = uvs.v2 / tex_dim.y;
            (*base++) = c.bottomColour.x;
            (*base++) = c.bottomColour.y;
            (*base++) = c.bottomColour.z;
            (*base++) = c.bottomAlpha;

            (*base++) = c.left + width;
            (*base++) = -(c.top - zero + height);
            (*base++) = uvs.u2 / tex_dim.x;
            (*base++) = uvs.v2 / tex_dim.y;
            (*base++) = c.bottomColour.x;
            (*base++) = c.bottomColour.y;
            (*base++) = c.bottomColour.z;
            (*base++) = c.bottomAlpha;
        }

        {
            // 6 because there are 2 triangles per letter
            rawIBuf.resize(rawIBuf.size() + 6);
            uint16_t *base = &rawIBuf[current_size * 6];
            (*base++) = current_size*4 + 0;
            (*base++) = current_size*4 + 2;
            (*base++) = current_size*4 + 1;
            (*base++) = current_size*4 + 1;
            (*base++) = current_size*4 + 2;
            (*base++) = current_size*4 + 3;
        }

        current_size++;

        // calculate text bounds
        currentDrawnDimensions.x = std::max(currentDrawnDimensions.x, float(c.left + width));
        currentDrawnDimensions.y = std::max(currentDrawnDimensions.y, float(c.top + font->getHeight()));

    }


    // do the actual copy to GPU (resize if necessary)

    unsigned vertex_size = current_size * 4; // 4 verts per quad
    unsigned index_size = current_size * 6; // 2 triangles per quad

    if (currentGPUCapacity < current_size) {
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

        currentGPUCapacity = current_size;
    }

    if (current_size > 0) {
        // copy the whole thing every time (could optimise this if needed)
        vBuf->writeData(0, vertex_size*VERT_BYTE_SZ, &rawVBuf[0], true);
        iBuf->writeData(0, index_size*sizeof(uint16_t), &rawIBuf[0], true);
    }
    
    vData.vertexCount = vertex_size;
    iData.indexCount = index_size;

    dirty = false;
    lastTop = top;
    lastBottom = bottom;
}

void GfxTextBuffer::recalculatePositions (unsigned long start)
{
    if (start == 0) {
        currentLeft = 0;
        currentTop = 0;
    }
    unsigned word_first_letter = 0;
    bool in_word = false;
    bool word_first_on_line = true;
    for (unsigned long i=start ; i<colouredText.size() ; ++i) {
        if (!in_word) {
            word_first_letter = i;
            in_word = true;
        }
        ColouredChar &c = colouredText[i];
        if (is_whitespace(c.cp)) {
            in_word = false;
            word_first_on_line = false;
        }

        c.left = currentLeft;
        c.top = currentTop;

        switch (c.cp) {
            case '\n':
            word_first_on_line = true;
            currentLeft = 0;
            currentTop += font->getHeight();
            break;

            case '\t': {
                GfxFont::CharRect uvs;
                if (font->getCodePointOrFail(' ', uvs)) {
                    // TODO: override tab width per textbuffer?
                    unsigned long tab_width = 8 * (uvs.u2 - uvs.u1);
                    // round up to next multiple of tab_width
                    if (tab_width > 0)
                        currentLeft = (currentLeft + tab_width)/tab_width * tab_width;
                }
            }
            break;

            default: {
                GfxFont::CharRect uvs;
                GfxFont::codepoint_t cp = override_cp(font, c.cp);
                if (!font->getCodePointOrFail(cp, uvs)) continue;
                float width = (uvs.u2 - uvs.u1);
                currentLeft += width;
                if (c.cp != ' ') {
                }
            }
        }

        if (wrap>0 && currentLeft>wrap) {
            if (c.cp == '\t') {
                // let the tab fill up the remainder of the line
                // continue on the next line
            } else if (c.cp == ' ') {
                // let the space take us to the next line
            } else if (c.left == 0) {
                // break at next char, wasn't even enough space for 1 char
            } else if (word_first_on_line) {
                // break at char
                i--;
            } else {
                // break at word
                i = word_first_letter - 1;
            }
            in_word = false;
            word_first_on_line = true;
            currentLeft = 0;
            currentTop += font->getHeight();
            continue;
        }

    }
    dirty = true;
}

void GfxTextBuffer::addFormattedString (const std::string &text,
                                        const Vector3 &top_colour, float top_alpha,
                                        const Vector3 &bot_colour, float bot_alpha)
{
    Vector3 ansi_colour[] = {
        Vector3(0,0,0), Vector3(.8,0,0), Vector3(0,.8,0), Vector3(.8,.8,0),
        Vector3(0,0,.93), Vector3(.8,0,.8), Vector3(0,.8,.8), Vector3(.75,.75,.75),
    };
    Vector3 ansi_bold_colour[] = {
        Vector3(0.5,0.5,0.5), Vector3(1,0,0), Vector3(0,1,0), Vector3(1,1,0),
        Vector3(.36,.36,1), Vector3(1,0,1), Vector3(0,1,1), Vector3(1,1,1),
    };

    ColouredChar c(0, top_colour, top_alpha, bot_colour, bot_alpha);

    unsigned long original_size = colouredText.size();

    bool bold = false;
    unsigned default_colour = 7; // FIXME: this assumes a 'white on black' console
    unsigned last_colour = default_colour;
    for (size_t i=0 ; i<text.length() ; ++i) {
        GfxFont::codepoint_t cp = decode_utf8(text, i);
        if (cp == UNICODE_ESCAPE_CODEPOINT) {
            if (++i >= text.length()) break;
            cp = decode_utf8(text, i);
            if (cp == '[') {
                // continue until 'm'
                unsigned code = 0;
                do {
                    if (++i >= text.length()) break;
                    cp = decode_utf8(text, i);
                    if (cp == 'm' || cp == ';') {
                        // act on code
                        if (code == 0) {
                            bold = false;
                            last_colour = default_colour;
                            c.topColour = top_colour;
                            c.bottomColour = bot_colour;
                        } else if (code == 1) {
                            bold = true;
                            Vector3 col = bold ? ansi_bold_colour[last_colour] : ansi_colour[last_colour];
                            c.topColour = col;
                            c.bottomColour = col;
                        } else if (code == 22) {
                            bold = false;
                            Vector3 col = bold ? ansi_bold_colour[last_colour] : ansi_colour[last_colour];
                            c.topColour = col;
                            c.bottomColour = col;
                        } else if (code >= 30 && code <= 37) {
                            last_colour = code - 30;
                            Vector3 col = bold ? ansi_bold_colour[last_colour] : ansi_colour[last_colour];
                            c.topColour = col;
                            c.bottomColour = col;
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
        
        // This char is not part of an ansi terminal colour code.
        c.cp = cp;
        colouredText.push_back(c);
    }

    recalculatePositions(original_size);
}
