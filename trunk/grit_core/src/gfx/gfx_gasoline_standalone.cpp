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
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <vert.gsl> <frag.gsl> <targetlang> <vert.out> <frag.out>" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream f;

    f.open(argv[1]);
    if (!f.good()) {
        std::cerr << "Opening file: ";
        perror(argv[1]);
        return EXIT_FAILURE;
    }
    std::string vert_code;
    vert_code.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    f.close();

    f.open(argv[2]);
    if (!f.good()) {
        std::cerr << "Opening file: ";
        perror(argv[2]);
        return EXIT_FAILURE;
    }
    std::string frag_code;
    frag_code.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    f.close();

    GfxGslBackend backend;
    std::string language = argv[3];
    if (language == "gsl") {
        backend = GFX_GSL_BACKEND_GSL;
    } else if (language == "cg") {
        backend = GFX_GSL_BACKEND_CG;
    } else if (language == "glsl") {
        backend = GFX_GSL_BACKEND_GLSL;
    } else {
        std::cerr << "Unrecognised shader target language: " << language << std::endl;
        return EXIT_FAILURE;
    }


    GfxGslParams params;
    params["starfieldMap"] = GFX_GSL_FLOAT_TEXTURE2;
    params["starfieldMask"] = GFX_GSL_FLOAT3;
    params["perlin"] = GFX_GSL_FLOAT_TEXTURE2;
    params["perlinN"] = GFX_GSL_FLOAT_TEXTURE2;
    params["emissiveMap"] = GFX_GSL_FLOAT_TEXTURE2;
    params["emissiveMask"] = GFX_GSL_FLOAT3;
    params["alphaMask"] = GFX_GSL_FLOAT1;
    params["alphaRejectThreshold"] = GFX_GSL_FLOAT1;

    try {
        std::pair<std::string, std::string> shaders;
        GfxGslUnboundTextures ubt;
        ubt["perlinN"] = GfxGslColour(0.5, 0.5, 0.5, 0);
        shaders = gfx_gasoline_compile(backend, vert_code, frag_code, params, ubt);

        std::ofstream of;
        of.open(argv[4]);
        if (!of.good()) {
            std::cerr << "Opening file: ";
            perror(argv[4]);
            return EXIT_FAILURE;
        }
        of << shaders.first;
        of.close();
        
        of.open(argv[5]);
        if (!of.good()) {
            std::cerr << "Opening file: ";
            perror(argv[5]);
            return EXIT_FAILURE;
        }
        of << shaders.second;
        of.close();
        return EXIT_SUCCESS;
    } catch (const Exception &e) {
        std::cerr << e << std::endl;
        return EXIT_FAILURE;
    }


}

// vim: shiftwidth=4:tabstop=4:expandtab
