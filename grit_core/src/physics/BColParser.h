/* Copyright (c) David Cunningham and the Grit Game Engine project 2011
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
 * THE SOFTwARE IS PROVIDED "AS IS", wITHOUT wARRANTy OF ANy KIND, ExPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE wARRANTIES OF MERCHANTABILITy,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPyRIGHT HOLDERS BE LIABLE FOR ANy CLAIM, DAMAGES OR OTHER
 * LIABILITy, wHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERwISE, ARISING FROM,
 * OUT OF OR IN CONNECTION wITH THE SOFTwARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTwARE.
 */

#include <iostream>

struct BColFile;
struct TColFile;

void write_bcol_as_tcol (std::ostream &o, BColFile &f);

void write_tcol_as_bcol (std::ostream &o, TColFile &f);

#ifndef BColParser_h
#define BColParser_h

#include <vector>

#ifdef _MSC_VER
        typedef unsigned long uint32_t;
#else
        #include <stdint.h>
#endif

#ifdef _MSC_VER
        #define GRIT_PACKED_BEFORE #pragma pack (push,1)
        #define GRIT_PACKED_AFTER #pragma pack (pop)
        #define GRIT_PACKED_ATTR
#else
        #define GRIT_PACKED_BEFORE
        #define GRIT_PACKED_AFTER
        #define GRIT_PACKED_ATTR __attribute__((__packed__))
#endif


GRIT_PACKED_BEFORE

struct OffsetBase {
    template<class T> T *offset (size_t n, size_t e=0)
    {
        char *ptr = reinterpret_cast<char*>(this);
        T *offset_ptr = reinterpret_cast<T*>(&ptr[n]);
        return &offset_ptr[e];
    }
} GRIT_PACKED_ATTR;

struct BColVert {
    float x, y, z;
    static size_t size (void) { return 3*4; }
} GRIT_PACKED_ATTR;
    
struct BColMat  {
    // had to duplicate code here instead of extending OffsetBae, otherwise some sort of
    // multiple inheritance problem caused BColHull to be 1 byte too large...
    template<class T> T *offset (size_t n, size_t e=0)
    {
        char *ptr = reinterpret_cast<char*>(this);
        T *offset_ptr = reinterpret_cast<T*>(&ptr[n]);
        return &offset_ptr[e];
    }
    uint32_t charOff; // offset (relative to this) where the text can be found
    char *name (void) { return offset<char>(charOff); }
    static size_t size (void) { return 1*4; }
} GRIT_PACKED_ATTR;

struct BColHull : OffsetBase {
    BColMat mat;
    float margin;
    uint32_t vertNum;
    uint32_t vertOff; // offset (relative to this) where BColVertex may be found
    BColVert *verts (int i=0) { return offset<BColVert>(vertOff,i); }
    static size_t size (void) { return 3*4 + BColMat::size(); }
} GRIT_PACKED_ATTR;

struct BColBox {
    BColMat mat;
    float margin;
    float px, py, pz;
    float dx, dy, dz;
    float qw, qx, qy, qz;
    static size_t size (void) { return 11*4 + BColMat::size(); }
} GRIT_PACKED_ATTR;

struct BColCyl {
    BColMat mat;
    float margin;
    float px, py, pz;
    float dx, dy, dz;
    float qw, qx, qy, qz;
    static size_t size (void) { return 11*4 + BColMat::size(); }
} GRIT_PACKED_ATTR;

struct BColCone {
    BColMat mat;
    float margin;
    float px, py, pz;
    float radius, height;
    float qw, qx, qy, qz;
    static size_t size (void) { return 10*4 + BColMat::size(); }
} GRIT_PACKED_ATTR;

struct BColPlane {
    BColMat mat;
    float nx, ny, nz;
    float d;
    static size_t size (void) { return 4*4 + BColMat::size(); }
} GRIT_PACKED_ATTR;

struct BColSphere { 
    BColMat mat;
    float px, py, pz;
    float radius;
    static size_t size (void) { return 4*4 + BColMat::size(); }
} GRIT_PACKED_ATTR;

struct BColFace {
    BColMat mat;
    uint32_t v1, v2, v3;
    static size_t size (void) { return 3*4 + BColMat::size(); }
} GRIT_PACKED_ATTR;

struct BColFile : OffsetBase {
    char header[8]; // "BCOL1.0\n"
    float mass;
    uint32_t inertiaProvided; // 
    float inertia[3];
    float linearDamping;
    float angularDamping;
    float linearSleepThreshold;
    float angularSleepThreshold;
    float ccdMotionThreshold;
    float ccdSweptSphereRadius;
    uint32_t hullNum, hullOff;
    uint32_t boxNum, boxOff;
    uint32_t cylNum, cylOff;
    uint32_t coneNum, coneOff;
    uint32_t planeNum, planeOff;
    uint32_t sphereNum, sphereOff;
    float triMeshMargin;
    float triMeshEdgeDistanceThreshold;
    uint32_t triMeshVertNum, triMeshVertOff; // offset (relative to this) to BColVertex array
    uint32_t triMeshFaceNum, triMeshFaceOff; // offset (relative to this) to BColFace array

    BColHull *hulls (int i=0) { return offset<BColHull>(hullOff,i); }
    BColBox *boxes (int i=0) { return offset<BColBox>(boxOff,i); }
    BColCyl *cyls (int i=0) { return offset<BColCyl>(cylOff,i); }
    BColCone *cones (int i=0) { return offset<BColCone>(coneOff,i); }
    BColPlane *planes (int i=0) { return offset<BColPlane>(planeOff,i); }
    BColSphere *spheres (int i=0) { return offset<BColSphere>(sphereOff,i); }

    long totalCompoundElements (void)
    { return hullNum + boxNum + cylNum + coneNum + planeNum + sphereNum; }
    
    BColVert *triMeshVerts (int i=0) { return offset<BColVert>(triMeshVertOff,i); }
    BColFace *triMeshFaces (int i=0) { return offset<BColFace>(triMeshFaceOff,i); }

    static size_t size (void) { return 31*4; }
} GRIT_PACKED_ATTR;

GRIT_PACKED_AFTER

typedef std::vector<BColFace> BColFaces;
typedef std::vector<BColVert> BColVerts;

#endif
