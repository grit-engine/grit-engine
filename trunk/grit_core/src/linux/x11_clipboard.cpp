/* Copyright (c) David Cunningham and the Grit Game Engine project 2014
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdlib>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "../clipboard.h"
#include "../centralised_log.h"

static Display *display = NULL;
Window win = None;

static std::string get_xlib_error (Display *display, int code)
{
    char buf[1000];
    XGetErrorText(display, code, buf, sizeof(buf));
    return std::string(buf);
}

void clipboard_init (size_t the_win)
{
    win = the_win;
    display = XOpenDisplay(NULL);
    if (!display) {
        EXCEPT << "Could not open X display for getting clipboard." << ENDL;
    }
}

void clipboard_shutdown (void)
{
    if (display) XCloseDisplay(display);
}

void clipboard_set (const std::string &s)
{
    (void) s;
}

static std::string clipboard_get (Atom source)
{
    unsigned char *data = NULL;
    std::string s;
    try {

        Window owner = XGetSelectionOwner(display, source);

        if (owner != None) {

            // FIXME: property has to come from the right thing...

            Atom property = XInternAtom(display, "SelectionPropertyTemp", False);
            Atom utf8_string = XInternAtom(display, "UTF8_STRING", False);

            // Ask owner to convert the string into unicode.
            XConvertSelection(display, source, utf8_string, property, owner, CurrentTime);

            /*
            // Ensure that property has been populated.
            bool got_it = false;
            while (!got_it) {
                XEvent event;
                XNextEvent(display, &event);
                if (event.type == SelectionNotify) {
                    std::cout << "Got it." << std::endl;
                    got_it = true;
                } else {
                    std::cout << event.type << std::endl;
                }
            }
            */

            Atom type;  // Unused.
            int format=0;
            unsigned long len=0, bytes_left=0;

            // First call gets the length.
            int result = XGetWindowProperty(display, owner, property,
                0, 0,
                False,
                AnyPropertyType,
                &type,
                &format,
                &len, &bytes_left,
                &data);
            if (result != Success) {
                EXCEPT << "Could not XGetWindowProperty: " << get_xlib_error(display, result) << ENDL;
            }


            if (bytes_left != 0) {
                unsigned long dummy = 0;
                result = XGetWindowProperty(display, owner, property,
                    0, bytes_left,
                    False,
                    AnyPropertyType, &type, &format,
                    &len, &dummy, &data);

                if (result != Success) {
                    EXCEPT << "Could not XGetWindowProperty: " << get_xlib_error(display, result) << ENDL;
                }
                s = std::string(data, data+len);
            }

        }

    } catch (Exception &e) {
        if (data) XFree(data);
        throw e;
    }

    if (data) XFree(data);
    return s;
}


std::string clipboard_get (void)
{
    // First try CLIPBOARD since it behaves pretty much like Windows's one.
    Atom source = XInternAtom(display, "CLIPBOARD", False);
    return clipboard_get(source);
}

std::string clipboard_selection_get (void)
{
    // The X selection is where 
    Atom source = XInternAtom(display, "PRIMARY", False);
    return clipboard_get(source);
}

#ifdef GRIT_CLIPBOARD_TEST
#include <cstdlib>
#include <iostream>

int main(void)
{
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        EXCEPT << "Could not open X display for getting clipboard." << ENDL;
    }
    Window root = XDefaultRootWindow (display);
    int black = BlackPixel (display, DefaultScreen (display));
    Window window = XCreateSimpleWindow (display, root, 0, 0, 1, 1, 0, black, black);

    clipboard_init(window);

    std::cout << "Clipboard: \n" << clipboard_get() << "\n" << std::endl;;
    std::cout << "Selection: \n" << clipboard_selection_get() << "\n" << std::endl;;

    clipboard_shutdown();
        
    return EXIT_SUCCESS;
}

#endif
