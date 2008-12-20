#include "math.h"

#include <OgreFontManager.h>

#include "HUD.h"
#include "app_error.h"

#define GET_CHAR(i) OGRE_DEREF_DISPLAYSTRING_ITERATOR((i))

namespace HUD {

        Overlay::Overlay()
        {
                me = Ogre::OverlayManager::getSingleton()
                        .create("HUD::Root");
                me->show();
        }
        Overlay::~Overlay()
        {
                me->clear();
                Ogre::OverlayManager::getSingleton().destroy(me);
        }
        Ogre::Overlay *Overlay::getOverlay()
        {
                return me;
        }


        Tree::Tree(OverlayPtr o, dim_t pw, dim_t ph)
        {
                this->overlay = o;
                this->pw = pw;
                this->ph = ph;
                this->w = DIM_T_MAX;
                this->h = DIM_T_MAX;
                this->x = DIM_T_MAX;
                this->y = DIM_T_MAX;
                this->orphaned = false;
        }

        void Tree::orphan()
        {
                this->orphaned = true;
        }

        // calls callback
        void Tree::_recalculate()
        {
                if (orphaned) return;
                dim_t oldw = getWidth(), oldh = getHeight();
                dim_t oldx = getPosX(), oldy = getPosY();
                calculator->f(this,pw,ph,x,y,w,h);
                if (oldx!=getPosX() || oldy!=getPosY())
                        getMe()->setPosition(getPosX(),getPosY());
                if (oldw!=getWidth() || oldh!=getHeight())
                        getMe()->setDimensions(getWidth(),getHeight());
        }

        void Tree::parentResized(dim_t pw, dim_t ph)
        {
                this->pw = pw;
                this->ph = ph;
                _recalculate();
        }

        dim_t Tree::getWidth() const
        { return w; }
        dim_t Tree::getHeight() const
        { return h; }

        dim_t Tree::getPosX() const
        { return x; }
        dim_t Tree::getPosY() const
        { return y; }

        void Tree::setCalculator (CalculatorPtr calculator)
        {
                this->calculator = calculator;
                _recalculate();
        }

        void Tree::setVisible (bool b)
        {
                if (b)
                        getMe()->show();
                else
                        getMe()->hide();
        }
        bool Tree::isVisible () const
        {
                return getMe()->isVisible();
        }

        Ogre::String Tree::nextName() {
                std::stringstream ss;
                ss << counter++;
                return ss.str();
        }

        int Tree::counter = 0;


        class DefaultCalculator : public Calculator {
                void f(Tree* t,
                       dim_t pw,dim_t ph,dim_t& x,dim_t& y,dim_t& w,dim_t& h)
                {
                        (void)t;
                        x = 0;
                        y = 0;
                        w = pw;
                        h = ph;
                }
        };


        Pane::Pane (OverlayPtr o, dim_t pw, dim_t ph) :
                Tree(o, pw, ph)
        {
                me = static_cast<Ogre::PanelOverlayElement*>(
                         Ogre::OverlayManager::getSingleton()
                                .createOverlayElement("Panel",nextName()));
                me->setMetricsMode(Ogre::GMM_PIXELS);
                me->setUV(0,0,1,1);
                me->show();
                setCalculator(CalculatorPtr(new DefaultCalculator()));
        }
        Pane::~Pane ()
        {
                removeAllChildren();
                Ogre::OverlayManager::getSingleton()
                        .destroyOverlayElement(me);
        }

        void Pane::removeAllChildren ()
        {
                for (TreePtrs::iterator i=children.begin() ;
                     i!=children.end() ; i++) {
                        TreePtr child = *i;
                        me->removeChild(child->getMe()->getName());
                        child->orphan();
                        child->setCalculator
                                (CalculatorPtr(new DefaultCalculator()));
                }
                children.clear();
        }

        void Pane::removeChild (TreePtr child)
        {
                me->removeChild(child->getMe()->getName());
                child->orphan();
                child->setCalculator(CalculatorPtr(new DefaultCalculator()));
                children.erase(std::find(children.begin(),
                                         children.end(),child));
        }

        TreePtrs Pane::getChildren ()
        {
                return children;
        }

        TreePtr Pane::createChild (Ogre::String type)
        {
                Tree *tree = NULL;
                if (type=="Pane") {
                        tree = new Pane(overlay,getWidth(),getHeight());
                } else if (type=="Text") {
                        tree = new Text(overlay,getWidth(),getHeight());
                } else {
                        APP_ERROR("","Unknown type \""+type+"\"");
                        app_fatal();
                }
                me->addChild(tree->getMe());
                TreePtr tree_ptr = TreePtr(tree);
                children.push_back(tree_ptr);
                return tree_ptr;
        }

        void Pane::_recalculate ()
        {
                if (orphaned) return;
                Tree::_recalculate();
                for (TreePtrs::iterator i=children.begin() ;
                     i!=children.end() ; i++) {
                        (*i)->parentResized(getWidth(),getHeight());
                }
        }

