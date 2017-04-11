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

#ifndef VECT_UTIL_H
#define VECT_UTIL_H

#include <vector>
#include <algorithm>

#include <centralised_log.h>

/** Remove the element at index from the vector vect in O(1) time by popping it
 * from the back. */
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

/** Inherit from this struct to get the index member variable required by
 * fast_erase_vector. */
struct fast_erase_index {
    size_t _index;
};

namespace {
    /** A utility function to dereference a value if required. */
    template<class T> struct maybe_deref {
        static T &_ (T &v)
        {
            return v;
        }
    };

    /** A utility function to dereference a value if required. */
    template<class T> struct maybe_deref<T*> {
        static T &_ (T *v)
        {
            return *v;
        }
    };
}

/** A class that has most of the useful functionality of std::vector but has
 * O(1) removal time and no ordering guarantees.  The vector can only contain
 * objects with a member variable called _index, which is updated to hold that
 * objects index when it is moved.  Pointers to such objects are also allowed.
 * */
template<class T> class fast_erase_vector {
    public:

    /** Add an object to the vector. */
    void push_back (const T &v)
    {
        maybe_deref<T>::_(v)._index = vect.size();
        vect.push_back(v);
    }

    /** Remove an object from the vector. */
    void erase (const T &v)
    {
        size_t index = maybe_deref<T>::_(v)._index;
        vect_remove_fast(vect, index);
        if (vect.size() == index) return; // v happened to be at the end
        // vect[index] now contains something else, update it
        maybe_deref<T>::_(vect[index])._index = index;
    }

    /** Return the number of objects present. */
    size_t size (void) const { return vect.size(); }

    /** Remove all objects from the vector. */
    void clear (void) { vect.clear(); }

    /** Prepare the vector for holding the given number of objects. */
    void reserve (size_t n) { vect.reserve(n); }

    /** Vector capacity.
     *
     * Return the number of objects this vector can hold before it must reallocate its internal
     * memory.
     */
    size_t capacity (void) const { return vect.capacity(); }

    /** Look up a particular index.
     * Warning: This is only useful for iterations because the indexes are not stable.  They change
     * during remove operations.
     */
    T &operator[] (size_t i) { return vect[i]; }

    /** Look up a particular index.
     * Warning: This is only useful for iterations because the indexes are not stable.  They change
     * during remove operations.
     */
    const T &operator[] (size_t i) const { return vect[i]; }

    const std::vector<T> &rawVector (void) { return vect; }

    private:
    std::vector<T> vect;
};

#endif
