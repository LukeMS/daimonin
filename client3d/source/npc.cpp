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

#include "npc.h"
#include "player.h"
#include "sound.h"
#include "option.h"
#include "logfile.h"
#include "textwindow.h"

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
int NPC::mInstanceNr = 0;

extern const char *StateNames[STATE_SUM];
NPC *NPC_Enemy1 = new NPC;

//=================================================================================================
// 
//=================================================================================================
bool NPC::Init(SceneManager *mSceneMgr, SceneNode  *Node)
{
	if (!mInstanceNr) { LogFile::getSingleton().Headline("Init NPCs"); }
	LogFile::getSingleton().Info("Parse description file %s\n", FILE_NPC_DESC);
	if (!(Option::getSingleton().openDescFile(FILE_NPC_DESC)))
	{
		LogFile::getSingleton().Success(false);
		LogFile::getSingleton().Error("CRITICAL: NPC description file was not found!\n");
		return (false);
	}
	LogFile::getSingleton().Success(true);

	string strTemp;
	Option::getSingleton().getDescStr("MeshName", strTemp);
	mEntity = mSceneMgr->createEntity("NPC_"+StringConverter::toString(mInstanceNr), strTemp.c_str());

	Option::getSingleton().getDescStr("StartX", strTemp);
	Real posX = atof(strTemp.c_str());
	Option::getSingleton().getDescStr("StartY", strTemp);
	Real posY = atof(strTemp.c_str());
	Option::getSingleton().getDescStr("StartZ", strTemp);
	Real posZ = atof(strTemp.c_str());
	mNode   = Node->createChildSceneNode(Vector3(posX, posY, posZ));

	Option::getSingleton().getDescStr("Facing", strTemp);
	mFacing = Degree(atof(strTemp.c_str()));

	mAnimGroup =0;
	mAnimType =-1;

	Option::getSingleton().getDescStr("FOffset", strTemp);
	Real faceing = atof(strTemp.c_str());
	mFacingOffset = faceing * RAD;
    mNode->attachObject(mEntity);

	Option::getSingleton().getDescStr("MeshSize", strTemp);
	Real size = atof(strTemp.c_str());
	mNode->setScale(size, size, size);
    mNode->yaw(mFacing);

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
	++mInstanceNr;
	return true;
}

//=================================================================================================
// 
//=================================================================================================
void NPC::toggleAnimaGroup()
{
	if (++mAnimGroup >2) { mAnimGroup =0; }
	char buf[80];
	sprintf(buf, "AnimGroup No %d is now active.", mAnimGroup+1);
	TextWin->Print(buf, TXT_WHITE);
}

//=================================================================================================
// 
//=================================================================================================
void NPC::updateAnim(const FrameEvent& event)
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
          	mAnimState->setLoop(true);
			mAnimState= mAnimStates[STATE_IDLE1];
  		    mFacing += Degree(mTurning);
			mNode->yaw(Radian(Degree(mTurning)));
		}
		if (mWalking)
		{
            mAnimState->setLoop(true);
			mAnimState= mAnimStates[STATE_WALK1];
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
