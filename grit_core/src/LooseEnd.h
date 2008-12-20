#include <vector>

#ifndef LooseEnd_h
#define LooseEnd_h


struct LooseEnd { virtual ~LooseEnd(void) { } };

template <class T> class LooseEndImpl : public LooseEnd {
    public:
        LooseEndImpl (T *ptr_) : ptr(ptr_) { }
    protected:
        virtual ~LooseEndImpl (void) { delete ptr; }
        T *ptr;
};

template <class T> class LooseEndArray : public LooseEnd {
    public:
        LooseEndArray (T *ptr_) : ptr(ptr_) { }
    protected:
        virtual ~LooseEndArray (void) { delete [] ptr; }
        T *ptr;
};

typedef std::vector<LooseEnd*> LooseEnds;

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