        // default is null (transparent)
        Ogre::MaterialPtr Pane::getMaterial ()
        {
                return me->getMaterial();
        }
        void Pane::setMaterial (const Ogre::String& name)
        {
                return me->setMaterialName(name);
        }
        void Pane::setMaterial (const Ogre::MaterialPtr& mat)
        {
                return me->setMaterialName(mat->getName());
        }
        void Pane::getUV (Ogre::Real &u1, Ogre::Real &v1,
                          Ogre::Real &u2, Ogre::Real &v2) const
        {
                me->getUV(u1,v1,u2,v2);
        }
        void Pane::setUV (Ogre::Real u1, Ogre::Real v1,
                          Ogre::Real u2, Ogre::Real v2)
        {
                me->setUV(u1,v1,u2,v2);
        }



        Ogre::PanelOverlayElement *Pane::getMe () const
        {
                return me;
        }



        Root::Root(dim_t pw, dim_t ph) :
                Pane(OverlayPtr(new Overlay()),pw,ph)
        {
                overlay->getOverlay()->add2D(getMe());
        }

        Root::~Root()
        {
        }



        Text::Text (OverlayPtr o, dim_t pw, dim_t ph) :
                Tree(o, pw, ph)
        {
                me = static_cast<TextListOverlayElement*>(
                         Ogre::OverlayManager::getSingleton()
                                .createOverlayElement("TextList",nextName()));
                me->setMetricsMode(Ogre::GMM_PIXELS);
                hasFont = false;
                setCalculator(CalculatorPtr(new DefaultCalculator()));
                appendText("No text entered.");
                commitText();
        }
        Text::~Text ()
        {
                Ogre::OverlayManager::getSingleton()
                        .destroyOverlayElement(me);
        }

        void Text::resetText()
        {
                me->resetCaption();
        }
        void Text::appendText(const DStr& str)
        {
                me->appendCaption(str);
        }
        void Text::commitText()
        {
                me->commitCaption();
                _recalculate();
        }
        const DStr& Text::getText() const
        {
                return me->getCaption();
        }

        void Text::setCharHeight(const Ogre::Real h)
        {
                me->setCharHeight(h);
                _recalculate();
        }
        Ogre::Real Text::getCharHeight() const
        {
                return me->getCharHeight();
        }

        void Text::setFont(const Ogre::String& font)
        {
                me->setFontName(font);
                hasFont = true;
                _recalculate();
        }
        const Ogre::String& Text::getFont() const
        {
                if (!hasFont) {
                        static Ogre::String nothing = "";
                        return nothing;
                }
                return me->getFontName();
        }

        dim_t Text::getWidth() const
        {
                return (dim_t)ceil(me->getWidth());
        }
        dim_t Text::getHeight() const
        {
                return (dim_t)ceil(me->getHeight());
        }

        void Text::setColourTop(Ogre::ColourValue v)
        {
                me->setColourTop(v);
        }
        const Ogre::ColourValue& Text::getColourTop() const
        {
                return me->getColourTop();
        }

        void Text::setColourBottom(Ogre::ColourValue v)
        {
                me->setColourBottom(v);
        }
        const Ogre::ColourValue& Text::getColourBottom() const
        {
                return me->getColourBottom();
        }


        TextListOverlayElement *Text::getMe() const
        {
                return me;
        }

        Ogre::Real text_width(const DStr& str, Ogre::FontPtr fp,
                              Ogre::Real height)
        {
                Ogre::Real counter = 0;
                for (DStr::const_iterator i=str.begin() ;
                     i!=str.end() ; i++) {
                        Ogre::Font::CodePoint character = GET_CHAR(i);
                        counter += fp->getGlyphAspectRatio(character) * height;
                }
                return counter;
        }

        DStr pixel_substr(const DStr& str, Ogre::FontPtr fp, Ogre::Real height,
                          Ogre::Real width, DStr *rest, bool wordwrap)
        {
                if (rest!=NULL) rest->clear();
                size_t break_point = 0;
                Ogre::Real counter = 0;
                for (DStr::const_iterator i=str.begin() ;
                     i!=str.end() ; i++) {
                        Ogre::Font::CodePoint character = GET_CHAR(i);
                        counter += fp->getGlyphAspectRatio(character) * height;
                        if (!wordwrap || character==' ')
                                break_point = i - str.begin();
                        if (counter > width) {
                                if (rest!=NULL) {
                                        if (character==' ') {
                                            *rest = str.substr(break_point+1);
                                        } else {
                                            *rest = str.substr(break_point);
                                        }

                                }
                                return str.substr(0,break_point);
                        }
                }
                return str;
        }

        bool end_word (DStr::const_iterator iter, DStr::const_iterator end)
        {
                if (iter==end) return true;
                if (GET_CHAR(iter+1)==' ') return true;
                return false;
        }

