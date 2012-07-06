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

#include <cmath>
#include <cerrno>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <locale>

#include "iplread.h"
#include "ios_util.h"
#include "csvread.h"

#include "console_colour.h"

static int tolowr (int c)
{
        return std::tolower(char(c),std::cout.getloc());
}

static std::string& strlower (std::string& s)
{
        std::transform(s.begin(),s.end(), s.begin(),tolowr);
        return s;
}

static std::string& str_lcase_crop (std::string& str)
{
        strlower(str);
        std::string::size_type b=str.find_first_not_of(' ');
        std::string::size_type e=str.find_last_not_of(' ');
        str = str.substr(b,e+1);
        return str;
}



void IPL::addBinary (std::istream &f)
{
        unsigned long fourcc = ios_read_u32(f);
        APP_ASSERT(fourcc==0x79726e62); // "bnry"
        unsigned long num_insts = ios_read_u32(f);
        unsigned long num_unk1 = ios_read_u32(f);
        unsigned long num_unk2 = ios_read_u32(f);
        unsigned long num_unk3 = ios_read_u32(f);
        unsigned long num_vehicles = ios_read_u32(f);
        unsigned long num_unk4 = ios_read_u32(f);

        unsigned long inst_offset = ios_read_u32(f);
        unsigned long zero = ios_read_u32(f);
        APP_ASSERT(zero==0);
        unsigned long unk1_offset = ios_read_u32(f);
        zero = ios_read_u32(f);
        APP_ASSERT(zero==0);
        unsigned long unk2_offset = ios_read_u32(f);
        zero = ios_read_u32(f);
        APP_ASSERT(zero==0);
        unsigned long unk3_offset = ios_read_u32(f);
        zero = ios_read_u32(f);
        APP_ASSERT(zero==0);
        unsigned long vehicles_offset = ios_read_u32(f);
        zero = ios_read_u32(f);
        APP_ASSERT(zero==0);
        unsigned long unk4_offset = ios_read_u32(f);
        zero = ios_read_u32(f);
        APP_ASSERT(zero==0);

        APP_ASSERT(inst_offset==76);

        (void) num_unk1;
        (void) num_unk2;
        (void) num_unk3;
        (void) num_unk4;
        (void) num_vehicles;

        (void) unk1_offset;
        (void) unk2_offset;
        (void) unk3_offset;
        (void) unk4_offset;
        (void) vehicles_offset;

        for (unsigned long i=0 ; i<num_insts ; i++) {
                Inst inst;
                inst.is_low_detail = false;
                inst.dff = "";
                inst.x = ios_read_float(f);
                inst.y = ios_read_float(f);
                inst.z = ios_read_float(f);
                inst.rx = ios_read_float(f);
                inst.ry = ios_read_float(f);
                inst.rz = ios_read_float(f);
                inst.rw = -ios_read_float(f);
                inst.id = ios_read_u32(f);
                inst.interior = ios_read_u32(f);
                inst.near_for = ios_read_u32(f);
                //APP_ASSERT(inst.near_for==0);
                insts.push_back(inst);
        }

        for (size_t i=0 ; i<insts.size() ; i++) {
                Inst& inst = insts[i];
                if (inst.near_for==-1) continue;
                if (inst.near_for<0 || (size_t)inst.near_for>=insts.size()) {
                        std::cerr<<"Invalid lod index at instance "<<i
                                 <<": "<<inst.near_for<<"\n";
                        continue;
                }
                insts[inst.near_for].is_low_detail = true;
        }
}


