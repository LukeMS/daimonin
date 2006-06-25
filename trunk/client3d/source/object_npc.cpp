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

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "particle_manager.h"
#include "object_npc.h"
#include "sound.h"
#include "option.h"
#include "logger.h"
#include "spell_manager.h"
#include "events.h"

// #define WRITE_MODELTEXTURE_TO_FILE

///================================================================================================
/// Init all static Elemnts.
///================================================================================================
const Real WALK_PRECISON = 1.0;
const int TURN_SPEED    = 200;

///================================================================================================
/// Destructor.
///================================================================================================
ObjectNPC::~ObjectNPC()
{
}

///================================================================================================
/// Free all recources.
///================================================================================================
void ObjectNPC::freeRecources()
{
    if (mAnim) delete mAnim;
}

///================================================================================================
/// Init the model from the description file.
///================================================================================================
ObjectNPC::ObjectNPC(sObject &obj):ObjectStatic(obj)
{
    mEntity->setQueryFlags(QUERY_NPC_MASK);
    mAutoTurning= false;
    mAutoMoving = false;
    mTurning =0;
    mWalking =0;
    mFriendly= obj.friendly;
    mAttack  = obj.attack;
    mDefend  = obj.defend;
    mMaxHP   = obj.maxHP;
    mMaxMana = obj.maxMana;
    mMaxGrace=obj.maxGrace;

    //mNode->showBoundingBox(true); // Remove Me!!!!
}

///================================================================================================
/// Move to the currently selected object.
///================================================================================================
void ObjectNPC::attackObjectOnTile(int posX, int posZ)
{
}

