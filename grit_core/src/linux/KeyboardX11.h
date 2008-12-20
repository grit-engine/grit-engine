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

        //virtual u32 get_unicode(unsigned int key);

protected:

        Display *display;
        Window win;

        std::map<KeySym,const char*> xKeysUp;
        std::map<KeySym,const char*> xKeysDown;

};

// vim: shiftwidth=8:tabstop=8:expandtab
