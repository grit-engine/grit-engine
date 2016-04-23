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

#include <cmath>

#include <sstream>

#include <portable_io.h>

#include "col_defaults.h"
#include "bcol_parser.h"
#include "tcol_parser.h"

static inline bool fnear(const float x, const float y)
{
    return fabs(x-y) < 1E-6;
}

static inline bool ffar(const float x, const float y)
{
    return !fnear(x,y);
}

void pretty_print_material (std::ostream &o, BColMat &material)
{
    o << "\"" << material.name() << "\"";
}

void write_bcol_as_tcol (std::ostream &o, BColFile &f)
{
    o << std::fixed; // use fixed point (no exponents)

    o << "TCOL1.0\n\n"
      << "attributes {\n";
    if (f.mass==0) {
        o << "\tstatic;\n";
    } else {
        o << "\tmass " << f.mass << ";\n";
    }
/*
    o << "\tinertia " << f.inertia_x << " "
              << f.inertia_y << " "
              << f.inertia_z << ";\n";
*/
    if (ffar(f.linearDamping,DEFAULT_LINEAR_DAMPING))
        o << "\tlinear_damping " << f.linearDamping << ";\n";
    if (ffar(f.angularDamping,DEFAULT_ANGULAR_DAMPING))
        o << "\tangular_damping " << f.angularDamping << ";\n";
    if (ffar(f.linearSleepThreshold,DEFAULT_LINEAR_SLEEP_THRESHOLD))
        o << "\tlinear_sleep_threshold " << f.linearSleepThreshold << ";\n";
    if (ffar(f.angularSleepThreshold,DEFAULT_ANGULAR_SLEEP_THRESHOLD))
        o << "\tangular_sleep_threshold " << f.angularSleepThreshold << ";\n";
    if (ffar(f.ccdMotionThreshold,DEFAULT_CCD_MOTION_THRESHOLD))
        o << "\tccd_motion_threshold " << f.ccdMotionThreshold << ";\n";
    if (ffar(f.ccdSweptSphereRadius,DEFAULT_CCD_SWEPT_SPHERE_RADIUS))
        o << "\tccd_swept_sphere_radius " << f.ccdSweptSphereRadius << ";\n";
    o << "}\n\n";

    if (f.totalCompoundElements() > 0 || f.triMeshFaceNum == 0) {
        o << "compound {\n";

        for (size_t i=0 ; i<f.hullNum ; ++i) {
            BColHull &h = *f.hulls(i);
            o<<"\t"<<"hull {\n";
            //o<<"\t\t"<<"material "; pretty_print_material(o, h.mat); o<<";\n";
            if (ffar(h.margin,DEFAULT_MARGIN)) {
                o<<"\t\t"<<"margin "<<h.margin<<";\n";
            }
            o<<"\t\t"<<"vertexes {\n";
            for (unsigned j=0 ; j<h.vertNum ; ++j) {
                BColVert &v = *h.verts(j);
                o<<"\t\t\t"<<v.x<<" "<<v.y<<" "<<v.z<<";"<<"\n";
            }
            o<<"\t\t"<<"}\n";
            o<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<f.boxNum ; ++i) {
            BColBox &b = *f.boxes(i);
            o<<"\t"<<"box {\n";
            o<<"\t\t"<<"material "; pretty_print_material(o, b.mat); o<<";\n";
            if (ffar(b.margin,DEFAULT_MARGIN)) {
                o<<"\t\t"<<"margin "<<b.margin<<";\n";
            }
            o<<"\t\t"<<"centre "<<b.px
                 <<" "<<b.py<<" "<<b.pz<<";\n";
            if (ffar(b.qw,1) && ffar(b.qx,0) &&
            ffar(b.qy,0) && ffar(b.qz,0)) {
                o<<"\t\t"<<"orientation "<<b.qw<<" "<<b.qx
                 <<" "<<b.qy<<" "<<b.qz<<";\n";
            }
            o<<"\t\t"<<"dimensions "<<b.dx<<" "<<b.dy
                        <<" "<<b.dz<<";\n";
            o<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<f.cylNum ; ++i) {
            BColCyl &cyl = *f.cyls(i);
            o<<"\t"<<"cylinder {\n";
            o<<"\t\t"<<"material "; pretty_print_material(o, cyl.mat); o<<";\n";
            if (ffar(cyl.margin,DEFAULT_MARGIN)) {
                o<<"\t\t"<<"margin "<<cyl.margin<<";\n";
            }    
            o<<"\t\t"<<"centre "<<cyl.px
                 <<" "<<cyl.py<<" "<<cyl.pz<<";\n";
            if (ffar(cyl.qw,1) && ffar(cyl.qx,0) &&
            ffar(cyl.qy,0) && ffar(cyl.qz,0)) {
                o<<"\t\t"<<"orientation "<<cyl.qw<<" "<<cyl.qx
                 <<" "<<cyl.qy<<" "<<cyl.qz<<";\n";
            }
            o<<"\t\t"<<"dimensions "<<cyl.dx<<" "<<cyl.dy
                        <<" "<<cyl.dz<<";\n";
            o<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<f.coneNum ; ++i) {
            BColCone &cone = *f.cones(i);
            o<<"\t"<<"cone {\n";
            o<<"\t\t"<<"material "; pretty_print_material(o, cone.mat); o<<";\n";
            if (ffar(cone.margin,DEFAULT_MARGIN)) {
                o<<"\t\t"<<"margin "<<cone.margin<<";\n";
            }
            o<<"\t\t"<<"centre "<<cone.px
                 <<" "<<cone.py<<" "<<cone.pz<<";\n";
            if (ffar(cone.qw,1) && ffar(cone.qx,0) &&
            ffar(cone.qy,0) && ffar(cone.qz,0)) {
                o<<"\t\t"<<"orientation "<<cone.qw<<" "<<cone.qx
                 <<" "<<cone.qy<<" "<<cone.qz<<";\n";
            }
            o<<"\t\t"<<"radius "<<cone.radius<<";\n";
            o<<"\t\t"<<"height "<<cone.height<<";\n";
            o<<"\t"<<"}\n";
        }
        
        for (size_t i=0 ; i<f.planeNum ; ++i) {
            BColPlane &p = *f.planes(i);
            o<<"\t"<<"plane {\n";
            o<<"\t\t"<<"material "; pretty_print_material(o, p.mat); o<<";\n";
            o<<"\t\t"<<"normal "<<p.nx<<" "<<p.ny<<" "<<p.nz<<";\n";
            o<<"\t\t"<<"distance "<<p.d<<";\n";
            o<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<f.sphereNum ; ++i) {
            BColSphere &s = *f.spheres(i);
            o<<"\t"<<"sphere {\n";
            o<<"\t\t"<<"material "; pretty_print_material(o, s.mat); o<<";\n";
            o<<"\t\t"<<"centre "<<s.px<<" "<<s.py<<" "<<s.pz<<";\n";
            o<<"\t\t"<<"radius "<<s.radius<<";\n";
            o<<"\t"<<"}\n";
        }

        o << "}\n"; 
    }

    if (f.triMeshFaceNum > 0) {
        o << "trimesh {\n";
        if (ffar(f.triMeshMargin,0.0)) {
            o<<"\t"<<"margin "<<f.triMeshMargin<<";\n";
        }
        o << "\tedge_distance_threshold "<<f.triMeshEdgeDistanceThreshold<<";\n";
        o << "\tvertexes {\n";
        for (unsigned i=0 ; i<f.triMeshVertNum ; ++i) {
            BColVert &v = *f.triMeshVerts(i);
            o<<"\t\t"<<v.x<<" "<<v.y<<" "<<v.z<<";"<<"\n";
        }
        o << "\t}\n";
        o << "\tfaces {\n";
        for (unsigned i=0 ; i<f.triMeshFaceNum ; ++i) {
            BColFace &face = *f.triMeshFaces(i);
            o<<"\t\t"<<face.v1<<" "<<face.v2<<" "<<face.v3<<" ";
            pretty_print_material(o, face.mat);
            o<<";"<<"\n";
        }
        o << "\t}\n";
        o << "}\n";
    }
}

