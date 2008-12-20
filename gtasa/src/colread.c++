#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <fstream>

#include "ios_util.h"
#include "TColParser.h"
#include "ColParser.h"

#define VERSION "1.0"

const char *info =
"colread (c) Dave Cunningham 2007  (version: "VERSION")\n"
"I can read COLL COL2 COL3 TCOL and BCOL files.\n"
"I can write TCOL and BCOL files.\n"
"Some TCOL features are preprocessed on read, such as hull shrinking.\n\n";

const char *usage =
"Usage: colread { <opt> }\n\n"
"where <opt> ::= \"-v\" | \"--verbose\"             increase debug level\n"
"              | \"-q\" | \"--quiet\"               decrease debug level\n"
"              | \"-h\" | \"--help\"                this message\n\n"
"              | \"-i\" | \"--input\" <name>        input file\n"
"              | \"-o\" | \"--output\" <name>       output file\n"
"                                               (if tcol/bcol input)\n\n"
"              | \"-B\" | \"--output-bcol\"         output binary\n"
"              | \"-T\" | \"--output-tcol\"         output text (default)\n";

std::string next_arg(int& so_far, int argc, char **argv)
{
        if (so_far==argc) {
                std::cerr<<"Ran out of arguments."<<std::endl;
                std::cerr<<usage<<std::endl;
                exit(EXIT_FAILURE);
        }
        return argv[so_far++];
}

int main(int argc, char **argv)
{
        if (argc==1) {
                std::cout<<info<<std::endl;
                std::cout<<usage<<std::endl;
                return EXIT_SUCCESS;
        }

        // default parameters
        int debug_level = 0;
        bool output_binary = false;
        std::string input_filename;
        std::string output_filename;

        int so_far = 1;

        while (so_far<argc) {
                std::string arg = next_arg(so_far,argc,argv);
                if (arg=="-v" || arg=="--verbose") {
                        debug_level++;
                } else if (arg=="-q" || arg=="--quiet") {
                        debug_level--;
                } else if (arg=="-i" || arg=="--input") {
                        input_filename = next_arg(so_far,argc,argv);
                } else if (arg=="-o" || arg=="--output") {
                        output_filename = next_arg(so_far,argc,argv);
                } else if (arg=="-B" || arg=="--output-bcol") {
                        output_binary = true;
                } else if (arg=="-T" || arg=="--output-tcol") {
                        output_binary = false;
                } else if (arg=="-h" || arg=="--help") {
                        std::cout<<info<<std::endl;
                        std::cout<<usage<<std::endl;
                } else {
                        std::cerr<<"Unrecognised argument: "<<arg<<std::endl;
                        std::cerr<<usage<<std::endl;
                        return EXIT_FAILURE;
                }
        }

        std::istream *in = NULL;
        std::ostream *out = NULL;

        try {

                if (input_filename=="" || input_filename=="-") {
                        in = &std::cin;
                } else {
                        std::ifstream *in_ = new std::ifstream();
                        in = in_;
                        in_->open(input_filename.c_str(), std::ios::binary);
                        ASSERT_IO_SUCCESSFUL(*in_,
                                             "opening "+input_filename);
                }

                unsigned long fourcc = ios_peek_u32(*in);
                ASSERT_IO_SUCCESSFUL(*in,"reading fourcc");

                if (fourcc==0x4c4f4354) { //TCOL

                        TColFile tcol;

                        quex::TColLexer* qlex =
                                new quex::TColLexer(in);
                        parse_tcol_1_0(input_filename,qlex,tcol);
                        delete qlex;

                        if (output_filename==""||output_filename=="-") {
                                out = &std::cout;
                        } else {
                                std::ofstream *out_=new std::ofstream();
                                out = out_;
                                out_->open(output_filename.c_str(),
                                           std::ios::binary);
                                ASSERT_IO_SUCCESSFUL(*out_,
                                                    "opening "+output_filename);
                        }
                        pretty_print_tcol(*out,tcol);

                } else if (fourcc==0x4c4f4342) { //BCOL
                        IOS_EXCEPT("Reading bcol not implemented.");
                } else if (fourcc==0x4c4c4f43) { // COLL
                                dump_all_cols(*in,output_binary,debug_level);
                } else if (fourcc==0x324c4f43) { // COL2
                                dump_all_cols(*in,output_binary,debug_level);
                } else if (fourcc==0x334c4f43) { // COL3
                                dump_all_cols(*in,output_binary,debug_level);
                } else {
                        IOS_EXCEPT("Unrecognised fourcc tag.");
                }


        } catch (Exception& e) {

                std::cerr << "ERROR: " << e.getFullDescription() << std::endl;

                if (in!=&std::cin) delete in;
                if (out!=&std::cout) delete out;

                return EXIT_FAILURE;

        }

        if (in!=&std::cin) delete in;
        if (out!=&std::cout) delete out;

        return EXIT_SUCCESS;
}

// vim: shiftwidth=8:tabstop=8:expandtab
