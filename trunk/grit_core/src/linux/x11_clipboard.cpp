/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

void clipboard_set (const std::string &s)
{
        (void) s;
}


std::string clipboard_get (void)
{
        Display *display = XOpenDisplay(NULL);
        if(!display)
                return "";
        
        //first try CLIPBOARD since it behaves pretty much like windows's one
        Atom selectionSource = XInternAtom(display, "CLIPBOARD", False);
        Window selectionOwner = XGetSelectionOwner(display, selectionSource);
        
        if(selectionOwner == None)
        {
                //use XA_PRIMARY as a fallback clipboard
                selectionSource=XA_PRIMARY;
                selectionOwner = XGetSelectionOwner(display, selectionSource);
        
                if(selectionOwner == None)
                {
                        return "";
                }
        }
        
        Atom property = XInternAtom(display, "SelectionPropertyTemp", False);
       
        //get atom for unicode string property
        Atom XA_UTF8_STRING = XInternAtom(display, "UTF8_STRING", False);
        //ask owner to convert the string into unicode
        //see this for reference http://svn.gna.org/svn/warzone/trunk/lib/betawidget/src/platform/sdl/clipboardX11.c
        XConvertSelection(display, selectionSource, XA_UTF8_STRING, property, selectionOwner, CurrentTime);
        XFlush(display);
        
        Atom type;
        int format=0, result=0;
        unsigned long len=0, bytes_left=0, dummy=0;
        uint8_t *data=NULL;
        
        result = XGetWindowProperty(display, selectionOwner, property,
                0, 0,   //offset, len
                0,      //delete 0==FALSE
                AnyPropertyType, //flag
                &type,          //return type
                &format,        //return format
                &len, &bytes_left,
                &data);
         if(result != Success)
         {
                return "";
         }
         if(result == Success && bytes_left > 0)
         {
                result = XGetWindowProperty (display, selectionOwner,
                        property, 0, bytes_left, 0,
                        AnyPropertyType, &type, &format,
                        &len, &dummy, &data);
                 
                 if(result != Success)
                 {
                        return "";
                 }
                 
                 if(type == XA_UTF8_STRING)
                 {
                        return std::string(data, data+bytes_left);
                 }
         }
        
        return "";
}

// vim: shiftwidth=8:tabstop=8:expandtab
