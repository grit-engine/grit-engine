/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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
#include <string>
#include <vector>

#include "gfx_gasoline.h"

#include <exception.h>

const char *info =
    "Gasoline (c) Dave Cunningham 2015  (version: 0.1)\n"
    "A command-line compiler for the Grit Shading Language.\n"
;

const char *usage =
    "Usage: gsl { <opt> } <vert.gsl> <dangs.gsl> <add.gsl> <kind> <vert.out> <frag.out>\n\n"
    "where <opt> ::= -h | --help                     This message\n"
    "              | -C | --cg                       Target CG\n"
    "              | -p | --param <var> <type>       Declare a parameter\n"
    "              | -u | --unbind <tex>             Unbind a texture (will be all 1s)\n"
    "              | -d | --alpha_dither             Enable dithering via alpha texture\n"
    "              | -i | --instanced               Enable instanced\n"
    "              | -e | --env1                     One env box\n"
    "              | -E | --env2                     Two env boxes\n"
    "              | -b | --bones <n>                Number of blended bones\n"
    "              | --                              End options passing\n"
;


std::string next_arg(int& so_far, int argc, char **argv)
{
        if (so_far==argc) {
                std::cerr<<"ERROR: Not enough commandline arguments...\n"<<std::endl;
                std::cerr<<usage<<std::endl;
                exit(EXIT_FAILURE);
        }
        return argv[so_far++];
}

enum FileOrSnippet { F, S };

static GfxGslParam from_string (const std::string &type_name)
{
    if (type_name == "Float") return GfxGslParam::float1(0);
    if (type_name == "Float2") return GfxGslParam::float2(0, 0);;
    if (type_name == "Float3") return GfxGslParam::float3(0, 0, 0);;
    if (type_name == "Float4") return GfxGslParam::float4(0, 0, 0, 0);
    if (type_name == "Int") return GfxGslParam::int1(0);
    if (type_name == "Int2") return GfxGslParam::int2(0, 0);
    if (type_name == "Int3") return GfxGslParam::int3(0, 0, 0);
    if (type_name == "Int4") return GfxGslParam::int4(0, 0, 0, 0);
    if (type_name == "FloatTexture1") return GfxGslParam(GFX_GSL_FLOAT_TEXTURE1, 0,0,0,0);
    if (type_name == "FloatTexture2") return GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 0,0,0,0);
    if (type_name == "FloatTexture3") return GfxGslParam(GFX_GSL_FLOAT_TEXTURE3, 0,0,0,0);
    if (type_name == "FloatTexture4") return GfxGslParam(GFX_GSL_FLOAT_TEXTURE4, 0,0,0,0);
    if (type_name == "FloatTextureCube") return GfxGslParam(GFX_GSL_FLOAT_TEXTURE_CUBE, 0,0,0,0);
    EXCEPT << "Not a Gasoline type: " << type_name << ENDL;
}


