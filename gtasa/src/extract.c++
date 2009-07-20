#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <locale>

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

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

static void open_file (std::ostream &out, std::ifstream &f,
                       const std::string fname)
{
        (void)out;
        //out << "Opening: \""+fname+"\"" << std::endl;
        f.open(fname.c_str(), std::ios::binary);
        ASSERT_IO_SUCCESSFUL(f,"opening "+fname);
}

struct ImgHandle {
        void init (const std::string &fname, const std::string &name_,
                   std::ostream &out)
        {
                open_file(out, f, fname);
                i.init(f, fname);
                name = name_;
        }
        void open_file_img (std::ostream &out, const std::string fname)
        {
                (void)out;
                //out << "Opening (from img): \""+fname+"\"" << std::endl;
                i.fileOffset(f,fname);
        }

        std::ifstream f;
        Img i;
        std::string name;
};

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

typedef std::vector<IPL> IPLs;

static void addFullIPL (std::ostream &out,
                        const std::string &gta_dir,
                        IPLs &ipls,
                        const std::string &text,
                        ImgHandle &img,
                        const std::string &bin,
                        size_t n)
{
        ipls.push_back(IPL());
        
        IPL &ipl = *(ipls.end()-1);

        ipl.setName(text);

        std::string name = gta_dir + "/data/maps/" + text;
        std::ifstream text_f;
        //out << "Opening text IPL: \"" << name << "\"" << std::endl;
        text_f.open(name.c_str(),std::ios::binary);
        ASSERT_IO_SUCCESSFUL(text_f,"opening text IPL: "+name);
        
        ipl.addMore(text_f);

        for (size_t i=0 ; i<n ; ++i) {

                std::stringstream ss;
                ss<<bin<<"_stream"<<i<<".ipl";

                img.open_file_img(out,ss.str());
                
                ipl.addMore(img.f);
        }

}

typedef std::map<std::string,Txd> TxdDir;

void process_txds (std::ostream &out,
                   Txd::Names &texs,
                   ImgHandle &img,
                   const std::string &dest_dir,
                   const std::string &modname)
{
        (void) out;
        for (unsigned int i=0 ; i<img.i.size(); ++i) {
                const std::string &fname = img.i.fileName(i);
                if (fname.size()<4) continue;
                std::string ext = fname.substr(fname.size()-4,4);
                if (ext!=".txd") continue;
                //out<<"Extracting: "<<img.name<<"/"<<fname<<std::endl;
                img.i.fileOffset(img.f,i);
                std::string txddir =
                        dest_dir+"/"+modname+"/"+img.name+"/"+fname;
                ensuredir(txddir);
                Txd txd(img.f,txddir); // extract dds files
                const Txd::Names &n = txd.getNames();
                typedef Txd::Names::const_iterator TI;
                for (TI j=n.begin(),j_=n.end();j!=j_;++j) {
                        const std::string &texname = *j;
                        // build a list of all textures we
                        // know about (relative to dest_dir)
                        texs.insert(modname+"/"+img.name+"/"+
                                    fname+"/"+texname+".dds");
                }
        }
}


typedef std::set<std::string> ColNames;

void process_cols (std::ostream &out,
                   ColNames &cols,
                   ColNames &cols_including_empty,
                   ImgHandle &img,
                   const std::string &dest_dir,
                   const std::string &modname)
{
        (void) out;
        for (unsigned int i=0 ; i<img.i.size(); ++i) {
                const std::string &fname = img.i.fileName(i);
                if (fname.size()<4) continue;
                std::string ext = fname.substr(fname.size()-4,4);
                if (ext!=".col") continue;
                //out<<"Extracting: "<<img.name<<"/"<<fname<<std::endl;
                img.i.fileOffset(img.f,i);

                std::istream::int_type next;

                do {

                        TColFile tcol;
                        std::string name;
                        parse_col(name,img.f,tcol);

                        std::string tcolname =
                                modname+"/"+img.name+"/"+name+".tcol";

                        cols_including_empty.insert(tcolname);

                        if (tcol.usingCompound || tcol.usingTriMesh) {

                                cols.insert(tcolname);

                                name = dest_dir+"/"+tcolname;

                                std::ofstream f;
                                f.open(name.c_str(), std::ios::binary);
                                ASSERT_IO_SUCCESSFUL(f,"opening tcol "
                                                         "for writing");

                                pretty_print_tcol(f,tcol);
                        }

                        next = img.f.peek();

                } while (next!=std::istream::traits_type::eof() && next!=0);

                // no more cols

        }
}

