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

#include <locale>

#include "ColParser.h"
#include "ios_util.h"


MaterialDB db;

void init_col_db (const std::string &prefix)
{
        // they are numbered internally from zero
        db.setMaterial(prefix+"DEFAULT", 0);
        db.setMaterial(prefix+"TARMAC", 0);
        db.setMaterial(prefix+"TARMAC_FUCKED", 0);
        db.setMaterial(prefix+"TARMAC_REALLYFUCKED", 0);
        db.setMaterial(prefix+"PAVEMENT", 0);
        db.setMaterial(prefix+"PAVEMENT_FUCKED", 0);
        db.setMaterial(prefix+"GRAVEL", 0);
        db.setMaterial(prefix+"FUCKED_CONCRETE", 0);
        db.setMaterial(prefix+"PAINTED_GROUND", 0);
        db.setMaterial(prefix+"GRASS_SHORT_LUSH", 0);
        db.setMaterial(prefix+"GRASS_MEDIUM_LUSH", 0);
        db.setMaterial(prefix+"GRASS_LONG_LUSH", 0);
        db.setMaterial(prefix+"GRASS_SHORT_DRY", 0);
        db.setMaterial(prefix+"GRASS_MEDIUM_DRY", 0);
        db.setMaterial(prefix+"GRASS_LONG_DRY", 0);
        db.setMaterial(prefix+"GOLFGRASS_ROUGH", 0);
        db.setMaterial(prefix+"GOLFGRASS_SMOOTH", 0);
        db.setMaterial(prefix+"STEEP_SLIDYGRASS", 0);
        db.setMaterial(prefix+"STEEP_CLIFF", 0);
        db.setMaterial(prefix+"FLOWERBED", 0);
        db.setMaterial(prefix+"MEADOW", 0);
        db.setMaterial(prefix+"WASTEGROUND", 0);
        db.setMaterial(prefix+"WOODLANDGROUND", 0);
        db.setMaterial(prefix+"VEGETATION", 0);
        db.setMaterial(prefix+"MUD_WET", 0);
        db.setMaterial(prefix+"MUD_DRY", 0);
        db.setMaterial(prefix+"DIRT", 0);
        db.setMaterial(prefix+"DIRTTRACK", 0);
        db.setMaterial(prefix+"SAND_DEEP", 0);
        db.setMaterial(prefix+"SAND_MEDIUM", 0);
        db.setMaterial(prefix+"SAND_COMPACT", 0);
        db.setMaterial(prefix+"SAND_ARID", 0);
        db.setMaterial(prefix+"SAND_MORE", 0);
        db.setMaterial(prefix+"SAND_BEACH", 0);
        db.setMaterial(prefix+"CONCRETE_BEACH", 0);
        db.setMaterial(prefix+"ROCK_DRY", 0);
        db.setMaterial(prefix+"ROCK_WET", 0);
        db.setMaterial(prefix+"ROCK_CLIFF", 0);
        db.setMaterial(prefix+"WATER_RIVERBED", 0);
        db.setMaterial(prefix+"WATER_SHALLOW", 0);
        db.setMaterial(prefix+"CORNFIELD", 0);
        db.setMaterial(prefix+"HEDGE", 0);
        db.setMaterial(prefix+"WOOD_CRATES", 0);
        db.setMaterial(prefix+"WOOD_SOLID", 0);
        db.setMaterial(prefix+"WOOD_THIN", 0);
        db.setMaterial(prefix+"GLASS", 0);
        db.setMaterial(prefix+"GLASS_WINDOWS_LARGE", 0);
        db.setMaterial(prefix+"GLASS_WINDOWS_SMALL", 0);
        db.setMaterial(prefix+"EMPTY1", 0);
        db.setMaterial(prefix+"EMPTY2", 0);
        db.setMaterial(prefix+"GARAGE_DOOR", 0);
        db.setMaterial(prefix+"THICK_METAL_PLATE", 0);
        db.setMaterial(prefix+"SCAFFOLD_POLE", 0);
        db.setMaterial(prefix+"LAMP_POST", 0);
        db.setMaterial(prefix+"METAL_GATE", 0);
        db.setMaterial(prefix+"METAL_CHAIN_FENCE", 0);
        db.setMaterial(prefix+"GIRDER", 0);
        db.setMaterial(prefix+"FIRE_HYDRANT", 0);
        db.setMaterial(prefix+"CONTAINER", 0);
        db.setMaterial(prefix+"NEWS_VENDOR", 0);
        db.setMaterial(prefix+"WHEELBASE", 0);
        db.setMaterial(prefix+"CARDBOARDBOX", 0);
        db.setMaterial(prefix+"PED", 0);
        db.setMaterial(prefix+"CAR", 0);
        db.setMaterial(prefix+"CAR_PANEL", 0);
        db.setMaterial(prefix+"CAR_MOVINGCOMPONENT", 0);
        db.setMaterial(prefix+"TRANSPARENT_CLOTH", 0);
        db.setMaterial(prefix+"RUBBER", 0);
        db.setMaterial(prefix+"PLASTIC", 0);
        db.setMaterial(prefix+"TRANSPARENT_STONE", 0);
        db.setMaterial(prefix+"WOOD_BENCH", 0);
        db.setMaterial(prefix+"CARPET", 0);
        db.setMaterial(prefix+"FLOORBOARD", 0);
        db.setMaterial(prefix+"STAIRSWOOD", 0);
        db.setMaterial(prefix+"P_SAND", 0);
        db.setMaterial(prefix+"P_SAND_DENSE", 0);
        db.setMaterial(prefix+"P_SAND_ARID", 0);
        db.setMaterial(prefix+"P_SAND_COMPACT", 0);
        db.setMaterial(prefix+"P_SAND_ROCKY", 0);
        db.setMaterial(prefix+"P_SANDBEACH", 0);
        db.setMaterial(prefix+"P_GRASS_SHORT", 0);
        db.setMaterial(prefix+"P_GRASS_MEADOW", 0);
        db.setMaterial(prefix+"P_GRASS_DRY", 0);
        db.setMaterial(prefix+"P_WOODLAND", 0);
        db.setMaterial(prefix+"P_WOODDENSE", 0);
        db.setMaterial(prefix+"P_ROADSIDE", 0);
        db.setMaterial(prefix+"P_ROADSIDEDES", 0);
        db.setMaterial(prefix+"P_FLOWERBED", 0);
        db.setMaterial(prefix+"P_WASTEGROUND", 0);
        db.setMaterial(prefix+"P_CONCRETE", 0);
        db.setMaterial(prefix+"P_OFFICEDESK", 0);
        db.setMaterial(prefix+"P_711SHELF1", 0);
        db.setMaterial(prefix+"P_711SHELF2", 0);
        db.setMaterial(prefix+"P_711SHELF3", 0);
        db.setMaterial(prefix+"P_RESTUARANTTABLE", 0);
        db.setMaterial(prefix+"P_BARTABLE", 0);
        db.setMaterial(prefix+"P_UNDERWATERLUSH", 0);
        db.setMaterial(prefix+"P_UNDERWATERBARREN", 0);
        db.setMaterial(prefix+"P_UNDERWATERCORAL", 0);
        db.setMaterial(prefix+"P_UNDERWATERDEEP", 0);
        db.setMaterial(prefix+"P_RIVERBED", 0);
        db.setMaterial(prefix+"P_RUBBLE", 0);
        db.setMaterial(prefix+"P_BEDROOMFLOOR", 0);
        db.setMaterial(prefix+"P_KIRCHENFLOOR", 0);
        db.setMaterial(prefix+"P_LIVINGRMFLOOR", 0);
        db.setMaterial(prefix+"P_CORRIDORFLOOR", 0);
        db.setMaterial(prefix+"P_711FLOOR", 0);
        db.setMaterial(prefix+"P_FASTFOODFLOOR", 0);
        db.setMaterial(prefix+"P_SKANKYFLOOR", 0);
        db.setMaterial(prefix+"P_MOUNTAIN", 0);
        db.setMaterial(prefix+"P_MARSH", 0);
        db.setMaterial(prefix+"P_BUSHY", 0);
        db.setMaterial(prefix+"P_BUSHYMIX", 0);
        db.setMaterial(prefix+"P_BUSHYDRY", 0);
        db.setMaterial(prefix+"P_BUSHYMID", 0);
        db.setMaterial(prefix+"P_GRASSWEEFLOWERS", 0);
        db.setMaterial(prefix+"P_GRASSDRYTALL", 0);
        db.setMaterial(prefix+"P_GRASSLUSHTALL", 0);
        db.setMaterial(prefix+"P_GRASSGRNMIX", 0);
        db.setMaterial(prefix+"P_GRASSBRNMIX", 0);
        db.setMaterial(prefix+"P_GRASSLOW", 0);
        db.setMaterial(prefix+"P_GRASSROCKY", 0);
        db.setMaterial(prefix+"P_GRASSSMALLTREES", 0);
        db.setMaterial(prefix+"P_DIRTROCKY", 0);
        db.setMaterial(prefix+"P_DIRTWEEDS", 0);
        db.setMaterial(prefix+"P_GRASSWEEDS", 0);
        db.setMaterial(prefix+"P_RIVEREDGE", 0);
        db.setMaterial(prefix+"P_POOLSIDE", 0);
        db.setMaterial(prefix+"P_FORESTSTUMPS", 0);
        db.setMaterial(prefix+"P_FORESTSTICKS", 0);
        db.setMaterial(prefix+"P_FORRESTLEAVES", 0);
        db.setMaterial(prefix+"P_DESERTROCKS", 0);
        db.setMaterial(prefix+"P_FORRESTDRY", 0);
        db.setMaterial(prefix+"P_SPARSEFLOWERS", 0);
        db.setMaterial(prefix+"P_BUILDINGSITE", 0);
        db.setMaterial(prefix+"P_DOCKLANDS", 0);
        db.setMaterial(prefix+"P_INDUSTRIAL", 0);
        db.setMaterial(prefix+"P_INDUSTJETTY", 0);
        db.setMaterial(prefix+"P_CONCRETELITTER", 0);
        db.setMaterial(prefix+"P_ALLEYRUBISH", 0);
        db.setMaterial(prefix+"P_JUNKYARDPILES", 0);
        db.setMaterial(prefix+"P_JUNKYARDGRND", 0);
        db.setMaterial(prefix+"P_DUMP", 0);
        db.setMaterial(prefix+"P_CACTUSDENSE", 0);
        db.setMaterial(prefix+"P_AIRPORTGRND", 0);
        db.setMaterial(prefix+"P_CORNFIELD", 0);
        db.setMaterial(prefix+"P_GRASSLIGHT", 0);
        db.setMaterial(prefix+"P_GRASSLIGHTER", 0);
        db.setMaterial(prefix+"P_GRASSLIGHTER2", 0);
        db.setMaterial(prefix+"P_GRASSMID1", 0);
        db.setMaterial(prefix+"P_GRASSMID2", 0);
        db.setMaterial(prefix+"P_GRASSDARK", 0);
        db.setMaterial(prefix+"P_GRASSDARK2", 0);
        db.setMaterial(prefix+"P_GRASSDIRTMIX", 0);
        db.setMaterial(prefix+"P_RIVERBEDSTONE", 0);
        db.setMaterial(prefix+"P_RIVERBEDSHALLOW", 0);
        db.setMaterial(prefix+"P_RIVERBEDWEEDS", 0);
        db.setMaterial(prefix+"P_SEAWEED", 0);
        db.setMaterial(prefix+"DOOR", 0);
        db.setMaterial(prefix+"PLASTICBARRIER", 0);
        db.setMaterial(prefix+"PARKGRASS", 0);
        db.setMaterial(prefix+"STAIRSSTONE", 0);
        db.setMaterial(prefix+"STAIRSMETAL", 0);
        db.setMaterial(prefix+"STAIRSCARPET", 0);
        db.setMaterial(prefix+"FLOORMETAL", 0);
        db.setMaterial(prefix+"FLOORCONCRETE", 0);
        db.setMaterial(prefix+"BIN_BAG", 0);
        db.setMaterial(prefix+"THIN_METAL_SHEET", 0);
        db.setMaterial(prefix+"METAL_BARREL", 0);
        db.setMaterial(prefix+"PLASTIC_CONE", 0);
        db.setMaterial(prefix+"PLASTIC_DUMPSTER", 0);
        db.setMaterial(prefix+"METAL_DUMPSTER", 0);
        db.setMaterial(prefix+"WOOD_PICKET_FENCE", 0);
        db.setMaterial(prefix+"WOOD_SLATTED_FENCE", 0);
        db.setMaterial(prefix+"WOOD_RANCH_FENCE", 0);
        db.setMaterial(prefix+"UNBREAKABLE_GLASS", 0);
        db.setMaterial(prefix+"HAY_BALE", 0);
        db.setMaterial(prefix+"GORE", 0);
        db.setMaterial(prefix+"RAILTRACK", 0);
        db.setMaterial("/common/Metal", 0);
}


