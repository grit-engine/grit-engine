/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

#include "PhysicalMaterial.h"
#include "../path_util.h"

PhysicalMaterialDB phys_mats; 

PhysicalMaterial *PhysicalMaterialDB::getMaterial (const std::string &mat_name)
{
    NameMap::const_iterator i = mdb.find(mat_name);
    if (i==mdb.end()) {
        CERR << "Physical material does not exist: \""<<mat_name<<"\"" << std::endl;
        return mdb[emergencyMat()];
    }
    return i->second;
}

PhysicalMaterial *PhysicalMaterialDB::getMaterial (const std::string &col_name,
                                                   const std::string &mat_name)
{
    NameMap::const_iterator i = mdb.find(mat_name);
    if (i==mdb.end()) {
        CERR << "Collision mesh \""<<col_name<<"\" references "
             << "non-existent physical material \""<<mat_name<<"\"" << std::endl;
        return mdb[emergencyMat()];
    }
    return i->second;
}

PhysicalMaterial *PhysicalMaterialDB::getMaterial (const std::string &dir,
                                                   const std::string &col_name,
                                                   const std::string &mat_name)
{
    // can fail in this function because of e.g. "/../Blah"
    const char * emergency = PhysicalMaterialDB::emergencyMat();
    std::string absolute_mat_name = pwd_full_ex(mat_name,dir,emergency);
    // can also fail here if that material does not exist
    return  getMaterial(col_name, absolute_mat_name);
}

PhysicalMaterial *PhysicalMaterialDB::getMaterialSafe (int mat_id)
{
    if ((unsigned)mat_id>=mdb2.size())  {
        CERR << "Got a bad material id: " << mat_id << std::endl;
        return mdb[emergencyMat()];
    }
    return getMaterial(mat_id);
}

void PhysicalMaterialDB::setMaterial (const std::string &name, int interaction_group)
{
    if (mdb.find(name) == mdb.end()) {
        PhysicalMaterial *m = new PhysicalMaterial();
        mdb[name] = m;
        m->name = name;
        m->interactionGroup = interaction_group;
        m->id = mdb2.size();
        mdb2.push_back(m);
    } else {
        PhysicalMaterial *m = mdb[name];
        m->interactionGroup = interaction_group;
    }
}

void PhysicalMaterialDB::setInteractionGroups (unsigned groups,
                                               const Interactions &interactions_)
{
    APP_ASSERT(groups * groups == interactions_.size());
    numInteractions = groups;
    interactions = interactions_;
}

