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

#include <map>

#include <OgreSharedPtr.h>
#include <OgreDataStream.h>
#include <OgreVector3.h>

#include "TColParser.h"
#include "LooseEnd.h"
#include "CentralisedLog.h"

class CollisionMesh;
typedef Ogre::SharedPtr<CollisionMesh> CollisionMeshPtr;

typedef std::map<Ogre::String,CollisionMeshPtr> CollisionMeshMap;

#ifndef CollisionMesh_h
#define CollisionMesh_h

#include <btBulletCollisionCommon.h>

class RigidBody;

class CollisionMesh {

        friend class PhysicsWorld;

    public:

        virtual ~CollisionMesh (void)
        {
                for (LooseEnds::iterator i=looseEnds.begin(),
                                         i_=looseEnds.end() ; i!=i_ ; ++i) {
                        delete *i;
                }
                APP_ASSERT(users.size()==0);
        }

        virtual btCompoundShape *getMasterShape (void) const { return masterShape; }

        virtual void importFromFile (const Ogre::DataStreamPtr &file);

        virtual const Ogre::String &getName (void) const
        { return name; }

        virtual float getMass (void) const { return mass; }
        virtual void setMass (float v) { mass = v; }

        virtual btVector3 getInertia (void) const { return inertia; }
        virtual void setInertia (const btVector3 &v) { inertia = v; }

        virtual float getCCDMotionThreshold (void) const
        { return ccdMotionThreshold; }
        virtual void setCCDMotionThreshold (float v)
        { ccdMotionThreshold = v; }

        virtual float getCCDSweptSphereRadius (void) const
        { return ccdSweptSphereRadius; }
        virtual void setCCDSweptSphereRadius (float v)
        { ccdSweptSphereRadius = v; }

        virtual float getLinearDamping (void) const
        { return linearDamping; }
        virtual void setLinearDamping (float r)
        { linearDamping = r; }
        virtual float getAngularDamping (void) const
        { return angularDamping; }
        virtual void setAngularDamping (float r)
        { angularDamping = r; }

        virtual float getLinearSleepThreshold (void) const
        { return linearSleepThreshold; }
        virtual void setLinearSleepThreshold (float r)
        { linearSleepThreshold = r; }
        virtual float getAngularSleepThreshold (void) const
        { return angularSleepThreshold; }
        virtual void setAngularSleepThreshold (float r)
        { angularSleepThreshold = r; }

        virtual unsigned int getMaterialFromPart (unsigned int id);
        virtual unsigned int getMaterialFromFace (unsigned int id);

/*
        virtual float getFriction (void) { return friction; }
        virtual void setFriction (float r) { friction = r; }

        virtual float getRestitution (void) { return restitution; }
        virtual void setRestitution (float r) { restitution = r; }
*/

        typedef std::vector<physics_mat> Materials;

        void reload (void);

        void registerUser (RigidBody *u)
        {
                users.insert(u);
        }

        void unregisterUser (RigidBody *u)
        {
                users.erase(u);
        }

    protected:

        CollisionMesh (const Ogre::String &name_)
              : name(name_),
                linearDamping(0),
                angularDamping(0.5),
                linearSleepThreshold(1),
                angularSleepThreshold(0.8),
                friction(0.5),
                restitution(0)
        { }

        const Ogre::String name;
        btCompoundShape *masterShape;

        typedef std::set<RigidBody*> Users;
        Users users;

        LooseEnds looseEnds;

        btVector3 inertia;
        float mass;
        float ccdMotionThreshold;
        float ccdSweptSphereRadius;
        float linearDamping;
        float angularDamping;
        float linearSleepThreshold;
        float angularSleepThreshold;
        float friction;
        float restitution;

    public: // make these protected again when bullet works
        Materials faceMaterials;
        Materials partMaterials;
};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
