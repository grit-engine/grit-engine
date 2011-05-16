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

#include <OgrePixelFormat.h>

#include "../lua_wrappers_common.h"

#define SCNMGR_TAG "Grit/SceneManager"
MT_MACRO_DECLARE(scnmgr);
void push_scnmgr (lua_State *L, Ogre::SceneManager *scnmgr);


#define NODE_TAG "Grit/SceneNode"
MT_MACRO_DECLARE(node);
void push_node (lua_State *L, Ogre::SceneNode *n);

Ogre::PixelFormat pixel_format_from_string (lua_State *L, const std::string& s);
const char *pixel_format_to_string (lua_State *L, Ogre::PixelFormat pf);

// vim: shiftwidth=8:tabstop=8:expandtab
