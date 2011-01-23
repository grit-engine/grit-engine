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

#ifndef SharedPtr_h
#define SharedPtr_h

#include <cstdlib>
#include <functional>

// Totally unthreadsafe
template<class T> class SharedPtr {
    T *ptr;
    unsigned int *cnt;
public:
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    bool isNull (void) const { return cnt==NULL; }
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
    const unsigned int &useCount (void) const { return *cnt; }
    unsigned int &useCount (void) { return *cnt; }
    SharedPtr (void) : ptr(NULL), cnt(NULL) { }
    explicit SharedPtr (T *p) : ptr(p), cnt(p==NULL?NULL:new unsigned int(1)) { }
    SharedPtr (const SharedPtr<T> &p) : ptr(p.ptr), cnt(p.cnt) { if (!isNull()) useCount()++; }
    ~SharedPtr (void) { setNull(); }
    SharedPtr &operator= (const SharedPtr<T> &p) {
        T *ptr_ = p.ptr;
        unsigned int *cnt_ = p.cnt;
        setNull();
        ptr = ptr_;
        cnt = cnt_;
        if (!isNull()) useCount()++;
        return *this;
    }
};

template<class T, class U> inline bool operator==(SharedPtr<T> const& a, SharedPtr<U> const& b)
{ return &*a == &*b; }

template<class T, class U> inline bool operator!=(SharedPtr<T> const& a, SharedPtr<U> const& b)
{ return ! (a==b); }

template<class T, class U> inline bool operator<(SharedPtr<T> const& a, SharedPtr<U> const& b)
{ return std::less<const void*>()(&*a, &*b); }

#endif
