#include <iostream>
#include <cstdlib>

#include <OgreException.h>
#include <OgreResourceGroupManager.h>

#include "CollisionMesh.h"
#include "TColParser.h"

#include "CentralisedLog.h"

#include <LinearMath/btGeometryUtil.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <../Extras/GIMPACTUtils/btGImpactConvexDecompositionShape.h>


btCompoundShape *import_compound (const Compound &c,
                                  LooseEnds &les,
                                  CollisionMesh::Materials &partMaterials)
{

        btCompoundShape *s = new btCompoundShape();
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
                faceMaterials.push_back(i->material);
        }

        btTriangleIndexVertexArray *v = new btTriangleIndexVertexArray(
                faces->size(), &((*faces)[0].v1), sizeof(Face),
                vertexes->size(), &((*vertexes)[0][0]), sizeof(btVector3));
        les.push_back(new LooseEndImpl<btTriangleIndexVertexArray>(v));

        btCollisionShape *s;

        if (is_static) {
                s = new btBvhTriangleMeshShape(v,true,true);
                s->setMargin(0);
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


btCollisionShape *import (const TColFile &f,
                          CollisionMesh *cm,
                          LooseEnds &les,
                          CollisionMesh::Materials &partMaterials,
                          CollisionMesh::Materials &faceMaterials)
{

        bool stat = f.mass == 0.0f; // static
        btCollisionShape *s;

        if (f.usingCompound) {
                btCompoundShape *cs = import_compound (f.compound, les, partMaterials);
                if (f.usingTriMesh) {
                        btCollisionShape *s2 = import_trimesh (f.triMesh, stat, les, faceMaterials);
                        cs->addChildShape(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)), s2);
                }
                s = cs;
        } else {
                s = import_trimesh (f.triMesh, stat, les, faceMaterials);
        }

        cm->setMass(f.mass);
        cm->setInertia(Ogre::Vector3(f.inertia_x,f.inertia_y,f.inertia_z));
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


void CollisionMesh::importFromFile (const Ogre::DataStreamPtr &file)
{
        ProxyStreamBuf proxy(file);
        {
                std::istream stream(&proxy);
                quex::TColLexer qlex(&stream);
                TColFile tcol;
                parse_tcol_1_0(file->getName(),&qlex,tcol);

                Materials m1, m2;
                LooseEnds ls;
                btCollisionShape *loaded_shape;

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
                shape = loaded_shape;
                looseEnds = ls;
                partMaterials = m1;
                faceMaterials = m2;
        }
}

void CollisionMesh::reload (void)
{
        importFromFile(Ogre::ResourceGroupManager::getSingleton().openResource(name,"GRIT"));
}

physics_mat CollisionMesh::getMaterialFromPart (unsigned int id)
{
        return partMaterials[id];
}

physics_mat CollisionMesh::getMaterialFromFace (unsigned int id)
{
        return faceMaterials[id];
}


// vim: shiftwidth=8:tabstop=8:expandtab
