#include <string>
#include <vector>

#include <OgreDataStream.h>

struct mat_bin;

std::vector<struct mat_bin> read_matbins (const Ogre::DataStreamPtr &f);

#ifndef matbin_h

#define matbin_h

struct mat_bin {
        unsigned char flags;
        std::string name;
        std::string tname;
        unsigned long colour;
};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
