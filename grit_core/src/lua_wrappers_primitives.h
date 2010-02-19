/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

#include "lua_wrappers_common.h"

#define STRINGDB_TAG "Grit/StringDB"
MT_MACRO_DECLARE(stringdb);
int stringdb_make(lua_State *L);

#define SPLINE_TAG "Grit/Spline"
MT_MACRO_DECLARE(spline);
int spline_make(lua_State *L);

#define PLOT_TAG "Grit/Plot"
MT_MACRO_DECLARE(plot);
int plot_make(lua_State *L);

#define VECTOR3_TAG "Grit/Vector3"
MT_MACRO_DECLARE(vector3);
int vector3_make(lua_State *L);

#define QUAT_TAG "Grit/Quaternion"
MT_MACRO_DECLARE(quat);
int quat_make(lua_State *L);

#define TIMER_TAG "Grit/Timer"
MT_MACRO_DECLARE(timer);
int timer_make(lua_State *L);


// vim: shiftwidth=8:tabstop=8:expandtab
