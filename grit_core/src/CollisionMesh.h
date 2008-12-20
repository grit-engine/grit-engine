#include <map>

#include <OgreSharedPtr.h>
#include <OgreDataStream.h>
#include <OgreVector3.h>

#include "LooseEnd.h"

class CollisionMesh;
typedef Ogre::SharedPtr<CollisionMesh> CollisionMeshPtr;

typedef std::map<Ogre::String,CollisionMeshPtr> CollisionMeshMap;

#ifndef CollisionMesh_h
#define CollisionMesh_h

#include <btBulletCollisionCommon.h>

class CollisionMesh {

        friend class PhysicsWorld;

    public:

        virtual ~CollisionMesh (void)
        {
                for (LooseEnds::iterator i=looseEnds.begin(),
                                         i_=looseEnds.end() ; i!=i_ ; ++i) {
                        delete *i;
                }
        }

        virtual btCollisionShape *getShape (void) const { return shape; }

        virtual void importFromFile (const Ogre::DataStreamPtr &file);

        virtual const Ogre::String &getName (void) const
        { return name; }

        virtual Ogre::Real getMass (void) const { return mass; }
        virtual void setMass (Ogre::Real v) { mass = v; }

        virtual Ogre::Vector3 getInertia (void) const { return inertia; }
        virtual void setInertia (Ogre::Vector3 v) { inertia = v; }

        virtual Ogre::Real getCCDMotionThreshold (void) const
        { return ccdMotionThreshold; }
        virtual void setCCDMotionThreshold (Ogre::Real v)
        { ccdMotionThreshold = v; }

        virtual Ogre::Real getCCDSweptSphereRadius (void) const
        { return ccdSweptSphereRadius; }
        virtual void setCCDSweptSphereRadius (Ogre::Real v)
        { ccdSweptSphereRadius = v; }

        virtual Ogre::Real getLinearDamping (void) const
        { return linearDamping; }
        virtual void setLinearDamping (Ogre::Real r)
        { linearDamping = r; }
        virtual Ogre::Real getAngularDamping (void) const
        { return angularDamping; }
        virtual void setAngularDamping (Ogre::Real r)
        { angularDamping = r; }

        virtual Ogre::Real getLinearSleepThreshold (void) const
        { return linearSleepThreshold; }
        virtual void setLinearSleepThreshold (Ogre::Real r)
        { linearSleepThreshold = r; }
        virtual Ogre::Real getAngularSleepThreshold (void) const
        { return angularSleepThreshold; }
        virtual void setAngularSleepThreshold (Ogre::Real r)
        { angularSleepThreshold = r; }

        virtual Ogre::Real getFriction (void) { return friction; }
        virtual void setFriction (Ogre::Real r) { friction = r; }

        virtual Ogre::Real getRestitution (void) { return restitution; }
        virtual void setRestitution (Ogre::Real r) { restitution = r; }

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
        btCollisionShape *shape;

        LooseEnds looseEnds;

        Ogre::Vector3 inertia;
        Ogre::Real mass;
        Ogre::Real ccdMotionThreshold;
        Ogre::Real ccdSweptSphereRadius;
        Ogre::Real linearDamping;
        Ogre::Real angularDamping;
        Ogre::Real linearSleepThreshold;
        Ogre::Real angularSleepThreshold;
        Ogre::Real friction;
        Ogre::Real restitution;

};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
