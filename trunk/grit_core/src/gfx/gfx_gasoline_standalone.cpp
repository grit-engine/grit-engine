/* Copyright (c) David Cunningham and the Grit Game Engine project 2013
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

#include <fstream>

#include "gfx_gasoline.h"
#include <exception.h>

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <kind> <filename>" << std::endl;
        return EXIT_FAILURE;
    }
    GfxGslKind kind;
    std::string kind_str(argv[1]);
    if (kind_str == "frag") {
        kind = GFX_GSL_FRAG;
    } else if (kind_str == "vert") {
        kind = GFX_GSL_VERT;
    } else {
        std::cerr << "Unknown shader type (should be frag or vert): " << argv[2] << std::endl;
        return EXIT_FAILURE;
    }
    std::ifstream f;
    f.open(argv[2]);
    if (!f.good()) {
        std::cerr << "Opening file: ";
        perror(argv[2]);
        return EXIT_FAILURE;
    }
    std::string input;
    input.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    try {
        Allocator alloc;
        auto *ast = gfx_gasoline_parse(alloc, input);
        gfx_gasoline_type(alloc, ast, kind);
        std::cout << gfx_gasoline_unparse(ast);
    } catch (const Exception &e) {
        std::cerr << argv[2] << ": " << e << std::endl;
    }
}

// vim: shiftwidth=4:tabstop=4:expandtab
