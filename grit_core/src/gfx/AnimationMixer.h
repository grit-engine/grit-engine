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
#pragma once
#ifndef _ANIMATION_MIXER_H_
#define _ANIMATION_MIXER_H_

#include <vector>
#include <string>

namespace Ogre
{
    class Entity;
    class AnimationState;
}

typedef unsigned int AnimId;
typedef std::string String;
typedef Ogre::Entity Entity;
typedef Ogre::AnimationState AnimationState;
typedef std::vector<AnimationState*> TStates;
typedef TStates::iterator TStateIterator;

#define IndexValid(index) (index < _States.size())

//! Encapsulates handling of animations of a given Ogre::Entity instance
class AnimationMixer
{
    //! Entity associated with this anim mixer
    Ogre::Entity *_Entity;
    //! List of all moves available to the entity
    TStates _States;
    //! Current active animation
    AnimId _Active;

public:
    AnimationMixer(Ogre::Entity *entity);
    virtual ~AnimationMixer(void);

    //! Initializes the mixer according to its Ogre::Entity instance
    void Init();
    //! Get the permament id of a given move (for quicker access)
    AnimId FindAnimation(String const &name);
    //! Play animation with a given id
    void PlayAnimation(AnimId id, bool looped);
    //! Play animation with a given name
    void PlayAnimation(String const &name, bool looped);
    //! Advances the active animation by given time in seconds
    void Update(float deltaT);
    //! Returns the list of all animation states available to the Ogre::Entity object
    TStates const &GetAnims() const {return _States;}
};

//! Helper constants and functions
struct Animation
{    
    //! ID of invalid (non-existing) animation state
    static const AnimId Invalid = 0xFFFFFFFF;
    //! Deletes pointer and sets it to nullptr
    static void Delete(AnimationMixer *&ptr)
    {
        delete ptr;
        ptr=NULL;
    }
};
#endif // _ANIMATION_MIXER_H_
