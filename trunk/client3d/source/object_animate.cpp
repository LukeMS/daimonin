/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "option.h"
#include "logger.h"
#include "object_animate.h"

/** IMPORTANT: Every animated model MUST have at least the Idle1 animation. **/

///=================================================================================================
/// Init all static Elemnts.
///=================================================================================================
const char *ObjectAnimate::StateNames[ANIM_GROUP_SUM]=
    {
        "Idle", "Idle_Fun",
        "Walk",
        "Run",
        "Ability",
        "Attack", "Attack_Fun",
        "Block",
        "Hit",
        "Slump",
        "Death",
        "Cast", "Cast_Fun",
    };

///=================================================================================================
/// Constructor.
///=================================================================================================
ObjectAnimate::ObjectAnimate(Entity *entity)
{
    if (!entity->hasSkeleton())
    {
        mIsAnimated = false;
        Logger::log().list() << "Object has no Skeleton. It is not animated.";
        return;
    }
    mAnimSpeed = 2;
    mAnimGroup =-1;
    String strTmp, strGroup;

    /// fill the animation states.
    int j;
    int sum =0;
    AnimationStateSet *animSet = entity->getAllAnimationStates();
    for (int i=0; i < ANIM_GROUP_SUM; ++i)
    {
        mAnimGroupEntries[i] =0;
        strGroup = StateNames[i];
        j =0;
        while(1)
        {
            strTmp = strGroup + StringConverter::toString(++j);
            if (!animSet->hasAnimationState(strTmp))
                break;
            mAnimState.push_back(animSet->getAnimationState(strTmp));
            mAnimGroupEntries[i]= j;
            ++sum;
        }
    }

    /// We have a animated mesh, but it has not a single valid name.
    int invalidAnims = entity->getSkeleton()->getNumAnimations()-sum;
    if (invalidAnims && !sum)
    {
        Logger::log().list() << "Object has no valid animation";
        mIsAnimated = false;
        return;
    }

    /// Every skeleton MUST have the Idle1 animation.
    if (!animSet->hasAnimationState("Idle1"))
    {
        Logger::log().error() << "The animation 'Idle1' is missing. No animation will be available.";
        mIsAnimated = false;
        return;
    }

    mIsAnimated = true;
    Logger::log().list() << "Object has " << entity->getSkeleton()->getNumAnimations() << " animations.";
    if (invalidAnims)
    {
      Logger::log().warning() << invalidAnims << " of the animations are invalid.";
    }

    /// Set the init-anim to Idle1.
    mActState= mAnimState[ANIM_GROUP_IDLE + 0];
    toggleAnimation(ANIM_GROUP_IDLE, 0, true, true, true);
}

///=================================================================================================
/// Constructor.
///=================================================================================================
ObjectAnimate::~ObjectAnimate()
{
    mAnimState.clear();
}

///=================================================================================================
/// Update the animation.
///=================================================================================================
void ObjectAnimate::update(const FrameEvent& event)
{
    if (!mIsAnimated)
        return;
    mActState->addTime(event.timeSinceLastFrame * mAnimSpeed);
    /// if an animation ends -> force the idle animation.
    if (mActState->getTimePosition() >= mActState->getLength())
    {
        toggleAnimation(ANIM_GROUP_IDLE, 0, true, true, true);
    }
}

///=================================================================================================
/// Toggle the animation.
///=================================================================================================
void ObjectAnimate::toggleAnimation(int animGroup, int animNr, bool loop, bool force, bool random)
{
    if (!mIsAnimated)
        return;
    /// Is the selected animation already running?
    if (animGroup == mAnimGroup && animNr == mAnimNr)
        return;
    /// Dont change a running (none-movement) anim without the force-switch.
    if (!force && !isMovement())
        return;

    /// On invalid animGroup choose Idle.
    if (animGroup >= ANIM_GROUP_SUM || !mAnimGroupEntries[animGroup])
        animGroup = ANIM_GROUP_IDLE;
    /// On invalid animNr choose 0.
    if (animNr >= mAnimGroupEntries[animGroup])
        animNr = 0;
    mAnimNr = animNr;
    mAnimGroup = animGroup;
    mActState->setEnabled(false);
    /// Find the anim pos in the anim-vector.
    animGroup =0;
    for (int i=0; i< mAnimGroup; ++i)
    {
        animGroup+= mAnimGroupEntries[i];
    }
    /// Set the Animation.
    mActState= mAnimState[animGroup+ animNr];

    /// Set a random offest for the animation start (prevent synchronous "dancing").
    if (random)
        mActState->setTimePosition(Math::RangeRandom(0.0, mActState->getLength()));
    else
        mActState->setTimePosition(0.0);
    mActState->setEnabled(true);
    mActState->setLoop(loop);
}
