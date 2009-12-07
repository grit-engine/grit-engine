#include <OgreString.h>

#include <LinearMath/btGeometryUtil.h>

#include "TColLexer"

struct TColFile;

void parse_tcol_1_0 (const Ogre::String &name,
                     quex::TColLexer* qlex,
                     TColFile &file);

void pretty_print_tcol (std::ostream &o, TColFile &f);

typedef btAlignedObjectArray<btVector3> Vertexes;

typedef unsigned int physics_mat;

#ifndef TColParser_h
#define TColParser_h

struct HasMargin {
        float margin;
};

struct HasMaterial {
        HasMaterial () { }
        HasMaterial (physics_mat material_) : material(material_) { }
        physics_mat material;
};

struct Hull : public HasMargin, public HasMaterial {
        Vertexes vertexes;
};
typedef std::vector<Hull> Hulls;


struct Box : public HasMargin, public HasMaterial {
        float px, py, pz;
        float qw, qx, qy, qz;
        float dx, dy, dz;
};
typedef std::vector<Box> Boxes;


struct Cylinder : public HasMargin, public HasMaterial {
        float px, py, pz;
        float qw, qx, qy, qz;
        float dx, dy, dz;
};
typedef std::vector<Cylinder> Cylinders;


struct Cone : public HasMargin, public HasMaterial {
        float px, py, pz;
        float qw, qx, qy, qz;
        float radius;
        float height;
};
typedef std::vector<Cone> Cones;


struct Plane : public HasMaterial {
        float nx, ny, nz, d;
};
typedef std::vector<Plane> Planes;


struct Sphere : public HasMaterial {
        float px, py, pz;
        float radius;
};
typedef std::vector<Sphere> Spheres;


struct Compound {
        Hulls hulls;
        Boxes boxes;
        Cylinders cylinders;
        Cones cones;
        Planes planes;
        Spheres spheres;
};


struct Face {
        Face (int v1_, int v2_, int v3_, physics_mat material_)
              : v1(v1_), v2(v2_), v3(v3_), material(material_) { }
        int v1, v2, v3;
        physics_mat material;
};
typedef std::vector<Face> Faces;


struct TriMesh {
        float margin;
        Vertexes vertexes;
        Faces faces;
};


struct TColFile {
        float mass;
        float inertia_x;
        float inertia_y;
        float inertia_z;
        float linearDamping;
        float angularDamping;
        float linearSleepThreshold;
        float angularSleepThreshold;
        float ccdMotionThreshold;
        float ccdSweptSphereRadius;
        bool usingCompound;
        bool usingTriMesh;
        Compound compound;
        TriMesh triMesh;
};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
