/* Copyright (c) The Grit Game Engine authors 2016
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

#include <cstdlib>
#include <cstddef>
#include <climits>
#include <locale>
//#include <cstdarg>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

//#include <errno.h>

#include <OgreDefaultHardwareBufferManager.h>
#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <OgreMeshSerializer.h>
#include <OgreLogManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreMeshManager.h>
#include <OgreLodStrategyManager.h>

#include "ios_util.h"

#include "dffread.h"
#include "ideread.h"
#include "tex_dups.h"

#ifdef rad2
#undef rad2
#endif
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define HEX(x) std::hex<<(x)<<std::dec

#define DECHEX(x) (x)<<" [0x"<<HEX(x)<<"]"
#define FLTDECHEX(x) (*(float*)&(x))<<" "<<(x)<<" [0x"<<HEX(x)<<"]"

static void unrec (const std::string &p, unsigned long type,
                   unsigned long size, const std::string &word)
{
    std::stringstream ss;
    ss << p << type << " 0x" << HEX(type)
       << " ("<<size<<"B) UNREC "<<word<<" EXT!";
    GRIT_EXCEPT(ss.str());
}


static int tolowr (int c)
{
    return std::tolower(char(c),std::cout.getloc());
}

static std::string& strlower(std::string& s)
{
    std::transform(s.begin(),s.end(), s.begin(),tolowr);
    return s;
}



#define check_spill(f,s,sz,p) check_spill_(f,s,sz,__FILE__,__LINE__,p)

#define VBOS(x,y) if (x<d) { std::cout<<y<<std::endl; } else { }


static inline void check_spill_(std::ifstream &f,
                                std::streamoff start, unsigned long should,
                                const char* src, int line, const std::string &p)
{{{
    std::streamoff did = (f.tellg() - start);
    ptrdiff_t overspill = did - should;
    if (overspill) {
        std::stringstream msg;
        msg<<p<<"Read "<<abs(overspill)<<" bytes too "<<(overspill>0?"many":"few")<<", "<<did<<" not "<<should<<" ("<<src<<":"<<line<<").[0m"<<std::endl;
        GRIT_EXCEPT(msg.str());
    }
}}}


static void ios_read_texture (int d, std::ifstream &f,
                              unsigned long file_version,
                              struct texture *t,
                              const std::string &p)
{{{
    unsigned long type, tex_size;
    ios_read_header(f,&type,&tex_size,NULL,&file_version);
    APP_ASSERT(type==RW_TEXTURE);
    std::streamoff tex_start = f.tellg();

    unsigned long size;
    ios_read_header(f,&type,&size,NULL,&file_version);
    APP_ASSERT(type==RW_DATA);
    APP_ASSERT(size==4);
    t->filter_flags = ios_read_u16(f);
    VBOS(3,p<<"filter_flags: "<<HEX(t->filter_flags));
    t->unk1 = ios_read_u16(f);
    VBOS(3,p<<"mysterious_flag: "<<HEX(t->unk1));

    ios_read_header(f,&type,&size,NULL,&file_version);
    APP_ASSERT(type==RW_STRING);
    t->name = ios_read_fixedstr(f,size);
    VBOS(3,p<<"name: "<<t->name);

    ios_read_header(f,&type,&size,NULL,&file_version);
    APP_ASSERT(type==RW_STRING);
    std::string alpha_str = ios_read_fixedstr(f,size);
    t->has_alpha = alpha_str!="";
    VBOS(3,p<<"has_alpha: "<<t->has_alpha);

    unsigned long totalsize;
    ios_read_header(f,&type,&totalsize,NULL,&file_version);
    APP_ASSERT(type==RW_EXT);

    while (totalsize>0) {
        unsigned long size;
        ios_read_header(f,&type,&size,NULL,&file_version);
        totalsize -= size + 12;
        switch(type) {
        case RW_SKY_MIPMAP_VAL: {
            unsigned long sky_mipmap = ios_read_u32(f);
            VBOS(3,p<<"sky_mipmap: "<<sky_mipmap);
         } break;
        default:
            unrec(p,type,size,"TEXTURE");
        }
    }

    check_spill(f,tex_start,tex_size,p);
}}}


static void ios_read_material_effs (int d,std::ifstream &f,
                                    unsigned long file_version,
                                    struct material *m,
                                    const std::string &p)
{{{
    m->mat_type1 = ios_read_u32(f);
    VBOS(3,p<<"mat_type1: "<<m->mat_type1);
    m->mat_type2 = ios_read_u32(f);
    VBOS(3,p<<"mat_type2: "<<m->mat_type2);
    switch(m->mat_type1) {
    case MATBUMP1:
        APP_ASSERT(m->mat_type2 == MATBUMP2);
        m->multiplier = ios_read_float(f);
        VBOS(3,p<<"multiplier: "<<m->multiplier);
        m->dest_blend_type = ios_read_u32(f);
        APP_ASSERT(m->dest_blend_type==0);
        m->num_effects_textures = ios_read_u32(f);
        VBOS(3,p<<"num_effects_textures: "<<m->num_effects_textures);
        m->effects_textures.resize(m->num_effects_textures);
        for (unsigned long i=0 ; i<m->num_effects_textures ; i++) {
            std::stringstream prefss;
            prefss<<p<<"effects_texture["<<i<<"].";
            std::string pref = prefss.str();
            ios_read_texture(d,f,file_version,&m->effects_textures[i],pref);
        }
        ios_read_u32(f);// WHAT THE FUCK?
        break;
    case MATBUMPENV1:
        APP_ASSERT(m->mat_type2 == MATBUMP1);
        m->multiplier = ios_read_float(f);
        VBOS(3,p<<"multiplier: "<<m->multiplier);
        m->dest_blend_type = ios_read_u32(f);
        APP_ASSERT(m->dest_blend_type==0);
        m->num_effects_textures = ios_read_u32(f);
        VBOS(3,p<<"num_effects_textures: "<<m->num_effects_textures);
        m->effects_textures.resize(m->num_effects_textures);
        for (unsigned long i=0 ; i<m->num_effects_textures ; i++) {
            std::stringstream prefss;
            prefss<<p<<"effects_texture["<<i<<"].";
            std::string pref = prefss.str();
            ios_read_texture(d,f,file_version,&m->effects_textures[i],pref);
        }

        m->mat_type3 = ios_read_u32(f);
        VBOS(3,p<<"mat_type3: "<<m->mat_type3);
        APP_ASSERT(m->mat_type3 == MATENV1);
        m->multiplier2 = ios_read_float(f);
        VBOS(3,p<<"multiplier2: "<<m->multiplier2);
        m->dest_blend_type2 = ios_read_u32(f);
        APP_ASSERT(m->dest_blend_type2==0);
        m->num_effects_textures2 = ios_read_u32(f);
        VBOS(3,p<<"num_effects_textures2: "<<m->num_effects_textures2);
        m->effects_textures2.resize(m->num_effects_textures2);
        for (unsigned long i=0 ; i<m->num_effects_textures2 ; i++) {
            std::stringstream prefss;
            prefss<<p<<"effects_texture["<<i<<"].";
            std::string pref = prefss.str();
            ios_read_texture(d,f,file_version,&m->effects_textures2[i],pref);
        }
        break;
    case MATUVTRANSFORM1:
        APP_ASSERT(m->mat_type2 == MATUVTRANSFORM2);
        ios_read_u32(f);// WHAT THE FUCK?
        break;
    case MATDUALPASS1:
        APP_ASSERT(m->mat_type2 == MATDUALPASS2);
        m->src_blend_type = ios_read_u32(f);
        VBOS(3,p<<"src_blend_type: "<<m->src_blend_type);
        m->dest_blend_type = ios_read_u32(f);
        VBOS(3,p<<"dest_blend_type: "<<m->dest_blend_type);
        m->num_effects_textures = ios_read_u32(f);
        VBOS(3,p<<"num_effects_textures: "<<m->num_effects_textures);
        m->effects_textures.resize(m->num_effects_textures);
        for (unsigned long i=0 ; i<m->num_effects_textures ; i++) {
            std::stringstream prefss;
            prefss<<p<<"effects_texture["<<i<<"].";
            std::string pref = prefss.str();
            ios_read_texture(d,f,file_version, &m->effects_textures[i],pref);
        }
        ios_read_u32(f);// WHAT THE FUCK?
        break;
    case MATENV1:
        APP_ASSERT(m->mat_type2 == MATENV2);
        m->multiplier = ios_read_float(f);
        VBOS(3,p<<"multiplier: "<<m->multiplier);
        m->src_blend_type = ios_read_u32(f);
        VBOS(3,p<<"dest_blend_type: "<<m->dest_blend_type);
        m->num_effects_textures = ios_read_u32(f);
        VBOS(3,p<<"num_effects_textures: "<<m->num_effects_textures);
        m->effects_textures.resize(m->num_effects_textures);
        for (unsigned long i=0 ; i<m->num_effects_textures ; i++) {
            std::stringstream prefss;
            prefss<<p<<"effects_texture["<<i<<"].";
            std::string pref = prefss.str();
            ios_read_texture(d,f,file_version, &m->effects_textures[i],pref);
        }
        ios_read_u32(f);// WHAT THE FUCK?
        break;
    default:
        std::stringstream ss;
        ss << p << m->mat_type1 << " 0x" << HEX(m->mat_type1)
           << "UNREC MATERIAL EFFECT!";
        GRIT_EXCEPT(ss.str());
    }
}}}

static void ios_read_material (int d,
                               std::ifstream &f,
                               unsigned long file_version,
                               struct material *m,
                               const std::string &p)
{{{
    unsigned long type, material_size;
    ios_read_header(f,&type,&material_size,NULL,&file_version); 
    APP_ASSERT(type==RW_MATERIAL);
    std::streamoff material_start = f.tellg();

    ios_read_header(f,&type,NULL,NULL,&file_version);
    APP_ASSERT(type==RW_DATA);
    unsigned long zero = ios_read_u32(f);
    APP_ASSERT(zero==zero);
    m->colour = ios_read_u32(f);
    m->unk2 = ios_read_u32(f);
    m->num_textures = ios_read_u32(f);
    m->textures.resize(m->num_textures);
    m->unk3 = ios_read_float(f);
    m->unk4 = ios_read_float(f);
    m->unk5 = ios_read_float(f);
    VBOS(3,p<<"colour: "<<HEX(m->colour));
    VBOS(3,p<<"mat_unk2: "<<HEX(m->unk2));
    VBOS(3,p<<"num_textures: "<<m->num_textures);
    VBOS(3,p<<"mat_unk3: "<<m->unk3);
    VBOS(3,p<<"mat_unk4: "<<m->unk4);
    APP_ASSERT(m->unk5==1.0);

    m->textures.resize(m->num_textures);
    for (unsigned long i=0 ; i<m->num_textures ; i++) {
        std::stringstream prefss;
        prefss<<p<<"texture["<<i<<"].";
        ios_read_texture(d,f,file_version,&m->textures[i],prefss.str());
    }

    unsigned long totalsize;
    ios_read_header(f,&type,&totalsize,NULL,&file_version);
    APP_ASSERT(type==RW_EXT);

    while (totalsize>0) {
        unsigned long size;
        ios_read_header(f,&type,&size,NULL,&file_version);
        std::streamoff start = f.tellg();
        totalsize -= size + 12;
        switch (type) {
        case RW_REFLECTION_MATERIAL:
            APP_ASSERT(size==24);
            m->reflection_material.unk1 = ios_read_float(f);
            m->reflection_material.unk2 = ios_read_float(f);
            m->reflection_material.unk3 = ios_read_float(f);
            m->reflection_material.unk4 = ios_read_float(f);
            m->reflection_material.unk5 = ios_read_float(f);
            m->reflection_material.unk6 = ios_read_float(f);
            VBOS(3,p<<"reflection_material.unk1: "
                   <<m->reflection_material.unk1);
            VBOS(3,p<<"reflection_material.unk2: "
                   <<m->reflection_material.unk2);
            VBOS(3,p<<"reflection_material.unk3: "
                   <<m->reflection_material.unk3);
            VBOS(3,p<<"reflection_material.unk4: "
                   <<m->reflection_material.unk4);
            VBOS(3,p<<"reflection_material.unk5: "
                   <<m->reflection_material.unk5);
            APP_ASSERT(m->reflection_material.unk6==0.0);
            break;
        case RW_MATERIAL_EFFECTS:
            ios_read_material_effs(d,f,file_version,m,p);
            break;
        case RW_SPEC:
            m->spec_level = ios_read_float(f);
            VBOS(3,p<<"spec_level: "<<m->spec_level);
            m->spec_texture = ios_read_fixedstr(f,size-4);
            VBOS(3,p<<"spec_texture: "<<m->spec_texture);
            break;
        case RW_UV_ANIMATION:
            unsigned long anim_size;
            ios_read_header(f,&type,&anim_size,NULL,&file_version);
            APP_ASSERT(type==RW_DATA);
            APP_ASSERT(anim_size==36);
            VBOS(1,p<<"SKIPPING_OVER_ADC: "<<DECHEX(anim_size));
            ios_read_byte_array(f,NULL,anim_size);
            break;
        case RW_RIGHT_TO_RENDER:
            APP_ASSERT(size==8);
            m->rtr_unk1 = ios_read_u32(f);
            VBOS(3,p<<"rtr_unk1: "<<DECHEX(m->rtr_unk1));
            m->rtr_unk2 = ios_read_u32(f);
            VBOS(3,p<<"rtr_unk2: "<<DECHEX(m->rtr_unk2));
            break;
        default:
            unrec(p,type,size,"MATERIAL");
        }
        check_spill(f,start,size,p);
    }
    check_spill(f,material_start,material_size,p);

}}}


typedef std::vector<unsigned long> Ixs;
typedef std::vector<struct vect> Vxs;

static Ixs decode_strip (const Ixs &strip, const Vxs &vertexes)
{
    Ixs r;
    r.reserve((strip.size()-2)*3);
    unsigned long v1=0, v2=0, v3=0;
    int vertexes_so_far = 0;
    int tri_counter = 0;
    // in a triangle strip, the normal alternates.
    // we reverse the order of the vertexes if tri_counter is odd
    for (Ixs::const_iterator i=strip.begin(),i_=strip.end() ; i!=i_ ; i++) {
        v1=v2;
        v2=v3;
        v3 = *i;
        vertexes_so_far++;
        if (vertexes_so_far>=3 && v1!=v2 && v2!=v3 && v3!=v1) {
            if (vect_eq(vertexes[v1],vertexes[v2]) ||
                vect_eq(vertexes[v2],vertexes[v3]) ||
                vect_eq(vertexes[v3],vertexes[v1])) {
                // degenerate due to independent but co-located vertexes
            } else {
                if (tri_counter%2 == 0) {
                    r.push_back(v1);
                    r.push_back(v2);
                    r.push_back(v3);
                } else {
                    r.push_back(v1);
                    r.push_back(v3);
                    r.push_back(v2);
                }
            }
        }
        if (vertexes_so_far>=3) tri_counter++;
    }

    return r;
}

static void ios_read_geometry (int d,
                               std::ifstream &f,
                               unsigned long file_version,
                               struct geometry &g,
                               const std::string &p)
{{{
    g.frame = -1;

    unsigned long type, geometry_size;
    ios_read_header(f,&type,&geometry_size,NULL,&file_version);
    APP_ASSERT(type==RW_GEOMETRY);
    std::streamoff geometry_start = f.tellg();

    unsigned long struct_size;
    ios_read_header(f,&type,&struct_size,NULL,&file_version);
    std::streamoff struct_start = f.tellg();
    APP_ASSERT(type==RW_DATA);
    g.flags = ios_read_u32(f);
    VBOS(3,p<<"geometry_struct_size: "<<struct_size);
    VBOS(3,p<<"geometry_flags: "<<HEX(g.flags)<<": "
            <<(g.flags&GEO_TRISTRIP?"STRIP ":"")
            <<(g.flags&GEO_POSITIONS?"POS ":"")
            <<(g.flags&GEO_TEXCOORDS?"UV ":"")
            <<(g.flags&GEO_COLOURS?"COLS ":"")
            <<(g.flags&GEO_NORMALS?"NORMS ":"")
            <<(g.flags&GEO_LIGHTS?"LIT ":"")
            <<(g.flags&GEO_MODULATE?"MOD ":"")
            <<(g.flags&GEO_TEXCOORDS2?"UV2 ":"")
            <<(g.flags&GEO_UNKNOWN1?"UNK1 ":"")
            <<(g.flags&GEO_UNKNOWN2?"UNK2 ":""));
    unsigned long tmp = g.flags;
    tmp |= GEO_TRISTRIP;
    tmp |= GEO_POSITIONS;
    tmp |= GEO_TEXCOORDS;
    tmp |= GEO_COLOURS;
    tmp |= GEO_NORMALS;
    tmp |= GEO_LIGHTS;
    tmp |= GEO_MODULATE;
    tmp |= GEO_TEXCOORDS2;
    tmp |= GEO_UNKNOWN1;
    tmp |= GEO_UNKNOWN2;
    APP_ASSERT(tmp==(GEO_TRISTRIP|GEO_POSITIONS|GEO_TEXCOORDS|GEO_COLOURS|
               GEO_NORMALS|GEO_LIGHTS|GEO_MODULATE|GEO_TEXCOORDS2|
               GEO_UNKNOWN1|GEO_UNKNOWN2));
    g.num_faces = ios_read_u32(f);
    VBOS(3,p<<"num_faces: "<<g.num_faces);
    g.faces.resize(g.num_faces);
    g.num_vertexes = ios_read_u32(f);
    VBOS(3,p<<"num_vertexes: "<<g.num_vertexes);
    g.vertexes.resize(g.num_vertexes);
    g.num_frames = ios_read_u32(f);
    APP_ASSERT(g.num_frames==1);
    if (file_version==0x1003FFFF) {
        g.ambient = ios_read_float(f);
        g.diffuse = ios_read_float(f);
        g.specular = ios_read_float(f);
        VBOS(0,p<<"lighting: "<<g.ambient<<" "<<g.diffuse<<" "<<g.specular);
    }
    if (g.flags&GEO_COLOURS) {
        g.vertex_cols.resize(g.num_vertexes);
        for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
            g.vertex_cols[i] = ios_read_u32(f);
            VBOS(4,p<<"vertex_cols["<<i<<"]: "<<HEX(g.vertex_cols[i]));
        }
    }
    if (g.flags&GEO_UNKNOWN1) {
        g.tex_coords.resize(g.num_vertexes);
        for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
            g.tex_coords[i].u = ios_read_float(f);
            g.tex_coords[i].v = ios_read_float(f);
            VBOS(4,p<<"texture_coords["<<i<<"]: "
                    <<g.tex_coords[i].u<<" "<<g.tex_coords[i].v);
        }
    }
    if (g.flags&GEO_UNKNOWN2) {
        g.tex_coords2.resize(g.num_vertexes);
        for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
            g.tex_coords2[i].u = ios_read_float(f);
            g.tex_coords2[i].v = ios_read_float(f);
            VBOS(4,p<<"texture_coords2["<<i<<"]: "
                    <<g.tex_coords2[i].u<<" "<<g.tex_coords2[i].v);
        }
        g.lights.resize(g.num_vertexes);
        for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
            g.lights[i].unk1 = ios_read_float(f);
            g.lights[i].unk2 = ios_read_float(f);
            VBOS(4,p<<"lights["<<i<<"]: "
                   <<g.lights[i].unk1<<" "<<g.lights[i].unk2);
        }
    }
    for (unsigned long i=0 ; i<g.num_faces ; i++) {
        g.faces[i].v2 = ios_read_u16(f);
        g.faces[i].v1 = ios_read_u16(f);
        g.faces[i].flags = ios_read_u16(f);
        g.faces[i].v3 = ios_read_u16(f);
        VBOS(4,p<<"face["<<i<<"]: "
               <<g.faces[i].v1<<" "<<g.faces[i].v2<<" "
               <<g.faces[i].v3<<" "<<HEX(g.faces[i].flags));
    }
    g.b_x = ios_read_float(f);
    g.b_y = ios_read_float(f);
    g.b_z = ios_read_float(f);
    g.b_r = ios_read_float(f);
    VBOS(3,p<<"bounding_sphere: ("
            <<g.b_x<<","<<g.b_y<<","<<g.b_z<<") radius "<<g.b_r);
    g.has_position = ios_read_u32(f);
    VBOS(3,p<<"has_position: "<<g.has_position);
    g.has_normals = ios_read_u32(f);
    VBOS(3,p<<"has_normals: "<<g.has_normals);
    //if (g.flags&GEO_POSITIONS) {
    if (g.has_position) {
        g.vertexes.resize(g.num_vertexes);
        for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
            g.vertexes[i].x = ios_read_float(f);
            g.vertexes[i].y = ios_read_float(f);
            g.vertexes[i].z = ios_read_float(f);
            VBOS(4,p<<"vertex_coords["<<i<<"]: "
                   <<"("<<g.vertexes[i].x<<","<<g.vertexes[i].y
                   <<","<<g.vertexes[i].z<<")");
        }
    }
    //if (g.flags&GEO_NORMALS) {
    if (g.has_normals) {
        g.normals.resize(g.num_vertexes);
        for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
            g.normals[i].x = ios_read_float(f);
            g.normals[i].y = ios_read_float(f);
            g.normals[i].z = ios_read_float(f);
            VBOS(4,p<<"vertex_normals["<<i<<"]: "
                   <<"("<<g.normals[i].x<<","<<g.normals[i].y
                   <<","<<g.normals[i].z<<")");
        }
    }
    check_spill(f,struct_start,struct_size,p);

    ios_read_header(f,&type,NULL,NULL,&file_version);
    APP_ASSERT(type==RW_MATERIAL_LIST);

    ios_read_header(f,&type,NULL,NULL,&file_version);
    APP_ASSERT(type==RW_DATA);
    g.num_materials = ios_read_u32(f);
    g.materials.resize(g.num_materials);
    VBOS(4,p<<"num_materials: "<<g.num_materials);
    for (unsigned long i=0 ; i<g.num_materials ; i++) {
        unsigned long big = ios_read_u32(f);
        APP_ASSERT(big==ULONG_MAX);
    }

    for (unsigned long i=0 ; i<g.num_materials ; i++) {
        std::stringstream prefss;
        prefss<<p<<"material["<<i<<"].";
        ios_read_material(d,f,file_version,&g.materials[i],prefss.str());
    }

    unsigned long totalsize;
    ios_read_header(f,&type,&totalsize,NULL,&file_version);
    APP_ASSERT(type==RW_EXT);

    while (totalsize>0) {
        unsigned long size; 
        ios_read_header(f,&type,&size,NULL,&file_version);
        totalsize -= size + 12;
        switch (type) {
        case RW_MATERIALSPLIT:
            g.face_type = ios_read_u32(f);
            VBOS(3,p<<"face_type: "<<g.face_type);
            g.num_mat_spls = ios_read_u32(f);
            g.mat_spls.resize(g.num_mat_spls);
            VBOS(3,p<<"num_mat_spls: "<<g.num_mat_spls);
            g.num_indexes = ios_read_u32(f);
            VBOS(3,p<<"num_indexes: "<<g.num_indexes);
            for (unsigned long i=0 ; i<g.num_mat_spls ; i++) {
                std::stringstream prefss;
                prefss<<p<<"MatSplit["<<i<<"].";
                std::string pref = prefss.str();
                struct MatSplit &s = g.mat_spls[i];
                s.num_indexes =ios_read_u32(f);
                VBOS(3,pref<<"num_indexes: "<<s.num_indexes);
                s.material = ios_read_u32(f);
                VBOS(3,pref<<"material: "<<s.material);
                s.indexes2.resize(s.num_indexes);
                for (unsigned long j=0 ; j<s.num_indexes ; j++) {
                    s.indexes2[j] = ios_read_u32(f);
                    VBOS(4,pref<<"index["<<j<<"]: "<<s.indexes2[j]);
                }
                if (g.flags & GEO_TRISTRIP)
                    s.indexes2 = decode_strip(s.indexes2, g.vertexes);
            }
            break;
        case RW_NIGHT:
            g.vertex_night_unk = ios_read_u32(f);
            VBOS(3,p<<"vertex_night_unk: "<<HEX(g.vertex_night_unk));
            if (!g.vertex_night_unk) break;
            g.vertex_night_cols.resize(g.num_vertexes);
            for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
                g.vertex_night_cols[i] = ios_read_u32(f);
                VBOS(4,p<<"vertex_night_colour["<<i<<"]: "
                        <<HEX(g.vertex_night_cols[i]));
            }
            break;
        case RW_ADC:
            VBOS(1,p<<"SKIPPING_OVER_ADC: "<<DECHEX(size));
            ios_read_byte_array(f,NULL,size);
            break;
        case RW_SKIN:
            VBOS(1,p<<"SKIPPING_OVER_SKIN: "<<DECHEX(size));
            ios_read_byte_array(f,NULL,size);
            break;
        case RW_MESHEXT: {

            std::streamoff mesh_ext_start = f.tellg();

            unsigned long fourcc = ios_read_u32(f);
            if (fourcc) {
                unsigned long one = ios_read_u32(f);
                (void)one;
                //APP_ASSERT(one==1);
                unsigned long vcount = ios_read_u32(f);
                unsigned long unk1 = ios_read_u32(f);
                VBOS(3,p<<"meshext_unk1: "<<DECHEX(unk1));
                unsigned long unk2 = ios_read_u32(f);
                VBOS(3,p<<"meshext_unk2: "<<DECHEX(unk2));
                unsigned long unk3 = ios_read_u32(f);
                VBOS(3,p<<"meshext_unk3: "<<DECHEX(unk3));
                unsigned long fcount = ios_read_u32(f);
                unsigned long unk4 = ios_read_u32(f);
                VBOS(3,p<<"meshext_unk4: "<<DECHEX(unk4));
                unsigned long unk5 = ios_read_u32(f);
                VBOS(3,p<<"meshext_unk5: "<<DECHEX(unk5));
                unsigned long fragcount = ios_read_u32(f);
                unsigned long unk6 = ios_read_u32(f);
                VBOS(3,p<<"meshext_unk6: "<<DECHEX(unk6));
                unsigned long unk7 = ios_read_u32(f);
                VBOS(3,p<<"meshext_unk7: "<<DECHEX(unk7));
                unsigned long unk8 = ios_read_u32(f);
                VBOS(3,p<<"meshext_unk8: "<<DECHEX(unk8));
                unsigned long unk9 = ios_read_u32(f);
                VBOS(3,p<<"meshext_unk9: "<<DECHEX(unk9));

                g.meshext_vertexes.reserve(3*vcount);
                for (unsigned long i=0 ; i<vcount ; ++i) {
                    g.meshext_vertexes.push_back(ios_read_float(f));
                    g.meshext_vertexes.push_back(ios_read_float(f));
                    g.meshext_vertexes.push_back(ios_read_float(f));
                }

                g.meshext_texcoords.reserve(2*vcount);
                for (unsigned long i=0 ; i<vcount ; ++i) {
                    g.meshext_texcoords.push_back(ios_read_float(f));
                    g.meshext_texcoords.push_back(ios_read_float(f));
                }

                g.meshext_vertexcols.reserve(vcount);
                for (unsigned long i=0 ; i<vcount ; ++i) {
                    g.meshext_vertexcols.push_back(ios_read_u32(f));
                }

                g.meshext_indexes.reserve(3*fcount);
                for (unsigned long i=0 ; i<fcount ; ++i) {
                    g.meshext_indexes.push_back(ios_read_u16(f));
                    g.meshext_indexes.push_back(ios_read_u16(f));
                    g.meshext_indexes.push_back(ios_read_u16(f));
                }

                g.meshext_face_fragment.reserve(fcount);
                for (unsigned long i=0 ; i<fcount ; ++i) {
                    g.meshext_face_fragment.push_back(ios_read_u16(f));
                }

                g.meshext_fragments.resize(fragcount);

                for (unsigned long i=0 ; i<fragcount ; ++i) {
                    fragment &frag = g.meshext_fragments[i];
                    frag.texname = ios_read_fixedstr(f,32);
                    VBOS(3,p<<"meshext_frag["<<i<<"].tex: "<<frag.texname);
                }

                for (unsigned long i=0 ; i<fragcount ; ++i) {
                    fragment &frag = g.meshext_fragments[i];
                    frag.atexname = ios_read_fixedstr(f,32);
                    VBOS(3,p<<"meshext_frag["<<i<<"].atex: "<<frag.atexname);
                }

                for (unsigned long i=0 ; i<fragcount ; ++i) {
                    fragment &frag = g.meshext_fragments[i];
                    frag.unk1 = ios_read_float(f);
                    VBOS(3,p<<"meshext_frag["<<i<<"].unk1: "<<frag.unk1);
                }

                for (unsigned long i=0 ; i<fragcount ; ++i) {
                    fragment &frag = g.meshext_fragments[i];
                    frag.unk2 = ios_read_float(f);
                    VBOS(3,p<<"meshext_frag["<<i<<"].unk2: "<<frag.unk2);
                }

                for (unsigned long i=0 ; i<fragcount ; ++i) {
                    fragment &frag = g.meshext_fragments[i];
                    frag.unk3 = ios_read_float(f);
                    VBOS(3,p<<"meshext_frag["<<i<<"].unk3: "<<frag.unk3);
                }

                check_spill(f,mesh_ext_start,size,p);

            }
        } break;

        case RW_2DFX: {
            uint32_t num_2dfx = ios_read_u32(f);
            VBOS(1,p<<"num_2dfx = "<<num_2dfx);
            g.twodfxs.resize(num_2dfx);
            uint32_t counter = 4;
            for (unsigned j=0 ; j<num_2dfx ; ++j) {
                twodfx &fx = g.twodfxs[j];
                fx.x = ios_read_float(f);
                fx.y = ios_read_float(f);
                fx.z = ios_read_float(f);
                VBOS(1,p<<"2DFX["<<j<<"].x = "<<fx.x);
                VBOS(1,p<<"2DFX["<<j<<"].y = "<<fx.y);
                VBOS(1,p<<"2DFX["<<j<<"].z = "<<fx.z);
                fx.type = ios_read_u32(f);
                VBOS(1,p<<"2DFX["<<j<<"].type = "<<fx.type);
                fx.sz = ios_read_u32(f);
                counter += fx.sz + 20;
                VBOS(1,p<<"2DFX["<<j<<"].sz = "<<fx.sz);
                switch (fx.type) {
                    case TWODFX_LIGHT:
                        fx.light.r = ios_read_u8(f);
                        VBOS(1,p<<"2DFX["<<j<<"].light.r = "<<(int)fx.light.r);
                        fx.light.g = ios_read_u8(f);
                        VBOS(1,p<<"2DFX["<<j<<"].light.g = "<<(int)fx.light.g);
                        fx.light.b = ios_read_u8(f);
                        VBOS(1,p<<"2DFX["<<j<<"].light.b = "<<(int)fx.light.b);
                        fx.light.x = ios_read_u8(f);
                        VBOS(1,p<<"2DFX["<<j<<"].light.x = "<<(int)fx.light.x);
                        fx.light.draw_distance = ios_read_float(f);
                        VBOS(1,p<<"2DFX["<<j<<"].light.draw_distance = "<<fx.light.draw_distance);
                        fx.light.outer_range = ios_read_float(f);
                        VBOS(1,p<<"2DFX["<<j<<"].light.outer_range = "<<fx.light.outer_range);
                        fx.light.size = ios_read_float(f);
                        VBOS(1,p<<"2DFX["<<j<<"].light.size = "<<fx.light.size);
                        fx.light.inner_range = ios_read_float(f);
                        VBOS(1,p<<"2DFX["<<j<<"].light.inner_range = "<<fx.light.inner_range);
                        for (int pi=0 ; pi<5; ++pi) {
                            fx.light.params[pi] = ios_read_u8(f);
                            VBOS(1,p<<"2DFX["<<j<<"].light.params["<<pi<<"] = "<<(int)fx.light.params[pi]);
                        }
                        fx.light.corona_name = ios_read_fixedstr(f,24);
                        VBOS(1,p<<"2DFX["<<j<<"].light.corona_name = "<<fx.light.corona_name);
                        fx.light.shadow_name = ios_read_fixedstr(f,24);
                        VBOS(1,p<<"2DFX["<<j<<"].light.shadow_name = "<<fx.light.shadow_name);
                        fx.light.flare = ios_read_s32(f);
                        VBOS(1,p<<"2DFX["<<j<<"].light.flare = "<<fx.light.flare);
                        for (int pi=0 ; pi<3; ++pi) {
                            fx.light.flare_params[pi] = ios_read_u8(f);
                            VBOS(1,p<<"2DFX["<<j<<"].light.flare_params["<<pi<<"] = "<<(int)fx.light.flare_params[pi]);
                        }
                    break;
                    case TWODFX_PFX:
                        fx.pfx.particle_name = ios_read_fixedstr(f,24);
                        VBOS(1,p<<"2DFX["<<j<<"].pfx.particle_name = "<<fx.pfx.particle_name);
                    break;
                    case TWODFX_PED:
                    case TWODFX_UNK1:
                    case TWODFX_SIGN:
                    case TWODFX_SLOTMACHINE:
                    case TWODFX_UNK2:
                    case TWODFX_ESCALATOR:
                    VBOS(1,p<<"Skipping over 2DFX type: "<<fx.type<<" of size "<<DECHEX(fx.sz));
                    ios_read_byte_array(f,NULL,fx.sz);
                    break;
                    default:
                    GRIT_EXCEPT("Unknown 2DFX type!");
                };
            }
            APP_ASSERT(counter==size);
        } break;
        default:
            unrec(p,type,size,"GEOMETRY");
        }
    }

    check_spill(f,geometry_start,geometry_size,p);
}}}


void ios_read_dff (int d, std::ifstream &f, struct dff *c, const std::string &p,
                   const std::string &phys_mat_pref, MaterialMap &db)
{{{
    unsigned long type, file_version, dff_size;
    ios_read_header(f,&type,&dff_size,&file_version,NULL);
    VBOS(3,p<<"file_version: "<<DECHEX(file_version));
    unsigned long size;
    if (type==RW_UV_ANIMATION_DICTIONARY) {
        size = dff_size;
        VBOS(1,p<<"SKIPPING_OVER_UV_ANIM_DICT: "<<DECHEX(size));
        ios_read_byte_array(f,NULL,size); // TODO
        ios_read_header(f,&type,&dff_size,NULL,&file_version);
    }
    std::streamoff dff_start = f.tellg();

    APP_ASSERT(type==RW_CHUNK);

    ios_read_header(f,&type,&size,NULL,&file_version);
    APP_ASSERT(type==RW_DATA);
    APP_ASSERT(size==12);
    unsigned long num_objects = ios_read_u32(f);
    VBOS(3,p<<"num_objects: "<<num_objects);
    unsigned long num_lights = ios_read_u32(f);
    VBOS(3,p<<"num_lights: "<<num_lights);
    unsigned long unk2 = ios_read_u32(f);
    APP_ASSERT(unk2==0);

    // frame list
    ios_read_header(f,&type,&size,NULL,&file_version);
    APP_ASSERT(type==RW_FRAME_LIST);
    ios_read_header(f,&type,&size,NULL,&file_version);
    APP_ASSERT(type==RW_DATA);
    unsigned long num_frames = ios_read_u32(f);
    c->frames.resize(num_frames);
    VBOS(3,p<<"num_frames: "<<num_frames);
    for (unsigned long i=0 ; i<num_frames ; i++) {
        std::stringstream prefss;
        prefss<<p<<"frame["<<i<<"].";
        std::string pref = prefss.str();
        for (int j=0 ; j<9 ; j++) {
            float v = ios_read_float(f);
            c->frames[i].rot[j] = v;
            VBOS(4,pref<<"rot["<<j<<"]: "<<v);
        }
        c->frames[i].x = ios_read_float(f);
        VBOS(3,pref<<"x: "<<c->frames[i].x);
        c->frames[i].y = ios_read_float(f);
        VBOS(3,pref<<"y: "<<c->frames[i].y);
        c->frames[i].z = ios_read_float(f);
        VBOS(3,pref<<"z: "<<c->frames[i].z);
        long parent = ios_read_s32(f);
        c->frames[i].parent_frame = parent;
        VBOS(3,pref<<"parent_frame: "<<parent);
        if (parent==-1) {
            c->root_frame = i;
        } else {
            c->frames[parent].children.push_back(i);
        }
        c->frames[i].unk = ios_read_u32(f);
        VBOS(3,pref<<"unk: "<<c->frames[i].unk);
    }

    for (unsigned long i=0 ; i<num_frames ; i++) {
        std::stringstream prefss;
        prefss<<p<<"frame["<<i<<"].";
        std::string pref = prefss.str();
        c->frames[i].geometry = -1;
        unsigned long total_size;
        ios_read_header(f,&type,&total_size,NULL,&file_version);
        APP_ASSERT(type==RW_EXT);
        while (total_size>0) {
            ios_read_header(f,&type,&size,NULL,&file_version);
            total_size -= size + 12;
            switch(type) {
            case RW_BONE:
                c->frames[i].bone_unk_flags = ios_read_u32(f);
                APP_ASSERT(c->frames[i].bone_unk_flags==0x100);
                c->frames[i].bone_id = ios_read_u32(f);
                VBOS(3,pref<<"bone_id: "<<c->frames[i].bone_id);
                c->frames[i].num_bones = ios_read_u32(f);
                VBOS(3,pref<<"num_bones: "<<c->frames[i].num_bones);
                c->frames[i].bone_ids.resize
                    (c->frames[i].num_bones);
                c->frames[i].bone_nums.resize
                    (c->frames[i].num_bones);
                c->frames[i].bone_types.resize
                    (c->frames[i].num_bones);
                if (c->frames[i].num_bones) {
                    c->frames[i].bone_unk1 =ios_read_u32(f);
                    VBOS(3,pref<<"bone_unk1: "<<c->frames[i].bone_unk1);
                    c->frames[i].bone_unk2 =ios_read_u32(f);
                    VBOS(3,pref<<"bone_unk2: "<<c->frames[i].bone_unk2);
                }
                for (unsigned long j=0 ;j<c->frames[i].num_bones; j++) {
                    c->frames[i].bone_ids[j] = ios_read_u32(f);
                    VBOS(3,pref<<"bone["<<j<<"].id: "
                               <<c->frames[i].bone_ids[j]);
                    c->frames[i].bone_nums[j] =ios_read_u32(f);
                    VBOS(3,pref<<"bone["<<j<<"].num: "
                               <<c->frames[i].bone_nums[j]);
                    c->frames[i].bone_types[j]=ios_read_u32(f);
                    VBOS(3,pref<<"bone["<<j<<"].type: "
                               <<c->frames[i].bone_types[j]);
                }
                break;
            case RW_FRAME: {
                c->frames[i].name = ios_read_fixedstr(f,size);
                break;
            } default:
                unrec(p,type,size,"FRAME");
            }
        }
        VBOS(3,pref<<"name: "<<c->frames[i].name);
    }

    // frame list
    ios_read_header(f,&type,&size,NULL,&file_version);
    APP_ASSERT(type==RW_GEOMETRY_LIST);
    ios_read_header(f,&type,&size,NULL,&file_version);
    APP_ASSERT(type==RW_DATA);
    unsigned long num_geometries = ios_read_u32(f);
    c->geometries.resize(num_geometries);
    VBOS(3,p<<"num_geometries: "<<num_geometries);
    for (unsigned long i=0 ; i<num_geometries ; i++) {
        std::stringstream prefss;
        prefss<<p<<"geometry["<<i<<"].";
        std::string pref = prefss.str();
        ios_read_geometry(d,f,file_version,c->geometries[i],pref);
    }

    c->objects.resize(num_objects);
    for (unsigned long i=0 ; i<num_objects ; i++) {
        std::stringstream prefss;
        prefss<<p<<"object["<<i<<"].";
        std::string pref = prefss.str();
        ios_read_header(f,&type,&size,NULL,&file_version);
        APP_ASSERT(type==RW_ATOMIC);
        ios_read_header(f,&type,&size,NULL,&file_version);
        APP_ASSERT(type==RW_DATA);
        APP_ASSERT(size==16);
        unsigned long frame_index = ios_read_u32(f);
        c->objects[i].frame_index = frame_index;
        VBOS(3,pref<<"frame_index: "<<frame_index);
        long geometry_index = ios_read_s32(f);
        c->objects[i].geometry_index = geometry_index;
        VBOS(3,pref<<"geometry_index: "<<geometry_index);
        unsigned long five = ios_read_u32(f);
        APP_ASSERT(five==5);
        unsigned long zero = ios_read_u32(f);
        APP_ASSERT(zero==0);

        APP_ASSERT(c->frames[frame_index].geometry==-1);
        c->frames[frame_index].geometry = geometry_index;

        APP_ASSERT(c->geometries[geometry_index].frame==-1);
        c->geometries[geometry_index].frame = (long)frame_index;

        unsigned long total_size;
        ios_read_header(f,&type,&total_size,NULL,&file_version);
        APP_ASSERT(type==RW_EXT);
        while (total_size>0) {
            ios_read_header(f,&type,&size,NULL,&file_version);
            total_size -= size + 12;
            switch(type) {
            case RW_RIGHT_TO_RENDER:
            APP_ASSERT(size==8);
            c->objects[i].rtr_unk1 = ios_read_u32(f);
            VBOS(3,pref<<"rtr_unk1: "<<DECHEX(c->objects[i].rtr_unk1));
            c->objects[i].rtr_unk2 = ios_read_u32(f);
            VBOS(3,pref<<"rtr_unk2: "<<DECHEX(c->objects[i].rtr_unk2));
            break;

            case RW_MATERIAL_EFFECTS: {
                APP_ASSERT(size==4);
                unsigned long eff = ios_read_u32(f);
                APP_ASSERT(eff==1);
            } break;

            case RW_UNKNOWN:
            APP_ASSERT(size==4);
            c->unk = ios_read_u32(f);
            VBOS(3,pref<<"obj_ext_unk: "<<c->unk);
            break;

            default:
                unrec(p,type,size,"ATOMIC");
            }
        }
    }

    // gos_town, racespot.dff has num_lights==0 but 10 light sections here so
    // ignore this header value
    //c->lights.resize(num_lights);
    unsigned long totalsize; // used after the loop
    int counter = 0;
    while (true) {
        light2 light;
        std::stringstream prefss;
        prefss<<p<<"light["<<counter<<"].";
        std::string pref = prefss.str();
        ios_read_header(f,&type,&size,NULL,&file_version);
        if (type==RW_EXT) {
            totalsize = size;
            break;
        }
        APP_ASSERT(type==RW_DATA);
        APP_ASSERT(size==4);
        unsigned long num = ios_read_u32(f);
        (void)num;
        //APP_ASSERT(num==counter+1);

        ios_read_header(f,&type,&size,NULL,&file_version);
        APP_ASSERT(type==RW_LIGHT);
        APP_ASSERT(size==48);

        ios_read_header(f,&type,&size,NULL,&file_version);
        APP_ASSERT(type==RW_DATA);
        APP_ASSERT(size==24);
        light.unk1 = ios_read_float(f);
        VBOS(3,pref<<"unk1: "<<light.unk1);
        light.unk3 = ios_read_float(f);
        VBOS(3,pref<<"unk2: "<<light.unk2);
        light.unk3 = ios_read_float(f);
        VBOS(3,pref<<"unk3: "<<light.unk3);
        light.unk4 = ios_read_float(f);
        VBOS(3,pref<<"unk4: "<<light.unk4);
        light.unk5 = ios_read_float(f);
        VBOS(3,pref<<"unk5: "<<light.unk5);
        light.flags = ios_read_u32(f);
        VBOS(3,pref<<"flags: 0x"<<std::hex<<light.flags<<std::dec);

        ios_read_header(f,&type,&size,NULL,&file_version);
        APP_ASSERT(type==RW_EXT);
        APP_ASSERT(size==0);

        c->lights.push_back(light);
        counter++;
    }

    c->has_tcol = false;

    while (totalsize>0) {
        unsigned long size;
        ios_read_header(f,&type,&size,NULL,&file_version);
        totalsize -= size + 12;
        switch (type) {
        case RW_COLLISION: {
            VBOS(1,p<<"Reading col data... "<<DECHEX(size));
            // rccam seems to have broken col, this is a crude heuristic to catch it
            if (size==160) { 
                unsigned char *data = new unsigned char[size];
                ios_read_byte_array(f,data,size); //TODO
                //std::ofstream f;
                //f.open("tmp.bin", std::ios::binary);
                //f.write((char*)data, size);
                delete [] data;
            } else {
                parse_col(c->col_name, f, c->tcol, phys_mat_pref, db, d-1);
                c->has_tcol = true;
            }
        } break;
        default:
            unrec(p,type,size,"MODEL");
        }
    }

    check_spill(f,dff_start,dff_size,p);

    //everything has just one clump except clothing which seems to have 3...
    //unsigned char byte = ios_read_u8(f);
    //APP_ASSERT(byte==0);

}}}

void offset_dff (dff &dff, float x, float y, float z)
{{{
    for (unsigned long j=0 ; j<dff.frames.size() ; ++j) {
        frame &fr = dff.frames[j];
        fr.x += x;
        fr.y += y;
        fr.z += z;
    }
/* offsetting the frames is obviously enough
    for (unsigned long i=0 ; i<dff.geometries.size() ; i++) {
        geometry &g = dff.geometries[i];
        std::vector<vect> &vs = g.vertexes;
        for (unsigned long j=0 ; j<vs.size() ; ++j) {
            vect &v = vs[j];
            v.x += x;
            v.y += y;
            v.z += z;
        }
    }
*/
    if (dff.has_tcol) {
        tcol_offset(dff.tcol, x, y, z);
    }
}}}

