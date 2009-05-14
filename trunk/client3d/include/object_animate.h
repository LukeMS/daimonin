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

#ifndef OBJECT_ANIMATE_H
#define OBJECT_ANIMATE_H

#include <Ogre.h>

/**
 ** This class handles object animation.
 ** @todo separate Upper/lower Body animation.
 **       So it will be possible to walk and shoot arrows at same time
 **       without generating mixed mode animations.
 *****************************************************************************/
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
        ANIM_GROUP_EMOTE,
        ANIM_GROUP_SUM
    };
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectAnimate(Ogre::Entity *entity);
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
    Ogre::Real getTimeLeft()
    {
        return mTimeLeft;
    }
    Ogre::Real getTimeLeft2()
    {
        return mTimeLeft2;
    }
    int getSumAnimsInGroup(int animGroup)
    {
        return mAnimGroupEntries[animGroup];
    }
    void update(const Ogre::FrameEvent& event);
    void toggleAnimation(int animGroup, int animNr, bool loop = false, bool force = false, bool random = false, bool freezeLastFrame = false);
    void toggleAnimation2(int animGroup, int animNr, bool loop = false, bool force = false, bool random = false);
    Ogre::Real getAnimSpeed()
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
    Ogre::Real mAnimSpeed;
    Ogre::Real mTimeLeft, mTimeLeft2;
    Ogre::AnimationState *mActState, *mActState2;
    std::vector<Ogre::AnimationState*>mAnimState;
    Ogre::uchar mAnimGroupEntries[ANIM_GROUP_SUM];
    static const char *StateNames[ANIM_GROUP_SUM];
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
};

#endif
