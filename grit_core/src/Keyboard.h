#ifndef Keyboard_h

#define Keyboard_h

#include <vector>


class Keyboard {

public:

        typedef std::string Press;

        typedef std::vector<Press> Presses;

        Keyboard() : flushRequested(false), verbose(false) { }

        virtual ~Keyboard() = 0;

        virtual Presses getPresses() = 0;

        virtual void flush() { flushRequested = true; }

        virtual Press getShifted(const Press &) = 0;

        virtual bool hasFocus() = 0;

        virtual void setVerbose (bool v) { verbose = v; }
        virtual bool getVerbose (void) { return verbose; }

protected:

        bool flushRequested;

        bool verbose;

};

inline Keyboard::~Keyboard() {}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
