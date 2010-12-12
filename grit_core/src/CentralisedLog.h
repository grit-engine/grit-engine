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

#ifndef CentralisedLog_h
#define CentralisedLog_h

#include <iostream>

#include <OgreLog.h>

#include "console_colour.h"

void app_fatal();

class CentralisedLog {

    public:

        virtual void echo (const std::string &line)
        {
                std::cout << line << RESET << std::endl;
                buffer << line << RESET << std::endl;
        }

        virtual std::string consolePoll (void) 
        {
                std::string r = buffer.str();
                buffer.str("");
                return r;
        }
            
        // used for storing incomplete lines (up to std::endl)
        std::stringstream tmp;

    protected:

        std::stringstream buffer;

};

extern CentralisedLog clog;

class CLog {
    public:

        CLog (const char *file, int line, bool error)
        {
                if (error) {
                        clog.tmp << BOLD << RED << "ERROR" << RESET;
                } else {
                        clog.tmp << BOLD << BLUE << "VERBOSE" << RESET;
                }       
                clog.tmp<<" ("<<BOLD<<file<<NOBOLD
                        <<":"<<BOLD<<line<<NOBOLD<<"): ";
        }

        CLog () { }

        typedef std::ostream &manip(std::ostream&);

        CLog &operator<< (manip *o)
        {
                if (o == (manip*)std::endl) {
                        clog.echo(clog.tmp.str());
                        clog.tmp.str("");
                } else {
                        clog.tmp << o;
                }
                return *this;
        }

        template<typename T> CLog &operator<<(T const &o)
        {
                clog.tmp << o;
                return *this;
        }

};

/*
CLOG << "hello world" << std::endl;
CERR << "hello world" << std::endl;
CVERB << "hello world" << std::endl;
*/

#define CERR CLog(__FILE__,__LINE__,true)
#define CLOG CLog()
#define CVERB CLog(__FILE__,__LINE__,false)

#define APP_ASSERT(cond) do { \
        if (!(cond)) { \
                CERR << "assertion failed: " << #cond << std::endl;\
        } \
} while (0)

#define APP_PERROR_ASSERT(cond) do { \
        if (!(cond)) {\
                CERR << "assertion failed: " << #cond << std::endl;\
                perror("perror says"); \
        } \
} while (0)

struct GritException {
        GritException (const std::string &msg_, const char *func_, const char *file_, int line_)
              : msg(msg_), func(func_), file(file_), line(line_) { }

        GritException (const std::string &msg_, const std::string &doing,
                       const char *func_, const char *file_, int line_)
              : msg(msg_), func(func_), file(file_), line(line_)
        {
                msg += " while " + doing;
        }
        
        std::string longMessage (void)
        {
                std::stringstream ss;
                ss << msg << " (" << func << " at " << file << ":" << line << ")";
                return ss.str();
        }

        std::string msg;
        const char *func;
        const char *file;
        int line;
};

inline std::ostream &operator << (std::ostream &o, GritException &e)
{ o << e.msg; return o; }

#if defined(_MSC_VER)
//#define GRIT_FUNC_NAME __FUNCDNAME__ 
#define GRIT_FUNC_NAME __FUNCSIG__ 
//#define GRIT_FUNC_NAME __FUNCTION__ 
#elif defined (__GNUC__)
#define GRIT_FUNC_NAME __PRETTY_FUNCTION__
#else
#define GRIT_FUNC_NAME "<unknown func>"
#endif

#define GRIT_EXCEPT(msg) throw GritException(msg, GRIT_FUNC_NAME, __FILE__, __LINE__)
#define GRIT_EXCEPT2(msg,doing) throw GritException(msg, doing, GRIT_FUNC_NAME, __FILE__, __LINE__)

#endif

// vim: tabstop=8:shiftwidth=8:expandtab
