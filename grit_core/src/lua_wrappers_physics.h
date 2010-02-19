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

#include "PhysicsWorld.h"

#define PWORLD_TAG "Grit/PhysicsWorld"
MT_MACRO_DECLARE(pworld);
int pworld_make(lua_State *L);
void push_pworld (lua_State *L, const PhysicsWorldPtr &self);


#define COLMESH_TAG "Grit/CollisionMesh"
MT_MACRO_DECLARE(colmesh);

#define RBODY_TAG "Grit/RigidBody"
MT_MACRO_DECLARE(rbody);
void push_rbody (lua_State *L, const RigidBodyPtr &self);



// vim: shiftwidth=8:tabstop=8:expandtab
