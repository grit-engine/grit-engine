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

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <fstream>

#include "ios_util.h"
#include "physics/TColParser.h"
#include "ColParser.h"

#define VERSION "1.0"

const char *info =
"colread (c) Dave Cunningham 2007  (version: "VERSION")\n"
"I can read COLL COL2 COL3 TCOL and BCOL files.\n"
"I can write TCOL and BCOL files.\n"
"Some TCOL features are preprocessed on read, such as hull shrinking.\n\n";

const char *usage =
"Usage: colread { <opt> }\n\n"
"where <opt> ::= \"-v\" | \"--verbose\"         increase debug level\n"
"              | \"-q\" | \"--quiet\"           decrease debug level\n"
"              | \"-h\" | \"--help\"            this message\n\n"
"              | \"-i\" | \"--input\" <name>    input file\n"
"              | \"-o\" | \"--output\" <name>   output file\n"
"                                                 (ignored if binary col found)\n\n"
"              | \"-B\" | \"--output-bcol\"     output binary\n"
"              | \"-T\" | \"--output-tcol\"     output text (default)\n";

std::string next_arg(int& so_far, int argc, char **argv)
{
        if (so_far==argc) {
                std::cerr<<"Ran out of arguments."<<std::endl;
                std::cerr<<usage<<std::endl;
                exit(EXIT_FAILURE);
        }
        return argv[so_far++];
}

CentralisedLog clog;
void app_fatal (void) { abort(); }


int main(int argc, char **argv)
{
        if (argc==1) {
                std::cout<<info<<std::endl;
                std::cout<<usage<<std::endl;
                return EXIT_SUCCESS;
        }

        // default parameters
        int debug_level = 0;
        bool output_binary_specified = false;
        bool output_binary = false;
        std::string input_filename;
        std::string output_filename;
        std::string mat_prefix;

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
                        output_binary_specified = true;
                } else if (arg=="-T" || arg=="--output-tcol") {
                        output_binary = false;
                        output_binary_specified = true;
                } else if (arg=="-m" || arg=="--material-prefix") {
                        mat_prefix = next_arg(so_far,argc,argv);
                } else if (arg=="-h" || arg=="--help") {
                        std::cout<<info<<std::endl;
                        std::cout<<usage<<std::endl;
                } else {
                        std::cerr<<"Unrecognised argument: "<<arg<<std::endl;
                        std::cerr<<usage<<std::endl;
                        return EXIT_FAILURE;
                }
        }

        if (!output_binary_specified) {
                if (output_filename.length()>=5 && output_filename.substr(output_filename.length()-5)==".bcol")
                        output_binary = true;
                else
                        output_binary = false;
        }

        std::istream *in = NULL;
        std::ostream *out = NULL;

        init_global_db();

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
                        if (output_binary) {
                                //write_tcol_as_bcol(*out, tcol);
                        } else {
                                pretty_print_tcol(*out,tcol);
                        }

                } else if (fourcc==0x4c4f4342) { //BCOL
                        IOS_EXCEPT("Reading bcol not implemented.");
                } else if (fourcc==0x4c4c4f43) { // COLL
                                dump_all_cols(*in,output_binary,mat_prefix,global_db,debug_level);
                } else if (fourcc==0x324c4f43) { // COL2
                                dump_all_cols(*in,output_binary,mat_prefix,global_db,debug_level);
                } else if (fourcc==0x334c4f43) { // COL3
                                dump_all_cols(*in,output_binary,mat_prefix,global_db,debug_level);
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
