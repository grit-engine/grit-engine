/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

struct PhysicalMaterial;
class PhysicalMaterialDB;

#ifndef PhysicalMaterial_h
#define PhysicalMaterial_h

#include <vector>
#include<string>
#include "../centralised_log.h"


struct PhysicalMaterial {
    std::string name;
    int interactionGroup;
    int id;
};

struct Interaction {
    float friction;
    float restitution;
    Interaction() : friction(0), restitution(0) { }
    Interaction(float f, float r) : friction(f), restitution(r) { }
};

typedef std::vector<Interaction> Interactions;

class PhysicalMaterialDB {

    public:

    static const char *emergencyMat (void) { return "/system/FallbackPhysicalMaterial"; }

    PhysicalMaterialDB (void)
    {
        Interactions interactions;
        interactions.push_back(Interaction(0.5,0.5));
        setInteractionGroups(1, interactions);
        setMaterial(emergencyMat(), 0);
    }

    PhysicalMaterial *getMaterial (const std::string &mat_name);

    PhysicalMaterial *getMaterial (const std::string &col_name, const std::string &mat_name);

    PhysicalMaterial *getMaterial (const std::string &dir,
                                   const std::string &col_name,
                                   const std::string &mat_name);


    PhysicalMaterial *getMaterial (int material)
    { return mdb2[material]; }

    PhysicalMaterial *getMaterialSafe (int material);


    void setMaterial (const std::string &name, int interaction_group);

    void setInteractionGroups (unsigned groups, const Interactions &interactions);

    void getFrictionRestitution (int mat0, int mat1, float &f, float &r)
    {
        unsigned char ig0 = getMaterial(mat0)->interactionGroup;
        unsigned char ig1 = getMaterial(mat1)->interactionGroup;
        if (ig0 > numInteractions) {
            CERR<<"Invalid interaction group "<<ig0<<" in material \""
                <<getMaterial(mat0)->name<<"\""<<std::endl;
            f = 0; r = 0; return;
        }
        if (ig1 > numInteractions) {
            CERR<<"Invalid interaction group "<<ig1<<" in material \""
                <<getMaterial(mat1)->name<<"\""<<std::endl;
            f = 0; r = 0; return;
        }
        int code = ig0*numInteractions + ig1;
        f = interactions[code].friction;
        r = interactions[code].restitution ;
    }

    protected:

    typedef std::map<std::string, PhysicalMaterial*> NameMap; // map string to material
    NameMap mdb;
    
    typedef std::vector<PhysicalMaterial*> IndexMap; // map id to material
    IndexMap mdb2;
        
    Interactions interactions; // size() == numInteractions*numInteractions
    unsigned numInteractions;
};

extern PhysicalMaterialDB phys_mats; 

#endif
