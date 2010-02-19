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

// Based on http://www.ogre3d.org/wiki/index.php/BulletDebugDrawer
// copyright ownership unknown

#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include <OgreRoot.h>

#include "BulletDebugDrawer.h"
#include "CentralisedLog.h"
#include "PhysicsWorld.h"

BulletDebugDrawer::BulletDebugDrawer (Ogre::SceneManager *scm)
{
    mContactPoints = &mContactPoints1;
    mLines = new Ogre::ManualObject("physics lines");
    APP_ASSERT(mLines);
    mTriangles = new Ogre::ManualObject("physics triangles");
    APP_ASSERT(mTriangles);
    mLines->setDynamic(true);
    mLines->setCastShadows(false);
    mTriangles->setDynamic(true);
    mTriangles->setCastShadows(false);
    //mLines->estimateVertexCount(100000);
    //mLines->estimateIndexCount(0);

    scm->getRootSceneNode()->attachObject(mLines);
    scm->getRootSceneNode()->attachObject(mTriangles);
    
    const char *matName = "OgreBulletCollisionsDebugDefault";
    Ogre::MaterialPtr mtl = Ogre::MaterialManager::getSingleton().getDefaultSettings()->clone(matName);
    mtl->setReceiveShadows(false);
    mtl->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    mtl->setDepthBias(0.1, 0);
    Ogre::TextureUnitState *tu = mtl->getTechnique(0)->getPass(0)->createTextureUnitState();
    APP_ASSERT(tu);
    tu->setColourOperationEx(Ogre::LBX_SOURCE1, Ogre::LBS_DIFFUSE);
    mtl->getTechnique(0)->setLightingEnabled(false);
    //mtl->getTechnique(0)->setSelfIllumination( ColourValue::White ); 

    mLines->begin(matName, Ogre::RenderOperation::OT_LINE_LIST);
    mLines->position(Ogre::Vector3::ZERO);
    mLines->colour(Ogre::ColourValue::Blue);
    mLines->position(Ogre::Vector3::ZERO);
    mLines->colour(Ogre::ColourValue::Blue);


    mTriangles->begin(matName, Ogre::RenderOperation::OT_TRIANGLE_LIST);
    mTriangles->position(Ogre::Vector3::ZERO);
    mTriangles->colour(Ogre::ColourValue::Blue);
    mTriangles->position(Ogre::Vector3::ZERO);
    mTriangles->colour(Ogre::ColourValue::Blue);
    mTriangles->position(Ogre::Vector3::ZERO);
    mTriangles->colour(Ogre::ColourValue::Blue);

    mDebugModes = (DebugDrawModes) DBG_NoDebug;
    Ogre::Root::getSingleton().addFrameListener(this);
}

BulletDebugDrawer::~BulletDebugDrawer (void)
{
    Ogre::Root::getSingleton().removeFrameListener(this);
    delete mLines;
    delete mTriangles;
}

void BulletDebugDrawer::drawLine (const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
    Ogre::ColourValue c(color.getX(), color.getY(), color.getZ());  
    c.saturate();
    mLines->position(to_ogre(from));
    mLines->colour(c);
    mLines->position(to_ogre(to));
    mLines->colour(c);
}

void BulletDebugDrawer::drawTriangle (const btVector3 &v0, const btVector3 &v1, const btVector3 &v2,
                                    const btVector3 &color, btScalar alpha)
{
    Ogre::ColourValue c(color.getX(), color.getY(), color.getZ(), alpha);
    c.saturate();
    mTriangles->position(to_ogre(v0));
    mTriangles->colour(c);
    mTriangles->position(to_ogre(v1));
    mTriangles->colour(c);
    mTriangles->position(to_ogre(v2));
    mTriangles->colour(c);
}

void BulletDebugDrawer::drawContactPoint (const btVector3 &PointOnB, const btVector3 &normalOnB,
                                        btScalar distance, int lifeTime, const btVector3 &color)
{
    mContactPoints->resize(mContactPoints->size() + 1);
    ContactPoint p = mContactPoints->back();
    p.from = to_ogre(PointOnB);
    p.to = p.from + to_ogre(normalOnB) * distance;
    p.dieTime = Ogre::Root::getSingleton().getTimer()->getMilliseconds() + lifeTime;
    p.color.r = color.x();
    p.color.g = color.y();
    p.color.b = color.z();
}

bool BulletDebugDrawer::frameStarted (const Ogre::FrameEvent&)
{
    size_t now = Ogre::Root::getSingleton().getTimer()->getMilliseconds();
    std::vector<ContactPoint> *newCP = mContactPoints == &mContactPoints1 ? &mContactPoints2 : &mContactPoints1;
    for ( std::vector<ContactPoint>::iterator i = mContactPoints->begin(); i < mContactPoints->end(); i++ ){
        ContactPoint &cp = *i;
        mLines->position(cp.from);
        mLines->colour(cp.color);
        mLines->position(cp.to);
        if (now <= cp.dieTime)
            newCP->push_back(cp);
    }
    mContactPoints->clear();
    mContactPoints = newCP;

    mLines->end();
    mTriangles->end();

    return true;
}

bool BulletDebugDrawer::frameEnded (const Ogre::FrameEvent&)
{
    mLines->beginUpdate(0);
    mTriangles->beginUpdate(0);
    return true;
}

void BulletDebugDrawer::reportErrorWarning (const char *warningString)
{
    Ogre::LogManager::getSingleton().getDefaultLog()->logMessage(warningString);
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