///================================================================================================
/// Toggle ObjectNPC equipment.
///================================================================================================
void  ObjectNPC::toggleMesh(int Bone, int WeaponNr)
{
/*
    if (!(Option::getSingleton().openDescFile(mDescFile.c_str())))
    {
        Logger::log().error()  << "CRITICAL: description file: '" << mDescFile << "' was not found!\n";
        return;
    }
    static int mWeapon=0, mShield=0, mHelmet=0, mArmor =0; // testing -> delete me!
    string mStrTemp;

    switch (Bone)
    {
        case BONE_WEAPON_HAND:
        WeaponNr = ++mWeapon; // testing -> delete me!
        if (mEntityWeapon)
        {
            mEntityNPC->detachObjectFromBone("weapon");
            mSceneMgr->destroyEntity(mEntityWeapon);
            mEntityWeapon =0;
        }
        if (Option::getSingleton().getDescStr("M_Name_Weapon", mStrTemp, WeaponNr))
        {
            mEntityWeapon = mSceneMgr->createEntity("weapon", mStrTemp);
            mEntityWeapon->setQueryFlags(QUERY_EQUIPMENT_MASK);
            Option::getSingleton().getDescStr("Bone_Right_Hand", mStrTemp);
//                    const AxisAlignedBox &AABB = mEntityWeapon->getBoundingBox();
//                    Vector3 pos = -(AABB.getMaximum() + AABB.getMinimum())/2;
//                    mEntityNPC->attachObjectToBone(mStrTemp, mEntityWeapon, Quaternion(1.0, 0.0, 0.0, 0.0), pos);
            mEntityNPC->attachObjectToBone(mStrTemp, mEntityWeapon);
            static ParticleSystem *pSystem;
            if (WeaponNr==1)
            {
                pSystem = ParticleManager::getSingleton().addBoneObject(mEntityNPC, mStrTemp.c_str(), "Particle/SwordGlow", -1);
            }
            if (WeaponNr==2)
            {
                ParticleManager::getSingleton().delObject(pSystem);
            }
        }
        else mWeapon =0;  // testing -> delete me!
        break;

        case BONE_SHIELD_HAND:
        {
            WeaponNr = ++mShield; // testing -> delete me!
            if (mEntityShield)
            {
                mEntityNPC->detachObjectFromBone("shield");
                mSceneMgr->destroyEntity(mEntityShield);
                mEntityShield =0;
            }
            if (Option::getSingleton().getDescStr("M_Name_Shield", mStrTemp, WeaponNr))
            {
                mEntityShield = mSceneMgr->createEntity("shield", mStrTemp);
                mEntityShield->setQueryFlags(QUERY_EQUIPMENT_MASK);
                const AxisAlignedBox &AABB = mEntityShield->getBoundingBox();
                Vector3 pos = -(AABB.getMaximum() + AABB.getMinimum())/2;
                Option::getSingleton().getDescStr("Bone_Left_Hand", mStrTemp);
                mEntityNPC->attachObjectToBone(mStrTemp, mEntityShield, Quaternion(1.0, 0.0, 0.0, 0.0), pos);
            }
            else mShield =0;  // testing -> delete me!
        }
        break;

        case BONE_HEAD:
        {
//                    { // delete me!
//                      if (WeaponNr) mNode->scale(1.1, 1.1, 1.1);
//                      else          mNode->scale(0.9, 0.9, 0.9);
//                      Vector3 sc = mNode->getScale();
//                      std::string scale= "model size: " + StringConverter::toString(sc.x);
//                      GuiManager::getSingleton().sendMessage(
//                        GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*) scale.c_str());
//                      return;
//                    }

            WeaponNr = ++mHelmet; // testing -> delete me!
            if (mEntityHelmet)
            {
                mEntityNPC->detachObjectFromBone("helmet");
                mSceneMgr->destroyEntity(mEntityHelmet);
                mEntityHelmet =0;
            }
            if (Option::getSingleton().getDescStr("M_Name_Helmet", mStrTemp, WeaponNr))
            {
                mEntityHelmet = mSceneMgr->createEntity("helmet", mStrTemp);
                mEntityHelmet->setQueryFlags(QUERY_EQUIPMENT_MASK);
                Option::getSingleton().getDescStr("Bone_Head", mStrTemp);
//                          const AxisAlignedBox &AABB = mEntityHelmet->getBoundingBox();
//                          Vector3 pos = -(AABB.getMaximum() + AABB.getMinimum())/2;
//                          mEntityNPC->attachObjectToBone(mStrTemp, mEntityHelmet, Quaternion(1.0, 0.0, 0.0, 0.0), pos);
                mEntityNPC->attachObjectToBone(mStrTemp, mEntityHelmet);
            }
            else mHelmet =0;  // testing -> delete me!
        }
        break;

        case BONE_BODY:
        {
            WeaponNr = ++mArmor; // testing -> delete me!
            if (mEntityArmor)
            {
                mEntityNPC->detachObjectFromBone("armor");
                mSceneMgr->destroyEntity(mEntityArmor);
                mEntityArmor =0;
            }
            if (Option::getSingleton().getDescStr("M_Name_Armor", mStrTemp, WeaponNr))
            {
                mEntityArmor =mSceneMgr->createEntity("armor", mStrTemp);
                mEntityArmor->setQueryFlags(QUERY_EQUIPMENT_MASK);
                const AxisAlignedBox &AABB = mEntityArmor->getBoundingBox();
                Option::getSingleton().getDescStr("Bone_Body", mStrTemp);

                Vector3 pos = -(AABB.getMaximum() + AABB.getMinimum())/2;

//                        Option::getSingleton().getDescStr("Bone_Head", mStrTemp);
//                        Option::getSingleton().getDescStr("StartX_Armor", mStrTemp, WeaponNr);
//                        Real posX = atof(mStrTemp.c_str());
//                        Option::getSingleton().getDescStr("StartY_Armor", mStrTemp, WeaponNr);
//                        Real posY = atof(mStrTemp.c_str());
//                        Option::getSingleton().getDescStr("StartZ_Armor", mStrTemp, WeaponNr);
//                        Real posZ = atof(mStrTemp.c_str());
                mEntityNPC->attachObjectToBone(mStrTemp, mEntityArmor, Quaternion::IDENTITY, pos);
            }
            else mArmor =0;  // testing -> delete me!
        }
        break;
    }
*/
}

