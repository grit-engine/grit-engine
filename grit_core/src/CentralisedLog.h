#ifndef CentralisedLog_h
#define CentralisedLog_h

#include <iostream>

#include <OgreLog.h>

class CentralisedLog : public Ogre::LogListener {

    public:

        virtual void messageLogged (const Ogre::String &message,
                                    Ogre::LogMessageLevel lml,
                                    bool maskDebug,
                                    const Ogre::String &logName)
        {
                echo(message);
        }

        virtual void echo (const Ogre::String &line)
        {
                std::cout << line << std::endl;
                buffer << line << std::endl;
        }

        virtual Ogre::String consolePoll (void) 
        {
                Ogre::String r = buffer.str();
                buffer.clear();
                return r;
        }
            

    protected:

        std::stringstream buffer;

};

extern CentralisedLog clog;

#endif

// vim: tabstop=8:shiftwidth=8:expandtab
