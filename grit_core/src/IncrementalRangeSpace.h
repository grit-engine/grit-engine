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

template <typename T> class IncrementalRangeSpace;

#ifndef INCREMENTALRANGESPACE_H
#define INCREMENTALRANGESPACE_H

#include <map>
#include <vector>
#include <iostream>
#include <math.h>


#include "CentralisedLog.h"

template <typename T>
class IncrementalRangeSpace {

    public:

        typedef std::set<T*> CargoMap;
        typedef std::set<T*> CargoSet;

        IncrementalRangeSpace() : mLast(NULL) { }

        virtual ~IncrementalRangeSpace() { }

        virtual void add (T *o)
        {
                // if it's already in there, this is a no-op
                mEverything.insert(o);
        }

        virtual const CargoMap& getPresent (const Ogre::Vector3 &cam,
                                            const float factor,
                                            const unsigned int num,
                                            CargoSet &killed)
        {
                if (num==0) return mWorkingSet;
                typename CargoSet::iterator it = mEverything.find(mLast),
                                   end = mEverything.end();
                if (it == end) {
                        it = mEverything.begin(); 
                        if (it == end) {
                               return mWorkingSet; // empty list
                        }
                }

                // iterate from this point for a while
                T *o;
                for (unsigned int i=0 ; i<num ; ++i) {
                        ++it;
                        if (it==end) {
                                it = mEverything.begin();
                        }
                        o = *it;
                        if (process(o, cam, factor)) {
                                mWorkingSet.insert(o);
                        } else {
                                if (mWorkingSet.erase(o))
                                        killed.insert(o);
                        }
                        if (o==mLast) return mWorkingSet; // we already looped
                }

                mLast = o;

                return mWorkingSet;
        }

        // no-op if o was not in the rangespace somewhere
        virtual void remove (T *o)
        {
                // if it's not in there, these are no-ops
                mWorkingSet.erase(o);
                mEverything.erase(o);
                if (mLast == o) {
                        mLast = NULL;
                }
        }

        virtual void clear (void)
        {
                // if it's not in there, these are no-ops
                mWorkingSet.clear();
                mEverything.clear();
                mLast = NULL;
        }

        std::ostream& toStream (std::ostream& o)
        {
                return o;
        }

    protected:

        virtual bool process (T* o, const Ogre::Vector3 &cam,
                              const float factor)
        {
                if (!o->isInScene()) return false;
                if (!o->getVisible() ) return false;

                float rd = o->getRenderingDistance();

                if (rd==0) return true;

                // now the distance test
                Ogre::SceneNode *n = o->getParentSceneNode();
                if (n==NULL) return false;
                const Ogre::Vector3 &o_pos = n->_getDerivedPosition();

                float sqd = (o_pos - cam).squaredLength();
                if (sqd > Ogre::Math::Sqr(factor*rd))
                        return false;

/* this in general won't work because o may not be loaded yet
                float sqd = (o_pos - cam).squaredLength();
                if (sqd > Ogre::Math::Sqr(factor*(rd+o->getBoundingRadius())))
                        return false;
*/

                return true;
        }

        T *mLast;
        CargoSet mEverything;
        CargoMap mWorkingSet;
};

template <typename T>
std::ostream& operator<<(std::ostream& o, IncrementalRangeSpace<T>& rs)
{
        return rs.toStream(o);
}


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
