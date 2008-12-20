#include "ColParser.h"
#include "ios_util.h"

static inline int tolowr (int c)
{
    return std::tolower(char(c),std::cout.getloc());
}

static inline std::string& strlower(std::string& s)
{
    std::transform(s.begin(),s.end(), s.begin(),tolowr);
    return s;
}

bool parse_col (std::string &name,
                std::istream &in,
                TColFile &tcol,
                int debug_level)
{
        unsigned long fourcc;
        fourcc = ios_read_u32(in);

        tcol.mass = 0;
        tcol.friction = 0.5;
        tcol.restitution = 0;
        tcol.linearDamping = 0;
        tcol.angularDamping = 0.5;
        tcol.linearSleepThreshold = 1;
        tcol.angularSleepThreshold = 0.8;

        unsigned long size = ios_read_u32(in);
        (void)size;

        int version;
        switch (fourcc) {
                case 0x4c4c4f43: // COLL
                        version = 1;
                        break;
                case 0x324c4f43: // COL2
                        version = 2;
                        break;
                case 0x334c4f43: // COL3
                        version = 3;
                        break;
                default:
                        IOS_EXCEPT("This is not a col file.");
        }

        if (debug_level>0)
                std::cout << "Version: " << version << std::endl;

        ASSERT(version==3 || version==2);

        name = ios_read_fixedstr(in,24);
        strlower(name);

        if (debug_level>0)
                std::cout << "Name: " << name << std::endl;

        // skip 40 byte bounding structure
        in.seekg(40,std::ios_base::cur);

        if (version >= 2) {

                unsigned short num_spheres = ios_read_u16(in);
                if (debug_level>1)
                        std::cout<<"num_spheres: "<<num_spheres<<std::endl;
                unsigned short num_boxes = ios_read_u16(in);
                if (debug_level>1)
                        std::cout<<"num_boxes: "<<num_boxes<<std::endl;
                unsigned long num_faces = ios_read_u32(in);
                if (debug_level>1)
                        std::cout<<"num_faces: "<<num_faces<<std::endl;
                unsigned long flags = ios_read_u32(in);
                //bool non_empty = flags & 2;
                //bool has_face_groups = flags & 8;
                //bool has_shadow_mesh = flags & 16;
                ASSERT((flags | 2 | 8 | 16) == 26);
                unsigned long offset_spheres = ios_read_u32(in);
                (void) offset_spheres;
                if (debug_level>2)
                    std::cout<<"offset_spheres: "<<offset_spheres<<std::endl;
                unsigned long offset_boxes = ios_read_u32(in);
                (void) offset_boxes;
                if (debug_level>2)
                    std::cout<<"offset_boxes: "<<offset_boxes<<std::endl;
                unsigned long unknown = ios_read_u32(in);
                ASSERT(unknown==0);
                unsigned long offset_vertexes = ios_read_u32(in);
                if (debug_level>2)
                    std::cout<<"offset_vertexes: "<<offset_vertexes<<std::endl;
                unsigned long offset_faces = ios_read_u32(in);
                if (debug_level>2)
                    std::cout<<"offset_faces: "<<offset_faces<<std::endl;
                unsigned long unknown2 = ios_read_u32(in);
                ASSERT(unknown2==0);

                unsigned long num_shadow_faces = 0;
                unsigned long offset_shadow_vertexes = 0;
                unsigned long offset_shadow_faces = 0;
                if (version==3) {
                        num_shadow_faces = ios_read_u32(in);
                        if (debug_level>1)
                                std::cout<<"num_shadow_faces: "
                                         <<num_shadow_faces<<std::endl;
                        offset_shadow_vertexes = ios_read_u32(in);
                        if (debug_level>1)
                                std::cout<<"offset_shadow_vertexes: "
                                         <<offset_shadow_vertexes<<std::endl;
                        offset_shadow_faces = ios_read_u32(in);
                        if (debug_level>1)
                                std::cout<<"offset_shadow_faces: "
                                         <<offset_shadow_faces<<std::endl;
                }

                tcol.usingCompound = num_spheres>0 || num_boxes>0;

                tcol.usingTriMesh = num_faces > 0;

                for (int i=0 ; i<num_spheres ; ++i) {
                        Sphere sphere;
                        sphere.px = ios_read_float(in);
                        sphere.py = ios_read_float(in);
                        sphere.pz = ios_read_float(in);
                        sphere.radius = ios_read_float(in);
                        unsigned long surface = ios_read_u32(in);
                        (void) surface; // not used yet
                        tcol.compound.spheres.push_back(sphere);
                }

                for (int i=0 ; i<num_boxes ; ++i) {
                        float xm = ios_read_float(in);
                        float ym = ios_read_float(in);
                        float zm = ios_read_float(in);
                        float xM = ios_read_float(in);
                        float yM = ios_read_float(in);
                        float zM = ios_read_float(in);
                        Box box;
                        box.px = (xm+xM)/2;
                        box.py = (ym+yM)/2;
                        box.pz = (zm+zM)/2;
                        box.dx = (xM-xm);
                        box.dy = (yM-ym);
                        box.dz = (zM-zm);
                        box.qw = 1;
                        box.qx = 0;
                        box.qy = 0;
                        box.qz = 0;
                        box.margin = 0.04;
                        unsigned long surface = ios_read_u32(in);
                        (void) surface; // not used yet
                        tcol.compound.boxes.push_back(box);
                }

                // an approximation, real value is much less.
                int num_vertexes = (offset_faces - offset_vertexes)/6;
                if (debug_level>3)
                        std::cout<<"num_vertexes: "<<num_vertexes<<std::endl;
                int remainder = (offset_faces - offset_vertexes) % 6;
                if (debug_level>3)
                        std::cout<<"remainder: "<<remainder << std::endl;
                for (int i=0 ; i<num_vertexes ; ++i) {
                        float x = ios_read_s16(in)/128.0;
                        float y = ios_read_s16(in)/128.0;
                        float z = ios_read_s16(in)/128.0;
                        tcol.triMesh.vertexes.push_back(btVector3(x,y,z));
                }

                // seek all the way up to offset_faces-4
                in.seekg(remainder,std::ios_base::cur);

                unsigned short max_vertex = 0;

                for (unsigned long i=0 ; i<num_faces ; ++i) {
                        // faces
                        unsigned short a = ios_read_u16(in);
                        unsigned short b = ios_read_u16(in);
                        unsigned short c = ios_read_u16(in);
                        max_vertex =
                                std::max(a,std::max(b,std::max(c,max_vertex)));
                        unsigned char mat = ios_read_u8(in);
                        (void) mat; // not used yet
                        unsigned char light = ios_read_u8(in);
                        (void) light; // not used yet
                        tcol.triMesh.faces.push_back(Face(a,b,c,0));
                }

                if (num_faces > 0) {
                        if (debug_level>1)
                            std::cout<<"max_vertex: "<<max_vertex<<std::endl;
                        ASSERT(max_vertex<num_vertexes);
                        tcol.triMesh.vertexes.resize(max_vertex+1);
                }

                in.seekg(offset_shadow_faces - offset_shadow_vertexes,
                         std::ios_base::cur);

                in.seekg(num_shadow_faces*8, std::ios_base::cur);

        } else {

                ASSERT(false);
/*
                unsigned long num_spheres = ios_read_u32(in);
                // spheres
                unsigned long unknown = ios_read_u32(in);
                (void) unknown;
                unsigned long num_boxes = ios_read_u32(in);
                // boxes
                unsigned long num_vertexes = ios_read_u32(in);
                // vertexes
                unsigned long num_faces = ios_read_u32(in);
                // faces
*/

        }

        return true;
}

void dump_all_cols (std::istream &in, bool binary, int debug_level)
{
        do {
                TColFile tcol;
                std::string name;
                parse_col(name,in,tcol,debug_level);

                if (!tcol.usingCompound && !tcol.usingTriMesh) {
                        if (debug_level>0)
                                std::cout<<"Col is empty."<<std::endl;
                        // skip empty cols.  They are pure lol.
                } else if (binary) {
                        name += ".bcol";
                        IOS_EXCEPT("Writing bcol not implemented.");
                } else {
                        name += ".tcol";

                        std::ofstream out;
                        out.open(name.c_str(), std::ios::binary);
                        ASSERT_IO_SUCCESSFUL(out,"opening tcol for writing");

                        pretty_print_tcol(out,tcol);
                }
                
                std::istream::int_type next = in.peek();

                // no more cols
                if (next==std::istream::traits_type::eof() || next==0) break; 

        } while (true);
}



// vim: shiftwidth=8:tabstop=8:expandtab
