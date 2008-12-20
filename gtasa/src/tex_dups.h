#include <string>
#include <map>

// stores whether a given texture can be substituted for another identical
// texture to avoid duplicates
typedef std::map<std::string,std::string> TexDupMap;

extern TexDupMap tex_dup_map;

void init_tex_dup_map();

// vim: shiftwidth=8:tabstop=8:expandtab
