#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "dirutil.h"
#include "ios_util.h"

void ensuredir (const std::string &path)
{
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
                

// vim: shiftwidth=8:tabstop=8:expandtab
