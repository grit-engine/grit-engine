#include <X11/Xlib.h>


#include "../Mouse.h"


class MouseX11 : public Mouse {

public:

        MouseX11 (size_t window);
        virtual ~MouseX11 ();

        virtual bool getEvents(std::vector<int> *clicks,
                               int *x, int *y, int *rel_x, int *rel_y);

        virtual void setPos(int x, int y);

        virtual void setHide(bool toggle);
        virtual bool getHide();

        virtual void setGrab(bool toggle);
        virtual bool getGrab();

protected:

        Display *display;
        Window win;
        Cursor blankCursor;


        // this is where the game or whatever thinks the mouse is
        int current_x;
        int current_y;

        // record whether the user asked for mouse grab / pointer hide.
        // when we regain focus we need to re-establish the grab
        bool requested_grab;
        bool requested_hide;

        // this is where X11 thinks the mouse is
        int real_last_x;
        int real_last_y;

        // used to signal the event listener that there will be a motion notify
        // event that needs to be ignored
        int warped_x;
        int warped_y;
        bool just_warped;
};

// vim: shiftwidth=8:tabstop=8:expandtab
