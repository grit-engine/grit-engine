/* Copyright Copyright (c) David Cunningham and the Grit Game Engine project 2012
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
#include <algorithm>
#include <iostream>
#include <fstream>

#include "ColParser.h"
#include "ios_util.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

MaterialMap global_db;

// This is called by the standalone tools (colread && dffread)
// extract.exe on the other hand reads surface.dat
// Don't want to read surface.dat in colread or dffread as we want a single input dff
// The output of such tools is not usually used as an input to grit, it is mainly
// for verifying or debugging the gta format parsing code, and using verbose output
// to inspect the gta dataset
void init_global_db ()
{
        int counter = 0;
        global_db[counter++] = "DEFAULT";
        global_db[counter++] = "TARMAC";
        global_db[counter++] = "TARMAC_FUCKED";
        global_db[counter++] = "TARMAC_REALLYFUCKED";
        global_db[counter++] = "PAVEMENT";
        global_db[counter++] = "PAVEMENT_FUCKED";
        global_db[counter++] = "GRAVEL";
        global_db[counter++] = "FUCKED_CONCRETE";
        global_db[counter++] = "PAINTED_GROUND";
        global_db[counter++] = "GRASS_SHORT_LUSH";
        global_db[counter++] = "GRASS_MEDIUM_LUSH";
        global_db[counter++] = "GRASS_LONG_LUSH";
        global_db[counter++] = "GRASS_SHORT_DRY";
        global_db[counter++] = "GRASS_MEDIUM_DRY";
        global_db[counter++] = "GRASS_LONG_DRY";
        global_db[counter++] = "GOLFGRASS_ROUGH";
        global_db[counter++] = "GOLFGRASS_SMOOTH";
        global_db[counter++] = "STEEP_SLIDYGRASS";
        global_db[counter++] = "STEEP_CLIFF";
        global_db[counter++] = "FLOWERBED";
        global_db[counter++] = "MEADOW";
        global_db[counter++] = "WASTEGROUND";
        global_db[counter++] = "WOODLANDGROUND";
        global_db[counter++] = "VEGETATION";
        global_db[counter++] = "MUD_WET";
        global_db[counter++] = "MUD_DRY";
        global_db[counter++] = "DIRT";
        global_db[counter++] = "DIRTTRACK";
        global_db[counter++] = "SAND_DEEP";
        global_db[counter++] = "SAND_MEDIUM";
        global_db[counter++] = "SAND_COMPACT";
        global_db[counter++] = "SAND_ARID";
        global_db[counter++] = "SAND_MORE";
        global_db[counter++] = "SAND_BEACH";
        global_db[counter++] = "CONCRETE_BEACH";
        global_db[counter++] = "ROCK_DRY";
        global_db[counter++] = "ROCK_WET";
        global_db[counter++] = "ROCK_CLIFF";
        global_db[counter++] = "WATER_RIVERBED";
        global_db[counter++] = "WATER_SHALLOW";
        global_db[counter++] = "CORNFIELD";
        global_db[counter++] = "HEDGE";
        global_db[counter++] = "WOOD_CRATES";
        global_db[counter++] = "WOOD_SOLID";
        global_db[counter++] = "WOOD_THIN";
        global_db[counter++] = "GLASS";
        global_db[counter++] = "GLASS_WINDOWS_LARGE";
        global_db[counter++] = "GLASS_WINDOWS_SMALL";
        global_db[counter++] = "EMPTY1";
        global_db[counter++] = "EMPTY2";
        global_db[counter++] = "GARAGE_DOOR";
        global_db[counter++] = "THICK_METAL_PLATE";
        global_db[counter++] = "SCAFFOLD_POLE";
        global_db[counter++] = "LAMP_POST";
        global_db[counter++] = "METAL_GATE";
        global_db[counter++] = "METAL_CHAIN_FENCE";
        global_db[counter++] = "GIRDER";
        global_db[counter++] = "FIRE_HYDRANT";
        global_db[counter++] = "CONTAINER";
        global_db[counter++] = "NEWS_VENDOR";
        global_db[counter++] = "WHEELBASE";
        global_db[counter++] = "CARDBOARDBOX";
        global_db[counter++] = "PED";
        global_db[counter++] = "CAR";
        global_db[counter++] = "CAR_PANEL";
        global_db[counter++] = "CAR_MOVINGCOMPONENT";
        global_db[counter++] = "TRANSPARENT_CLOTH";
        global_db[counter++] = "RUBBER";
        global_db[counter++] = "PLASTIC";
        global_db[counter++] = "TRANSPARENT_STONE";
        global_db[counter++] = "WOOD_BENCH";
        global_db[counter++] = "CARPET";
        global_db[counter++] = "FLOORBOARD";
        global_db[counter++] = "STAIRSWOOD";
        global_db[counter++] = "P_SAND";
        global_db[counter++] = "P_SAND_DENSE";
        global_db[counter++] = "P_SAND_ARID";
        global_db[counter++] = "P_SAND_COMPACT";
        global_db[counter++] = "P_SAND_ROCKY";
        global_db[counter++] = "P_SANDBEACH";
        global_db[counter++] = "P_GRASS_SHORT";
        global_db[counter++] = "P_GRASS_MEADOW";
        global_db[counter++] = "P_GRASS_DRY";
        global_db[counter++] = "P_WOODLAND";
        global_db[counter++] = "P_WOODDENSE";
        global_db[counter++] = "P_ROADSIDE";
        global_db[counter++] = "P_ROADSIDEDES";
        global_db[counter++] = "P_FLOWERBED";
        global_db[counter++] = "P_WASTEGROUND";
        global_db[counter++] = "P_CONCRETE";
        global_db[counter++] = "P_OFFICEDESK";
        global_db[counter++] = "P_711SHELF1";
        global_db[counter++] = "P_711SHELF2";
        global_db[counter++] = "P_711SHELF3";
        global_db[counter++] = "P_RESTUARANTTABLE";
        global_db[counter++] = "P_BARTABLE";
        global_db[counter++] = "P_UNDERWATERLUSH";
        global_db[counter++] = "P_UNDERWATERBARREN";
        global_db[counter++] = "P_UNDERWATERCORAL";
        global_db[counter++] = "P_UNDERWATERDEEP";
        global_db[counter++] = "P_RIVERBED";
        global_db[counter++] = "P_RUBBLE";
        global_db[counter++] = "P_BEDROOMFLOOR";
        global_db[counter++] = "P_KIRCHENFLOOR";
        global_db[counter++] = "P_LIVINGRMFLOOR";
        global_db[counter++] = "P_CORRIDORFLOOR";
        global_db[counter++] = "P_711FLOOR";
        global_db[counter++] = "P_FASTFOODFLOOR";
        global_db[counter++] = "P_SKANKYFLOOR";
        global_db[counter++] = "P_MOUNTAIN";
        global_db[counter++] = "P_MARSH";
        global_db[counter++] = "P_BUSHY";
        global_db[counter++] = "P_BUSHYMIX";
        global_db[counter++] = "P_BUSHYDRY";
        global_db[counter++] = "P_BUSHYMID";
        global_db[counter++] = "P_GRASSWEEFLOWERS";
        global_db[counter++] = "P_GRASSDRYTALL";
        global_db[counter++] = "P_GRASSLUSHTALL";
        global_db[counter++] = "P_GRASSGRNMIX";
        global_db[counter++] = "P_GRASSBRNMIX";
        global_db[counter++] = "P_GRASSLOW";
        global_db[counter++] = "P_GRASSROCKY";
        global_db[counter++] = "P_GRASSSMALLTREES";
        global_db[counter++] = "P_DIRTROCKY";
        global_db[counter++] = "P_DIRTWEEDS";
        global_db[counter++] = "P_GRASSWEEDS";
        global_db[counter++] = "P_RIVEREDGE";
        global_db[counter++] = "P_POOLSIDE";
        global_db[counter++] = "P_FORESTSTUMPS";
        global_db[counter++] = "P_FORESTSTICKS";
        global_db[counter++] = "P_FORRESTLEAVES";
        global_db[counter++] = "P_DESERTROCKS";
        global_db[counter++] = "P_FORRESTDRY";
        global_db[counter++] = "P_SPARSEFLOWERS";
        global_db[counter++] = "P_BUILDINGSITE";
        global_db[counter++] = "P_DOCKLANDS";
        global_db[counter++] = "P_INDUSTRIAL";
        global_db[counter++] = "P_INDUSTJETTY";
        global_db[counter++] = "P_CONCRETELITTER";
        global_db[counter++] = "P_ALLEYRUBISH";
        global_db[counter++] = "P_JUNKYARDPILES";
        global_db[counter++] = "P_JUNKYARDGRND";
        global_db[counter++] = "P_DUMP";
        global_db[counter++] = "P_CACTUSDENSE";
        global_db[counter++] = "P_AIRPORTGRND";
        global_db[counter++] = "P_CORNFIELD";
        global_db[counter++] = "P_GRASSLIGHT";
        global_db[counter++] = "P_GRASSLIGHTER";
        global_db[counter++] = "P_GRASSLIGHTER2";
        global_db[counter++] = "P_GRASSMID1";
        global_db[counter++] = "P_GRASSMID2";
        global_db[counter++] = "P_GRASSDARK";
        global_db[counter++] = "P_GRASSDARK2";
        global_db[counter++] = "P_GRASSDIRTMIX";
        global_db[counter++] = "P_RIVERBEDSTONE";
        global_db[counter++] = "P_RIVERBEDSHALLOW";
        global_db[counter++] = "P_RIVERBEDWEEDS";
        global_db[counter++] = "P_SEAWEED";
        global_db[counter++] = "DOOR";
        global_db[counter++] = "PLASTICBARRIER";
        global_db[counter++] = "PARKGRASS";
        global_db[counter++] = "STAIRSSTONE";
        global_db[counter++] = "STAIRSMETAL";
        global_db[counter++] = "STAIRSCARPET";
        global_db[counter++] = "FLOORMETAL";
        global_db[counter++] = "FLOORCONCRETE";
        global_db[counter++] = "BIN_BAG";
        global_db[counter++] = "THIN_METAL_SHEET";
        global_db[counter++] = "METAL_BARREL";
        global_db[counter++] = "PLASTIC_CONE";
        global_db[counter++] = "PLASTIC_DUMPSTER";
        global_db[counter++] = "METAL_DUMPSTER";
        global_db[counter++] = "WOOD_PICKET_FENCE";
        global_db[counter++] = "WOOD_SLATTED_FENCE";
        global_db[counter++] = "WOOD_RANCH_FENCE";
        global_db[counter++] = "UNBREAKABLE_GLASS";
        global_db[counter++] = "HAY_BALE";
        global_db[counter++] = "GORE";
        global_db[counter++] = "RAILTRACK";
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
                const std::string &phys_mat_pref,
                MaterialMap &db,
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
        tcol.angularSleepThreshold = 0.8f;
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
                        GRIT_EXCEPT("This is not a col file.");
        }

        if (debug_level>0)
                std::cout << "Version: " << version << std::endl;

        APP_ASSERT(version==3 || version==2);

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
                APP_ASSERT((flags | 2 | 8 | 16) == 26);
                unsigned long offset_spheres = ios_read_u32(in);
                (void) offset_spheres;
                if (debug_level>2)
                    std::cout<<"offset_spheres: "<<offset_spheres<<std::endl;
                unsigned long offset_boxes = ios_read_u32(in);
                (void) offset_boxes;
                if (debug_level>2)
                    std::cout<<"offset_boxes: "<<offset_boxes<<std::endl;
                unsigned long unknown = ios_read_u32(in);
                APP_ASSERT(unknown==0);
                unsigned long offset_vertexes = ios_read_u32(in);
                if (debug_level>2)
                    std::cout<<"offset_vertexes: "<<offset_vertexes<<std::endl;
                unsigned long offset_faces = ios_read_u32(in);
                if (debug_level>2)
                    std::cout<<"offset_faces: "<<offset_faces<<std::endl;
                unsigned long unknown2 = ios_read_u32(in);
                APP_ASSERT(unknown2==0);

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
                        TColSphere sphere;
                        sphere.px = ios_read_float(in);
                        sphere.py = ios_read_float(in);
                        sphere.pz = ios_read_float(in);
                        sphere.radius = ios_read_float(in);
                        sphere.material = phys_mat_pref+db[ios_read_u8(in)];
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
                        TColBox box;
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
                        box.margin = 0.04f;
                        box.material = phys_mat_pref+db[ios_read_u8(in)];
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
                        float x = ios_read_s16(in)/128.0f;
                        float y = ios_read_s16(in)/128.0f;
                        float z = ios_read_s16(in)/128.0f;
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
                        tcol.triMesh.faces.push_back(TColFace(a,b,c,phys_mat_pref+db[mat]));
                }

                if (num_faces > 0) {
                        if (debug_level>1)
                            std::cout<<"max_vertex: "<<max_vertex<<std::endl;
                        APP_ASSERT(max_vertex<num_vertexes);
                        tcol.triMesh.vertexes.resize(max_vertex+1);
                }

                in.seekg(offset_shadow_faces - offset_shadow_vertexes,
                         std::ios_base::cur);

                in.seekg(num_shadow_faces*8, std::ios_base::cur);

        } else {

                APP_ASSERT(false);
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

void dump_all_cols (std::istream &in, bool binary, const std::string &phys_mat_pref,
                    MaterialMap &db, int debug_level)
{
        do {
                TColFile tcol;
                std::string name;
                parse_col(name,in,tcol,phys_mat_pref,db,debug_level);

                if (!tcol.usingCompound && !tcol.usingTriMesh) {
                        if (debug_level>0)
                                std::cout<<"Col is empty."<<std::endl;
                        // skip empty cols.  They are pure lol.
                } else if (binary) {
                        name += ".bcol";
                        GRIT_EXCEPT("Writing bcol not implemented.");
                } else {
                        name += ".tcol";

                        std::ofstream out;
                        out.open(name.c_str(), std::ios::binary);
                        APP_ASSERT_IO_SUCCESSFUL(out,"opening tcol for writing");

                        pretty_print_tcol(out,tcol);
                }
                
                std::istream::int_type next = in.peek();

                // no more cols
                if (next==std::istream::traits_type::eof() || next==0) break; 

        } while (true);
}



// vim: shiftwidth=8:tabstop=8:expandtab
