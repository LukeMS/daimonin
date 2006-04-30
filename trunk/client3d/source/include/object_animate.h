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

#ifndef OBJECT_ANIMATE_H
#define OBJECT_ANIMATE_H

#include <Ogre.h>

using namespace Ogre;

/// ////////////////////////////////////////////////////////////////////
/// Defines.
/// ////////////////////////////////////////////////////////////////////

static const Real RAD = 3.14159265/180.0;

///================================================================================================
/// Class.
///================================================================================================
class Animate
{
public:
  /// ////////////////////////////////////////////////////////////////////
  /// Functions.
  /// ////////////////////////////////////////////////////////////////////
  Animate(Entity *entity);
  ~Animate();
  bool isMovement()
  {
    return (mAnimGroup <= ANIM_GROUP_RUN);
  }
  int getSumAnimsInGroup(int animGroup)
  {
    return mAnimGroupEntries[animGroup];
  }
  void update(const FrameEvent& event);
  void toggleAnimation(int animGroup, int animNr, bool loop = false, bool force = false);
  Real getAnimSpeed()
  {
    return mAnimSpeed;
  }
  enum AnimGroup
  {
    /// Movement.
    ANIM_GROUP_IDLE,
    ANIM_GROUP_IDLE_FUN,
    ANIM_GROUP_WALK,
    ANIM_GROUP_RUN,
    /// Non-movement.
    ANIM_GROUP_ABILITY,
    ANIM_GROUP_ATTACK,
    ANIM_GROUP_ATTACK_FUN,
    ANIM_GROUP_BLOCK,
    ANIM_GROUP_HIT,
    ANIM_GROUP_SLUMP,
    ANIM_GROUP_DEATH,
    ANIM_GROUP_CAST,
    ANIM_GROUP_CAST_FUN,
    SUM_ANIM_GROUP
  };

private:
  /// ////////////////////////////////////////////////////////////////////
  /// Variables.
  /// ////////////////////////////////////////////////////////////////////
  int mAnimGroup, mAnimNr;
  bool mPause;
  bool mIsAnimated;
  Real mAnimSpeed;
  AnimationState *mActState;
  std::vector<AnimationState*>mAnimState;
  unsigned char mAnimGroupEntries[SUM_ANIM_GROUP];
  static const char *StateNames[SUM_ANIM_GROUP];
  /// ////////////////////////////////////////////////////////////////////
  /// Functions.
  /// ////////////////////////////////////////////////////////////////////
};

#endif
