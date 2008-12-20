template <typename T> class CacheFriendlyRangeSpace;

#ifndef CACHEFRIENDLYRANGESPACESIMD_H
#define CACHEFRIENDLYRANGESPACESIMD_H

#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>


#include "SSEAllocator.h"
#include "app_error.h"

template <typename T>
class CacheFriendlyRangeSpace {

    protected:



        class Position {

            protected:

                typedef float v4sf __attribute__ ((vector_size (16)));

                struct PositionRaw {
                        inline void updateSphere (float x_, float y_, float z_,
                                             float d_)
                        {
                                x=x_ ; y=y_ ; z=z_ ; d=d_;
                        }
                        float x, y, z, d;
                };

                union PositionAbstraction {
                        PositionRaw raw;
                        v4sf vector;
                };


            public:

                inline void updateSphere (float x, float y, float z, float d)
                {
                        pos.raw.updateSphere(x,y,z,d);
                }

                // only meaningful if other.d == 0 || d = 0
                inline bool near (const Position &other, const float factor2)
                {
                        PositionAbstraction diff;
                        diff.vector = pos.vector - other.pos.vector;
                        diff.vector *= diff.vector;
                        return diff.raw.x + diff.raw.y + diff.raw.z
                                < diff.raw.d * factor2;
                }


            protected:

                PositionAbstraction pos;
        };

        typedef std::vector<Position,SSEAllocator<Position> > Positions;


    public:

        typedef std::vector<T> Cargo;

        CacheFriendlyRangeSpace() : hence(0) { }

        virtual ~CacheFriendlyRangeSpace() { }

        virtual void add (const T &o)
        {
                typename Cargo::iterator begin = cargo.begin(),
                                             end = cargo.end();
                // if it's already in there, this is a no-op
                if (find(begin,end,o)!=end) return;
                
                size_t index = cargo.size();
                cargo.push_back(o);
                positions.push_back(Position());
                o->updateIndex(index);
        }

        virtual inline void updateSphere (size_t index, Ogre::Real x,
                                            Ogre::Real y, Ogre::Real z,
                                            Ogre::Real d)
        {
                positions[index].updateSphere(x,y,z,d);
        }


        virtual void getPresent (const Ogre::Real x,
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
                typename Positions::iterator begin = positions.begin(),
                                             iter = begin + hence,
                                             end = positions.end();

                if (num>positions.size()) {
                        iter = begin;
                        num = positions.size();
                }

                Position home;
                float factor2 = factor*factor;
                home.updateSphere(x,y,z,0);
                for (size_t i=0 ; i<num ; ++i) {
                        Position &pos = *iter;
                        if (home.near(pos,factor2)) {
                                T &o = *(cargo.begin() + (iter-begin));
                                found.push_back(o);
                        }
                        iter++;
                        if (iter==end) iter=begin;
                }

                hence = iter - positions.begin();
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
        Positions positions;
        Cargo cargo;
};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