static float &lu(float (&rot)[9], int x, int y)
{
    return rot[x*3+y];
}
void reset_dff_frame (dff &dff, unsigned frame_id)
{
    APP_ASSERT(frame_id<dff.frames.size());
    frame &fr = dff.frames[frame_id];

    geometry &g = dff.geometries[fr.geometry];
    std::vector<vect> &vs = g.vertexes;
    for (unsigned long j=0 ; j<vs.size() ; ++j) {
        vect &v = vs[j];
        float x_ = lu(fr.rot,0,0)*v.x + lu(fr.rot,1,0)*v.y + lu(fr.rot,2,0)*v.z;
        float y_ = lu(fr.rot,0,1)*v.x + lu(fr.rot,1,1)*v.y + lu(fr.rot,2,1)*v.z;
        float z_ = lu(fr.rot,0,2)*v.x + lu(fr.rot,1,2)*v.y + lu(fr.rot,2,2)*v.z;
        v.x = x_ + fr.x;
        v.y = y_ + fr.y;
        v.z = z_ + fr.z;
    }

    for (unsigned long j=0 ; j<fr.children.size() ; ++j) {
        APP_ASSERT(j<dff.frames.size());
        frame &frc = dff.frames[frame_id];
        float x_ = lu(fr.rot,0,0)*frc.x + lu(fr.rot,1,0)*frc.y + lu(fr.rot,2,0)*frc.x;
        float y_ = lu(fr.rot,0,1)*frc.x + lu(fr.rot,1,1)*frc.y + lu(fr.rot,2,1)*frc.z;
        float z_ = lu(fr.rot,0,2)*frc.x + lu(fr.rot,1,2)*frc.y + lu(fr.rot,2,2)*frc.z;
        frc.x = x_ + fr.x;
        frc.y = y_ + fr.y;
        frc.z = z_ + fr.z;
        // TODO: matrix multiplication
        float rot_[9] = {0};
        for (int i=0 ; i<9 ; ++i) {
            frc.rot[i] = rot_[i];
        }
    }
    
    fr.x = 0;
    fr.y = 0;
    fr.z = 0;
    for (int i=0 ; i<9 ; ++i) fr.rot[i] = 0;
}

