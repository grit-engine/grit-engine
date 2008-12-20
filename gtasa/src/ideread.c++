#include <climits>
#include <cmath>
#include <cerrno>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <istream>
#include <iomanip>
#include <algorithm>


#include "ideread.h"
#include "ios_util.h"

static int tolowr (int c)
{
        return std::tolower(char(c),std::cout.getloc());
}

static std::string& strlower (std::string& s)
{
        std::transform(s.begin(),s.end(), s.begin(),tolowr);
        return s;
}

static std::string& str_lcase_crop (std::string& str)
{
        strlower(str);
        std::string::size_type b=str.find_first_not_of(' ');
        std::string::size_type e=str.find_last_not_of(' ');
        str = str.substr(b,e+1);
        return str;
}


long get_long(const std::string& val_, const char* name)
{
        double val =  strtod(val_.c_str(),NULL);
        if (val!=floor(val)){
                std::cerr<<name<<" not an integer: \""<<val_<<"\""<<std::endl;
                exit(EXIT_FAILURE);
        }
        return (long) floor(val);
}

unsigned long get_ulong(const std::string& val_, const char* name)
{
        double val =  strtod(val_.c_str(),NULL);
        if (val!=floor(val) || val<0) {
                std::cerr<<name<<" not a positive integer: \""
                         <<val_<<"\""<<std::endl;
                exit(EXIT_FAILURE);
        }
        return (unsigned long) floor(val);
}

unsigned char get_uchar(const std::string& val_, const char* name)
{
        double val =  strtod(val_.c_str(),NULL);
        if (val!=floor(val) || val<0 || val>UCHAR_MAX) {
                std::cerr<<name<<" not a positive integer <=255: \""
                         <<val_<<"\""<<std::endl;
                exit(EXIT_FAILURE);
        }
        return (unsigned char) floor(val);
}

bool get_bool(const std::string& val_, const char* name)
{
        double val =  strtod(val_.c_str(),NULL);
        if (val!=0 && val!=1) {
                std::cerr<<name<<" not a boolean value (1 or 0): \""
                         <<val_<<"\""<<std::endl;
                exit(EXIT_FAILURE);
        }
        return val==1.0;
}

unsigned long get_hex(const std::string& val_, const char* name)
{
        if (val_.size()==0) {
                std::cerr<<name<<" not a hex value: \""<<val_<<"\""<<std::endl;
                exit(EXIT_FAILURE);
        }
        char *ret;
        unsigned long val =  strtoul(val_.c_str(),&ret,16);
        if (*ret!=0) {
                std::cerr<<name<<" not a hex value: \""<<val_<<"\""<<std::endl;
                exit(EXIT_FAILURE);
        }
        return val;
}

