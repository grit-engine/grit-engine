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

#include <windows.h>

#include <unicode_util.h>

#include "../clipboard.h"

void clipboard_init (void)
{
    (void) win;
}

void clipboard_shutdown (void)
{
}

void clipboard_pump (void)
{
}

void clipboard_set (const std::string &s_)
{
    std::wstring s = utf8_to_utf16(s_);;
    const size_t len = s.size() + 1;
    HGLOBAL gmem = GlobalAlloc(GMEM_MOVEABLE, len);
    ::memcpy(GlobalLock(gmem), s.c_str(), len);
    GlobalUnlock(gmem);
    if (!OpenClipboard(0)) return;
    EmptyClipboard();
    SetClipboardData(CF_TEXT, gmem);
    CloseClipboard();
}

void clipboard_selection_set (const std::string &s)
{
    clipboard_set(s);
}

std::string clipboard_get (void)
{
    if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) return ""; 
    if (!OpenClipboard(0)) return "";
    HGLOBAL gmem = GetClipboardData(CF_UNICODETEXT);
    std::wstring r;
    LPWSTR gmem_ = (LPWSTR)GlobalLock(gmem);
    if (gmem_) r = gmem_;
    GlobalUnlock(gmem);
    CloseClipboard();
    return utf16_to_utf8(r);;
}

std::string clipboard_selection_get (void)
{
    return clipboard_get();
}

// vim: shiftwidth=8:tabstop=8:expandtab
