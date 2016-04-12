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
                         
#ifndef PROCOBJ_H               
#define PROCOBJ_H       

#include <istream>
#include <vector>
#include <string> 
#include <map>  

#include "csvread.h"

struct ProcObj {
    std::string name;

    std::string object_name;
    float spacing, mindist, minrot, maxrot, minscl, maxscl, minsclz, maxsclz, zoffmin, zoffmax;
    bool align;
    bool use_grid;
};

struct ProcObjData {    
    std::vector<ProcObj> procobjs;

    // the map points into the vehicles vector above, so don't resize the above vector
    typedef std::map<std::string,std::vector<ProcObj*> > ProcObjMap;
    typedef ProcObjMap::iterator iterator;
    ProcObjMap map;

    std::vector<ProcObj *> &operator [] (const std::string &i) { return map[i]; }
                        
    iterator begin() { return map.begin(); }
    iterator end() { return map.end(); }
    iterator find(const std::string &i) { return map.find(i); }

};              

// the csv should be already read in
void read_procobj (Csv &csv, ProcObjData &data);
                        
#endif                  

