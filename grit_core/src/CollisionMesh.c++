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

#include <iostream>
#include <cstdlib>
#include <ctime>

#include <OgreException.h>
#include <OgreTimer.h>
#include <OgreResourceGroupManager.h>

#include <LinearMath/btGeometryUtil.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <../Extras/GIMPACTUtils/btGImpactConvexDecompositionShape.h>

#include "CollisionMesh.h"
#include "TColParser.h"
#include "CentralisedLog.h"
#include "PhysicsWorld.h"

#include "path_util.h"

#ifndef M_PI
#define M_PI 3.1415926535897932385
#endif

btCompoundShape *import_compound (btCompoundShape *s, const Compound &c,
                                  LooseEnds &les,
                                  CollisionMesh::Materials &partMaterials)
{

        les.push_back(new LooseEndImpl<btCollisionShape>(s));

        const btVector3 ZV(0,0,0);
        const btQuaternion ZQ(0,0,0,1);

        for (size_t i=0 ; i<c.hulls.size() ; ++i) {
                const Hull &h = c.hulls[i];
                btConvexHullShape *s2 = new btConvexHullShape();
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s2->setMargin(h.margin);
                for (unsigned j=0 ; j<h.vertexes.size() ; ++j) {
                        const Vector3 &v = h.vertexes[j];
                        s2->addPoint(v.bullet());
                }
                s->addChildShape(btTransform(ZQ,ZV), s2);
                partMaterials.push_back(h.material);
        }

        for (size_t i=0 ; i<c.boxes.size() ; ++i) {
                const Box &b = c.boxes[i];
                btBoxShape *s2 =new btBoxShape(btVector3(b.dx/2,b.dy/2,b.dz/2));
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s2->setMargin(b.margin);
                s->addChildShape(btTransform(btQuaternion(b.qx,b.qy,b.qz,b.qw),
                                             btVector3(b.px,b.py,b.pz)), s2);
                partMaterials.push_back(b.material);
        }

        for (size_t i=0 ; i<c.cylinders.size() ; ++i) {
                const Cylinder &cyl = c.cylinders[i];
                btCylinderShape *s2 =
                    new btCylinderShapeZ(btVector3(cyl.dx/2,cyl.dy/2,cyl.dz/2));
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s2->setMargin(cyl.margin);
                s->addChildShape(
                        btTransform(btQuaternion(cyl.qx,cyl.qy,cyl.qz,cyl.qw),
                                    btVector3(cyl.px,cyl.py,cyl.pz)), s2);
                partMaterials.push_back(cyl.material);
        }

        for (size_t i=0 ; i<c.cones.size() ; ++i) {
                const Cone &cone = c.cones[i];
                btConeShapeZ *s2 = new btConeShapeZ(cone.radius,cone.height);
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s2->setMargin(cone.margin);
                s->addChildShape(
                      btTransform(btQuaternion(cone.qx,cone.qy,cone.qz,cone.qw),
                                  btVector3(cone.px,cone.py,cone.pz)), s2);
                partMaterials.push_back(cone.material);
        }

        for (size_t i=0 ; i<c.planes.size() ; ++i) {
                const Plane &p = c.planes[i];
                btStaticPlaneShape *s2 =
                        new btStaticPlaneShape(btVector3(p.nx,p.ny,p.nz),p.d);
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s->addChildShape(btTransform(ZQ,ZV), s2);
                partMaterials.push_back(p.material);
        }

        for (size_t i=0 ; i<c.spheres.size() ; ++i) {
                const Sphere &sp = c.spheres[i];
                btSphereShape *s2 = new btSphereShape(sp.radius);
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s->addChildShape(btTransform(ZQ,
                                             btVector3(sp.px,sp.py,sp.pz)), s2);
                partMaterials.push_back(sp.material);
        }

        return s;
}


