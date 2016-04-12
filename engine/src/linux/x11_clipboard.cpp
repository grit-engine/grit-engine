/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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
Window window = None;
Atom scratch_property = None;  // Clipboard copied to here upon get
Atom utf8_string = None;  // Clipboard data type to ask other apps for.
Atom src_selection = None;
Atom src_clipboard = None;
std::string data_selection;
std::string data_clipboard;

static std::string get_xlib_error (Display *display, int code)
{
    char buf[1000];
    XGetErrorText(display, code, buf, sizeof(buf));
    return std::string(buf);
}

void clipboard_init (void)
{
    display = XOpenDisplay(NULL);
    if (!display) {
        EXCEPTEX << "Could not open X display for getting clipboard." << ENDL;
    }
    Window root = XDefaultRootWindow(display);
    int black = BlackPixel(display, DefaultScreen (display));
    window = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, black, black);
    scratch_property = XInternAtom(display, "ScratchProperty", False);
    utf8_string = XInternAtom(display, "UTF8_STRING", False);
    src_selection = XInternAtom(display, "PRIMARY", False);
    src_clipboard = XInternAtom(display, "CLIPBOARD", False);
}

static bool handle_event (const XEvent &event)
{
    if (event.type != SelectionRequest)
        return false;

    const XSelectionRequestEvent &req = event.xselectionrequest;
    std::string *to_serve;
    if (req.selection == src_selection) {
        to_serve = &data_selection;
    } else if (req.selection == src_clipboard) {
        to_serve = &data_clipboard;
    } else {
        return true;
    }
    XSelectionEvent res;
    res.type = SelectionNotify;
    res.display = req.display;
    res.requestor = req.requestor;
    res.selection = req.selection;
    res.time = req.time;
    res.target = req.target;
    res.property = req.property;
    XChangeProperty(display, req.requestor, req.property,
                    req.target, 8, PropModeReplace,
                    reinterpret_cast<const unsigned char*>(to_serve->c_str()), to_serve->length());
    XSendEvent(display, res.requestor, False, (unsigned long)NULL, (XEvent *)&res);
    return true;
}

void clipboard_pump (void)
{
    XEvent event;
    int r = XCheckTypedEvent(display, SelectionRequest, &event);
    if (!r) return;
    handle_event(event);
}

void clipboard_shutdown (void)
{
    if (display) XCloseDisplay(display);
}

void clipboard_set (const std::string &s)
{
    data_clipboard = s;
    XSetSelectionOwner(display, src_clipboard, window, CurrentTime);
}

void clipboard_selection_set (const std::string &s)
{
    data_selection = s;
    XSetSelectionOwner(display, src_selection, window, CurrentTime);
}

static std::string clipboard_get (Atom source)
{
    unsigned char *data = NULL;
    std::string s;
    try {

        XConvertSelection(display, source, utf8_string, scratch_property, window, CurrentTime);

        // Ensure that scratch_property has been populated.
        bool got_it = false;
        XEvent event;
        while (!got_it) {
            // Blocks if queue is empty.
            XNextEvent(display, &event);
            if (!handle_event(event)) {
                if (event.type == SelectionNotify) {
                    if (event.xselection.selection == source)
                        got_it = true;
                }
            }
        }

        if (event.xselection.property != None) {

            Atom type;  // Unused.
            int format = 0;
            unsigned long len = 0;
            unsigned long bytes_left = 0;

            // First call gets the length.
            int result = XGetWindowProperty(display, window, scratch_property,
                0, 0,
                False,
                AnyPropertyType,
                &type,
                &format,
                &len, &bytes_left,
                &data);
            if (result != Success) {
                EXCEPTEX << "Could not XGetWindowProperty: " << get_xlib_error(display, result) << ENDL;
            }

            if (bytes_left != 0) {
                unsigned long dummy = 0;
                result = XGetWindowProperty(display, window, scratch_property,
                    0, bytes_left,
                    False,
                    AnyPropertyType, &type, &format,
                    &len, &dummy, &data);
                if (result != Success) {
                    EXCEPTEX << "Could not XGetWindowProperty: " << get_xlib_error(display, result) << ENDL;
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
    return clipboard_get(src_clipboard);
}

std::string clipboard_selection_get (void)
{
    return clipboard_get(src_selection);
}

#ifdef GRIT_CLIPBOARD_TEST
#include <cstdlib>
#include <iostream>

int main(void)
{
    try {
        clipboard_init();

        std::cout << "Clipboard: \n" << clipboard_get() << "\n" << std::endl;;
        std::cout << "Selection: \n" << clipboard_selection_get() << "\n" << std::endl;;

        clipboard_selection_set("Selection data.");
        clipboard_set("Clipboard data.");
        for (long long i=0 ; i<10000000 ; ++i)
            clipboard_pump();

        clipboard_shutdown();
            
        return EXIT_SUCCESS;
    } catch (Exception &e) {
        std::cerr << "Exception: " << e << std::endl;
        return EXIT_FAILURE;
    }
}

#endif
