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

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include"navigation_system.h"
#include"navigation_manager.h"

class NavigationSystem
{
public:

    NavigationSystem(void);
    ~NavigationSystem(void);

    void init();
    void Update(const Ogre::Real tslf);
    void updateDebug(const Ogre::Real tslf);

    void reset(void);

    void set_dest_pos(Ogre::Vector3 pos);
    void buildNavMesh();

    void addObj(const char* mesh_name);
    void addGfxBody(GfxBodyPtr bd);
    void addGfxBodies(std::vector<GfxBodyPtr> bds);
    void addRigidBody(RigidBody *bd);

    void addTempObstacle(Ogre::Vector3 pos);
    void removeTempObstacle(Ogre::Vector3 pos);

    void addOffmeshConection(Ogre::Vector3 pos, Ogre::Vector3 pos2, bool bidir);
    void removeOffmeshConection(Ogre::Vector3 pos);

    void removeConvexVolume(Ogre::Vector3 pos);
    void createConvexVolume(Ogre::Vector3 pos);

    bool saveNavmesh(const char* path);
    bool loadNavmesh(const char* path);

    bool findNearestPointOnNavmesh(Ogre::Vector3 pos, dtPolyRef& m_targetRef, Ogre::Vector3 &resultPoint);
    bool getRandomNavMeshPoint(Ogre::Vector3 &resultPoint);
    bool getRandomNavMeshPointInCircle(Ogre::Vector3 point, float radius, Ogre::Vector3 &resultPoint);

    bool anyNavmeshLoaded(void) { return navMeshLoaded; };

    // Agent:
    int addAgent(Ogre::Vector3 pos);
    void removeAgent(int idx);
    bool isAgentActive(int idx);
    void agentStop(int idx);
    Ogre::Vector3 getAgentPosition(int idx);
    void setAgentPosition(int idx, Ogre::Vector3 pos);
    Ogre::Vector3 getAgentVelocity(int idx);
    void agentRequestVelocity(int idx, Ogre::Vector3 vel);
    void setAgentMoveTarget(int idx, Ogre::Vector3 pos, bool adjust);
    float getAgentHeight(int idx);
    float getAgentRadius(int idx);
    float getDistanceToGoal(int idx, const float maxRange);

    InputGeom* geom;

    NavigationManager* getNavigationManager(void) { return nvmgr; };
protected:
    NavigationManager* nvmgr;
    BuildContext ctx;

    bool navMeshLoaded = false;
};
#endif
