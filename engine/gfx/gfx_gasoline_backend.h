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

#include <sstream>
#include <string>

#include "gfx_gasoline_parser.h"
#include "gfx_gasoline_type_system.h"

#ifndef GFX_GSL_BACKEND_H
#define GFX_GSL_BACKEND_H

class GfxGslBackendUnparser {
    std::stringstream ss;
    const std::string varPref;

    public:
    GfxGslBackendUnparser (const std::string &var_pref)
      : varPref(var_pref)
    { }

    void zeroInitialise(const std::string &space, const std::string &name, GfxGslType *t);
    void unparseType (const GfxGslType *t_);
    void unparse (const GfxGslAst *ast_, int indent);

    std::string getUserVertexFunction (void)
    {
        std::stringstream ss2;
        ss2 << "void func_user_vertex (out Float3 out_position)\n";
        ss2 << "{\n";
        ss2 << "    out_position  = transform_to_world(vert_position.xyz);\n";
        ss2 << ss.str();
        ss2 << "}\n";
        return ss2.str();
    }
    std::string getUserDangsFunction (void)
    {
        std::stringstream ss2;
        ss2 << "void func_user_dangs (out Float3 out_diffuse, out Float out_alpha, out Float3 out_normal, out Float out_gloss, out Float out_specular)\n";
        ss2 << "{\n";
        ss2 << "    out_diffuse = Float3(0.5, 0.25, 0.0);\n";
        ss2 << "    out_alpha = 1.0;\n";
        ss2 << "    out_normal = Float3(0.0, 0.0, 1.0);\n";
        ss2 << "    out_gloss = 1.0;\n";
        ss2 << "    out_specular = 0.04;\n";
        ss2 << ss.str();
        ss2 << "}\n";
        return ss2.str();
    }
    std::string getUserColourAlphaFunction (void)
    {
        std::stringstream ss2;
        ss2 << "void func_user_colour (out Float3 out_colour, out Float out_alpha)\n";
        ss2 << "{\n";
        ss2 << "    out_colour = Float3(0.0, 0.0, 0.0);\n";
        ss2 << "    out_alpha = 1;\n";
        ss2 << ss.str();
        ss2 << "}\n";
        return ss2.str();
    }
};

std::string gfx_gasoline_generate_preamble_functions (void);

std::string gfx_gasoline_generate_global_fields (const GfxGslContext &ctx, bool reg);

std::string gfx_gasoline_generate_var_decls (const GfxGslTypeMap &vars);

std::string gfx_gasoline_generate_trans_encode (const std::vector<GfxGslTrans> &trans,
                                                const std::string &var_pref);

std::string gfx_gasoline_generate_trans_decode (const std::vector<GfxGslTrans> &trans,
                                                const std::string &pref,
                                                GfxGslTrans::Kind only);

/** Generate general purpose utility functions for computing lighting. */
std::string gfx_gasoline_preamble_lighting (const GfxGslConfigEnvironment &cfg_env);

/** Generate general purpose utility functions for dither fading. */
std::string gfx_gasoline_preamble_fade (void);

/** Generate general purpose utility functions for transforming geometry to world space. */
std::string gfx_gasoline_preamble_transformation (bool first_person,
                                                  const GfxGslMeshEnvironment &mesh_env);

typedef std::map<std::string, const GfxGslFloatType *> GfxGslInternalMap;
static inline void gfx_gasoline_add_internal_trans(const GfxGslInternalMap &internals,
                                                   std::vector<GfxGslTrans> &trans)
{
    for (const auto &pair : internals) {
        if (pair.second->dim == 1) {
            trans.emplace_back(GfxGslTrans{ GfxGslTrans::INTERNAL, {pair.first}, pair.second });
        } else {
            const char *chars[] = {"x", "y", "z"};
            for (unsigned i = 0; i < pair.second->dim ; ++i) {
                trans.emplace_back(
                    GfxGslTrans{ GfxGslTrans::INTERNAL, {pair.first, chars[i]}, pair.second });
            }
        }
    }
}

#endif
