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
                for (int j=0 ; j<h.vertexes.size() ; ++j) {
                        const btVector3 &v = h.vertexes[j];
                        s2->addPoint(v);
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
                                  CollisionMesh::Materials &faceMaterials)
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
        }

        btTriangleIndexVertexArray *v = new btTriangleIndexVertexArray(
                faces->size(), &((*faces)[0].v1), sizeof(Face),
                vertexes->size(), &((*vertexes)[0][0]), sizeof(btVector3));
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
                                                              btVector3(1,1,1),
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
                         CollisionMesh::Materials &faceMaterials)
{

        bool stat = f.mass == 0.0f; // static
        btCompoundShape *s = new btCompoundShape();

        if (f.usingCompound) {
                import_compound(s, f.compound, les, partMaterials);
        }

        if (f.usingTriMesh) {
                btCollisionShape *s2 = import_trimesh (f.triMesh, stat, les, faceMaterials);
                s->addChildShape(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)), s2);
        }

        cm->setMass(f.mass);
        cm->setInertia(btVector3(f.inertia_x,f.inertia_y,f.inertia_z));
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
                        OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                                    "Cannot write to an Ogre::DataStream",
                                    "ProxyStreamBuf::seekpos");
                }
                file->seek(sp);
                return file->tell();
        }

        pos_type seekoff (off_type off,
                          std::ios_base::seekdir way,
                          std::ios_base::openmode which)
        {
                if (which == std::ios_base::out) {
                        OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                                    "Cannot write to an Ogre::DataStream",
                                    "ProxyStreamBuf::seekoff");
                }
                switch (way) {
                        case std::ios_base::beg: file->seek(off); break;
                        case std::ios_base::cur: file->skip(off); break;
                        case std::ios_base::end:
                        OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                                    "Cannot seek to end of an Ogre::DataStream",
                                    "ProxyStreamBuf::seekoff");

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
                OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "Cannot write to an Ogre::DataStream",
                            "ProxyStreamBuf::xsputn");
        }

        Ogre::DataStreamPtr file;
};


void CollisionMesh::importFromFile (const Ogre::DataStreamPtr &file, const PhysicsWorld &world)
{
        ProxyStreamBuf proxy(file);
        {
                std::istream stream(&proxy);
                quex::TColLexer qlex(&stream);
                TColFile tcol;
                pwd_push_file("/"+file->getName());
                parse_tcol_1_0(file->getName(),&qlex,tcol,world);
                pwd_pop();

                Materials m1, m2;
                LooseEnds ls;
                btCompoundShape *loaded_shape;

                try {
                        loaded_shape = import(tcol,this,ls,m1,m2);
                } catch( Ogre::Exception& e ) {
                        for (LooseEnds::iterator i=ls.begin(), i_=ls.end() ; i!=i_ ; ++i) {
                                delete *i;
                        }
                        throw e;
                }

                for (LooseEnds::iterator i=looseEnds.begin(),
                                         i_=looseEnds.end() ; i!=i_ ; ++i) {
                        delete *i;
                }
                masterShape = loaded_shape;

                if (mass != 0 && inertia == btVector3(0,0,0))
                        masterShape->calculateLocalInertia(mass,inertia);

                looseEnds = ls;
                partMaterials = m1;
                faceMaterials = m2;
        }
}

