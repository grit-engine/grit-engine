/* Copyright (c) Simon Kolciter and the Grit Game Engine project 2013
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
#include "AnimationMixer.h"
#include "Ogre.h"
#include "gfx.h"

AnimationMixer::AnimationMixer(Entity *entity) 
    : _Entity(entity)
    , _Active(Animation::Invalid)
{
    Init();
}

void AnimationMixer::Init(void)
{
    _States.clear();

    if (!_Entity)
    {
        GRIT_EXCEPT("Animations: invalid entity");
        return;
    }

    Ogre::AnimationStateIterator it = _Entity->getAllAnimationStates()->getAnimationStateIterator();

    while (it.hasMoreElements())
    {
        _States.push_back(it.getNext());
    }
}

AnimId AnimationMixer::FindAnimation(String const &name)
{
    int i = 0;
    for (TStateIterator it = _States.begin(); it != _States.end(); ++it, ++i)
        if (!(*it)->getAnimationName().compare(name))
            return i;

    return Animation::Invalid;
}

void AnimationMixer::PlayAnimation(AnimId id, bool looped)
{
    // Turn off current animation state
    if (IndexValid(_Active))
    {        
        _States[_Active]->setEnabled(false);
    }

    if (IndexValid(id))
    {
        _Active = id;
        AnimationState* const active = _States[_Active];
        active->setLoop(looped);
        active->setEnabled(true);
    }
}

void AnimationMixer::PlayAnimation(String const &name, bool looped)
{
    PlayAnimation(FindAnimation(name),looped);
}

void AnimationMixer::Update(float deltaT)
{
    if (IndexValid(_Active))
        _States[_Active]->addTime(deltaT);  
}

AnimationMixer::~AnimationMixer(void)
{
}