int main (int argc, char **argv)
{

    try {
        int so_far = 1;
        bool no_more_switches = false;
        bool instanced = false;
        bool alpha_dither = false;
        unsigned env_boxes = 0;
        unsigned bones = 0;
        std::vector<std::string> args;
        std::string language = "GLSL";
        GfxGslRunParams params;
        GfxGslUnboundTextures ubt;
        while (so_far < argc) {
            std::string arg = next_arg(so_far, argc, argv);
            if (no_more_switches) {
                args.push_back(arg);
            } else if (arg=="-h" || arg=="--help") {
                std::cout<<info<<std::endl;
                std::cout<<usage<<std::endl;
                exit(EXIT_SUCCESS);
            } else if (arg=="--") {
                no_more_switches = true;
            } else if (arg=="-C" || arg=="--cg") {
                language = "CG";
            } else if (arg=="-p" || arg=="--param") {
                std::string name = next_arg(so_far, argc, argv);
                std::string type_name = next_arg(so_far, argc, argv);
                params[name] = from_string(type_name);
            } else if (arg=="-u" || arg=="--unbind") {
                std::string name = next_arg(so_far, argc, argv);
                ubt.insert(name);
            } else if (arg=="-i" || arg=="--instanced") {
                instanced = true;
            } else if (arg=="-d" || arg=="--alpha_dither") {
                alpha_dither = true;
            } else if (arg=="-e" || arg=="--env1") {
                env_boxes = 1;
            } else if (arg=="-E" || arg=="--env2") {
                env_boxes = 2;
            } else if (arg=="-b" || arg=="--bones") {
                std::string num_str = next_arg(so_far, argc, argv);
                char **nptr = nullptr;
                long long num = strtoll(num_str.c_str(), nptr, 10);
                if (num < 0 || num > 4) {
                    std::cerr<<"Number of bones must be in [0,4] range." << std::endl;
                    return EXIT_FAILURE;
                }
                bones = unsigned(num);
            } else {
                args.push_back(arg);
            }
        }

        if (args.size() != 6) {
            std::cerr << info << std::endl;
            std::cerr << usage << std::endl;
            return EXIT_FAILURE;
        }

        const std::string vert_in_filename = args[0];
        const std::string dangs_in_filename = args[1];
        const std::string additional_in_filename = args[2];
        const std::string kind = args[3];
        const std::string vert_out_filename = args[4];
        const std::string frag_out_filename = args[5];

        std::ifstream f;

        std::string vert_code;
        if (vert_in_filename != "")  {
            f.open(vert_in_filename);
            if (!f.good()) {
                std::cerr << "Opening file: ";
                perror(vert_in_filename.c_str());
                return EXIT_FAILURE;
            }
            vert_code.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
            f.close();
        }

        std::string dangs_code;
        if (dangs_in_filename != "") {
            f.open(dangs_in_filename);
            if (!f.good()) {
                std::cerr << "Opening file: ";
                perror(dangs_in_filename.c_str());
                return EXIT_FAILURE;
            }
            dangs_code.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
            f.close();
        }

        f.open(additional_in_filename);
        if (!f.good()) {
            std::cerr << "Opening file: ";
            perror(additional_in_filename.c_str());
            return EXIT_FAILURE;
        }
        std::string additional_code;
        additional_code.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        f.close();

        GfxGslBackend backend;
        if (language == "CG") {
            backend = GFX_GSL_BACKEND_CG;
        } else if (language == "GLSL") {
            backend = GFX_GSL_BACKEND_GLSL;
        } else {
            std::cerr << "Unrecognised shader target language: " << language << std::endl;
            return EXIT_FAILURE;
        }

        GfxGslMetadata md;
        md.params = params;
        md.env.ubt = ubt;
        md.env.fadeDither = alpha_dither;
        md.env.envBoxes = env_boxes;
        md.env.instanced = instanced;
        md.env.boneWeights = bones;
        md.d3d9 = true;

        GfxGasolineResult shaders;
        try {
            if (kind == "SKY") {
                shaders = gfx_gasoline_compile_sky(backend, vert_code, additional_code, md);
            } else if (kind == "HUD") {
                shaders = gfx_gasoline_compile_hud(backend, vert_code, additional_code, md);
            } else if (kind == "FIRST_PERSON") {
                shaders = gfx_gasoline_compile_first_person(backend, vert_code, dangs_code,
                                                            additional_code, md);
            } else if (kind == "DECAL") {
                shaders = gfx_gasoline_compile_decal(backend, dangs_code, additional_code, md);
            } else {
                std::cerr << "Unrecognised shader kind language: " << kind << std::endl;
                return EXIT_FAILURE;
            }
        } catch (const Exception &e) {
            EXCEPT << vert_in_filename << ", " << dangs_in_filename << ", "
                   << additional_in_filename << ": " << e.msg << ENDL;
        }

        std::ofstream of;
        of.open(vert_out_filename);
        if (!of.good()) {
            std::cerr << "Opening file: ";
            perror(vert_out_filename.c_str());
            return EXIT_FAILURE;
        }
        of << shaders.vertexShader;
        of.close();
        
        of.open(frag_out_filename);
        if (!of.good()) {
            std::cerr << "Opening file: ";
            perror(frag_out_filename.c_str());
            return EXIT_FAILURE;
        }
        of << shaders.fragmentShader;
        of.close();
        return EXIT_SUCCESS;

    } catch (const Exception &e) {
        std::cerr << e << std::endl;
        return EXIT_FAILURE;

    }


}

// vim: shiftwidth=4:tabstop=4:expandtab
