/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include "centralised_log.h"
#include "path_util.h"
#include "grit_lua_util.h"

// handle .. in paths
static std::string path_collapse (const std::string &p, bool &err)
{
    APP_ASSERT(p[0] == '/');
    std::vector<std::string> new_path;
    size_t last = 0;
    //CVERB<<"Starting with: "<<p<<std::endl;
    do {
        size_t next = p.find('/', last+1);
        std::string dir(p, last+1, next-last-1);
        //CVERB<<"Piece: "<<last<<" "<<next<<" "<<dir<<std::endl;
        if (dir == "..") {
            if (new_path.size()==0) {
                err = true;
                return "";
            }
            new_path.pop_back();
        } else {
            new_path.push_back(dir);
        }
        if (next == std::string::npos) break;
        last = next;
    } while (true);
    std::string r;
    for (size_t i=0 ; i<new_path.size(); ++i) {
        r.append("/");
        r.append(new_path[i]);
    }
    return r;
}

std::string grit_dirname (const std::string &absolute)
{
    APP_ASSERT(absolute[0] == '/');
    std::string dir(absolute, 0, absolute.rfind('/')+1);
    return dir;
}

std::string pwd_full_ex (std::string rel, const std::string &path)
{
    if (rel[0] != '/') {
        rel = path + rel;
    }
    // process '..'
    bool err = false;
    std::string r = path_collapse(rel, err);
    if (err)
        EXCEPT << "Path tries to escape game dir: \"" << rel << "\"" << ENDL;
    return r;
}

std::string pwd_full_ex (std::string rel, const std::string &path, const std::string &def)
{
    if (rel[0] != '/') {
        rel = path + rel;
    }
    // process '..'
    bool err = false;
    std::string r = path_collapse(rel, err);
    if (err) {
        CERR <<  "Path tries to escape game dir: \"" << rel << "\"" << std::endl;
        r = def;
    }
    return r;
}

// vim: ts=4:sw=4:et
