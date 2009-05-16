#include <map>
#include <set>

#include <windows.h>


#include "../Keyboard.h"


class KeyboardWinAPI : public Keyboard {

    public:

        KeyboardWinAPI (size_t);

        virtual ~KeyboardWinAPI (void);

        virtual Presses getPresses (void);

        virtual bool hasFocus (void);

        virtual Press getShifted (const Press &);

        virtual LRESULT wndproc (HWND msgwin, UINT msg, WPARAM wParam, LPARAM lParam);

    protected:

        virtual Press getPress (WPARAM wParam, LPARAM scan_code);

        HWND win;

        WNDPROC old_wndproc;

        LONG_PTR old_user_data;

        Keyboard::Presses presses;

        typedef std::set<Press> DownSet;
        DownSet down;

        typedef std::map<Press,Press> ShiftMap;

        ShiftMap shiftMap;
};

// vim: shiftwidth=8:tabstop=8:expandtab
