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

typedef std::map<std::string,CollisionMeshPtr> CollisionMeshMap;

#ifndef CollisionMesh_h
#define CollisionMesh_h

#include <btBulletCollisionCommon.h>

class RigidBody;

class CollisionMesh {

        friend class PhysicsWorld;

    public:

        ~CollisionMesh (void)
        {
                for (LooseEnds::iterator i=looseEnds.begin(),
                                         i_=looseEnds.end() ; i!=i_ ; ++i) {
                        delete *i;
                }
                APP_ASSERT(users.size()==0);
        }

        btCompoundShape *getMasterShape (void) const { return masterShape; }

        void importFromFile (const Ogre::DataStreamPtr &file, const MaterialDB &db);

        const std::string &getName (void) const
        { return name; }

        float getMass (void) const { return mass; }
        void setMass (float v) { mass = v; }

        Vector3 getInertia (void) const { return inertia; }
        void setInertia (const Vector3 &v) { inertia = v; }

        float getCCDMotionThreshold (void) const
        { return ccdMotionThreshold; }
        void setCCDMotionThreshold (float v)
        { ccdMotionThreshold = v; }

        float getCCDSweptSphereRadius (void) const
        { return ccdSweptSphereRadius; }
        void setCCDSweptSphereRadius (float v)
        { ccdSweptSphereRadius = v; }

        float getLinearDamping (void) const
        { return linearDamping; }
        void setLinearDamping (float r)
        { linearDamping = r; }
        float getAngularDamping (void) const
        { return angularDamping; }
        void setAngularDamping (float r)
        { angularDamping = r; }

        float getLinearSleepThreshold (void) const
        { return linearSleepThreshold; }
        void setLinearSleepThreshold (float r)
        { linearSleepThreshold = r; }
        float getAngularSleepThreshold (void) const
        { return angularSleepThreshold; }
        void setAngularSleepThreshold (float r)
        { angularSleepThreshold = r; }

        int getMaterialFromPart (unsigned int id);
        int getMaterialFromFace (unsigned int id);

        typedef std::vector<int> Materials;

        void reload (const MaterialDB &db);

        void registerUser (RigidBody *u)
        {
                users.insert(u);
        }

        void unregisterUser (RigidBody *u)
        {
                users.erase(u);
        }

        struct ScatterOptions {
                Transform worldTrans;
                float density;
                float minElevation;
                float maxElevation;
                float minSlope;
                float maxSlope;
                bool noZ;
                bool rotate;
                bool alignSlope;
                bool seed;
                ScatterOptions () 
                {
                        minElevation = FLT_MIN;
                        maxElevation = FLT_MAX;
                        maxSlope = 90;
                        minSlope = 0;
                        density = 1;
                        noZ = false;
                        rotate = true;
                        alignSlope = true;
                        seed = false;
                }
        };
        void scatter (int mat,const ScatterOptions &opts,std::vector<Transform> &r);

        struct ProcObjFace {
                Vector3 A;
                Vector3 AB;
                Vector3 AC;
        };
        typedef std::vector<ProcObjFace> ProcObjFaces;
        typedef std::vector<float> ProcObjFaceAreas;

        typedef std::map<int,std::pair<ProcObjFaceAreas, ProcObjFaces> > ProcObjFaceDB;

        void getProcObjMaterials (std::vector<int> &r) {
                typedef ProcObjFaceDB::iterator I;
                for (I i=procObjFaceDB.begin(),i_=procObjFaceDB.end() ; i!=i_ ; ++i)
                        r.push_back(i->first);
        }

    protected:

        CollisionMesh (const std::string &name_)
              : name(name_),
                linearDamping(0.0f),
                angularDamping(0.5f),
                linearSleepThreshold(1.0f),
                angularSleepThreshold(0.8f),
                friction(0.5f),
                restitution(0.0f)
        { }

        const std::string name;
        btCompoundShape *masterShape;

        typedef std::set<RigidBody*> Users;
        Users users;

        LooseEnds looseEnds;

        Vector3 inertia;
        float mass;
        float ccdMotionThreshold;
        float ccdSweptSphereRadius;
        float linearDamping;
        float angularDamping;
        float linearSleepThreshold;
        float angularSleepThreshold;
        float friction;
        float restitution;

        ProcObjFaceDB procObjFaceDB;

    public: // make these protected again when bullet works
        Materials faceMaterials;
        Materials partMaterials;
};


#endif

// vim: shiftwidth=8:tabstop=8:expandtab
