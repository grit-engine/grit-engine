#include <iostream>
#include <cstdlib>

#include <OgreException.h>

#include "CollisionMesh.h"
#include "TColParser.h"

#include <LinearMath/btGeometryUtil.h>


btCompoundShape *import_compound (const Compound &c,
                                  CollisionMesh *cm,
                                  LooseEnds &les)
{

        btCompoundShape *s =  new btCompoundShape();
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
        }

        for (size_t i=0 ; i<c.boxes.size() ; ++i) {
                const Box &b = c.boxes[i];
                btBoxShape *s2 =new btBoxShape(btVector3(b.dx/2,b.dy/2,b.dz/2));
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s2->setMargin(b.margin);
                s->addChildShape(btTransform(btQuaternion(b.qx,b.qy,b.qz,b.qw),
                                             btVector3(b.px,b.py,b.pz)), s2);
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
        }

        for (size_t i=0 ; i<c.cones.size() ; ++i) {
                const Cone &cone = c.cones[i];
                btConeShapeZ *s2 = new btConeShapeZ(cone.radius,cone.height);
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s2->setMargin(cone.margin);
                s->addChildShape(
                      btTransform(btQuaternion(cone.qx,cone.qy,cone.qz,cone.qw),
                                  btVector3(cone.px,cone.py,cone.pz)), s2);
        }

        for (size_t i=0 ; i<c.planes.size() ; ++i) {
                const Plane &p = c.planes[i];
                btStaticPlaneShape *s2 =
                        new btStaticPlaneShape(btVector3(p.nx,p.ny,p.nz),p.d);
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s->addChildShape(btTransform(ZQ,ZV), s2);
        }

        for (size_t i=0 ; i<c.spheres.size() ; ++i) {
                const Sphere &sp = c.spheres[i];
                btSphereShape *s2 = new btSphereShape(sp.radius);
                les.push_back(new LooseEndImpl<btCollisionShape>(s2));
                s->addChildShape(btTransform(ZQ,
                                             btVector3(sp.px,sp.py,sp.pz)), s2);
        }

        for (size_t i=0 ; i<c.compounds.size() ; ++i) {
                const Compound &c2 = c.compounds[i];
                btCompoundShape *s2 = import_compound(c2,cm,les);
                s->addChildShape(btTransform(ZQ,ZV), s2);
        }

        return s;
}


btCollisionShape *import_trimesh (const TriMesh &f,
                                  CollisionMesh *cm,
                                  LooseEnds &les)
{
        (void) cm;

        Vertexes *vertexes = new Vertexes();
        les.push_back(new LooseEndImpl<Vertexes>(vertexes));
        int sz = f.vertexes.size();
        vertexes->reserve(sz);
        for (int i=0 ; i<sz ; ++i) {
                vertexes->push_back(f.vertexes[i]);
        }

        Faces *faces = new Faces(f.faces);
        les.push_back(new LooseEndImpl<Faces>(faces));

        btTriangleIndexVertexArray *v = new btTriangleIndexVertexArray(
                faces->size(), &((*faces)[0].v1), sizeof(Face),
                vertexes->size(), &((*vertexes)[0][0]), sizeof(btVector3));
        les.push_back(new LooseEndImpl<btTriangleIndexVertexArray>(v));

        btCollisionShape *s = new btBvhTriangleMeshShape(v,true,true);
        s->setMargin(0);
        les.push_back(new LooseEndImpl<btCollisionShape>(s));

        return s;
}


btCollisionShape *import (const TColFile &f, CollisionMesh *cm, LooseEnds &les)
{
        cm->setMass(f.mass);
        cm->setInertia(Ogre::Vector3(f.inertia_x,f.inertia_y,f.inertia_z));
        cm->setFriction(f.friction);
        cm->setRestitution(f.restitution);
        cm->setLinearDamping(f.linearDamping);
        cm->setAngularDamping(f.angularDamping);
        cm->setLinearSleepThreshold(f.linearSleepThreshold);
        cm->setAngularSleepThreshold(f.angularSleepThreshold);
        cm->setCCDMotionThreshold(f.ccdMotionThreshold);
        cm->setCCDSweptSphereRadius(f.ccdSweptSphereRadius);

        if (f.usingCompound) {
                btCompoundShape *s = import_compound (f.compound, cm, les);
                if (f.usingTriMesh) {
                        btCollisionShape *s2 =
                                import_trimesh (f.triMesh, cm, les);
                        const btVector3 ZV(0,0,0);
                        const btQuaternion ZQ(0,0,0,1);
                        s->addChildShape(btTransform(ZQ,ZV), s2);
                }
                return s;
        } else {
                return import_trimesh (f.triMesh, cm, les);
        }
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
                shape = import(tcol,this,looseEnds);
        }
}


// vim: shiftwidth=8:tabstop=8:expandtab
