/* Copyright (c) The Grit Game Engine authors 2016
 *
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

#ifndef GFX_DEBUG_H
#define GFX_DEBUG_H

#include <math_util.h>

#include "gfx_pipeline.h"

// Call these functions at any time between frames to render basic polygons in the world.

void gfx_debug_point (Vector3 pos, float radius, Vector3 col, float alpha);

void gfx_debug_line (Vector3 from, Vector3 to, Vector3 col, float alpha);

void gfx_debug_triangle (Vector3 v1, Vector3 v2, Vector3 v3, Vector3 col, float alpha);

void gfx_debug_quad (Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4, Vector3 col, float alpha);

// The following are called by the graphics subsystem.

// Called every frame.
void gfx_debug_render (GfxPipeline *p);

void gfx_debug_init (void);
void gfx_debug_shutdown (void);

#endif
