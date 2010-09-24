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

#ifndef OBJECT_ELEMENT_ANIMATE3D_H
#define OBJECT_ELEMENT_ANIMATE3D_H

/// @brief This class handles 3d animation of an object.
/// @details Every animated model MUST have at least the Idle1 animation.
/// @todo separate Upper/lower Body animation.
///       So it will be possible to walk and shoot arrows at same time
///       without generating mixed mode animations.
class ObjectElementAnimate3d : public ObjectElement
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
    /// @brief Default constructor.
    ObjectElementAnimate3d(Object *parent, Ogre::Entity *entity);

    /// @brief Default destructor.
    ~ObjectElementAnimate3d();

    /// @brief Get the familyID of the element.
    Object::familyID getFamilyID() const
    {
        return Object::FAMILY_ANIMATE3D;
    }

    /// @brief Update this element.
    /// @param event Ogre frame event. Used to get time since last frame.
    bool update(const Ogre::FrameEvent &event);

    /// @brief Does this animation shows a movement?
    bool isMovement() const
    {
        return (mAnimGroup <= ANIM_GROUP_RUN);
    }

    /// @brief Does this animation shows an idle state?
    bool isIdle() const
    {
        return (mAnimGroup <= ANIM_GROUP_IDLE_FUN);
    }

    /// @brief Does this animation shows an attack?
    bool isAttack() const
    {
        return (mAnimGroup == ANIM_GROUP_ATTACK) || (mAnimGroup == ANIM_GROUP_ATTACK_FUN);
    }

    /// @brief Does this animation shows a hit?
    bool isHit() const
    {
        return (mAnimGroup == ANIM_GROUP_HIT);
    }

    /// @brief Get the name of the current animation.
    const char *getActiveStateName() const
    {
        return mSTATE_NAMES[mAnimGroup];
    }

    /// @brief Get the time left for the current animation.
    Ogre::Real getTimeLeft() const
    {
        return mTimeLeft;
    }

    /// @brief Get the number of animations for a given group.
    int getSumAnimsInGroup(int animGroup) const
    {
        return mAnimGroupEntries[animGroup];
    }

    /// @brief Get the speed of the current animation.
    Ogre::Real getAnimSpeed() const
    {
        return mAnimSpeed;
    }

    /// @brief Set the current animation.
    void setAnimation(int animGroup, int animNr, bool loop = false, bool force = false, bool random = false, bool freezeLastFrame = false);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::Real mTimeLeft;  ///< Time left for the current animation.
    Ogre::Real mAnimSpeed; ///< The current animation speed.
    int mAnimGroup;        ///< The current animation group number.
    int mAnimNr;           ///< The current animation number.
    bool mFreezeLastFrame; ///< False: after current anim is done play idle0, true: last frame of current anim will be freezed.
    Ogre::AnimationState *mActState;
    std::vector<Ogre::AnimationState*>mAnimState;
    Ogre::uchar mAnimGroupEntries[ANIM_GROUP_SUM];
    static const char *mSTATE_NAMES[ANIM_GROUP_SUM];
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectElementAnimate3d(const ObjectElementAnimate3d&);            ///< disable copy-constructor.
    ObjectElementAnimate3d &operator=(const ObjectElementAnimate3d&); ///< disable assignment operator.
};

#endif
