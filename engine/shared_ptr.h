/* Copyright (c) The Grit Game Engine authors 2016
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

#ifndef SharedPtr_h
#define SharedPtr_h

#include <cstdlib>
#include <functional>

/** A smart pointer that does reference counting and calls delete on the target
 * when the number of references hits zero.  Note that unlike Ogre::SharedPtr,
 * this one is not thread-safe (i.e. you cannot make reference or unreference
 * the target in concurrent threads or the counter updates will race). */
template<class T> class SharedPtr {

    /** The target. */
    T *ptr;

    /** The number of references to the target.  Note that to allow a general
     * T, we allocate the counter separately.  It would also be possible to require
     * a counter inside the T object. */
    unsigned int *cnt;

public:

    /** Deference to the target. */
    T& operator*() const { return *ptr; }

    /** Deference to the target (-> form). */
    T* operator->() const { return ptr; }

    /** Is the target NULL? */
    bool isNull (void) const { return cnt==NULL; }

    /** Set the target to NULL. */
    void setNull (void) {
        if (isNull()) return;
        useCount()--;
        if (useCount()==0) {
            delete cnt;
            delete ptr;
        }
        ptr = NULL;
        cnt = NULL;
    }

    /** Return the number of references to the target. */
    unsigned int &useCount (void) const { return *cnt; }

    /** Create with a NULL target. */
    SharedPtr (void) : ptr(NULL), cnt(NULL) { }

    /** Create with a given raw pointer as a target.  The raw pointer should be
     * used only through the SharedPtr from this point onwards, as it can be
     * difficult to know when raw pointers to reference-counted memory become
     * dangling. */
    explicit SharedPtr (T *p) : ptr(p), cnt(p==NULL?NULL:new unsigned int(1)) { }

    /** Make a new reference to an existing SharedPtr (incrementing the reference counter). */
    SharedPtr (const SharedPtr<T> &p) : ptr(p.ptr), cnt(p.cnt) { if (!isNull()) useCount()++; }

    /** Destructor (decrements the reference counter). */
    ~SharedPtr (void) { setNull(); }

    /** Make this SharedPtr reference the target of SharedPtr p (incrementing the reference counter). */
    SharedPtr &operator= (const SharedPtr<T> &p) {
        // The next line is not just an optimisation, it is needed for correctness
        // without it if *cnt==1 then the setNull() would free the storage,
        // in what ought to be a no-op.  Obviously this would cause considerable drama.
        if (p==*this) return *this;
        T *ptr_ = p.ptr;
        unsigned int *cnt_ = p.cnt;
        setNull();
        ptr = ptr_;
        cnt = cnt_;
        if (!isNull()) useCount()++;
        return *this;
    }

    /** Return a SharedPtr to the same object that has a supertype (according to c++ static_cast rules). */
    template<class U> SharedPtr<U> staticCast (void) const
    {
        SharedPtr<U> r;
        r.cnt = cnt;
        r.ptr = static_cast<U*>(ptr);
        if (!isNull()) useCount()++;
        return r;
    }

    template<class U> friend class SharedPtr;
};

/** Do the targets match? */
template<class T, class U> inline bool operator==(SharedPtr<T> const& a, SharedPtr<U> const& b)
{ return &*a == &*b; }

/** Are the targets different? */
template<class T, class U> inline bool operator!=(SharedPtr<T> const& a, SharedPtr<U> const& b)
{ return ! (a==b); }

/** Call std::less on the targets. */
template<class T, class U> inline bool operator<(SharedPtr<T> const& a, SharedPtr<U> const& b)
{ return std::less<const void*>()(&*a, &*b); }

#endif
