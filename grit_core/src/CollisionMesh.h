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

#include <OgreDataStream.h>

#include "TColParser.h"
#include "LooseEnd.h"
#include "CentralisedLog.h"
#include "sleep.h"
#include "SharedPtr.h"

class CollisionMesh;
typedef SharedPtr<CollisionMesh> CollisionMeshPtr;

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

        struct ProcObjFace {
                Vector3 A;
                Vector3 AB;
                Vector3 AC;
        };
        typedef std::vector<ProcObjFace> ProcObjFaces;
        typedef std::vector<float> ProcObjFaceAreas;

        struct ProcObjFaceDBEntry {
                ProcObjFaces faces;
                ProcObjFaceAreas areas;
                float totalArea;
                ProcObjFaceDBEntry (void) : totalArea(0) { }
        };
        typedef std::map<int,ProcObjFaceDBEntry> ProcObjFaceDB;

        void getProcObjMaterials (std::vector<int> &r) {
                typedef ProcObjFaceDB::iterator I;
                for (I i=procObjFaceDB.begin(),i_=procObjFaceDB.end() ; i!=i_ ; ++i)
                        r.push_back(i->first);
        }

        // Required in T:
        // member function void T::push_back(const WorldTransform &)
        // member function size_t T::size()
        template<class T>
        void scatter (int mat, const Transform &world_trans, float density,
                      float min_slope, float max_slope,
                      float min_elevation, float max_elevation,
                      bool no_z, bool rotate, bool align_slope,
                      unsigned seed,
                      T &r)
        {
                float left_overs = 0; // fractional samples carried over to the next triangle
                float min_slope_sin = gritsin(Degree(90-max_slope));
                float max_slope_sin = gritsin(Degree(90-min_slope));
                float range_slope_sin = (max_slope_sin-min_slope_sin);

                const ProcObjFaceDBEntry &ent = procObjFaceDB[mat];
                const ProcObjFaces &mat_faces = ent.faces;
                const ProcObjFaceAreas &mat_face_areas = ent.areas;
                float total_area = ent.totalArea;
                int max_samples = total_area * density;

                r.reserve(r.size() + max_samples);

                srand(seed);

                unsigned long long before = micros();
                for (unsigned i=0 ; i<mat_face_areas.size() ; ++i) {

                        float area = mat_face_areas[i];
                        float samples_f = area * density + left_overs;
                        int samples = samples_f;
                        left_overs = samples_f - samples;
                        if (samples==0) continue;

                        const ProcObjFace &f = mat_faces[i];

                        Vector3 A  = world_trans * f.A;
                        Vector3 AB = world_trans.r * f.AB;
                        Vector3 AC = world_trans.r * f.AC;

                        Vector3 n = AB.cross(AC).normalisedCopy();
                        if (n.z < min_slope_sin) continue;
                        if (n.z > max_slope_sin) continue;
                        if (no_z) {
                                samples_f *= 1 - (max_slope_sin-n.z)/range_slope_sin;
                                samples = samples_f;
                                if (samples == 0) continue;
                        }
                        //float density = float(rand())/RAND_MAX * density;

                        // base_q may be multiplied by a random rotation for each sample later
                        Quaternion base_q = align_slope ?
                                            Vector3(0,0,1).getRotationTo(n) : Quaternion(1,0,0,0);
                        
                        for (int i=0 ; i<samples ; ++i) {
                                float x = float(rand())/RAND_MAX;
                                float y = float(rand())/RAND_MAX;
                                if (x+y > 1) { x=1-x; y=1-y; }
                                
                                // scale up
                                Vector3 p = A + x*AB + y*AC;

                                if (p.z < min_elevation && p.z > max_elevation) continue;

                                // a whole bunch of sanity checks for debugging purposes
                                #if 0
                                Vector3 max(std::max(A.x,std::max(B.x,C.x)),
                                            std::max(A.y,std::max(B.y,C.y)),
                                            std::max(A.z,std::max(B.z,C.z)));
                                Vector3 min(std::min(A.x,std::min(B.x,C.x)),
                                            std::min(A.y,std::min(B.y,C.y)),
                                            std::min(A.z,std::min(B.z,C.z)));
                                if (p.x < min.x) abort();
                                if (p.y < min.y) abort();
                                if (p.z < min.z) abort();
                                if (p.x > max.x) abort();
                                if (p.y > max.y) abort();
                                if (p.z > max.z) abort();
                                APP_ASSERT(!isnan(p.x));
                                APP_ASSERT(!isnan(p.y));
                                APP_ASSERT(!isnan(p.z));
                                APP_ASSERT(!isnan(base_q.w));
                                APP_ASSERT(!isnan(base_q.x));
                                APP_ASSERT(!isnan(base_q.y));
                                APP_ASSERT(!isnan(base_q.z));
                                #endif

                                if (rotate) {
                                        Quaternion rnd(Radian(float(rand())/RAND_MAX * 2*M_PI),
                                                               Vector3(0,0,1));
                                        r.push_back(Transform(base_q * rnd, p));
                                } else {
                                        r.push_back(Transform(base_q, p));
                                }
                        }
                }
                CLOG << "scatter time: " << micros()-before << "us"
                     << "  max_samples: " << max_samples
                     << "  samples: " << r.size() << "  tris: " << mat_faces.size()
                     << "  area: " << total_area
                     << std::endl;;
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