btCollisionShape *import_trimesh (const TriMesh &f, bool is_static, LooseEnds &les,
                                  CollisionMesh::Materials &faceMaterials,
                                  CollisionMesh::ProcObjFaceDB &faceDB)
{
        Vertexes *vertexes = new Vertexes();
        les.push_back(new LooseEndImpl<Vertexes>(vertexes));
        int sz = f.vertexes.size();
        vertexes->reserve(sz);
        for (int i=0 ; i<sz ; ++i) {
                vertexes->push_back(f.vertexes[i]);
        }

        Faces *faces = new Faces(f.faces);
        les.push_back(new LooseEndImpl<Faces>(faces));

        faceMaterials.reserve(f.faces.size());
        for (Faces::const_iterator i=f.faces.begin(), i_=f.faces.end() ; i!=i_ ; ++i) {
                int m = i->material;
                faceMaterials.push_back(m);
                CollisionMesh::ProcObjFace f;
                f.A =  (*vertexes)[i->v1];
                f.AB = (*vertexes)[i->v2] - f.A;
                f.AC = (*vertexes)[i->v3] - f.A;
                faceDB[m].first.push_back((f.AB.cross(f.AC)).length());
                faceDB[m].second.push_back(f);
        }

        btTriangleIndexVertexArray *v = new btTriangleIndexVertexArray(
                faces->size(), &((*faces)[0].v1), sizeof(Face),
                vertexes->size(), &((*vertexes)[0].x), sizeof(Vector3));
        les.push_back(new LooseEndImpl<btTriangleIndexVertexArray>(v));

        btCollisionShape *s;

        if (is_static) {
                btBvhTriangleMeshShape *tm = new btBvhTriangleMeshShape(v,true,true);
                s = tm;
                s->setMargin(0);
                btTriangleInfoMap* tri_info_map = new btTriangleInfoMap();
                // maybe adjust thresholds in tri_info_map

                btGenerateInternalEdgeInfo(tm,tri_info_map);
                les.push_back(new LooseEndImpl<btCollisionShape>(s));
        } else {
                btGImpactShapeInterface *s2 = new btGImpactMeshShape(v);
                //assume mesh is already shrunk to the given margin
                s2->setMargin(f.margin);
                /* this is hopelessly awful in comparison (but faster)
                btGImpactShapeInterface *s2 =
                        new btGImpactConvexDecompositionShape(v,
                                                              Vector3(1,1,1),
                                                              0.01);
                */
                s2->updateBound();
                s = s2;
                les.push_back(new LooseEndImpl<btCollisionShape>(s));
        }

        return s;
}


btCompoundShape *import (const TColFile &f,
                         CollisionMesh *cm,
                         LooseEnds &les,
                         CollisionMesh::Materials &partMaterials,
                         CollisionMesh::Materials &faceMaterials,
                         CollisionMesh::ProcObjFaceDB &faceDB)
{

        bool stat = f.mass == 0.0f; // static
        btCompoundShape *s = new btCompoundShape();

        if (f.usingCompound) {
                import_compound(s, f.compound, les, partMaterials);
        }

        if (f.usingTriMesh) {
                btCollisionShape *s2 = import_trimesh (f.triMesh, stat, les, faceMaterials, faceDB);
                s->addChildShape(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)), s2);
        }

        cm->setMass(f.mass);
        cm->setInertia(Vector3(f.inertia_x,f.inertia_y,f.inertia_z));
        cm->setLinearDamping(f.linearDamping);
        cm->setAngularDamping(f.angularDamping);
        cm->setLinearSleepThreshold(f.linearSleepThreshold);
        cm->setAngularSleepThreshold(f.angularSleepThreshold);
        cm->setCCDMotionThreshold(f.ccdMotionThreshold);
        cm->setCCDSweptSphereRadius(f.ccdSweptSphereRadius);

        return s;
}



class ProxyStreamBuf : public std::streambuf
{
    public:
        ProxyStreamBuf (const Ogre::DataStreamPtr &file_)
              : file(file_) { }

    protected:

        std::streamsize showmanyc (void)
        {
                return 0;
        }

/*
        int_type uflow (void)
        {
                char buf;
                std::streamsize sz = file->read(&buf,1);
                if (sz==1) return traits_type::not_eof(buf);
                else return traits_type::eof();
        }
*/

        virtual std::streamsize _Xsgetn_s (char* s, size_t, std::streamsize n)
        {
            return xsgetn(s,n);
        }
        

/*
        std::streamsize xsgetn (char* s, std::streamsize n)
        {
                std::streamsize sz = std::streambuf::xsgetn(s,n);
                std::cout << "("<<(void*)this<<").xsgetn("<<(void*)s<<","<<n<<") = "<<sz<<std::endl;
                return sz;
        }
*/


        pos_type seekpos (pos_type sp, std::ios_base::openmode which)
        {
                if (which == std::ios_base::out) {
                        GRIT_EXCEPT("Cannot write to an Ogre::DataStream");
                }
                file->seek(sp);
                return file->tell();
        }

        pos_type seekoff (off_type off,
                          std::ios_base::seekdir way,
                          std::ios_base::openmode which)
        {
                if (which == std::ios_base::out) {
                        GRIT_EXCEPT("Cannot write to an Ogre::DataStream");
                }
                switch (way) {
                        case std::ios_base::beg: file->seek(off); break;
                        case std::ios_base::cur: file->skip(off); break;
                        case std::ios_base::end:
                        GRIT_EXCEPT("Cannot seek to end of an Ogre::DataStream");

                        default: // compiler is whining at me
                        return -1;
                }
                return file->tell();
        }



        std::streamsize xsgetn (char* s, std::streamsize n)
        {
                std::streamsize left = n;
                while (left > 0) {
                        std::streamsize sz = file->read(s,left);
                        if (sz==0) break;
                        left -= sz;
                        s += sz;
                }
                return n - left;
        }

        std::streamsize xsputn (const char_type*, std::streamsize)
        {
                GRIT_EXCEPT("Cannot write to an Ogre::DataStream");
        }

        Ogre::DataStreamPtr file;
};


