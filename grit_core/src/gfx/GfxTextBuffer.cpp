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

#include "GfxTextBuffer.h"

#include "../unicode_util.h"

const unsigned VERT_FLOAT_SZ = 2+2+4;
const unsigned VERT_BYTE_SZ = VERT_FLOAT_SZ*sizeof(float);

GfxTextBuffer::GfxTextBuffer (GfxFont *font)
  : font(font), currentOffset(0,0), dirty(false)
{   
    APP_ASSERT(font != NULL);

    reserve(0);

    vBuf.setNull();
    iBuf.setNull();
    vData.vertexBufferBinding->setBinding(0, vBuf);
    vData.vertexStart = 0;
    vData.vertexCount = 0;

    iData.indexBuffer = iBuf;
    iData.indexStart = 0;
    iData.indexCount = 0;
    currentGPUCapacity = 0;

    vData.vertexDeclaration = &vDecl;
    unsigned vdecl_sz = 0;
    vdecl_sz += vDecl.addElement(1, vdecl_sz, Ogre::VET_FLOAT2, Ogre::VES_POSITION, 1).getSize();
    vdecl_sz += vDecl.addElement(1, vdecl_sz, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0).getSize(); // uv
    vdecl_sz += vDecl.addElement(1, vdecl_sz, Ogre::VET_FLOAT4, Ogre::VES_TEXTURE_COORDINATES, 1).getSize(); // colour
    APP_ASSERT(vdecl_sz == VERT_BYTE_SZ);
}

void GfxTextBuffer::reserve (unsigned new_capacity)
{
    unsigned vertex_capacity = new_capacity * 4; // 4 verts per quad
    unsigned index_capacity = new_capacity * 6; // 2 triangles per quad

    rawVBuf.reserve(vertex_capacity * VERT_BYTE_SZ / sizeof(float));
    rawIBuf.reserve(index_capacity);

    currentCapacity = new_capacity;
}

void GfxTextBuffer::copyToGPUIfNeeded (void)
{
    if (!dirty) return;
    dirty = false;

    unsigned vertex_capacity = currentCapacity * 4; // 4 verts per quad
    unsigned index_capacity = currentCapacity * 6; // 2 triangles per quad

    if (currentGPUCapacity < currentCapacity) {
        // resize needed

        vBuf.setNull();
        iBuf.setNull();

        vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                        VERT_BYTE_SZ,
                        vertex_capacity,
                        Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

        iBuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
                        Ogre::HardwareIndexBuffer::IT_16BIT,
                        index_capacity,
                        Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

        vData.vertexBufferBinding->setBinding(0, vBuf);
        iData.indexBuffer = iBuf;

        currentGPUCapacity = currentCapacity;
    }

    if (currentCapacity > 0) {
        // copy the whole thing every time (could optimise this if needed)
        vBuf->writeData(0, vertex_capacity*VERT_BYTE_SZ, &rawVBuf[0], true);
        iBuf->writeData(0, index_capacity*sizeof(unsigned short), &rawIBuf[0], true);
    }
    
    vData.vertexCount = currentSize * 4; // do not include slack at the end of reserved space
    iData.indexCount = currentSize * 6; // do not include slack at the end of reserved space
}

bool GfxTextBuffer::addRawChar (GfxFont::codepoint_t cp, Vector3 top_colour, float top_alpha, Vector3 bot_colour, float bot_alpha)
{
    // resize if necessary
    if (currentSize + 1 > currentCapacity) reserve(currentSize+100);

    GfxFont::CharRect uvs;
    bool r = font->getCodePointOrFail(cp, uvs);
    if (!r) {
        // font didn't have this char
        return false;
    }
    Vector2 tex_dim = font->getTextureDimensions();
    float width = fabsf(uvs.u2 - uvs.u1) * tex_dim.x;
    float height = fabsf(uvs.v2 - uvs.v1) * tex_dim.y;

    if (width == 0 || height == 0) {
        // invalid char rect
        return true;
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
        (*base++) = top_colour.x;
        (*base++) = top_colour.y;
        (*base++) = top_colour.z;
        (*base++) = top_alpha;

        (*base++) = currentOffset.x + width;
        (*base++) = currentOffset.y;
        (*base++) = uvs.u2;
        (*base++) = uvs.v1;
        (*base++) = top_colour.x;
        (*base++) = top_colour.y;
        (*base++) = top_colour.z;
        (*base++) = top_alpha;

        (*base++) = currentOffset.x;
        (*base++) = currentOffset.y - height;
        (*base++) = uvs.u1;
        (*base++) = uvs.v2;
        (*base++) = bot_colour.x;
        (*base++) = bot_colour.y;
        (*base++) = bot_colour.z;
        (*base++) = bot_alpha;

        (*base++) = currentOffset.x + width;
        (*base++) = currentOffset.y - height;
        (*base++) = uvs.u2;
        (*base++) = uvs.v2;
        (*base++) = bot_colour.x;
        (*base++) = bot_colour.y;
        (*base++) = bot_colour.z;
        (*base++) = bot_alpha;
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
    dirty = true;

    return true;
}

void GfxTextBuffer::endLine (float line_height)
{
    currentOffset.x = 0;
    currentOffset.y -= line_height;
}

void GfxTextBuffer::addFormattedString (const std::string &text)
{
    // TODO: colour codes
    // TODO: \n
    Vector3 colour(1,1,1);
    float alpha = 1;
    for (unsigned i=0 ; i<text.length() ; ) {
        GfxFont::codepoint_t cp = decode_utf8 (text, i);
        bool r = addRawChar(cp, colour, alpha, colour, alpha);
        if (!r) {
            // try the error char
            r = addRawChar(0xfffd, colour, alpha, colour, alpha);
            if (!r) {
                // try a space...
                r = addRawChar(' ', colour, alpha, colour, alpha);
                if (!r) {
                    // just display nothing
                }
            }
        }
    }
}

