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
NPC *NPC_Enemy1 = 0;

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
	mNode = Node->createChildSceneNode(Vector3(posX, posY, posZ), Quaternion(1.0,0.0,0.0,0.0));

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
	++mInstanceNr;
	mAnimType  =-1;
	mAnimState = mAnimStates[STATE_IDLE1];
	mAnimState->setEnabled(true);
	mAnimState->setLoop(false);
	return true;
}

//=================================================================================================
//
//=================================================================================================
void NPC::updateTexture(int textureNr)
{
	string strTemp;
	Option::getSingleton().getDescStr("Material", strTemp);
	MaterialPtr mMaterial = MaterialManager::getSingleton().getByName(strTemp);
	if (!textureNr) Option::getSingleton().getDescStr("Tex_001", strTemp);
	else            Option::getSingleton().getDescStr("Tex_000", strTemp);
	mMaterial->unload();
	mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(strTemp);
	mMaterial->reload();
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
void NPC::toggleAnimation(int animationNr)
{
	if (mAnimType >= STATE_ATTACK1 && animationNr >= STATE_ATTACK1) { return; }
	mAnimType = animationNr;
	mAnimState->setEnabled(false);
	mAnimState = mAnimStates[mAnimType + mAnimGroup];
	mAnimState->setEnabled(true);
	mAnimState->setTimePosition(0);
	mAnimState->setLoop(false);
}

//=================================================================================================
//
//=================================================================================================
void NPC::updateAnim(const FrameEvent& event)
{
	mAnimState = mAnimStates[mAnimType + mAnimGroup];
	mAnimState->addTime(event.timeSinceLastFrame * PLAYER_ANIM_SPEED);
	if (mAnimState->getTimePosition() >= mAnimState->getLength())
	{
		toggleAnimation(STATE_IDLE1);
		mAnimState->setTimePosition(0);
	}
	mAnimState->setEnabled(true);
	mTranslateVector = Vector3(0,0,0);
	if (mAnimType < STATE_ATTACK1)
	{
		if (mTurning)
		{
		    mFacing += Radian(event.timeSinceLastFrame *mTurning);
			mNode->yaw(Radian(event.timeSinceLastFrame *mTurning));
		}
		if (mWalking)
		{
			if (mAnimType != STATE_WALK1) { toggleAnimation(STATE_WALK1); }
	        mTranslateVector.z =  sin(mFacing.valueRadians()+mFacingOffset)* mWalking;
		    mTranslateVector.x = -cos(mFacing.valueRadians()+mFacingOffset)* mWalking;
		}
		else
		{
			if (mAnimType != STATE_IDLE1) { toggleAnimation(STATE_IDLE1); }
		}
	}
}
