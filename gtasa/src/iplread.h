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

#ifndef IPLREAD_H
#define IPLREAD_H

#include <string>
#include <vector>

struct Inst {
        unsigned long id;
        std::string dff;
        unsigned long interior;
        double x,y,z, rx,ry,rz,rw;
        long near_for;
        bool is_low_detail;
};

typedef std::vector<struct Inst> Insts;

class IPL {
    public:
        virtual void addMore (std::istream &f);
        virtual const Insts &getInsts (void) const { return insts; }
        virtual const std::string &getName (void) const { return name; }
        virtual void setName (const std::string &v) { name = v; }
    protected:
        virtual void addText (std::istream &f);
        virtual void addBinary (std::istream &f);
        Insts insts;
        std::string name;
};

#endif


// vim: shiftwidth=8:tabstop=8:expandtab