namespace {


}

void write_tcol_as_bcol (std::ostream &o, TColFile &f)
{

    TColCompound &c = f.compound;
    TColTriMesh &t = f.triMesh;

    const size_t hull_start = BColFile::size();
    const size_t box_start = hull_start + BColHull::size()*c.hulls.size();
    const size_t cyl_start = box_start + BColBox::size()*c.boxes.size();
    const size_t cone_start = cyl_start + BColCyl::size()*c.cylinders.size();
    const size_t plane_start = cone_start + BColCone::size()*c.cones.size();
    const size_t sphere_start = plane_start + BColPlane::size()*c.planes.size();
    const size_t hull_verts_start = sphere_start + BColSphere::size()*c.spheres.size();

    size_t trimesh_vert_start = hull_verts_start;
    for (unsigned i=0 ; i<c.hulls.size() ; ++i) {
        size_t n = c.hulls[i].vertexes.size();
        trimesh_vert_start += BColVert::size() * n;
    }

    size_t trimesh_face_start = trimesh_vert_start + BColVert::size()*t.vertexes.size();
    size_t text_start = trimesh_face_start + BColFace::size()*t.faces.size();

    ios_write_byte_array(o, "BCOL1.0\n", 8);
    ios_write_float(o, f.mass);
    ios_write_u32(o, f.hasInertia);
    ios_write_float(o,f.inertia_x);
    ios_write_float(o,f.inertia_y);
    ios_write_float(o,f.inertia_z);
    ios_write_float(o,f.linearDamping);
    ios_write_float(o,f.angularDamping);
    ios_write_float(o,f.linearSleepThreshold);
    ios_write_float(o,f.angularSleepThreshold);
    ios_write_float(o,f.ccdMotionThreshold);
    ios_write_float(o,f.ccdSweptSphereRadius);
    ios_write_u32(o, c.hulls.size()); ios_write_u32(o, hull_start); // hull
    ios_write_u32(o, c.boxes.size()); ios_write_u32(o, box_start); // box
    ios_write_u32(o, c.cylinders.size()); ios_write_u32(o, cyl_start); // cyl
    ios_write_u32(o, c.cones.size()); ios_write_u32(o, cone_start); // cone
    ios_write_u32(o, c.planes.size()); ios_write_u32(o, plane_start); // plane
    ios_write_u32(o, c.spheres.size()); ios_write_u32(o, sphere_start); // sphere
    ios_write_u32(o, 0); ios_write_u32(o, 0); // sphere
    ios_write_float(o,t.margin);
    ios_write_float(o,t.edgeDistanceThreshold);
    ios_write_u32(o, t.vertexes.size()); ios_write_u32(o, trimesh_vert_start); // verts
    ios_write_u32(o, t.faces.size()); ios_write_u32(o, trimesh_face_start); // faces
    //CTRACE(t.vertexes.size());
    //CTRACE(t.faces.size());

    class StringAccumulator {
        private:
        typedef std::map<std::string, int> Map;
        Map map;
        std::vector<std::string> table;
        size_t off;
        public:
        StringAccumulator (size_t off) : off(off) { }
        int operator[] (const std::string &str)
        {
            Map::iterator i = map.find(str);
            if (i != map.end()) return i->second;
            size_t r = off;
            map[str] = off;
            table.push_back(str);
            off += str.length()+1;
            return r;
        }
        void write(std::ostream &o)
        {
            for (unsigned i=0 ; i<table.size() ; ++i) {
                ios_write_byte_array(o, table[i].c_str(), table[i].length()+1);
            }
        }
        
    } sa(text_start);

    // hulls
    size_t hull_verts_current = hull_verts_start;
    size_t local_hull_start = hull_start;
    for (size_t i=0 ; i<c.hulls.size() ; ++i) {
        TColHull &h = c.hulls[i];
        size_t local_start = hull_verts_current - local_hull_start;
        ios_write_u32(o, sa[h.material] - local_start);
        ios_write_float(o, h.margin);
        ios_write_u32(o, h.vertexes.size());
        ios_write_u32(o, hull_verts_current - local_hull_start);
        hull_verts_current += BColVert::size() * h.vertexes.size();
        local_hull_start += BColHull::size();
    }

    // boxes
    size_t local_box_start = hull_start;
    for (size_t i=0 ; i<c.boxes.size() ; ++i) {
        TColBox &b = c.boxes[i];
        ios_write_u32(o, sa[b.material] - local_box_start);
        ios_write_float(o, b.margin);
        ios_write_float(o, b.px);
        ios_write_float(o, b.py);
        ios_write_float(o, b.pz);
        ios_write_float(o, b.dx);
        ios_write_float(o, b.dy);
        ios_write_float(o, b.dz);
        ios_write_float(o, b.qw);
        ios_write_float(o, b.qx);
        ios_write_float(o, b.qy);
        ios_write_float(o, b.qz);
        local_box_start += BColBox::size();
    }

    // cylinders
    size_t local_cyl_start = cyl_start;
    for (size_t i=0 ; i<c.cylinders.size() ; ++i) {
        TColCylinder &cyl = c.cylinders[i];
        ios_write_u32(o, sa[cyl.material] - local_cyl_start);
        ios_write_float(o, cyl.margin);
        ios_write_float(o, cyl.px);
        ios_write_float(o, cyl.py);
        ios_write_float(o, cyl.pz);
        ios_write_float(o, cyl.dx);
        ios_write_float(o, cyl.dy);
        ios_write_float(o, cyl.dz);
        ios_write_float(o, cyl.qw);
        ios_write_float(o, cyl.qx);
        ios_write_float(o, cyl.qy);
        ios_write_float(o, cyl.qz);
        local_cyl_start += BColCyl::size();
    }

    // cones
    size_t local_cone_start = cone_start;
    for (size_t i=0 ; i<c.cones.size() ; ++i) {
        TColCone &cone = c.cones[i];
        ios_write_u32(o, sa[cone.material] - local_cone_start);
        ios_write_float(o, cone.margin);
        ios_write_float(o, cone.px);
        ios_write_float(o, cone.py);
        ios_write_float(o, cone.pz);
        ios_write_float(o, cone.radius);
        ios_write_float(o, cone.height);
        ios_write_float(o, cone.qw);
        ios_write_float(o, cone.qx);
        ios_write_float(o, cone.qy);
        ios_write_float(o, cone.qz);
        local_cone_start += BColCone::size();
    }

    // planes
    size_t local_plane_start = plane_start;
    for (size_t i=0 ; i<c.planes.size() ; ++i) {
        TColPlane &p = c.planes[i];
        ios_write_u32(o, sa[p.material] - local_plane_start);
        ios_write_float(o, p.nx);
        ios_write_float(o, p.ny);
        ios_write_float(o, p.nz);
        ios_write_float(o, p.d);
        local_plane_start += BColPlane::size();
    }

    // spheres
    size_t local_sphere_start = sphere_start;
    for (size_t i=0 ; i<c.spheres.size() ; ++i) {
        TColSphere &s = c.spheres[i];
        ios_write_u32(o, sa[s.material] - local_sphere_start);
        ios_write_float(o, s.px);
        ios_write_float(o, s.py);
        ios_write_float(o, s.pz);
        ios_write_float(o, s.radius);
        local_sphere_start += BColSphere::size();
    }

    // verts for hulls
    for (size_t i=0 ; i<c.hulls.size() ; ++i) {
        TColHull &h = c.hulls[i];
        for (unsigned j=0 ; j<h.vertexes.size() ; ++j) {
            ios_write_float(o, h.vertexes[j].x);
            ios_write_float(o, h.vertexes[j].y);
            ios_write_float(o, h.vertexes[j].z);
        }
    }


    // verts for trimesh
    for (unsigned j=0 ; j<t.vertexes.size() ; ++j) {
        ios_write_float(o, t.vertexes[j].x);
        ios_write_float(o, t.vertexes[j].y);
        ios_write_float(o, t.vertexes[j].z);
    }


    // faces for trimesh
    size_t local_trimesh_face_start = trimesh_face_start;
    for (unsigned j=0 ; j<t.faces.size() ; ++j) {
        ios_write_u32(o, sa[t.faces[j].material] - local_trimesh_face_start);
        ios_write_u32(o, t.faces[j].v1);
        ios_write_u32(o, t.faces[j].v2);
        ios_write_u32(o, t.faces[j].v3);
        local_trimesh_face_start += BColFace::size();
    }   

    // text
    sa.write(o);
    
}


