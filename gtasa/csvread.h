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
                        
#ifndef CSVREAD_H               
#define CSVREAD_H       

#include <istream>
#include <vector>
#include <string>
#include <map>

struct CsvLine {
    std::vector<std::string> vals;
    int orig_line;
    int section_line;
    const std::string &operator [] (size_t i) const { return vals[i]; }
    std::string &operator [] (size_t i) { return vals[i]; }
    size_t size (void) const { return vals.size(); }
    void push_back (const std::string &val) { return vals.push_back(val); }
};

struct CsvSection {
    std::vector<CsvLine> lines;
    std::string section_name;
    const CsvLine &operator [] (size_t i) const { return lines[i]; }
    CsvLine &operator [] (size_t i) { return lines[i]; }
    size_t size (void) const { return lines.size(); }
    void push_back (const CsvLine &line) { return lines.push_back(line); }
};


struct Csv {
    typedef std::map<std::string,CsvSection> SectionMap;
    typedef SectionMap::iterator iterator;
    SectionMap sections;

    CsvSection &operator [] (const std::string &i) { return sections[i]; }

    iterator begin() { return sections.begin(); }
    iterator end() { return sections.end(); }
    iterator find(const std::string &i) { return sections.find(i); }

    std::string filename;
};

void read_csv (std::istream &f, Csv &csv);

#endif
