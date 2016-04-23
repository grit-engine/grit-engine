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

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <locale>
#include <algorithm>

#include "imgread.h"
#include "iplread.h"
#include "ideread.h"
#include "tex_dups.h"
#include "dffread.h"
#include "txdread.h"
//#include "physics/tcol_parser.h"
#include "physics/bcol_parser.h"
#include "col_parser.h"

#include "ios_util.h"
#include "dirutil.h"
#include "csvread.h"
#include "handling.h"
#include "surfinfo.h"
#include "procobj.h"

CentralisedLog clog;
void app_fatal (void) { abort(); }
void assert_triggered (void) { } 



static void open_file (std::ostream &out, std::ifstream &f,
               const std::string fname)
{
    (void)out;
    //out << "Opening: \""+fname+"\"" << std::endl;
    f.open(fname.c_str(), std::ios::binary);
    APP_ASSERT_IO_SUCCESSFUL(f,"opening "+fname);
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
    APP_ASSERT_IO_SUCCESSFUL(text_f,"opening text IPL: "+name);
    
    ipl.addMore(text_f);

    for (size_t i=0 ; i<n ; ++i) {

        std::stringstream ss;
        ss<<bin<<"_stream"<<i<<".ipl";

        img.open_file_img(out,ss.str());
        
        ipl.addMore(img.f);
    }

}

typedef std::map<std::string,Txd> TxdDir;

void process_txd (std::ostream &out,
          Txd::Names &texs,
          const std::string &fname,
          const std::string &dest_dir,
          const std::string &modprefix,
          std::istream &in)
{
    if (getenv("SKIP_TEXTURES")!=NULL) return;
    (void) out;
    std::string txddir = dest_dir+"/"+modprefix+fname;
    ensuredir(txddir);
    Txd txd(in,txddir); // extract dds files
    const Txd::Names &n = txd.getNames();
    typedef Txd::Names::const_iterator TI;
    for (TI j=n.begin(),j_=n.end();j!=j_;++j) {
        const std::string &texname = *j;
        // build a list of all textures we
        // know about (relative to dest_dir)
        texs.insert(fname+"/"+texname+".dds");
    }
}

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
        process_txd(out, texs, img.name+"/"+fname, dest_dir, modname+"/", img.f);
    }
}


typedef std::set<std::string> ColNames;

