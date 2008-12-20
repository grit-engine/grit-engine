#include <OgreString.h>

#include <LinearMath/btGeometryUtil.h>

#include "TColLexer"

struct TColFile;

void parse_tcol_1_0 (const Ogre::String &name,
                     quex::TColLexer* qlex,
                     TColFile &file);

void pretty_print_tcol (std::ostream &o, TColFile &f);

typedef btAlignedObjectArray<btVector3> Vertexes;

#ifndef TColParser_h
#define TColParser_h



struct Hull {
        Ogre::Real margin;
        Vertexes vertexes;
};
typedef std::vector<Hull> Hulls;


struct Box {
        Ogre::Real margin;
        Ogre::Real px, py, pz;
        Ogre::Real qw, qx, qy, qz;
        Ogre::Real dx, dy, dz;
};
typedef std::vector<Box> Boxes;


struct Cylinder {
        Ogre::Real margin;
        Ogre::Real px, py, pz;
        Ogre::Real qw, qx, qy, qz;
        Ogre::Real dx, dy, dz;
};
typedef std::vector<Cylinder> Cylinders;


struct Cone {
        Ogre::Real margin;
        Ogre::Real px, py, pz;
        Ogre::Real qw, qx, qy, qz;
        Ogre::Real radius;
        Ogre::Real height;
};
typedef std::vector<Cone> Cones;


struct Plane {
        Ogre::Real nx, ny, nz, d;
};
typedef std::vector<Plane> Planes;


struct Sphere {
        Ogre::Real px, py, pz;
        Ogre::Real radius;
};
typedef std::vector<Sphere> Spheres;


struct Compound {
        Hulls hulls;
        Boxes boxes;
        Cylinders cylinders;
        Cones cones;
        Planes planes;
        Spheres spheres;
        std::vector<Compound> compounds;
};


struct Face {
        Face (int v1_, int v2_, int v3_, unsigned int flag_)
              : v1(v1_), v2(v2_), v3(v3_), flag(flag_) { }
        int v1, v2, v3;
        unsigned long flag;
};
typedef std::vector<Face> Faces;


struct TriMesh {
        Vertexes vertexes;
        Faces faces;
};


struct TColFile {
        Ogre::Real mass;
        Ogre::Real inertia_x;
        Ogre::Real inertia_y;
        Ogre::Real inertia_z;
        Ogre::Real friction;
        Ogre::Real restitution;
        Ogre::Real linearDamping;
        Ogre::Real angularDamping;
        Ogre::Real linearSleepThreshold;
        Ogre::Real angularSleepThreshold;
        Ogre::Real ccdMotionThreshold;
        Ogre::Real ccdSweptSphereRadius;
        bool usingCompound;
        bool usingTriMesh;
        Compound compound;
        TriMesh triMesh;
};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