////////////////////////////////////////////////////////////////////////////////
// OUTPUT OF MESH AND MATERIALS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ind (std::ostream &out, unsigned int level)
{
    for (unsigned int i=0 ; i<level ; i++)
        out << "  ";
}


#define ARRLEN(a) (sizeof a/sizeof *a)

std::string get_tex_name (const std::string &img,
                          const std::string &txd,
                          std::string tex_name)
{{{
    strlower(tex_name);
    tex_name += ".dds";

    if (txd!="") tex_name = txd + ".txd" + "/" + tex_name;
    if (img!="") tex_name = img + "/" + tex_name;
    //if (mod_name!="") tex_name = mod_name + "/" + tex_name;

    // if it's a duplicate of another texture, use the other texture instead
    tex_name = tex_dup(tex_name);

    return tex_name;
}}}

bool operator< (const existing_material &a, const existing_material &b)
{
    if (a.texs > b.texs) return false;
    if (a.texs < b.texs) return true;
    if (a.alpha_blend > b.alpha_blend) return false;
    if (a.alpha_blend < b.alpha_blend) return true;
    if (a.no_alpha_reject > b.no_alpha_reject) return false;
    if (a.no_alpha_reject < b.no_alpha_reject) return true;
    if (a.double_sided > b.double_sided) return false;
    if (a.double_sided < b.double_sided) return true;
    if (a.dynamic_lighting < b.dynamic_lighting) return true;
    if (a.dynamic_lighting > b.dynamic_lighting) return false;
    if (a.r > b.r) return false;
    if (a.r < b.r) return true;
    if (a.g > b.g) return false;
    if (a.g < b.g) return true;
    if (a.b > b.b) return false;
    if (a.b < b.b) return true;
    if (a.a > b.a) return false;
    if (a.a < b.a) return true;
    return false;
}

