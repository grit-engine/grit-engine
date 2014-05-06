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

#include <algorithm>
#include <sstream>

#include "dense_index_map.h"
#include "centralised_log.h"

unsigned DenseIndexMap::reserve (unsigned new_capacity)
{
    unsigned old_capacity = capacity();
    if (new_capacity <= old_capacity) return old_capacity;

    sparseIndexes.reserve(new_capacity);
    freeList.reserve(new_capacity);
    denseIndexes.resize(new_capacity);
    for (unsigned i=old_capacity ; i<new_capacity ; ++i) {
        denseIndexes[i] = 0xFFFF;
        freeList.push_back(i);
    }

    return new_capacity;
}

void DenseIndexMap::sparseIndexValid (unsigned sparse_index) const
{
    if (sparse_index >= denseIndexes.size() || denseIndexes[sparse_index] == 0xFFFF) {
        std::stringstream ss;
        ss << "Index out of range or unassigned: " << sparse_index;
        GRIT_EXCEPT(ss.str());
    }
}

unsigned DenseIndexMap::newSparseIndex (void)
{
    if (size() >= capacity()) reserve(std::max(128u, unsigned(capacity() * 1.3)));
    unsigned sparse_index = freeList[freeList.size()-1];
    freeList.pop_back();
    unsigned dense_index = sparseIndexes.size();
    denseIndexes[sparse_index] = dense_index;
    sparseIndexes.push_back(sparse_index);
    return sparse_index;
}

void DenseIndexMap::delSparseIndex (unsigned sparse_index)
{
    unsigned dense_index = denseIndexes[sparse_index];
    unsigned last = sparseIndexes.size()-1;
    if (dense_index != last) {
        // reorganise buffers to move last instance into the position of the one just removed
        sparseIndexes[dense_index] = sparseIndexes[last];
        denseIndexes[sparseIndexes[last]] = dense_index;
    }
    sparseIndexes.resize(last);
    denseIndexes[sparse_index] = 0xFFFF;
    freeList.push_back(sparse_index);

}



