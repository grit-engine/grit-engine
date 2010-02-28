/* Copyright (c) David Cunningham 2010
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

#ifndef DFFREAD_H_
#define DFFREAD_H_

#include <string>
#include <set>
#include <vector>

#include "ideread.h"

#define GEO_TRISTRIP 0x1
#define GEO_POSITIONS 0x2
#define GEO_TEXCOORDS 0x4
#define GEO_COLOURS 0x8
#define GEO_NORMALS 0x10
#define GEO_LIGHTS 0x20
#define GEO_MODULATE 0x40
#define GEO_TEXCOORDS2 0x80
#define GEO_UNKNOWN1 0x10000
#define GEO_UNKNOWN2 0x20000


#define RW_DATA 0x1
#define RW_STRING 0x2
#define RW_EXT 0x3
#define RW_TEXTURE 0x6
#define RW_MATERIAL 0x7
#define RW_MATERIAL_LIST 0x8
#define RW_FRAME_LIST 0xE
#define RW_GEOMETRY 0xF
#define RW_CHUNK 0x10
#define RW_LIGHT 0x12
#define RW_ATOMIC 0x14
#define RW_GEOMETRY_LIST 0x1A
#define RW_RIGHT_TO_RENDER 0x1F
#define RW_UV_ANIMATION_DICTIONARY 0x2B
#define RW_SKY_MIPMAP_VAL 0x110
#define RW_BONE 0x11E
#define RW_SKIN 0x116
#define RW_MATERIAL_EFFECTS 0x120
#define RW_ADC 0x134 // no idea what this is
#define RW_UV_ANIMATION 0x135
#define RW_MATERIALSPLIT 0x50E

#define RW_UNKNOWN 0x0253F2F3
#define RW_SPEC 0x0253F2F6
#define RW_2DFX 0x0253F2F8
#define RW_NIGHT 0x0253F2F9
#define RW_COLLISION 0x0253F2FA
#define RW_REFLECTION_MATERIAL 0x0253F2FC
#define RW_MESHEXT 0x0253F2FD
#define RW_FRAME 0x253F2FE


#define MATBUMP1 0x1 //these are for use in mat_type1
#define MATENV1 0x2
#define MATBUMPENV1 0x3
#define MATDUALPASS1 0x4
#define MATUVTRANSFORM1 0x5
#define MATDUALUVTRANSFORM1 0x6

#define MATBUMP2 0x1 //these are for use in mat_type2
#define MATENV2 0x2
#define MATBUMPENV2 0x1
#define MATDUALPASS2 0x4
#define MATUVTRANSFORM2 0x5
#define MATDUALUVTRANSFORM2 0x5

typedef std::set<std::string> StringSet;

typedef std::vector<std::string> Strings;

struct texture {
        unsigned short filter_flags;
        unsigned short unk1;
        std::string name;
        bool has_alpha;
};

struct reflection_material { float unk1, unk2, unk3, unk4, unk5, unk6; };

struct material {
        unsigned long colour;
        unsigned long unk2;
        unsigned long num_textures;
        float unk3, unk4, unk5;
        std::vector<struct texture> textures;
        Strings rewrittenTextures;
        struct reflection_material reflection_material;
        unsigned long mat_type1; //for effects stuff
        unsigned long mat_type2; //for effects stuff
        unsigned long mat_type3; //for effects stuff
        float multiplier; //for effects stuff
        float multiplier2; //for effects stuff
        unsigned long src_blend_type; //for effects stuff
        unsigned long dest_blend_type; //for effects stuff
        unsigned long dest_blend_type2; //for effects stuff
        unsigned long num_effects_textures; //for effects stuff
        unsigned long num_effects_textures2; //for effects stuff
        std::vector<struct texture> effects_textures;
        std::vector<struct texture> effects_textures2;
        float spec_level;
        std::string spec_texture;
        unsigned long rtr_unk1, rtr_unk2;
};

struct existing_material { 
    float r;
    float g;
    float b;
    float a;
    Strings texs;
    bool alpha_blend;
    bool double_sided;
    bool no_alpha_reject;
    bool dynamic_lighting;
};

bool operator< (const existing_material &a, const existing_material &b);

typedef std::map<existing_material,std::string> MatDB;


struct fragment {
        std::string texname, atexname;
        float unk1;
        float unk2;
        float unk3;
};

// material split
struct MatSplit {
        std::string surrogate; // a material we should use instead
        unsigned long num_indexes;
        unsigned long material;
        //unsigned longset indexes;
        std::vector<unsigned long> indexes2;
};
typedef std::vector<MatSplit> MatSplits;

struct face { unsigned short v2, v1, flags, v3; };

struct vect {
        float x, y, z;
        vect () : x(0), y(0), z(0) { }
        vect (float x_, float y_, float z_) : x(x_), y(y_), z(z_) { }
};

struct texcoord { float u, v; };
struct light { float unk1, unk2; };

static inline bool vect_eq (struct vect v1, struct vect v2)
{
    return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z;
}


struct geometry {
        unsigned long flags;
        unsigned long num_faces;
        unsigned long num_vertexes;
        unsigned long num_frames;
        float ambient, diffuse, specular;
        std::vector<unsigned long> vertex_cols;
        std::vector<struct texcoord> tex_coords;
        std::vector<struct face> faces;
        float b_x, b_y, b_z, b_r; // bounding sphere
        unsigned long has_position;
        unsigned long has_normals;
        std::vector<struct vect> vertexes;
        std::vector<struct vect> normals;
        std::vector<struct light> lights;
        std::vector<struct texcoord> tex_coords2;
        unsigned long num_materials;
        std::vector<struct material> materials;
        unsigned long face_type;
        unsigned long num_mat_spls;
        unsigned long num_indexes;
        std::vector<struct MatSplit> mat_spls;
        unsigned long unk2;
        unsigned long vertex_night_unk;
        std::vector<unsigned long> vertex_night_cols;
        std::vector<float> meshext_vertexes;
        std::vector<unsigned long> meshext_vertexcols;
        std::vector<float> meshext_texcoords;
        std::vector<unsigned short> meshext_indexes;
        std::vector<unsigned short> meshext_face_fragment;
        std::vector<struct fragment> meshext_fragments;

        unsigned long frame;
};

struct frame {
        float rot[9];
        float x, y, z;
        unsigned long parent_frame;
        std::vector<unsigned long> children;
        unsigned long unk;
        unsigned long bone_unk_flags;
        unsigned long bone_id;
        unsigned long num_bones;
        unsigned long bone_unk1, bone_unk2;
        std::vector<unsigned long> bone_ids;
        std::vector<unsigned long> bone_nums;
        std::vector<unsigned long> bone_types;
        std::string name;
        unsigned long geometry;
};

struct object {
        unsigned long frame_index, geometry_index;
        unsigned long unk1, unk2, rtr_unk1, rtr_unk2, eff;
};

struct light2 {
        float unk1;
        float unk2;
        float unk3;
        float unk4;
        float unk5;
        float unk6;
};

struct dff {
        std::vector<struct frame> frames;
        unsigned long root_frame;
        std::vector<struct geometry> geometries;
        std::vector<struct object> objects;
        std::vector<struct light2> lights;
        unsigned long unk;
};

void ios_read_dff(int debug_threshold, std::ifstream &f,
                  struct dff *c, const std::string &p);

void export_xml (const StringSet &texs,
                 const ide &ide,
                 const std::vector<std::string> &img,
                 std::ostream &out,
                 const std::string &fname,
                 const Obj &obj,
                 const std::string &oname,
                 struct geometry &g,
                 MatDB &matdb,
                 std::ostream &materials_lua);

void init_ogre (void);

void export_mesh (const StringSet &texs,
                  const ide &ide,
                  const std::vector<std::string> &img,
                  std::ostream &out,
                  const std::string &fname,
                  const Obj &obj,
                  const std::string &oname,
                  struct geometry &g,
                  MatDB &matdb,
                  std::ostream &materials_lua);

void generate_normals (struct geometry &g);
#endif

// vim: shiftwidth=8:tabstop=8:expandtab