const std::string &already_seen (MatDB &matdb,
                                 float r,float g,float b,float a,
                                 const Strings& texs,
                                 bool alpha_blend, bool no_alpha_reject,
                                 bool double_sided, bool dynamic_lighting,
                                 const std::string& new_name)
{{{
    struct existing_material m =
       {r,g,b,a,texs,alpha_blend,no_alpha_reject,double_sided,dynamic_lighting};
    std::string &existing = matdb[m];
    if (existing=="") {
        existing = new_name;
        //std::cout<<"registering: "<<new_name<<std::endl;
        //std::cout<<r<<" "<<g<<" "<<b<<" "<<alpha_blend<<" "<<double_sided<<std::endl;
        //for (unsigned long i=0 ; i<texs.size() ; i++) {
        //    std::cout<<texs[i]<<std::endl;
        //}
    } else {
        //std::cout<<"\""<<new_name<<"\" is the same as \""
        //     <<existing<<"\""<<std::endl;
    }
    return existing;
}}}

// return the given txd and recursively all parents (if any)
static std::vector<std::string> search_txds(bool is_car,
                                            const std::string &txd,
                                            const ide &ide)
{
    std::vector<std::string> r;
    r.push_back(txd);
    for (size_t j=0 ; j<ide.txdps.size() ; ++j) {
        const TXDP &txdp = ide.txdps[j]; 
        if (txdp.txd1 == txd) {
            std::vector<std::string> sub = search_txds(is_car, txdp.txd2, ide);
            r.insert(r.end(), sub.begin(), sub.end());
        }
    }
    return r;
}

