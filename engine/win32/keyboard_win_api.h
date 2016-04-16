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

#include <map>
#include <set>

#include <windows.h>

#include "../keyboard.h"

class KeyboardWinAPI : public Keyboard {

    public:

        KeyboardWinAPI (size_t);

        virtual ~KeyboardWinAPI (void);

        virtual Presses getPresses (void);

        virtual bool hasFocus (void);

        virtual LRESULT wndproc (HWND msgwin, UINT msg, WPARAM wParam, LPARAM lParam);

    protected:

        virtual Press getPress (WPARAM wParam, LPARAM scan_code);

        HWND win;

        WNDPROC old_wndproc;

        LONG_PTR old_user_data;

        Keyboard::Presses presses;

};

// vim: shiftwidth=8:tabstop=8:expandtab
