#include <cstdlib>
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

#define MODNAME std::string("gtasa")

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

static void unrec (const std::string &p, unsigned long type,
                   unsigned long size, const std::string &word)
{
    std::stringstream ss;
    ss << p << type << " 0x" << HEX(type)
       << " ("<<size<<"B) UNREC "<<word<<" EXT!";
    IOS_EXCEPT(ss.str());
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
        IOS_EXCEPT(msg.str());
    }
}}}


static void ios_read_texture (int d, std::ifstream &f,
                              unsigned long file_version,
                              struct texture *t,
                              const std::string &p)
{{{
    unsigned long type, tex_size;
    ios_read_header(f,&type,&tex_size,NULL,&file_version);
    ASSERT(type==RW_TEXTURE);
    std::streamoff tex_start = f.tellg();

    unsigned long size;
    ios_read_header(f,&type,&size,NULL,&file_version);
    ASSERT(type==RW_DATA);
    ASSERT(size==4);
    t->filter_flags = ios_read_u16(f);
    VBOS(3,p<<"filter_flags: "<<HEX(t->filter_flags));
    t->unk1 = ios_read_u16(f);
    VBOS(3,p<<"mysterious_flag: "<<HEX(t->unk1));

    ios_read_header(f,&type,&size,NULL,&file_version);
    ASSERT(type==RW_STRING);
    t->name = ios_read_fixedstr(f,size);
    VBOS(3,p<<"name: "<<t->name);

    ios_read_header(f,&type,&size,NULL,&file_version);
    ASSERT(type==RW_STRING);
    std::string alpha_str = ios_read_fixedstr(f,size);
    t->has_alpha = alpha_str!="";
    VBOS(3,p<<"has_alpha: \""<<t->has_alpha<<"\"");

    unsigned long totalsize;
    ios_read_header(f,&type,&totalsize,NULL,&file_version);
    ASSERT(type==RW_EXT);

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
        ASSERT(m->mat_type2 == MATBUMP2);
        m->multiplier = ios_read_float(f);
        VBOS(3,p<<"multiplier: "<<m->multiplier);
        m->dest_blend_type = ios_read_u32(f);
        ASSERT(m->dest_blend_type==0);
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
        ASSERT(m->mat_type2 == MATBUMP1);
        m->multiplier = ios_read_float(f);
        VBOS(3,p<<"multiplier: "<<m->multiplier);
        m->dest_blend_type = ios_read_u32(f);
        ASSERT(m->dest_blend_type==0);
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
        ASSERT(m->mat_type3 == MATENV1);
        m->multiplier2 = ios_read_float(f);
        VBOS(3,p<<"multiplier2: "<<m->multiplier2);
        m->dest_blend_type2 = ios_read_u32(f);
        ASSERT(m->dest_blend_type2==0);
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
        ASSERT(m->mat_type2 == MATUVTRANSFORM2);
        ios_read_u32(f);// WHAT THE FUCK?
        break;
    case MATDUALPASS1:
        ASSERT(m->mat_type2 == MATDUALPASS2);
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
        ASSERT(m->mat_type2 == MATENV2);
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
        IOS_EXCEPT(ss.str());
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
    ASSERT(type==RW_MATERIAL);
    std::streamoff material_start = f.tellg();

    ios_read_header(f,&type,NULL,NULL,&file_version);
    ASSERT(type==RW_DATA);
    unsigned long zero = ios_read_u32(f);
    ASSERT(zero==zero);
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
    ASSERT(m->unk5==1.0);

    m->textures.resize(m->num_textures);
    for (unsigned long i=0 ; i<m->num_textures ; i++) {
        std::stringstream prefss;
        prefss<<p<<"texture["<<i<<"].";
        ios_read_texture(d,f,file_version,&m->textures[i],prefss.str());
    }

    unsigned long totalsize;
    ios_read_header(f,&type,&totalsize,NULL,&file_version);
    ASSERT(type==RW_EXT);

    while (totalsize>0) {
        unsigned long size;
        ios_read_header(f,&type,&size,NULL,&file_version);
        std::streamoff start = f.tellg();
        totalsize -= size + 12;
        switch (type) {
        case RW_REFLECTION_MATERIAL:
            ASSERT(size==24);
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
            ASSERT(m->reflection_material.unk6==0.0);
            break;
        case RW_MATERIAL_EFFECTS:
            ios_read_material_effs(d,f,file_version,m,p);
            break;
        case RW_SPEC:
            m->spec_level = ios_read_float(f);
            VBOS(3,p<<"spec_level: "<<m->spec_level);
            m->spec_texture = ios_read_fixedstr(f,size-4);
            VBOS(3,p<<"spec_texture: "<<m->spec_texture.c_str());
            break;
        case RW_UV_ANIMATION:
            unsigned long anim_size;
            ios_read_header(f,&type,&anim_size,NULL,&file_version);
            ASSERT(type==RW_DATA);
            ASSERT(anim_size==36);
            VBOS(1,p<<"SKIPPING_OVER_ADC: "<<DECHEX(anim_size));
            ios_read_byte_array(f,NULL,anim_size);
            break;
        case RW_RIGHT_TO_RENDER:
            ASSERT(size==8);
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

static void ios_read_geometry (int d,
                               std::ifstream &f,
                               unsigned long file_version,
                               struct geometry &g,
                               const std::string &p)
{{{
    g.frame = 0xFFFFFFFF;

    unsigned long type, geometry_size;
    ios_read_header(f,&type,&geometry_size,NULL,&file_version);
    ASSERT(type==RW_GEOMETRY);
    std::streamoff geometry_start = f.tellg();

    unsigned long struct_size;
    ios_read_header(f,&type,&struct_size,NULL,&file_version);
    std::streamoff struct_start = f.tellg();
    ASSERT(type==RW_DATA);
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
    ASSERT(tmp==(GEO_TRISTRIP|GEO_POSITIONS|GEO_TEXCOORDS|GEO_COLOURS|
               GEO_NORMALS|GEO_LIGHTS|GEO_MODULATE|GEO_TEXCOORDS2|
               GEO_UNKNOWN1|GEO_UNKNOWN2));
    g.num_faces = ios_read_u32(f);
    VBOS(3,p<<"num_faces: "<<g.num_faces);
    g.faces.resize(g.num_faces);
    g.num_vertexes = ios_read_u32(f);
    VBOS(3,p<<"num_vertexes: "<<g.num_vertexes);
    g.vertexes.resize(g.num_vertexes);
    g.num_frames = ios_read_u32(f);
    ASSERT(g.num_frames==1);
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
                    <<g.tex_coords[i].u<<g.tex_coords[i].v);
        }
    }
    if (g.flags&GEO_UNKNOWN2) {
        g.tex_coords2.resize(g.num_vertexes);
        for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
            g.tex_coords2[i].u = ios_read_float(f);
            g.tex_coords2[i].v = ios_read_float(f);
            VBOS(4,p<<"texture_coords2["<<i<<"]: "
                    <<g.tex_coords2[i].u<<g.tex_coords2[i].v);
        }
        g.lights.resize(g.num_vertexes);
        for (unsigned long i=0 ; i<g.num_vertexes ; i++) {
            g.lights[i].unk1 = ios_read_float(f);
            g.lights[i].unk2 = ios_read_float(f);
            VBOS(4,p<<"lights["<<i<<"]: "
                   <<g.lights[i].unk1<<g.lights[i].unk2);
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
    ASSERT(type==RW_MATERIAL_LIST);

    ios_read_header(f,&type,NULL,NULL,&file_version);
    ASSERT(type==RW_DATA);
    g.num_materials = ios_read_u32(f);
    g.materials.resize(g.num_materials);
    VBOS(4,p<<"num_materials: "<<g.num_materials);
    for (unsigned long i=0 ; i<g.num_materials ; i++) {
        unsigned long big = ios_read_u32(f);
        ASSERT(big==ULONG_MAX);
    }

    for (unsigned long i=0 ; i<g.num_materials ; i++) {
        std::stringstream prefss;
        prefss<<p<<"material["<<i<<"].";
        ios_read_material(d,f,file_version,&g.materials[i],prefss.str());
    }

    unsigned long totalsize;
    ios_read_header(f,&type,&totalsize,NULL,&file_version);
    ASSERT(type==RW_EXT);

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
                //ASSERT(one==1);
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

        case RW_2DFX:
            VBOS(1,p<<"SKIPPING_OVER_2DFX: "<<DECHEX(size));
            ios_read_byte_array(f,NULL,size);
            break;
        default:
            unrec(p,type,size,"GEOMETRY");
        }
    }

    check_spill(f,geometry_start,geometry_size,p);
}}}


void ios_read_dff (int d, std::ifstream &f, struct dff *c, const std::string &p)
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

    ASSERT(type==RW_CHUNK);

    ios_read_header(f,&type,&size,NULL,&file_version);
    ASSERT(type==RW_DATA);
    ASSERT(size==12);
    unsigned long num_objects = ios_read_u32(f);
    VBOS(3,p<<"num_objects: "<<num_objects);
    unsigned long num_lights = ios_read_u32(f);
    VBOS(3,p<<"num_lights: "<<num_lights);
    unsigned long unk2 = ios_read_u32(f);
    ASSERT(unk2==0);

    // frame list
    ios_read_header(f,&type,&size,NULL,&file_version);
    ASSERT(type==RW_FRAME_LIST);
    ios_read_header(f,&type,&size,NULL,&file_version);
    ASSERT(type==RW_DATA);
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
        unsigned long parent = ios_read_u32(f);
        c->frames[i].parent_frame = parent;
        VBOS(3,pref<<"parent_frame: "<<parent);
        if (parent==0xFFFFFFFF) {
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
        c->frames[i].geometry = 0xFFFFFFFF;
        unsigned long total_size;
        ios_read_header(f,&type,&total_size,NULL,&file_version);
        ASSERT(type==RW_EXT);
        while (total_size>0) {
            ios_read_header(f,&type,&size,NULL,&file_version);
            total_size -= size + 12;
            switch(type) {
            case RW_BONE:
                c->frames[i].bone_unk_flags = ios_read_u32(f);
                ASSERT(c->frames[i].bone_unk_flags==0x100);
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
    ASSERT(type==RW_GEOMETRY_LIST);
    ios_read_header(f,&type,&size,NULL,&file_version);
    ASSERT(type==RW_DATA);
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
        ASSERT(type==RW_ATOMIC);
        ios_read_header(f,&type,&size,NULL,&file_version);
        ASSERT(type==RW_DATA);
        ASSERT(size==16);
        unsigned long frame_index = ios_read_u32(f);
        c->objects[i].frame_index = frame_index;
        VBOS(3,pref<<"frame_index: "<<frame_index);
        unsigned long geometry_index = ios_read_u32(f);
        c->objects[i].geometry_index = geometry_index;
        VBOS(3,pref<<"geometry_index: "<<geometry_index);
        unsigned long five = ios_read_u32(f);
        ASSERT(five==5);
        unsigned long zero = ios_read_u32(f);
        ASSERT(zero==0);

        ASSERT(c->frames[frame_index].geometry==0xFFFFFFFF);
        c->frames[frame_index].geometry = geometry_index;

        ASSERT(c->geometries[geometry_index].frame==0xFFFFFFFF);
        c->geometries[geometry_index].frame = frame_index;

        unsigned long total_size;
        ios_read_header(f,&type,&total_size,NULL,&file_version);
        ASSERT(type==RW_EXT);
        while (total_size>0) {
            ios_read_header(f,&type,&size,NULL,&file_version);
            total_size -= size + 12;
            switch(type) {
            case RW_RIGHT_TO_RENDER:
            ASSERT(size==8);
            c->objects[i].rtr_unk1 = ios_read_u32(f);
            VBOS(3,pref<<"rtr_unk1: "<<DECHEX(c->objects[i].rtr_unk1));
            c->objects[i].rtr_unk2 = ios_read_u32(f);
            VBOS(3,pref<<"rtr_unk2: "<<DECHEX(c->objects[i].rtr_unk2));
            break;

            case RW_MATERIAL_EFFECTS: {
                ASSERT(size==4);
                unsigned long eff = ios_read_u32(f);
                ASSERT(eff==1);
            } break;

            case RW_UNKNOWN:
            ASSERT(size==4);
            c->unk = ios_read_u32(f);
            VBOS(3,pref<<"obj_ext_unk: "<<c->unk);
            break;

            default:
                unrec(p,type,size,"ATOMIC");
            }
        }
    }

    c->lights.resize(num_lights);
    for (unsigned long i=0 ; i<num_lights ; i++) {
        std::stringstream prefss;
        prefss<<p<<"light["<<i<<"].";
        std::string pref = prefss.str();
        ios_read_header(f,&type,&size,NULL,&file_version);
        ASSERT(type==RW_DATA);
        ASSERT(size==4);
        unsigned long num = ios_read_u32(f);
        (void)num;
        //ASSERT(num==i+1);

        ios_read_header(f,&type,&size,NULL,&file_version);
        ASSERT(type==RW_LIGHT);
        ASSERT(size==48);

        ios_read_header(f,&type,&size,NULL,&file_version);
        ASSERT(type==RW_DATA);
        ASSERT(size==24);
        c->lights[i].unk1 = ios_read_float(f);
        VBOS(3,pref<<"unk1: "<<c->lights[i].unk1);
        c->lights[i].unk3 = ios_read_float(f);
        VBOS(3,pref<<"unk2: "<<c->lights[i].unk2);
        c->lights[i].unk3 = ios_read_float(f);
        VBOS(3,pref<<"unk3: "<<c->lights[i].unk3);
        c->lights[i].unk4 = ios_read_float(f);
        VBOS(3,pref<<"unk4: "<<c->lights[i].unk4);
        c->lights[i].unk5 = ios_read_float(f);
        VBOS(3,pref<<"unk5: "<<c->lights[i].unk5);
        c->lights[i].unk6 = ios_read_float(f);
        VBOS(3,pref<<"unk6: "<<c->lights[i].unk6);

        ios_read_header(f,&type,&size,NULL,&file_version);
        ASSERT(type==RW_EXT);
        ASSERT(size==0);
        //
    }

    unsigned long totalsize;
    ios_read_header(f,&type,&totalsize,NULL,&file_version);
    ASSERT(type==RW_EXT);
    while (totalsize>0) {
        unsigned long size;
        ios_read_header(f,&type,&size,NULL,&file_version);
        totalsize -= size + 12;
        switch (type) {
        case RW_COLLISION:
            VBOS(0,p<<"SKIPPING_OVER_COL_DATA: "<<DECHEX(size));
            ios_read_byte_array(f,NULL,size); //TODO
            break;
        default:
            unrec(p,type,size,"MODEL");
        }
    }

    check_spill(f,dff_start,dff_size,p);

    //everything has just one clump except clothing which seems to have 3...
    //unsigned char byte = ios_read_u8(f);
    //ASSERT(byte==0);

}}}



