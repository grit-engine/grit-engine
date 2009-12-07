#include <map>

#include <OgreSharedPtr.h>
#include <OgreDataStream.h>
#include <OgreVector3.h>

#include "TColParser.h"
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

        virtual float getMass (void) const { return mass; }
        virtual void setMass (float v) { mass = v; }

        virtual Ogre::Vector3 getInertia (void) const { return inertia; }
        virtual void setInertia (Ogre::Vector3 v) { inertia = v; }

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
