#include <set>
#include <map>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>


#include "../Keyboard.h"


class KeyboardX11 : public Keyboard {

public:

        KeyboardX11 (size_t window);
        virtual ~KeyboardX11 ();

        virtual Presses getPresses();

        virtual Press getShifted(const Press &);

        virtual bool hasFocus (void);

protected:

        virtual void add_key (Keyboard::Presses &keys, XEvent event, int kind);

        Display *display;
        Window win;

        std::map<KeySym,const char*> myKeyMap;

        std::set<Press> currentlyPressed;

        std::map<Press, Press> shiftMap;

        bool focussed;
};

// vim: shiftwidth=8:tabstop=8:expandtab
