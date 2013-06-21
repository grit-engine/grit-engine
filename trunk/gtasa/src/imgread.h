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
#include <map>
#include <fstream>

class Img {

    public:

        void init (std::istream &f, std::string name_="IMG file");

        virtual const std::string &fileName (unsigned long i) const;

        virtual bool fileExists (const std::string &fname) const;

        virtual void fileOffset (std::istream &f, unsigned long i) const;

        virtual void fileOffset (std::istream &f,const std::string &fname)const;

        virtual unsigned long fileOffset (unsigned long i) const;

        virtual unsigned long fileOffset (const std::string &fname)const;

        virtual unsigned long fileSize (unsigned long i) const;

        virtual unsigned long fileSize (const std::string &fname) const;

        virtual unsigned long size (void) const { return numFiles; }


    protected:

        std::string name;

        unsigned long numFiles;

        std::vector<std::string> names;

        std::vector<unsigned long> offsets;

        std::vector<unsigned long> sizes;

        typedef std::map<std::string,unsigned long> Dir;
        Dir dir;

        Dir::const_iterator find (const std::string &fname) const;

};

// vim: shiftwidth=8:tabstop=8:expandtab
