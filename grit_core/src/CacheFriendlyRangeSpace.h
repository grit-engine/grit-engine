template <typename T> class CacheFriendlyRangeSpace;

#ifndef CACHEFRIENDLYRANGESPACE_H
#define CACHEFRIENDLYRANGESPACE_H

#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>


#include "CentralisedLog.h"

template <typename T>
class CacheFriendlyRangeSpace {

    protected:

        struct Position {
                Position()
                      : x(0), y(0), d(0)
                { }
                
                signed short x;
                signed short y;
                signed short d;
        };

        typedef std::vector<Position> Positions;
        typedef std::vector<T> Cargoes;


    public:

        CacheFriendlyRangeSpace() : hence(0) { }

        virtual ~CacheFriendlyRangeSpace() { }

        virtual void add (const T &o)
        {
                typename Cargoes::iterator begin = cargoes.begin(),
                                             end = cargoes.end();
                // if it's already in there, this is a no-op
                if (find(begin,end,o)!=end) return;
                
                size_t index = cargoes.size();
                cargoes.push_back(o);
                positions.push_back(Position());
                o->updateIndex(index);
        }

        virtual inline void updateSphere (size_t index, Ogre::Real x,
                                          Ogre::Real y, Ogre::Real z,
                                          Ogre::Real d)
        {
                positions[index].x = (short)x;
                positions[index].y = (short)y;
                (void) z;
                positions[index].d = (short)d;
        }

        // conservative approximation - x/y bounding box
        virtual inline bool broadPhase (const Position &pos,
                                        const short x2, const short y2)
        {

                if (std::abs(pos.x-x2)>pos.d) return false;
                if (std::abs(pos.y-y2)>pos.d) return false;
                return true;
        }


        virtual void getPresent (Ogre::Real x, Ogre::Real y, Ogre::Real z,
                                 size_t num, std::vector<T*> &found)
        {
                if (num==0) return;
                if (cargoes.size()==0) return;
                if (num>cargoes.size()) num=cargoes.size();
                if (hence>=cargoes.size()) hence = 0;

                // iterate from this point for a while
                typename Positions::iterator begin = positions.begin(),
                                             iter = begin + hence,
                                             end = positions.end();

                if (num>positions.size()) {
                        iter = begin;
                        num = positions.size();
                }

                short x2 = (short)x;
                short y2 = (short)y;
                for (size_t i=0 ; i<num ; ++i) {
                        Position &pos = *iter;
                        if (broadPhase(pos,x2,y2)) {
                                // do more thorough check
                                T &o = *(cargoes.begin() + (iter-begin));
                                float xd2 = (o->getX()-x) * (o->getX()-x);
                                float yd2 = (o->getY()-y) * (o->getY()-y);
                                float zd2 = (o->getZ()-z) * (o->getZ()-z);
                                if (xd2+yd2+zd2 < o->getR() * o->getR()) {
                                        found.push_back(&o);
                                }
                        }
                        iter++;
                        if (iter==end) iter=begin;
                }

                hence = iter - positions.begin();
        }

        virtual void remove (const T &o)
        {
                typename Cargoes::iterator begin = cargoes.begin(),
                                             end = cargoes.end();
                typename Cargoes::iterator iter = find(begin,end,o);

                // no-op if o was not in the rangespace somewhere
                if (iter==end) return;

                // otherwise, carefully remove it -
                size_t index = iter - begin;

                positions[index] = positions[positions.size()-1];
                positions.pop_back();
                cargoes[index] = cargoes[cargoes.size()-1];
                cargoes[index]->updateIndex(index);
                cargoes.pop_back();
                o->updateIndex(-1);
        }

        virtual void clear (void)
        {
                cargoes.clear();
                positions.clear();
                hence = 0;
        }


    protected:

        size_t hence;
        Positions positions;
        Cargoes cargoes;
};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
