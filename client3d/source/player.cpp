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

#include "sound.h"
#include "player.h"
#include "option.h"
#include "logfile.h"
#include "textwindow.h"

const char *StateNames[STATE_SUM]=
{
	"Idle1",	"Idle2",	"Idle3",
	"Walk1",	"Walk2",	"Walk3",
	"Run1",		"Run2",		"Run3",
	"Attack1",	"Attack2",	"Attack3",
	"Block1",	"Block2",	"Block3",
	"Slump1",	"Slump2",	"Slump3",
	"Death1",	"Death2",	"Death3",
	"Hit1",		"Hit2",		"Hit3"
};

//=================================================================================================
// 
//=================================================================================================
bool Player::Init(SceneManager *mSceneMgr)
{
	LogFile::getSingleton().Headline("Init Player");
	LogFile::getSingleton().Info("Parse description file %s\n", FILE_PLAYER_DESC);
	if (!(Option::getSingleton().openDescFile(FILE_PLAYER_DESC)))
	{
		LogFile::getSingleton().Success(false);
		LogFile::getSingleton().Error("CRITICAL: Player description file was not found!\n");
		return (false);
	}
	LogFile::getSingleton().Success(true);

	string strTemp;
	Option::getSingleton().getDescStr("MeshName", strTemp);
	mEntity = mSceneMgr->createEntity("player", strTemp.c_str());

	Option::getSingleton().getDescStr("StartX", strTemp);
	Real posX = atof(strTemp.c_str());
	Option::getSingleton().getDescStr("StartY", strTemp);
	Real posY = atof(strTemp.c_str());
	Option::getSingleton().getDescStr("StartZ", strTemp);
	Real posZ = atof(strTemp.c_str());
	mNode   = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(posX, posY, posZ), Quaternion(1.0,0.0,0.0,0.0));

	Option::getSingleton().getDescStr("MeshSize", strTemp);
	Real size = atof(strTemp.c_str());
	mNode->setScale(size, size, size);

	Option::getSingleton().getDescStr("Facing", strTemp);
	mFacing = Radian(atof(strTemp.c_str()));
    mNode->yaw(mFacing);

	Option::getSingleton().getDescStr("FOffset", strTemp);
	Real faceing = atof(strTemp.c_str());
	mFacingOffset = faceing * RAD;
    mNode->attachObject(mEntity);

	mAnimGroup =0;
	mAnimType =-1;
	mTurning =0;
	mWalking =0;

	for (int state = 0; state < STATE_SUM; ++state)
	{
		Option::getSingleton().getDescStr(StateNames[state], strTemp);
		if (strTemp.size())
		{
			mAnimStates[state] = mEntity->getAnimationState(strTemp.c_str());
		}
		else
		{
			Option::getSingleton().getDescStr(StateNames[STATE_IDLE1], strTemp);
			mAnimStates[state] = mEntity->getAnimationState(strTemp.c_str());
		}
	}
	return true;
}

//=================================================================================================
// 
//=================================================================================================
void Player::toggleAnimaGroup()
{
	if (++mAnimGroup >2) { mAnimGroup =0; }
	char buf[80];
	sprintf(buf, "AnimGroup No %d is now active.", mAnimGroup+1);
	TextWin->Print(buf, TXT_WHITE);
}

//=================================================================================================
// 
//=================================================================================================
void Player::updateAnim(const FrameEvent& event)
{
	if (mAnimType >= STATE_ATTACK1)
	{
		mAnimState->setLoop(false);
		mAnimState= mAnimStates[mAnimType+mAnimGroup];
		mAnimState->addTime(event.timeSinceLastFrame * PLAYER_ANIM_SPEED);
		if (mAnimState->getTimePosition() >= mAnimState->getLength())
		{
			mAnimType =-1;
			mAnimState->setTimePosition(0.0f);
		}
	}
	else
	{
		if (mTurning)
		{
			mAnimState= mAnimStates[STATE_IDLE1];
          	mAnimState->setLoop(true);
  		    mFacing += Radian(event.timeSinceLastFrame *mTurning);
			mNode->yaw(Radian(event.timeSinceLastFrame *mTurning));
		}
		if (mWalking)
		{
			mAnimState= mAnimStates[STATE_WALK1];
            mAnimState->setLoop(true);
	        mTranslateVector.z =  sin(mFacing.valueRadians()+mFacingOffset)* mWalking;
		    mTranslateVector.x = -cos(mFacing.valueRadians()+mFacingOffset)* mWalking;
			mAnimState->addTime(event.timeSinceLastFrame * mWalking);
		}
		else
		{
			mTranslateVector = Vector3(0,0,0);
			mAnimState= mAnimStates[STATE_IDLE1];
			mAnimState->addTime(event.timeSinceLastFrame * PLAYER_ANIM_SPEED);
		}
	}
	mAnimState->setEnabled(true);
}
