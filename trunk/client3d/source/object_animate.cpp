/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#include "option.h"
#include "logger.h"
#include "object_animate.h"

using namespace Ogre;

/** IMPORTANT: Every animated model MUST have at least the Idle1 animation. **/
//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
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
    "Spawn",
    "Cast", "Cast_Fun",
    "Emote",
};

//=================================================================================================
// Constructor.
//=================================================================================================
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
    // fill the animation states.
    int j;
    int sum =0;
    AnimationStateSet *animSet = entity->getAllAnimationStates();
    for (int i=0; i < ANIM_GROUP_SUM; ++i)
    {
        mAnimGroupEntries[i] =0;
        strGroup = StateNames[i];
        j =0;
        while (1)
        {
            strTmp = strGroup + StringConverter::toString(++j);
            if (!animSet->hasAnimationState(strTmp))
                break;
            mAnimState.push_back(animSet->getAnimationState(strTmp));
            mAnimGroupEntries[i]= j;
            ++sum;
        }
    }
    // We have a animated mesh, but it has not a single valid name.
    int invalidAnims = entity->getSkeleton()->getNumAnimations()-sum;
    if (invalidAnims && !sum)
    {
        Logger::log().list() << "Object has no valid animation";
        mIsAnimated = false;
        return;
    }
    // Every skeleton MUST have the Idle1 animation.
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
    // Set the init-anim to Idle1.
    mActState= mAnimState[ANIM_GROUP_IDLE + 0];
    toggleAnimation(ANIM_GROUP_IDLE, 0, true, true, true);

    mActState2= mAnimState[ANIM_GROUP_IDLE + 0];
    toggleAnimation2(ANIM_GROUP_IDLE, 0, true, true, true);
}
//=================================================================================================
// Constructor.
//=================================================================================================
ObjectAnimate::~ObjectAnimate()
{
    mAnimState.clear();
}
//=================================================================================================
// Update the animation.
//=================================================================================================
void ObjectAnimate::update(const FrameEvent& event)
{
    if (!mIsAnimated) return;

    mActState->addTime(event.timeSinceLastFrame * mAnimSpeed);
    mTimeLeft = mActState->getLength() - mActState->getTimePosition();
    // if an animation ends -> force the idle animation.
    if (mActState->getTimePosition() >= mActState->getLength() && !mActState->getLoop())
    {
        if (mAnimGroup != ANIM_GROUP_DEATH && mFreezeLastFrame == false)
            toggleAnimation(ANIM_GROUP_IDLE, 0, true, true, true);
    }
    /*
        mActState2->addTime(event.timeSinceLastFrame * mAnimSpeed);
        mTimeLeft2 = mActState->getLength() - mActState->getTimePosition();
        // if an animation ends -> force the idle animation.
        if (mActState2->getTimePosition() >= mActState2->getLength() && !mActState2->getLoop())
        {
            if (mAnimGroup2 != ANIM_GROUP_DEATH)
                toggleAnimation2(ANIM_GROUP_IDLE, 0, true, true, true);
        }
    */
}

//=================================================================================================
// Toggle the animation.
//=================================================================================================
void ObjectAnimate::toggleAnimation(int animGroup, int animNr, bool loop, bool force, bool random, bool freezeLastFrame)
{
    if (!mIsAnimated)
        return;
    // Is the selected animation already running?
    if (animGroup == mAnimGroup && animNr == mAnimNr)
        return;
    // Dont change a running (none-movement) anim without the force-switch.
    if (!force && !isMovement())
        return;
    mFreezeLastFrame = freezeLastFrame;
    // On invalid animGroup choose Idle.
    if (animGroup >= ANIM_GROUP_SUM || !mAnimGroupEntries[animGroup])
    {
        animGroup = ANIM_GROUP_IDLE;
        loop = true;
    }
    // On invalid animNr choose 0.
    if (animNr >= mAnimGroupEntries[animGroup])
        animNr = 0;
    mAnimNr = animNr;
    // If the previous anim was spawn, we cant use random offsets (because of seamless animations).

    if (animGroup == ANIM_GROUP_SPAWN) random = false;
    mAnimGroup = animGroup;
    // Find the anim pos in the anim-vector.
    animGroup =0;
    for (int i=0; i< mAnimGroup; ++i)
    {
        animGroup+= mAnimGroupEntries[i];
    }
    // Set the Animation.
    mActState->setEnabled(false);
    mActState= mAnimState[animGroup+ animNr];

    // Set a random offest for the animation start (prevent synchronous "dancing").
    if (random)
        mActState->setTimePosition(Math::RangeRandom(0.0, mActState->getLength()));
    else
        mActState->setTimePosition(0.0);

    mActState->setEnabled(true);
    mActState->setLoop(loop);
}

//=================================================================================================
// Toggle the animation.
//=================================================================================================
void ObjectAnimate::toggleAnimation2(int animGroup, int animNr, bool loop, bool force, bool random)
{
    if (!mIsAnimated)
        return;
    // Is the selected animation already running?
    if (animGroup == mAnimGroup && animNr == mAnimNr)
        return;
    // Dont change a running (none-movement) anim without the force-switch.
    if (!force && !isMovement())
        return;
    // On invalid animGroup choose Idle.
    if (animGroup >= ANIM_GROUP_SUM || !mAnimGroupEntries[animGroup])
    {
        animGroup = ANIM_GROUP_IDLE;
        loop = true;
    }
    // On invalid animNr choose 0.
    if (animNr >= mAnimGroupEntries[animGroup])
        animNr = 0;
    mAnimNr2 = animNr;
    // If the previous anim was spawn, we cant use random offsets (because of seamless animations).

    if (animGroup == ANIM_GROUP_SPAWN) random = false;
    mAnimGroup2 = animGroup;
    // Find the anim pos in the anim-vector.
    animGroup =0;
    for (int i=0; i< mAnimGroup2; ++i)
    {
        animGroup+= mAnimGroupEntries[i];
    }
    // Set the Animation.
    mActState2->setEnabled(false);
    mActState2= mAnimState[animGroup+ animNr];

    // Set a random offest for the animation start (prevent synchronous "dancing").
    if (random)
        mActState2->setTimePosition(Math::RangeRandom(0.0, mActState2->getLength()));
    else
        mActState2->setTimePosition(0.0);

    mActState2->setEnabled(true);
    mActState2->setLoop(loop);
}
