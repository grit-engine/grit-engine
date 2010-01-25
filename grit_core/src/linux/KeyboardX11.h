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

        Press getShifted(const Press &);
        Press getAlted(const Press &);

        bool hasFocus (void);

protected:

        std::string key_to_string (XEvent &ev);
        void add_key (Keyboard::Presses &keys, XEvent event, int kind);

        Display *display;
        Window win;
/*
        XIM im;
        XIC ic;
*/

        std::map<KeySym,const char*> myKeyMap;

        std::set<Press> currentlyPressed;

        std::map<Press, Press> shiftMap;
        std::map<Press, Press> altMap;

        bool focussed;
};

// vim: shiftwidth=8:tabstop=8:expandtab
