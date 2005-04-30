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

#include "object_npc.h"
#include "sound.h"
#include "option.h"
#include "logfile.h"
#include "textwindow.h"

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
int NPC::mInstanceNr = 0;

//=================================================================================================
// Init the model from the description file.
//=================================================================================================
NPC::NPC(SceneManager *SceneMgr, SceneNode *Node, const char *desc_filename)
{
    mDescFile = DIR_MODEL_DESCRIPTION;
    mDescFile += desc_filename;
	if (!mInstanceNr) { LogFile::getSingleton().Headline("Init Actor Models"); }
	LogFile::getSingleton().Info("Parse description file %s...", mDescFile.c_str());
	if (!(Option::getSingleton().openDescFile(mDescFile.c_str())))
	{
		LogFile::getSingleton().Success(false);
		LogFile::getSingleton().Error("CRITICAL: description file was not found!\n");
		return;
	}
	LogFile::getSingleton().Success(true);
	mSceneMgr = SceneMgr;     
	string strTemp;
	Option::getSingleton().getDescStr("MeshName", strTemp);
	mEntityNPC = mSceneMgr->createEntity("NPC_"+StringConverter::toString(mInstanceNr), strTemp.c_str());

	Option::getSingleton().getDescStr("StartX", strTemp);
	Real posX = atof(strTemp.c_str());
	Option::getSingleton().getDescStr("StartY", strTemp);
	Real posY = atof(strTemp.c_str());
	Option::getSingleton().getDescStr("StartZ", strTemp);
	Real posZ = atof(strTemp.c_str());
	mNode = Node->createChildSceneNode(Vector3(posX, posY, posZ), Quaternion(1.0,0.0,0.0,0.0));

	Option::getSingleton().getDescStr("Facing", strTemp);
	mFacing = Radian(atof(strTemp.c_str()));
    mNode->yaw(mFacing);

	Option::getSingleton().getDescStr("FOffset", strTemp);
	Real faceing = atof(strTemp.c_str());
	mFacingOffset = faceing * RAD;
    mNode->attachObject(mEntityNPC);

    // Create Animations and Animation sounds.
    mAnim = new Animate(mEntityNPC); // Description File must be open when you call me.

	mTurning =0;
	mWalking =0;
	mEntityWeapon =0;
	mEntityShield =0;	
	++mInstanceNr;
	return;
}

//=================================================================================================
// Select a new texture.
//=================================================================================================
void NPC::toggleTexture(int pos, int texture)
{
	string strValue , strKeyword;
	if (!(Option::getSingleton().openDescFile(mDescFile.c_str())))
	{
		LogFile::getSingleton().Success(false);
		LogFile::getSingleton().Error("NPC::toggleTexture(...) -> description file was not found!\n");
		return;
	}
	// Get material.
	strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Name";
	if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue))) { return; }
	MaterialPtr mMaterial = MaterialManager::getSingleton().getByName(strValue);
    // Get texture.
    if (texture >=0) // select a texture by value.
    {
        strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Texture_" + StringConverter::toString(texture, 2, '0');
        if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue))) { return; }
    }
    else // toggle textures
    { // only for testing...
        static int actTexture[100];
        strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Texture_" + StringConverter::toString(actTexture[pos], 2, '0');
        if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue))) 
        { 
            actTexture[pos] =0;
            strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Texture_" + StringConverter::toString(actTexture[pos], 2, '0');
            if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue))) { return; }
        }
        ++actTexture[pos];
    }
    // set new texture.
	mMaterial->unload();
	mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(strValue);
	mMaterial->reload();
}

//=================================================================================================
// Toggle npc equipment.
//=================================================================================================
void  NPC::toggleWeapon(int Hand, int WeaponNr)
{
	if (!(Option::getSingleton().openDescFile(mDescFile.c_str())))
	{
		LogFile::getSingleton().Error("CRITICAL: description file: '%s' was not found!\n", mDescFile.c_str());
		return;
	}
	static int mWeapon=0, mShield=0; // testing -> delete me!
	string mStrTemp;
	if (Hand == WEAPON_HAND)
	{
		WeaponNr = ++mWeapon; // testing -> delete me!
		if (mEntityWeapon)
		{
			mEntityNPC->detachObjectFromBone("weapon");
			mSceneMgr->removeEntity(mEntityWeapon);
			mEntityWeapon =0;
		}
		if (Option::getSingleton().getDescStr("M_Name_Weapon", mStrTemp, WeaponNr))
		{
			mEntityWeapon = mSceneMgr->createEntity("weapon", mStrTemp);
			Option::getSingleton().getDescStr("StartX_Weapon", mStrTemp, WeaponNr);
			Real posX = atof(mStrTemp.c_str());
			Option::getSingleton().getDescStr("StartY_Weapon", mStrTemp, WeaponNr);
			Real posY = atof(mStrTemp.c_str());
			Option::getSingleton().getDescStr("StartZ_Weapon", mStrTemp, WeaponNr);
			Real posZ = atof(mStrTemp.c_str());
			Option::getSingleton().getDescStr("Bone_Right_Hand", mStrTemp);
		    mEntityNPC->attachObjectToBone(mStrTemp, mEntityWeapon, Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(posX, posY, posZ));
		}
		else mWeapon =0;  // testing -> delete me!
	}
	else
	{
		WeaponNr = ++mShield; // testing -> delete me!
		if (mEntityShield)
		{
			mEntityNPC->detachObjectFromBone("shield");
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
		    mEntityNPC->attachObjectToBone(mStrTemp, mEntityShield, Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(posX, posY, posZ));
		}
		else mShield =0;  // testing -> delete me!
	}
}

//=================================================================================================
// Update npc.
//=================================================================================================
void NPC::update(const FrameEvent& event)
{
    mAnim->update(event);
	mTranslateVector = Vector3(0,0,0);
	if (mAnim->isMovement())
	{
		if (mTurning)
		{
		    mFacing += Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mTurning);
			mNode->yaw(Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mTurning));
		}
		if (mWalking)
		{
            // just a test...
			mAnim->toggleAnimation(STATE_WALK1);
		    mFacing += Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mWalking);
			mNode->yaw(Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mWalking));
		
	        mTranslateVector.y = -sin(mFacing.valueRadians()+mFacingOffset)* mAnim->getAnimSpeed() * mWalking;
		    mTranslateVector.x = -cos(mFacing.valueRadians()+mFacingOffset)* mAnim->getAnimSpeed() * mWalking;
            mTranslateVector.z =0;
            mNode->translate(mTranslateVector);
		}
		else 
		{
            mAnim->toggleAnimation(STATE_IDLE1);
        }
	}
}
