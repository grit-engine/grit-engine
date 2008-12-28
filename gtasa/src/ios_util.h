#include <sstream>
#include <iostream>

#include <cstdlib>
#include <cstring>
#include <cerrno>

#ifndef ios_util_h
#define ios_util_h

class Exception : public std::exception
{

    public:

        Exception (const std::string &msg_,
                   const char *file_,
                   unsigned long line_)
                : msg(msg_), file(file_), line(line_) { }

        virtual ~Exception (void) throw () { }

        const std::string &getMsg (void) const { return msg; }

        const char *getFile (void) const { return file; }

        unsigned long getLine (void) const { return line; }

        std::string getFullDescription (void) const
        {
                std::stringstream ss;
                ss << msg << " at ("<<file<<":"<<line<<")";
                return ss.str();
        }

    protected:

        const std::string msg;

        const char * const file;

        unsigned long line;

};


#define ASSERT(x) if (!(x)) { \
        IOS_EXCEPT("Assertion failed: "#x); \
} else { }


#define IOS_EXCEPT(msg) throw Exception(msg,__FILE__,__LINE__)

#define ASSERT_IO_SUCCESSFUL(in,doing) if (!(in).good()) { \
        if ((in).eof()) { \
                IOS_EXCEPT(std::string("EOF while ")+doing); \
        } else { \
                IOS_EXCEPT(std::string(strerror(errno))+std::string(" while ")+doing); \
        } \
}

/* the following routines do IO on little endian files.  The host system can be
 * any endian. */

static inline unsigned long ios_read_u32(std::istream &in)
{{{
        unsigned char raw[4];
        in.read((char*)raw,sizeof raw);
        ASSERT_IO_SUCCESSFUL(in,"reading uint32");
        return raw[3]<<24ul | raw[2]<<16ul | raw[1]<<8ul | raw[0];
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
        ASSERT_IO_SUCCESSFUL(in,"reading uint16");
        return raw[1]<<8ul | raw[0];
}}}

static inline signed short ios_read_s16(std::istream &in)
{{{
        unsigned char raw[2];
        in.read((char*)raw,sizeof raw);
        ASSERT_IO_SUCCESSFUL(in,"reading int16");
        short r = raw[0];
        r += (((int)raw[1]&0x7F)<<8);
        if (raw[1]&0x80) r -= 0x8000;
        return r;
}}}

static inline unsigned char ios_read_u8(std::istream &in)
{{{
        unsigned char raw[1];
        in.read((char*)raw,sizeof raw);
        ASSERT_IO_SUCCESSFUL(in,"reading uint8");
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
                ASSERT_IO_SUCCESSFUL(in,"reading byte array");
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
        ASSERT_IO_SUCCESSFUL(in,"reading string");
        std::string s(raw);
        delete [] raw;
        return s;
}}}

static inline void ios_write_u32(std::ostream &out, unsigned long v)
{{{
        out << (unsigned char)((v & 0x000000FF) >> 0);
        ASSERT_IO_SUCCESSFUL(out,"writing uint32");
        out << (unsigned char)((v & 0x0000FF00) >> 8);
        ASSERT_IO_SUCCESSFUL(out,"writing uint32");
        out << (unsigned char)((v & 0x00FF0000) >> 16);
        ASSERT_IO_SUCCESSFUL(out,"writing uint32");
        out << (unsigned char)((v & 0xFF000000) >> 24);
        ASSERT_IO_SUCCESSFUL(out,"writing uint32");
}}}

static inline void ios_write_u16(std::ostream &out, unsigned short v)
{{{
        out << (unsigned char)((v & 0x00FF) >> 0);
        ASSERT_IO_SUCCESSFUL(out,"writing uint16");
        out << (unsigned char)((v & 0xFF00) >> 8);
        ASSERT_IO_SUCCESSFUL(out,"writing uint16");
}}}

static inline void ios_write_u8(std::ostream &out, unsigned char v)
{{{
        out << (unsigned char)((v & 0x00FF) >> 0);
        ASSERT_IO_SUCCESSFUL(out,"writing uint8");
}}}

static inline void ios_write_str(std::ostream &out, const std::string &s)
{{{
        ios_write_u32(out,s.length());
        out << s;
        ASSERT_IO_SUCCESSFUL(out,"writing string");
}}}

static inline void ios_write_byte_array(std::ostream &out,
                                        const unsigned char *raw,size_t sz)
{{{
        out.write((char*)raw,sz);
        ASSERT_IO_SUCCESSFUL(out,"writing byte array");
}}}

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
        IOS_EXCEPT(s.str());
    }
}}}


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
