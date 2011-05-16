/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

/*-------------------------------------------------------------------------
This source file is based on a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/


#include <OgreStableHeaders.h>

#include <OgreRoot.h>
#include <OgreLogManager.h>
#include <OgreOverlayManager.h>
#include <OgreHardwareBufferManager.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreException.h>
#include <OgreStringConverter.h>
#include <OgreFont.h>
#include <OgreFontManager.h>

#include "TextListOverlayElement.h"
#include "main.h"
#include "gfx.h"

namespace HUD {

        //---------------------------------------------------------------------
        const Ogre::String TextListOverlayElement::msTypeName = "TextList";
        //---------------------------------------------------------------------

        #define UNICODE_NEL 0x85 // some crazy fucked up shit
        #define UNICODE_CR 0x0D // '\r'
        #define UNICODE_LF 0x0A // '\n'
        #define UNICODE_SPACE 0x20 // ' '
        #define UNICODE_ZERO 0x30 // '0'

        //---------------------------------------------------------------------
        TextListOverlayElement::TextListOverlayElement(const Ogre::String& name)
                : Ogre::OverlayElement(name)
        {
                mDimensionsOutOfDate = true;
                mAlignment = Left;

                mRunningColourTop = Ogre::ColourValue::White;
                mRunningColourBottom = Ogre::ColourValue::White;

                resetCaption();

                mAllocSize = 0;

                mCharHeight = 0.02;
                mPixelCharHeight = 13;
                mViewportAspectCoef = 1;

        }

        void TextListOverlayElement::resetCaption(void)
        {
                mCaption.clear();
                mColoursTop.clear();
                mColoursBottom.clear();
        }

        void TextListOverlayElement::commitCaption(void)
        {
                mDimensionsOutOfDate = true; //width and height have changed
                _positionsOutOfDate();
        }

        void TextListOverlayElement::appendCaption (const Ogre::DisplayString&s)
        {
                mCaption.append(s);
                mColoursTop.reserve(mCaption.size());
                mColoursBottom.reserve(mCaption.size());
                for (size_t i = 0 ; i<s.size() ; i++) {
                        mColoursTop.push_back(mRunningColourTop);
                        mColoursBottom.push_back(mRunningColourBottom);
                }
        }

        void TextListOverlayElement::initialise(void)
        {

                if (!mInitialised) {
                        // Set up the render op Combine positions and texture
                        // coords since they tend to change together since
                        // character sizes are different

                        mRenderOp.vertexData = new Ogre::VertexData();
                        Ogre::VertexDeclaration* decl =
                                        mRenderOp.vertexData->vertexDeclaration;
                        size_t offset = 0;
                        // Positions
                        decl->addElement(0, offset,
                                         Ogre::VET_FLOAT3, Ogre::VES_POSITION);
                        offset +=
                             Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

                        // Colours
                        decl->addElement(0, offset,
                                         Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
                        offset +=
                             Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR);

                        // Texcoords
                        decl->addElement(0, offset,
                                         Ogre::VET_FLOAT2,
                                         Ogre::VES_TEXTURE_COORDINATES,0);
                        offset +=
                             Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);


                        mRenderOp.operationType =
                                Ogre::RenderOperation::OT_TRIANGLE_LIST;
                        mRenderOp.useIndexes = false;
                        mRenderOp.vertexData->vertexStart = 0;

                        // Vertex buffer will be created in
                        // checkMemoryAllocation

                        checkMemoryAllocation (12);

                        mInitialised = true;
                }

        }

        void TextListOverlayElement::checkMemoryAllocation (size_t numChars)
        {
                if (mAllocSize < numChars) {

                        // Create and bind new buffers Note that old buffers
                        // will be deleted automatically through reference
                        // counting

                        // 6 verts per char since we're doing tri lists without
                        // indexes Allocate space for positions & texture
                        // coords

                        Ogre::VertexDeclaration* decl =
                                mRenderOp.vertexData->vertexDeclaration;
                        Ogre::VertexBufferBinding* bind =
                                mRenderOp.vertexData->vertexBufferBinding;

                        mRenderOp.vertexData->vertexCount = numChars * 6;

                        // Create dynamic since text tends to change alot
                        // positions & texcoords

                        Ogre::HardwareBufferManager& mgr =
                                Ogre::HardwareBufferManager::getSingleton();

                        Ogre::HardwareVertexBufferSharedPtr vbuf;
                        vbuf = mgr.createVertexBuffer(
                                  decl->getVertexSize(0),
                                  mRenderOp.vertexData->vertexCount,
                                  Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
                        bind->setBinding(0, vbuf);

                        mAllocSize = numChars;

                }

        }

        inline void setVertexColour (float *& vertex_ptr,
                                     const Ogre::ColourValue& cv)
        {
                Ogre::Root::getSingleton().convertColourValue(
                        cv,
                        reinterpret_cast<Ogre::RGBA*> (vertex_ptr)
                );
                vertex_ptr += sizeof (Ogre::RGBA) / sizeof(*vertex_ptr);
        }


        void TextListOverlayElement::updatePositionGeometry()
        {
                if (mpFont.isNull()) {
                        // not initialised yet, probably due to the order of
                        // creation in a template
                        return;
                }

                float *pVert;

                size_t charlen = mCaption.size();
                checkMemoryAllocation (charlen);

                mRenderOp.vertexData->vertexCount = charlen * 6;
                // Get position / texcoord buffer
                Ogre::HardwareVertexBufferSharedPtr vbuf =
                        mRenderOp.vertexData->
                                vertexBufferBinding->getBuffer(0);
                pVert = static_cast<float*> ( 
                        vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

                float left = _getDerivedLeft() * 2.0 - 1.0;
                float top = -( (_getDerivedTop() * 2.0 ) - 1.0 );

                // Use iterator
                Ogre::DisplayString::iterator i, iend = mCaption.end();
                bool new_line = true;
                for (i = mCaption.begin() ; i!=iend ; ++i) {
                        if (new_line) {
                                // calculate length of line first
                                Ogre::Real len = 0.0f;
                                for (Ogre::DisplayString::iterator j=i ;
                                     j!=iend ; j++){
                                        Ogre::Font::CodePoint character =
                                           OGRE_DEREF_DISPLAYSTRING_ITERATOR(j);
                                        if (character == UNICODE_CR
                                                || character == UNICODE_NEL
                                                || character == UNICODE_LF) {
                                                break;
                                        } else {
                                                Ogre::Real v;
                                                v = mpFont->getGlyphAspectRatio
                                                        (character);
                                                v *= mCharHeight * 2.0;
                                                v *= mViewportAspectCoef;
                                                len += v;
                                        }
                                }

                                switch (mAlignment) {
                                        case Right: left -= len * 0.5;
                                        case Center: left -= len * 0.5;
                                        case Left: break;
                                }

                                new_line = false;
                        }

                        Ogre::Font::CodePoint character =
                                OGRE_DEREF_DISPLAYSTRING_ITERATOR(i);
                        size_t index = i - mCaption.begin();

                        if (character==UNICODE_CR || character==UNICODE_NEL
                                                  || character==UNICODE_LF) {
                                left = _getDerivedLeft() * 2.0 - 1.0;
                                top -= mCharHeight * 2.0;
                                new_line = true;
                                // Also reduce tri count
                                mRenderOp.vertexData->vertexCount -= 6;

                                // consume CR/LF in one
                                if (character == UNICODE_CR) {
                                        Ogre::DisplayString::iterator peeki = i;
                                        peeki++;
                                        if (peeki != iend &&
                       OGRE_DEREF_DISPLAYSTRING_ITERATOR(peeki) == UNICODE_LF) {
                                                // skip both as one newline
                                                i = peeki;
                                                // Also reduce tri count
                                                mRenderOp.vertexData
                                                        ->vertexCount -= 6;
                                        }

                                }
                                continue;
                        }

                        Ogre::Real horiz_height = mpFont->
                             getGlyphAspectRatio(character)*mViewportAspectCoef;
                        const Ogre::Font::UVRect& uvRect =
                                mpFont->getGlyphTexCoords(character);

                        // each vert is (x, y, z, u, v)
                        //------------------------------------------------------
                        // First tri
                        //
                        // Upper left
                        *pVert++ = left;
                        *pVert++ = top;
                        *pVert++ = -1.0;
                        setVertexColour(pVert,mColoursTop[index]);
                        *pVert++ = uvRect.left;
                        *pVert++ = uvRect.top;

                        // Bottom left
                        *pVert++ = left;
                        *pVert++ = top - mCharHeight * 2.0;
                        *pVert++ = -1.0;
                        setVertexColour(pVert,mColoursBottom[index]);
                        *pVert++ = uvRect.left;
                        *pVert++ = uvRect.bottom;

                        // Top right
                        *pVert++ = left + horiz_height * mCharHeight * 2.0;
                        *pVert++ = top;
                        *pVert++ = -1.0;
                        setVertexColour(pVert,mColoursTop[index]);
                        *pVert++ = uvRect.right;
                        *pVert++ = uvRect.top;
                        //------------------------------------------------------

                        //------------------------------------------------------
                        // Second tri
                        //
                        // Top right (again)
                        *pVert++ = left + horiz_height * mCharHeight * 2.0;
                        *pVert++ = top;
                        *pVert++ = -1.0;
                        setVertexColour(pVert,mColoursTop[index]);
                        *pVert++ = uvRect.right;
                        *pVert++ = uvRect.top;


                        // Bottom left (again)
                        *pVert++ = left;
                        *pVert++ = top - mCharHeight * 2.0;
                        *pVert++ = -1.0;
                        setVertexColour(pVert,mColoursBottom[index]);
                        *pVert++ = uvRect.left;
                        *pVert++ = uvRect.bottom;

                        // Bottom right
                        *pVert++ = left + horiz_height  * mCharHeight * 2.0;
                        *pVert++ = top - mCharHeight * 2.0;
                        *pVert++ = -1.0;
                        setVertexColour(pVert,mColoursBottom[index]);
                        *pVert++ = uvRect.right;
                        *pVert++ = uvRect.bottom;
                        //------------------------------------------------------

                        // move to next character
                        left += horiz_height  * mCharHeight * 2.0;

                }
                // Unlock vertex buffer
                vbuf->unlock();

        }


        void TextListOverlayElement::updateDimensions()
        {
                if (mpFont.isNull()) {
                        // not initialised yet, probably due to the order of
                        // creation in a template
                        return;
                }

                float largestWidth = 0;

                // Use iterator
                Ogre::DisplayString::iterator i, iend = mCaption.end();
                unsigned int num_lines = 0;
                for (i = mCaption.begin() ; i!=iend ; ++i) {
                        // calculate length of line first
                        Ogre::Real len = 0.0f;
                        for (; i!=iend ; ++i) {
                                Ogre::Font::CodePoint character =
                                   OGRE_DEREF_DISPLAYSTRING_ITERATOR(i);
                                if (character == UNICODE_CR
                                        || character == UNICODE_NEL
                                        || character == UNICODE_LF) {
                                        break;
                                } else {
                                        Ogre::Real v;
                                        v = mpFont->getGlyphAspectRatio
                                                (character);
                                        v *= mCharHeight;
                                        v *= mViewportAspectCoef;
                                        len += v;
                                }
                        }

                        num_lines++;

                        if (largestWidth<len)
                                largestWidth = len;

                        if (i==iend)
                                break;

                }

                Ogre::Real textHeight = mCharHeight*num_lines;

/*
                std::cout << "mCharHeight: " << mCharHeight << std::endl;
                std::cout << "width: " << largestWidth << std::endl
                          << "height: " << textHeight << std::endl;
*/

                if (mMetricsMode == Ogre::GMM_PIXELS) {
                        // Derive pixel version of dimensions
                        Ogre::Real vpWidth = ogre_win->getWidth();
                        largestWidth *= vpWidth;
                        Ogre::Real vpHeight = ogre_win->getHeight();
                        textHeight *= vpHeight;
/*
                        std::cout << "vpWidth: " << vpWidth << std::endl
                                  << "vpHeight: " << vpHeight << std::endl;
                        std::cout << "width2: " << largestWidth << std::endl
                                  << "height2: " << textHeight << std::endl;
*/
                }


                setDimensions_(largestWidth,textHeight);
        }

        void TextListOverlayElement::updateTextureGeometry()
        {
                // Nothing to do, we combine positions and textures
        }

        void TextListOverlayElement::setCaption (const Ogre::DisplayString& s)
        {
                resetCaption();
                appendCaption(s);
                commitCaption();
        }

        void TextListOverlayElement::setFontName (const Ogre::String& font)
        {
                Ogre::FontPtr fp = Ogre::FontManager::getSingleton()
                                        .getByName(font);
                if (fp.isNull())
                        OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND,
                                    "Could not find font \""+font+"\"",
                                    "TextListOverlayElement::setFontName");

                mpFont = fp;
                mpFont->load();
                mpMaterial = mpFont->getMaterial();

                // the font should surely do this for itself?
                mpMaterial->setDepthCheckEnabled(false);

                // we supply our own lighting, which this presumably overrides
                mpMaterial->setLightingEnabled(false);

                mDimensionsOutOfDate = true;
                _positionsOutOfDate();
        }
        const Ogre::String& TextListOverlayElement::getFontName() const
        {
                return mpFont->getName();
        }

        void TextListOverlayElement::setCharHeight (Ogre::Real height)
        {
                if (mMetricsMode != Ogre::GMM_RELATIVE) {
                        mPixelCharHeight = static_cast<unsigned>(height);
                } else {
                        mCharHeight = height;
                }
                mDimensionsOutOfDate = true;
                _positionsOutOfDate();
        }
        Ogre::Real TextListOverlayElement::getCharHeight() const
        {
                if (mMetricsMode == Ogre::GMM_PIXELS) {
                        return mPixelCharHeight;
                } else {
                        return mCharHeight;
                }
        }

        //---------------------------------------------------------------------
        TextListOverlayElement::~TextListOverlayElement()
        {
                delete mRenderOp.vertexData;
        }
        //---------------------------------------------------------------------
        const Ogre::String& TextListOverlayElement::getTypeName(void) const
        {
                return msTypeName;
        }
        //---------------------------------------------------------------------
        void TextListOverlayElement::getRenderOperation (
                                                      Ogre::RenderOperation& op)
        {
                op = mRenderOp;
        }
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        void TextListOverlayElement::setColourBottom (
                                                const Ogre::ColourValue& col)
        {
                mRunningColourBottom = col;
        }
        //---------------------------------------------------------------------
        const Ogre::ColourValue&
        TextListOverlayElement::getColourBottom (void) const
        {
                return mRunningColourBottom;
        }
        //---------------------------------------------------------------------
        void TextListOverlayElement::setColourTop(const Ogre::ColourValue& col)
        {
                mRunningColourTop = col;
        }
        //---------------------------------------------------------------------
        const Ogre::ColourValue&
        TextListOverlayElement::getColourTop(void) const
        {
                return mRunningColourTop;
        }
        //---------------------------------------------------------------------
        void TextListOverlayElement::setMetricsMode(Ogre::GuiMetricsMode gmm)
        {
                Ogre::Real vpWidth, vpHeight;

                vpWidth = ogre_win->getWidth();
                vpHeight = ogre_win->getHeight();

                mViewportAspectCoef = vpHeight / vpWidth;

                Ogre::OverlayElement::setMetricsMode(gmm);
                if (gmm != Ogre::GMM_RELATIVE) {
                        // Set pixel variables based on viewport multipliers
                        mPixelCharHeight =
                                static_cast<unsigned>(mCharHeight * vpHeight);
                }
        }

        //----------------------------------------------------------------------

        void TextListOverlayElement::_update(void)
        {
                Ogre::Real vpWidth, vpHeight;

                vpWidth = ogre_win->getWidth();
                vpHeight = ogre_win->getHeight();

                mViewportAspectCoef = vpHeight/vpWidth;

                if (mMetricsMode != Ogre::GMM_RELATIVE &&
                    (Ogre::OverlayManager::getSingleton().hasViewportChanged()
                     || mGeomPositionsOutOfDate)) {

                        // Recalc character size
                        mCharHeight = (Ogre::Real) mPixelCharHeight / vpHeight;

                        _positionsOutOfDate();
                }

                OverlayElement::_update();

        }

        Ogre::Real TextListOverlayElement::getWidth()
        {
                if (!mDimensionsOutOfDate)
                        return Ogre::OverlayElement::getWidth();

                if (mMetricsMode!=Ogre::GMM_RELATIVE) {
                        Ogre::Real vpHeight;
                        vpHeight = ogre_win->getHeight();
                        mCharHeight = (Ogre::Real) mPixelCharHeight / vpHeight;
                }

                updateDimensions();

                return Ogre::OverlayElement::getWidth();
        }

        Ogre::Real TextListOverlayElement::getHeight()
        {
                if (!mDimensionsOutOfDate)
                        return Ogre::OverlayElement::getHeight();

                if (mMetricsMode!=Ogre::GMM_RELATIVE) {
                        Ogre::Real vpHeight;
                        vpHeight = ogre_win->getHeight();
                        mCharHeight = (Ogre::Real) mPixelCharHeight / vpHeight;
                }

                updateDimensions();

                return Ogre::OverlayElement::getHeight();
        }

        void TextListOverlayElement::setDimensions (Ogre::Real width,
                                                    Ogre::Real height) {
                (void) width;
                (void) height;
        }

        void TextListOverlayElement::setWidth(Ogre::Real width)
        { (void) width; }
        void TextListOverlayElement::setHeight(Ogre::Real height)
        { (void) height; }

        void TextListOverlayElement::setDimensions_(Ogre::Real w, Ogre::Real h)
        {
                //std::cout << "setDimensions_("<<w<<","<<h<<")" << std::endl;
                if (mMetricsMode != Ogre::GMM_RELATIVE) {
                        mPixelWidth = w;
                        mPixelHeight = h;
                } else {
                        mWidth = w;
                        mHeight = h;
                }
                mDimensionsOutOfDate = false;
        }

}

// vim: shiftwidth=8:tabstop=8:expandtab
