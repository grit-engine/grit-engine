#include <windows.h>

void mysleep(long nanos)
{
        long millis = nanos/1000000;
        if (millis<=0) return;
        Sleep(millis);
}

// vim: shiftwidth=8:tabstop=8:expandtab
