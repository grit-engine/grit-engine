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

#include "CentralisedLog.h"
#include "path_util.h"
#include "lua_util.h"

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

std::vector<std::string> pwd;

void pwd_push_dir (const std::string &dir)
{
    //CVERB << "PUSH: " << dir << std::endl;
    APP_ASSERT(dir[dir.size()-1] == '/');
    pwd.push_back(dir);
}

std::string grit_dirname (const std::string &absolute)
{
    APP_ASSERT(absolute[0] == '/');
    std::string dir(absolute, 0, absolute.rfind('/')+1);
    return dir;
}

void pwd_push_file (const std::string &filename)
{
    // push everytihng up to and including the last /
    APP_ASSERT(filename[0] == '/');
    std::string dir(filename, 0, filename.rfind('/')+1);
    pwd_push_dir(dir);
}

std::string pwd_pop (void) { std::string r = pwd[pwd.size()-1]; pwd.pop_back(); return r; }

std::string pwd_get (void) { return pwd.size()==0 ? "/" : pwd[pwd.size()-1]; }

std::string pwd_full_ex (lua_State *L, std::string rel, const std::string &path)
{
    if (rel[0] != '/') {
        rel = path + rel;
    }
    // process '..'
    bool err = false;
    std::string r = path_collapse(rel, err);
    if (err)
        my_lua_error(L, "Path tries to escape game dir: \""+rel+"\"");
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

std::string pwd_full (lua_State *L, const std::string &rel)
{
    return pwd_full_ex(L, rel, pwd_get());
}

std::string pwd_full (const std::string &rel, const std::string &def)
{
    return pwd_full_ex(rel, pwd_get(), def);
}

// vim: ts=4:sw=4:et
