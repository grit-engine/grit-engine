#ifndef Keyboard_h

#define Keyboard_h

#include <vector>


class Keyboard {

public:

        typedef std::string Press;

        typedef std::vector<Press> Presses;

        Keyboard() : fullFlushRequested(false), verbose(false) { }

        virtual ~Keyboard() = 0;

        virtual Presses getPresses() = 0;

        virtual void flush() { fullFlushRequested = true; }

        virtual void flush(Press p) { keysToFlush.push_back(p); }

        virtual Press getShifted(const Press &) = 0;
        virtual Press getAlted(const Press &) = 0;

        virtual bool hasFocus() = 0;

        virtual void setVerbose (bool v) { verbose = v; }
        virtual bool getVerbose (void) { return verbose; }

protected:

        bool fullFlushRequested;

        bool verbose;

        Presses keysToFlush;

};

inline Keyboard::~Keyboard() {}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
