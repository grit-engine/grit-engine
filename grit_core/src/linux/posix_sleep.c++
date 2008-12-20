#define _POSIX_C_SOURCE 199309
#include <time.h>
#include <assert.h>
#include <stdio.h>

void mysleep(long nanos)
{
        if (nanos<=0) return;
        struct timespec t = {0,nanos};
        int r = nanosleep(&t, NULL);
        if (r) {
                perror("sleeping between frames");
        }
}

// vim: shiftwidth=8:tabstop=8:expandtab
