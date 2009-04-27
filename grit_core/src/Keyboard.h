#ifndef Keyboard_h

#define Keyboard_h

#include <vector>


class Keyboard {

public:

        typedef std::vector<const char*> Presses;

        virtual ~Keyboard() = 0;

        virtual Presses getPresses() = 0;

        virtual void lostFocus() = 0;

};

inline Keyboard::~Keyboard() {}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
