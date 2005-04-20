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
bool Player::Init(SceneManager *SceneMgr)
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
	mSceneMgr = SceneMgr;

	Option::getSingleton().getDescStr("MeshName", mStrTemp);
	mEntityPlayer = mSceneMgr->createEntity("player", mStrTemp.c_str());

	Option::getSingleton().getDescStr("StartX", mStrTemp);
	Real posX = atof(mStrTemp.c_str());
	Option::getSingleton().getDescStr("StartY", mStrTemp);
	Real posY = atof(mStrTemp.c_str());
	Option::getSingleton().getDescStr("StartZ", mStrTemp);
	Real posZ = atof(mStrTemp.c_str());
	mNode   = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(posX, posY, posZ), Quaternion(1.0,0.0,0.0,0.0));

	Option::getSingleton().getDescStr("MeshSize", mStrTemp);
	Real size = atof(mStrTemp.c_str());
	mNode->setScale(size, size, size);

	Option::getSingleton().getDescStr("Facing", mStrTemp);
	mFacing = Radian(atof(mStrTemp.c_str()));
    mNode->yaw(mFacing);

	Option::getSingleton().getDescStr("FOffset", mStrTemp);
	Real faceing = atof(mStrTemp.c_str());
	mFacingOffset = faceing * RAD;
    mNode->attachObject(mEntityPlayer);

	mTurning =0;
	mWalking =0;

	// fill the animation states.
	for (int state = 0; state < STATE_SUM; ++state)
	{
		Option::getSingleton().getDescStr(StateNames[state], mStrTemp);
		if (mStrTemp.size())
		{
			mAnimStates[state] = mEntityPlayer->getAnimationState(mStrTemp.c_str());
		}
		else // not found animation-name in description file -> use the standard one (IDLE1).
		{
			Option::getSingleton().getDescStr(StateNames[STATE_IDLE1], mStrTemp);
			mAnimStates[state] = mEntityPlayer->getAnimationState(mStrTemp.c_str());
		}
	}
	mEntityWeapon =0;
	mEntityShield =0;
	mAnimGroup = 0;
	mAnimType  =-1;
	mAnimState = mAnimStates[STATE_IDLE1];
	mAnimState->setEnabled(true);
	mAnimState->setLoop(false);
	return true;
}


//=================================================================================================
//
//=================================================================================================
void Player::toggleWeapon(int Hand, int WeaponNr)
{
	if (!(Option::getSingleton().openDescFile(FILE_PLAYER_EQUIPMENT_DESC)))
	{
		LogFile::getSingleton().Error("CRITICAL: Player-Equipment description file was not found!\n");
		return;
	}
	static int mWeapon=0, mShield=0; // testing -> delete me!

	if (Hand == WEAPON_HAND)
	{
		WeaponNr = ++mWeapon; // testing -> delete me!
		if (mEntityWeapon)
		{
			mEntityPlayer->detachObjectFromBone("weapon");
			mSceneMgr->removeEntity(mEntityWeapon);
			mEntityWeapon =0;
		}
		if (Option::getSingleton().getDescStr("M_Name_Weapon", mStrTemp, WeaponNr))
		{
			mEntityWeapon = mSceneMgr->createEntity("weapon", mStrTemp);           //    oben  links  vorne
			Option::getSingleton().getDescStr("StartX_Weapon", mStrTemp, WeaponNr);
			Real posX = atof(mStrTemp.c_str());
			Option::getSingleton().getDescStr("StartY_Weapon", mStrTemp, WeaponNr);
			Real posY = atof(mStrTemp.c_str());
			Option::getSingleton().getDescStr("StartZ_Weapon", mStrTemp, WeaponNr);
			Real posZ = atof(mStrTemp.c_str());
			Option::getSingleton().getDescStr("Bone_Right_Hand", mStrTemp);
		    mEntityPlayer->attachObjectToBone(mStrTemp, mEntityWeapon, Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(posX, posY, posZ));
		}
		else mWeapon =0;  // testing -> delete me!
	}
	else
	{
		WeaponNr = ++mShield; // testing -> delete me!
		if (mEntityShield)
		{
			mEntityPlayer->detachObjectFromBone("shield");
			mSceneMgr->removeEntity(mEntityShield);
			mEntityShield =0;
		}
		if (Option::getSingleton().getDescStr("M_Name_Shield", mStrTemp, WeaponNr))
		{
			mEntityShield = mSceneMgr->createEntity("shield", mStrTemp);           //    oben  links  vorne
			Option::getSingleton().getDescStr("StartX_Shield", mStrTemp, WeaponNr);
			Real posX = atof(mStrTemp.c_str());
			Option::getSingleton().getDescStr("StartY_Shield", mStrTemp, WeaponNr);
			Real posY = atof(mStrTemp.c_str());
			Option::getSingleton().getDescStr("StartZ_Shield", mStrTemp, WeaponNr);
			Real posZ = atof(mStrTemp.c_str());
			Option::getSingleton().getDescStr("Bone_Left_Hand", mStrTemp);
		    mEntityPlayer->attachObjectToBone(mStrTemp, mEntityShield, Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(posX, posY, posZ));
		}
		else mShield =0;  // testing -> delete me!
	}
}

//=================================================================================================
// 
//=================================================================================================
void Player::toggleAnimGroup()
{
	if (++mAnimGroup >2) { mAnimGroup =0; }
	toggleAnimation(mAnimType);
	char buf[80];
	sprintf(buf, "AnimGroup No %d is now active.", mAnimGroup+1);
	TextWin->Print(buf, TXT_WHITE);
}

//=================================================================================================
//
//=================================================================================================
void Player::toggleAnimation(int animationNr)
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
void Player::updateAnim(const FrameEvent& event)
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
	        mTranslateVector.y = -sin(mFacing.valueRadians()+mFacingOffset)* mWalking;
		    mTranslateVector.x = -cos(mFacing.valueRadians()+mFacingOffset)* mWalking;
		}
		else
		{
			if (mAnimType != STATE_IDLE1) { toggleAnimation(STATE_IDLE1); }
		}
	}
}
