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
                         
#ifndef SURFINFO_H               
#define SURFINFO_H       

#include <istream>
#include <vector>
#include <string> 
#include <map>  

#include "csvread.h"

struct SurfInfo {
    std::string name;

    std::string adhesion_group;
    float tyre_grip;
    float wet_grit;
    std::string skidmark;
    std::string friction_effect;
    bool soft_landing;
    bool see_through;
    bool shoot_through;
    bool sand;
    bool water;
    bool shallow_water;
    bool beach;
    bool steep_slope; // can't climb
    bool glass; // shatters
    bool stairs; // hack to stop camera tilt
    bool skateable;
    bool pavement;
    int roughness; // pad vibration
    int flammability;
    bool sparks; // not used
    bool sprint;
    bool footsteps; // leave footprints
    bool footdust;
    bool cardirt;
    bool carclean;
    bool w_grass;
    bool w_gravel;
    bool w_mud;
    bool w_dust;
    bool w_sand;
    bool w_spray;
    bool proc_plant;
    bool proc_obj;
    bool climbable;
    std::string bullet_fx;
};

struct SurfInfoData {    
    std::vector<SurfInfo> surfaces;

    // the map points into the vehicles vector above, so don't resize the above vector
    typedef std::map<std::string,SurfInfo*> SurfInfoMap;
    typedef SurfInfoMap::iterator iterator;
    SurfInfoMap map;

    SurfInfo *&operator [] (const std::string &i) { return map[i]; }
                        
    iterator begin() { return map.begin(); }
    iterator end() { return map.end(); }
    iterator find(const std::string &i) { return map.find(i); }

};              

// the csv should be already read in
void read_surfinfo (Csv &csv, SurfInfoData &data);
                        
#endif                  

