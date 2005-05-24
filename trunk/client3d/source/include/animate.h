/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#ifndef ANIMATE_H
#define ANIMATE_H

#include <Ogre.h>
#include <OgreSceneManager.h>

using namespace Ogre;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////

enum AnimState
{
    STATE_IDLE1,	STATE_IDLE2,	STATE_IDLE3,
    STATE_WALK1,	STATE_WALK2,	STATE_WALK3,
    STATE_RUN1,		STATE_RUN2,		STATE_RUN3,
    STATE_ATTACK1,	STATE_ATTACK2,	STATE_ATTACK3,
    STATE_BLOCK1,	STATE_BLOCK2,	STATE_BLOCK3,
    STATE_SLUMP1,	STATE_SLUMP2,	STATE_SLUMP3,
    STATE_DEATH1,	STATE_DEATH2,	STATE_DEATH3,
    STATE_HIT1,		STATE_HIT2,		STATE_HIT3,
    // Castings MUST me the last entrys!
    STATE_CAST1,	STATE_CAST2,	STATE_CAST3,    
	STATE_SUM
};

const Real RAD = 3.14159265/180.0;

////////////////////////////////////////////////////////////
// Class.
////////////////////////////////////////////////////////////
class Animate
{
  private:
    ////////////////////////////////////////////////////////////
	// Variables.
    ////////////////////////////////////////////////////////////
	int mAnimType;
	int mAnimGroup;
	bool mSpellTrigger;
	AnimationState *mAnimState;    
	AnimationState *mAnimStates[STATE_SUM];
    Real mAnimSpeed, mTurnSpeed;
	std::string mStrTemp;
    int sound_handle[STATE_SUM];
    
    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////

  public:
    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////
	 Animate(Entity *entity);
	~Animate() {;}

    bool isMovement() { return (mAnimType < STATE_ATTACK1); }
    void update(const FrameEvent& event);
    void toggleAnimGroup();
	void toggleAnimation(int animationNr, bool force = false);
	Real getAnimSpeed() { return mAnimSpeed; }
	Real getTurnSpeed() { return mTurnSpeed; }	
};

#endif
