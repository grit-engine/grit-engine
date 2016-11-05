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

#include <centralised_log.h>

#include "bullet_debug_drawer.h"
#include "gfx/gfx.h"
#include "gfx/gfx_debug.h"
#include "gfx/gfx_internal.h"
#include "physics/physics_world.h"

static inline Vector3 from_bullet (const btVector3 &from)
{ return Vector3(from.x(), from.y(), from.z()); }

static inline Quaternion from_bullet (const btQuaternion &from)
{ return Quaternion(from.w(), from.x(), from.y(), from.z()); }

BulletDebugDrawer::BulletDebugDrawer (void)
{
    contactPoints = &contactPoints1;
    mDebugModes = (DebugDrawModes) DBG_NoDebug;
}

BulletDebugDrawer::~BulletDebugDrawer (void)
{
}

void BulletDebugDrawer::drawLine (const btVector3 &from, const btVector3 &to,
                                  const btVector3 &colour)
{
    gfx_debug_line(from_bullet(from), from_bullet(to), from_bullet(colour), 1);
}

void BulletDebugDrawer::drawTriangle (const btVector3 &v0, const btVector3 &v1, const btVector3 &v2,
                                      const btVector3 &colour, btScalar alpha)
{
    gfx_debug_triangle(from_bullet(v0), from_bullet(v1), from_bullet(v2),
                       from_bullet(colour), alpha);
}

void BulletDebugDrawer::drawContactPoint (const btVector3 &point, const btVector3 &normal,
                                          btScalar distance, int life_time, const btVector3 &colour)
{
    contactPoints->push_back(ContactPoint{
        from_bullet(point),
        from_bullet(point) + from_bullet(normal) * distance,
        from_bullet(colour),
        micros() + life_time,
    });
}

void BulletDebugDrawer::frameCallback (void)
{
    size_t now = micros();
    std::vector<ContactPoint> *new_cps =
        contactPoints == &contactPoints1 ? &contactPoints2 : &contactPoints1;
    for (const auto &cp : *contactPoints) {
        gfx_debug_point(cp.from, 0.1, Vector3(1, 1, 1), 0.3);
        gfx_debug_line(cp.from, cp.to, cp.colour, 1.0);
        if (now <= cp.dieTime)
            new_cps->push_back(cp);
    }
    contactPoints->clear();
    contactPoints = new_cps;
}

void BulletDebugDrawer::reportErrorWarning (const char *warningString)
{
    CVERB << warningString << std::endl;
}

void BulletDebugDrawer::draw3dText (const btVector3 &location, const char *textString)
{
    (void) location; (void) textString;
}

void BulletDebugDrawer::setDebugMode (int debugMode)
{
    mDebugModes = (DebugDrawModes) debugMode;
}

int BulletDebugDrawer::getDebugMode (void) const
{
    return mDebugModes;
}