static inline int tolowr (int c)
{
    return std::tolower(char(c),std::cout.getloc());
}

static inline std::string& strlower(std::string& s)
{
    std::transform(s.begin(),s.end(), s.begin(),tolowr);
    return s;
}

void parse_col (std::string &name,
                std::istream &in,
                TColFile &tcol,
                int debug_level)
{
        unsigned long fourcc;
        fourcc = ios_read_u32(in);

        tcol.mass = 0;
        tcol.inertia_x = 0;
        tcol.inertia_y = 0;
        tcol.inertia_z = 0;
        tcol.linearDamping = 0;
        tcol.angularDamping = 0.5;
        tcol.linearSleepThreshold = 1;
        tcol.angularSleepThreshold = 0.8;
        tcol.ccdMotionThreshold = 0;
        tcol.ccdSweptSphereRadius = 0;


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
                        sphere.material = ios_read_u8(in) & 0xFF;
                        unsigned char flag = ios_read_u8(in);
                        unsigned char unk = ios_read_u8(in);
                        unsigned char light = ios_read_u8(in);
                        //std::cout<<"sphere mat: "<<sphere.material<<std::endl;
                        //std::cout<<"sphere flag: "<<(int)flag<<std::endl;
                        //std::cout<<"sphere unk: "<<(int)unk<<std::endl;
                        //std::cout<<"sphere light: "<<(int)light<<std::endl;
                        (void) flag; // not used yet
                        (void) unk; // not used yet
                        (void) light; // not used yet
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
                        box.material = ios_read_u8(in);
                        unsigned char flag = ios_read_u8(in);
                        unsigned char unk = ios_read_u8(in);
                        unsigned char light = ios_read_u8(in);
                        //std::cout<<"box mat: "<<box.material<<std::endl;
                        //std::cout<<"box flag: "<<(int)flag<<std::endl;
                        //std::cout<<"box unk: "<<(int)unk<<std::endl;
                        //std::cout<<"box light: "<<(int)light<<std::endl;
                        (void) flag; // not used yet
                        (void) unk; // not used yet
                        (void) light; // not used yet
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
                        tcol.triMesh.vertexes.push_back(Vector3(x,y,z));
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
                        unsigned char light = ios_read_u8(in);
                        //std::cout<<"face mat: "<<(int)mat<<std::endl;
                        //std::cout<<"face light: "<<(int)light<<std::endl;
                        (void) light; // not used yet
                        tcol.triMesh.faces.push_back(Face(a,b,c,mat));
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

                        pretty_print_tcol(out,tcol,db);
                }
                
                std::istream::int_type next = in.peek();

                // no more cols
                if (next==std::istream::traits_type::eof() || next==0) break; 

        } while (true);
}



// vim: shiftwidth=8:tabstop=8:expandtab
