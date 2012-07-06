#include <iostream>
#include <fstream>

#include "csvread.h"
#include "ios_util.h"

void read_csv (std::istream &f, Csv &csv)
{
        bool in_section = false;
        std::string section = "nosection";
        {
                CsvSection &s = csv[section];
                s.section_name = section;
        }

        std::vector<std::string> strs;

        size_t orig_line_counter = 0;
        size_t section_line_counter = 0;

        for (std::string str_ ; std::getline(f,str_) ; ) {

                orig_line_counter++;

                std::string::size_type b, e;

                e = str_.find('\r');
                str_ = str_.substr(0,e);

                e = str_.find('\n');
                str_ = str_.substr(0,e);

                e = str_.find(';');
                str_ = str_.substr(0,e);

                e = str_.find('#');
                str_ = str_.substr(0,e);

                e = str_.find("//");
                str_ = str_.substr(0,e);

                if (str_[0] == '*') continue;

                b = str_.find_first_not_of("\t ");
                e = str_.find_last_not_of("\t ");
                if (b==str_.npos) continue;
                if (e==str_.npos) continue;
                str_ = str_.substr(b,e+1);

                std::string str;
                bool last_was_wspace = false;
                for (size_t i=0 ; i<str_.size() ; i++) {
                        bool wspace = str_[i]==' ' || str_[i]=='\t' || str_[i]==',';
                        if (!wspace) {
                                str += str_[i];
                        } else if (!last_was_wspace) {
                                str += ",";
                        }
                        last_was_wspace = wspace;
                }

                if (str.size()==0) continue;

                if (!in_section && str.find(",")==str.npos) {
                        section = str;
                        in_section = true;
                        section_line_counter = 0;
                        if (csv.find(section)!=csv.end()) {
                            GRIT_EXCEPT("Already seen section \""+section+"\"  in this file");
                        } else {
                                CsvSection &s = csv[section];
                                s.section_name = section;
                        }
                } else {
                        if (str=="end") {
                                in_section = false;
                                section = "nosection";
                        } else {
                                section_line_counter++;
                                CsvSection &s = csv[section];
                                CsvLine line;
                                line.orig_line = orig_line_counter;
                                line.section_line = section_line_counter;
                                std::string val;
                                for (size_t i=0 ; i<str.size() ; i++) {
                                        if (str[i]!=',') {
                                                val += str[i];
                                        } else {
                                            line.push_back(val);
                                            val = "";
                                        }
                                }
                                line.push_back(val);
                                s.push_back(line);
                        }
                }
        }
        if (in_section) {
                if (csv.filename != "") {
                        GRIT_EXCEPT("CSV file did not close section \""+section+"\" with 'end'");
                } else {
                }
                        GRIT_EXCEPT("CSV file \""+csv.filename+"\" did not close section \""
                                   +section+"\" with 'end'");
        }
}

#ifdef _CSVREAD_EXEC

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

void assert_triggered (void) { } 

int main(int argc, char *argv[])
{
        if (argc!=2) {
                std::cerr<<"Usage: "<<argv[0]<<" <csv file>"<<std::endl;
                return EXIT_FAILURE;
        }

        try {

                std::ifstream csvfstream;
                std::istream *csvstream = &csvfstream;
                std::string filename;

                if (strcmp(argv[1],"-")==0) {
                        csvstream = &std::cin;
                        filename = "<stdin>";
                } else {
                        filename = argv[1];
                        csvfstream.open (filename.c_str());
                        APP_ASSERT_IO_SUCCESSFUL(csvfstream,
                                          "Opening csv: "+filename);
                        if (csvfstream.fail() || csvfstream.bad()) {
                                std::stringstream ss;
                                ss << filename << ": IO Error: " << strerror(errno) << "\n";
                                GRIT_EXCEPT(ss.str());
                        }
                }

                Csv csv;
                csv.filename = filename;
                read_csv(*csvstream,csv);

                for (Csv::iterator i=csv.begin(), i_=csv.end() ; i!=i_ ; ++i) {
                        const std::string section = i->first;
                        const CsvSection &lines = i->second;

                        APP_ASSERT(section == i->second.section_name);

                        if (section=="nosection" && lines.size()==0) continue;

                        std::cout << section << std::endl;


                        for (unsigned j=0 ; j<lines.size() ; ++j) {

                                CsvLine line = lines[j];

                                for (unsigned k=0 ; k<line.size() ; ++k) {
                                        const std::string &val = line[k];
                                        std::cout << (k>0?",":"") << val;
                                }
                                
                            std::cout << std::endl;

                        }

                        std::cout << "end" << std::endl;
                }

        } catch (GritException &e) {

                CERR << e << std::endl;
                return EXIT_FAILURE;

        }

        return EXIT_SUCCESS;
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
