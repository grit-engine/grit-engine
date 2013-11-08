/* Copyright Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

#include <sstream>
#include <iostream>

#include <cstdlib>
#include <cstring>
#include <cerrno>

#include "portable_io.h"
#include <console.h>

#ifndef ios_util_h
#define ios_util_h

#define ios_read_header(f,type,size,ver,vercheck) \
    ios_read_header_(f,type,size,ver,vercheck,__FILE__,__LINE__)

static inline void ios_read_header_(std::istream &f,
                                    unsigned long *type, unsigned long *size,
                                    unsigned long *ver, unsigned long *vercheck,
                                    const char *file, int line)
{{{
    *type = ios_read_u32(f);
    unsigned long size2 = ios_read_u32(f);
    if (size) *size = size2;
    unsigned long ver2 = ios_read_u32(f);
    if (ver) *ver=ver2;
    //std::cout<<"version: "<<DECHEX(ver2)<<std::endl;
    if(vercheck && ver2!=*vercheck) {
        std::stringstream s;
        s<<"Expected version "<<*vercheck
            <<" (0x"<<std::hex<<*vercheck<<std::dec<<"), "
        <<"got version "<<ver2
            <<" (0x"<<std::hex<<ver2<<std::dec<<"), "
        <<"at " <<file<<":"<<line<<".";
        GRIT_EXCEPT(s.str());
    }
}}}


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
