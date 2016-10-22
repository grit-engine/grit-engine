/* Copyright (c) The Grit Game Engine authors 2016
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
#include <cerrno>
#include <cstring>

#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <portable_io.h>
#include "tcol_parser.h"
#include "bcol_parser.h"

#define VERSION "1.0"

const char *info =
"grit_col_conv (c) Dave Cunningham 2011  (version: " VERSION ")\n"
"I convert between tcol and bcol files.  The process is not lossless:\n"
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

void app_fatal (void) { abort(); }

CentralisedLog clog;

int main (int argc, char **argv)
{
    if (argc==1) {
        std::cout<<info<<std::endl;
        std::cout<<usage<<std::endl;
        return EXIT_SUCCESS;
    }

    // default parameters
    int debug_level = 0;
    bool output_binary_specified = false;
    bool output_binary = false; // always overwritten
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
        if (output_filename.length()>=5) {
            std::string ext = output_filename.substr(output_filename.length()-5);
            output_binary = ext==".bcol" || ext==".gcol";
        } else {
            output_binary = false;
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
                APP_ASSERT_IO_SUCCESSFUL(*in_,
                                     "opening "+input_filename);
        }

        unsigned long fourcc = ios_peek_u32(*in);
        APP_ASSERT_IO_SUCCESSFUL(*in,"reading fourcc");

        if (output_filename==""||output_filename=="-") {
            out = &std::cout;
        } else {
            std::ofstream *out_=new std::ofstream();
            out = out_;
            out_->open(output_filename.c_str(),
                       std::ios::binary);
            APP_ASSERT_IO_SUCCESSFUL(*out_,
                                "opening "+output_filename);
        }

        if (fourcc==0x4c4f4354) { //TCOL

            TColFile tcol;

            quex::tcol_lexer* qlex =
                    new quex::tcol_lexer(in);
            parse_tcol_1_0(input_filename,qlex,tcol);
            delete qlex;

            if (output_binary) {
                    write_tcol_as_bcol(*out, tcol);
            } else {
                    pretty_print_tcol(*out,tcol);
            }

        } else if (fourcc==0x4c4f4342) { //BCOL

            if (in == &std::cin)  {
                GRIT_EXCEPT("Reading bcol from stdin not implemented (use a file).");
                return EXIT_FAILURE;
            }

            static_cast<std::ifstream*>(in)->close();

            const char *in_name = input_filename.c_str();

            int fdin = open(in_name, O_RDONLY);
            if (fdin < 0) {
                CERR << "cannot open: \""<<in_name<<"\" ("<<strerror(errno)<<")" << std::endl;
                return EXIT_FAILURE;
            }
            /* find size of input file */
            size_t sz;
            {
                struct stat statbuf;
                if (fstat(fdin,&statbuf) < 0) {
                    CERR << "cannot fstat: \""<<in_name<<"\" ("<<strerror(errno)<<")" << std::endl;
                    return EXIT_FAILURE;
                }
                sz = statbuf.st_size;
            }

            BColFile *bcol = static_cast<BColFile*>(mmap(NULL, sz, PROT_READ, MAP_SHARED, fdin, 0));
            if (bcol == MAP_FAILED) {
                CERR << "cannot mmap: \""<<in_name<<"\" ("<<strerror(errno)<<")"<<std::endl;
                return EXIT_FAILURE;
            }

            if (close(fdin)) {
                CERR << "cannot close: \""<<in_name<<"\" ("<<strerror(errno)<<")"<<std::endl;
                return EXIT_FAILURE;
            }

            if (output_binary) {
                GRIT_EXCEPT("Writing a BCol from a BCol not implemented.");
                return EXIT_FAILURE;
            } else {
                write_bcol_as_tcol(*out, *bcol);
            }


            if (munmap(bcol, sz)) {
                CERR << "cannot mummap: \""<<in_name<<"\" ("<<strerror(errno)<<")"<<std::endl;
                return EXIT_FAILURE;
            }

        } else {
                GRIT_EXCEPT("Unrecognised fourcc tag.");
        }

        if (in!=&std::cin) delete in;
        if (out!=&std::cout) delete out;

    } catch (Exception& e) {

        CERR << e << std::endl;

        if (in!=&std::cin) delete in;
        if (out!=&std::cout) delete out;

        return EXIT_FAILURE;

    }






/*

    int fdout = open(out_name, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
    if (fdout < 0) {
        std::cerr << "cannot open: \""<<out_name<<"\" ("<<strerror(errno)<<")" << std::endl;
        return EXIT_FAILURE;
    }

    // go to the location corresponding to the last byte
    if (lseek (fdout, sz - 1, SEEK_SET) == -1)
        std::cerr << "cannot lseek: \""<<out_name<<"\" ("<<strerror(errno)<<")" << std::endl;
        return EXIT_FAILURE;
    }

    // write a dummy byte at the last location
    if (write (fdout, "", 1) != 1)
    err_sys ("write error");

    char *dst;
    if ((dst = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) == (caddr_t) -1)
    err_sys ("mmap error for output");


    // this copies the input file to the output file
    memcpy (dst, src, statbuf.st_size);

*/

    return EXIT_SUCCESS;
}

// vim: shiftwidth=4:tabstop=4:expandtab