static std::string to_string(const std::vector<std::string> &strs)
{
    std::stringstream ss;
    ss << "[";
    for (size_t j=0 ; j<strs.size() ; ++j) {
        ss << (j>0?", ":"") << strs[j];
    }
    ss << "]";
    return ss.str();
}

static const std::string &
export_or_provide_mat (const StringSet &texs,
                       const ide &ide,
                       const std::vector<std::string> &imgs,
                       geometry &g,
                       int mindex,
                       const Obj &obj,
                       const std::string &oname,
                       MatDB &matdb,
                       std::ostream &lua_file)
{{{

    material &m = g.materials[mindex];

    bool has_alpha = false; // old method(stupid): obj.flags & OBJ_FLAG_ALPHA1;
    bool decal = (obj.flags & OBJ_FLAG_ALPHA1) && (obj.flags & OBJ_FLAG_NO_SHADOW);
    bool double_sided = 0 != (obj.flags & OBJ_FLAG_DRAW_BACKFACE);
    bool dynamic_lighting = g.normals.size()>0;

    float R = ((m.colour >> 0) & 0xFF) / 255.0f;
    float G = ((m.colour >> 8) & 0xFF) / 255.0f;
    float B = ((m.colour >> 16) & 0xFF) / 255.0f;
    float A = ((m.colour >> 24) & 0xFF) / 255.0f;

    Strings textures;

    APP_ASSERT(m.num_textures==1  || m.num_textures==0);
    for (unsigned int i=0 ; i<m.num_textures ; i++) {
        std::vector<std::string> txds = search_txds(obj.is_car, obj.txd, ide);
        bool found = false;
        std::string tex_name;
        for (size_t j=0 ; j<txds.size() ; ++j) {
            const std::string &txd = txds[j];
            for (size_t k=0 ; k<imgs.size() ; ++k) {
                const std::string &img = imgs[k];
                tex_name = get_tex_name(img, txd, m.textures[i].name);
                if (texs.empty() || texs.find(tex_name)!=texs.end()) {
                    found = true;
                    goto done;
                }
            }
        }
        if (obj.is_car) {
            tex_name = get_tex_name("", "../generic/vehicle", m.textures[i].name);
            if (texs.empty() || texs.find(tex_name)!=texs.end()) {
                found = true;
                goto done;
            }
        }
        done:
        if (!found) {
            std::cerr<<"For "<<oname<<" "<<imgs[0]<<"/"<<obj.dff<<".dff "
                     <<"couldn't find tex: \""<<m.textures[i].name
                     <<"\" in "<<to_string(txds)<<std::endl;
            tex_name = "";
        }
        textures.push_back(tex_name);
        if (m.textures[i].has_alpha) has_alpha = true;
    }           

    if (textures.size()==0)
        textures.push_back("");

    m.rewrittenTextures = textures;

    std::ostringstream mname;
    mname<<oname<<":"<<mindex;

    const std::string &mat = already_seen(matdb, R,G,B,A, textures, has_alpha,
                                          decal, double_sided,
                                          dynamic_lighting, mname.str());

    if (mat!=mname.str())
        return mat;

    
    lua_file << "material `" << mname.str() << "` { ";
    if (g.vertex_cols.size() > 0) {
            lua_file << "vertexDiffuse=true, ";
            lua_file << "vertexAlpha=true, ";
    }
    if (has_alpha || decal) lua_file << "alpha=true, ";
    if (has_alpha && !decal) lua_file << "depthWrite=true, ";
    if (decal) {
        lua_file << "castShadows=false, alphaReject=0, ";
    } else {
        lua_file << "alphaReject=0.25, ";
    }
    if (double_sided || obj.is_car) lua_file << "backfaces=true, ";
    if (!dynamic_lighting) lua_file << "normals=false, ";

    if (m.colour!=0xFFFFFFFF) {
        if ((m.colour|0xff000000)==0xff00ff3c && obj.is_car) {
            // colour 1
            m.colour=0xFFFFFFFF;
            lua_file<<"paintColour=1, ";
        } else if ((m.colour|0xff000000)==0xffaf00ff && obj.is_car) {
            // colour 2
            m.colour=0xFFFFFFFF;
            lua_file<<"paintColour=2, ";
        } else if ((m.colour|0xff000000)==0xffffff00 && obj.is_car) {
            // colour 3
            m.colour=0xFFFFFFFF;
            lua_file<<"paintColour=3, ";
        } else if ((m.colour|0xff000000)==0xffff00ff && obj.is_car) {
            // colour 4
            m.colour=0xFFFFFFFF;
            lua_file<<"paintColour=4, ";
        } else if ((m.colour|0xff000000)==0xff00afff && obj.is_car) {
            // left headlight
            m.colour=0xFFFFFFFF;
        } else if ((m.colour|0xff000000)==0xffc8ff00 && obj.is_car) {
            // right headlight
            m.colour=0xFFFFFFFF;
        } else if ((m.colour|0xff000000)==0xff00ffb9 && obj.is_car) {
            // left rearlight
            m.colour=0xFFFFFFFF;
        } else if ((m.colour|0xff000000)==0xff003cff && obj.is_car) {
            // right rearlight
            m.colour=0xFFFFFFFF;
        } else {
            lua_file<<"diffuseColour=0x"<<std::hex<<m.colour<<std::dec<<", ";
        }
    }
    if (textures[0]!="") lua_file << "diffuseMap=`"<<textures[0]<<"`, ";
    lua_file << "}\n";

    return mat;
}}}

