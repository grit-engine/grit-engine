#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WINVER                 0x0500
#define _WIN32_WINNT         0x0500
#define _WIN32_WINDOWS         0x0500

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Commdlg.h>
#include <windowsx.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <richedit.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <string>

#define LOG_LENGTH (20*1024*1024)

HWND win_main;
HWND win_log;
HWND win_prompt;
HWND win_send;
HWND win_launch;

std::string now (void)
{
        SYSTEMTIME now;
        GetLocalTime(&now);
        char now_str[1024];
        ::sprintf(now_str, "%04d/%02d/%02d %02d:%02d:%02d.%03d", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
        return now_str;
}

void edit_scroll (HWND edit)
{
        SendMessage(edit, WM_VSCROLL, SB_BOTTOM, 0);
        SendMessage(edit, WM_VSCROLL, SB_LINEUP, 0);
}

void edit_append (HWND edit, const char *text)
{
        CHARRANGE range = {-1, -1};
        SendMessage(edit, EM_EXSETSEL, 0, (LPARAM)&range);
        SendMessage(edit, EM_REPLACESEL, FALSE, (LPARAM)text);
        edit_scroll(edit);
}

void log_print (const char *text) {
        std::string time = now();
        time = "["+time+"] "+text+"\n";
        edit_append(win_log, time.c_str()); 
}

bool toggle_menu_check (HWND win, UINT id)
{
        if (GetMenuState(GetMenu(win),id,MF_BYCOMMAND) & MF_CHECKED) {
                CheckMenuItem(GetMenu(win),id,MF_UNCHECKED);
                return false;
        } else {
                CheckMenuItem(GetMenu(win),id,MF_CHECKED);
                return true;
        }
}

BOOL CALLBACK AboutProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
        if (message==WM_COMMAND && (LOWORD(wParam)==IDOK || LOWORD(wParam)==IDCANCEL)) {
                EndDialog(hwndDlg, wParam);
                return TRUE;
        }
        return FALSE;
} 

