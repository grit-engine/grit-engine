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

#include <string>
#include <sstream>
#include <ostream>

#ifndef OPTION_H
#define OPTION_H

#include <exception.h>

/** Abstract base class for an option (i.e. a configurable parameter of one of
 * the Grit subsystems such as gfx_option) that defines the valid range of
 * values. */
template<class T> struct ValidOption {

    virtual ~ValidOption (void) { }

    /** Returns whether or not the given value is part of the range. */
    virtual bool isValid (T v) = 0;

    /** Writes to the stream a string of the form "must be <property of values
     * that are valid in this range>" . */
    virtual void err (ExceptionStream &o) = 0;

    /** Throws an error with an appropriate message including the name of the
     * option if the given value is not valid. */
    virtual void maybeThrow (const std::string name, T v)
    {
        if (!isValid(v)) {
            ExceptionStream ex;
            ex << name << " option: new value " << v << ": ";
            err(ex);
            ex << ENDL;
        }
    }
};

/** An implementation of ValidOption that allows a dense range of values [min, max].
 *
 * T must have <= and >= defined.
 */
template<class T> struct ValidOptionRange : ValidOption<T> {
    T min, max;
    ValidOptionRange (T min_, T max_) : min(min_), max(max_) { }
    virtual bool isValid (T v) { return v>=min && v<=max; }
    virtual void err (ExceptionStream &o) { o << "must be between "<<min<<" and "<<max; }
};

/** An implementation of ValidOption that allows a defined list of values.
 *
 * A must be of type T[n] for some literal n.
 */
template<class T, class A> struct ValidOptionList : ValidOption<T> {
    A list;
    /** The array of valid options is passed by reference. */
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
    virtual void err (ExceptionStream &o)
    {
        o << "must be one of [";
        for (unsigned i=0 ; i<sizeof(A)/sizeof(T) ; ++i)
            o << (i==0?"":", ") << list[i];
        o << "]";
    }   
};  

#endif

