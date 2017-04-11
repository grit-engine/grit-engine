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

// Based on http://www.ogre3d.org/wiki/index.php/BulletDebugDrawer
// copyright ownership unknown

#ifndef BulletDebugDrawer_h
#define BulletDebugDrawer_h

#include <btBulletCollisionCommon.h>

#include <math_util.h>

class BulletDebugDrawer: public btIDebugDraw
{
    public:
    BulletDebugDrawer (void);
    ~BulletDebugDrawer (void);
    virtual void drawLine (const btVector3 &from, const btVector3 &to, const btVector3 &colour);
    virtual void drawTriangle (const btVector3 &v0, const btVector3 &v1, const btVector3 &v2,
                               const btVector3 &colour, btScalar alpha);
    virtual void drawContactPoint (const btVector3 &PointOnB, const btVector3 &normalOnB,
                                   btScalar distance, int lifeTime, const btVector3 &colour);
    virtual void reportErrorWarning (const char *warningString);
    virtual void draw3dText (const btVector3 &location, const char *textString);
    virtual void setDebugMode (int debugMode);
    virtual int getDebugMode () const;
    void frameCallback (void);

    private:
    struct ContactPoint{
        Vector3 from;
        Vector3 to;
        Vector3 colour;
        size_t dieTime;
    };
    DebugDrawModes mDebugModes;
    // Every frame we throw out some contact points that have been displayed for long enough.
    // To implement that efficiently, we have a double buffer.
    std::vector<ContactPoint> contactPoints1;
    std::vector<ContactPoint> contactPoints2;
    std::vector<ContactPoint> *contactPoints;
};

#endif
