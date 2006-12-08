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

#ifndef OBJECT_ANIMATE_H
#define OBJECT_ANIMATE_H

#include <Ogre.h>

using namespace Ogre;

// ////////////////////////////////////////////////////////////////////
// Defines.
// ////////////////////////////////////////////////////////////////////


// This will be rewritten for separted Upper/lower Body animation.
// So it will be possible to walk and shoot arrows at same time without generating
// Mixed Mode animations.

//================================================================================================
// Class.
//================================================================================================
class ObjectAnimate
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum AnimGroup
    {
        // Movement.
        ANIM_GROUP_IDLE,
        ANIM_GROUP_IDLE_FUN,
        ANIM_GROUP_WALK,
        ANIM_GROUP_RUN,
        // Non-movement.
        ANIM_GROUP_ABILITY,
        ANIM_GROUP_ATTACK,
        ANIM_GROUP_ATTACK_FUN,
        ANIM_GROUP_BLOCK,
        ANIM_GROUP_HIT,
        ANIM_GROUP_SLUMP,
        ANIM_GROUP_DEATH,
        ANIM_GROUP_SPAWN,
        ANIM_GROUP_CAST,
        ANIM_GROUP_CAST_FUN,
        ANIM_GROUP_SUM
    };
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectAnimate(Entity *entity);
    ~ObjectAnimate();
    bool isMovement()
    {
        return (mAnimGroup <= ANIM_GROUP_RUN);
    }
    bool isIdle()
    {
        return (mAnimGroup <= ANIM_GROUP_IDLE_FUN);
    }
    bool isAttack()
    {
        return (mAnimGroup == ANIM_GROUP_ATTACK) || (mAnimGroup == ANIM_GROUP_ATTACK_FUN);
    }
    bool isHit()
    {
        return (mAnimGroup == ANIM_GROUP_HIT);
    }
    const char *getActiceStateName()
    {
        return StateNames[mAnimGroup];
    }
    Real getTimeLeft()
    {
        return mTimeLeft;
    }
    Real getTimeLeft2()
    {
        return mTimeLeft2;
    }
    int getSumAnimsInGroup(int animGroup)
    {
        return mAnimGroupEntries[animGroup];
    }
    void update(const FrameEvent& event);
    void toggleAnimation(int animGroup, int animNr, bool loop = false, bool force = false, bool random = false, bool freezeLastFrame = false);
    void toggleAnimation2(int animGroup, int animNr, bool loop = false, bool force = false, bool random = false);
    Real getAnimSpeed()
    {
        return mAnimSpeed;
    }
    void pause(bool p)
    {
        mActState->setEnabled(!p);
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    int mAnimGroup, mAnimNr;
    int mAnimGroup2, mAnimNr2;
    bool mPause;
    bool mIsAnimated;
    bool mFreezeLastFrame; /**< false: after current anim is done play idle0, true: last frame of current anim will be freezed. */
    Real mAnimSpeed;
    Real mTimeLeft, mTimeLeft2;
    AnimationState *mActState, *mActState2;
    std::vector<AnimationState*>mAnimState;
    unsigned char mAnimGroupEntries[ANIM_GROUP_SUM];
    static const char *StateNames[ANIM_GROUP_SUM];
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
};

#endif
