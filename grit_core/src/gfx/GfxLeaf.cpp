/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

#include "gfx_internal.h"

#include "GfxLeaf.h"
#include "GfxNode.h"

const std::string GfxLeaf::className = "GfxLeaf";

GfxLeaf::GfxLeaf (const GfxNodePtr &par_)
{
    dead = false;
    pos = Vector3(0,0,0);
    scl = Vector3(1,1,1);
    quat = Quaternion(1,0,0,0);
    node = ogre_sm->createSceneNode();
    par = GfxNodePtr(NULL); setParent(par_);
}

GfxLeaf::~GfxLeaf (void)
{
    if (!dead) destroy();
}
    
void GfxLeaf::destroy (void)
{       
    if (!par.isNull()) par->notifyLostChild(this);
    ogre_sm->destroySceneNode(node->getName());
    node = NULL; 
    par = GfxNodePtr(NULL);
    dead = true;
}       
    
void GfxLeaf::notifyParentDead (void)
{
    if (dead) THROW_DEAD(className);
    APP_ASSERT(!par.isNull());
    par->node->removeChild(node);
    par = GfxNodePtr(NULL);
    ogre_root_node->addChild(node);
}   
    
const GfxNodePtr &GfxLeaf::getParent (void) const
{   
    if (dead) THROW_DEAD(className);
    return par;
}   
    
void GfxLeaf::setParent (const GfxNodePtr &par_)
{
    if (dead) THROW_DEAD(className);
    if (!par.isNull()) {
        par->node->removeChild(node);
        par->notifyLostChild(this);
    } else {
        ogre_root_node->removeChild(node);
    }
    par = par_;
    if (!par.isNull()) {
        par->notifyGainedChild(this);
        par->node->addChild(node);
    } else {
        ogre_root_node->addChild(node);
    }
}

void GfxLeaf::ensureNotChildOf (GfxNode *leaf) const
{
    if (leaf==this) GRIT_EXCEPT("Parenthood must be acyclic.");
    if (par.isNull()) return;
    par->ensureNotChildOf(leaf);
}

void GfxLeaf::setLocalPosition (const Vector3 &p)
{
    if (dead) THROW_DEAD(className);
    pos = p;
    node->setPosition(to_ogre(p));
}

void GfxLeaf::setLocalOrientation (const Quaternion &q)
{
    if (dead) THROW_DEAD(className);
    quat = q;
    node->setOrientation(to_ogre(q));
}

void GfxLeaf::setLocalScale (const Vector3 &s)
{
    if (dead) THROW_DEAD(className);
    scl = s;
    node->setScale(to_ogre(s));
}

Vector3 GfxLeaf::transformPosition (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return getWorldPosition() + getWorldScale()*(getWorldOrientation()*v);
}
Quaternion GfxLeaf::transformOrientation (const Quaternion &v)
{
    if (dead) THROW_DEAD(className);
    return getWorldOrientation()*v;
}
Vector3 GfxLeaf::transformScale (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return getWorldScale()*v;
}

Vector3 GfxLeaf::transformPositionParent (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return par.isNull() ? v : par->transformPosition(v);
}
Quaternion GfxLeaf::transformOrientationParent (const Quaternion &v)
{
    if (dead) THROW_DEAD(className);
    return par.isNull() ? v : par->transformOrientation(v);
}
Vector3 GfxLeaf::transformScaleParent (const Vector3 &v)
{
    if (dead) THROW_DEAD(className);
    return par.isNull() ? v : par->transformScale(v);
}

const Vector3 &GfxLeaf::getLocalPosition (void)
{
    if (dead) THROW_DEAD(className);
    return pos;
}
Vector3 GfxLeaf::getWorldPosition (void)
{
    if (dead) THROW_DEAD(className);
    return transformPositionParent(pos);
}

const Quaternion &GfxLeaf::getLocalOrientation (void)
{
    if (dead) THROW_DEAD(className);
    return quat;
}
Quaternion GfxLeaf::getWorldOrientation (void)
{
    if (dead) THROW_DEAD(className);
    return transformOrientationParent(quat);
}

const Vector3 &GfxLeaf::getLocalScale (void)
{
    if (dead) THROW_DEAD(className);
    return scl;
}
Vector3 GfxLeaf::getWorldScale (void)
{
    if (dead) THROW_DEAD(className);
    return transformScaleParent(scl);
}

