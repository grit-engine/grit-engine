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

#ifndef portable_io_h
#define portable_io_h

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>

#include <string>
#include <iostream>
#include <fstream>

#ifdef _MSC_VER
        typedef unsigned char uint8_t;
        typedef unsigned short uint16_t;
        typedef signed short int16_t;
        typedef unsigned long uint32_t;
#else
        #include <stdint.h>
#endif

#include "CentralisedLog.h"


#define APP_ASSERT_IO_SUCCESSFUL(in,doing) if (!(in).good()) { \
        if ((in).eof()) { \
                GRIT_EXCEPT(std::string("EOF while ")+doing); \
        } else { \
                GRIT_EXCEPT(std::string(strerror(errno))+std::string(" while ")+doing); \
        } \
}

/* the following routines do IO on little endian files.  The host system can be
 * any endian. */

static inline unsigned long ios_read_u32(std::istream &in)
{{{
        unsigned char raw[4];
        in.read((char*)raw,sizeof raw);
        APP_ASSERT_IO_SUCCESSFUL(in,"reading uint32");
        return raw[3]<<24ul | raw[2]<<16ul | raw[1]<<8ul | raw[0];
}}}

static inline unsigned long ios_read_s32(std::istream &in)
{{{
        unsigned char raw[4];
        in.read((char*)raw,sizeof raw);
        APP_ASSERT_IO_SUCCESSFUL(in,"reading int32");
        long r = (raw[3]&0x7f)<<24ul | raw[2]<<16ul | raw[1]<<8ul | raw[0];
        if (raw[3]&0x80) r -= 0x80000000;
        return r;
}}}

static inline unsigned long ios_peek_u32(std::istream &in)
{{{
        char raw[4];
        in.read(raw,sizeof raw);
        in.putback(raw[3]);
        in.putback(raw[2]);
        in.putback(raw[1]);
        in.putback(raw[0]);
        return raw[3]<<24ul | raw[2]<<16ul | raw[1]<<8ul | raw[0];
}}}

static inline unsigned short ios_read_u16(std::istream &in)
{{{
        unsigned char raw[2];
        in.read((char*)raw,sizeof raw);
        APP_ASSERT_IO_SUCCESSFUL(in,"reading uint16");
        return raw[1]<<8ul | raw[0];
}}}

static inline signed short ios_read_s16(std::istream &in)
{{{
        unsigned char raw[2];
        in.read((char*)raw,sizeof raw);
        APP_ASSERT_IO_SUCCESSFUL(in,"reading int16");
        short r = raw[0];
        r += (((int)raw[1]&0x7F)<<8);
        if (raw[1]&0x80) r -= 0x8000;
        return r;
}}}

static inline unsigned char ios_read_u8(std::istream &in)
{{{
        unsigned char raw[1];
        in.read((char*)raw,sizeof raw);
        APP_ASSERT_IO_SUCCESSFUL(in,"reading uint8");
        return raw[0];
}}}

static inline float ios_read_float(std::istream &in)
{{{
        union floatint {
                unsigned long i;
                float f;
        };
        floatint val;
        val.i = ios_read_u32(in);
        return val.f;
}}}

static inline void ios_read_byte_array(std::istream &in,
                                       unsigned char *raw,size_t sz)
{{{
        if (raw) {
                in.read((char*)raw,sz);
                APP_ASSERT_IO_SUCCESSFUL(in,"reading byte array");
        } else {
                in.seekg(sz,std::ios_base::cur);
        }
}}}

static inline std::string ios_read_fixedstr(std::istream &in, size_t sz)
{{{
        //amount_read+=n;
        char *raw = new char[sz+1];
        memset(raw,0,sz+1);
        in.read(raw,sz);
        APP_ASSERT_IO_SUCCESSFUL(in,"reading string");
        std::string s(raw);
        delete [] raw;
        return s;
}}}

static inline void ios_write_u32(std::ostream &out, unsigned long v)
{{{
        out << (unsigned char)((v & 0x000000FF) >> 0);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing uint32");
        out << (unsigned char)((v & 0x0000FF00) >> 8);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing uint32");
        out << (unsigned char)((v & 0x00FF0000) >> 16);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing uint32");
        out << (unsigned char)((v & 0xFF000000) >> 24);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing uint32");
}}}

static inline void ios_write_u16(std::ostream &out, unsigned short v)
{{{
        out << (unsigned char)((v & 0x00FF) >> 0);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing uint16");
        out << (unsigned char)((v & 0xFF00) >> 8);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing uint16");
}}}

static inline void ios_write_u8(std::ostream &out, unsigned char v)
{{{
        out << (unsigned char)((v & 0x00FF) >> 0);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing uint8");
}}}

static inline void ios_write_float(std::ostream &out, float v)
{{{
        union floatint {
                unsigned long i;
                float f;
        };
        floatint val;
        val.f = v;
        ios_write_u32(out, val.i);
}}}

static inline void ios_write_str(std::ostream &out, const std::string &s)
{{{
        ios_write_u32(out,s.length());
        out << s;
        APP_ASSERT_IO_SUCCESSFUL(out,"writing string");
}}}

static inline void ios_write_byte_array(std::ostream &out, const unsigned char *raw,size_t sz)
{{{
        out.write((char*)raw,sz);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing byte array");
}}}

static inline void ios_write_byte_array(std::ostream &out, const char *raw,size_t sz)
{{{
        out.write(raw,sz);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing byte array");
}}}


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
