#include <string>
#include <map>

// if not called, tex_dup is the identity map (sensible default)
void init_tex_dup_map();

// returns whether a given texture can be substituted for another identical
// texture to avoid duplicates (if not, just returns argument)
std::string tex_dup (const std::string &in);

// vim: shiftwidth=8:tabstop=8:expandtab
