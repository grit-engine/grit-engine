/* Copyright (c) The Grit Game Engine authors 2016
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
    "              | -i | --instanced                Enable instanced\n"
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
    if (type_name == "StaticFloat") return GfxGslParam::float1(0).setStatic();
    if (type_name == "StaticFloat2") return GfxGslParam::float2(0, 0).setStatic();
    if (type_name == "StaticFloat3") return GfxGslParam::float3(0, 0, 0).setStatic();
    if (type_name == "StaticFloat4") return GfxGslParam::float4(0, 0, 0, 0).setStatic();
    if (type_name == "StaticInt") return GfxGslParam::int1(0).setStatic();
    if (type_name == "StaticInt2") return GfxGslParam::int2(0, 0).setStatic();
    if (type_name == "StaticInt3") return GfxGslParam::int3(0, 0, 0).setStatic();
    if (type_name == "StaticInt4") return GfxGslParam::int4(0, 0, 0, 0).setStatic();
    EXCEPT << "Not a Gasoline type: " << type_name << ENDL;
}


CentralisedLog clog;
void assert_triggered (void) { }

int main (int argc, char **argv)
{

    try {
        int so_far = 1;
        bool no_more_switches = false;
        bool instanced = false;
        bool alpha_dither = false;
        bool internal = false;
        unsigned env_boxes = 0;
        unsigned bones = 0;
        std::vector<std::string> args;
        std::string language = "GLSL33";
        GfxGslRunParams params;
        GfxGslUnboundTextures ubt;
        GfxGslRunParams static_values;
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
                GfxGslParam param = from_string(type_name);
                params[name] = param;
                if (gfx_gasoline_param_is_static(param)) {
                    static_values[name] = param;  // Use the default colour.
                }
            } else if (arg=="-u" || arg=="--unbind") {
                std::string name = next_arg(so_far, argc, argv);
                ubt[name] = false;
            } else if (arg=="-U" || arg=="--unbind-to-uniform") {
                std::string name = next_arg(so_far, argc, argv);
                ubt[name] = true;
            } else if (arg=="-I" || arg=="--internal") {
                internal = true;
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
        f.open(vert_in_filename);
        if (!f.good()) {
            std::cerr << "Opening file: ";
            perror(vert_in_filename.c_str());
            return EXIT_FAILURE;
        }
        vert_code.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        f.close();

        std::string dangs_code;
        f.open(dangs_in_filename);
        if (!f.good()) {
            std::cerr << "Opening file: ";
            perror(dangs_in_filename.c_str());
            return EXIT_FAILURE;
        }
        dangs_code.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        f.close();

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
        } else if (language == "GLSL33") {
            backend = GFX_GSL_BACKEND_GLSL33;
        } else {
            std::cerr << "Unrecognised shader target language: " << language << std::endl;
            return EXIT_FAILURE;
        }


        GfxGasolineResult shaders;
        try {
            GfxGslPurpose purpose;
            if (kind == "SKY") {
                purpose = GFX_GSL_PURPOSE_SKY;
            } else if (kind == "HUD") {
                purpose = GFX_GSL_PURPOSE_HUD;
            } else if (kind == "FORWARD") {
                purpose = GFX_GSL_PURPOSE_FORWARD;
            } else if (kind == "ALPHA") {
                purpose = GFX_GSL_PURPOSE_ALPHA;
            } else if (kind == "FIRST_PERSON") {
                purpose = GFX_GSL_PURPOSE_FIRST_PERSON;
            } else if (kind == "FIRST_PERSON_WIREFRAME") {
                purpose = GFX_GSL_PURPOSE_FIRST_PERSON_WIREFRAME;
            } else if (kind == "DECAL") {
                purpose = GFX_GSL_PURPOSE_DECAL;
            } else if (kind == "CAST") {
                purpose = GFX_GSL_PURPOSE_CAST;
            } else {
                std::cerr << "Unrecognised shader kind language: " << kind << std::endl;
                return EXIT_FAILURE;
            }
            GfxGslMetadata md;
            md.params = params;
            md.matEnv.ubt = ubt;
            md.matEnv.staticValues = static_values;
            md.matEnv.fadeDither = alpha_dither;
            md.cfgEnv.envBoxes = env_boxes;
            md.meshEnv.instanced = instanced;
            md.meshEnv.boneWeights = bones;
            md.d3d9 = true;
            md.internal = internal;
            md.lightingTextures = gfx_gasoline_does_lighting(purpose);
            shaders = gfx_gasoline_compile(purpose, backend, vert_code, dangs_code,
                                           additional_code, md);
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