vect operator+= (vect &a, const vect &b)
{ a.x += b.x; a.y += b.y; a.z += b.z; return a; }

vect operator- (const vect &a, const vect &b)
{ return vect(a.x-b.x, a.y-b.y, a.z-b.z); }

vect operator/ (const vect &a, float b)
{ return vect(a.x/b, a.y/b, a.z/b); }

float len (const vect &a)
{ return ::sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }

vect normalise (const vect &a)
{ return a / len(a); }

vect cross_prod (const vect &a, const vect &b)
{ return vect(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x); }

void add_face_normal (vect &N, const vect &v1, const vect &v2, const vect &v3)
{
    N += cross_prod (v2-v1, v3-v1);
}

void generate_normals (struct geometry &g)
{
    const std::vector<vect> &positions = g.vertexes;
    std::vector<vect> &normals = g.normals;
    if (normals.size() > 0) {
        APP_ASSERT(positions.size() == normals.size());
        // already has normals
        return;
    }
    normals.resize(positions.size()); // initialises to zero
    for (MatSplits::iterator s=g.mat_spls.begin(),s_=g.mat_spls.end() ; s!=s_ ; ++s) {
        const std::vector<unsigned long> &mindexes = s->indexes2;
        APP_ASSERT(mindexes.size() % 3 == 0);
        for (size_t f=0 ; f<mindexes.size() ; f+=3) {
            unsigned long index1=mindexes[f+0], index2=mindexes[f+1], index3=mindexes[f+2];
            add_face_normal(normals[index1], positions[index1],positions[index2],positions[index3]);
            add_face_normal(normals[index2], positions[index1],positions[index2],positions[index3]);
            add_face_normal(normals[index3], positions[index1],positions[index2],positions[index3]);
        }
    }
    for (size_t i=0 ; i<normals.size() ; ++i) {
        normals[i] = normalise(normals[i]);
    }
}

void export_xml (const StringSet &texs,
                 const ide &ide,
                 const std::vector<std::string> &imgs,
                 std::ostream &out,
                 const std::string &fname,
                 const Obj &obj,
                 const std::string &oname,
                 struct geometry &g,
                 MatDB &matdb,
                 std::ostream &lua_file)
{{{
    out << "xml filename: " << fname << std::endl;

    std::ofstream f;
    f.open(fname.c_str(), std::ios::binary);
    APP_ASSERT_IO_SUCCESSFUL(f, "opening "+fname);


    ind(f,0);f<<"<mesh>\n";
    ind(f,1);f<<"<sharedgeometry vertexcount=\""<<g.num_vertexes<<"\">\n";
    int positions = g.vertexes.size();
    int normals = g.normals.size();
    int colours_diffuse = g.vertex_cols.size();
    int texture_coords = g.tex_coords.size() ? 1 : 0;
    int texture_coords2 = g.tex_coords2.size() ? 1 : 0;
    APP_ASSERT(obj.id==14825 || obj.id==18009 || obj.id==18036 || !texture_coords || !texture_coords2);
    APP_ASSERT(!texture_coords2 || texture_coords);
    ind(f,2);f<<"<vertexbuffer positions=\""
              <<(positions?"true":"false")<<"\" "
              <<"normals=\""<<(normals?"true":"false")<<"\" "
              <<"colours_diffuse=\""<<(colours_diffuse?"true":"false")<<"\" "
              <<"texture_coords=\""<<(texture_coords+texture_coords2)<<"\" "
              <<"texture_coords_dimensions_0=\"2\">\n";

    for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
        ind(f,3);f<<"<vertex> <!--"<<i<<"-->\n";
        if (positions) {
            struct vect *v = &g.vertexes[i];
            float x = v->x, y = v->y, z = v->z;
            ind(f,4);
            f<<"<position x=\""<<x<<"\" y=\""<<y<<"\" z=\""<<z<<"\"/>\n";
        }
        if (normals) {
            struct vect *v = &g.normals[i];
            float x = v->x, y = v->y, z = v->z;
            ind(f,4);f<<"<normal x=\""<<x<<"\" y=\""<<y<<"\" z=\""<<z<<"\"/>\n";
        }
        if (colours_diffuse) {
            float r = ((g.vertex_cols[i] >> 0) & 0xFF) / 255.0f;
            float gr =((g.vertex_cols[i] >> 8) & 0xFF) / 255.0f;
            float b = ((g.vertex_cols[i] >> 16) & 0xFF) / 255.0f;
            float a = ((g.vertex_cols[i] >> 24) & 0xFF) / 255.0f;
            ind(f,4);f<<"<colour_diffuse value="
                       "\""<<r<<" "<<gr<<" "<<b<<" "<<a<<"\"/>\n";
        }
        if (texture_coords) {
            struct texcoord *uv = &g.tex_coords[i];
            ind(f,4);f<<"<texcoord u=\""<<uv->u<<"\" v=\""<<uv->v<<"\"/>\n";
        }
        if (texture_coords2) {
            struct texcoord *uv = &g.tex_coords2[i];
            ind(f,4);f<<"<texcoord u=\""<<uv->u<<"\" v=\""<<uv->v<<"\"/>\n";
        }
        ind(f,3);f<<"</vertex>\n";
    }
    ind(f,2);f<<"</vertexbuffer>\n";
    ind(f,1);f<<"</sharedgeometry>\n";

    ind(f,1);f<<"<submeshes>\n";
    for (MatSplits::iterator s=g.mat_spls.begin(),s_=g.mat_spls.end() ;
         s!=s_ ; s++) {

        int tris = s->indexes2.size() / 3;

        if (tris==0) {
            //out<<"Empty submesh: \""<<oname<<"/"<<s->material<<"\"\n";
            continue;
        }

        std::string mname = export_or_provide_mat(texs,ide,imgs,g,s->material,
                                                  obj,oname,matdb,lua_file);

        s->surrogate = mname;

        ind(f,2);f<<"<submesh material=\""<<mname<<"\" "
                  <<"usesharedvertices=\"true\" "
                  <<"operationtype=\"triangle_list\">\n";
        ind(f,3);f<<"<faces count=\""<<tris<<"\">\n";

        for (int tri=0 ; tri<tris ; ++tri) {
            unsigned long v1 = s->indexes2[3*tri + 0];
            unsigned long v2 = s->indexes2[3*tri + 1];
            unsigned long v3 = s->indexes2[3*tri + 2];
            APP_ASSERT(v1 < g.vertexes.size());
            APP_ASSERT(v2 < g.vertexes.size());
            APP_ASSERT(v3 < g.vertexes.size());
            f<<"<face v1=\""<<v1<<"\" "
                    "v2=\""<<v2<<"\" "
                    "v3=\""<<v3<<"\"/>\n";
        }
        ind(f,3);f<<"</faces>\n";

        ind(f,2);f<<"</submesh>\n";

    }
    ind(f,1);f<<"</submeshes>\n";
    ind(f,0);f<<"</mesh>\n";
}}}