void IPL::addText (std::istream &f)
{

        Csv csv;
        csv.filename = name;
        read_csv(f,csv);

        for (Csv::iterator i=csv.begin(), i_=csv.end() ; i!=i_ ; ++i) {
                const std::string section = i->first;

                const CsvSection &lines = i->second;

                for (unsigned j=0 ; j<lines.size() ; ++j) {

                        CsvLine line = lines[j];

                        if (section=="inst" && line.size()==11) {
                                Inst inst;
                                inst.is_low_detail = false;
                                double id = strtod(line[0].c_str(),NULL);
                                if (id!=floor(id)) {
                                        std::cerr<<"Id not an integer "<<id<<"\n";
                                        exit(EXIT_FAILURE);
                                }
                                inst.id = (long unsigned int) id;
                                inst.dff = str_lcase_crop(line[1]);
                                double interior = strtod(line[2].c_str(),NULL);
                                if (interior!=floor(interior)) {
                                        std::cerr<<"Interior not an integer "
                                                 <<interior<<"\n";
                                        exit(EXIT_FAILURE);
                                }
                                inst.interior = (long unsigned int) interior;
                                inst.x = strtod(line[3].c_str(),NULL);
                                inst.y = strtod(line[4].c_str(),NULL);
                                inst.z = strtod(line[5].c_str(),NULL);
                                inst.rx = strtod(line[6].c_str(),NULL);
                                inst.ry = strtod(line[7].c_str(),NULL);
                                inst.rz = strtod(line[8].c_str(),NULL);
                                inst.rw = -strtod(line[9].c_str(),NULL);
                                double lod = strtod(line[10].c_str(),NULL);
                                if (lod!=floor(lod)) {
                                        std::cerr<<"LOD not an integer "<<lod<<"\n";
                                        exit(EXIT_FAILURE);
                                }
                                inst.near_for = (long unsigned int) lod;
                                insts.push_back(inst);

                        } else if (section=="occl" && line.size()==10) {
                        } else if (section=="occl" && line.size()==7) {
                        } else if (section=="auzo" && line.size()==7) {
                        } else if (section=="auzo" && line.size()==9) {
                        } else if (section=="grge" && line.size()==11) {
                        } else if (section=="tcyc" && line.size()==11) {
                        } else if (section=="pick" && line.size()==4) {
                        } else if (section=="cull" && line.size()==14) {
                        } else if (section=="cull" && line.size()==11) {
                        } else if (section=="enex" && line.size()==18) {
                        } else if (section=="path") {
                        } else if (section=="between sections" && line.size()==0 ) {
                        } else {
                                std::cerr<<"In "<<name<<":"<<line.orig_line<<" "
                                         <<"section "<<section<<", row "<<line.section_line<<", "
                                         <<"did not have the right number of values: "
                                         <<line.size()<<std::endl;

                        }
                }
        }


        for (size_t i=0 ; i<insts.size() ; i++) {
                Inst& inst = insts[i];
                if (inst.near_for==-1) continue;
                if (inst.near_for<0 ||
                    (size_t)inst.near_for >=insts.size()) {
                        std::cerr<<"Invalid lod index at instance "<<i
                                 <<": "<<inst.near_for<<"\n";
                        continue;
                }
                insts[inst.near_for].is_low_detail = true;
        }
}
#if 0
void IPL::addText (std::istream &f)
{
        std::string section = "no section";

        std::vector<std::string> strs;

        while (!f.eof()) {

                std::string str;

                std::getline(f,str);

                size_t len = str.size();
                if (len==0 || str.at(0)=='#') continue;

                if (str.at(len-1)=='\n') str.erase(str.end()-1);
                len = str.size();
                if (str.at(len-1)=='\r') str.erase(str.end()-1);

                if (str=="inst") { section=str; continue; }
                if (str=="cull") { section=str; continue; }
                if (str=="path") { section=str; continue; }
                if (str=="grge") { section=str; continue; }
                if (str=="enex") { section=str; continue; }
                if (str=="pick") { section=str; continue; }
                if (str=="jump") { section=str; continue; }
                if (str=="tcyc") { section=str; continue; }
                if (str=="auzo") { section=str; continue; }
                if (str=="mult") { section=str; continue; }
                if (str=="cars") { section=str; continue; }
                if (str=="occl") { section=str; continue; }
                if (str=="end")  { section="between sections"; continue; }

                std::stringstream ss;
                ss << str;
                strs.clear();
                for (std::string word ; std::getline(ss,word,',') ; ) {
                        strs.push_back(word);
                }


                if (section=="inst" && strs.size()==11) {
                        Inst inst;
                        inst.is_low_detail = false;
                        double id = strtod(strs[0].c_str(),NULL);
                        if (id!=floor(id)) {
                                std::cerr<<"Id not an integer "<<id<<"\n";
                                exit(EXIT_FAILURE);
                        }
                        inst.id = (long unsigned int) id;
                        inst.dff = str_lcase_crop(strs[1]);
                        double interior = strtod(strs[2].c_str(),NULL);
                        if (interior!=floor(interior)) {
                                std::cerr<<"Interior not an integer "
                                         <<interior<<"\n";
                                exit(EXIT_FAILURE);
                        }
                        inst.interior = (long unsigned int) interior;
                        inst.x = strtod(strs[3].c_str(),NULL);
                        inst.y = strtod(strs[4].c_str(),NULL);
                        inst.z = strtod(strs[5].c_str(),NULL);
                        inst.rx = strtod(strs[6].c_str(),NULL);
                        inst.ry = strtod(strs[7].c_str(),NULL);
                        inst.rz = strtod(strs[8].c_str(),NULL);
                        inst.rw = -strtod(strs[9].c_str(),NULL);
                        double lod = strtod(strs[10].c_str(),NULL);
                        if (lod!=floor(lod)) {
                                std::cerr<<"LOD not an integer "<<lod<<"\n";
                                exit(EXIT_FAILURE);
                        }
                        inst.near_for = (long unsigned int) lod;
                        insts.push_back(inst);

                } else if (section=="occl" && strs.size()==10) {
                } else if (section=="occl" && strs.size()==7) {
                } else if (section=="auzo" && strs.size()==7) {
                } else if (section=="auzo" && strs.size()==9) {
                } else if (section=="grge" && strs.size()==11) {
                } else if (section=="tcyc" && strs.size()==11) {
                } else if (section=="pick" && strs.size()==4) {
                } else if (section=="cull" && strs.size()==14) {
                } else if (section=="cull" && strs.size()==11) {
                } else if (section=="enex" && strs.size()==18) {
                } else if (section=="path") {
                } else if (section=="between sections" && strs.size()==0 ) {
                } else {
                        std::cerr<<"In "<<section<<", couldn't understand ["
                                 <<strs.size()<<"] \""<<str<<"\"\n";
                }
        }

        for (size_t i=0 ; i<insts.size() ; i++) {
                Inst& inst = insts[i];
                if (inst.near_for==-1) continue;
                if (inst.near_for<0 ||
                    (size_t)inst.near_for >=insts.size()) {
                        std::cerr<<"Invalid lod index at instance "<<i
                                 <<": "<<inst.near_for<<"\n";
                        continue;
                }
                insts[inst.near_for].is_low_detail = true;
        }
}
#endif
void IPL::addMore(std::istream &f)
{
        unsigned long fourcc = ios_read_u32(f);
        f.seekg(-4,std::ios_base::cur);
        if (fourcc==0x79726e62) { // "bnry"
                addBinary(f);
        } else {
                addText(f);
        }
}

