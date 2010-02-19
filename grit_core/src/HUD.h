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

#ifndef HUD_h
#define HUD_h

#include <vector>

#include <Ogre.h>
#include <OgrePanelOverlayElement.h>

#include "TextListOverlayElement.h"



namespace HUD {

        typedef Ogre::DisplayString DStr;

        class Overlay {
                public:

                Overlay ();
                virtual ~Overlay ();

                virtual Ogre::Overlay *getOverlay();

                protected:
                Ogre::Overlay *me;
        };

        typedef Ogre::SharedPtr<Overlay> OverlayPtr;
        typedef unsigned int dim_t;
        const dim_t DIM_T_MAX = UINT_MAX;

        class Tree;

        class Calculator {
                public:
                virtual ~Calculator();
                virtual void f(Tree*,dim_t,dim_t,
                               dim_t&,dim_t&,dim_t&,dim_t&) = 0;
        };

        inline Calculator::~Calculator() {}

        typedef Ogre::SharedPtr<Calculator> CalculatorPtr;

        // abstract base class
        class Tree {
                public:

                Tree(OverlayPtr o, dim_t pw, dim_t ph);
                virtual ~Tree();

                virtual void _recalculate();

                virtual void parentResized(dim_t w, dim_t h);

                virtual dim_t getWidth() const;
                virtual dim_t getHeight() const;

                virtual dim_t getPosX() const;
                virtual dim_t getPosY() const;

                virtual void setVisible (bool b);
                virtual bool isVisible () const;

                virtual void setCalculator (CalculatorPtr calculator);

                static Ogre::String nextName();

                virtual Ogre::OverlayElement *getMe() const = 0;

                virtual void orphan();

                protected:

                dim_t pw, ph; //parent's dimensions

                dim_t x, y, w, h; //our dimensions

                bool orphaned;

                OverlayPtr overlay;

                CalculatorPtr calculator;

                static int counter;
        };

        inline Tree::~Tree() { }

        typedef Ogre::SharedPtr<Tree> TreePtr;
        typedef std::vector<TreePtr> TreePtrs;

        class Pane : public Tree {
                public:

                Pane (OverlayPtr, dim_t pw, dim_t ph);
                virtual ~Pane ();

                virtual void removeChild (TreePtr);
                virtual void removeAllChildren ();
                virtual TreePtr createChild (Ogre::String type);

                virtual TreePtrs getChildren ();

                template <typename T>
                void createChild (const Ogre::String& name,
                                  Ogre::SharedPtr<T>& ptr)
                {
                        TreePtr t = createChild(name);
                        ptr = *(Ogre::SharedPtr<T>*)&t;
                }


                virtual void _recalculate ();


                // default is null (transparent)
                virtual Ogre::MaterialPtr getMaterial ();
                virtual void setMaterial (const Ogre::String& name);
                virtual void setMaterial (const Ogre::MaterialPtr& mat);

                virtual void getUV (Ogre::Real &u1, Ogre::Real &v1,
                                    Ogre::Real &u2, Ogre::Real &v2) const;
                virtual void setUV (Ogre::Real u1, Ogre::Real v1,
                                    Ogre::Real u2, Ogre::Real v2);

                virtual Ogre::PanelOverlayElement *getMe () const;

                protected:

                Pane ();

                Ogre::PanelOverlayElement *me;

                TreePtrs children;

        };


        typedef Ogre::SharedPtr<Pane> PanePtr;


        class Root : public Pane {
                public:

                Root(dim_t pw, dim_t ph);
                virtual ~Root();

                protected:

        };

        typedef Ogre::SharedPtr<Root> RootPtr;


        class Text : public Tree {
                public:

                Text (OverlayPtr, dim_t pw, dim_t ph);
                virtual ~Text();

                virtual void resetText();
                virtual void appendText(const DStr& str);
                virtual void commitText();
                virtual const DStr& getText() const;

                virtual void setFont(const Ogre::String& font);
                virtual const Ogre::String& getFont() const;

                virtual void setCharHeight(Ogre::Real h);
                virtual Ogre::Real getCharHeight() const;

                virtual dim_t getWidth() const;
                virtual dim_t getHeight() const;

                void setColourTop(Ogre::ColourValue v);
                const Ogre::ColourValue& getColourTop() const;

                void setColourBottom(Ogre::ColourValue v);
                const Ogre::ColourValue& getColourBottom() const;

                virtual TextListOverlayElement *getMe() const;

                protected:

                bool hasFont;

                TextListOverlayElement *me;

        };

        typedef Ogre::SharedPtr<Text> TextPtr;

        Ogre::Real text_width(const DStr& str, Ogre::FontPtr fp,
                              Ogre::Real height);

        DStr pixel_substr(const DStr& str, Ogre::FontPtr fp, Ogre::Real height,
                          Ogre::Real width, DStr *rest, bool wordwrap);

        void expand_tabs(const DStr& input, const unsigned int expand_tabs,
                         const bool ignore_codes, DStr& output);

        void wrap (const DStr& input, const Ogre::Real width, const dim_t lines,
                   const bool word_wrap, const bool chop_top,
                   const bool ignore_codes, const Ogre::FontPtr fp,
                   const Ogre::Real char_height, DStr& output, DStr *offcut);

}


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
