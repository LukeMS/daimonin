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

bool Player::Init(SceneManager *mSceneMgr)
{
	LogFile::getSingelton().Headline("Init Player");
	LogFile::getSingelton().Info("Parse description file...");
	if (!(Option::getSingelton().openDescFile("./media/models/player.desc")))
	{
		LogFile::getSingelton().Success(false);
		LogFile::getSingelton().Error("CRITICAL: Player description file was not found!\n");		
		return (false);
	}
	LogFile::getSingelton().Success(true);	

	string strTemp;
	Option::getSingelton().getDescStr("MeshName", strTemp);
	mEntity = mSceneMgr->createEntity("player", strTemp.c_str());

	Option::getSingelton().getDescStr("SeaLevel", strTemp);
	Real posY = atof(strTemp.c_str());
	mNode   = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, posY, 0));
	mFacing = Degree(0);

	mAnimGroup =0;
	mAnimType =-1;

	Option::getSingelton().getDescStr("Facing", strTemp);
	Real faceing = atof(strTemp.c_str());	
	mFacingOffset = faceing * RAD;
    mNode->attachObject(mEntity);

	Option::getSingelton().getDescStr("MeshSize", strTemp);
	Real size = atof(strTemp.c_str());	
	mNode->setScale(size, size, size);

    mNode->yaw(Radian(Degree(mFacing)));

	for (int state = 0; state < STATE_SUM; ++state)
	{
		Option::getSingelton().getDescStr(StateNames[state], strTemp);
		if (strTemp.size())
		{
			mAnimStates[state] = mEntity->getAnimationState(strTemp.c_str());
		}
		else
		{
			Option::getSingelton().getDescStr(StateNames[STATE_IDLE1], strTemp);
			mAnimStates[state] = mEntity->getAnimationState(strTemp.c_str());
		}
	}
	return true;
}

void Player::toggleAnimaGroup()
{
	if (++mAnimGroup >2) { mAnimGroup =0; }
	char buf[80];
	sprintf(buf, "AnimGroup No %d is now active.", mAnimGroup+1);
	TextWin->Print(buf, ColourValue::White);
}

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
		    mFacing += Degree(mTurning);
			mNode->yaw(Radian(Degree(mTurning)));
		}
		if (mWalking)
		{
	        mTranslateVector.z =  sin(mFacing.valueRadians()+mFacingOffset)* mWalking;
		    mTranslateVector.x = -cos(mFacing.valueRadians()+mFacingOffset)* mWalking;
			mAnimState= mAnimStates[STATE_WALK1];
			mAnimState->addTime(event.timeSinceLastFrame * mWalking);
		}
		else 
		{
			mTranslateVector = Vector3::ZERO;
			mAnimState= mAnimStates[STATE_IDLE1];
			mAnimState->addTime(event.timeSinceLastFrame * PLAYER_ANIM_SPEED);
		}
	}
	mAnimState->setEnabled(true);
}
