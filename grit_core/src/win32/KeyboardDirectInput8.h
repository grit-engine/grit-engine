#include <map>
#include <set>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


#include "../Keyboard.h"


class KeyboardDirectInput8 : public Keyboard {

public:

        KeyboardDirectInput8 (size_t);
        virtual ~KeyboardDirectInput8 ();

        virtual Presses getPresses();

        virtual bool hasFocus (void);

        virtual Press getShifted (const Press&);
        virtual Press getAlted (const Press&);

protected:

        HWND win;

        IDirectInput8 *directInput;

        IDirectInputDevice8 *dev;

        std::map<DWORD,const char *> keysUp, keysDown, keysRep;
        std::map<DWORD, ULONGLONG> pressTime;

        ULONGLONG lastTime;
};

// vim: shiftwidth=8:tabstop=8:expandtab
