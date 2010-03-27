#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WINVER                 0x0500
#define _WIN32_WINNT           0x0500
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
#include <sstream>

#define LOG_LENGTH (20*1024*1024)
#define BUFFER_SIZE 16384

#define MENU_GL 151
#define MENU_FULLSCREEN 152
#define MENU_DINPUT 153

// Formats a message string using the specified message and variable
// list of arguments.

#define got_error(x) do { \
        LPVOID lpMsgBuf; \
        DWORD err = GetLastError(); \
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
                      NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), \
                      (LPTSTR) &lpMsgBuf, 0, NULL ); \
        std::stringstream ss; \
        ss << x << " (" << __FILE__ << ":" << __LINE__ << ")\n" << err << ": "<<((char*)lpMsgBuf); \
        MessageBox(NULL, ss.str().c_str(), "Error", MB_ICONEXCLAMATION | MB_OK); \
        ExitProcess(1); \
} while (0)

char *cmdline;

HWND win_main;
HWND win_log;
//HWND win_prompt;
//HWND win_send;
HWND win_launch;

std::string now (void)
{
        SYSTEMTIME now;
        GetLocalTime(&now);
        char now_str[1024];
        ::sprintf(now_str, "%04d/%02d/%02d %02d:%02d:%02d.%03d", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
        return now_str;
}

bool scrolled_to_bottom = true;

void test_scrolled_to_bottom (void)
{
        SCROLLINFO scrollinfo;
        scrollinfo.cbSize = sizeof(SCROLLINFO);
        scrollinfo.fMask = SIF_ALL;
        GetScrollInfo(win_log, SB_VERT, &scrollinfo);
        scrolled_to_bottom = scrollinfo.nPos >= scrollinfo.nMax - ((int)scrollinfo.nPage) - 32;
}

void scroll_bottom (void)
{
        SendMessage(win_log, WM_VSCROLL, SB_BOTTOM, 0);
        SendMessage(win_log, WM_VSCROLL, SB_LINEUP, 0);
}

void edit_append (const char *text)
{
        CHARRANGE range = {-1, -1};
        SendMessage(win_log, EM_EXSETSEL, 0, (LPARAM)&range);
        SendMessage(win_log, EM_REPLACESEL, FALSE, (LPARAM)text);
}

void edit_clear (void)
{
        CHARRANGE range = {0, -1};
        SendMessage(win_log, EM_EXSETSEL, 0, (LPARAM)&range);
        SendMessage(win_log, EM_REPLACESEL, FALSE, (LPARAM)"");
}

int current_colour = 7;
bool current_bold = false;
void set_colour (HWND edit, int c, bool b, bool everything=false)
{
        CHARFORMAT fmt;
        fmt.cbSize = sizeof(fmt);
        fmt.dwMask = CFM_COLOR;
        fmt.dwEffects = 0;
        switch (c) {
                case 0: fmt.crTextColor  =  b ? RGB(127,127,127) : RGB(  0,  0,  0); break;
                case 1: fmt.crTextColor  =  b ? RGB(255,  0,  0) : RGB(204,  0,  0); break;
                case 2: fmt.crTextColor  =  b ? RGB(  0,255,  0) : RGB(  0,204,  0); break;
                case 3: fmt.crTextColor  =  b ? RGB(255,255,  0) : RGB(204,204,  0); break;
                case 4: fmt.crTextColor  =  b ? RGB( 92, 92,255) : RGB(  0,  0,237); break;
                case 5: fmt.crTextColor  =  b ? RGB(255,  0,255) : RGB(204,  0,204); break;
                case 6: fmt.crTextColor  =  b ? RGB(  0,255,255) : RGB(  0,204,204); break;
                case 7: fmt.crTextColor  =  b ? RGB(255,255,255) : RGB(191,191,191); break;
        }
        SendMessage(edit, EM_SETCHARFORMAT, everything?SCF_ALL:SCF_SELECTION, (LPARAM)&fmt);
        current_colour = c;
        current_bold = b;
}

void append_time_stamp (void)
{
        int c = current_colour;
        bool b = current_bold;
        set_colour(win_log, 7, false, false);
        edit_append(("["+now()+"] ").c_str());
        set_colour(win_log, c, b, false);
}
char *get_text (HWND rich_edit, size_t &sz)
{
        GETTEXTEX text;
        char *buf = (char*)malloc(sz);
        text.cb = sz;
        text.flags = GT_USECRLF;
        text.codepage = CP_ACP;
        text.lpDefaultChar = NULL;
        text.lpUsedDefChar = NULL;
        LRESULT bytes = SendMessage(rich_edit, EM_GETTEXTEX, (WPARAM)&text, (LPARAM)buf);
        if ((size_t)bytes>=sz-1) {
                free(buf);
                if (sz==0) sz=1;
                sz *= 2;
                return get_text(rich_edit, sz);
        }
        sz = bytes;
        return buf;
}
bool menu_checked (HWND win, UINT id)
{
        return (GetMenuState(GetMenu(win),id,MF_BYCOMMAND) & MF_CHECKED)!=0;
}

bool toggle_menu_check (HWND win, UINT id)
{
        if (menu_checked(win,id)) {
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

bool subproc_running = false;
HANDLE subproc_pipe, inherited_pipe;
OVERLAPPED subproc_read, subproc_write;

void subproc_has_quit (void)
{
        CloseHandle(subproc_pipe);
        subproc_running = false;
        CloseHandle(subproc_read.hEvent);
        CloseHandle(subproc_write.hEvent);

        Button_Enable(win_launch,TRUE);
        //Button_Enable(win_send,FALSE);
        //Edit_Enable(win_prompt,FALSE);
        EnableMenuItem(GetMenu(win_main), MENU_GL, MF_ENABLED);
        EnableMenuItem(GetMenu(win_main), MENU_FULLSCREEN, MF_ENABLED);
        EnableMenuItem(GetMenu(win_main), MENU_DINPUT, MF_ENABLED);
}

std::string buffer;
#define RAW_BUFFER_SIZE 1024
char raw_buffer[RAW_BUFFER_SIZE+1] = "";
bool this_line_started = false;
int in_colour = 0;
void subproc_handle_output (DWORD bytes_read)
{
        test_scrolled_to_bottom();
        SendMessage(win_log, WM_SETREDRAW, false, 0);
        for (DWORD i=0 ; i<bytes_read ; ++i) {
                char c = raw_buffer[i];
                if (in_colour) {
                        if (in_colour==1) {
                                if (c!='[') {
                                        got_error("no [ here: " << c);
                                } else {
                                        in_colour = 2;
                                }
                        } else {
                                switch (c) {
                                        case 'm':
                                        in_colour = 0;
                                        case ';': {
                                                // process buffer
                                                long num = strtol(buffer.c_str(),NULL,10);
                                                buffer.clear();
                                                switch (num) {
                                                        case 0:
                                                        // reset
                                                        set_colour(win_log, 7, false);
                                                        break;
                                                        case 1:
                                                        // bold
                                                        set_colour(win_log, current_colour, true);
                                                        break;
                                                        case 22:
                                                        // nobold
                                                        set_colour(win_log, current_colour, false);
                                                        break;
                                                        case 30: case 31: case 32: case 33:
                                                        case 34: case 35: case 36: case 37:
                                                        // colour
                                                        set_colour(win_log, num-30, current_bold);
                                                        break;
                                                }
                                        } break;

                                        case '0': case '1': case '2': case '3': case '4':
                                        case '5': case '6': case '7': case '8': case '9':
                                        buffer.append(raw_buffer+i,1);
                                        break;
                                        
                                        default: got_error("unrecognised colour code: "<<c);

                                }
                        }
                } else {
                        if (c=='\n') {
                                buffer.append(raw_buffer+i,1);
                                if (!this_line_started) append_time_stamp();
                                edit_append(buffer.c_str());
                                this_line_started = false;
                                buffer.clear();
                        } else if (c=='\033') { // enter colour code
                                in_colour = true;
                                if (!this_line_started) append_time_stamp();
                                edit_append(buffer.c_str());
                                this_line_started = true;
                                buffer.clear();                        
                        } else {
                                buffer.append(raw_buffer+i,1);
                        }
                }
        }
        SendMessage(win_log, WM_SETREDRAW, true, 0);
        InvalidateRect(win_log, 0, true);
        if (scrolled_to_bottom)
                scroll_bottom();
}

void subproc_initiate_read (void)
{
        ZeroMemory(raw_buffer, RAW_BUFFER_SIZE+1);
        if (!ReadFile(subproc_pipe, raw_buffer, RAW_BUFFER_SIZE, NULL, &subproc_read)) {
                if (GetLastError()==ERROR_NO_DATA) {
                } else if (GetLastError()==ERROR_BROKEN_PIPE) {
                        subproc_has_quit();
                } else if (GetLastError()!=ERROR_IO_PENDING) {
                        got_error("ReadFile of pipe");
                }
        }
}

void set_env_var_to_menu_checked (const char *var, UINT menu_id)
{
        if (!SetEnvironmentVariable(var, "temporary"))
                got_error("SetEnvironmentVariable(\""<<var<<"\", ...)");
        if (!SetEnvironmentVariable(var, menu_checked(win_main, menu_id)?"1":NULL))
                got_error("SetEnvironmentVariable(\""<<var<<"\", ...)");
}
void subproc_spawn (void)
{
        PROCESS_INFORMATION piProcInfo;
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        subproc_pipe = CreateNamedPipe(
                "\\\\.\\pipe\\name",
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                1,
                BUFFER_SIZE,
                BUFFER_SIZE,
                0,
                NULL
        );
        if (subproc_pipe==INVALID_HANDLE_VALUE) got_error("Stdout pipe creation failed\n");

        // Set the bInheritHandle flag
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        inherited_pipe = CreateFile("\\\\.\\pipe\\name", GENERIC_READ | GENERIC_WRITE, 0, &saAttr, OPEN_EXISTING, 0, NULL);
        if (inherited_pipe == INVALID_HANDLE_VALUE) got_error("Could not open sub_proc side of pipe");

        STARTUPINFO siStartInfo;
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = inherited_pipe;
        siStartInfo.hStdOutput = inherited_pipe;
        siStartInfo.hStdInput = inherited_pipe;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
               
        if (!ConnectNamedPipe(subproc_pipe,NULL) && GetLastError()!=ERROR_PIPE_CONNECTED)
                got_error("Could not open launcher side of pipe");
        
        subproc_read.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (subproc_read.hEvent == NULL) got_error("Could not create event");

        subproc_write.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (subproc_write.hEvent == NULL) got_error("Could not create event");

        // sub_proc need not inherit our end of the pipe
        //SetHandleInformation(subproc_pipe, HANDLE_FLAG_INHERIT, 0);

        set_env_var_to_menu_checked("GRIT_GL", MENU_GL);
        set_env_var_to_menu_checked("GRIT_FULLSCREEN", MENU_FULLSCREEN);
        set_env_var_to_menu_checked("GRIT_DINPUT", MENU_DINPUT);
        //set_env_var_to_menu_checked("GRIT_NOVSYNC", MENU_SOMETHING);

        BOOL bFuncRetn = CreateProcess(NULL, cmdline,
                                  NULL,          // process security attributes
                                  NULL,          // primary thread security attributes
                                  TRUE,          // handles are inherited
                                  CREATE_NO_WINDOW,             // creation flags
                                  NULL,          // use parent's environment
                                  NULL,          // use parent's current directory
                                  &siStartInfo,  // STARTUPINFO pointer
                                  &piProcInfo);  // receives PROCESS_INFORMATION
        if (bFuncRetn == 0) got_error("CreateProcess failed.");
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);

        // Close down on our side (sub_proc still has handles open)
        if (!CloseHandle(inherited_pipe)) got_error("Closing sub_proc_pipe failed");

        subproc_running = true;
        subproc_initiate_read();

        Button_Enable(win_launch,FALSE);
        //Button_Enable(win_send,TRUE);
        //Edit_Enable(win_prompt,TRUE);
        EnableMenuItem(GetMenu(win_main), MENU_GL, MF_GRAYED);
        EnableMenuItem(GetMenu(win_main), MENU_FULLSCREEN, MF_GRAYED);
        EnableMenuItem(GetMenu(win_main), MENU_DINPUT, MF_GRAYED);
}

/*
void subproc_write_line(const char *line, size_t sz)
{
        WriteFile(subproc_pipe, line, sz, NULL, &subproc_write);
        DWORD bytes;
        BOOL r = GetOverlappedResult(subproc_pipe, &subproc_write, &bytes, TRUE);
        if (!r) got_error("GetOverlappedResult");
}
*/

void subproc_process_output (void)
{
        DWORD bytes_read;
        if (GetOverlappedResult(subproc_pipe, &subproc_read, &bytes_read, TRUE)) {
                subproc_handle_output(bytes_read);
                subproc_initiate_read();
        } else {
                if (GetLastError()==ERROR_BROKEN_PIPE) {
                        subproc_has_quit();
                } else {
                        got_error("GetOverlappedResult");
                }
        }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        int wmId = LOWORD(wParam), wmEvent = HIWORD(wParam);

        switch (message) {
/*
                case WM_NOTIFY: {
                        // intercept keypresses from win_prompt and use this to process tab and enter key
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
*/

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
                                                        got_error("Could not write to file.");
                                                }
                                                fclose(f);
                                                free(buf);
                                        }
                                } return 0;

                                case 102: // clear log
                                edit_clear();
                                return 0;

                                case 103: // exit
                                DestroyWindow(hWnd);
                                return 0;

                                case MENU_GL: // GL
                                toggle_menu_check(hWnd, 151);
                                return 0;

                                case MENU_FULLSCREEN: // fullscreen
                                toggle_menu_check(hWnd, 152);
                                return 0;

                                case MENU_DINPUT: // dinput
                                toggle_menu_check(hWnd, 153);
                                return 0;

                                case 201: // about
                                DialogBox((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(103), hWnd, AboutProc);
                                return 0;
                        }
                } else if ((HWND)lParam==win_launch) { // launch button
                         switch (wmEvent) {
                                case BN_CLICKED:
                                subproc_spawn();
                                return 0;
                        }
                } else if ((HWND)lParam==win_log) { // log
                         switch (wmEvent) {
                                case EN_VSCROLL:
                                test_scrolled_to_bottom();
                                break;
                        }
                } /*else if ((HWND)lParam==win_send) { // send button
                        switch (wmEvent) {
                                case BN_CLICKED: {
                                        size_t sz = 1024;
                                        char *buf = get_text(win_prompt, sz);
                                        subproc_write_line(buf, sz);
                                        subproc_write_line("\n", 1);
                                        free(buf);
                                        Edit_SetText(win_prompt, "");
                                } return 0;
                        }
                } */
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
                        MoveWindow(win_launch, p,           h-p-bh, w-p-p,           bh,         TRUE);
