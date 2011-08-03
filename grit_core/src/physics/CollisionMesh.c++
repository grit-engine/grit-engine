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
//#include <../Extras/GIMPACTUtils/btGImpactConvexDecompositionShape.h>

#include "../CentralisedLog.h"
#include "../path_util.h"

#include "CollisionMesh.h"
#include "TColParser.h"
#include "BColParser.h"
#include "PhysicsWorld.h"


#ifndef M_PI
#define M_PI 3.1415926535897932385
#endif

static inline Vector3 from_bullet (const btVector3 &from)
{ return Vector3 (from.x(), from.y(), from.z()); }

static inline Quaternion from_bullet (const btQuaternion &from)
{ return Quaternion (from.w(), from.x(), from.y(), from.z()); }

static inline btVector3 to_bullet (const Vector3 &from)
{ return btVector3(from.x,from.y,from.z); }

static inline btQuaternion to_bullet (const Quaternion &from)
{ return btQuaternion(from.x, from.y, from.z, from.w); }


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

namespace {

    // Optimisation: for things in the bcol that have the same material,
    // they will almost certainly use the same char* value within the bcol
    // for the name of that material, so this map will avoid having to walk
    // the strings (which can be quite long) to look up actual PhysicalMaterial*
    class BColMaterialMap {
        std::string dir, name;
        typedef std::map<const char *, PhysicalMaterial*> Map;
        Map mmap;
        public:
        BColMaterialMap (const std::string &dir, const std::string &name)
            : dir(dir), name(name) { }
        PhysicalMaterial *operator() (const char *s)
        {
            Map::iterator i = mmap.find(s);
            if (i!=mmap.end()) return i->second;
            PhysicalMaterial *pm = phys_mats.getMaterial(dir, name, s);
            mmap[s] = pm;
            return pm;
        }
    };
}

static inline Vector3 to_v3(BColVert &v) { return Vector3(v.x, v.y, v.z); }

