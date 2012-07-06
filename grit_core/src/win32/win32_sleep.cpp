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

#include <windows.h>

void mysleep (long micros)
{
        if (micros<=0) return;
        long millis = micros/1000;
        Sleep(millis);
}

unsigned long long micros (void)
{
        static bool initialised = false;
        static unsigned long long freq; // cpu cycles per microsecond
        if (!initialised) {
                LARGE_INTEGER f;
                QueryPerformanceFrequency(&f);
                freq = f.QuadPart / 1000000ULL;
        }
        static unsigned long long last_tick_count_us;
        static unsigned long long last_perf_us;
        static unsigned long long adjust_ticks = 0;

        unsigned long long perf_ticks;
        {
                LARGE_INTEGER perf_counter;
                HANDLE curr_thread = GetCurrentThread();
                DWORD_PTR old_mask = SetThreadAffinityMask(curr_thread, 0);
                QueryPerformanceCounter(&perf_counter);
                SetThreadAffinityMask(curr_thread, old_mask);
                perf_ticks = perf_counter.QuadPart;
                perf_ticks -= adjust_ticks;
        }

        unsigned long long this_perf_us = perf_ticks / freq;

        unsigned long long this_tick_count_us = GetTickCount() * 1000;

        if (initialised) {
                // http://support.microsoft.com/kb/274323
                unsigned long long elapsed_ticks_us = this_tick_count_us - last_tick_count_us;
                unsigned long long elapsed_perf_us = this_perf_us - last_perf_us;
                int error_us = ((long long)(elapsed_perf_us)) - ((long long)(elapsed_ticks_us));
                if (error_us > 500000) {
                        unsigned long long adj_ticks = error_us*freq;
                        adjust_ticks += adj_ticks;
                        perf_ticks -= adj_ticks;
                        this_perf_us = perf_ticks / freq; // needs recomputing
                }
                // make sure adjustment doesn't make time run backwards
                if ((long long)(this_perf_us - last_perf_us) < 0) this_perf_us = last_perf_us;
        }

        last_tick_count_us = this_tick_count_us;
        last_perf_us = this_perf_us;
        initialised = true;
        return this_perf_us;
}
// vim: shiftwidth=8:tabstop=8:expandtab
