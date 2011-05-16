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
This source file is a based on a part of OGRE
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

#ifndef TextListOverlayElement_h
#define TextListOverlayElement_h

#include <OgrePrerequisites.h>
#include <OgreOverlayElement.h>
#include <OgreFont.h>

namespace HUD {


        class TextListOverlayElement : public Ogre::OverlayElement
        {
                public:

                enum Alignment { Left, Right, Center };

                /** Constructor. */
                TextListOverlayElement (const Ogre::String& name);
                virtual ~TextListOverlayElement();

                virtual void initialise(void);
                virtual void setCaption(const Ogre::DisplayString& s);

                virtual void resetCaption();
                virtual void commitCaption();
                virtual void appendCaption(const Ogre::DisplayString& s);

                virtual void setCharHeight( Ogre::Real height );
                virtual Ogre::Real getCharHeight() const;

                virtual Ogre::Real getWidth();
                virtual Ogre::Real getHeight();

                virtual void setWidth(Ogre::Real width);
                virtual void setHeight(Ogre::Real height);
                virtual void setDimensions(Ogre::Real width, Ogre::Real height);


                virtual void setFontName( const Ogre::String& font );
                virtual const Ogre::String& getFontName() const;

                /** See OverlayElement. */
                virtual const Ogre::String& getTypeName(void) const;
                /** See Renderable. */
                virtual void getRenderOperation(Ogre::RenderOperation& op);
                /** Overridden from OverlayElement */

                virtual void setColourBottom(const Ogre::ColourValue& col);
                /** Gets the colour of the bottom of the letters. */
                virtual const Ogre::ColourValue& getColourBottom(void) const;
                /** Sets the colour of the top of the letters.
                @remarks
                        By setting a separate top and bottom colour, you
                        can create a text area which has a graduated colour
                        effect to it.
                */
                virtual void setColourTop(const Ogre::ColourValue& col);
                /** Gets the colour of the top of the letters. */
                virtual const Ogre::ColourValue& getColourTop(void) const;

                virtual inline void setAlignment( Alignment a )
                {
                        mAlignment = a;
                        mGeomPositionsOutOfDate = true;
                }
                virtual inline Alignment getAlignment() const
                {
                        return mAlignment;
                }

                /** Overridden from OverlayElement */
                virtual void setMetricsMode(Ogre::GuiMetricsMode gmm);

                /** Overridden from OverlayElement */
                virtual void _update(void);

                static const Ogre::String msTypeName;

        protected:

                bool mDimensionsOutOfDate;
                /// The text alignment
                Alignment mAlignment;

                /// Render operation
                Ogre::RenderOperation mRenderOp;


                Ogre::FontPtr mpFont;
                Ogre::Real mCharHeight;
                Ogre::ushort mPixelCharHeight;
                size_t mAllocSize;
                Ogre::Real mViewportAspectCoef;

                /// The last colour set
                Ogre::ColourValue mRunningColourBottom;
                Ogre::ColourValue mRunningColourTop;

                /// Colours to use for the vertices (per character)
                std::vector<Ogre::ColourValue> mColoursBottom;
                std::vector<Ogre::ColourValue> mColoursTop;

                /// Internal method to allocate memory
                /// (only reallocates when necessary)
                void checkMemoryAllocation( size_t numChars );

                /// Inherited function
                virtual void updateDimensions();
                /// Inherited function
                virtual void updatePositionGeometry();
                /// Inherited function
                virtual void updateTextureGeometry();

                void setDimensions_(Ogre::Real w, Ogre::Real h);

        };
}

#endif


// vim: shiftwidth=8:tabstop=8:expandtab
