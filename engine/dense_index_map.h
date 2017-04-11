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

class DenseIndexMap;

#ifndef DENSE_INDEX_MAP_H
#define DENSE_INDEX_MAP_H

#include <vector>

// Maps fixed (sparse) indexes to a dense list, by keeping an internal
// dense list of indexes that are reassigned to fill the holes.
// The assignment of dense indexes changes often, but it never has any
// 'holes'. On the other hand, sparse indexes may be unused sometimes, and
// are retained in a free list for future use.
class DenseIndexMap {
    
    protected:

    // CLASS INVARIANT: freeList.size() + sparseIndexes.size() == denseIndexes.size()

    // Gives the sparse index of a given dense index
    std::vector<unsigned int> sparseIndexes;
    
    // Gives the dense index of a given sparse index
    std::vector<unsigned int> denseIndexes;

    // Unused sparse indexes
    std::vector<unsigned int> freeList;


    public:
    
    // Class does not do range checking internally, since indexes are often not computed
    // This utility function will check if an index is valid.  Call it if you want.
    void sparseIndexValid (unsigned sparse_index) const;

    // Looking up an index from the free list in denseIndexes gets you 0xFFFF
    unsigned denseIndex (unsigned sparse_index) const { return denseIndexes[sparse_index]; }

    // Reserve is called implicitly by newIndex, or you can call it yourself.
    unsigned reserve (unsigned new_capacity); 

    // Number of allocated indexes
    unsigned size (void) const { return sparseIndexes.size(); }

    // Number of reserved indexes
    unsigned capacity (void) const { return denseIndexes.size(); }

    // Return a new sparse index from the freeList and allocate space for it
    unsigned newSparseIndex (void);

    // Return an index to the pool
    void delSparseIndex (unsigned index);
};      


template<class T> class DenseIndexMapWithCargo : public DenseIndexMap {

    protected:

    std::vector<T> cargo;

    public:

    T &operator [] (unsigned i) { return cargo[i]; }
    const T &operator [] (unsigned i) const { return cargo[i]; }

    unsigned newSparseIndex (const T &init)
    {
        unsigned i = DenseIndexMap::newSparseIndex();
        cargo.push_back(init);
        return i;
    }

    unsigned newSparseIndex (void) { return newSparseIndex(T()); }

    unsigned reserve (unsigned new_capacity)
    {
        new_capacity = DenseIndexMap::reserve(new_capacity);
        cargo.reserve(new_capacity);
        return new_capacity;
    }
    
    void delSparseIndex (unsigned index)
    {
        unsigned dense_index = denseIndexes[index];
        unsigned last = sparseIndexes.size()-1;
        if (dense_index != last) {
            // reorganise buffer to move last instance into the position of the one just removed
            cargo[dense_index] = cargo[last];
        }
        cargo.pop_back();
        DenseIndexMap::delSparseIndex(index);
    }
};
    
#endif
