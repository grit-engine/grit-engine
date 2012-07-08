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

#ifndef VECT_UTIL_H
#define VECT_UTIL_H

#include <vector>
#include <algorithm>

#include "CentralisedLog.h"

template<class T> void vect_remove_fast (std::vector<T> &vect, size_t index)
{
    APP_ASSERT(index < vect.size());
	// I would use std::swap at this point, but apparently that makes visual studio crash...
	if (index != vect.size()-1) {
		T tmp = vect[vect.size()-1];
		vect[index] = tmp;
	}
    vect.pop_back();
}

struct fast_erase_index {
    size_t _index;
};

namespace {
    template<class T> struct maybe_deref {
        static T &_ (T &v)
        {
            return v;
        }
    };

    template<class T> struct maybe_deref<T*> {
        static T &_ (T *v)
        {
            return *v;
        }
    };
}

template<class T> class fast_erase_vector {
    public:

    void push_back (const T &v)
    {
        maybe_deref<T>::_(v)._index = vect.size();
        vect.push_back(v);
    }

    void erase (const T &v)
    {
        size_t index = maybe_deref<T>::_(v)._index;
        vect_remove_fast(vect, index);
        if (vect.size() == index) return; // v happened to be at the end
        // vect[index] now contains something else, update it
        maybe_deref<T>::_(vect[index])._index = index;
    }

    size_t size (void) { return vect.size(); }

    void clear (void) { vect.clear(); }

    T &operator[] (size_t i) { return vect[i]; }

    private:
    std::vector<T> vect;
};

#endif
