#ifndef CentralisedLog_h
#define CentralisedLog_h

#include <iostream>

#include <OgreLog.h>

#include "console_colour.h"

void app_fatal();

class CentralisedLog : public Ogre::LogListener {

    public:

        virtual void messageLogged (const Ogre::String &message,
                                    Ogre::LogMessageLevel lml,
                                    bool maskDebug,
                                    const Ogre::String &logName)
        {
                (void)lml;
                (void)maskDebug;
                (void)logName;
                echo(message);
        }

        virtual void echo (const Ogre::String &line)
        {
                std::cout << line << RESET << std::endl;
                buffer << line << RESET << std::endl;
        }

        virtual Ogre::String consolePoll (void) 
        {
                Ogre::String r = buffer.str();
                buffer.str("");
                return r;
        }
            

    protected:

        std::stringstream buffer;

};

extern CentralisedLog clog;

class CLog {
    public:

        CLog (const char *file, int line, bool error)
        {
                if (error) {
                        ss << BOLD << RED << "ERROR" << RESET;
                } else {
                        ss << BOLD << BLUE << "VERBOSE" << RESET;
                }       
                ss<<" ("<<BOLD<<file<<NOBOLD<<":"<<BOLD<<line<<NOBOLD<<"): ";
        }

        CLog () { }

        typedef std::ostream &manip(std::ostream&);

        CLog &operator<< (manip *o)
        {
                if (o == (manip*)std::endl) {
                        clog.echo(ss.str());
                        ss.str("");
                } else {
                        ss << o;
                }
                return *this;
        }

        template<typename T> CLog &operator<<(T const &o)
        {
                ss << o;
                return *this;
        }

    protected:

        std::stringstream ss;
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

#endif

// vim: tabstop=8:shiftwidth=8:expandtab
