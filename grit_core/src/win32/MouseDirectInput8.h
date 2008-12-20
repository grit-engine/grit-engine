#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


#include "../Mouse.h"


class MouseDirectInput8 : public Mouse {

public:

        MouseDirectInput8 (size_t);
        virtual ~MouseDirectInput8 ();

        virtual bool getEvents(std::vector<int> *clicks,
                               int *x, int *y, int *rel_x, int *rel_y);

        virtual void setPos(int x, int y);

        virtual void setHide(bool toggle);
        virtual bool getHide();

        virtual void setGrab(bool toggle);
        virtual bool getGrab();

protected:

        HWND win;

        IDirectInput8 *directInput;

        IDirectInputDevice8 *dev;

        int current_x, current_y;

        int scroll;

        bool hidden;
        bool grabbed;
};

// vim: shiftwidth=8:tabstop=8:expandtab