        DStr::const_iterator skip_esc_seq (const DStr::const_iterator& backup,
                                           const DStr::const_iterator& end)
        {
                if (backup==end) return backup;
                DStr::const_iterator iter = backup;
                if (GET_CHAR(iter++)!='\x1b' || iter==end) return backup;
                if (GET_CHAR(iter++)!='[' || iter==end) return backup;
                Ogre::Font::CodePoint c;
                do {
                        c = GET_CHAR(iter++);
                        if (!(c=='0'||c=='1'||c=='2'||c=='3'||c=='4'||c=='5'||
                              c=='6'||c=='7'||c=='8'||c=='9'||c==';'|| c=='m')
                            || (c!='m' && iter==end)) return backup;
                } while (c!='m');
                return iter;
        }

        void expand_tabs (const DStr& input, const unsigned int expand_tabs,
                          const bool ignore_codes, DStr& output)
        {
                output.reserve(input.size());
                int col = 0;
                for (DStr::const_iterator i=input.begin() ;
                     i<input.end() ; i++) {
                        if (ignore_codes) {
                                DStr::const_iterator before;
                                do {
                                        before = i;
                                        i = skip_esc_seq(i,input.end());
                                        size_t from = before-input.begin();
                                        size_t len = i - before;
                                        if (len==0) break;
                                        output.append(input,from,len);
                                } while (true);
                                if (i==input.end()) break;
                        }
                        Ogre::Font::CodePoint c = GET_CHAR(i);
                        switch (c) {
                                case '\t':
                                        do {
                                               output.push_back(' ');
                                               col++;
                                        } while (col%expand_tabs!=0);
                                        break;
                                case '\n': col = -1;
                                default:
                                        col++;
                                        output.push_back(c);
                        }
                }
        }

        void wrap (const DStr& input, const Ogre::Real width, dim_t lines,
                   const bool word_wrap, const bool chop_top,
                   const bool ignore_codes,
                   const Ogre::FontPtr fp, const Ogre::Real char_height,
                   DStr& output, DStr *offcut)
        {

                std::vector<DStr> vec;
                DStr::const_iterator break_from = input.begin();
                DStr::const_iterator break_to = break_from;
                Ogre::Real counter = 0;
                for (DStr::const_iterator i=break_from ;
                     i!=input.end() ; i++) {
                        if (ignore_codes) {
                                DStr::const_iterator before;
                                do {
                                        before = i;
                                        i = skip_esc_seq(i,input.end());
                                } while (i!=before);
                                if (i==input.end()) break;
                        }
                        Ogre::Font::CodePoint c = GET_CHAR(i);

                        counter += fp->getGlyphAspectRatio(c)*char_height;

                        if (!word_wrap || c==' ' || c=='\n')
                                break_to = i;

                        if (counter>width || c=='\n') {

                                // if wordwrap and we haven't found a gap yet
                                // then break here
                                if (break_from==break_to) break_to = i;

                                // this happens when width < one char
                                if (c!='\n' && break_from==break_to) return;

                                // we break line

                                size_t from_index= break_from-input.begin();
                                size_t len = break_to - break_from;

                                vec.push_back (input.substr(from_index,len));

                                Ogre::Font::CodePoint c2 = GET_CHAR(break_to);

                                // skip initial space on next line
                                if (c=='\n' || c2==' ') {
                                        break_to++;
                                }
                                i = break_to - 1;
                                break_from = break_to;

                                counter = 0;
                        }

                }

                size_t from_index = break_from - input.begin();
                size_t len = input.size() - from_index;
                vec.push_back(input.substr(from_index,len));


                output.reserve(16384);

                if (!lines) {
                        bool first = true;
                        for (std::vector<DStr>::const_iterator i=vec.begin();
                             i<vec.end() ; i++) {
                                if (!first) {
                                        output.append("\n");
                                }
                                output.append(*i);
                                first = false;
                        }
                        return;
                }

                if (chop_top) {
                        bool first = true;
                        unsigned int skip;
                        if (vec.size() < lines) {
                                skip = 0;
                        } else {
                                skip = vec.size() - lines;
                        }
                        for (std::vector<DStr>::const_iterator i=vec.begin();
                             i<vec.end() ; i++) {
                                if (skip>0) {
                                        if (offcut!=NULL) {
                                                offcut->append(*i);
                                                offcut->append("\n");
                                        }
                                        skip--;
                                } else {
                                        if (!first) {
                                                output.append("\n");
                                        }
                                        output.append(*i);
                                        first = false;
                                }
                        }
                } else {
                        bool first = true;
                        for (std::vector<DStr>::const_iterator i=vec.begin();
                             i<vec.end() ; i++) {
                                if (lines==0) {
                                        if (offcut!=NULL) {
                                                offcut->append("\n");
                                                offcut->append(*i);
                                        }
                                } else {
                                        if (!first) {
                                                output.append("\n");
                                        }
                                        output.append(*i);
                                        first = false;
                                        lines--;
                                }
                        }
                }

        }
}


// vim: shiftwidth=8:tabstop=8:expandtab