void CollisionMesh::importFromFile (const Ogre::DataStreamPtr &file, const MaterialDB &db)
{
        ProxyStreamBuf proxy(file);
        {
                std::istream stream(&proxy);
                quex::TColLexer qlex(&stream);
                TColFile tcol;
                pwd_push_file("/"+file->getName());
                parse_tcol_1_0(file->getName(),&qlex,tcol,db);
                pwd_pop();

                Materials m1, m2;
                ProcObjFaceDB fdb;
                LooseEnds ls;
                btCompoundShape *loaded_shape;

                try {
                        loaded_shape = import(tcol,this,ls,m1,m2,fdb);
                } catch (GritException& e) {
                        for (LooseEnds::iterator i=ls.begin(), i_=ls.end() ; i!=i_ ; ++i) {
                                delete *i;
                        }
                        throw e;
                }

                for (LooseEnds::iterator i=looseEnds.begin(), i_=looseEnds.end() ; i!=i_ ; ++i) {
                        delete *i;
                }
                masterShape = loaded_shape;

                if (mass != 0 && inertia == Vector3(0,0,0)) {
                        btVector3 i;
                        masterShape->calculateLocalInertia(mass,i);
                        inertia = i;
                }

                looseEnds = ls;
                partMaterials = m1;
                faceMaterials = m2;
                procObjFaceDB = fdb;
        }
}

void CollisionMesh::reload (const MaterialDB &db)
{
        importFromFile(Ogre::ResourceGroupManager::getSingleton().openResource(name,"GRIT"), db);
        for (Users::iterator i=users.begin(),i_=users.end() ; i!=i_ ; ++i) {
                (*i)->notifyMeshReloaded();
        }
}

int CollisionMesh::getMaterialFromPart (unsigned int id)
{
        if (id >= partMaterials.size()) return 0;
        return partMaterials[id];
}

int CollisionMesh::getMaterialFromFace (unsigned int id)
{
        if (id >= faceMaterials.size()) return 0;
        return faceMaterials[id];
}

void CollisionMesh::scatter (int mat, const ScatterOptions &opts,
                             std::vector<Transform> (&r))
{
        float left_overs = 0; // fractional samples carried over to the next triangle
        float min_slope_sin = gritsin(Degree(90-opts.maxSlope));
        float max_slope_sin = gritsin(Degree(90-opts.minSlope));
        float half_slope_sin = (max_slope_sin-min_slope_sin)/2;
        float mid_slope_sin = min_slope_sin + half_slope_sin;
        float total_area = 0;

        std::pair<ProcObjFaceAreas, ProcObjFaces> pair = procObjFaceDB[mat];
        ProcObjFaceAreas &mat_face_areas = pair.first;
        ProcObjFaces &mat_faces = pair.second;

        srand(opts.seed ? 0 : time(NULL));

        Ogre::Timer t;
        for (unsigned i=0 ; i<mat_face_areas.size() ; ++i) {

                float area = mat_face_areas[i];
                total_area += area;
                float samples_f = area * opts.density + left_overs;
                int samples = samples_f;
                left_overs = samples_f - samples;
                if (samples==0) continue;

                ProcObjFace &f = mat_faces[i];

                Vector3 A  = opts.worldTrans * f.A;
                Vector3 AB = opts.worldTrans.r * f.AB;
                Vector3 AC = opts.worldTrans.r * f.AC;

                Vector3 n = AB.cross(AC).normalisedCopy();
                if (n.z < min_slope_sin) continue;
                if (n.z > max_slope_sin) continue;
                if (opts.noZ) {
                        samples_f *= 1 - fabs(n.z-mid_slope_sin)/half_slope_sin;
                        samples = samples_f;
                        if (samples == 0) continue;
                }
                //float density = float(rand())/RAND_MAX * opts.density;

                // base_q may be multiplied by a random rotation for each sample later
                Quaternion base_q = opts.alignSlope ?
                                    Vector3(0,0,1).getRotationTo(n) : Quaternion(1,0,0,0);
                
                for (int i=0 ; i<samples ; ++i) {
                        float x = float(rand())/RAND_MAX;
                        float y = float(rand())/RAND_MAX;
                        if (x+y > 1) { x=1-x; y=1-y; }
                        
                        // scale up
                        Vector3 p = A + x*AB + y*AC;

                        if (p.z < opts.minElevation && p.z > opts.maxElevation) continue;

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

                        if (opts.rotate) {
                                Quaternion rnd(Radian(float(rand())/RAND_MAX * 2*M_PI),
                                                       Vector3(0,0,1));
                                r.push_back(Transform(base_q * rnd, p));
                        } else {
                                r.push_back(Transform(base_q, p));
                        }
                }
        }
        CLOG << "scatter time (s): " << t.getMicroseconds()/1E6
             << "  samples: " << r.size() << "  tris: " << mat_faces.size()
             << "  area: " << total_area
             << std::endl;;
}

// vim: ts=8:sw=8:et
