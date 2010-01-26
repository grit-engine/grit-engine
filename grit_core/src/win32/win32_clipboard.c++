#include <cstdlib>
#include <string>

#include <windows.h>

#include "../unicode_util.h"

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

// vim: shiftwidth=8:tabstop=8:expandtab
