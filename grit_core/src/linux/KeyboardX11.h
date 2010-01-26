#include <set>
#include <map>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>


#include "../Keyboard.h"


class KeyboardX11 : public Keyboard {

public:

        KeyboardX11 (size_t window);
        ~KeyboardX11 ();

        Presses getPresses();

        bool hasFocus (void);

protected:

        void add_key (Keyboard::Presses &keys, XEvent event, int kind);

        Display *display;
        Window win;
/*
        XIM im;
        XIC ic;
*/

        std::map<KeySym,const char*> myKeyMap;

        std::set<Press> currentlyPressed;

        bool focussed;
};

// vim: shiftwidth=8:tabstop=8:expandtab