char *get_text (HWND rich_edit, size_t &sz)
{
        GETTEXTEX text;
        char *buf = (char*)malloc(LOG_LENGTH);
        text.cb = LOG_LENGTH;
        text.flags = GT_USECRLF;
        text.codepage = CP_ACP;
        text.lpDefaultChar = NULL;
        text.lpUsedDefChar = NULL;
        LRESULT bytes = SendMessage(rich_edit, EM_GETTEXTEX, (WPARAM)&text, (LPARAM)buf);
        if ((size_t)bytes<=sz) {
                free(buf);
                sz *= 2;
                return get_text(rich_edit, sz);
        }
        sz = bytes;
        return buf;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        int wmId = LOWORD(wParam), wmEvent = HIWORD(wParam);

        switch (message) {
                case WM_NOTIFY: {
                        if (((LPNMHDR)lParam)->code==EN_MSGFILTER && ((LPNMHDR)lParam)->hwndFrom==win_prompt) {
                                MSGFILTER *mf = (MSGFILTER*) lParam;
                                if (mf->msg==WM_KEYUP && mf->wParam==0xd) {
                                        SendMessage(win_send, BM_CLICK, 0, 0);
                                        SetFocus(win_prompt);
                                        return 1;
                                } else if (mf->msg==WM_KEYDOWN && mf->wParam==0x9){
                                        SetFocus(win_send);
                                        return 1;
                                }
                        }
                } break;

                case WM_COMMAND:
                if (lParam==0 && wmEvent==0) { // menu
                        switch (wmId) {
                                case 101: { // save as
                                        OPENFILENAME fn;
                                        ZeroMemory(&fn,sizeof(fn));
                                        fn.lStructSize = sizeof (OPENFILENAME);
                                        fn.hwndOwner = hWnd;
                                        char name[1024] = "output.txt";
                                        fn.lpstrFile = name;
                                        fn.nMaxFile = sizeof(name);
                                        fn.lpstrDefExt = "txt";
                                        if(GetSaveFileName(&fn)) {
                                                FILE *f = fopen(name, "w");
                                                size_t textsz = 1024;
                                                char *buf = get_text(win_log, textsz);
                                                size_t written = fwrite(buf, 1, textsz, f);
                                                if (written!=textsz) {
                                                        MessageBox(NULL, "Could not write to file.", "Error", MB_ICONEXCLAMATION | MB_OK);
                                                }
                                                fclose(f);
                                                free(buf);
                                        }
                                } return 0;

                                case 102: // exit
                                DestroyWindow(hWnd);
                                return 0;

                                case 151: // GL
                                toggle_menu_check(hWnd, 151);
                                return 0;

                                case 152: // fullscreen
                                toggle_menu_check(hWnd, 152);
                                return 0;

                                case 153: // dinput
                                toggle_menu_check(hWnd, 153);
                                return 0;

                                case 201: // about
                                DialogBox((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(103), hWnd, AboutProc);
                                return 0;
                        }
                } else if ((HWND)lParam==win_launch) { // launch button
                         switch (wmEvent) {
                                case BN_CLICKED:
                                log_print("I liek cows.");
                                return 0;
                        }
                } else if ((HWND)lParam==win_send) { // send button
                        switch (wmEvent) {
                                case BN_CLICKED: {
                                        size_t sz = 1024;
                                        char *buf = get_text(win_prompt, sz);
                                        log_print(buf);
                                        free(buf);
                                } return 0;
                        }
                }
                break;

                case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

                case WM_SIZE: {
                        int w = LOWORD(lParam), h = HIWORD(lParam);
                        int bh = 24; // button height
                        int bw = 128; // button width
                        int p = 4; // padding
                        MoveWindow(win_log,    p,           p,      w-p-p,           h-bh-p-p-p, TRUE);
                        MoveWindow(win_prompt, p,           h-p-bh, w-p-p-bw-p-bw-p, bh,       TRUE);
                        MoveWindow(win_send,   w-p-bw-p-bw, h-p-bh, bw,              bh,       TRUE);
                        MoveWindow(win_launch, w-p-bw,      h-p-bh, bw,              bh,       TRUE);
                        edit_scroll(win_log);
                }

        }
        return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
        UNREFERENCED_PARAMETER(hPrevInstance);
        UNREFERENCED_PARAMETER(lpCmdLine);

        InitCommonControls();

        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = 0;
        wcex.hInstance     = hInstance;
        wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(111));
        wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
        wcex.lpszMenuName  = MAKEINTRESOURCE(110);
        wcex.lpszClassName = "Grit Launcher";
        wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(111));
        RegisterClassEx(&wcex);


        LoadLibrary("riched20.dll");

        win_main = CreateWindow("Grit Launcher", "Grit Launcher", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
        if (!win_main) return FALSE;


        win_log = CreateWindow(RICHEDIT_CLASS, TEXT("Type here\n"),
                               WS_GROUP | ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_DISABLENOSCROLL | ES_AUTOVSCROLL | ES_SUNKEN, 
                               0, 0, 0, 0, win_main, NULL, hInstance, NULL);
        SendMessage(win_log, EM_LIMITTEXT, 0, LOG_LENGTH); // set char limit
        CHARFORMAT fmt;
        fmt.cbSize = sizeof(fmt);
        fmt.dwMask = CFM_FACE | CFM_COLOR | CFM_SIZE | CFM_BOLD;
        fmt.yHeight = 20*8;
/*
        if bold and colour=="black" then r,g,b = .5,.5,.5
        elseif colour=="black" then r,g,b = 0,0,0
        elseif bold and colour=="red" then r,g,b = 1,0,0
        elseif colour=="red" then r,g,b = .8,0,0
        elseif bold and colour=="green" then r,g,b = 0,1,0
        elseif colour=="green" then r,g,b = 0,.8,0
        elseif bold and colour=="yellow" then r,g,b = 1,1,0
        elseif colour=="yellow"then r,g,b = .8,.8,0
        elseif bold and colour=="blue" then r,g,b = .36,.36,1
        elseif colour=="blue" then r,g,b = 0,0,.93
        elseif bold and colour=="magenta" then r,g,b = 1,0,1
        elseif colour=="magenta"then r,g,b = .8,0,.8
        elseif bold and colour=="cyan" then r,g,b = 0,1,1
        elseif colour=="cyan" then r,g,b = 0,.8,.8
        elseif bold and colour=="white" then r,g,b = 1,1,1
        elseif colour=="white" then r,g,b = .75,.75,.75
*/
        fmt.crTextColor = RGB(191,191,191);
        fmt.dwEffects = 0;
        strcpy(fmt.szFaceName,"Lucida Console");
        SendMessage(win_log, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&fmt);
        SendMessage(win_log, EM_SETBKGNDCOLOR, 0, RGB(0,0,0));
        Edit_SetReadOnly(win_log,  TRUE);
 
        win_prompt = CreateWindow(RICHEDIT_CLASS, "I liek fish.",
                                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_SUNKEN | ES_LEFT, 
                                  0, 0, 0, 0, win_main, NULL, hInstance, NULL);
        SendMessage(win_prompt, EM_EXLIMITTEXT, 0, 1024*1024); // set char limit
        //Edit_Enable(win_prompt,FALSE);

        win_send = CreateWindow("BUTTON", "&Send Lua",
                                 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                 0, 0, 0, 0, win_main, NULL, hInstance, NULL);
        //Button_Enable(win_send,FALSE);

        win_launch = CreateWindow("BUTTON", "&Launch",
                                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                  0, 0, 0, 0, win_main, NULL, hInstance, NULL);

        SendMessage(win_prompt, EM_SETEVENTMASK, 0, ENM_KEYEVENTS);

        ShowWindow(win_main, nCmdShow);

        // Main message loop:
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
                if (!IsDialogMessage(win_main, &msg)) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                }
        }

        return (int) msg.wParam;
}


