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

#ifndef navigation_system_h
#define navigation_system_h

#include"Ogre.h"
#include"../gfx/gfx_body.h"
#include"../physics/physics_world.h"
#include<vector>

extern Ogre::Vector3 swap_yz(Ogre::Vector3 from);
extern float* Vec3ToFloat(Ogre::Vector3 pos);
extern Ogre::Vector3 FloatToVec3(float* pos);

class NavigationSystem;

extern NavigationSystem* nvsys;

void navigation_init(void);
void navigation_update(const float tslf);
void navigation_update_debug(const float tslf);
void navigation_shutdown(void);

namespace NavSysDebug
{
    extern bool Enabled;
    extern bool ShowNavmesh;
    extern bool NavmeshUseTileColours;
    extern bool ShowBounds;
    extern bool ShowTilingGrid;
    extern bool ShowAgents;
    extern bool ShowAgentArrows;
    extern bool ShowObstacles;
    extern bool ShowOffmeshConnections;
    extern bool ShowConvexVolumes;

    void init(void);
    void destroy(void);
}

#endif // navigation_system_h
