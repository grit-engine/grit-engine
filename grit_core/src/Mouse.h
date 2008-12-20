#ifndef Mouse_h
#define Mouse_h

#include <vector>

#define NUM_BUTTONS 8

class Mouse {

public:

        enum Button
        {
                MOUSE_LEFT = 1, MOUSE_RIGHT, MOUSE_MIDDLE,
                MOUSE_SCROLL_UP, MOUSE_SCROLL_DOWN
        };

        virtual ~Mouse() = 0;

        virtual bool getEvents(std::vector<int> *clicks,
                               int *x, int *y, int *rel_x, int *rel_y) = 0;

        virtual void setPos(int x, int y) = 0;

        virtual void setHide(bool toggle) = 0;
        virtual bool getHide() = 0;

        virtual void setGrab(bool toggle) = 0;
        virtual bool getGrab() = 0;

};

inline Mouse::~Mouse() { }

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