void CollisionMesh::reload (const PhysicsWorld &world)
{
        importFromFile(Ogre::ResourceGroupManager::getSingleton().openResource(name,"GRIT"), world);
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

void CollisionMesh::scatter (const ScatterOptions &opts, std::vector<btTransform> (&r)[3])
{
        struct ScatterTriangleCallback : public btTriangleCallback {
                const ScatterOptions &opts;
                CollisionMesh *cm;
                std::vector<btTransform> (&r)[3];
                int tris;
                float leftOvers[3];
                ScatterTriangleCallback (const ScatterOptions &opts_, CollisionMesh *cm_,
                                         std::vector<btTransform> (&r_)[3])
                      : opts(opts_), cm(cm_), r(r_), tris(0)
                {
                        for (int i=0 ; i<3 ; ++i) leftOvers[i] = 0;
                }
                void processTriangle (btVector3 *triangle, int partId, int triangleIndex) {
                        tris++;
                        assert(partId == 0); (void) partId;
                        btVector3 t1 = opts.worldTrans*triangle[0];
                        btVector3 t2 = opts.worldTrans*triangle[1];
                        btVector3 t3 = opts.worldTrans*triangle[2];
                        btVector3 *v1_p;
                        btVector3 *v2_p;
                        btVector3 *v3_p;
                        {
                                // find longest length edge
                                float l1 = (t2-t3).length2();
                                float l2 = (t1-t3).length2();
                                float l3 = (t2-t1).length2();
                                // note that order is important in these
                                // reassignments because we want to preserve
                                // the same winding, otherwise the normal is inverted
                                if (l1>=l2 && l1>=l3) { // base is opposite t1
                                        v1_p = &t2;
                                        v2_p = &t3;
                                        v3_p = &t1;
                                } else if (l2>=l1 && l2>=l3) { // base is opposite t2
                                        v1_p = &t3;
                                        v2_p = &t1;
                                        v3_p = &t2;
                                } else if (l3>=l2 && l3>=l1) { // base is opposite t3
                                        v1_p = &t1;
                                        v2_p = &t2;
                                        v3_p = &t3;
                                } else {
                                        CERR << "No triangle side is the longest." << std::endl;;
                                        v1_p = &t1;
                                        v2_p = &t2;
                                        v3_p = &t3;
                                }
                        }
                        btVector3 &v1 = *v1_p;
                        btVector3 &v2 = *v2_p;
                        btVector3 &v3 = *v3_p;
                                
                        // now we have the triangle as follows, where v4 is guaranteed to lie between v1 and v2
                        /*
                                   v3     ^
                             A    /|\  C  |
                                /  | \    h
                              /  B |D \   |
                            v1----v4---v2 |
                             --b1--
                             ----base-->
                        */
                        btVector3 base = v2-v1;
                        btVector3 n = base.cross(v3-v1);
                        float n_l = n.length();
                        float birds_eye_area;
                        {
                                btVector3 v1_=v1, v2_=v2, v3_=v3;
                                v1_.setZ(0);
                                v2_.setZ(0);
                                v3_.setZ(0);
                                birds_eye_area = (v2_-v1_).cross(v3_-v1_).length()/2;
                        }
                        float true_area = n_l/2;
                        n /= n_l; // normalise
                        btQuaternion base_q;
                        if (n.dot(btVector3(0,0,1)) > 0.98) {
                                 base_q = btQuaternion(0,0,0,1);
                        } else {
                                 base_q = btQuaternion(btVector3(0,0,1).cross(n), btVector3(0,0,1).angle(n));
                        }
                        
                        
                        float u = base.dot(v3-v1) / base.length2();
                        //APP_ASSERT(u>=0 && u <=1);
                        //APP_ASSERT(!isnan(u));
                        btVector3 v4 = v1 + u * base;

                        btVector3 h = v3-v4;

                        // handle the objects independently
                        for (int t=0 ; t<3 ; ++t) {
                                if (opts.noCeiling[t] && n.z() < 0) continue;
                                if (opts.noFloor[t] && n.z() > 0) continue;
                                r[t].reserve(3000);
                                float density = float(rand())/RAND_MAX * opts.density[t];
                                //density *= cm->faceProcObjDensities[3*triangleIndex + t];
                                float samples_f = (opts.noZ[t] ? birds_eye_area : true_area) * density + leftOvers[t];
                                int samples = samples_f;
                                leftOvers[t] = samples_f - samples;
                                for (int i=0 ; i<samples ; ++i) {
                                        // random along base
                                        float x = float(rand())/RAND_MAX;

                                        // random along height
                                        float y = float(rand())/RAND_MAX;
                                        
                                        // mirror image stuff
                                        if (x<u) {
                                                if (y > x/u) {
                                                        // A
                                                        x = u - x;
                                                        y = 1 - y;
                                                } else {
                                                        // B
                                                }
                                        } else {
                                                float u1 = 1-u;
                                                float x1 = 1-x;
                                                if (y > x1/u1) {
                                                        // C
                                                        x = 1 - (u1 - x1);
                                                        y = 1 - y;
                                                } else {
                                                        // D
                                                }
                                        }
                                        
                                        // scale up
                                        btVector3 p = v1 + x*base + y*h;

/*
                                        // a whole bunch of sanity checks for debugging purposes
                                        btVector3 max(std::max(v1.x(),std::max(v2.x(),v3.x())),std::max(v1.y(),std::max(v2.y(),v3.y())),std::max(v1.z(),std::max(v2.z(),v3.z())));
                                        btVector3 min(std::min(v1.x(),std::min(v2.x(),v3.x())),std::min(v1.y(),std::min(v2.y(),v3.y())),std::min(v1.z(),std::min(v2.z(),v3.z())));
                                        if (p.x() < min.x()) abort();
                                        if (p.y() < min.y()) abort();
                                        if (p.z() < min.z()) abort();
                                        if (p.x() > max.x()) abort();
                                        if (p.y() > max.y()) abort();
                                        if (p.z() > max.z()) abort();
                                        APP_ASSERT(!isnan(p.x()));
                                        APP_ASSERT(!isnan(p.y()));
                                        APP_ASSERT(!isnan(p.z()));
                                        APP_ASSERT(!isnan(base_q.w()));
                                        APP_ASSERT(!isnan(base_q.x()));
                                        APP_ASSERT(!isnan(base_q.y()));
                                        APP_ASSERT(!isnan(base_q.z()));
*/

                                        if (p.z() >= opts.minElevation[t] && p.z() <= opts.maxElevation[t]) {
                                                btTransform trans(opts.rotate ? base_q * btQuaternion(btVector3(0,0,1), float(rand())/RAND_MAX * 2*M_PI) : base_q, p);
                                                r[t].push_back(trans);
                                        }
                                }
                        }
                }
        };

        ScatterTriangleCallback stc(opts, this, r);

        for (int i=0 ; i<masterShape->getNumChildShapes() ; ++i) {
                btCollisionShape *child = masterShape->getChildShape(i);
                if (child->getShapeType()!=TRIANGLE_MESH_SHAPE_PROXYTYPE) return;
                btTriangleMeshShape *mesh = static_cast<btTriangleMeshShape*>(child);
                // HACK: use a massive aabb to capture the whole mesh
                Ogre::Timer t;
                mesh->processAllTriangles(&stc, btVector3(-10000,-10000,-10000), btVector3(10000,10000,10000));
                CLOG << "scatter time (s): " << t.getMicroseconds()/1E6 << "  samples: " << r[0].size() << "  tris: " << stc.tris << std::endl;;

        }
}

