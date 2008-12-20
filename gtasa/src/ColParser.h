#include <iostream>
#include "TColParser.h"

bool parse_col (std::string &name,
                std::istream &in,
                TColFile &tcol,
                int debug_level=0);

void dump_all_cols (std::istream &in, bool binary, int debug_level=0);

#ifndef ColParser_h
#define ColParser_h

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
