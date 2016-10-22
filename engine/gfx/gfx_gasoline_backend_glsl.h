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

#include <cstdlib>

#include <string>

#ifndef GFX_GASOLINE_BACKEND_GLSL_H
#define GFX_GASOLINE_BACKEND_GLSL_H

#include "gfx_gasoline.h"
#include "gfx_gasoline_parser.h"
#include "gfx_gasoline_type_system.h"

void gfx_gasoline_unparse_glsl (const GfxGslContext &ctx,
                                const GfxGslTypeSystem *vert_ts,
                                const GfxGslAst *vert_ast,
                                std::string &vert_output,
                                const GfxGslTypeSystem *frag_ts,
                                const GfxGslAst *frag_ast,
                                std::string &frag_output,
                                const GfxGslEnvironment &env,
                                bool glsl33,
                                bool flat_z,
                                bool das);


void gfx_gasoline_unparse_body_glsl(const GfxGslContext &ctx,
                                    const GfxGslTypeSystem *vert_ts,
                                    const GfxGslAst *vert_ast,
                                    const GfxGslTypeSystem *dangs_ts,
                                    const GfxGslAst *dangs_ast,
                                    const GfxGslTypeSystem *additional_ts,
                                    const GfxGslAst *additional_ast,
                                    std::string &vert_out,
                                    std::string &frag_out,
                                    const GfxGslEnvironment &env,
                                    bool glsl33,
                                    bool first_person,
                                    bool wireframe,
                                    bool forward_only,
                                    bool cast);

void gfx_gasoline_unparse_decal_glsl(const GfxGslContext &ctx,
                                     const GfxGslTypeSystem *dangs_ts,
                                     const GfxGslAst *dangs_ast,
                                     const GfxGslTypeSystem *additional_ts,
                                     const GfxGslAst *additional_ast,
                                     std::string &vert_out,
                                     std::string &frag_out,
                                     const GfxGslEnvironment &env,
                                     bool glsl33);


#endif