/*
                        MoveWindow(win_prompt, p,           h-p-bh, w-p-p-bw-p-bw-p, bh,         TRUE);
                        MoveWindow(win_send,   w-p-bw-p-bw, h-p-bh, bw,              bh,         TRUE);
                        MoveWindow(win_launch, w-p-bw,      h-p-bh, bw,              bh,         TRUE);
*/
                        if (scrolled_to_bottom)
                                scroll_bottom();
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

        cmdline = getenv("GRIT_PROCESS");
        if (lpCmdLine && ::strcmp(lpCmdLine,"")) {
                cmdline = lpCmdLine;
        }
        if (cmdline == NULL) cmdline = "Grit.dat";
        
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


        win_log = CreateWindow(RICHEDIT_CLASS, NULL,
                               WS_GROUP | ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_DISABLENOSCROLL | ES_AUTOVSCROLL | ES_SUNKEN, 
                               0, 0, 0, 0, win_main, NULL, hInstance, NULL);
        SendMessage(win_log, EM_LIMITTEXT, 0, LOG_LENGTH); // set char limit
        SendMessage(win_log, EM_SETEVENTMASK, 0, ENM_SCROLL);
        CHARFORMAT fmt;
        fmt.cbSize = sizeof(fmt);
        fmt.dwMask = CFM_FACE | CFM_SIZE | CFM_BOLD;
        fmt.yHeight = 20*8;
        strcpy(fmt.szFaceName,"Lucida Console");
        SendMessage(win_log, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&fmt);
        SendMessage(win_log, EM_SETBKGNDCOLOR, 0, RGB(20,20,20));
        set_colour(win_log, 7, false, true);
        Edit_SetReadOnly(win_log,  TRUE);
 
