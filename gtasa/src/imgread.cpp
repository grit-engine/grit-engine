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

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <locale>

#include "ios_util.h"
#include "imgread.h"

#define BLK_SZ 2048

static int tolowr (int c)
{
        return std::tolower(char(c),std::cout.getloc());
}

static std::string& strlower (std::string& s)
{
        std::transform(s.begin(),s.end(), s.begin(),tolowr);
        return s;
}


void Img::init (std::istream &f, std::string name_)
{
        name = name_;
        unsigned long version = ios_read_u32(f);
        APP_ASSERT(version==0x32524556);

        numFiles = ios_read_u32(f);
        names.resize(numFiles);
        offsets.resize(numFiles);
        sizes.resize(numFiles);

        for (size_t i=0 ; i<numFiles ; i++) {
                offsets[i] = ios_read_u32(f) * BLK_SZ;
                sizes[i]= ios_read_u32(f) * BLK_SZ;
                names[i] = ios_read_fixedstr(f,24);
                strlower(names[i]);
        }

        if (dir.size()==numFiles) return;
        for (unsigned long i=0 ; i<numFiles ; ++i) {
                dir[names[i]] = i;
        }
}

const std::string &Img::fileName (unsigned long i) const
{
        return names[i];
}

bool Img::fileExists (const std::string &fname) const
{
        return dir.find(fname) != dir.end();
}

void Img::fileOffset (std::istream &f, unsigned long i) const
{
        f.seekg(fileOffset(i)); 
        APP_ASSERT_IO_SUCCESSFUL(f,"seeking to: "+fileName(i));
}

Img::Dir::const_iterator Img::find (const std::string &fname) const
{
        Dir::const_iterator iter = dir.find(fname);
        if (iter==dir.end())
                GRIT_EXCEPT(name+" did not contain: \""+fname+"\"");
        return iter;
}

void Img::fileOffset (std::istream &f, const std::string &fname) const
{
        Dir::const_iterator iter = find(fname);
        fileOffset(f, iter->second);
}                               

unsigned long Img::fileOffset (unsigned long i) const
{
        return offsets[i];
}

unsigned long Img::fileOffset (const std::string &fname) const
{
        Dir::const_iterator iter = find(fname);
        return fileOffset(iter->second);
}                               

unsigned long Img::fileSize (unsigned long i) const
{
        return sizes[i];
}

unsigned long Img::fileSize (const std::string &fname) const
{
        Dir::const_iterator iter = find(fname);
        return fileSize(iter->second);
}                               



#ifdef _IMGREAD_EXEC

size_t amount_read = 0;
size_t amount_seeked = 0;

void assert_triggered (void) { } 

void extract_file (std::ifstream &in,
                   const std::string &name,
                   unsigned long off, unsigned long sz)
{
        char *data = new char[sz];
        in.seekg(off);
        APP_ASSERT_IO_SUCCESSFUL(in,"extracting: "+name);
        in.read(data,sz);
        APP_ASSERT_IO_SUCCESSFUL(in,"extracting: "+name);
        
        std::ofstream out;
        out.open(name.c_str(), std::ios::binary);
        APP_ASSERT_IO_SUCCESSFUL(out,"opening output: "+name);

        out.write(data,sz);
        APP_ASSERT_IO_SUCCESSFUL(out,"writing output: "+name);

        delete data;
}



int main(int argc, char *argv[])
{
        if (argc!=2 && argc!=3) {
                std::cerr<<"To extract a file: "<<std::endl;
                std::cerr<<argv[0]<<" <img> <file>"<<std::endl;
                std::cerr<<"To extract all files: "<<std::endl;
                std::cerr<<argv[0]<<" <img>"<<std::endl;
                return EXIT_FAILURE;
        }
        bool all_files = argc==2;
        std::string extract;
        if (!all_files) extract = argv[2];

        try {

                std::ifstream in;
                in.open(argv[1], std::ios::binary);
                APP_ASSERT_IO_SUCCESSFUL(in,"opening IMG");

                Img img;

                img.init(in, argv[1]);

                if (!all_files) {
                        unsigned long off = img.fileOffset(extract);
                        unsigned long sz = img.fileSize(extract);
                        extract_file(in, extract, off, sz);
                        return EXIT_SUCCESS;
                }

                for (size_t i=0 ; i<img.size() ; i++) {
                        unsigned long off = img.fileOffset(i);
                        unsigned long sz = img.fileSize(i);
                        extract_file(in, img.fileName(i), off, sz);
                }
                return EXIT_SUCCESS;

        } catch (GritException &e) {
                CERR << e << std::endl;

                return EXIT_FAILURE;
        }
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
