#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <errno.h>

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
                                IOS_EXCEPT(std::string(
                                    "root dir doesn't exist(?), "
                                    "making: \""+path+"\""));
                        }
                } else if (GetLastError()!=ERROR_ALREADY_EXISTS) {
                        IOS_EXCEPT(std::string("error while making dir: "
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
                IOS_EXCEPT(std::string(strerror(errno))+
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
