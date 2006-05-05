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

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "option.h"
#include "logger.h"
#include "object_animate.h"

/// IMPORTANT:
/// Every animated model MUST have at least the Idle1 animation.

///=================================================================================================
/// Init all static Elemnts.
///=================================================================================================
const char *Animate::StateNames[SUM_ANIM_GROUP]=
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
Animate::Animate(Entity *entity)
{
    mAnimSpeed = 2;
    mIsAnimated = false;
    std::string strTmp, strGroup;
    AnimationState *AnimState;

    // fill the animation states.
    int j;
    int sum =0;
    for (int i=0; i < SUM_ANIM_GROUP; ++i)
    {
        mAnimGroupEntries[i] =0;
        strGroup = StateNames[i];
        try
        {
            j =0;
            while(1)
            {
                strTmp = strGroup + StringConverter::toString(++j);
                AnimState = entity->getAnimationState(strTmp.c_str());
                mAnimState.push_back(AnimState);
                mAnimGroupEntries[i]= j;
                ++sum;
            }
        }
        catch(Exception& )
        {
            // No animation with this name found.
        }
    }
    if (sum)
    {
      mIsAnimated = true;
      Logger::log().info() << "- Model has " << sum << " valid animations.";
    }
    else
    {
      Logger::log().info() << "- Model is not animated.";
      return;
    }
    /// Set the init-anim to Idle1.
    mActState= mAnimState[ANIM_GROUP_IDLE + 0];
    toggleAnimation(ANIM_GROUP_IDLE, 0, true, true);
}

///=================================================================================================
/// Constructor.
///=================================================================================================
Animate::~Animate()
{
  mAnimState.clear();
}

///=================================================================================================
/// Update the animation.
///=================================================================================================
void Animate::update(const FrameEvent& event)
{
    if (!mIsAnimated) return;
    mActState->addTime(event.timeSinceLastFrame * mAnimSpeed);
    /// if an animation ends -> force the idle animation.
    if (mActState->getTimePosition() >= mActState->getLength())
    {
        toggleAnimation(ANIM_GROUP_IDLE, 0, true, true);
    }
}

///=================================================================================================
/// Toggle the animation.
///=================================================================================================
void Animate::toggleAnimation(int animGroup, int animNr, bool loop, bool force)
{
    if (!mIsAnimated) return;
    /// Is the selected animation already running?
    if (animGroup == mAnimGroup && animNr == mAnimNr)
      return;
    /// Dont change a running (none-movement) anim without the force-switch.
    if (!force && !isMovement())
      return;

    /// On invalid animGroup choose Idle.
    if (animGroup >= SUM_ANIM_GROUP || !mAnimGroupEntries[animGroup])
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
    mActState->setTimePosition(0);
    mActState->setEnabled(true);
    mActState->setLoop(loop);
}
