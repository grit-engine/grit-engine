#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>

#include "imgread.h"
#include "iplread.h"
#include "ideread.h"
#include "tex_dups.h"
#include "dffread.h"
#include "txdread.h"
#include "TColParser.h"
#include "ColParser.h"

#include "ios_util.h"
#include "dirutil.h"

 
void dump_stats (std::ostream &out, Objs &objs) //{{{
{
        unsigned long num_wet = 0;
        unsigned long num_night = 0;
        unsigned long num_alpha1 = 0;
        unsigned long num_alpha2 = 0;
        unsigned long num_day = 0;
        unsigned long num_interior = 0;
        unsigned long num_no_shadow = 0;
        unsigned long num_no_col = 0;
        unsigned long num_no_draw_dist = 0;
        unsigned long num_break_glass = 0;
        unsigned long num_break_glass_crack = 0;
        unsigned long num_garage_door = 0;
        unsigned long num_2clump = 0;
        unsigned long num_sways = 0;
        unsigned long num_other_veg = 0;
        unsigned long num_pole_shadow = 0;
        unsigned long num_explosive = 0;
        unsigned long num_unk1 = 0;
        unsigned long num_unk2 = 0;
        unsigned long num_unk3 = 0;
        unsigned long num_grafitti = 0;
        unsigned long num_draw_backface = 0;
        unsigned long num_unk4 = 0;

        for (Objs::iterator i=objs.begin(),i_=objs.end() ; i!=i_ ; ++i) {
                Obj &obj = *i;
                unsigned long flags = obj.flags;
                if (flags & OBJ_FLAG_WET) num_wet++;
                if (flags & OBJ_FLAG_NIGHT) num_night++;
                if (flags & OBJ_FLAG_ALPHA1) num_alpha1++;
                if (flags & OBJ_FLAG_ALPHA2) num_alpha2++;
                if (flags & OBJ_FLAG_DAY) num_day++;
                if (flags & OBJ_FLAG_INTERIOR) num_interior++;
                if (flags & OBJ_FLAG_NO_SHADOW) num_no_shadow++;
                if (flags & OBJ_FLAG_NO_COL) num_no_col++;
                if (flags & OBJ_FLAG_NO_DRAW_DIST) num_no_draw_dist++;
                if (flags & OBJ_FLAG_BREAK_GLASS) num_break_glass++;
                if (flags & OBJ_FLAG_BREAK_GLASS_CRACK) num_break_glass_crack++;
                if (flags & OBJ_FLAG_GARAGE_DOOR) num_garage_door++;
                if (flags & OBJ_FLAG_2CLUMP) num_2clump++;
                if (flags & OBJ_FLAG_SWAYS) num_sways++;
                if (flags & OBJ_FLAG_OTHER_VEG) num_other_veg++;
                if (flags & OBJ_FLAG_POLE_SHADOW) num_pole_shadow++;
                if (flags & OBJ_FLAG_EXPLOSIVE) num_explosive++;
                if (flags & OBJ_FLAG_UNK1) num_unk1++;
                if (flags & OBJ_FLAG_UNK2) num_unk2++;
                if (flags & OBJ_FLAG_UNK3) num_unk3++;
                if (flags & OBJ_FLAG_GRAFITTI) num_grafitti++;
                if (flags & OBJ_FLAG_DRAW_BACKFACE) num_draw_backface++;
                if (flags & OBJ_FLAG_UNK4) num_unk4++;

        }

        out << "num_wet " << num_wet << std::endl;
        out << "num_night " << num_night << std::endl;
        out << "num_alpha1 " << num_alpha1 << std::endl;
        out << "num_alpha2 " << num_alpha2 << std::endl;
        out << "num_day " << num_day << std::endl;
        out << "num_interior " << num_interior << std::endl;
        out << "num_no_shadow " << num_no_shadow << std::endl;
        out << "num_no_col " << num_no_col << std::endl;
        out << "num_no_draw_dist " << num_no_draw_dist << std::endl;
        out << "num_break_glass " << num_break_glass << std::endl;
        out << "num_break_glass_crack " << num_break_glass_crack << std::endl;
        out << "num_garage_door " << num_garage_door << std::endl;
        out << "num_2clump " << num_2clump << std::endl;
        out << "num_sways " << num_sways << std::endl;
        out << "num_other_veg " << num_other_veg << std::endl;
        out << "num_pole_shadow " << num_pole_shadow << std::endl;
        out << "num_explosive " << num_explosive << std::endl;
        out << "num_unk1 " << num_unk1 << std::endl;
        out << "num_unk2 " << num_unk2 << std::endl;
        out << "num_unk3 " << num_unk3 << std::endl;
        out << "num_grafitti " << num_grafitti << std::endl;
        out << "num_draw_backface " << num_draw_backface << std::endl;
        out << "num_unk4 " << num_unk4 << std::endl;
}