void read_ide (std::istream &f, struct ide *ide)
{
        std::string section("no section");

        std::vector<std::string> strs;

        for (std::string str ; std::getline(f,str) ; ) {

                size_t len = str.size();
                if (len==0) continue;
                if (str[0]=='#') continue;

                bool all_whitespace = true;
                for (size_t i=0 ; i<str.size() ; i++) {
                        if (str[i]!='\n' &&
                            str[i]!='\r' &&
                            str[i]!=' ' &&
                            str[i]!='\t') {
                                //std::cerr<<(int)str[i]<<std::endl;
                                all_whitespace = false;
                        } else {
                                str[i] = ' ';
                        }
                        if (str[i]==',') str[i] = ' ';
                }

                if (all_whitespace) continue;

                std::string::size_type b = str.find_first_not_of(' ');
                std::string::size_type e = str.find_last_not_of(' ');
                str = str.substr(b,e+1);

                if (str=="hier") { section = str; continue; }
                if (str=="weap") { section = str; continue; }
                if (str=="objs") { section = str; continue; }
                if (str=="tobj") { section = str; continue; }
                if (str=="path") { section = str; continue; }
                if (str=="2dfx") { section = str; continue; }
                if (str=="anim") { section = str; continue; }
                if (str=="txdp") { section = str; continue; }
                if (str=="peds") { section = str; continue; }
                if (str=="cars") { section = str; continue; }
                if (str=="end")  { section = "between sections" ; continue; }


                std::stringstream ss;
                ss << str;
                strs.clear();
                for (std::string word ; std::getline(ss,word,' ') ; ) {
                        //std::cout<<word2<<std::endl;
                        if  (word=="") continue;
                        strs.push_back(word);
                }

                // airtrain_vlo in default.ide has 6 for some reason
                if (section=="objs" && strs.size()==5) {
                        Obj obj;
                        obj.id = get_ulong(strs[0],"Id");
                        obj.dff = str_lcase_crop(strs[1]);
                        obj.txd = str_lcase_crop(strs[2]);
                        obj.draw_distance = (float)strtod(strs[3].c_str(),NULL);
                        obj.flags = get_ulong(strs[4],"Flags");
                        ASSERT((obj.flags|0x77feef) == 0x77feef);
                        ide->objs.push_back(obj);
                } else if (section=="objs" && strs.size()==6) {
                        // mysterious airtrain_vlo thing
                } else if (section=="tobj" && strs.size()==7) {
                        TObj tobj;
                        tobj.id = get_ulong(strs[0],"Id");
                        tobj.dff = str_lcase_crop(strs[1]);
                        tobj.txd = str_lcase_crop(strs[2]);
                        tobj.draw_distance =(float)strtod(strs[3].c_str(),NULL);
                        tobj.flags = get_ulong(strs[4],"Flags");
                        ASSERT((tobj.flags|0x77feef) == 0x77feef);
                        tobj.hour_on = get_uchar(strs[5],"Hour on");
                        tobj.hour_off = get_uchar(strs[6],"Hour off");
                        ide->tobjs.push_back(tobj);
                } else if (section=="2dfx" && strs.size()==38) {
                        //std::cout<<"In "<<section<<" ["<<strs.size()<<"] "
                        //         <<str<<"\n";
                } else if (section=="anim" && strs.size()==6) {
                        Anim anim;
                        anim.id = get_ulong(strs[0],"Id");
                        anim.dff = str_lcase_crop(strs[1]);
                        anim.txd = str_lcase_crop(strs[2]);
                        anim.ifp_file = str_lcase_crop(strs[3]);
                        anim.draw_distance =(float)strtod(strs[4].c_str(),NULL);
                        anim.flags = get_ulong(strs[5],"Flags");
                        ASSERT((anim.flags|0x77feef) == 0x77feef);
                        ide->anims.push_back(anim);
                } else if (section=="txdp" && strs.size()==2) {
                        TXDP txdp;
                        txdp.txd1 = str_lcase_crop(strs[0]);
                        txdp.txd2 = str_lcase_crop(strs[1]);
                        ide->txdps.push_back(txdp);
                } else if (section=="weap" && strs.size()==7) {
                        Weap weap;
                        weap.id = get_ulong(strs[0],"Id");
                        weap.dff = str_lcase_crop(strs[1]);
                        weap.txd = str_lcase_crop(strs[2]);
                        weap.type = str_lcase_crop(strs[3]);
                        weap.unk_one = get_ulong(strs[4],"Unknown");
                        ASSERT(weap.unk_one==1);
                        weap.unk_num = get_ulong(strs[5],"Unknown");
                        weap.unk_zero = get_ulong(strs[6],"Unknown");
                        ASSERT(weap.unk_zero==0);
                        ide->weaps.push_back(weap);
                } else if (section=="hier" && strs.size()==5) {
                } else if (section=="peds" && strs.size()==14) {
                        Ped ped;
                        ped.id = get_ulong(strs[0],"Id");
                        ped.dff = str_lcase_crop(strs[1]);
                        ped.txd = str_lcase_crop(strs[2]);
                        ped.type = str_lcase_crop(strs[3]);
                        ped.stat_type = str_lcase_crop(strs[4]);
                        ped.anim_group = str_lcase_crop(strs[5]);
                        ped.can_drive = get_ulong(strs[6],"can_drive");
                        ped.buys_drugs = get_bool(strs[7],"buys drugs");
                        ped.anim_file = str_lcase_crop(strs[8]);
                        ped.radio1 = get_ulong(strs[9],"radio1");
                        ped.radio2 = get_ulong(strs[10],"radio2");
                        ped.unk1 = str_lcase_crop(strs[11]);
                        ped.unk2 = str_lcase_crop(strs[12]);
                        ped.unk3 = str_lcase_crop(strs[13]);
                        ide->peds.push_back(ped);
                } else if (section=="cars"&&(strs.size()==11||strs.size()==15)) {
                        Vehicle vehicle;
                        vehicle.id = get_ulong(strs[0],"Id");
                        vehicle.dff = str_lcase_crop(strs[1]);
                        vehicle.txd = str_lcase_crop(strs[2]);
                        //bike bmx boat car heli mtruck plane quad trailer train
                        vehicle.type = str_lcase_crop(strs[3]);
                        vehicle.handling_id = str_lcase_crop(strs[4]);
                        vehicle.game_name = str_lcase_crop(strs[5]);
                        // BF_injection biked bikeh bikes bikev bmx bus choppa
                        // coach dozer KART mtb nevada null quad rustler shamal
                        // tank truck van vortex wayfarer
                        vehicle.anims = str_lcase_crop(strs[6]);
                        // bicycle big executive ignore leisureboat moped
                        // motorbike normal poorfamily richfamily taxi worker
                        // workerboat
                        vehicle.class_ = str_lcase_crop(strs[7]);
                        vehicle.freq = get_ulong(strs[8],"Frequency");
                        vehicle.flags = get_ulong(strs[9],"Flags");
                        // 0 1012 1f10 1f341210 2ff0 3012 30123345
                        // 3210 3f01 3f10 3f341210 4fff
                        vehicle.comp_rules = get_hex(strs[10],"CompRules");
                        // boats do not have the following:
                        if (strs.size()==15) {
                                ASSERT(vehicle.type!="boat");
                                // on bikes, 16 or 23, otherwise -1
                                vehicle.unk1 = get_long(strs[11],"Unknown1");
                                ASSERT(vehicle.type=="bike" ||
                                           vehicle.type=="bmx" ||
                                           vehicle.unk1==-1);
                                ASSERT(vehicle.type!="bmx" ||
                                           vehicle.unk1==16||vehicle.unk1==23);
                                ASSERT(vehicle.type!="bike" ||
                                           vehicle.unk1==16||vehicle.unk1==23);
                                vehicle.front_wheel_size =
                                        (float)strtod(strs[12].c_str(),NULL);
                                vehicle.rear_wheel_size =
                                        (float)strtod(strs[13].c_str(),NULL);
                                // -1 0 1 2 (on non-cars always -1)
                                vehicle.unk2 = get_long(strs[14],"Unknown2");
                                ASSERT(vehicle.unk2>=-1 && vehicle.unk2<=2);
                                ASSERT(vehicle.type=="car" ||
                                           vehicle.unk2==-1);
                        }
                        ide->vehicles.push_back(vehicle);
                } else {
                        std::cerr<<"In "<<section<<", couldn't understand \""
                                <<str<<"\" "<<strs.size()<<std::endl;
                }
        }
}


