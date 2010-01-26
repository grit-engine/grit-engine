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

protected:

        HWND win;

        IDirectInput8 *directInput;

        IDirectInputDevice8 *dev;

        std::map<DWORD,const char *> keysUp, keysDown, keysRep, keysText, keysTextShifted;
        std::map<Press,DWORD> keyCode;
        std::map<DWORD, ULONGLONG> pressTime;

        ULONGLONG lastTime;
};

// vim: shiftwidth=8:tabstop=8:expandtab