////////////////////////////////////////////////////////////////////////////////
// OUTPUT OF MESH AND MATERIALS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ind (std::ostream &out, unsigned int level)
{
    for (unsigned int i=0 ; i<level ; i++)
        out << "  ";
}

/*

const char *car_generic_texs[] = { //{{{
"xvehicleenv128", "vehicletyres128", "vehiclesteering128", "vehiclespecdot64",
"vehicleshatter128", "vehiclescratch64", "vehiclepoldecals128",
"vehiclelightson128", "vehiclelights128", "vehiclegrunge256",
"vehiclegeneric256", "vehicleenvmap128", "vehicledash32", "platecharset",
"plateback3", "plateback2", "plateback1", "carplate", "carpback",
}; //}}}

*/


#define ARRLEN(a) (sizeof a/sizeof *a)

std::string get_tex_name (const std::string& img,
                          std::string txd,
                          std::string tex_name)
{{{
    strlower(tex_name);
    tex_name += ".dds";
    txd  += ".txd";

    // prepend img if given
    if (img!="") {
        txd = img+"/"+txd;
    }

    tex_name = MODNAME + "/" + txd + "/" + tex_name;

    // if it's one of these textures, it is not found in a per-dff txd file
    // it is instead found in vehicle.txd
/*
    for (unsigned long j=0 ; j<ARRLEN(car_generic_texs) ; j++) {
        if (src==car_generic_texs[j]) {
            // one of the generic textures,
            // which we find in:
            txd = "vehicle.txd";
            break;
        }
    }
*/

    // if it's a duplicate of another texture, use the other texture instead
    TexDupMap::iterator i = tex_dup_map.find(tex_name);

    if (i!=tex_dup_map.end()) {
        tex_name = i->second;
    }

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

static const std::string &export_or_provide_mat (const StringSet &texs,
                                                 const std::string &img,
                                                 geometry &g,
                                                 int mindex,
                                                 const Obj &obj,
                                                 MatDB &matdb,
                                                 std::ostream &matbin)
{{{

    material &m = g.materials[mindex];

    bool has_alpha = false; // old method(stupid): obj.flags & OBJ_FLAG_ALPHA1;
    bool no_alpha_reject = 0 != (obj.flags & OBJ_FLAG_NO_SHADOW);
    bool double_sided = 0 != (obj.flags & OBJ_FLAG_DRAW_BACKFACE);
    bool dynamic_lighting = g.normals.size()>0;

    float R = ((m.colour >> 0) & 0xFF) / 255.0f;
    float G = ((m.colour >> 8) & 0xFF) / 255.0f;
    float B = ((m.colour >> 16) & 0xFF) / 255.0f;
    float A = ((m.colour >> 24) & 0xFF) / 255.0f;

    Strings textures;

    ASSERT(m.num_textures==1  || m.num_textures==0);
    for (unsigned int i=0 ; i<m.num_textures ; i++) {
        std::string tex_name;
        tex_name=get_tex_name(img, obj.txd, m.textures[i].name);
        if (tex_name=="") continue;
        if (!texs.empty() && texs.find(tex_name)==texs.end()) {
                tex_name = "";
        }
        textures.push_back(tex_name);
        if (m.textures[i].has_alpha) has_alpha = true;
    }           

    if (textures.size()==0)
        textures.push_back("");

    m.rewrittenTextures = textures;

    std::ostringstream mname;
    mname<<MODNAME<<"/"<<obj.id<<"/"<<mindex;

    const std::string &mat = already_seen(matdb, R,G,B,A, textures, has_alpha,
                                          no_alpha_reject, double_sided,
                                          dynamic_lighting, mname.str());

    if (mat!=mname.str())
        return mat;

    unsigned char flags = 0;

    if (has_alpha) flags |= 0x1;
    if (double_sided) flags |= 0x2;
    if (dynamic_lighting) flags |= 0x4;
    if (no_alpha_reject) flags |= 0x8;

    ios_write_u32(matbin,0x4254414d);
    ios_write_u16(matbin,1); // version
    ios_write_u8(matbin,flags);
    //fwrite_fixedstr(d,f,mname.str(),61);
    ios_write_str(matbin,mname.str());
    ios_write_u32(matbin,m.colour);
    //fwrite_fixedstr(d,f,tname,88);
    ios_write_str(matbin,textures[0].c_str());

    return mat;
}}}

void export_xml (const StringSet &texs,
                 const std::string &img,
                 std::ostream &out,
                 const std::string &fname,
                 const Obj &obj,
                 struct geometry &g,
                 MatDB &matdb,
                 std::ostream &matbin)
{{{
    out << "xml filename: " << fname << std::endl;

    std::ofstream f;
    f.open(fname.c_str(), std::ios::binary);
    ASSERT_IO_SUCCESSFUL(f, "opening "+fname);


    ind(f,0);f<<"<mesh>\n";
    ind(f,1);f<<"<sharedgeometry vertexcount=\""<<g.num_vertexes<<"\">\n";
    int positions = g.vertexes.size();
    int normals = g.normals.size();
    int colours_diffuse = g.vertex_cols.size();
    int texture_coords = g.tex_coords.size() ? 1 : 0;
    int texture_coords2 = g.tex_coords2.size() ? 1 : 0;
    ASSERT(obj.id==14825 || obj.id==18009 || obj.id==18036 || !texture_coords || !texture_coords2);
    ASSERT(!texture_coords2 || texture_coords);
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

        int tri_counter = 0;
        int vertexes_so_far = 0;
        unsigned long v1,v2=0,v3=0;
        typedef std::vector<unsigned long> Ixs;
        for (Ixs::iterator face=s->indexes2.begin(),face_=s->indexes2.end() ;
             face!=face_ ; face++) {
            v1=v2;
            v2=v3;
            v3 = *face;
            ASSERT(v1 < g.vertexes.size());
            ASSERT(v2 < g.vertexes.size());
            ASSERT(v3 < g.vertexes.size());
            vertexes_so_far++;
            if (vertexes_so_far>=3 && v1!=v2 && v2!=v3 && v3!=v1) {
                if (vect_eq(g.vertexes[v1],g.vertexes[v2]) ||
                    vect_eq(g.vertexes[v2],g.vertexes[v3]) ||
                    vect_eq(g.vertexes[v3],g.vertexes[v1])) {
                } else {
                    tri_counter++;
                }
            }
        }

        if (tri_counter==0) {
            //out<<"Empty submesh: \""<<obj.id<<"/"<<s->material<<"\"\n";
            continue;
        }

        std::string mname =
            export_or_provide_mat(texs,img,g,s->material,obj,matdb,matbin);

        s->surrogate = mname;

        ind(f,2);f<<"<submesh material=\""<<mname<<"\" "
                  <<"usesharedvertices=\"true\" "
                  <<"operationtype=\"triangle_list\">\n";
        ind(f,3);f<<"<faces count=\""<<tri_counter<<"\">\n";

        // in a triangle strip, the normal alternates.
        // we reverse the order of the vertexes if tri_counter2 is odd
        int tri_counter2 = 0;
        vertexes_so_far = 0;
        for (std::vector<unsigned long>::iterator face=s->indexes2.begin() ;
             face!=s->indexes2.end() ; face++) {
            v1=v2;
            v2=v3;
            v3 = *face;
            vertexes_so_far++;
            if (vertexes_so_far>=3 && v1!=v2 && v2!=v3 && v3!=v1) {
                if (vect_eq(g.vertexes[v1],g.vertexes[v2]) ||
                    vect_eq(g.vertexes[v2],g.vertexes[v3]) ||
                    vect_eq(g.vertexes[v3],g.vertexes[v1])) {
                } else {
                    ind(f,4);
                    if (tri_counter2%2 == 0) {
                        f<<"<face v1=\""<<v1<<"\" "
                                "v2=\""<<v2<<"\" "
                                "v3=\""<<v3<<"\"/>\n";
                    } else {
                        f<<"<face v1=\""<<v1<<"\" "
                                "v2=\""<<v3<<"\" "
                                "v3=\""<<v2<<"\"/>\n";
                    }
                }
            }
            if (vertexes_so_far>=3) tri_counter2++;
        }
        ind(f,3);f<<"</faces>\n";

        //
        ind(f,2);f<<"</submesh>\n";

#if 0
 //OgreXMLConvertor doesn't like triangle lists very much...
        ind(f,2);fprintf(f,"<submesh material=\"%s\" "
                   "usesharedvertices=\"true\" "
                   "operationtype=\"triangle_strip\">\n",
                    mat.str().c_str());

        ind(f,3);fprintf(f,"<faces count=\"%d\">\n",
                    s->indexes2.size()-2);
        int num_so_far = 0;
        unsigned long v1,v2=0,v3=0;
        for (std::vector<unsigned long>::iterator face=s->indexes2.begin() ;
             face!=s->indexes2.end() ; face++) {
            v1=v2;
            v2=v3;
            v3 = *face;
            num_so_far++;
            if (num_so_far==3) {
                ind(f,4);fprintf(f,"<face v1=\"%d\" v2=\"%d\" "
                           "v3=\"%d\"/>\n", v1,v2,v3);
            } else if (num_so_far>3) {
                ind(f,4);fprintf(f,"<face v1=\"%d\"/>\n", v3);
            }
        }
        ind(f,3);fprintf(f,"</faces>\n");

        //
        ind(f,2);fprintf(f,"</submesh>\n");
#endif
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
                  const std::string &img,
                  std::ostream &out,
                  const std::string &fname,
                  const Obj &obj,
                  struct geometry &g,
                  MatDB &matdb,
                  std::ostream &matbin)
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

    if (!day_colours) night_colours = 0;

    ASSERT(positions>0);
    ASSERT(!normals || normals==positions);
    ASSERT(!day_colours || day_colours==positions);
    ASSERT(!night_colours || night_colours==positions);
    ASSERT(!texture_coords || texture_coords==positions);

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

        int tri_counter = 0;
        int vertexes_so_far = 0;
        unsigned long v1,v2=0,v3=0;
        typedef std::vector<unsigned long> Ixs;
        for (Ixs::iterator face=s->indexes2.begin(),face_=s->indexes2.end() ;
             face!=face_ ; face++) {
            v1=v2;
            v2=v3;
            v3 = *face;
            ASSERT(v1 < g.vertexes.size());
            ASSERT(v2 < g.vertexes.size());
            ASSERT(v3 < g.vertexes.size());
            vertexes_so_far++;
            if (vertexes_so_far>=3 && v1!=v2 && v2!=v3 && v3!=v1) {
                if (vect_eq(g.vertexes[v1],g.vertexes[v2]) ||
                    vect_eq(g.vertexes[v2],g.vertexes[v3]) ||
                    vect_eq(g.vertexes[v3],g.vertexes[v1])) {
                } else {
                    tri_counter++;
                }
            }
        }

        if (tri_counter==0) {
            //out<<"Empty submesh: \""<<obj.id<<"/"<<s->material<<"\"\n";
            continue;
        }

        unsigned short *ibuf = new unsigned short[tri_counter*3];
        unsigned short *ibuf_next = ibuf;

        std::string mname =
            export_or_provide_mat(texs,img,g,s->material,obj,matdb,matbin);

        s->surrogate = mname;

        // in a triangle strip, the normal alternates.
        // we reverse the order of the vertexes if tri_counter2 is odd
        int tri_counter2 = 0;
        vertexes_so_far = 0;
        for (Ixs::iterator face=s->indexes2.begin(),face_=s->indexes2.end() ;
             face!=face_ ; face++) {
            v1=v2;
            v2=v3;
            v3 = *face;
            vertexes_so_far++;
            if (vertexes_so_far>=3 && v1!=v2 && v2!=v3 && v3!=v1) {
                if (vect_eq(g.vertexes[v1],g.vertexes[v2]) ||
                    vect_eq(g.vertexes[v2],g.vertexes[v3]) ||
                    vect_eq(g.vertexes[v3],g.vertexes[v1])) {
                } else {
                    if (tri_counter2%2 == 0) {
                        *(ibuf_next++) = v1;
                        *(ibuf_next++) = v2;
                        *(ibuf_next++) = v3;
                    } else {
                        *(ibuf_next++) = v1;
                        *(ibuf_next++) = v3;
                        *(ibuf_next++) = v2;
                    }
                }
            }
            if (vertexes_so_far>=3) tri_counter2++;
        }

        Ogre::HardwareIndexBufferSharedPtr hwibuf = hwbmgr
                ->createIndexBuffer(
                        Ogre::HardwareIndexBuffer::IT_16BIT,
                        tri_counter*3,
                        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        hwibuf->writeData(0, hwibuf->getSizeInBytes(), ibuf, true);


        Ogre::SubMesh* submesh = mesh->createSubMesh();
        submesh->setMaterialName(mname);
        submesh->useSharedVertices = true;
        submesh->indexData->indexBuffer = hwibuf;
        submesh->indexData->indexCount = tri_counter * 3;
        submesh->indexData->indexStart = 0;

        delete [] ibuf;
    }

    mesh->_setBounds(Ogre::AxisAlignedBox(min_x,min_y,min_z,max_x,max_y,max_z));
    mesh->_setBoundingSphereRadius(Ogre::Math::Sqrt(rad2));

    mesh->load();

    serialiser->exportMesh(&*mesh, fname);

    mmgr->remove(mesh->getName());

    delete [] vbuf;

}}}

// vim: shiftwidth=4:tabstop=4:expandtab