struct IPLConfig {
        IPLConfig () { }
        IPLConfig (const std::string &base_, const std::string &img_,
                   const std::string &bin_, int num_)
              : base(base_), img(img_), bin(bin_), num(num_) { }
        std::string base;
        std::string img;
        std::string bin;
        size_t num;
};

struct Config {
        std::string gta_dir;
        std::string dest_dir;
        std::string modname;
        const char **idesv;
        size_t idesc;
        std::vector<std::pair<std::string,std::string> > imgs;
        std::vector<IPLConfig> ipls;
};

void extract (const Config &cfg, std::ostream &out)
{ 
        const std::string &gta_dir = cfg.gta_dir;
        const std::string &dest_dir = cfg.dest_dir;

        ide everything;
        for (size_t i=0 ; i<cfg.idesc ; ++i) {
                std::ifstream f;
                open_file(out, f, gta_dir+"/data/"+cfg.idesv[i]);
                read_ide(f, &everything);
        }

        Txd::Names texs;
            
        // cols_i = cols + the empty cols (for error checking)
        ColNames cols, cols_i;

        // imgs
        std::map<std::string,ImgHandle*> imgs;
        for (size_t i=0 ; i<cfg.imgs.size() ; ++i) {
                std::string name = cfg.imgs[i].second;
                ImgHandle *img = imgs[name] = new ImgHandle();
                img->init(gta_dir+cfg.imgs[i].first, name, out);
                process_txds(out, texs, *img, dest_dir, cfg.modname);
                process_cols(out, cols, cols_i, *img, dest_dir, cfg.modname);
        }

        if (getenv("DUMP_TEX_LIST")) {
                for (Txd::Names::iterator i=texs.begin(),
                                          i_=texs.end() ; i!=i_ ; ++i) {
                        out << *i << std::endl;
                }
        }

        if (getenv("DUMP_COL_LIST")) {
                for (ColNames::iterator i=cols.begin(),
                                        i_=cols.end() ; i!=i_ ; ++i) {
                        out << *i << std::endl;
                }
        }

        // ipls
        std::vector<IPL> ipls;
        for (size_t i=0 ; i<cfg.ipls.size() ; ++i) {
                const std::string &base = cfg.ipls[i].base;
                const std::string &img = cfg.ipls[i].img;
                const std::string &bin = cfg.ipls[i].bin;
                size_t num = cfg.ipls[i].num;
                if (imgs.find(img)==imgs.end()) {
                        std::stringstream ss;
                        ss << "ERROR: no such IMG \""<<img<<"\"";
                        IOS_EXCEPT(ss.str());
                }
                addFullIPL(out, gta_dir, ipls, base, *imgs[img], bin, num);
        }
        

        std::ofstream map;
        map.open((dest_dir+"/"+cfg.modname+"/map.lua").c_str(),
                 std::ios::binary);
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
                        std::stringstream cls;
                        cls << cfg.modname << "/" << inst.id;
                        if (inst.near_for==-1) {
                                map<<"tryAdd(\""<<cls.str()<<"\","
                                   <<inst.x<<","<<inst.y<<","<<inst.z;
                                if (inst.rx!=0 || inst.ry!=0 || inst.rz!=0) {
                                        map<<",{rot=Quat("
                                           <<inst.rw<<","<<inst.rx<<","
                                           <<inst.ry<<","<<inst.rz<<")}";
                                }
                                map<<")\n";
                        } else {
                                const Inst &far = insts[inst.near_for];
                                std::stringstream farcls;
                                farcls << cfg.modname << "/" << far.id;
                                map<<"last=tryAdd(\""<<cls.str()<<"\","
                                   <<inst.x<<","<<inst.y<<","<<inst.z;
                                if (inst.rx!=0 || inst.ry!=0 || inst.rz!=0) {
                                        map<<",{rot=Quat("
                                           <<inst.rw<<","<<inst.rx<<","
                                           <<inst.ry<<","<<inst.rz<<")}";
                                }
                                map<<")\n";
                                map<<"tryAdd(\""<<farcls.str()<<"\","
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

        std::ofstream classes;
        classes.open((dest_dir+"/"+cfg.modname+"/grit_classes.lua").c_str(),
                          std::ios::binary);
        ASSERT_IO_SUCCESSFUL(classes, "opening grit_classes.lua");

        classes << "print(\"Loading GTA San Andreas classes\")\n";
        classes << "local gom = get_gom()\n";

        std::ofstream matbin;
        matbin.open((dest_dir+"/"+cfg.modname+"/san_andreas.matbin").c_str(),
                    std::ios::binary);
        ASSERT_IO_SUCCESSFUL(matbin, "opening san_andreas.matbin");

        Objs &objs = everything.objs;

        for (Objs::iterator i=objs.begin(),i_=objs.end() ; i!=i_ ; ++i) {
                Obj &o = *i;

                if (!used_ids[o.id]) continue;

                //out << "id: " << o.id << "  "
                //    << "dff: " << o.dff << std::endl;

                struct dff dff;
                std::string dff_name = o.dff+".dff";
                ImgHandle *img = NULL;
                for (size_t i=0 ; i<cfg.imgs.size() ; ++i) {
                        ImgHandle *img2 = imgs[cfg.imgs[i].second];
                        if (img2->i.fileExists(dff_name)) {
                                img = img2;
                                break;
                        }
                }
                if (img == NULL) {
                        out << "Not found in any IMG file: "
                            << "\"" << dff_name << "\"" << std::endl;
                        continue;
                }

                img->open_file_img(out,dff_name);
                ios_read_dff(1,img->f,&dff,img->name+"/"+dff_name+"/");
        
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
                        out_name_ss<<dest_dir<<"/"<<cfg.modname<<"/"<<o.id<<".mesh";
                        std::vector<std::string> export_imgs;
                        export_imgs.push_back(img->name);
                        for (size_t k=0 ; k<imgs.size() ; ++k) {
                                ImgHandle *img2 = imgs[cfg.imgs[k].second];
                                if (img2->name == img->name) continue;
                                export_imgs.push_back(img2->name);
                        }
                        std::string out_name = out_name_ss.str();
                        export_mesh(texs,everything,export_imgs,
                                    out,out_name,
                                    o,g,matdb,matbin,cfg.modname);

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

                std::string tcol_name = cfg.modname+"/"+img->name+"/"+o.dff+".tcol";
                bool use_col = true;

                // once only
                if (cols_i.find(tcol_name)==cols_i.end()) {
                        //if (!(o.flags & OBJ_FLAG_NO_COL))
                        //        out<<"Couldn't find col \""<<tcol_name<<"\" "
                        //           <<"referenced from "<<o.id<<std::endl;
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
            

                bool cast_shadow = o.flags & OBJ_FLAG_POLE_SHADOW;

                classes<<"gom:addClass("
                       <<"\""<<cfg.modname<<"/"<<o.id<<"\","
                       <<cls<<",{"
                           <<"castShadows="<<(cast_shadow?"true":"false")
                           <<",renderingDistance="<<(o.draw_distance+rad)
                           <<",textures={"<<background_texs_ss.str()<<"}"
                           <<col_field.str()
                       <<"})\n";
                
        }

}

void extract_gtasa (const std::string &gta_dir, const std::string &dest_dir)
{

        Config cfg;

        cfg.modname = "gtasa";

        cfg.gta_dir = gta_dir;
        cfg.dest_dir = dest_dir;

        init_tex_dup_map();
        init_ogre();

        typedef std::pair<std::string,std::string> P;
        cfg.imgs.push_back(P("/models/gta3.img","gta3.img"));
        cfg.imgs.push_back(P("/models/gta_int.img","gta_int.img"));


        // ides {{{
        const char *ides[] = {
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
        }; //}}}

        cfg.idesv = ides;
        cfg.idesc = sizeof ides / sizeof *ides;

        //{{{
        cfg.ipls.push_back(IPLConfig("leveldes/seabed.ipl","gta3.img","seabed",1));
        cfg.ipls.push_back(IPLConfig("leveldes/levelmap.ipl","gta3.img","levelmap",1));
        cfg.ipls.push_back(IPLConfig("SF/SFe.ipl","gta3.img","sfe",4));
        cfg.ipls.push_back(IPLConfig("SF/SFn.ipl","gta3.img","sfn",3));
        cfg.ipls.push_back(IPLConfig("SF/SFSe.ipl","gta3.img","sfse",7));
        cfg.ipls.push_back(IPLConfig("SF/SFs.ipl","gta3.img","sfs",9));
        cfg.ipls.push_back(IPLConfig("SF/SFw.ipl","gta3.img","sfw",6));
        cfg.ipls.push_back(IPLConfig("vegas/vegasE.ipl","gta3.img","vegase",9));
        cfg.ipls.push_back(IPLConfig("vegas/vegasN.ipl","gta3.img","vegasn",9));
        cfg.ipls.push_back(IPLConfig("vegas/vegasS.ipl","gta3.img","vegass",6));
        cfg.ipls.push_back(IPLConfig("vegas/vegasW.ipl","gta3.img","vegasw",12));
        cfg.ipls.push_back(IPLConfig("country/countn2.ipl","gta3.img","countn2",9));
        cfg.ipls.push_back(IPLConfig("country/countrye.ipl","gta3.img","countrye",14));
        cfg.ipls.push_back(IPLConfig("country/countryN.ipl","gta3.img","countryn",4));
        cfg.ipls.push_back(IPLConfig("country/countryS.ipl","gta3.img","countrys",5));
        cfg.ipls.push_back(IPLConfig("country/countryw.ipl","gta3.img","countryw",9));
        cfg.ipls.push_back(IPLConfig("LA/LAe2.ipl","gta3.img","lae2",7));
        cfg.ipls.push_back(IPLConfig("LA/LAe.ipl","gta3.img","lae",6));
        cfg.ipls.push_back(IPLConfig("LA/LAhills.ipl","gta3.img","lahills",5));
        cfg.ipls.push_back(IPLConfig("LA/LAn2.ipl","gta3.img","lan2",4));
        cfg.ipls.push_back(IPLConfig("LA/LAn.ipl","gta3.img","lan",3));
        cfg.ipls.push_back(IPLConfig("LA/LAs2.ipl","gta3.img","las2",5));
        cfg.ipls.push_back(IPLConfig("LA/LAs.ipl","gta3.img","las",6));
        cfg.ipls.push_back(IPLConfig("LA/LAw2.ipl","gta3.img","law2",5));
        cfg.ipls.push_back(IPLConfig("LA/LAw.ipl","gta3.img","law",6));
        cfg.ipls.push_back(IPLConfig("LA/LaWn.ipl","gta3.img","lawn",4));
        cfg.ipls.push_back(IPLConfig("interior/gen_int1.ipl","gta_int.img","",0));
        cfg.ipls.push_back(IPLConfig("interior/gen_int2.ipl","gta_int.img","gen_int2",1));
        cfg.ipls.push_back(IPLConfig("interior/gen_int3.ipl","gta_int.img","gen_int3",1));
        cfg.ipls.push_back(IPLConfig("interior/gen_int4.ipl","gta_int.img","gen_int4",3));
        cfg.ipls.push_back(IPLConfig("interior/gen_int5.ipl","gta_int.img","gen_int5",8));
        cfg.ipls.push_back(IPLConfig("interior/gen_intb.ipl","gta_int.img","gen_intb",1));
        cfg.ipls.push_back(IPLConfig("interior/int_cont.ipl","gta_int.img","",0));
        cfg.ipls.push_back(IPLConfig("interior/int_LA.ipl","gta_int.img","int_la",4));
        cfg.ipls.push_back(IPLConfig("interior/int_SF.ipl","gta_int.img","int_sf",1));
        cfg.ipls.push_back(IPLConfig("interior/int_veg.ipl","gta_int.img","int_veg",4));
        cfg.ipls.push_back(IPLConfig("interior/savehous.ipl","gta_int.img","savehous",2));
        cfg.ipls.push_back(IPLConfig("interior/stadint.ipl","gta_int.img","stadint",1));
        //}}}

        extract(cfg,std::cout);
}


void extract_gostown (const std::string &gta_dir, const std::string &dest_dir)
{

        Config cfg;

        cfg.modname = "gostown";

        cfg.gta_dir = gta_dir;
        cfg.dest_dir = dest_dir;

        init_ogre();

        typedef std::pair<std::string,std::string> P;
        cfg.imgs.push_back(P("/models/gta3.img","gta3.img"));
        cfg.imgs.push_back(P("/models/gostown6.img","gostown6.img"));


        // {{{ IDES
        const char *ides[] = {
                //"maps/country/countn2.ide",
                //"maps/country/countrye.ide",
                //"maps/country/countryN.ide",
                //"maps/country/countryS.ide",
                //"maps/country/countryW.ide",
                //"maps/country/counxref.ide",
                "maps/generic/barriers.ide",
                "maps/generic/dynamic.ide",
                "maps/generic/dynamic2.ide",
                "maps/generic/multiobj.ide",
                "maps/generic/procobj.ide",
                "maps/generic/vegepart.ide",
                //"maps/interior/gen_int1.ide",
                //"maps/interior/gen_int2.ide",
                //"maps/interior/gen_int3.ide",
                //"maps/interior/gen_int4.ide",
                //"maps/interior/gen_int5.ide",
                //"maps/interior/gen_intb.ide",
                //"maps/interior/int_cont.ide",
                //"maps/interior/int_LA.ide",
                //"maps/interior/int_SF.ide",
                //"maps/interior/int_veg.ide",
                //"maps/interior/propext.ide",
                //"maps/interior/props.ide",
                //"maps/interior/props2.ide",
                //"maps/interior/savehous.ide",
                //"maps/interior/stadint.ide",
                //"maps/LA/LAe.ide",
                //"maps/LA/LAe2.ide",
                //"maps/LA/LAhills.ide",
                //"maps/LA/LAn.ide",
                //"maps/LA/LAn2.ide",
                //"maps/LA/LAs.ide",
                //"maps/LA/LAs2.ide",
                //"maps/LA/LAw.ide",
                //"maps/LA/LAw2.ide",
                //"maps/LA/LaWn.ide",
                //"maps/LA/LAxref.ide",
                //"maps/leveldes/levelmap.ide",
                //"maps/leveldes/levelxre.ide",
                "maps/leveldes/seabed.ide",
                //"maps/SF/SFe.ide",
                //"maps/SF/SFn.ide",
                //"maps/SF/SFs.ide",
                //"maps/SF/SFSe.ide",
                //"maps/SF/SFw.ide",
                //"maps/SF/SFxref.ide",
                //"maps/txd.ide",
                //"maps/vegas/vegasE.ide",
                //"maps/vegas/VegasN.ide",
                //"maps/vegas/VegasS.ide",
                //"maps/vegas/VegasW.ide",
                //"maps/vegas/vegaxref.ide",
                //"maps/veh_mods/veh_mods.ide",
                "maps/Gostown6/Gp_land50.IDE",
                "maps/Gostown6/Gp_tunnels.IDE",
                "maps/Gostown6/Gp_docks.IDE",
                "maps/Gostown6/Gp_palmtheme.IDE",
                "maps/Gostown6/Gp_A13.IDE",
                "maps/Gostown6/Gp_airport.IDE",
                "maps/Gostown6/Gp_city.IDE",
                "maps/Gostown6/Gp_GSS.IDE",
                "maps/Gostown6/lots/ParoXum.IDE",
                "maps/Gostown6/lots/JostVice.IDE",
                "maps/Gostown6/lots/Fredskin.IDE",
                "maps/Gostown6/lots/Raycen.IDE",
                "maps/Gostown6/lots/Generics.IDE",
                "maps/Gostown6/lots/Parent.IDE",
                "maps/Gostown6/Gp_laguna.IDE",
                //"vehicles.ide",
                //"peds.ide",
                //"default.ide"
        }; //}}}

        cfg.idesv = ides;
        cfg.idesc = sizeof ides / sizeof *ides;

        //{{{ IPL
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_land50.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_tunnels.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_docks.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_veg.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_A13.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_airport.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_City.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_GSS.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_props.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/lots/ParoXum.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/lots/JostVice.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/lots/Fredskin.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/lots/Raycen.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/Gp_laguna.IPL","gostown6.img","",0));
        cfg.ipls.push_back(IPLConfig("Gostown6/occlu.ipl","gostown6.img","",0));
        //}}}

        extract(cfg,std::cout);
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

int main(int argc, char **argv)
{

        if (argc!=3 && argc!=4) {
                std::cerr<<"Usage: "<<argv[ 0]<<" <mod> <source_dir> "
                         <<" [ <grit_dir> ]"<<std::endl;
                std::cerr<<"Where <mod> can be \"gtasa\" "
                         <<"or \"gostown\""<<std::endl;
                return EXIT_FAILURE;
        }

        try {
                std::string mod = argv[1];
                strlower(mod);
                const char *src = argv[2];
                const char *dest = argc>3?argv[3]:".";
                if (mod=="gostown") {
                        extract_gostown(src, dest);
                } else if (mod=="gtasa") {
                        extract_gtasa(src, dest);
                } else {
                        std::cerr<<"Unrecognised mod: \""
                                 <<argv[1]<<"\""<<std::endl;
                        return EXIT_FAILURE;
                }
        } catch (Exception &e) {
                std::cerr << "ERROR: "
                          << e.getFullDescription() << std::endl;

                return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
}

// vim: shiftwidth=8:tabstop=8:expandtab