Ogre::LogManager *lmgr;
Ogre::ResourceGroupManager *rgmgr;
Ogre::MeshManager *mmgr;
Ogre::HardwareBufferManager *hwbmgr;
Ogre::LodStrategyManager *lodsm;
Ogre::MeshSerializer *serialiser;

void init_ogre (void)
{
    lmgr = new Ogre::LogManager();
    lmgr->createLog("Ogre.log",true,false,false); // log is disabled for now
    lmgr->setLogDetail(Ogre::LL_LOW);

    rgmgr = new Ogre::ResourceGroupManager();
    mmgr = new Ogre::MeshManager();
    hwbmgr = new Ogre::DefaultHardwareBufferManager();

    lodsm = new Ogre::LodStrategyManager();

    serialiser = new Ogre::MeshSerializer();
}

void export_mesh (const StringSet &texs,
                  const ide &ide,
                  const std::vector<std::string> &imgs,
                  std::ostream &out,
                  const std::string &fname,
                  const Obj &obj,
                 const std::string &oname,
                  struct geometry &g,
                  MatDB &matdb,
                  std::ostream &lua_file)
{{{
    (void) out;
    //out << "mesh filename: " << fname << std::endl;

    Ogre::MeshPtr mesh = mmgr->createManual(fname, "General");

    // FIRST - SHARED GEOMETRY

    mesh->sharedVertexData = new Ogre::VertexData();
    Ogre::VertexDeclaration* decl = mesh->sharedVertexData->vertexDeclaration;

    int positions = g.vertexes.size();
    int normals = g.normals.size();
    int day_colours = g.vertex_cols.size();
    int night_colours = g.vertex_night_cols.size();
    int texture_coords = g.tex_coords.size();
    int texture_coords2 = g.tex_coords2.size();

    if (!day_colours) night_colours = 0;

    APP_ASSERT(positions>0);
    APP_ASSERT(!normals || normals==positions);
    APP_ASSERT(!day_colours || day_colours==positions);
    APP_ASSERT(!night_colours || night_colours==positions);
    APP_ASSERT(!texture_coords || texture_coords==positions);

    size_t offset = 0;
    if (positions) {
        decl->addElement(0,offset,Ogre::VET_FLOAT3,Ogre::VES_POSITION);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    }

    if (normals) {
        decl->addElement(0,offset,Ogre::VET_FLOAT3,Ogre::VES_NORMAL);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    }

    if (day_colours) {
        decl->addElement(0,offset,Ogre::VET_FLOAT4,Ogre::VES_DIFFUSE,0);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    }

    if (texture_coords) {
        decl->addElement(0,offset,Ogre::VET_FLOAT2,Ogre::VES_TEXTURE_COORDINATES,0);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
    }

    if (texture_coords2) {
        decl->addElement(0,offset,Ogre::VET_FLOAT2,Ogre::VES_TEXTURE_COORDINATES,0);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
    }

    if (night_colours) {
        decl->addElement(0,offset,Ogre::VET_FLOAT4,Ogre::VES_TEXTURE_COORDINATES,1);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    }


    float *vbuf = new float[g.num_vertexes*offset/sizeof(float)];
    float *vbuf_next = vbuf;

    Ogre::Real min_x=0, min_y=0, min_z=0, max_x=0, max_y=0, max_z=0;
    Ogre::Real rad2=0;

    for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
        if (positions) {
            struct vect *v = &g.vertexes[i];
            float x = v->x, y = v->y, z = v->z;
            min_x = std::min(min_x,x);
            min_y = std::min(min_y,y);
            min_z = std::min(min_z,z);
            max_x = std::max(max_x,x);
            max_y = std::max(max_y,y);
            max_z = std::max(max_z,z);
            rad2 = std::max(rad2,x*x+y*y+z*z);
            (*vbuf_next++) = x;
            (*vbuf_next++) = y;
            (*vbuf_next++) = z;
        }
        if (normals) {
            struct vect *v = &g.normals[i];
            float x = v->x, y = v->y, z = v->z;
            (*vbuf_next++) = x;
            (*vbuf_next++) = y;
            (*vbuf_next++) = z;
        }
        if (day_colours) {
            float r = ((g.vertex_cols[i] >> 0) & 0xFF) / 255.0f;
            float gr =((g.vertex_cols[i] >> 8) & 0xFF) / 255.0f;
            float b = ((g.vertex_cols[i] >> 16) & 0xFF) / 255.0f;
            float a = ((g.vertex_cols[i] >> 24) & 0xFF) / 255.0f;
            (*vbuf_next++) = r;
            (*vbuf_next++) = gr;
            (*vbuf_next++) = b;
            (*vbuf_next++) = a;
        }
        if (texture_coords) {
            struct texcoord &uv = g.tex_coords[i];
            float u = uv.u, v = uv.v;
            (*vbuf_next++) = u;
            (*vbuf_next++) = v;
        }
        if (texture_coords2) {
            struct texcoord &uv = g.tex_coords2[i];
            float u = uv.u, v = uv.v;
            (*vbuf_next++) = u;
            (*vbuf_next++) = v;
        }
        if (night_colours) {
            float r = ((g.vertex_night_cols[i] >> 0) & 0xFF) / 255.0f;
            float gr =((g.vertex_night_cols[i] >> 8) & 0xFF) / 255.0f;
            float b = ((g.vertex_night_cols[i] >> 16) & 0xFF) / 255.0f;
            float a = ((g.vertex_night_cols[i] >> 24) & 0xFF) / 255.0f;
            (*vbuf_next++) = r;
            (*vbuf_next++) = gr;
            (*vbuf_next++) = b;
            (*vbuf_next++) = a;
        }
    }
    mesh->sharedVertexData->vertexCount = g.num_vertexes;

    Ogre::HardwareVertexBufferSharedPtr hwvbuf = hwbmgr
            ->createVertexBuffer(
                    offset,
                    mesh->sharedVertexData->vertexCount,
                    Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    hwvbuf->writeData(0, hwvbuf->getSizeInBytes(), vbuf, true);

    mesh->sharedVertexData->vertexBufferBinding->setBinding(0, hwvbuf);


    MatSplits &ms = g.mat_spls;
    for (MatSplits::iterator s=ms.begin(),s_=ms.end() ; s!=s_ ; s++) {

        int tris = s->indexes2.size() / 3;

        if (tris==0) {
            //out<<"Empty submesh: \""<<oname<<"/"<<s->material<<"\"\n";
            continue;
        }

        unsigned short *ibuf = new unsigned short[tris*3];
        unsigned short *ibuf_next = ibuf;

        std::string mname = export_or_provide_mat(texs,ide,imgs,g,s->material,
                                                  obj,oname,matdb,lua_file);

        s->surrogate = mname;

        for (int tri=0 ; tri<tris ; ++tri) {
            unsigned short v1 = (unsigned short) s->indexes2[3*tri + 0];
            unsigned short v2 = (unsigned short) s->indexes2[3*tri + 1];
            unsigned short v3 = (unsigned short) s->indexes2[3*tri + 2];
            APP_ASSERT(v1 < g.vertexes.size());
            APP_ASSERT(v2 < g.vertexes.size());
            APP_ASSERT(v3 < g.vertexes.size());
            *(ibuf_next++) = v1;
            *(ibuf_next++) = v2;
            *(ibuf_next++) = v3;
        }

        Ogre::HardwareIndexBufferSharedPtr hwibuf = hwbmgr
                ->createIndexBuffer(
                        Ogre::HardwareIndexBuffer::IT_16BIT,
                        tris*3,
                        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        hwibuf->writeData(0, hwibuf->getSizeInBytes(), ibuf, true);


        Ogre::SubMesh* submesh = mesh->createSubMesh();
        submesh->setMaterialName(mname);
        submesh->useSharedVertices = true;
        submesh->indexData->indexBuffer = hwibuf;
        submesh->indexData->indexCount = tris * 3;
        submesh->indexData->indexStart = 0;

        delete [] ibuf;
    }

    mesh->_setBounds(Ogre::AxisAlignedBox(min_x,min_y,min_z,max_x,max_y,max_z));
    mesh->_setBoundingSphereRadius(Ogre::Math::Sqrt(rad2));

    mesh->load();

    serialiser->exportMesh(&*mesh, fname);

    mmgr->remove(mesh);

    delete [] vbuf;

}}}


////////////////////////////////////////////////////////////////////////////////
// DFFREAD CMDLINE TOOL STUFF //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef _DFFREAD_EXEC

void print_frame_tree (int d, struct dff *c, const std::string &p,
                       int more, unsigned long node)
{
        const char *thing = more ? "├" : "└";
        unsigned long num_children = c->frames[node].children.size();
        const char *thing2 = num_children > 0 ? "┬" : "─";
        std::stringstream output;
        output<<p<<thing<<"─"<<thing2<<"─\""<<c->frames[node].name<<"\" ";
        output<<"(frame:"<<node<<") ";
        if (c->frames[node].geometry==-1) {
                output<<"([0;31mDUMMY![0m) ";
        } else {
                output<<"([0;1;37mgeometry:"<<c->frames[node].geometry<<"[0m) ";
        }
        output<<"("<<c->frames[node].x<<","<<c->frames[node].y<<","<<c->frames[node].z<<")";
        output<<" [ ";
        for (int i=0 ; i<9 ; ++i) {
            if (i%3==0 && i!=0) output << ", ";
            else if (i!=0) output << ",";
            output<<c->frames[node].rot[i];
        }
        output<<" ]";
        VBOS(3,output.str());

        for (unsigned long i=0 ; i<num_children ; i++) {
                unsigned long child = c->frames[node].children[i];
                std::string pref = p+(more?"│":" ");
                pref += " ";
                print_frame_tree(d,c,pref,i<num_children-1,child);
        }
}

void list_orphans(int d,struct dff *c, const std::string &p)
{
        print_frame_tree(d,c,p,0,c->root_frame);
        for (unsigned long i=0 ; i<c->geometries.size() ; i++) {
                if (c->geometries[i].frame==-1) {
                        VBOS(-1000,p<<"orphaned_geometry: "<<i<<"\n");
                }
        }
}

void app_verbose(char const* file, int line, const std::string& msg)
{
        std::cout<<BOLD<<GREEN<<"VERBOSE "<<RESET
                 <<BOLD<<file<<NOBOLD<<":"<<BOLD<<line<<NOBOLD
                 << ": \""<<BOLD<<BLUE<<msg<<RESET"\"";
        std::cout<<std::endl;
}

void app_error(char const* file, int line,
               const std::string& i_was, const std::string& msg)
{
        std::cout<<BOLD RED"ERROR "<<RESET
                 <<BOLD<<file<<NOBOLD<<":"<<BOLD<<line<<NOBOLD
                 <<": \""<<BOLD<<YELLOW<<msg<<RESET<<"\"";
        if (i_was!="")
                std::cout<<" ("<<BOLD<<YELLOW<<i_was<<RESET<<")";
        std::cout<<std::endl;
}

void app_line(const std::string &msg)
{
        std::cout<<BOLD<<msg<<NOBOLD<<std::endl;
}

void app_fatal()
{
        abort();
}

void assert_triggered (void) { } 

#define VERSION "1.1"

const char *info =
"dffread (c) Dave Cunningham 2007  (version: " VERSION ")\n"
"I print information about dff files.\n";

const char *usage =
"Usage: dffread { <opt> } filename.dff\n\n"
"where <opt> ::= \"-v\" | \"--verbose\"               increase debug level\n"
"              | \"-q\" | \"--quiet\"                 decrease debug level\n"
"              | \"-e\" <name> | \"--export\" <name>  export to .mesh\n"
"              | \"-t\" <name> | \"--txd\" <name>     for export\n\n"
"              | \"-o\" | \"--orphans\"               list orphaned frames\n"
"              | \"-O\" | \"--no-orphans\"            complement of above\n\n"
"              | \"-h\" | \"--help\"                  this message\n";

std::string next_arg(int& so_far, int argc, char **argv)
{
        if (so_far==argc) {
                std::cerr<<"Ran out of arguments."<<std::endl;
                std::cerr<<usage<<std::endl;
                exit(EXIT_FAILURE);
        }
        return argv[so_far++];
}


std::string get_ext(const std::string& name, std::string *base_name)
{
        std::string r;
        std::string::size_type dot = name.find_last_of('.');
        if (dot!=std::string::npos) {
                r = name.substr(dot);
                if (base_name) *base_name = name.substr(0,dot);
        }
        return r;
}

// I think we never actually call this as we never read a tcol
std::string pwd_full (const std::string &, const std::string &)
{ abort(); return std::string(); } 
                 
int main(int argc, char **argv)
{
    if (argc==1) {
        std::cout<<info<<std::endl;
        std::cout<<usage<<std::endl;
    }

    // default parameters
    int d= 0;
    bool orphans = false;
    bool do_export_mesh = false;
    bool car = false;
    std::string oname;
    std::string txdname;

    int so_far = 1;
    int extras = 0; // number of args that weren't options
    std::string file_name;
    bool filename_given = false;
    std::string phys_mat_pref;

    //init_col_db_same("/common/Metal");
    init_global_db();

    while (so_far<argc) {
        std::string arg = next_arg(so_far,argc,argv);
        if (arg=="-v" || arg=="--verbose") {
            d++;
        } else if (arg=="-q" || arg=="--quiet") {
            d--;
        } else if (arg=="-o" || arg=="--orphans") {
            orphans = true;
        } else if (arg=="-O" || arg=="--no-orphans") {
            orphans = false;
        } else if (arg=="-t" || arg=="--txd") {
            txdname = next_arg(so_far,argc,argv);
        } else if (arg=="-p" || arg=="--phys-mat-prefix") {
            phys_mat_pref = next_arg(so_far,argc,argv);
        } else if (arg=="-e" || arg=="--export") {
            oname = next_arg(so_far,argc,argv);
            do_export_mesh = true;
        } else if (arg=="-c" || arg=="--car") {
            car = true;
        } else if (arg=="-C" || arg=="--nocar") {
            car = false;
        } else if (arg=="-h" || arg=="--help") {
            std::cout<<info<<std::endl;
            std::cout<<usage<<std::endl;
        } else if (extras==0) {
            extras++;
            filename_given = true;
            file_name = arg;
            if (get_ext(file_name,NULL)!=".dff") {
                std::cerr << file_name<<": does not end in .dff"
                          << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr<<"Unrecognised argument: "<<arg<<std::endl;
            std::cerr<<usage<<std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if (!filename_given) {
        std::cerr<<"No filename given."<<std::endl;
        std::cerr<<usage<<std::endl;
        exit(EXIT_FAILURE);
    }

    try {

        std::ofstream lua_file;
        if  (do_export_mesh) {
            init_ogre();
            std::string s = oname+".lua";
            lua_file.open(s.c_str(), std::ios::binary);
            APP_ASSERT_IO_SUCCESSFUL(lua_file, "opening "+s);
        }

        std::ifstream f;
        f.open(file_name.c_str(), std::ios::binary);
        APP_ASSERT_IO_SUCCESSFUL(f,"opening "+file_name);
        VBOS(0,"reading dff: "<<file_name<<"\n");

        struct dff dff;
        ios_read_dff(d,f,&dff,file_name+"/", phys_mat_pref,global_db);

        if (orphans)
            list_orphans(d,&dff,file_name+"/");

        if (do_export_mesh) {
            if (car) {
                StringSet texs;
                ide ide;
                Strings imgs;
                imgs.push_back("");
                Obj obj; // currently obj only needs a few things set
                obj.dff = file_name;
                obj.txd = txdname;
                obj.flags = 0;
                obj.is_car = true;
                MatDB matdb;
                for (unsigned long j=0 ; j<dff.frames.size() ; ++j) {
                    frame &fr = dff.frames[j];

                    // ignore dummies for now
                    if (fr.geometry == -1) continue;

                    geometry &g = dff.geometries[fr.geometry];

                    generate_normals(g);

                    export_mesh(texs,ide,imgs,
                                std::cout, oname+"."+fr.name+".mesh",
                                obj,oname+"."+fr.name,g,matdb,lua_file);
                }

                lua_file << std::endl;

                for (unsigned long j=0 ; j<dff.frames.size() ; ++j) {
                    frame &fr = dff.frames[j];

                    if (fr.name.substr(fr.name.size()-4,4)=="_dam") continue;
                    if (fr.name.substr(fr.name.size()-4,4)=="_vlo") continue;

                    std::string parent = "sm.root";
                    if (fr.parent_frame >= 0) {
                        frame &parent_frame = dff.frames[fr.parent_frame];
                        parent = oname + "_" + parent_frame.name;
                    }
                    lua_file<<oname<<"_"<<fr.name<<" = "<<parent<<":createChild()"<<std::endl;
                    lua_file<<oname<<"_"<<fr.name<<".position = "
                            <<"vector3("<<fr.x<<","<<fr.y<<","<<fr.z<<")"<<std::endl;

                    // ignore dummies for now
                    if (fr.geometry == -1) continue;

                    std::string name = oname+"."+fr.name;
                    std::string mesh = name+".mesh";
                    lua_file<<"safe_destroy(sm:getEntity(\""<<name<<"\"))"<<std::endl;
                    lua_file<<oname<<"_"<<fr.name<<":attachObject("
                            <<"sm:createEntity(\""<<name<<"\",\"cheetah/"<<mesh<<"\"))"<<std::endl;
                    lua_file<<"sm:getEntity(\""<<name<<"\"):setCustomParameterAll(0,1,1,1,1)"
                            <<std::endl;
                }

                if (dff.has_tcol) {
                    std::string col_name = oname;
                    static const char *phys_mat = "/common/Metal";

                    for (unsigned i=0 ; i<dff.tcol.compound.hulls.size() ; ++i)
                        dff.tcol.compound.hulls[i].material = phys_mat;
                    for (unsigned i=0 ; i<dff.tcol.compound.boxes.size() ; ++i)
                        dff.tcol.compound.boxes[i].material = phys_mat;
                    for (unsigned i=0 ; i<dff.tcol.compound.cylinders.size() ; ++i)
                        dff.tcol.compound.cylinders[i].material = phys_mat;
                    for (unsigned i=0 ; i<dff.tcol.compound.cones.size() ; ++i)
                        dff.tcol.compound.cones[i].material = phys_mat;
                    for (unsigned i=0 ; i<dff.tcol.compound.planes.size() ; ++i)
                        dff.tcol.compound.planes[i].material = phys_mat;
                    for (unsigned i=0 ; i<dff.tcol.compound.spheres.size() ; ++i)
                        dff.tcol.compound.spheres[i].material = phys_mat;
                    for (unsigned i=0 ; i<dff.tcol.triMesh.faces.size() ; ++i)
                        dff.tcol.triMesh.faces[i].material = phys_mat;

                    dff.tcol.mass = 1000;

                    if (!dff.tcol.usingCompound && !dff.tcol.usingTriMesh) {
                            GRIT_EXCEPT("Collision data had no compound or trimesh");
                    /*
                    } else if (binary) {
                            col_name += ".bcol";
                            GRIT_EXCEPT("Writing bcol not implemented.");
                    */
                    } else {
                            col_name += ".tcol";

                            std::ofstream out;
                            out.open(col_name.c_str(), std::ios::binary);
                            APP_ASSERT_IO_SUCCESSFUL(out,"opening tcol for writing");

                            pretty_print_tcol(out,dff.tcol);
                    }

                    lua_file << std::endl;
                    lua_file << "class `"<<oname<<"` (ColClass) {" << std::endl;
                    lua_file << "    gfxMesh=`"<<oname<<".chassis.mesh`;" << std::endl;
                    lua_file << "    colMesh=`"<<col_name<<"`;" << std::endl;
                    lua_file << "}" << std::endl;
                }

            } else {
                StringSet texs;
                ide ide;
                Strings imgs;
                imgs.push_back("");
                Obj obj; // currently obj only needs a few things set
                obj.dff = file_name;
                obj.txd = txdname;
                obj.flags = 0;
                obj.is_car = false;
                MatDB matdb;
                export_mesh(texs, ide, imgs, std::cout, oname+".mesh", obj,
                            oname, dff.geometries[0], matdb, lua_file);
            }
        }

    } catch (const Exception &e) {
        std::cerr << e << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif

// vim: shiftwidth=4:tabstop=4:expandtab
