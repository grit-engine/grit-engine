#include <cstdlib>
#include <string>

#include <windows.h>

void clipboard_set (const std::string &s)
{
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
        if (!IsClipboardFormatAvailable(CF_TEXT)) return ""; 
        if (!OpenClipboard(0)) return "";
        HGLOBAL gmem = GetClipboardData(CF_TEXT);
        std::string r;
        LPTSTR gmem_ = (LPTSTR)GlobalLock(gmem);
        if (gmem_) r = gmem_;
        GlobalUnlock(gmem);
        CloseClipboard();
        return r;
}

// vim: shiftwidth=8:tabstop=8:expandtab