/*
        win_prompt = CreateWindow(RICHEDIT_CLASS, NULL,
                                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_SUNKEN | ES_LEFT, 
                                  0, 0, 0, 0, win_main, NULL, hInstance, NULL);
        SendMessage(win_prompt, EM_EXLIMITTEXT, 0, 1024*1024); // set char limit
        fmt.yHeight = 20*12;
        SendMessage(win_prompt, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&fmt);
        SendMessage(win_prompt, EM_SETEVENTMASK, 0, ENM_KEYEVENTS);
        SendMessage(win_prompt, EM_SETBKGNDCOLOR, 0, RGB(20,20,20));
        set_colour(win_prompt, 7, false, true);
        Edit_Enable(win_prompt,FALSE);

        win_send = CreateWindow("BUTTON", "&Send Lua",
                                 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                 0, 0, 0, 0, win_main, NULL, hInstance, NULL);
        Button_Enable(win_send,FALSE);
*/

        win_launch = CreateWindow("BUTTON", "&Launch",
                                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                  0, 0, 0, 0, win_main, NULL, hInstance, NULL);


        ShowWindow(win_main, nCmdShow);

        // Main message loop:
        bool alive = true;
        MSG msg;
        while (alive) {
                DWORD nCount = subproc_running ? 1 : 0;
                DWORD r = MsgWaitForMultipleObjectsEx(nCount, &subproc_read.hEvent, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
                if (r>=WAIT_OBJECT_0 && r<WAIT_OBJECT_0+nCount) {
                        subproc_process_output();
                } else if (r==WAIT_OBJECT_0+nCount) {
                        while (PeekMessage(&msg, NULL,  0, 0, PM_REMOVE)) {
                                if (msg.message == WM_QUIT) {
                                        alive = false;
                                        break;
                                }
                                if (!IsDialogMessage(win_main, &msg)) {
                                        TranslateMessage(&msg);
                                        DispatchMessage(&msg);
                                }
                        }
                } else if (r>=WAIT_ABANDONED_0 && r<WAIT_ABANDONED_0+nCount) {
                        got_error("wait abandoned");
                } else if (r==WAIT_TIMEOUT) {
                        got_error("wait timeout");
                } else if (r==WAIT_FAILED) {
                        got_error("wait failed");
                } else {
                        got_error("Unrecognised return code from MsgWaitForMultipleObjects");
                }
        }




        return (int) msg.wParam;
}
