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
#include <OgreSceneManager.h>

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
  ~Animate()
  {
  }
  bool isMovement()
  {
    return (mAnimType < STATE_ATTACK1);
  }
  void update(const FrameEvent& event);
  void toggleAnimGroup();
  void toggleAnimation(int animationNr, bool force = false);
  Real getAnimSpeed()
  {
    return mAnimSpeed;
  }
  Real getTurnSpeed()
  {
    return mTurnSpeed;
  }
  enum AnimState
  {
    STATE_IDLE1, STATE_IDLE2, STATE_IDLE3, STATE_IDLE4, STATE_IDLE5, STATE_IDLE6, STATE_IDLE7, STATE_IDLE8, STATE_IDLE9,STATE_IDLE10,
    STATE_IDLE11, STATE_IDLE12, STATE_IDLE13, STATE_IDLE14, STATE_IDLE15, STATE_IDLE16, STATE_IDLE17, STATE_IDLE18, STATE_IDLE19,STATE_IDLE20,
    STATE_WALK1, STATE_WALK2, STATE_WALK3,
    STATE_RUN1,  STATE_RUN2,  STATE_RUN3,
    STATE_ATTACK1, STATE_ATTACK2, STATE_ATTACK3, STATE_ATTACK4, STATE_ATTACK5, STATE_ATTACK6,
    STATE_BLOCK1, STATE_BLOCK2, STATE_BLOCK3,
    STATE_SLUMP1, STATE_SLUMP2, STATE_SLUMP3,
    STATE_DEATH1, STATE_DEATH2, STATE_DEATH3,
    STATE_HIT1,  STATE_HIT2,  STATE_HIT3,
    /// Castings MUST be the last entries!
    STATE_CAST1, STATE_CAST2, STATE_CAST3,
    STATE_SUM
  };

private:
  /// ////////////////////////////////////////////////////////////////////
  /// Variables.
  /// ////////////////////////////////////////////////////////////////////
  int mAnimType;
  int mAnimGroup;
  bool mSpellTrigger;
  AnimationState *mAnimState;
  AnimationState *mAnimStates[STATE_SUM];
  Real mAnimSpeed, mTurnSpeed;
  std::string mStrTemp;
  int sound_handle[STATE_SUM];
  static const char *StateNames[STATE_SUM];
  /// ////////////////////////////////////////////////////////////////////
  /// Functions.
  /// ////////////////////////////////////////////////////////////////////
};

#endif
