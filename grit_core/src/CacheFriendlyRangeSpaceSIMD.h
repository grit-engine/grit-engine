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

template <typename T> class CacheFriendlyRangeSpace;

#ifndef CACHEFRIENDLYRANGESPACESIMD_H
#define CACHEFRIENDLYRANGESPACESIMD_H

#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>


#include "SSEAllocator.h"
#include "CentralisedLog.h"

#ifdef near
#undef near
#endif

class SIMDVector4 {

    protected:

        union U {
                struct Raw {
                        float x, y, z, d;
                } raw;
                #ifdef WIN32
                __m128 simd;
                #else
                float simd __attribute__ ((vector_size (16)));
                #endif
        };


    public:

        inline void updateAll (float x, float y, float z, float d)
        {
                u.raw.x = x;
                u.raw.y = y;
                u.raw.z = z;
                u.raw.d = d;
        }

        inline float x(void) const { return u.raw.x; }
        inline float y(void) const { return u.raw.y; }
        inline float z(void) const { return u.raw.z; }
        inline float d(void) const { return u.raw.d; }

#ifdef WIN32
        SIMDVector4 operator- (const SIMDVector4 &b) const
        {
                SIMDVector4 r;
                r.u.simd = _mm_sub_ps(u.simd, b.u.simd);
                return r;
        }

        SIMDVector4 operator* (const SIMDVector4 &b) const
        {
                SIMDVector4 r;
                r.u.simd = _mm_mul_ps(u.simd, b.u.simd);
                return r;
        }
#else
        SIMDVector4 operator- (const SIMDVector4 &b) const
        {
                SIMDVector4 r;
                r.u.simd = u.simd - b.u.simd;
                return r;
        }

        SIMDVector4 operator* (const SIMDVector4 &b) const
        {
                SIMDVector4 r;
                r.u.simd = u.simd * b.u.simd;
                return r;
        }
#endif
        SIMDVector4 &operator*= (SIMDVector4 &b)
        {
                *this = *this * b;
                return *this;
        }

     protected:

        U u;
};

// msvc needs a horrible wrapper class because it can't handle aligned parameters
// thus operations like resize() and push_back() cause compile errors
#ifdef WIN32
class SIMDVector4s {

    public:

        struct Stupid { float x, y, z, d; };
        typedef std::vector<Stupid,SSEAllocator<Stupid> > Wrapped;
        typedef Wrapped::size_type size_type;

        void pop_back (void) { wrapped.pop_back(); }

        void push_back (const SIMDVector4 &v)
        { wrapped.push_back(reinterpret_cast<const Stupid&>(v)); }

        void clear (void) { wrapped.clear(); }

        SIMDVector4 &operator[] (size_type index)
        { return reinterpret_cast<SIMDVector4&>(wrapped[index]); }

        const SIMDVector4 &operator[] (size_type index) const
        { return reinterpret_cast<const SIMDVector4&>(wrapped[index]); }

        Wrapped::size_type size (void) const { return wrapped.size(); }


    protected:
        Wrapped wrapped;
};
#else
typedef std::vector<SIMDVector4,SSEAllocator<SIMDVector4> > SIMDVector4s;
#endif

template <typename T>
class CacheFriendlyRangeSpace {

    public:

        typedef std::vector<T> Cargo;

        CacheFriendlyRangeSpace() : hence(0) { }

        ~CacheFriendlyRangeSpace() { }

        void add (const T &o)
        {
                typename Cargo::iterator begin = cargo.begin(),
                                             end = cargo.end();
                // if it's already in there, this is a no-op
                if (find(begin,end,o)!=end) return;
                
                size_t index = cargo.size();
                cargo.push_back(o);
                positions.push_back(SIMDVector4());
                o->updateIndex(index);
        }

        inline void updateSphere (size_t index, Ogre::Real x,
                                  Ogre::Real y, Ogre::Real z,
                                  Ogre::Real d)
        {
                positions[index].updateAll(x,y,z,d);
        }

        // only meaningful if the radius is stored in pos.d and
        // other.d is 0, thus the subtraction and squaring
        // yields d^2
        static inline bool near (const SIMDVector4 &pos,
                                 const SIMDVector4 &centre,
                                 const float factor2)
        {
                SIMDVector4 diff;
                diff = pos - centre;
                diff *= diff;
                return diff.x() + diff.y() + diff.z()
                        < diff.d() * factor2;
        }

        void getPresent (const Ogre::Real x,
                         const Ogre::Real y,
                         const Ogre::Real z,
                         size_t num,
                         const Ogre::Real factor,
                         Cargo &found)
        {
                if (num==0) return;
                if (cargo.size()==0) return;
                if (num>cargo.size()) num=cargo.size();
                if (hence>=cargo.size()) hence = 0;

                // iterate from this point for a while
                SIMDVector4s::size_type iter = hence, end = positions.size();                

                if (num>positions.size()) {
                        iter = 0;
                        num = positions.size();
                }

                SIMDVector4 home;
                float factor2 = factor*factor;
                home.updateAll(x,y,z,0);
                for (SIMDVector4s::size_type i=0 ; i<num ; ++i) {
                        SIMDVector4 &pos = positions[iter];
                        if (near(home,pos,factor2)) {
                                found.push_back(cargo[iter]);
                        }
                        iter++;
                        if (iter==end) iter=0;
                }

                hence = iter;
        }

        virtual void remove (const T &o)
        {
                typename Cargo::iterator begin = cargo.begin(),
                                           end = cargo.end();
                typename Cargo::iterator iter = find(begin,end,o);

                // no-op if o was not in the rangespace somewhere
                if (iter==end) return;

                // otherwise, carefully remove it -
                size_t index = iter - begin;

                positions[index] = positions[positions.size()-1];
                positions.pop_back();
                cargo[index] = cargo[cargo.size()-1];
                cargo[index]->updateIndex(index);
                cargo.pop_back();
                o->updateIndex(-1);
        }

        virtual void clear (void)
        {
                cargo.clear();
                positions.clear();
                hence = 0;
        }


    protected:

        size_t hence;
        SIMDVector4s positions;
        Cargo cargo;
};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
