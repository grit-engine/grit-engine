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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef _MSC_VER
        typedef unsigned char uint8_t;
        typedef unsigned short uint16_t;
        typedef signed short int16_t;
        typedef unsigned long uint32_t;
#else
        #include <stdint.h>
#endif

#include <string>

typedef uint8_t u8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef float f32;

////////////////////////////////////////////////////////////////////////////////

#ifndef portable_io_h
#define portable_io_h

#define rassert(x) if (!(x)) { \
        if (feof(f)) { \
                fprintf(stderr,"EOF on read!"); \
        } else { \
                perror("Read error"); \
        } \
        fprintf(stderr,"%s:%d\n", __FILE__,__LINE__);  \
        fflush(stderr); \
}

#define wassert(x) if (!(x)) { \
        if (feof(f)) { \
                fprintf(stderr,"EOF on write!"); \
        } else { \
                perror("Write error"); \
        } \
        fprintf(stderr,"%s:%d\n", __FILE__,__LINE__);  \
        fflush(stderr); \
}

//extern size_t amount_read;
//extern size_t amount_seeked;

/* the following routines do FILE* io, where the file is little endian, and the
 * host system can be any endian. */

static inline void fread_u32_(FILE *f, u32 *ptr)
{{{
#ifdef OGRE_CONFIG_BIG_ENDIAN
        //amount_read+=4;
        unsigned char raw[4];
        size_t r=fread(raw,sizeof raw, 1, f);
        rassert(r==1);
        if (ptr) {
                *ptr = raw[0];
                *ptr += raw[1] << 8;
                *ptr += raw[2] << 16;
                *ptr += raw[3] << 24;
        }
#else
        if (ptr) {
                //amount_read+=4;
                size_t r=fread(ptr,sizeof *ptr, 1, f);
                rassert(r==1);
        } else {
                //amount_seeked+=4;
                fseek(f,4,SEEK_CUR);
        }
#endif
}}}

static inline void fread_u16_(FILE *f, u16 *ptr)
{{{
        //amount_read+=2;
        unsigned char raw[2];
        size_t r=fread(raw,sizeof raw, 1, f);
        rassert(r==1);
        if (ptr) {
                *ptr = raw[0];
                *ptr += raw[1] << 8;
        }
}}}

static inline void fread_s16_(FILE *f, s16 *ptr)
{{{
        //amount_read+=2;
        unsigned char raw[2];
        size_t r=fread(raw,sizeof raw, 1, f);
        rassert(r==1);
        if (ptr) {
                *ptr = raw[0];
                if (raw[1]&0x80) {
                        *ptr -= 0x8000;
                }
                *ptr += (raw[1]&0x7F) << 8;
        }
}}}

static inline void fread_u8_(FILE *f, u8 *ptr)
{{{
        if (ptr) {
                //amount_read+=1;
                size_t r=fread(ptr,sizeof *ptr, 1, f);
                rassert(r==1);
        } else {
                //amount_seeked+=1;
                fseek(f,1,SEEK_CUR);
        }
}}}


static inline void fwrite_u32(FILE *f, u32 v)
{{{
        size_t r=fwrite(&v,sizeof v, 1, f);
        wassert(r==1);
}}}

static inline void fwrite_u16(FILE *f, u16 v)
{{{
        size_t r=fwrite(&v,sizeof v, 1, f);
        wassert(r==1);
}}}

static inline void fwrite_u8(FILE *f, u8 v)
{{{
        size_t r=fwrite(&v,sizeof v, 1, f);
        wassert(r==1);
}}}


static inline void fread_bytearray(FILE *f, u8 *ptr, size_t sz)
{{{
        if (ptr) {
                //amount_read+=sz;
                size_t r = fread(ptr, sizeof *ptr, sz, f);
                rassert(r==sz);
        } else {
                //amount_seeked+=sz;
                int r = fseek(f, sz, SEEK_CUR);
                rassert (r==0);
        }
}}}


static inline std::string fread_fixedstr(FILE *f, size_t n)
{{{
        //amount_read+=n;
        std::string s;
        s.reserve(n+1);
        char *buf = (char*)malloc(n+1);
        memset(buf,0,n+1);
        size_t r = fread(buf,n, 1, f);
        rassert(r==1);
        s += buf;
        free(buf);
        return s;
}}}

static inline void fwrite_fixedstr(FILE *f, const std::string& str, size_t n)
{{{
        size_t len = str.size() + 1;
        wassert(len<=n);
        size_t r = fwrite(str.c_str(),len, 1, f);
        wassert(r==1);
        while (n>len) {
                const u8 zero = 0;
                r = fwrite(&zero,1, 1, f);
                wassert(r==1);
                len++;
        }
}}}

static inline void fwrite_str(FILE *f, const std::string& str)
{{{
        size_t len = str.size() + 1;
        fwrite_u32(f,len);
        fwrite_fixedstr(f,str,len);
}}}


static inline u8 fread_u8(FILE *f)
{{{
        //amount_read+=1;
        u8 data;
        size_t r=fread(&data,sizeof data, 1, f);
        rassert(r==1);
        return data;
}}}

static inline u16 fread_u16(FILE *f)
{ u16 data; fread_u16_(f,&data); return data; }

static inline s16 fread_s16(FILE *f)
{ s16 data; fread_s16_(f,&data); return data; }

static inline u32 fread_u32(FILE *f)
{ u32 data; fread_u32_(f,&data); return data; }


static inline void fread_f32_(FILE *f, f32 *ptr)
{{{
        // assumes 
        if (ptr) {
                #ifdef OGRE_CONFIG_BIG_ENDIAN
                        u32 val = fread_u32(f);
                        memcpy(ptr,&val,4);
                #else
                        //amount_read+=4;
                        size_t r=fread(ptr,sizeof *ptr, 1, f);
                        rassert(r==1);
                #endif
        } else {
                //amount_seeked+=4;
                fseek(f,4,SEEK_CUR);
        }
}}}

static inline f32 fread_f32(FILE *f)
{ f32 data; fread_f32_(f,&data); return data; }


#define fread_header(f,type,size,ver,vercheck) \
        fread_header_(f,type,size,ver,vercheck,__FILE__,__LINE__)

static inline void fread_header_(FILE *f, u32 *type, u32 *size,
                                 u32 *ver, u32 *vercheck,
                                 const char* file, int line)
{{{
        fread_u32_(f,type);
        fread_u32_(f,size);
        u32 ver2 = fread_u32(f);
        if (ver) *ver=ver2;
        //printf("[0;34mversion: %d (0x%x)[0m\n",ver2,ver2);
        if(vercheck && ver2!=*vercheck) {
                fprintf(stderr,"Expected version %d (0x%x), "
                               "got version %d (0x%x), at %s:%d.\n",
                               *vercheck,*vercheck,ver2,ver2,file,line);
        }
}}}

static inline size_t ftell_(FILE *f)
{ return ftell(f); }

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