void CollisionMesh::load (void)
{
    Ogre::DataStreamPtr file =
        Ogre::ResourceGroupManager::getSingleton().openResource(name.substr(1),"GRIT");

    std::string ext = name.substr(name.length()-5);

    uint32_t fourcc = 0;
    for (int i=0 ; i<4 ; ++i) {
        unsigned char c;
        file->read(&c, 1);
        fourcc |= c << (i*8);
    }
    file->seek(0);

    std::string dir = grit_dirname(name);

    const btVector3 ZV(0,0,0);
    const btQuaternion ZQ(0,0,0,1);

    bool compute_inertia = false;
    bool is_static = false;

    if (fourcc==0x4c4f4342) { //BCOL

        Ogre::MemoryDataStreamPtr mem = 
            Ogre::DataStreamPtr(OGRE_NEW Ogre::MemoryDataStream(name,file));

        BColFile &bcol = *reinterpret_cast<BColFile*>(mem->getPtr());

        is_static = bcol.mass == 0.0f; // static

        masterShape = new btCompoundShape();

        BColMaterialMap mmap(dir,name);

        for (unsigned i=0 ; i<bcol.hullNum ; ++i) {
            BColHull &p = *bcol.hulls(i);
            btConvexHullShape *s2 = new btConvexHullShape();
            s2->setMargin(p.margin);
            for (unsigned j=0 ; j<p.vertNum ; ++j) {
                BColVert &v = *p.verts(j);
                s2->addPoint(btVector3(v.x, v.y, v.z));
            }
            masterShape->addChildShape(btTransform(ZQ,ZV), s2);
            partMaterials.push_back(mmap(p.mat.name()));
        }

        for (unsigned i=0 ; i<bcol.boxNum ; ++i) {
            BColBox &p = *bcol.boxes(i);
            btBoxShape *s2 = new btBoxShape(btVector3(p.dx/2,p.dy/2,p.dz/2));
            s2->setMargin(p.margin);
            masterShape->addChildShape(btTransform(btQuaternion(p.qx,p.qy,p.qz,p.qw),
                                                   btVector3(p.px,p.py,p.pz)), s2);
            partMaterials.push_back(mmap(p.mat.name()));
        }

        for (unsigned i=0 ; i<bcol.cylNum ; ++i) {
            BColCyl &p = *bcol.cyls(i);
            btCylinderShape *s2 = new btCylinderShapeZ(btVector3(p.dx/2,p.dy/2,p.dz/2));
            s2->setMargin(p.margin);
            masterShape->addChildShape(btTransform(btQuaternion(p.qx,p.qy,p.qz,p.qw),
                                                   btVector3(p.px,p.py,p.pz)), s2);
            partMaterials.push_back(mmap(p.mat.name()));
        }

        for (unsigned i=0 ; i<bcol.coneNum ; ++i) {
            BColCone &p = *bcol.cones(i);
            btConeShape *s2 = new btConeShapeZ(p.radius,p.height);
            s2->setMargin(p.margin);
            masterShape->addChildShape(btTransform(btQuaternion(p.qx,p.qy,p.qz,p.qw),
                                                   btVector3(p.px,p.py,p.pz)), s2);
            partMaterials.push_back(mmap(p.mat.name()));
        }

        for (unsigned i=0 ; i<bcol.planeNum ; ++i) {
            BColPlane &p = *bcol.planes(i);
            btStaticPlaneShape *s2 = new btStaticPlaneShape(btVector3(p.nx,p.ny,p.nz),p.d);
            masterShape->addChildShape(btTransform(ZQ,ZV), s2);
            partMaterials.push_back(mmap(p.mat.name()));
        }

        for (unsigned i=0 ; i<bcol.sphereNum ; ++i) {
            BColSphere &p = *bcol.spheres(i);
            btSphereShape *s2 = new btSphereShape(p.radius);
            masterShape->addChildShape(btTransform(ZQ, btVector3(p.px,p.py,p.pz)), s2);
            partMaterials.push_back(mmap(p.mat.name()));
        }


        if (bcol.triMeshFaceNum > 0) {

            bcolVerts.resize(bcol.triMeshVertNum);
            bcolFaces.resize(bcol.triMeshFaceNum);

            memcpy(&bcolVerts[0], bcol.triMeshVerts(0), bcol.triMeshVertNum * sizeof(BColVert));
            memcpy(&bcolFaces[0], bcol.triMeshFaces(0), bcol.triMeshFaceNum * sizeof(BColFace));

            faceMaterials.reserve(bcol.triMeshFaceNum);

            int counter = 0;
            float accum_area = 0;
            for (unsigned i=0 ; i<bcol.triMeshFaceNum ; ++i) {
                BColFace &face = *bcol.triMeshFaces(i);
                PhysicalMaterial *mat = mmap(face.mat.name());
                faceMaterials.push_back(mat);
                CollisionMesh::ProcObjFace po_face(to_v3(bcolVerts[face.v1]), 
                                                   to_v3(bcolVerts[face.v2]),
                                                   to_v3(bcolVerts[face.v3]));
                procObjFaceDB[mat->id].faces.push_back(po_face);
                float area = (po_face.AB.cross(po_face.AC)).length();
                APP_ASSERT(area>=0);
                procObjFaceDB[mat->id].areas.push_back(area);
                procObjFaceDB[mat->id].totalArea += area;
                if (++counter = 10) {
                    counter = 0;
                    accum_area = 0;
                    procObjFaceDB[mat->id].areas10.push_back(accum_area);
                }
                accum_area += area;
                    
            }

            btTriangleIndexVertexArray *v = new btTriangleIndexVertexArray(
                bcolFaces.size(), reinterpret_cast<int*>(&(bcolFaces[0].v1)), sizeof(BColFace),
                bcolVerts.size(), &(bcolVerts[0].x), sizeof(BColVert));

            btCollisionShape *s;

            if (is_static) {
                btBvhTriangleMeshShape *tm = new btBvhTriangleMeshShape(v,true,true);
                s = tm;
                s->setMargin(bcol.triMeshMargin);
                btTriangleInfoMap* tri_info_map = new btTriangleInfoMap();
                tri_info_map->m_edgeDistanceThreshold = bcol.triMeshEdgeDistanceThreshold;

                btGenerateInternalEdgeInfo(tm,tri_info_map);
            } else {
                btGImpactShapeInterface *s2 = new btGImpactMeshShape(v);
                //assume mesh is already shrunk to the given margin
                s2->setMargin(bcol.triMeshMargin);
                /* this is hopelessly awful in comparison (but faster)
                btGImpactShapeInterface *s2 =
                    new btGImpactConvexDecompositionShape(v,
                                          Vector3(1,1,1),
                                          0.01);
                */
                s2->updateBound();
                s = s2;
            }

            masterShape->addChildShape(btTransform::getIdentity(), s);
        }

        setMass(bcol.mass);
        setLinearDamping(bcol.linearDamping);
        setAngularDamping(bcol.angularDamping);
        setLinearSleepThreshold(bcol.linearSleepThreshold);
        setAngularSleepThreshold(bcol.angularSleepThreshold);
        setCCDMotionThreshold(bcol.ccdMotionThreshold);
        setCCDSweptSphereRadius(bcol.ccdSweptSphereRadius);
        setInertia(Vector3(bcol.inertia[0],bcol.inertia[1],bcol.inertia[2]));

        compute_inertia = !bcol.inertiaProvided;

    } else if (fourcc==0x4c4f4354) { //TCOL

        ProxyStreamBuf proxy(file);

        std::istream stream(&proxy);
        quex::TColLexer qlex(&stream);
        TColFile tcol;
        parse_tcol_1_0(name,&qlex,tcol);

        is_static = tcol.mass == 0.0f; // static

        masterShape = new btCompoundShape();

        if (tcol.usingCompound) {

            TColCompound &c = tcol.compound;

            for (size_t i=0 ; i<c.hulls.size() ; ++i) {
                const TColHull &h = c.hulls[i];
                btConvexHullShape *s2 = new btConvexHullShape();
                s2->setMargin(h.margin);
                for (unsigned j=0 ; j<h.vertexes.size() ; ++j) {
                    const Vector3 &v = h.vertexes[j];
                    s2->addPoint(to_bullet(v));
                }
                masterShape->addChildShape(btTransform(ZQ,ZV), s2);
                partMaterials.push_back(phys_mats.getMaterial(dir,name,h.material));
            }

            for (size_t i=0 ; i<c.boxes.size() ; ++i) {
                const TColBox &b = c.boxes[i];
                /* implement with hulls
                btConvexHullShape *s2 = new btConvexHullShape();
                s2->addPoint(btVector3(-b.dx/2+b.margin, -b.dy/2+b.margin, -b.dz/2+b.margin));
                s2->addPoint(btVector3(-b.dx/2+b.margin, -b.dy/2+b.margin,  b.dz/2-b.margin));
                s2->addPoint(btVector3(-b.dx/2+b.margin,  b.dy/2-b.margin, -b.dz/2+b.margin));
                s2->addPoint(btVector3(-b.dx/2+b.margin,  b.dy/2-b.margin,  b.dz/2-b.margin));
                s2->addPoint(btVector3( b.dx/2-b.margin, -b.dy/2+b.margin, -b.dz/2+b.margin));
                s2->addPoint(btVector3( b.dx/2-b.margin, -b.dy/2+b.margin,  b.dz/2-b.margin));
                s2->addPoint(btVector3( b.dx/2-b.margin,  b.dy/2-b.margin, -b.dz/2+b.margin));
                s2->addPoint(btVector3( b.dx/2-b.margin,  b.dy/2-b.margin,  b.dz/2-b.margin));
                */
                btBoxShape *s2 =new btBoxShape(btVector3(b.dx/2,b.dy/2,b.dz/2));
                s2->setMargin(b.margin);
                masterShape->addChildShape(btTransform(btQuaternion(b.qx,b.qy,b.qz,b.qw),
                                 btVector3(b.px,b.py,b.pz)), s2);
                partMaterials.push_back(phys_mats.getMaterial(dir,name,b.material));
            }

            for (size_t i=0 ; i<c.cylinders.size() ; ++i) {
                const TColCylinder &cyl = c.cylinders[i];
                btCylinderShape *s2 =
                    new btCylinderShapeZ(btVector3(cyl.dx/2,cyl.dy/2,cyl.dz/2));
                s2->setMargin(cyl.margin);
                masterShape->addChildShape(
                    btTransform(btQuaternion(cyl.qx,cyl.qy,cyl.qz,cyl.qw),
                            btVector3(cyl.px,cyl.py,cyl.pz)), s2);
                partMaterials.push_back(phys_mats.getMaterial(dir,name,cyl.material));
            }

            for (size_t i=0 ; i<c.cones.size() ; ++i) {
                const TColCone &cone = c.cones[i];
                btConeShapeZ *s2 = new btConeShapeZ(cone.radius,cone.height);
                s2->setMargin(cone.margin);
                masterShape->addChildShape(
                      btTransform(btQuaternion(cone.qx,cone.qy,cone.qz,cone.qw),
                          btVector3(cone.px,cone.py,cone.pz)), s2);
                partMaterials.push_back(phys_mats.getMaterial(dir,name,cone.material));
            }

            for (size_t i=0 ; i<c.planes.size() ; ++i) {
                const TColPlane &p = c.planes[i];
                btStaticPlaneShape *s2 =
                    new btStaticPlaneShape(btVector3(p.nx,p.ny,p.nz),p.d);
                masterShape->addChildShape(btTransform(ZQ,ZV), s2);
                partMaterials.push_back(phys_mats.getMaterial(dir,name,p.material));
            }

            for (size_t i=0 ; i<c.spheres.size() ; ++i) {
                const TColSphere &sp = c.spheres[i];
                btSphereShape *s2 = new btSphereShape(sp.radius);
                masterShape->addChildShape(btTransform(ZQ,
                                 btVector3(sp.px,sp.py,sp.pz)), s2);
                partMaterials.push_back(phys_mats.getMaterial(dir,name,sp.material));
            }
        }

        if (tcol.usingTriMesh) {

            TColTriMesh &t = tcol.triMesh;

            std::swap(verts, t.vertexes);
            std::swap(faces, t.faces);


            faceMaterials.reserve(faces.size());
            int counter = 0;
            float accum_area = 0;
            for (TColFaces::const_iterator i=faces.begin(), i_=faces.end() ; i!=i_ ; ++i) {
                //optimisation possible here by changing the TCol struct to be more liek what
                //bullet wants, and then re-using memory
                PhysicalMaterial *mat = phys_mats.getMaterial(dir,name,i->material);
                faceMaterials.push_back(mat);
                CollisionMesh::ProcObjFace po_face(verts[i->v1], verts[i->v2], verts[i->v3]);
                procObjFaceDB[mat->id].faces.push_back(po_face);
                float area = (po_face.AB.cross(po_face.AC)).length();
                APP_ASSERT(area>=0);
                procObjFaceDB[mat->id].areas.push_back(area);
                procObjFaceDB[mat->id].totalArea += area;
                if (++counter = 10) {
                    counter = 0;
                    accum_area = 0;
                    procObjFaceDB[mat->id].areas10.push_back(accum_area);
                }
                accum_area += area;
                    
            }

            btTriangleIndexVertexArray *v = new btTriangleIndexVertexArray(
                faces.size(), &(faces[0].v1), sizeof(TColFace),
                verts.size(), &(verts[0].x), sizeof(Vector3));

            btCollisionShape *s;

            if (is_static) {
                btBvhTriangleMeshShape *tm = new btBvhTriangleMeshShape(v,true,true);
                s = tm;
                s->setMargin(t.margin);
                btTriangleInfoMap* tri_info_map = new btTriangleInfoMap();
                tri_info_map->m_edgeDistanceThreshold = t.edgeDistanceThreshold;

                btGenerateInternalEdgeInfo(tm,tri_info_map);
            } else {
                btGImpactShapeInterface *s2 = new btGImpactMeshShape(v);
                //assume mesh is already shrunk to the given margin
                s2->setMargin(t.margin);
                /* this is hopelessly awful in comparison (but faster)
                btGImpactShapeInterface *s2 =
                    new btGImpactConvexDecompositionShape(v,
                                          Vector3(1,1,1),
                                          0.01);
                */
                s2->updateBound();
                s = s2;
            }

            masterShape->addChildShape(btTransform::getIdentity(), s);
        }

        setMass(tcol.mass);
        setInertia(Vector3(tcol.inertia_x,tcol.inertia_y,tcol.inertia_z));
        setLinearDamping(tcol.linearDamping);
        setAngularDamping(tcol.angularDamping);
        setLinearSleepThreshold(tcol.linearSleepThreshold);
        setAngularSleepThreshold(tcol.angularSleepThreshold);
        setCCDMotionThreshold(tcol.ccdMotionThreshold);
        setCCDSweptSphereRadius(tcol.ccdSweptSphereRadius);

        compute_inertia = !tcol.hasInertia;

    } else {
        GRIT_EXCEPT("Collision mesh \""+name+"\" seems to be corrupt.");
    }


    if (is_static) {
        setInertia(Vector3(0,0,0));
    } else {
        if (compute_inertia) {
            btVector3 i;
            masterShape->calculateLocalInertia(mass,i);
            setInertia(from_bullet(i));
        }
    }

}

void CollisionMesh::unload (void)
{
    //compound of shapes, recursive

    partMaterials.clear();
    faceMaterials.clear();
    procObjFaceDB.clear();
    faces.clear();
    verts.clear();
    bcolFaces.clear();
    bcolVerts.clear();

    if (masterShape == NULL) return;

    int num_children = masterShape->getNumChildShapes();
    for (int i=num_children-1 ; i>=0 ; --i) {
        btCollisionShape *s =  masterShape->getChildShape(i);
        masterShape->removeChildShapeByIndex(i);
        delete s;
    }
    delete masterShape;
    masterShape = NULL;
}

void CollisionMesh::reload (void)
{
    unload();
    load();
    for (Users::iterator i=users.begin(),i_=users.end() ; i!=i_ ; ++i) {
        (*i)->notifyMeshReloaded();
    }
}

PhysicalMaterial *CollisionMesh::getMaterialFromPart (unsigned int id)
{
    if (id >= partMaterials.size()) return 0;
    return partMaterials[id];
}

PhysicalMaterial *CollisionMesh::getMaterialFromFace (unsigned int id)
{
    if (id >= faceMaterials.size()) return 0;
    return faceMaterials[id];
}


// vim: ts=4:sw=4:et
