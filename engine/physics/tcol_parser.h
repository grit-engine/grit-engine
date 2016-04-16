/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include <string>

#include <math_util.h>

#include "tcol_lexer"

struct TColFile;

void parse_tcol_1_0 (const std::string &name, quex::tcol_lexer* qlex, TColFile &file);

void pretty_print_tcol (std::ostream &o, TColFile &f);

typedef std::vector<Vector3> Vertexes;


#ifndef TColParser_h
#define TColParser_h


struct TColHasMargin {
        float margin;
};

struct TColHasMaterial {
        TColHasMaterial () { }
        TColHasMaterial (const std::string &material_) : material(material_) { }
        std::string material;
};

struct TColHull : public TColHasMargin, public TColHasMaterial {
        Vertexes vertexes;
};
typedef std::vector<TColHull> TColHulls;


struct TColBox : public TColHasMargin, public TColHasMaterial {
        float px, py, pz;
        float qw, qx, qy, qz;
        float dx, dy, dz;
};
typedef std::vector<TColBox> TColBoxes;


struct TColCylinder : public TColHasMargin, public TColHasMaterial {
        float px, py, pz;
        float qw, qx, qy, qz;
        float dx, dy, dz;
};
typedef std::vector<TColCylinder> TColCylinders;


struct TColCone : public TColHasMargin, public TColHasMaterial {
        float px, py, pz;
        float qw, qx, qy, qz;
        float radius;
        float height;
};
typedef std::vector<TColCone> TColCones;


struct TColPlane : public TColHasMaterial {
        float nx, ny, nz, d;
};
typedef std::vector<TColPlane> TColPlanes;


struct TColSphere : public TColHasMaterial {
        float px, py, pz;
        float radius;
};
typedef std::vector<TColSphere> TColSpheres;


struct TColCompound {
        TColHulls hulls;
        TColBoxes boxes;
        TColCylinders cylinders;
        TColCones cones;
        TColPlanes planes;
        TColSpheres spheres;
};


struct TColFace : TColHasMaterial {
        TColFace (int v1_, int v2_, int v3_, const std::string &material_)
              : TColHasMaterial(material_), v1(v1_), v2(v2_), v3(v3_) { }
        int v1, v2, v3;
};
typedef std::vector<TColFace> TColFaces;


struct TColTriMesh {
        float margin;
        float edgeDistanceThreshold;
        Vertexes vertexes;
        TColFaces faces;
};


struct TColFile {
        float mass;
        bool hasInertia;
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
        TColCompound compound;
        TColTriMesh triMesh;
};

// useful help with using a different centre of mass
void tcol_offset (TColFile &tcol, float x, float y, float z);

// If someone is using polys in a non-meshy fashion then they might be better off
// extruded into individual hulls
void tcol_triangles_to_hulls (TColFile &tcol, float extrude_by, float margin);

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
