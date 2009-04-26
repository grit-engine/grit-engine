#include <sstream>

#include "MouseX11.h"

#include "../CentralisedLog.h"

MouseX11::MouseX11(size_t window)
{

        win = window;

        display = XOpenDisplay(0);
        APP_ASSERT(display);

        long event_mask = ButtonPressMask|ButtonReleaseMask|PointerMotionMask;
        event_mask |= FocusChangeMask;
        if (XSelectInput(display, win, event_mask) == BadWindow) {
                CERR << "calling XSelectInput: BadWindow" << std::endl;
                app_fatal();
        }

        Colormap colormap = DefaultColormap( display, DefaultScreen(display) );
        XColor black, dummy;
        XAllocNamedColor( display, colormap, "black", &black, &dummy );

        static char blank_cursor_raw[] = { 0,0,0,0,0,0,0,0 };
        Pixmap pm =
                XCreateBitmapFromData(display, win, blank_cursor_raw, 8, 8);
        blankCursor =
                XCreatePixmapCursor(display, pm, pm, &black, &black, 0, 0);

        current_x = 0;
        current_y = 0;
        real_last_x = 0;
        real_last_y = 0;

        requested_grab = 0;
        requested_hide = 0;

        just_warped = false;
}

MouseX11::~MouseX11()
{
        if(display)
        {
                getEvents(NULL,NULL,NULL,NULL,NULL);
                XFreeCursor(display, blankCursor);
                XCloseDisplay(display);
        }
}

bool MouseX11::getEvents(std::vector<int> *clicks,
                           int *x, int *y, int *rel_x, int *rel_y)
{
        if (clicks) clicks->clear();
        // if there is no mouse grab in action, last_x = real_last_x
        // otherwise: last_x is where the app thinks the cursor is
        //            real_last_x
        int last_x = current_x;
        int last_y = current_y;
        bool moved = false;

        //Poll x11 for events
        XEvent event;
        while (XPending(display)>0) {

                XNextEvent(display, &event);

                int mod = 1;

                switch (event.type) {

                        case FocusIn:

                        // re-establish grab if it is supposed to be in action
                        if (requested_grab) setGrab(requested_grab);

                        break;

                        case MotionNotify:

                        if (just_warped && event.xmotion.x==warped_x
                                        && event.xmotion.y==warped_y) {
                                just_warped = false;
                                // adjust the "last" position to the new
                                // position so that it looks like the mouse
                                // hasn't moved during the warp
                                real_last_x = event.xmotion.x;
                                real_last_y = event.xmotion.y;
                                break;
                        }

                        if (requested_grab) {
                                current_x += event.xmotion.x - real_last_x;
                                current_y += event.xmotion.y - real_last_y;
                        } else {
                                current_x = event.xmotion.x;
                                current_y = event.xmotion.y;
                        }

                        real_last_x = event.xmotion.x;
                        real_last_y = event.xmotion.y;

                        moved = true;

                        if (requested_grab) {
                                XWindowAttributes attr;
                                XGetWindowAttributes(display,win,&attr);
                                setPos(attr.width/2,attr.height/2);
                        }

                        break;

                        case ButtonRelease:

                        mod = -1;

                        case ButtonPress:

                        if (event.xbutton.button >= NUM_BUTTONS) break;

                        enum Button from_x11[NUM_BUTTONS];
                        from_x11[Button1] = MOUSE_LEFT;
                        from_x11[Button2] = MOUSE_MIDDLE;
                        from_x11[Button3] = MOUSE_RIGHT;
                        from_x11[Button4] = MOUSE_SCROLL_UP;
                        from_x11[Button5] = MOUSE_SCROLL_DOWN;

                        if (clicks) {
                                int code = from_x11[event.xbutton.button];
                                clicks->push_back(mod*code);
                        }

                        break;

                }

        }

        if (x) *x = current_x;
        if (y) *y = current_y;
        if (rel_x) *rel_x = current_x - last_x;
        if (rel_y) *rel_y = current_y - last_y;

        return moved;
}

void MouseX11::setPos(int x, int y)
{
        XWarpPointer(display,None,win,0,0,0,0, x,y);
        warped_x = x;
        warped_y = y;
        just_warped = true;
}


void MouseX11::setHide(bool toggle)
{
        requested_hide = toggle;
        if (toggle)
                XDefineCursor(display, win, blankCursor);
        else
                XUndefineCursor(display, win);
}

bool MouseX11::getHide()
{
        return requested_hide;
}


void MouseX11::setGrab(bool toggle)
{
        requested_grab = toggle;

        if (toggle) {
                XGrabPointer (display, win,
                              True, 0,
                              GrabModeAsync, GrabModeAsync,
                              win, None, CurrentTime);
        } else {
                XUngrabPointer (display, CurrentTime);
        }
}

bool MouseX11::getGrab()
{
        return requested_grab;
}



// vim: shiftwidth=8:tabstop=8:expandtab
