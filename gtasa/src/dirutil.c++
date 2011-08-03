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

#include <cerrno>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "dirutil.h"
#include "ios_util.h"

// 
#ifdef WIN32
void ensuredir (const std::string &path)
{
        // winapi version
        const char *cpath = path.c_str();
        if (!CreateDirectory(cpath,NULL)) {
                if (GetLastError()==ERROR_PATH_NOT_FOUND) {
                        std::string::size_type slashpos = path.find_last_of("/");
                        if (slashpos<path.size()) {
                                std::string prefix = path.substr(0,slashpos);
                                ensuredir(prefix);
                                ensuredir(path); // hopefully this won't loop horribly
                        } else {
                                GRIT_EXCEPT(std::string(
                                    "root dir doesn't exist(?), "
                                    "making: \""+path+"\""));
                        }
                } else if (GetLastError()!=ERROR_ALREADY_EXISTS) {
                        GRIT_EXCEPT(std::string("error while making dir: "
                                               "\""+path+"\""));
                }
        }
}
#else
void ensuredir (const std::string &path)
{
        // posix version
        struct stat stat_results;
        int err = stat(path.c_str(),&stat_results);
        if (!err) return; // dir exists already
        if (err && errno!=ENOENT && errno!=ENOTDIR)
                GRIT_EXCEPT(std::string(strerror(errno))+
                           "while making dir: \""+path+"\"");

        // dir doesn't exist, check prefix exists
        std::string::size_type slashpos = path.find_last_of("/");
        if (slashpos<path.size()) {
                std::string prefix = path.substr(0,slashpos);
                ensuredir(prefix);
        }

        // finally make dir with 755 permissions
        mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}
#endif
                

// vim: shiftwidth=8:tabstop=8:expandtab