#ifdef _IDEREAD_TEST

#include "console_colour.h"

void app_verbose(char const* file, int line, const std::string& msg)
{
        std::cout<<BOLD GREEN"VERBOSE "RESET
                 <<BOLD<<file<<NOBOLD":"BOLD<<line<<NOBOLD
                 <<": \""BOLD BLUE<<msg<<RESET"\"";
        std::cout<<std::endl;
}

void app_error(char const* file, int line,
               const std::string& i_was, const std::string& msg)
{
        std::cout<<BOLD RED"ERROR "RESET
                 <<BOLD<<file<<NOBOLD":"BOLD<<line<<NOBOLD
                 <<": \""BOLD YELLOW<<msg<<RESET"\"";
        if (i_was!="")
                std::cout<<" ("BOLD YELLOW<<i_was<<RESET")";
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

std::string binu32(unsigned int flags)
{
        std::stringstream ss;
        for (int i=0 ; i<32 ; i++) {
                ss << (flags >> 31);
                flags <<= 1;
        }
        return ss.str();
}

int main(int argc, char *argv[])
{
        if (argc!=2) {
                std::cerr<<"Usage: "<<argv[0]<<" <ide file>"<<std::endl;
                return EXIT_FAILURE;
        }

        try {

                std::ifstream idefstream;
                std::istream *idestream = &idefstream;

                if (strcmp(argv[1],"-")==0) {
                        idestream = &std::cin;
                } else {
                        idefstream.open (argv[1]);
                        ASSERT_IO_SUCCESSFUL(idefstream,
                                          "Opening ide: "+std::string(argv[1]));
                        if (idefstream.fail() || idefstream.bad()) {
                                std::cout << argv[1] << ": IO Error: "
                                          << strerror(errno) << "\n";
                        }
                }

                struct ide ide;
                read_ide(*idestream,&ide);

                for (size_t i=0 ; i<ide.objs.size() ; i++) {
                        std::cout << "obj: "<<ide.objs[i].id<<" "
                                             <<"\""<<ide.objs[i].dff<<"\" "
                                             <<"\""<<ide.objs[i].txd<<"\" "
                                             <<ide.objs[i].draw_distance<<" "
                                             <<binu32(ide.objs[i].flags)<<"\n";
                }
                for (size_t i=0 ; i<ide.tobjs.size() ; i++) {
                        std::cout << "tobj: "<<ide.tobjs[i].id<<" "
                                             <<"\""<<ide.tobjs[i].dff<<"\" "
                                             <<"\""<<ide.tobjs[i].txd<<"\" "
                                             <<ide.tobjs[i].draw_distance<<" "
                                             <<binu32(ide.tobjs[i].flags)<<"\n";
                }
                for (size_t i=0 ; i<ide.anims.size() ; i++) {
                        std::cout << "anims: "<<ide.anims[i].id<<" "
                                             <<"\""<<ide.anims[i].dff<<"\" "
                                             <<"\""<<ide.anims[i].txd<<"\" "
                                             <<ide.anims[i].draw_distance<<" "
                                             <<binu32(ide.anims[i].flags)<<"\n";
                }

                for (size_t i=0 ; i<ide.txdps.size() ; i++) {
                        std::cout << "txdp: "<<ide.txdps[i].txd1<<"/"
                                             <<ide.txdps[i].txd2<<std::endl;
                }

        } catch (Exception &e) {

                std::cerr << "ERROR: " << e.getFullDescription() << std::endl;
                return EXIT_FAILURE;

        }

        return EXIT_SUCCESS;
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