#ifdef _IPLREAD_EXEC

size_t amount_read = 0;
size_t amount_seeked = 0;

void assert_triggered (void) { } 

int main(int argc, char *argv[])
{
        if (argc<2) {
                std::cerr<<"Usage: "<<argv[0]<<" <ipl> {<streams>}"<<std::endl;
                exit(EXIT_FAILURE);
        }

        try {

                IPL ipl;

                for (int i=1 ; i<argc ; ++i) {
                        std::string name = argv[i];

                        std::ifstream f;
                        f.open(name.c_str(),std::ios::binary);
                        APP_ASSERT_IO_SUCCESSFUL(f,"opening IPL: "+name);

                        ipl.setName(name);
                        ipl.addMore(f);
                }

/*
                for (size_t i=0 ; i<insts.size() ; i++) {
                        std::cout <<RESET<<argv[1]<<":"<<i<<" "
                                  <<insts[i].id<<" "
                                  <<"\""<<insts[i].dff<<"\" "
                                  <<BOLD<<GREEN<<insts[i].interior<<" "
                                  <<NOBOLD<<WHITE<<insts[i].x<<" "
                                  <<WHITE<<insts[i].y<<" "
                                  <<WHITE<<insts[i].z<<" "
                                  <<BLUE<<insts[i].rx<<" "
                                  <<BLUE<<insts[i].ry<<" "
                                  <<BLUE<<insts[i].rz<<" "
                                  <<BLUE<<insts[i].rw<<" "
                                  <<BOLD<<RED<<insts[i].near_for<<" "
                                  <<NOBOLD<<RED<<insts[i].is_low_detail<<std::endl;
                }
*/

                const Insts &insts = ipl.getInsts();

                for (size_t i=0 ; i<insts.size() ; i++) {
                        std::cout <<argv[1]<<":"<<i<<" "
                                  <<"id:"<<insts[i].id<<" "
                                  <<"\""<<insts[i].dff<<"\" "
                                  <<insts[i].interior<<" "
                                  <<insts[i].x<<" "
                                  <<insts[i].y<<" "
                                  <<insts[i].z<<" "
                                  <<insts[i].rx<<" "
                                  <<insts[i].ry<<" "
                                  <<insts[i].rz<<" "
                                  <<insts[i].rw<<" "
                                  <<insts[i].near_for<<" "
                                  <<(insts[i].is_low_detail?"near":"no_near")
                                  <<std::endl;
                }

        } catch (GritException &e) {
                CERR<<e<<std::endl;
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
