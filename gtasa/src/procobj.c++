#include <iostream>
#include <fstream>

#include "procobj.h"
#include "ios_util.h"

static float f (const std::string &s) { return (float)strtod(s.c_str(), NULL); }
static bool b (const std::string &s) { return ((int)f(s))==1; }

void read_procobj (Csv &csv, ProcObjData &data)
{
        const CsvSection &s = csv["nosection"];
        for (unsigned i=0 ; i<s.size() ; ++i) {
                const CsvLine &line = s[i];

                ProcObj v;
                v.name = line[0];

                v.object_name = line[1];
                v.spacing = f(line[2]);
                v.mindist = f(line[3]);
                v.minrot = f(line[4]);
                v.maxrot = f(line[5]);
                v.minscl = f(line[6]);
                v.maxscl = f(line[7]);
                v.minsclz = f(line[8]);
                v.maxsclz = f(line[9]);
                v.zoffmin = f(line[10]);
                v.zoffmax = f(line[11]);
                v.align = b(line[12]);;
                v.use_grid = b(line[13]);;

                data.procobjs.push_back(v);
        }
        for (unsigned i=0 ; i<data.procobjs.size() ; ++i) {
                ProcObj &v = data.procobjs[i];
                data[v.name].push_back(&v);
        }
}



#ifdef _PROCOBJ_EXEC

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

int main(int argc, char *argv[])
{
        if (argc!=2) {
                std::cerr<<"Usage: "<<argv[0]<<" <procobj.dat file>"<<std::endl;
                return EXIT_FAILURE;
        }

        try {

                std::ifstream procobjfstream;
                std::istream *procobjstream = &procobjfstream;
                std::string filename;

                if (strcmp(argv[1],"-")==0) {
                        procobjstream = &std::cin;
                        filename = "<stdin>";
                } else {
                        filename = argv[1];
                        procobjfstream.open (filename.c_str());
                        ASSERT_IO_SUCCESSFUL(procobjfstream,
                                          "Opening procobj: "+filename);
                        if (procobjfstream.fail() || procobjfstream.bad()) {
                                std::stringstream ss;
                                ss << filename << ": IO Error: " << strerror(errno) << "\n";
                                IOS_EXCEPT(ss.str());
                        }
                }

                Csv procobj;
                procobj.filename = filename;
                read_csv(*procobjstream,procobj);
                ProcObjData data;
                read_procobj(procobj, data);

                for (ProcObjData::iterator i=data.begin(), i_=data.end() ; i!=i_ ; ++i) {
                        const std::string name = i->first;
                        std::cout << name << ":  ";
                        for (unsigned j=0 ; j<i->second.size() ; ++j) {
                                const ProcObj &v = *i->second[j];

                                ASSERT(name == v.name);
                                std::cout << (j==0?"":", ") << v.object_name;

                        }
                        std::cout << std::endl;
                }

        } catch (Exception &e) {

                std::cerr << "ERROR: " << e.getFullDescription() << std::endl;
                return EXIT_FAILURE;

        }

        return EXIT_SUCCESS;
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
