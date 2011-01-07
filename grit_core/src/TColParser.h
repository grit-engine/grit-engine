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

#include <OgreString.h>

#include "TColLexer"
#include "math_util.h"

struct TColFile;
struct PhysicsMaterial;
class MaterialDB;

void parse_tcol_1_0 (const std::string &name,
                     quex::TColLexer* qlex,
                     TColFile &file,
                     const MaterialDB &db);

void pretty_print_tcol (std::ostream &o, TColFile &f, const MaterialDB &db);

typedef std::vector<Vector3> Vertexes;


#ifndef TColParser_h
#define TColParser_h

struct PhysicsMaterial {
        std::string name;
        int interactionGroup;
        int id;
};  

class MaterialDB {

    public:

        const PhysicsMaterial &getMaterial (const std::string &name) const
        {
                SDB::const_iterator i = mdb.find(name);
                if (i==mdb.end())
                        GRIT_EXCEPT("Physical Material \""+name+"\" does not exist.");
                return i->second;
        }

        const PhysicsMaterial &getMaterial (int material) const
        { return *mdb2[material]; }

        const PhysicsMaterial &getMaterialSafe (int material) const
        {
                if ((size_t)material<mdb2.size()) return *mdb2[material];
                if (mdb2.size() == 0) {
                        CERR << "MaterialDB is empty!" << std::endl;
                        app_fatal();
                }
                CERR << "Got a bad material id: " << material << std::endl;
                return *mdb2[0];
        }

        void setMaterial (const std::string &name, int interaction_group)
        {
                if (mdb.find(name) == mdb.end()) {
                        PhysicsMaterial &m = mdb[name];
                        m.name = name;
                        m.interactionGroup = interaction_group;
                        m.id = mdb2.size();
                        mdb2.push_back(&m);
                } else {
                        PhysicsMaterial &m = mdb[name];
                        m.interactionGroup = interaction_group;
                }
        }

    protected:

        typedef std::map<std::string, PhysicsMaterial> SDB; // map string to material
        SDB mdb;

        typedef std::vector<PhysicsMaterial*> IDB; // map id to material
        IDB mdb2;
};

struct HasMargin {
        float margin;
};

struct HasMaterial {
        HasMaterial () { }
        HasMaterial (int material_) : material(material_) { }
        int material;
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
        Face (int v1_, int v2_, int v3_, int material_)
              : v1(v1_), v2(v2_), v3(v3_), material(material_) { }
        int v1, v2, v3;
        int material;
};
typedef std::vector<Face> Faces;


struct TriMesh {
        float margin;
        Radian maxEdgeAngleThreshold;
        float edgeDistanceThreshold;
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

// useful help with using a different centre of mass
void tcol_offset (TColFile &tcol, float x, float y, float z);

// If someone is using polys in a non-meshy fashion then they might be better off
// extruded into individual hulls
void tcol_triangles_to_hulls (TColFile &tcol, float extrude_by, float margin);

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
