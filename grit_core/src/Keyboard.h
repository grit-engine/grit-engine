#ifndef Keyboard_h

#define Keyboard_h

#include <vector>


class Keyboard {

public:

        typedef std::vector<const char*> Presses;

        Keyboard() : flushRequested(false) { }

        virtual ~Keyboard() = 0;

        virtual Presses getPresses() = 0;

        virtual void flush() { flushRequested = true; }

protected:

        bool flushRequested;

};

inline Keyboard::~Keyboard() {}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