template<class T> static bool bcol_assert_struct_size_ok (const char *name)
{
    unsigned is = sizeof(T);
    unsigned want = T::size();
    if (is != want) {
        fprintf(stderr,"Executable is broken: %s is %u bytes, need %u bytes.\n", name, is, want);
        return false;
    }
    return true;
}

static bool bcol_assert_structs_size_ok (void) {
    bool r = true;
    r &= bcol_assert_struct_size_ok<BColVert>("BColVert");
    r &= bcol_assert_struct_size_ok<BColMat>("BColMat");
    r &= bcol_assert_struct_size_ok<BColHull>("BColHull");
    r &= bcol_assert_struct_size_ok<BColBox>("BColBox");
    r &= bcol_assert_struct_size_ok<BColCyl>("BColCyl");
    r &= bcol_assert_struct_size_ok<BColCone>("BColCone");
    r &= bcol_assert_struct_size_ok<BColPlane>("BColPlane");
    r &= bcol_assert_struct_size_ok<BColSphere>("BColSphere");
    r &= bcol_assert_struct_size_ok<BColFace>("BColFace");
    r &= bcol_assert_struct_size_ok<BColFile>("BColFile");
    if (!r) exit(EXIT_FAILURE);
    return r;
}

bool unused = bcol_assert_structs_size_ok();
