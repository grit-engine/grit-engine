#include <iostream>
#include <fstream>

#include "surfinfo.h"
#include "ios_util.h"

static float f (const std::string &s) { return (float)strtod(s.c_str(), NULL); }
static int dec (const std::string &s) { return (int)strtod(s.c_str(), NULL); }
static bool b (const std::string &s) { return dec(s)==1; }

void read_surfinfo (Csv &csv, SurfInfoData &data)
{
        const CsvSection &s = csv["nosection"];
        for (unsigned i=0 ; i<s.size() ; ++i) {
                const CsvLine &line = s[i];
                APP_ASSERT(line.size()>=36);

                SurfInfo v;
                v.name = line[0];

                v.adhesion_group = line[1];
                v.tyre_grip = f(line[2]);;
                v.wet_grip = f(line[3]);;
                v.skidmark = line[4];
                v.friction_effect = line[5];
                v.soft_landing = b(line[6]);
                v.see_through = b(line[7]);
                v.shoot_through = b(line[8]);
                v.sand = b(line[9]);
                v.water = b(line[10]);
                v.shallow_water = b(line[11]);
                v.beach = b(line[12]);
                v.steep_slope = b(line[13]);
                v.glass = b(line[14]);
                v.stairs = b(line[15]);
                v.skateable = b(line[16]);
                v.pavement = b(line[17]);
                v.roughness = dec(line[18]);
                v.flammability = dec(line[19]);
                v.sparks = b(line[20]);
                v.sprint = b(line[21]);
                v.footsteps = b(line[22]);
                v.footdust = b(line[23]);
                v.cardirt = b(line[24]);
                v.carclean = b(line[25]);
                v.w_grass = b(line[26]);
                v.w_gravel = b(line[27]);
                v.w_mud = b(line[28]);
                v.w_dust = b(line[29]);
                v.w_sand = b(line[30]);
                v.w_spray = b(line[31]);
                v.proc_plant = b(line[32]);
                v.proc_obj = b(line[33]);
                v.climbable = b(line[34]);
                v.bullet_fx = line[35];

                data.surfaces.push_back(v);
        }
        for (unsigned i=0 ; i<data.surfaces.size() ; ++i) {
                SurfInfo &v = data.surfaces[i];
                data[v.name] = &v;
        }
}



#ifdef _SURFINFO_EXEC

#include "console_colour.h"

void assert_triggered (void) { }

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

int main(int argc, char *argv[])
{
        if (argc!=2) {
                std::cerr<<"Usage: "<<argv[0]<<" <surfinfo.dat file>"<<std::endl;
                return EXIT_FAILURE;
        }

        try {

                std::ifstream surfinfofstream;
                std::istream *surfinfostream = &surfinfofstream;
                std::string filename;

                if (strcmp(argv[1],"-")==0) {
                        surfinfostream = &std::cin;
                        filename = "<stdin>";
                } else {
                        filename = argv[1];
                        surfinfofstream.open (filename.c_str());
                        APP_ASSERT_IO_SUCCESSFUL(surfinfofstream,
                                          "Opening surfinfo: "+filename);
                        if (surfinfofstream.fail() || surfinfofstream.bad()) {
                                std::stringstream ss;
                                ss << filename << ": IO Error: " << strerror(errno) << "\n";
                                GRIT_EXCEPT(ss.str());
                        }
                }

                Csv surfinfo;
                surfinfo.filename = filename;
                read_csv(*surfinfostream,surfinfo);
                SurfInfoData data;
                read_surfinfo(surfinfo, data);

                for (SurfInfoData::iterator i=data.begin(), i_=data.end() ; i!=i_ ; ++i) {
                        const std::string name = i->first;
                        const SurfInfo &v = *i->second;

                        APP_ASSERT(name == v.name);

                        std::cout << name << std::endl;
                }

        } catch (GritException &e) {

                CERR << e << std::endl;
                return EXIT_FAILURE;

        }

        return EXIT_SUCCESS;
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
