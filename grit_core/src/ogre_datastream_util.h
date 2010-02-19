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

#include <string.h>

#include <string>

#include <OgreDataStream.h>
#include <OgreException.h>

#ifndef ogre_datastream_util_h
#define ogre_datastream_util_h

#define ASSERT_IO_SUCCESSFUL(x,doing)if (!(x)) { \
        if (f->eof()) { \
                OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR,\
                            "EOF",doing); \
        } else { \
                OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR,\
                            std::string(strerror(errno)),doing); \
        } \
}

/* the following routines do IO on little endian files.  The host system can be
 * any endian. */

static inline unsigned long ods_read_u32 (const Ogre::DataStreamPtr &f)
{{{
        unsigned char raw[4];
        size_t sz = f->read(raw,sizeof raw);
        ASSERT_IO_SUCCESSFUL(sz==sizeof raw,"Reading uint32");
        unsigned long r;
        r = raw[0];
        r += raw[1] << 8;
        r += raw[2] << 16;
        r += raw[3] << 24;
        return r;
}}}

static inline unsigned short ods_read_u16 (const Ogre::DataStreamPtr &f)
{{{
        unsigned char raw[2];
        size_t sz = f->read(raw,sizeof raw);
        ASSERT_IO_SUCCESSFUL(sz==sizeof raw,"Reading uint16");
        unsigned short r;
        r = raw[0];
        r += raw[1] << 8;
        return r;
}}}

static inline short ods_read_s16 (const Ogre::DataStreamPtr &f)
{{{
        unsigned char raw[2];
        size_t sz = f->read(raw,sizeof raw);
        ASSERT_IO_SUCCESSFUL(sz==sizeof raw,"Reading int16");
        short r = raw[0];
        r += (((int)raw[1]&0x7F)<<8);
        if (raw[1]&0x80) r -= 0x8000;
        return r;
}}}

static inline unsigned char ods_read_u8 (const Ogre::DataStreamPtr &f)
{{{
        unsigned char r;
        size_t sz = f->read(&r,sizeof r);
        ASSERT_IO_SUCCESSFUL(sz==sizeof r,"Reading uint8");
        return r;
}}}


static inline float ods_read_float (const Ogre::DataStreamPtr &f)
{{{
        unsigned long val = ods_read_u32(f);
        return *(float*)(void*)&val;
}}}

static inline std::string ods_read_fixedstr(Ogre::DataStreamPtr f, size_t n)
{{{
        //amount_read+=n;
        char *buf = (char*)malloc(n+1);
        memset(buf,0,n+1);
        size_t r = f->read(buf,n);
        ASSERT_IO_SUCCESSFUL(r==n,"Reading string");
        std::string s(buf);
        free(buf);
        return s;
}}}


static inline void ods_read_bytearray (Ogre::DataStreamPtr f,
                                       unsigned char *ptr, size_t sz)
{{{
        size_t r = f->read(ptr, (sizeof *ptr) * sz);
        ASSERT_IO_SUCCESSFUL(r==(sizeof *ptr)*sz,"Reading byte array");
}}}


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
