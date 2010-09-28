/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

#define _POSIX_C_SOURCE 199309
#include <time.h>
#include <assert.h>
#include <stdio.h>

void mysleep(long micros)
{
        if (micros<=0) return;
        struct timespec t = {0,micros*1000};
        int r = nanosleep(&t, NULL);
        if (r) {
                perror("sleep");
        }
}

unsigned long long micros (void) {
#ifdef USE_GETTIMEOFDAY
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((unsigned long long)t.tv_sec)*1000000ULL + ((unsigned long long)t.tv_usec);
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return ((unsigned long long)t.tv_sec)*1000000ULL + ((unsigned long long)t.tv_nsec/1000ULL);
#endif
}

// vim: shiftwidth=8:tabstop=8:expandtab