void process_cols (std::ostream &out,
           ColNames &cols,
           ColNames &cols_including_empty,
           ImgHandle &img,
           const std::string &dest_dir,
           const std::string &modname,
           MaterialMap &db)
{
    if (getenv("SKIP_COLS")!=NULL) return;
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
            // the materials are in gtasa/ but imgs are behind another dir so prefix ../
            parse_col(name,img.f,tcol, "../", db);

            std::string gcolname = img.name+"/"+name+".gcol";

            cols_including_empty.insert(gcolname);

            if (tcol.usingCompound || tcol.usingTriMesh) {

                cols.insert(gcolname);

                name = dest_dir+"/"+modname+"/"+gcolname;

                std::ofstream f;
                f.open(name.c_str(), std::ios::binary);
                APP_ASSERT_IO_SUCCESSFUL(f,"opening tcol for writing");

                write_tcol_as_bcol(f, tcol);
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
    std::vector<std::pair<std::string,std::string> > txds;
    std::vector<IPLConfig> ipls;
};

void extract (const Config &cfg, std::ostream &out)
{ 
    const std::string &gta_dir = cfg.gta_dir;
    const std::string &dest_dir = cfg.dest_dir;

    ensuredir(dest_dir+"/"+cfg.modname);

    out << "Extracting car colours..." << std::endl;
    Csv carcols;
    std::map<std::string, std::vector<int> > carcols_2;
    std::map<std::string, std::vector<int> > carcols_4;
    {
        std::ofstream carcols_lua;
        carcols_lua.open((dest_dir+"/"+cfg.modname+"/carcols.lua").c_str(),
                  std::ios::binary);
        APP_ASSERT_IO_SUCCESSFUL(carcols_lua, "opening carcols.lua");

        carcols_lua << "print(\"Loading carcols\")\n";
        carcols.filename = gta_dir+"/data/carcols.dat";
        std::ifstream carcols_f;
        carcols_f.open(carcols.filename.c_str(), std::ios::binary);
        read_csv(carcols_f, carcols);
        const CsvSection &coldefs = carcols["col"];
        for (unsigned i=0 ; i<coldefs.size() ; ++i) {
            const CsvLine &line = coldefs[i];
            float r,g,b;
            if (line.size()==2) {
                // there is one case where a decimal point
                //is used instead of a comma
                size_t n = line[0].find(".");
                APP_ASSERT(n<line[0].npos-1);
                std::string p1 = line[0].substr(0,n);
                std::string p2 = line[0].substr(n+1,line[0].npos);
                r = (float) strtod(p1.c_str(), NULL)/255;
                g = (float) strtod(p2.c_str(), NULL)/255;
                b = (float) strtod(line[1].c_str(), NULL)/255;
            } else {
                APP_ASSERT(line.size()==3);
                r = (float) strtod(line[0].c_str(), NULL)/255;
                g = (float) strtod(line[1].c_str(), NULL)/255;
                b = (float) strtod(line[2].c_str(), NULL)/255;
            }
            carcols_lua<<"carcols.gtasa"<<i<<" = { { "<<r<<", "<<g<<", "<<b<<" } }"
                   <<std::endl;
        }
        const CsvSection &col2defs = carcols["car"];
        for (unsigned i=0 ; i<col2defs.size() ; ++i) {
            const CsvLine &line = col2defs[i];
            APP_ASSERT(line.size()>=1);
            for (unsigned j=1 ; j<(line.size()-1)/2*2+1 ; ++j) {
                carcols_2[line[0]].push_back(strtol(line[j].c_str(), NULL, 10));
            }
        }
        const CsvSection &col4defs = carcols["car4"];
        for (unsigned i=0 ; i<col4defs.size() ; ++i) {
            const CsvLine &line = col4defs[i];
            APP_ASSERT(line.size()>=1);
            APP_ASSERT(line.size()%4==1);
            for (unsigned j=1 ; j<(line.size()-1)/4*4+1 ; ++j) {
                carcols_4[line[0]].push_back(strtol(line[j].c_str(), NULL, 10));
            }
        }
    }

    out << "Reading car handling file..." << std::endl;
    HandlingData handling;
    {
        Csv handling_csv;
        handling_csv.filename = gta_dir+"/data/handling.cfg";
        std::ifstream handling_f;
        handling_f.open(handling_csv.filename.c_str(), std::ios::binary);
        read_csv(handling_f, handling_csv);
        read_handling(handling_csv, handling);
    }

    out << "Reading surface info file..." << std::endl;
    SurfInfoData surfinfo;
    MaterialMap db;
    {
        std::ofstream physmats_lua;
        physmats_lua.open((dest_dir+"/"+cfg.modname+"/phys_mats.lua").c_str(),
                  std::ios::binary);
        APP_ASSERT_IO_SUCCESSFUL(physmats_lua, "opening physmats.lua");

        Csv surfinfo_csv;
        surfinfo_csv.filename = gta_dir+"/data/surfinfo.dat";
        std::ifstream surfinfo_f;
        surfinfo_f.open(surfinfo_csv.filename.c_str(), std::ios::binary);
        read_csv(surfinfo_f, surfinfo_csv);
        read_surfinfo(surfinfo_csv, surfinfo);
        for (unsigned i=0 ; i<surfinfo.surfaces.size() ; ++i) {
            SurfInfo &surf = surfinfo.surfaces[i];
            db[i] = surf.name;
            physmats_lua<<"physical_material `"<<surf.name<<"` {"<<std::endl;
            float rtf, ortf;
            if (surf.adhesion_group=="RUBBER") {
                physmats_lua<<"    interactionGroup=StickyGroup;"<<std::endl;
                rtf = 1;
                ortf = 1;
            } else if (surf.adhesion_group=="HARD") {
                physmats_lua<<"    interactionGroup=SmoothHardGroup;"<<std::endl;
                rtf = 0.9f;
                ortf = 0.7f;
            } else if (surf.adhesion_group=="ROAD") {
                physmats_lua<<"    interactionGroup=RoughGroup;"<<std::endl;
                rtf = 1;
                ortf = 0.8f;
            } else if (surf.adhesion_group=="LOOSE") {
                physmats_lua<<"    interactionGroup=DeformGroup;"<<std::endl;
                rtf = 0.7f;
                ortf = 1;
            } else if (surf.adhesion_group=="SAND") {
                physmats_lua<<"    interactionGroup=DeformGroup;"<<std::endl;
                rtf = 0.4f;
                ortf = 1;
            } else if (surf.adhesion_group=="WET") {
                physmats_lua<<"    interactionGroup=SlipperyGroup;"<<std::endl;
                rtf = 0.3f;
                ortf = 0.6f;
            } else {
                GRIT_EXCEPT("Unrecognised adhesion group: "+surf.adhesion_group);
            }
            physmats_lua<<"    roadTyreFriction="<<rtf<<";"<<std::endl;
            physmats_lua<<"    offRoadTyreFriction="<<ortf<<";"<<std::endl;
            physmats_lua<<"}"<<std::endl<<std::endl;;
        }

    }

    out << "Reading procedural objects file..." << std::endl;
    ProcObjData procobj;
    {
        Csv procobj_csv;
        procobj_csv.filename = gta_dir+"/data/procobj.dat";
        std::ifstream procobj_f;
        procobj_f.open(procobj_csv.filename.c_str(), std::ios::binary);
        read_csv(procobj_f, procobj_csv);
        read_procobj(procobj_csv, procobj);
    }

    out << "Reading IDE files..." << std::endl;
    ide everything;
    for (size_t i=0 ; i<cfg.idesc ; ++i) {
        std::ifstream f;
        open_file(out, f, gta_dir+"/data/"+cfg.idesv[i]);
        read_ide(cfg.idesv[i], f, &everything);
    }

    Txd::Names texs;
        
    // cols_i = cols + the empty cols (for error checking)
    ColNames cols, cols_i;

    out << "Extracting standalone txd files..." << std::endl;
    for (size_t i=0 ; i<cfg.txds.size() ; ++i) {
        std::string fname = gta_dir+cfg.txds[i].first;
        std::string name = cfg.txds[i].second;
        std::ifstream txd_f;
        txd_f.open(fname.c_str(), std::ios::binary);
        APP_ASSERT_IO_SUCCESSFUL(txd_f,"opening "+fname);
        //out<<"Extracting: "<<fname<<std::endl;
        process_txd (out, texs, name, dest_dir, cfg.modname+"/", txd_f);

    }

    out << "Reading img files..." << std::endl;
    std::map<std::string,ImgHandle*> imgs;
    for (size_t i=0 ; i<cfg.imgs.size() ; ++i) {
        std::string name = cfg.imgs[i].second;
        imgs[name] = new ImgHandle();
        imgs[name]->init(gta_dir+cfg.imgs[i].first, name, out);
    }

    out << "Extracting txds from imgs..." << std::endl;
    for (size_t i=0 ; i<cfg.imgs.size() ; ++i) {
        process_txds(out, texs, *imgs[cfg.imgs[i].second], dest_dir, cfg.modname);
    }

    out << "Extracting cols from imgs..." << std::endl;
    for (size_t i=0 ; i<cfg.imgs.size() ; ++i) {
        process_cols(out, cols, cols_i, *imgs[cfg.imgs[i].second], dest_dir, cfg.modname, db);
    }

    if (getenv("DUMP_TEX_LIST")) {
        for (Txd::Names::iterator i=texs.begin(), i_=texs.end() ; i!=i_ ; ++i) {
            out << *i << std::endl;
        }
    }

    if (getenv("DUMP_COL_LIST")) {
        for (ColNames::iterator i=cols.begin(), i_=cols.end() ; i!=i_ ; ++i) {
            out << *i << std::endl;
        }
    }

    std::vector<IPL> ipls;
    {
        out << "Reading IPLs..." << std::endl;
        for (size_t i=0 ; i<cfg.ipls.size() ; ++i) {
            const std::string &base = cfg.ipls[i].base;
            const std::string &img = cfg.ipls[i].img;
            const std::string &bin = cfg.ipls[i].bin;
            size_t num = cfg.ipls[i].num;
            if (imgs.find(img)==imgs.end()) {
                std::stringstream ss;
                ss << "ERROR: no such IMG \""<<img<<"\"";
                GRIT_EXCEPT(ss.str());
            }
            addFullIPL(out, gta_dir, ipls, base, *imgs[img], bin, num);
        }
    }

    MatDB matdb;


    // don't bother generating classes for things that aren't instantiated
    out << "Working out what to export..." << std::endl;
    std::map<unsigned long,bool> ids_used_in_ipl;
    std::map<unsigned long,bool> ids_written_out;

    for (IPLs::iterator i=ipls.begin(),i_=ipls.end() ; i!=i_ ; ++i) {
        const Insts &insts = i->getInsts();
        for (Insts::const_iterator j=insts.begin(),j_=insts.end() ; j!=j_ ; ++j) {
            const Inst &inst = *j;
            ids_used_in_ipl[inst.id] = true;
        }
    }

    Objs &objs = everything.objs;
    for (Objs::iterator i=objs.begin(),i_=objs.end() ; i!=i_ ; ++i) {
        Obj &o = *i;

        // id never used
        if (!ids_used_in_ipl[o.id]) continue;

        ids_written_out[o.id] = true;
    }

    if (getenv("SKIP_CLASSES")==NULL) {
        std::ofstream classes;
        std::ofstream materials_lua;

        out << "Exporting classes..." << std::endl;

        classes.open((dest_dir+"/"+cfg.modname+"/classes.lua").c_str(),
                 std::ios::binary);
        APP_ASSERT_IO_SUCCESSFUL(classes, "opening classes.lua");

        classes << "print(\"Loading classes\")\n";

        materials_lua.open((dest_dir+"/"+cfg.modname+"/materials.lua").c_str(),
                   std::ios::binary);
        APP_ASSERT_IO_SUCCESSFUL(materials_lua, "opening materials.lua");

        for (Objs::iterator i=objs.begin(),i_=objs.end() ; i!=i_ ; ++i) {
            Obj &o = *i;

            if (!ids_written_out[o.id]) continue;

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
            ios_read_dff(1,img->f,&dff,img->name+"/"+dff_name+"/","../",db);
        
            APP_ASSERT(dff.geometries.size()==1 || dff.geometries.size()==2);

            float rad = 0;
            for (unsigned long j=0 ; j<dff.frames.size() ; ++j) {
                frame &fr = dff.frames[j];
                // ignore dummies for now
                if (fr.geometry == -1)
                    continue;

                if (o.flags & OBJ_FLAG_2CLUMP) {
                    APP_ASSERT(dff.geometries.size()==2);
                    APP_ASSERT(j==1 || j==2);
                    // j==1 is the damaged version
                    // j==2 is the undamaged version
                    if (j==1) continue;
                    APP_ASSERT(fr.geometry==1);
                } else {
                    APP_ASSERT(fr.geometry==0);
                    APP_ASSERT(dff.geometries.size()==1);
                }

                geometry &g = dff.geometries[fr.geometry];

                rad = sqrt(g.b_x*g.b_x + g.b_y*g.b_y + g.b_z*g.b_z)
                      + g.b_r;

                std::stringstream objname_ss;
                objname_ss << o.id;
                std::string objname = objname_ss.str();

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
                generate_normals(g);
                export_mesh(texs,everything,export_imgs,
                        out,out_name,
                        o,objname,g,matdb,materials_lua);

            }

            std::stringstream col_field;
            std::stringstream lights_field;
            std::string cls = "BaseClass";

            std::string gcol_name = img->name+"/"+o.dff+".gcol";
            bool use_col = true;

            // once only
            if (cols_i.find(gcol_name)==cols_i.end()) {
                //if (!(o.flags & OBJ_FLAG_NO_COL))
                //    out<<"Couldn't find col \""<<gcol_name<<"\" "
                //       <<"referenced from "<<o.id<<std::endl;
                use_col = false;
            }
            if (cols.find(gcol_name)==cols.end()) {
                //out<<"Skipping empty col \""<<gcol_name<<"\" "
                //   <<"referenced from "<<o.id<<std::endl;
                use_col = false;
            }
            if (use_col) {
                //out<<"col: \""<<gcol_name<<"\" "<<std::endl;
                // add col to grit class
                col_field << ",colMesh=`"<<gcol_name<<"`";
                cls = "ColClass";
            }
            if (dff.geometries.size()==1) {
                bool no_lights_yet = true;
                std::string prefix = ", lights={ ";
                for (unsigned i=0 ; i<dff.geometries[0].twodfxs.size() ; ++i) {
                    twodfx &fx = dff.geometries[0].twodfxs[i];
                    if (fx.type != TWODFX_LIGHT) continue;
                    float r=fx.light.r/255.0f, g=fx.light.g/255.0f, b=fx.light.b/255.0f;
                    float R=std::max(fx.light.outer_range,fx.light.size);
                    lights_field << prefix << "{ "
                                 << "pos=vector3("<<fx.x<<","<<fx.y<<","<<fx.z<<"), "
                                 << "range="<<R<<", "
                                 << "diff=vector3("<<r<<","<<g<<","<<b<<"), "
                                 << "spec=vector3("<<r<<","<<g<<","<<b<<") }";
                    no_lights_yet = false;
                    prefix = ", ";
                }
                if (!no_lights_yet) lights_field << "}";
            }

            bool cast_shadow = 0 != (o.flags&OBJ_FLAG_POLE_SHADOW);
            cast_shadow = true;
            if ((o.flags & OBJ_FLAG_ALPHA1) && (o.flags & OBJ_FLAG_NO_SHADOW))
                cast_shadow = false;

            classes<<"class_add("
                   <<"`/gtasa/"<<o.id<<"`,"
                   <<cls<<",{"
                   <<"castShadows="<<(cast_shadow?"true":"false")
                   <<",renderingDistance="<<(o.draw_distance+rad)
                   <<col_field.str()
                   <<lights_field.str()
                   <<"})\n";
            
        }
    }

    Vehicles &vehicles = everything.vehicles;

    Txd::Names vehicle_texs;
    for (Txd::Names::iterator i=texs.begin(),i_=texs.end() ; i!=i_ ; ++i) {
        vehicle_texs.insert("../"+*i);
    }

    out << "Exporting vehicles..." << std::endl;
    std::ofstream vehicles_lua;
    std::string s = dest_dir+"/"+cfg.modname+"/vehicles.lua";
    vehicles_lua.open(s.c_str(), std::ios::binary);
    APP_ASSERT_IO_SUCCESSFUL(vehicles_lua, "opening "+s);
    for (Vehicles::iterator i=vehicles.begin(),i_=vehicles.end() ; i!=i_ ; ++i) {
        Vehicle &v = *i;
        out << "Exporting vehicle:  " << v.dff << std::endl;
        if (v.id==594) continue; // has broken col data or something
        //if (v.id!=415) continue;

        std::string vname = v.dff; // assuming that each dff is only used by one id
        std::string vehicle_dir = dest_dir+"/"+cfg.modname+"/"+vname;
        ensuredir(vehicle_dir);

        std::ofstream lua_file;
        std::string s = vehicle_dir+"/init.lua";
        lua_file.open(s.c_str(), std::ios::binary);
        APP_ASSERT_IO_SUCCESSFUL(lua_file, "opening "+s);

        vehicles_lua << "include `"<<vname<<"/init.lua`" << std::endl;
            
        struct dff dff;
        std::string dff_name = v.dff+".dff";
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
        // use a ../ prefix because the car gcol lives in its own directory
        ios_read_dff(1,img->f,&dff,img->name+"/"+dff_name+"/","../",db);

        VehicleData *vdata = handling[v.handling_id];
        APP_ASSERT(vdata!=NULL);
        
        // account for centre of gravity by adjusting entire mesh
        offset_dff(dff, -vdata->com_x, -vdata->com_y, -vdata->com_z);

        Obj obj; // currently obj only needs a few things set
        obj.dff = v.dff;
        obj.txd = v.txd;
        obj.flags = 0;
        obj.is_car = true;

        std::vector<std::string> export_imgs;
        export_imgs.push_back("../"+img->name);
        for (size_t k=0 ; k<imgs.size() ; ++k) {
            ImgHandle *img2 = imgs[cfg.imgs[k].second];
            if (img2->name == img->name) continue;
            export_imgs.push_back("../"+img2->name);
        }

        // since it's in a different directory, use a new matdb
        // this means that there could be duplicates of materials
        // but otherwise we'd have to refactor export_mesh to use different
        // strings in the mesh file than in the file defining the materials
        MatDB car_matdb;

        frame *fr_chassis = NULL;
        frame *fr_wheel_lb = NULL;
        frame *fr_wheel_lf = NULL;
        frame *fr_wheel_rb = NULL;
        frame *fr_wheel_rf = NULL;


        for (unsigned long j=0 ; j<dff.frames.size() ; ++j) {
            frame &fr = dff.frames[j];

            if (fr.name=="wheel_lb_dummy") fr_wheel_lb = &fr;
            if (fr.name=="wheel_lf_dummy") fr_wheel_lf = &fr;
            if (fr.name=="wheel_rb_dummy") fr_wheel_rb = &fr;
            if (fr.name=="wheel_rf_dummy") fr_wheel_rf = &fr;

            // ignore dummies for now
            if (fr.geometry == -1) continue;

            if (fr.name=="chassis") {
                reset_dff_frame(dff, j);
                fr_chassis = &fr;
            }
        }
        (void) fr_chassis; // suppress new gcc warning

        for (unsigned long j=0 ; j<dff.frames.size() ; ++j) {
            frame &fr = dff.frames[j];

            // ignore dummies for now
            if (fr.geometry == -1) continue;

            geometry &g = dff.geometries[fr.geometry];

            generate_normals(g);

            std::string oname = vehicle_dir+"/";

            // ides are to chase txdps
            export_mesh(vehicle_texs,everything,export_imgs,out,
                    vehicle_dir+"/"+fr.name+".mesh",
                    obj,fr.name,g,car_matdb,lua_file);
        }

        lua_file << std::endl;


        if (dff.has_tcol) {

/*
            for (unsigned i=0 ; i<dff.tcol.compound.hulls.size() ; ++i)
                dff.tcol.compound.hulls[i].material = 179;
            for (unsigned i=0 ; i<dff.tcol.compound.boxes.size() ; ++i)
                dff.tcol.compound.boxes[i].material = 179;
            for (unsigned i=0 ; i<dff.tcol.compound.cylinders.size() ; ++i)
                dff.tcol.compound.cylinders[i].material = 179;
            for (unsigned i=0 ; i<dff.tcol.compound.cones.size() ; ++i)
                dff.tcol.compound.cones[i].material = 179;
            for (unsigned i=0 ; i<dff.tcol.compound.planes.size() ; ++i)
                dff.tcol.compound.planes[i].material = 179;
            for (unsigned i=0 ; i<dff.tcol.compound.spheres.size() ; ++i)
                dff.tcol.compound.spheres[i].material = 179;
            for (unsigned i=0 ; i<dff.tcol.triMesh.faces.size() ; ++i)
                dff.tcol.triMesh.faces[i].material = 179;
*/

            dff.tcol.mass = vdata->mass;
            dff.tcol.linearDamping = 0.15f;

            tcol_triangles_to_hulls(dff.tcol, 0.04f, 0.04f);

            if (!dff.tcol.usingCompound && !dff.tcol.usingTriMesh) {
                GRIT_EXCEPT("Collision data had no compound or trimesh");
                /*
                } else if (binary) {
                    col_name += ".bcol";
                    GRIT_EXCEPT("Writing bcol not implemented.");
                */
            } else {

                std::ofstream out;
                out.open((vehicle_dir+"/chassis.gcol").c_str(), std::ios::binary);
                APP_ASSERT_IO_SUCCESSFUL(out,"opening gcol for writing");

                write_tcol_as_bcol(out, dff.tcol);
            }

            if (v.type == "car") {
                lua_file << "class_add(`/gtasa/"<<vname<<"`, extends(Vehicle) {\n";
                lua_file << "    castShadows = true;\n";
                lua_file << "    gfxMesh = `"<<vname<<"/chassis.mesh`;\n";
                lua_file << "    colMesh = `"<<vname<<"/chassis.gcol`;\n";
                lua_file << "    placementZOffset=0.4;\n";
                lua_file << "    powerPlots = {\n";
                float torque = vdata->mass * vdata->engine_accel * v.rear_wheel_size/2 * 0.5f; // 0.75 is a fudge factor
                lua_file << "        [-1] = { [0] = -"<<torque<<"; [10] = -"<<torque<<"; [25] = -"<<torque<<"; [40] = 0; };\n";
                lua_file << "        [0] = {};\n";
                lua_file << "        [1] = { [0] = "<<torque<<"; [10] = "<<torque<<"; [25] = "<<torque<<"; [60] = "<<torque<<"; };\n";
                lua_file << "    };\n";
                lua_file << "    meshWheelInfo = {\n";

                std::stringstream all_wheels;
                all_wheels << "castRadius=0.05;" << "mesh=`"<<vname<<"/wheel.mesh`;"
                           << "slack="<<(vdata->susp_upper)<<";"
                           << "len="<<(- vdata->susp_lower)<<";"
                           << "hookeFactor=1.0;";
                std::stringstream front_wheels;
                front_wheels << (vdata->front_wheel_drive?"drive=1;":"")
                             << "rad="<<v.front_wheel_size/2<<";"
                             << (vdata->steer_rearwheels?"":"steer=1;")
                             << "mu = "<<3*vdata->traction_mult<<";";
                std::stringstream rear_wheels;
                rear_wheels << (vdata->back_wheel_drive?"drive=1;":"")
                            << "rad="<<v.rear_wheel_size/2<<";"
                            << (vdata->steer_rearwheels?"steer=1;":"")
                            << (vdata->no_handbrake?"":"handbrake = true;")
                            << "mu = "<<3*vdata->traction_mult<<";";
                
                APP_ASSERT(fr_wheel_lf!=NULL);
                APP_ASSERT(fr_wheel_lb!=NULL);
                APP_ASSERT(fr_wheel_rf!=NULL);
                APP_ASSERT(fr_wheel_rb!=NULL);
                float x,y,z;
                
                x = fr_wheel_lf->x; y = fr_wheel_lf->y; z = fr_wheel_lf->z + vdata->susp_upper;
                lua_file << "        front_left = {\n";
                lua_file << "          "<<all_wheels.str() << front_wheels.str() << "left=true;\n";
                lua_file << "          attachPos=vector3("<<x<<","<<y<<","<<z<<");\n";
                lua_file << "        },\n";

                x = fr_wheel_rf->x; y = fr_wheel_rf->y; z = fr_wheel_rf->z + vdata->susp_upper;
                lua_file << "        front_right = {\n";
                lua_file << "          "<<all_wheels.str() << front_wheels.str() << "\n";
                lua_file << "          attachPos=vector3("<<x<<","<<y<<","<<z<<");\n";
                lua_file << "        },\n";

                x = fr_wheel_lb->x; y = fr_wheel_lb->y; z = fr_wheel_lb->z + vdata->susp_upper;
                lua_file << "        rear_left = {\n";
                lua_file << "          "<<all_wheels.str() << rear_wheels.str() << "left=true;\n";
                lua_file << "          attachPos=vector3("<<x<<","<<y<<","<<z<<");\n";
                lua_file << "        },\n";

                x = fr_wheel_rb->x; y = fr_wheel_rb->y; z = fr_wheel_rb->z + vdata->susp_upper;
                lua_file << "        rear_right = {\n";
                lua_file << "          "<<all_wheels.str() << rear_wheels.str() << "\n";
                lua_file << "          attachPos=vector3("<<x<<","<<y<<","<<z<<");\n";
                lua_file << "        },\n";

                lua_file << "    },\n";

                lua_file << "    colourSpec = {\n";
                std::vector<int> cols2 = carcols_2[vname];
                std::vector<int> cols4 = carcols_4[vname];
                APP_ASSERT(cols2.size()%2==0);
                APP_ASSERT(cols4.size()%4==0);
                for (unsigned i=0 ; i<cols2.size() ; i+=2) {
                    int c1 = cols2[i], c2 = cols2[i+1];
                    lua_file << "            {  {\"gtasa"<<c1<<"\"}, {\"gtasa"<<c2<<"\"} },\n";
                }
                for (unsigned i=0 ; i<cols4.size() ; i+=4) {
                    int c1 = cols4[i], c2 = cols4[i+1];
                    int c3 = cols4[i+2], c4 = cols4[i+3];
                    lua_file << "            {  {\"gtasa"<<c1<<"\"}, {\"gtasa"<<c2<<"\"}, "
                             << "{\"gtasa"<<c3<<"\"}, {\"gtasa"<<c4<<"\"} },\n";
                }
                lua_file << "    };\n";

                lua_file << "},{})" << std::endl;
            } else {
                lua_file << std::endl;
                lua_file << "class `../"<<vname<<"` (ColClass) {\n";
                lua_file << "    gfxMesh=`"<<vname<<"/chassis.mesh`;\n";
                lua_file << "    colMesh=`"<<vname<<"/chassis.gcol`;\n";
                lua_file << "    castShadows = true;\n";
                lua_file << "    colourSpec = {\n";
                std::vector<int> cols2 = carcols_2[vname];
                std::vector<int> cols4 = carcols_4[vname];
                APP_ASSERT(cols2.size()%2==0);
                APP_ASSERT(cols4.size()%4==0);
                for (unsigned i=0 ; i<cols2.size() ; i+=2) {
                    int c1 = cols2[i], c2 = cols2[i+1];
                    lua_file << "            {  {\"gtasa"<<c1<<"\"}, {\"gtasa"<<c2<<"\"} },\n";
                }
                for (unsigned i=0 ; i<cols4.size() ; i+=4) {
                    int c1 = cols4[i], c2 = cols4[i+1];
                    int c3 = cols4[i+2], c4 = cols4[i+3];
                    lua_file << "            {  {\"gtasa"<<c1<<"\"}, {\"gtasa"<<c2<<"\"}, "
                             << "{\"gtasa"<<c3<<"\"}, {\"gtasa"<<c4<<"\"} },\n";
                }
                lua_file << "    };\n";

                lua_file << "}" << std::endl;
            }

        }


    }

    vehicles_lua << std::endl;
    vehicles_lua << "all_vehicles = { ";

    for (Vehicles::iterator i=vehicles.begin(),i_=vehicles.end() ; i!=i_ ; ++i) {
        vehicles_lua << "`"<<i->dff<<"`, ";
    }
    vehicles_lua << "}" << std::endl;

    if (getenv("SKIP_MAP")==NULL) {
        out << "Exporting map..." << std::endl;
        std::ofstream map;
        map.open((dest_dir+"/"+cfg.modname+"/map.lua").c_str(),
             std::ios::binary);
        APP_ASSERT_IO_SUCCESSFUL(map, "opening map.lua");
        map.precision(25);

        map << "print(\"Loading world\")\n";
        map << "object_all_del()\n";

        map << "local last\n";

        for (IPLs::iterator i=ipls.begin(),i_=ipls.end() ; i!=i_ ; ++i) {
            const IPL &ipl = *i;
            const Insts &insts = ipl.getInsts();
            for (Insts::const_iterator j=insts.begin(),j_=insts.end() ;
                 j!=j_ ; ++j) {
                const Inst &inst = *j;
                if (inst.is_low_detail) continue;
                if (!ids_written_out[inst.id]) continue;
                if (inst.near_for==-1) {
                    map<<"object_add(`/gtasa/"<<inst.id<<"`,"
                       <<"vector3("<<inst.x<<","<<inst.y<<","<<inst.z<<")";
                    if (inst.rx!=0 || inst.ry!=0 || inst.rz!=0) {
                        map<<",{rot=quat("
                           <<inst.rw<<","<<inst.rx<<","
                           <<inst.ry<<","<<inst.rz<<")}";
                    }
                    map<<")\n";
                } else {
                    const Inst &far_inst = insts[inst.near_for];
                    map<<"last=object_add(`/gtasa/"<<inst.id<<"`,"
                       <<"vector3("<<inst.x<<","<<inst.y<<","<<inst.z<<")";
                    if (inst.rx!=0 || inst.ry!=0 || inst.rz!=0) {
                        map<<",{rot=quat("
                           <<inst.rw<<","<<inst.rx<<","
                           <<inst.ry<<","<<inst.rz<<")}";
                    }
                    map<<")\n";
                    map<<"object_add(`/gtasa/"<<far_inst.id<<"`,"
                       <<"vector3("<<far_inst.x<<","<<far_inst.y<<","<<far_inst.z<<")";
                    map<<",{";
                    if (far_inst.rx!=0 || far_inst.ry!=0 || far_inst.rz!=0) {
                        map<<"rot=quat("
                           <<far_inst.rw<<","<<far_inst.rx<<","
                           <<far_inst.ry<<","<<far_inst.rz<<"),";
                    }
                    map<<"near=last}";
                    map<<")\n";
                }
            }
        }
    }

    out << "Export complete." << std::endl;

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
    cfg.txds.push_back(P("/models/effectsPC.txd","effectsPC.txd"));
    cfg.txds.push_back(P("/models/fonts.txd","fonts.txd"));
    cfg.txds.push_back(P("/models/fronten1.txd","fronten1.txd"));
    cfg.txds.push_back(P("/models/fronten2.txd","fronten2.txd"));
    cfg.txds.push_back(P("/models/fronten3.txd","fronten3.txd"));
    cfg.txds.push_back(P("/models/fronten_pc.txd","fronten_pc.txd"));
    cfg.txds.push_back(P("/models/generic/vehicle.txd","generic/vehicle.txd"));
    cfg.txds.push_back(P("/models/generic/wheels.txd","generic/wheels.txd"));
    cfg.txds.push_back(P("/models/grass/plant1.txd","grass/plant1.txd"));
    cfg.txds.push_back(P("/models/hud.txd","hud.txd"));
    cfg.txds.push_back(P("/models/misc.txd","misc.txd"));
    cfg.txds.push_back(P("/models/particle.txd","particle.txd"));
    cfg.txds.push_back(P("/models/pcbtns.txd","pcbtns.txd"));
    cfg.txds.push_back(P("/models/txd/LD_RCE2.txd","txd/LD_RCE2.txd"));
    cfg.txds.push_back(P("/models/txd/intro1.txd","txd/intro1.txd"));
    cfg.txds.push_back(P("/models/txd/intro2.txd","txd/intro2.txd"));
    cfg.txds.push_back(P("/models/txd/intro4.txd","txd/intro4.txd"));
    cfg.txds.push_back(P("/models/txd/LD_BEAT.txd","txd/LD_BEAT.txd"));
    cfg.txds.push_back(P("/models/txd/LD_BUM.txd","txd/LD_BUM.txd"));
    cfg.txds.push_back(P("/models/txd/LD_CARD.txd","txd/LD_CARD.txd"));
    cfg.txds.push_back(P("/models/txd/LD_CHAT.txd","txd/LD_CHAT.txd"));
    cfg.txds.push_back(P("/models/txd/LD_DRV.txd","txd/LD_DRV.txd"));
    cfg.txds.push_back(P("/models/txd/LD_DUAL.txd","txd/LD_DUAL.txd"));
    cfg.txds.push_back(P("/models/txd/ld_grav.txd","txd/ld_grav.txd"));
    cfg.txds.push_back(P("/models/txd/LD_NONE.txd","txd/LD_NONE.txd"));
    cfg.txds.push_back(P("/models/txd/LD_OTB.txd","txd/LD_OTB.txd"));
    cfg.txds.push_back(P("/models/txd/LD_OTB2.txd","txd/LD_OTB2.txd"));
    cfg.txds.push_back(P("/models/txd/LD_PLAN.txd","txd/LD_PLAN.txd"));
    cfg.txds.push_back(P("/models/txd/LD_POKE.txd","txd/LD_POKE.txd"));
    cfg.txds.push_back(P("/models/txd/LD_POOL.txd","txd/LD_POOL.txd"));
    cfg.txds.push_back(P("/models/txd/LD_RACE.txd","txd/LD_RACE.txd"));
    cfg.txds.push_back(P("/models/txd/LD_RCE1.txd","txd/LD_RCE1.txd"));
    cfg.txds.push_back(P("/models/txd/LD_RCE3.txd","txd/LD_RCE3.txd"));
    cfg.txds.push_back(P("/models/txd/LD_RCE4.txd","txd/LD_RCE4.txd"));
    cfg.txds.push_back(P("/models/txd/LD_RCE5.txd","txd/LD_RCE5.txd"));
    cfg.txds.push_back(P("/models/txd/LD_ROUL.txd","txd/LD_ROUL.txd"));
    cfg.txds.push_back(P("/models/txd/ld_shtr.txd","txd/ld_shtr.txd"));
    cfg.txds.push_back(P("/models/txd/LD_SLOT.txd","txd/LD_SLOT.txd"));
    cfg.txds.push_back(P("/models/txd/LD_SPAC.txd","txd/LD_SPAC.txd"));
    cfg.txds.push_back(P("/models/txd/LD_TATT.txd","txd/LD_TATT.txd"));
    cfg.txds.push_back(P("/models/txd/load0uk.txd","txd/load0uk.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc0.txd","txd/loadsc0.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc1.txd","txd/loadsc1.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc10.txd","txd/loadsc10.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc11.txd","txd/loadsc11.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc12.txd","txd/loadsc12.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc13.txd","txd/loadsc13.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc14.txd","txd/loadsc14.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc2.txd","txd/loadsc2.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc3.txd","txd/loadsc3.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc4.txd","txd/loadsc4.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc5.txd","txd/loadsc5.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc6.txd","txd/loadsc6.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc7.txd","txd/loadsc7.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc8.txd","txd/loadsc8.txd"));
    cfg.txds.push_back(P("/models/txd/loadsc9.txd","txd/loadsc9.txd"));
    cfg.txds.push_back(P("/models/txd/LOADSCS.txd","txd/LOADSCS.txd"));
    cfg.txds.push_back(P("/models/txd/LOADSUK.txd","txd/LOADSUK.txd"));
    //cfg.txds.push_back(P("/models/txd/outro.txd","txd/outro.txd"));
    cfg.txds.push_back(P("/models/txd/splash1.txd","txd/splash1.txd"));
    cfg.txds.push_back(P("/models/txd/splash2.txd","txd/splash2.txd"));
    cfg.txds.push_back(P("/models/txd/splash3.txd","txd/splash3.txd"));



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
        "txdcut.ide",
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
    } catch (const Exception &e) {
        CERR << e << std::endl;

        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// vim: shiftwidth=4:tabstop=4:expandtab
