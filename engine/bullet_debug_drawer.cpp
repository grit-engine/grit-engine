/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include "bullet_debug_drawer.h"
#include <centralised_log.h>

#include "gfx/gfx.h"
#include "gfx/gfx_internal.h"
#include "physics/physics_world.h"

static inline Vector3 from_bullet (const btVector3 &from)
{ return Vector3 (from.x(), from.y(), from.z()); }

static inline Quaternion from_bullet (const btQuaternion &from)
{ return Quaternion (from.w(), from.x(), from.y(), from.z()); }

static inline btVector3 to_bullet (const Vector3 &from)
{ return btVector3(from.x,from.y,from.z); }

static inline btQuaternion to_bullet (const Quaternion &from)
{ return btQuaternion(from.x, from.y, from.z, from.w); }

BulletDebugDrawer::BulletDebugDrawer (Ogre::SceneManager *sm)
{
    mContactPoints = &mContactPoints1;

    mLines = new Ogre::ManualObject("physics lines");
    APP_ASSERT(mLines);
    mLines->setRenderQueueGroup(75);
    mLines->setDynamic(true);
    mLines->setCastShadows(false);
    sm->getRootSceneNode()->attachObject(mLines);

    mTriangles = new Ogre::ManualObject("physics triangles");
    APP_ASSERT(mTriangles);
    mTriangles->setRenderQueueGroup(75);
    mTriangles->setDynamic(true);
    mTriangles->setCastShadows(false);
    sm->getRootSceneNode()->attachObject(mTriangles);

    // We don't call update on the scenemanager anymore, so have to provide this ourselves.
    sm->getRootSceneNode()->overrideCachedTransform(Ogre::Matrix4::IDENTITY);
    
    mat = Ogre::MaterialManager::getSingleton().create("/system/BulletDebugDrawer", RESGRP);
    mat->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
    mat->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
    mat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
    mat->getTechnique(0)->getPass(0)->createTextureUnitState();
    mat->getTechnique(0)->getPass(0)->setFog(true, Ogre::FOG_NONE);
    mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setColourOperationEx(Ogre::LBX_MODULATE, Ogre::LBS_DIFFUSE, Ogre::LBS_DIFFUSE);

    mLines->begin(mat->getName(), Ogre::RenderOperation::OT_LINE_LIST);
    mLines->position(Ogre::Vector3::ZERO);
    mLines->colour(Ogre::ColourValue::Blue);
    mLines->position(Ogre::Vector3::ZERO);
    mLines->colour(Ogre::ColourValue::Blue);


    mTriangles->begin(mat->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);
    mTriangles->position(Ogre::Vector3::ZERO);
    mTriangles->colour(Ogre::ColourValue::Blue);
    mTriangles->position(Ogre::Vector3::ZERO);
    mTriangles->colour(Ogre::ColourValue::Blue);
    mTriangles->position(Ogre::Vector3::ZERO);
    mTriangles->colour(Ogre::ColourValue::Blue);

    mDebugModes = (DebugDrawModes) DBG_NoDebug;

}

BulletDebugDrawer::~BulletDebugDrawer (void)
{
    delete mLines;
    delete mTriangles;
}

void BulletDebugDrawer::drawLine (const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
    Ogre::ColourValue c(color.getX(), color.getY(), color.getZ());  
    c.saturate();
    mLines->position(to_ogre(from_bullet(from)));
    mLines->colour(c);
    mLines->position(to_ogre(from_bullet(to)));
    mLines->colour(c);
}

void BulletDebugDrawer::drawTriangle (const btVector3 &v0, const btVector3 &v1, const btVector3 &v2,
                                    const btVector3 &color, btScalar alpha)
{
    Ogre::ColourValue c(color.getX(), color.getY(), color.getZ(), alpha);
    c.saturate();
    mTriangles->position(to_ogre(from_bullet(v0)));
    mTriangles->colour(c);
    mTriangles->position(to_ogre(from_bullet(v1)));
    mTriangles->colour(c);
    mTriangles->position(to_ogre(from_bullet(v2)));
    mTriangles->colour(c);
}

void BulletDebugDrawer::drawContactPoint (const btVector3 &PointOnB, const btVector3 &normalOnB,
                                        btScalar distance, int lifeTime, const btVector3 &color)
{
    mContactPoints->resize(mContactPoints->size() + 1);
    ContactPoint p = mContactPoints->back();
    p.from = to_ogre(from_bullet(PointOnB));
    p.to = p.from + to_ogre(from_bullet(normalOnB)) * distance;
    p.dieTime = micros() + lifeTime*1000;
    p.color.r = color.x();
    p.color.g = color.y();
    p.color.b = color.z();
}

bool BulletDebugDrawer::frameStarted (void)
{
    size_t now = micros();
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

bool BulletDebugDrawer::frameEnded (void)
{
    mLines->beginUpdate(0);
    mTriangles->beginUpdate(0);
    return true;
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

