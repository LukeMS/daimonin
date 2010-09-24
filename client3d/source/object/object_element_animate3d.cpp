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

#include <OgreEntity.h>
#include <OgreAnimation.h>
#include <OgreStringConverter.h>
#include "logger.h"
#include "profiler.h"
#include "object/object.h"
#include "object/object_element_animate3d.h"

using namespace Ogre;

//=================================================================================================
// Init all static Elements.
//=================================================================================================
const char *ObjectElementAnimate3d::mSTATE_NAMES[ANIM_GROUP_SUM]=
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
ObjectElementAnimate3d::ObjectElementAnimate3d(Object *parent, Entity *entity)
    : ObjectElement(parent), mAnimSpeed(2), mAnimGroup(-1), mActState(0)
{
    PROFILE()
    parent->addElement(getFamilyID(), this);
    // Fill the animation states.
    int validAnimation =0;
    AnimationStateSet *animSet = entity->getAllAnimationStates();
    for (int i=0; i < ANIM_GROUP_SUM; ++i)
    {
        mAnimGroupEntries[i] =0;
        String strGroup = mSTATE_NAMES[i];
        int j =0;
        while (1)
        {
            String strTmp = strGroup + StringConverter::toString(++j);
            if (!animSet->hasAnimationState(strTmp))
                break;
            mAnimState.push_back(animSet->getAnimationState(strTmp));
            mAnimGroupEntries[i]= j;
            ++validAnimation;
        }
    }
    // Not a single animation has a compatible name.
    if (!validAnimation)
    {
        Logger::log().warning() << Logger::ICON_CLIENT << "Object has no valid animations.";
        return;
    }
    // Every skeleton MUST have the Idle1 animation.
    if (!animSet->hasAnimationState("Idle1"))
    {
        Logger::log().warning() << Logger::ICON_CLIENT << "The animation 'Idle1' is missing. No animation will be available.";
        return;
    }
    Logger::log().info() << Logger::ICON_CLIENT << "Object has " << validAnimation << " supported animations.";
    // Set the init-anim to Idle1.
    setAnimation(ANIM_GROUP_IDLE, 0, true, true, true);
}

//=================================================================================================
// Destructor.
//=================================================================================================
ObjectElementAnimate3d::~ObjectElementAnimate3d()
{
    PROFILE()
    mAnimState.clear();
}
//=================================================================================================
// Update the animation.
//=================================================================================================
bool ObjectElementAnimate3d::update(const FrameEvent &event)
{
    PROFILE()
    mActState->addTime(event.timeSinceLastFrame * mAnimSpeed);
    mTimeLeft = mActState->getLength() - mActState->getTimePosition();
    // if an animation ends -> force the idle animation.
    if (mActState->getTimePosition() >= mActState->getLength() && !mActState->getLoop())
    {
        if (mAnimGroup != ANIM_GROUP_DEATH && mFreezeLastFrame == false)
            setAnimation(ANIM_GROUP_IDLE, 0, true, true, true);
    }
    return true;
}

//=================================================================================================
// Stops the old and starts the new animation.
//=================================================================================================
void ObjectElementAnimate3d::setAnimation(int animGroup, int animNr, bool loop, bool force, bool random, bool freezeLastFrame)
{
    PROFILE()
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
        animGroup+= mAnimGroupEntries[i];
    if (mActState) mActState->setEnabled(false); // Stop the current animation.
    mActState= mAnimState[animGroup+ animNr];
    // Set a random offest for the animation start (prevent synchronous "dancing" of all objects).
    mActState->setTimePosition(random?Math::RangeRandom(0.0f, mActState->getLength()):0.0f);
    mActState->setEnabled(true); // Start the new animation.
    mActState->setLoop(loop);
}