///================================================================================================
/// Update ObjectNPC.
///================================================================================================
void ObjectNPC::update(const FrameEvent& event)
{
    mAnim->update(event);
    ///  Finish the current (non movement) anim first.
    if (!mAnim->isMovement()) return;

    mTranslateVector = Vector3(0,0,0);
    if (mFacing.valueDegrees() >= 360) mFacing -= Degree(360);
    if (mFacing.valueDegrees() <    0) mFacing += Degree(360);

    if (mAutoTurning)
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0);
        int turningDirection;
        int deltaDegree = ((int)mFacing.valueDegrees() - (int)mNewFacing.valueDegrees());
        if (deltaDegree <   0) deltaDegree += 360;
        if (deltaDegree < 180) turningDirection = -1; else turningDirection = 1;
        mFacing += Degree(event.timeSinceLastFrame * TURN_SPEED * turningDirection);
        mNode->yaw(Degree(event.timeSinceLastFrame * TURN_SPEED * turningDirection));
        /// Are we facing into the right direction (+/- 1 degree)?
        if (deltaDegree <= .5) mAutoTurning = false;
    }
    else if (mAutoMoving)
    {
        /// We are very close to destination.
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_WALK, 0);
        Vector3 dist = mWalkToPos - mNode->getPosition();
        dist.y =0;
        if(dist.squaredLength() < WALK_PRECISON)
        {
            /// Set the exact destination pos.
            mWalkToPos.x = mBoundingBox.x + mActPos.x * TILE_SIZE_X;
            mWalkToPos.z = mBoundingBox.z + mActPos.z * TILE_SIZE_Z;
            mWalkToPos.y = TileManager::getSingleton().getAvgMapHeight(mDstPos.x, mDstPos.z) - mBoundingBox.y;
            mNode->setPosition(mWalkToPos);
            mAutoMoving = false;
            mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0);
        }
        else
        {
            /// We have to move on.
            Vector3 NewTmpPosition = - event.timeSinceLastFrame * mDeltaPos;;
            //ParticleManager::getSingleton().pauseAll(true);
            mNode->setPosition(mNode->getPosition() + NewTmpPosition);
            //ParticleManager::getSingleton().pauseAll(false);
        }
        return;
    }
    if (mAnim->isMovement() && mTurning)
    {
        mFacing += Degree(event.timeSinceLastFrame * TURN_SPEED * mTurning);
        mNode->yaw(Degree(event.timeSinceLastFrame * TURN_SPEED * mTurning));
    }
}

///================================================================================================
/// Cast a spell.
///================================================================================================
void ObjectNPC::castSpell(int spell)
{
    //  if (!askServer.AllowedToCast(spell)) return;
    SpellManager::getSingleton().addObject(spell, mIndex);
}

///================================================================================================
/// Turn the player until it faces the given tile.
///================================================================================================
void ObjectNPC::faceToTile(int x, int z)
{
    float deltaX = x - mActPos.x;
    float deltaZ = z - mActPos.z;

    /// This is the position of the player.
    if (deltaX ==0 && deltaZ ==0) return;

    mNewFacing = Radian(Math::ATan(deltaX/deltaZ));
    if      (deltaZ <0) mNewFacing+=Degree(180);
    else if (deltaX <0) mNewFacing+=Degree(360);
    mAutoTurning = true;
}

///================================================================================================
/// Move the player to the given tile.
///================================================================================================
void ObjectNPC::moveToTile(int x, int z)
{
    if(mActPos.x == x && mActPos.z == z || mAutoTurning || mAutoMoving) return;

    /// Split into waypoints (distance = 1 tile)
    // todo

    // testing: limit the moving distance.
    if (x > mActPos.x+1) x = mActPos.x+1;
    if (x < mActPos.x-1) x = mActPos.x-1;
    if (z > mActPos.z+1) z = mActPos.z+1;
    if (z < mActPos.z-1) z = mActPos.z-1;

    /// Turn the head into the moving direction.
    faceToTile(x, z);
    /// Move it.
    mWalkToPos.x = x * TILE_SIZE_X + mBoundingBox.x;
    mWalkToPos.y = (Real) (TileManager::getSingleton().getAvgMapHeight(x, z) - mBoundingBox.y);
    mWalkToPos.z = z * TILE_SIZE_Z + mBoundingBox.z;
    mDeltaPos = mNode->getPosition() - mWalkToPos;
    mDstPos.x = x;
    mDstPos.z = z;
    mAutoMoving = true;
}
