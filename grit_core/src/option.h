/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

#ifndef Option_h

#define Option_h

template<class T> struct ValidOption {
    virtual ~ValidOption (void) { }
    virtual bool isValid (T v) = 0;
    virtual void err (std::ostream &o) = 0;
    virtual void maybeThrow (const std::string name, T v)
    {
        if (!isValid(v)) {
            std::stringstream ss;
            ss << name << " option: new value " << v << ": ";
            err(ss);
            GRIT_EXCEPT(ss.str());
        }
    }
};

template<class T> struct ValidOptionRange : ValidOption<T> {
    T min, max;
    ValidOptionRange (T min_, T max_) : min(min_), max(max_) { }
    virtual bool isValid (T v) { return v>=min && v<=max; }
    virtual void err (std::ostream &o) { o << "must be between "<<min<<" and "<<max; }
};

template<class T, class A> struct ValidOptionList : ValidOption<T> {
    A list;
    ValidOptionList (A &list_)
    {
        for (unsigned i=0 ; i<sizeof(A)/sizeof(T) ; ++i)
            list[i] = list_[i];
    }       
    virtual bool isValid (T v)
    {
        for (unsigned i=0 ; i<sizeof(A)/sizeof(T) ; ++i)
            if (v==list[i]) return true;
        return false;
    }   
    virtual void err (std::ostream &o)
    {
        o << "must be one of [";
        for (unsigned i=0 ; i<sizeof(A)/sizeof(T) ; ++i)
            o << (i==0?"":", ") << list[i];
        o << "]";
    }   
};  



#endif

// vim: shiftwidth=8:tabstop=8:expandtab