//}}}

static void open_file (std::ostream &out, std::ifstream &f, const std::string fname)
{
        (void)out;
        //out << "Opening: \""+fname+"\"" << std::endl;
        f.open(fname.c_str(), std::ios::binary);
        ASSERT_IO_SUCCESSFUL(f,"opening "+fname);
}

static void open_file_img (std::ostream &out, std::ifstream &f,
                           const Img &img, const std::string fname)
{
        (void)out;
        //out << "Opening (from img): \""+fname+"\"" << std::endl;
        img.fileOffset(f,fname);
}

typedef std::vector<IPL> IPLs;

static void addFullIPL (std::ostream &out,
                        const std::string &gta_dir,
                        IPLs &ipls,
                        const char *text,
                        const Img &img,
                        std::ifstream &img_f,
                        const char *bin,
                        size_t n)
{
        ipls.push_back(IPL());
        
        IPL &ipl = *(ipls.end()-1);

        ipl.setName(text);

        std::string name = gta_dir + "/data/maps/" + text;
        std::ifstream text_f;
        text_f.open(name.c_str(),std::ios::binary);
        ASSERT_IO_SUCCESSFUL(text_f,"opening text IPL: "+name);
        
        ipl.addMore(text_f);

        for (size_t i=0 ; i<n ; ++i) {

                std::stringstream ss;
                ss<<bin<<"_stream"<<i<<".ipl";

                open_file_img(out,img_f,img,ss.str());
                
                ipl.addMore(img_f);
        }

}

typedef std::map<std::string,Txd> TxdDir;

void process_txds (std::ostream &out,
                   Txd::Names &texs,
                   const Img &img,
                   std::istream &imgf,
                   const std::string &dest_dir,
                   const std::string &imgname)
{
        (void) out;
        for (unsigned int i=0 ; i<img.size(); ++i) {
                const std::string &fname = img.fileName(i);
                if (fname.size()<4) continue;
                std::string ext = fname.substr(fname.size()-4,4);
                if (ext!=".txd") continue;
                //out<<"Extracting: "<<imgname<<"/"<<fname<<std::endl;
                img.fileOffset(imgf,i);
                std::string txddir = dest_dir+"/"+imgname+"/"+fname;
                ensuredir(txddir);
                Txd txd(imgf,txddir);
                const Txd::Names &n = txd.getNames();
                for (Txd::Names::iterator j=n.begin(),j_=n.end();j!=j_;++j) {
                        const std::string &texname = *j;
                        texs.insert(imgname+"/"+fname+"/"+texname+".dds");
                }
        }
}


typedef std::set<std::string> ColNames;

void process_cols (std::ostream &out,
                   ColNames &cols,
                   ColNames &cols_including_empty,
                   const Img &img,
                   std::istream &imgf,
                   const std::string &dest_dir,
                   const std::string &imgname)
{
        (void) out;
        for (unsigned int i=0 ; i<img.size(); ++i) {
                const std::string &fname = img.fileName(i);
                if (fname.size()<4) continue;
                std::string ext = fname.substr(fname.size()-4,4);
                if (ext!=".col") continue;
                //out<<"Extracting: "<<imgname<<"/"<<fname<<std::endl;
                img.fileOffset(imgf,i);

                std::istream::int_type next;

                do {

                        TColFile tcol;
                        std::string name;
                        parse_col(name,imgf,tcol);

                        cols_including_empty.insert(imgname+"/"+name+".tcol");

                        if (tcol.usingCompound || tcol.usingTriMesh) {

                                cols.insert(imgname+"/"+name+".tcol");

                                name = dest_dir+"/"+imgname+"/"+name+".tcol";

                                std::ofstream out;
                                out.open(name.c_str(), std::ios::binary);
                                ASSERT_IO_SUCCESSFUL(out,"opening tcol "
                                                         "for writing");

                                pretty_print_tcol(out,tcol);
                        }

                        next = imgf.peek();

                } while (next!=std::istream::traits_type::eof() && next!=0);

                // no more cols

        }
}


