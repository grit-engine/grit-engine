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

#include "../centralised_log.h"

void clipboard_set (const std::string &s)
{
    (void) s;
}

static std::string get_xlib_error (Display *display, int code)
{
    char buf[1000];
    XGetErrorText(display, code, buf, sizeof(buf));
    return std::string(buf);
}

std::string clipboard_get (void)
{
    Display *display = NULL;
    unsigned char *data = NULL;
    std::string s;
    try {

        display = XOpenDisplay(NULL);

        if (!display) {
            EXCEPT << "Could not open X display for getting clipboard." << ENDL;
        }

        // First try CLIPBOARD since it behaves pretty much like Windows's one.
        Atom source = XInternAtom(display, "CLIPBOARD", False);
        Window owner = XGetSelectionOwner(display, source);

        if (owner != None) {

            Atom property = XInternAtom(display, "SelectionPropertyTemp", False);
            Atom utf8_string = XInternAtom(display, "UTF8_STRING", False);

            // Ask owner to convert the string into unicode.
            XConvertSelection(display, source, utf8_string, property, owner, CurrentTime);

            // Ensure that property has been populated.
            XSync(display, False);

            Atom type;  // Unused.
            int format=0;
            unsigned long len=0, bytes_left=0;

            // Returns all zeros the first call.  No idea why.
            int result = XGetWindowProperty(display, owner, property,
                0, 0,   //offset, len
                False,      //delete
                AnyPropertyType,
                &type,
                &format,
                &len, &bytes_left,
                &data);
            if (result != Success) {
                EXCEPT << "Could not XGetWindowProperty: " << get_xlib_error(display, result) << ENDL;
            }
            if (data) {
                XFree(data);
                data = NULL;
            }

            // Second call gets the length.
            result = XGetWindowProperty(display, owner, property,
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
        if (display) XCloseDisplay(display);
        throw e;
    }

    if (data) XFree(data);
    if (display) XCloseDisplay(display);

    return s;
}