void extract (const std::string &gta_dir,
              const std::string &dest_dir, std::ostream &out)
{ 

        // ides
        const char *ides[] = { //{{{
                "maps/country/countn2.ide",
                "maps/country/countrye.ide",
                "maps/country/countryN.ide",
                "maps/country/countryS.ide",
                "maps/country/countryW.ide",
                "maps/country/counxref.ide",
                "maps/generic/barriers.ide",
                "maps/generic/dynamic.ide",
                "maps/generic/dynamic2.ide",
                "maps/generic/multiobj.ide",
                "maps/generic/procobj.ide",
                "maps/generic/vegepart.ide",
                "maps/interior/gen_int1.ide",
                "maps/interior/gen_int2.ide",
                "maps/interior/gen_int3.ide",
                "maps/interior/gen_int4.ide",
                "maps/interior/gen_int5.ide",
                "maps/interior/gen_intb.ide",
                "maps/interior/int_cont.ide",
                "maps/interior/int_LA.ide",
                "maps/interior/int_SF.ide",
                "maps/interior/int_veg.ide",
                "maps/interior/propext.ide",
                "maps/interior/props.ide",
                "maps/interior/props2.ide",
                "maps/interior/savehous.ide",
                "maps/interior/stadint.ide",
                "maps/LA/LAe.ide",
                "maps/LA/LAe2.ide",
                "maps/LA/LAhills.ide",
                "maps/LA/LAn.ide",
                "maps/LA/LAn2.ide",
                "maps/LA/LAs.ide",
                "maps/LA/LAs2.ide",
                "maps/LA/LAw.ide",
                "maps/LA/LAw2.ide",
                "maps/LA/LaWn.ide",
                "maps/LA/LAxref.ide",
                "maps/leveldes/levelmap.ide",
                "maps/leveldes/levelxre.ide",
                "maps/leveldes/seabed.ide",
                "maps/SF/SFe.ide",
                "maps/SF/SFn.ide",
                "maps/SF/SFs.ide",
                "maps/SF/SFSe.ide",
                "maps/SF/SFw.ide",
                "maps/SF/SFxref.ide",
                "maps/txd.ide",
                "maps/vegas/vegasE.ide",
                "maps/vegas/VegasN.ide",
                "maps/vegas/VegasS.ide",
                "maps/vegas/VegasW.ide",
                "maps/vegas/vegaxref.ide",
                "maps/veh_mods/veh_mods.ide",
                "vehicles.ide",
                "peds.ide",
                "default.ide"
        };
        const int num_ides = sizeof ides / sizeof *ides; //}}}

        ide everything;
        
        for (int i=0 ; i<num_ides ; ++i) {
                std::ifstream f;
                open_file(out, f, gta_dir+"/data/"+ides[i]);
                read_ide(f, &everything);
        }

        // imgs
        std::ifstream gta3_f;
        open_file(out, gta3_f, gta_dir+"/models/gta3.img");
        Img gta3(gta3_f);

        std::ifstream gta_int_f;
        open_file(out, gta_int_f, gta_dir+"/models/gta_int.img");
        Img gta_int(gta_int_f);

        // txds
        Txd::Names texs;
        process_txds(out,texs,gta3,gta3_f,dest_dir,"gta3.img");
        process_txds(out,texs,gta_int,gta_int_f,dest_dir,"gta_int.img");

        ColNames cols, cols_i; // cols_i is with the empty cols
        process_cols(out,cols,cols_i,gta3,gta3_f,dest_dir,"gta3.img");
        process_cols(out,cols,cols_i,gta_int,gta_int_f,dest_dir,"gta_int.img");

        // ipls
        std::vector<IPL> ipls;
        ipls.reserve(43); // the list is below

        //{{{
        addFullIPL(out,gta_dir,ipls,"leveldes/seabed.ipl",gta3,gta3_f,"seabed",1);
        addFullIPL(out,gta_dir,ipls,"leveldes/levelmap.ipl",gta3,gta3_f,"levelmap",1);
        addFullIPL(out,gta_dir,ipls,"SF/SFe.ipl",gta3,gta3_f,"sfe",4);
        addFullIPL(out,gta_dir,ipls,"SF/SFn.ipl",gta3,gta3_f,"sfn",3);
        addFullIPL(out,gta_dir,ipls,"SF/SFSe.ipl",gta3,gta3_f,"sfse",7);
        addFullIPL(out,gta_dir,ipls,"SF/SFs.ipl",gta3,gta3_f,"sfs",9);
        addFullIPL(out,gta_dir,ipls,"SF/SFw.ipl",gta3,gta3_f,"sfw",6);
        addFullIPL(out,gta_dir,ipls,"vegas/vegasE.ipl",gta3,gta3_f,"vegase",9);
        addFullIPL(out,gta_dir,ipls,"vegas/vegasN.ipl",gta3,gta3_f,"vegasn",9);
        addFullIPL(out,gta_dir,ipls,"vegas/vegasS.ipl",gta3,gta3_f,"vegass",6);
        addFullIPL(out,gta_dir,ipls,"vegas/vegasW.ipl",gta3,gta3_f,"vegasw",12);
        addFullIPL(out,gta_dir,ipls,"country/countn2.ipl",gta3,gta3_f,"countn2",9);
        addFullIPL(out,gta_dir,ipls,"country/countrye.ipl",gta3,gta3_f,"countrye",14);
        addFullIPL(out,gta_dir,ipls,"country/countryN.ipl",gta3,gta3_f,"countryn",4);
        addFullIPL(out,gta_dir,ipls,"country/countryS.ipl",gta3,gta3_f,"countrys",5);
        addFullIPL(out,gta_dir,ipls,"country/countryw.ipl",gta3,gta3_f,"countryw",9);
        addFullIPL(out,gta_dir,ipls,"LA/LAe2.ipl",gta3,gta3_f,"lae2",7);
        addFullIPL(out,gta_dir,ipls,"LA/LAe.ipl",gta3,gta3_f,"lae",6);
        addFullIPL(out,gta_dir,ipls,"LA/LAhills.ipl",gta3,gta3_f,"lahills",5);
        addFullIPL(out,gta_dir,ipls,"LA/LAn2.ipl",gta3,gta3_f,"lan2",4);
        addFullIPL(out,gta_dir,ipls,"LA/LAn.ipl",gta3,gta3_f,"lan",3);
        addFullIPL(out,gta_dir,ipls,"LA/LAs2.ipl",gta3,gta3_f,"las2",5);
        addFullIPL(out,gta_dir,ipls,"LA/LAs.ipl",gta3,gta3_f,"las",6);
        addFullIPL(out,gta_dir,ipls,"LA/LAw2.ipl",gta3,gta3_f,"law2",5);
        addFullIPL(out,gta_dir,ipls,"LA/LAw.ipl",gta3,gta3_f,"law",6);
        addFullIPL(out,gta_dir,ipls,"LA/LaWn.ipl",gta3,gta3_f,"lawn",4);
        addFullIPL(out,gta_dir,ipls,"interior/gen_int1.ipl",gta_int,gta_int_f,"",0);
        addFullIPL(out,gta_dir,ipls,"interior/gen_int2.ipl",gta_int,gta_int_f,"gen_int2",1);
        addFullIPL(out,gta_dir,ipls,"interior/gen_int3.ipl",gta_int,gta_int_f,"gen_int3",1);
        addFullIPL(out,gta_dir,ipls,"interior/gen_int4.ipl",gta_int,gta_int_f,"gen_int4",3);
        addFullIPL(out,gta_dir,ipls,"interior/gen_int5.ipl",gta_int,gta_int_f,"gen_int5",8);
        addFullIPL(out,gta_dir,ipls,"interior/gen_intb.ipl",gta_int,gta_int_f,"gen_intb",1);
        addFullIPL(out,gta_dir,ipls,"interior/int_cont.ipl",gta_int,gta_int_f,"",0);
        addFullIPL(out,gta_dir,ipls,"interior/int_LA.ipl",gta_int,gta_int_f,"int_la",4);
        addFullIPL(out,gta_dir,ipls,"interior/int_SF.ipl",gta_int,gta_int_f,"int_sf",1);
        addFullIPL(out,gta_dir,ipls,"interior/int_veg.ipl",gta_int,gta_int_f,"int_veg",4);
        addFullIPL(out,gta_dir,ipls,"interior/savehous.ipl",gta_int,gta_int_f,"savehous",2);
        addFullIPL(out,gta_dir,ipls,"interior/stadint.ipl",gta_int,gta_int_f,"stadint",1);
        //}}}


        std::ofstream map;
        map.open((dest_dir+"/map.lua").c_str(), std::ios::binary);
        ASSERT_IO_SUCCESSFUL(map, "opening map.lua");
        map.precision(25);

        map << "print(\"Loading GTA San Andreas world\")\n";

        map << "local last\n";

        std::vector<bool> used_ids;

        for (IPLs::iterator i=ipls.begin(),i_=ipls.end() ; i!=i_ ; ++i) {
                const IPL &ipl = *i;
                const Insts &insts = ipl.getInsts();
                for (Insts::const_iterator j=insts.begin(),j_=insts.end() ;
                     j!=j_ ; ++j) {
                        const Inst &inst = *j;
                        unsigned long id = inst.id;
                        if (used_ids.size()<id+1) {
                                used_ids.resize(id+1);
                        }
                        used_ids[id] = true;
                        if (inst.is_low_detail) continue;
                        if (inst.near_for==-1) {
                                map<<"tryAdd(\""<<inst.id<<"\","
                                   <<inst.x<<","<<inst.y<<","<<inst.z;
                                if (inst.rx!=0 || inst.ry!=0 || inst.rz!=0) {
                                        map<<",{rot=Quat("
                                           <<inst.rw<<","<<inst.rx<<","
                                           <<inst.ry<<","<<inst.rz<<")}";
                                }
                                map<<")\n";
                        } else {
                                const Inst &far = insts[inst.near_for];
                                map<<"last=tryAdd(\""<<inst.id<<"\","
                                   <<inst.x<<","<<inst.y<<","<<inst.z;
                                if (inst.rx!=0 || inst.ry!=0 || inst.rz!=0) {
                                        map<<",{rot=Quat("
                                           <<inst.rw<<","<<inst.rx<<","
                                           <<inst.ry<<","<<inst.rz<<")}";
                                }
                                map<<")\n";
                                map<<"tryAdd(\""<<far.id<<"\","
                                   <<far.x<<","<<far.y<<","<<far.z;
                                map<<",{";
                                if (far.rx!=0 || far.ry!=0 || far.rz!=0) {
                                        map<<"rot=Quat("
                                           <<far.rw<<","<<far.rx<<","
                                           <<far.ry<<","<<far.rz<<"),";
                                }
                                map<<"near=last}";
                                map<<")\n";
                        }
                }
        }

        MatDB matdb;

        std::ofstream grit_classes;
        grit_classes.open((dest_dir+"/grit_classes.lua").c_str(),
                          std::ios::binary);
        ASSERT_IO_SUCCESSFUL(grit_classes, "opening grit_classes.lua");

        grit_classes << "print(\"Loading GTA San Andreas classes\")\n";
        grit_classes << "local gom = get_gom()\n";

        std::ofstream matbin;
        matbin.open((dest_dir+"/san_andreas.matbin").c_str(), std::ios::binary);
        ASSERT_IO_SUCCESSFUL(matbin, "opening san_andreas.matbin");

        Objs &objs = everything.objs;

        for (Objs::iterator i=objs.begin(),i_=objs.end() ; i!=i_ ; ++i) {
                Obj &o = *i;

                if (!used_ids[o.id]) continue;

                //out << "id: " << o.id << "  "
                //    << "dff: " << o.dff << std::endl;

                struct dff dff;
                std::string dff_name = o.dff+".dff";
                Img *img = NULL;
                if (gta3.fileExists(dff_name)) {
                        img = &gta3;
                } else if (gta_int.fileExists(dff_name)) {
                        img = &gta_int;
                } else {
                        out << "Not found in gta3.img or gta_int.img: "
                            << "\"" << dff_name << "\"" << std::endl;
                        continue;
                }

                std::string img_name = img==&gta3 ? "gta3.img" : "gta_int.img";
                std::ifstream &img_f = img==&gta3 ? gta3_f : gta_int_f;

                open_file_img(out,img_f,*img,dff_name);
                ios_read_dff(1,img_f,&dff,img_name+"/"+dff_name+"/");
        
                ASSERT(dff.geometries.size()==1 || dff.geometries.size()==2);

                typedef std::vector<std::string> Strs;
                Strs background_texs;

                float rad = 0;
                for (unsigned long j=0 ; j<dff.frames.size() ; ++j) {
                        frame &fr = dff.frames[j];
                        // ignore dummies for now
                        if (fr.geometry == 0xFFFFFFFF)
                                continue;

                        if (o.flags & OBJ_FLAG_2CLUMP) {
                                ASSERT(dff.geometries.size()==2);
                                ASSERT(j==1 || j==2);
                                // j==1 is the damaged version
                                // j==2 is the undamaged version
                                if (j==1) continue;
                                ASSERT(fr.geometry==1);
                        } else {
                                ASSERT(fr.geometry==0);
                                ASSERT(dff.geometries.size()==1);
                        }

                        geometry &g = dff.geometries[fr.geometry];

                        rad = sqrt(g.b_x*g.b_x + g.b_y*g.b_y + g.b_z*g.b_z)
                              + g.b_r;

                        std::stringstream out_name_ss;
                        out_name_ss<<dest_dir<<"/"<<o.id<<".mesh";
                        std::string out_name = out_name_ss.str();
                        export_mesh(texs,img_name,
                                    out,out_name,
                                    o,g,matdb,matbin);

                        MatSplits &ms = g.mat_spls;
                        for (MatSplits::iterator s=ms.begin(),
                                                 s_=ms.end() ; s!=s_ ; s++) {
                            const material &m = g.materials[s->material];
                            const Strings &texs = m.rewrittenTextures;
                            if (texs.size()==0) continue;
                            ASSERT(texs.size()==1);
                            background_texs.push_back(texs[0]);
                        }

                        
                }

                std::stringstream col_field;
                std::string cls = "BaseClass";

                std::string tcol_name = img_name+"/"+o.dff+".tcol";
                bool use_col = true;

                // once only
                if (cols_i.find(tcol_name)==cols.end()) {
                        if (!(o.flags && OBJ_FLAG_NO_COL))
                                out<<"Couldn't find col \""<<tcol_name<<"\" "
                                   <<"referenced from "<<o.id<<std::endl;
                        use_col = false;
                }
                if (cols.find(tcol_name)==cols.end()) {
                        //out<<"Skipping empty col \""<<tcol_name<<"\" "
                        //   <<"referenced from "<<o.id<<std::endl;
                        use_col = false;
                }
                if (use_col) {
                        //out<<"col: \""<<tcol_name<<"\" "<<std::endl;
                        // add col to grit class
                        col_field << ",colMesh=\""<<tcol_name<<"\"";
                        cls = "ColClass";
                }

                //preloads -- the mesh(s) and textures fr
                std::stringstream background_texs_ss;
                for (Strs::iterator i=background_texs.begin(),
                                    i_=background_texs.end() ; i!=i_ ; ++i) {
                        if (*i=="") continue;
                        background_texs_ss<<"\""<<*i<<"\",";
                }
            

                grit_classes<<"gom:addClass(extends("<<cls<<"){"
                                <<"name=\"" <<o.id<<"\""
                                <<",renderingDistance="<<(o.draw_distance+rad)
                                <<",textures=StringDB{"<<background_texs_ss.str()<<"}"
                                <<col_field.str()<<"})\n";
                
        }

        // imgs


}

int main(int argc, char **argv)
{

        if (argc!=2 && argc!=3) {
                std::cerr<<"Usage: "<<argv[ 0]<<" <San Andreas Dir> "
                         <<" [ <Destination Dir> ]"<<std::endl;
                return EXIT_FAILURE;
        }

        std::string gta_dir = argv[1];
        std::string dest_dir = argc>2 ? argv[2] : ".";

        init_tex_dup_map();
        init_ogre();

        try {
                extract(gta_dir,dest_dir,std::cout);
        } catch (Exception &e) {
                std::cerr << "ERROR: "
                          << e.getFullDescription() << std::endl;

                return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
}



// vim: shiftwidth=8:tabstop=8:expandtab
