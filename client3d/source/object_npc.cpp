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

#include "object_npc.h"
#include "sound.h"
#include "option.h"
#include "logger.h"
#include "spell_manager.h"
#include "gui_manager.h"
#include "object_manager.h"
#include "object_visuals.h"
#include "particle_manager.h"
#include "events.h"
#include "tile_manager.h"
#include "tile_path.h"

const int   TURN_SPEED   = 400;
const Real  WALK_PRECISON= 1.0f;
const float BIG_LAGGING  = 0.04f;
const float TIME_BEFORE_CORPSE_VANISHES = 2.0f;

//================================================================================================
// Init all static Elemnts.
//================================================================================================

//================================================================================================
// Destructor.
//================================================================================================
ObjectNPC::~ObjectNPC()
{}

//================================================================================================
// Free all recources.
//================================================================================================
void ObjectNPC::freeRecources()
{
    if (mType == ObjectManager::OBJECT_PLAYER)
    {
        mEquip->freeRecources();
        delete mEquip;
    }
}

//================================================================================================
// Init the model from the description file.
//================================================================================================
ObjectNPC::ObjectNPC(sObject &obj, bool spawn):ObjectStatic(obj)
{
    mReadyWeaponStatus = 0;
    mType    = obj.type;
    mFriendly= obj.friendly;
    mAttack  = obj.attack;
    mDefend  = obj.defend;
    mMaxHP   = obj.maxHP;
    mActHP   = obj.maxHP;
    mMaxMana = obj.maxMana;
    mActMana = obj.maxMana;
    mMaxGrace=obj.maxGrace;
    mActGrace=obj.maxGrace;
    mBoundingRadius = obj.boundingRadius;
    if (spawn)
        mSpawnSize = 0.0;
    else
        mSpawnSize = 1.0;
    mNode->setScale(mSpawnSize,mSpawnSize,mSpawnSize);
    // ////////////////////////////////////////////////////////////////////
    // Only players can change equipment.
    // ////////////////////////////////////////////////////////////////////
    if (mType == ObjectManager::OBJECT_PLAYER)
    {
        mEquip = new ObjectEquipment(mEntity);
        //mEquip->equipItem(ObjectEquipment::BONE_WEAPON_HAND, ObjectEquipment::ITEM_WEAPON, 0, -1);  // Just for test (Sword)
        //mEquip->equipItem(ObjectEquipment::BONE_SHIELD_HAND, ObjectEquipment::ITEM_WEAPON, 2, -1);  // Just for test (Bow)
        //mEquip->equipItem(ObjectEquipment::BONE_WEAPON_HAND, ObjectEquipment::ITEM_WEAPON, 0, 0);  // Just for test (Fire Sword)
        //mEquip->equipItem(ObjectEquipment::BONE_WEAPON_HAND, ObjectEquipment::ITEM_WEAPON, 0, -1);  // Just for test (Sword)
    }
    // ////////////////////////////////////////////////////////////////////
    // The first Object is our Hero.
    // ////////////////////////////////////////////////////////////////////
    if (!mIndex)
    {
        // Attach camera to players node.
        // (Players Bounding box is increased by that and cant be used for collision detection anymore)
        SceneNode *cNode = mNode->createChildSceneNode();
        cNode->attachObject(Event->getCamera());
        cNode->setInheritOrientation(false); // Camera needs no turning.
    }
    mCursorTurning =0;
    mAutoTurning = TURN_NONE;
    mAutoMoving = false;
    mEnemyObject = 0;
    mAttacking = ATTACK_NONE;
    // mNode->showBoundingBox(true); // Remove Me!!!!

    mOffX =0;
    mOffZ =0;
}

//================================================================================================
// .
//================================================================================================
void ObjectNPC::setPrimaryWeapon(int weapon)
{}

//================================================================================================
// Ready / Unready the primary weapon.
//================================================================================================
void ObjectNPC::readyPrimaryWeapon(bool ready)
{
    if (isSecondaryWeaponReady())
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ABILITY, 4, false, true, false);
        if (ready)
        {
            mReadyWeaponStatus |= READY_WEAPON_PRIMARY_TAKE;
            mReadyWeaponStatus |= READY_WEAPON_SECONDARY_DROP;
        }
        else
        {
            mReadyWeaponStatus |= READY_WEAPON_PRIMARY_DROP;
        }
    }
    else
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ABILITY, 2, false, true, false);
        if (ready)
            mReadyWeaponStatus |= READY_WEAPON_PRIMARY_TAKE;
        else
            mReadyWeaponStatus |= READY_WEAPON_PRIMARY_DROP;
    }
}

//================================================================================================
// Ready / Unready the secondary weapon.
//================================================================================================
void ObjectNPC::readySecondaryWeapon(bool ready)
{
    if (isPrimaryWeaponReady())
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ABILITY, 4, false, true, false);
        if (ready)
        {
            mReadyWeaponStatus |= READY_WEAPON_SECONDARY_TAKE;
            mReadyWeaponStatus |= READY_WEAPON_PRIMARY_DROP;
        }
        else
        {
            mReadyWeaponStatus |= READY_WEAPON_SECONDARY_DROP;
        }
    }
    else
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ABILITY, 3, false, true, false);
        //mAnim->toggleAnimation2(ObjectAnimate::ANIM_GROUP_ABILITY, 3, false, true, false);
        if (ready)
            mReadyWeaponStatus |= READY_WEAPON_SECONDARY_TAKE;
        else
            mReadyWeaponStatus |= READY_WEAPON_SECONDARY_DROP;
    }
}

//================================================================================================
// Move to the currently selected object.
//================================================================================================
void ObjectNPC::attackObjectOnTile(TilePos pos)
{}

//================================================================================================
// Move the Object to the given tile.
//================================================================================================
void ObjectNPC::moveToDistantTile(TilePos pos, int precision)
{
    if (mActPos == pos || mAutoTurning || mAutoMoving) return;
    mEnemyObject= 0; // After this move, we have to check again if enemy is in attack range.
    mDestWalkPos= pos;
    mAutoMoving = true;
    moveToNeighbourTile(precision);
}

//================================================================================================
// Move the Object to a neighbour subtile.
//================================================================================================
void ObjectNPC::moveToNeighbourTile(int precision)
{
    static TilePath tp;
    static int step = 0;

    if (!step++)
    {
        mOffX = 0;
        mOffZ = 0;
        tp.FindPath(mActPos, mDestWalkPos, precision);
    }
    // We reached the destination pos.
    if (!tp.ReadPath())
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0, true);
        mAutoMoving = false;
        step = 0;
        // For attack we dont move onto the destination tile, but some subtiles before.
        // So we have to do another faceToTile().
        if (mActPos != mDestWalkPos) faceToTile(mDestWalkPos);
        return;
    }
    mDestStepPos.x = tp.xPath;
    mDestStepPos.z = tp.yPath;
    mDestStepPos.subX = mDestStepPos.x &7;
    mDestStepPos.subZ = mDestStepPos.z &7;
    mDestStepPos.x /=8;
    mDestStepPos.z /=8;
    mDestStepPos.x += mOffX;
    mDestStepPos.z += mOffZ;

    // ////////////////////////////////////////////////////////////////////
    // If the player has moved over a tile border, we have to sync the world.
    // ////////////////////////////////////////////////////////////////////
    int dx = mActPos.x - mDestStepPos.x;
    int dz = mActPos.z - mDestStepPos.z;
    if (!mIndex && (dx|| dz))
    {
        Event->setWorldPos(dx, -dz);
        mOffX+=dx;
        mOffZ+=dz;
        mDestStepPos.x += dx;
        mDestStepPos.z += dz;
        TileManager::getSingleton().scrollMap(-dx, -dz);
        Vector3 deltaPos = ObjectManager::getSingleton().synchToWorldPos(dx, dz);
        ParticleManager::getSingleton().synchToWorldPos(deltaPos);
    }

    // Turn the head into the moving direction.
    faceToTile(mDestStepPos);
    // Walk 1 subtile.
    mDestWalkVec = TileManager::getSingleton().getTileInterface()->tileToWorldPos(mDestStepPos);
    mWalkSpeed = (mDestWalkVec - mNode->getPosition()) / mDistance;
}

//================================================================================================
// Update ObjectNPC.
// Returns false if the object needs to be deleted.
//================================================================================================
bool ObjectNPC::update(const FrameEvent& event)
{
    mAnim->update(event);
    //  Finish the current (non movement) anim first.
    //  if (!mAnim->isMovement()) return;

    // ////////////////////////////////////////////////////////////////////
    // Ready / unready weapon.
    // ////////////////////////////////////////////////////////////////////
    if (mReadyWeaponStatus > READY_WEAPON_IN_PROGRESS)
    {
        if (mAnim->getTimeLeft() < 1.0)
        {
            if (mReadyWeaponStatus & READY_WEAPON_PRIMARY_TAKE)
            {
                mEquip->equipItem(ObjectEquipment::BONE_WEAPON_HAND, 0, 0, -1);
                mReadyWeaponStatus &= ~READY_WEAPON_PRIMARY_TAKE;
                mReadyWeaponStatus |=  READY_WEAPON_PRIMARY_READY;
            }
            else if (mReadyWeaponStatus & READY_WEAPON_PRIMARY_DROP)
            {
                mEquip->dropItem(ObjectEquipment::BONE_WEAPON_HAND);
                mReadyWeaponStatus &= ~READY_WEAPON_PRIMARY_DROP;
                mReadyWeaponStatus &= ~READY_WEAPON_PRIMARY_READY;
            }
            else if (mReadyWeaponStatus & READY_WEAPON_SECONDARY_TAKE)
            {
                mEquip->equipItem(ObjectEquipment::BONE_SHIELD_HAND, 0, 2, -1);
                mReadyWeaponStatus &= ~READY_WEAPON_SECONDARY_TAKE;
                mReadyWeaponStatus |=  READY_WEAPON_SECONDARY_READY;
            }
            else if (mReadyWeaponStatus & READY_WEAPON_SECONDARY_DROP)
            {
                mEquip->dropItem(ObjectEquipment::BONE_SHIELD_HAND);
                mReadyWeaponStatus &= ~READY_WEAPON_SECONDARY_DROP;
                mReadyWeaponStatus &= ~READY_WEAPON_SECONDARY_READY;
            }

        }
    }

    // ////////////////////////////////////////////////////////////////////
    // Corpse vanishing.
    // ////////////////////////////////////////////////////////////////////
    if (mActHP <=0)
    {
        if (mIndex) // Our Hero (mIndex ==0) will never vanish.
        {
            // mDeltaDegree is not used any longer. So we can misuse it here ;)
            mDeltaDegree += event.timeSinceLastFrame;
            if (mDeltaDegree > TIME_BEFORE_CORPSE_VANISHES)
            {
                mNode->translate(Vector3(0, -event.timeSinceLastFrame, 0));
                mSpawnSize += event.timeSinceLastFrame;
                if (mSpawnSize > TIME_BEFORE_CORPSE_VANISHES*2)
                {
                    return false; // Delete this object.
                }
            }
        }
        return true;
    }
    // ////////////////////////////////////////////////////////////////////
    // Spawn effects.
    // ////////////////////////////////////////////////////////////////////
    if (mSpawnSize != 1.0)
    {
        if (mSpawnSize == 0.0)
        {
            ParticleManager::getSingleton().addNodeObject(mNode, "Particle/JoinGame", 4.8);
            mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_SPAWN, 0, false, true, false);
        }
        mSpawnSize+= event.timeSinceLastFrame;
        if (mSpawnSize > 1.0) mSpawnSize =1.0;
        mNode->setScale(mSpawnSize,mSpawnSize,mSpawnSize);
    }
    // ////////////////////////////////////////////////////////////////////
    // Auto turning.
    // ////////////////////////////////////////////////////////////////////
    bool waitForTurning= false;
    if (mAutoTurning != TURN_NONE)
    {
        Real delta = event.timeSinceLastFrame * TURN_SPEED;
        if (mAutoTurning == TURN_RIGHT)
        {
            mDeltaDegree -= delta;
            if (mDeltaDegree <=0)
            {
                delta = mDeltaDegree*-1;
                mAutoTurning = TURN_NONE;
            }
            mFacing -= Degree(delta);
            if (mFacing.valueDegrees() < 0) mFacing += Degree(360);
            mNode->yaw(Degree(-delta));
        }
        else
        {
            mDeltaDegree += delta;
            if (mDeltaDegree >=360)
            {
                delta = mDeltaDegree *-1;
                mAutoTurning = TURN_NONE;

            }
            mFacing += Degree(delta);
            if (mFacing.valueDegrees() <= 360) mFacing += Degree(360);
            mNode->yaw(Degree(delta));
        }
        // First turn and then start to walk (on big delta facing).
        if (Math::Abs(mDeltaDegree > 90))
        {
            waitForTurning = true;
            //mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0, true);
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Turning by cursor keys.
    // ////////////////////////////////////////////////////////////////////
    if (mAnim->isIdle() && mCursorTurning)
    {
        mEnemyObject = 0; // We are no longer looking at the enemy.
        mNode->yaw(Degree(event.timeSinceLastFrame * TURN_SPEED * mCursorTurning));
        mFacing += Degree(event.timeSinceLastFrame * TURN_SPEED * mCursorTurning);
        if (mFacing.valueDegrees() >= 360) mFacing -= Degree(360);
        if (mFacing.valueDegrees() <    0) mFacing += Degree(360);
    }

    // ////////////////////////////////////////////////////////////////////
    // Auto movement.
    // ////////////////////////////////////////////////////////////////////
    if (mAutoMoving && !waitForTurning)
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_WALK, 0, true, true);
        // On slow systems it will happen that distance goes negative.
        // So we look if the squared length is bigger than the previous squared length.
        static float squaredLengthPrev = 1000.0; // Somthing big.
        Vector3 dist = mDestWalkVec - mNode->getPosition();
        float squaredLength = dist.squaredLength();
        // We have reached a waypoint || we went too far.
        if (squaredLength < WALK_PRECISON || squaredLength > squaredLengthPrev)
        {
            mActPos = mDestStepPos;
            mNode->setPosition(mDestWalkVec); // Set the exact position.
            squaredLengthPrev = 1000.0;       // Somthing big.
            moveToNeighbourTile(0);           // Move to next waypoint.
        }
        // We have to move on.
        else
        {
            // On slow system (or big lag) we move instantly to the destination.
            if (event.timeSinceLastFrame  >= BIG_LAGGING)
                mNode->setPosition(mDestWalkVec);
            // No lagging, move smoothly.
            else
                mNode->translate(event.timeSinceLastFrame * mWalkSpeed);
            squaredLengthPrev = squaredLengthPrev;
        }
        return true;
    }

    // ////////////////////////////////////////////////////////////////////
    // Attacking.
    // ////////////////////////////////////////////////////////////////////
    if (mAttacking != ATTACK_NONE)
    {
        switch (mAttacking)
        {
            case ATTACK_APPROACH:
                if (mAutoTurning == TURN_NONE && !mAutoMoving)
                    mAttacking = ATTACK_ANIM_START;
                break;
            case ATTACK_ANIM_START:
                mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ATTACK, 0);
                mAttacking = ATTACK_ANIM_RUNNUNG;
                break;

            case ATTACK_ANIM_RUNNUNG:
                if (mAnim->getTimeLeft() < 0.5 || mAnim->isIdle())
                {
                    // Just a quick hack to get some action on the screen...
                    if (mAnim->isAttack())
                    {
                        // Player is getting hurt.
                        if (mIndex)
                        {
                            ObjectNPC *mob = ObjectManager::getSingleton().getObjectNPC(HERO);
                            static int oo =0;
                            if (++oo <= 3)
                            {
                                mob->setDamage(3);
                                mob->mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_HIT, 0);
                                Sound::getSingleton().playStream(Sound::MALE_HIT_01);
                            }
                            if (++oo > 5) oo =0;
                        }
                        // Monster is getting hurt.
                        else
                        {
                            ObjectNPC *mob = ObjectManager::getSingleton().getSelectedNPC();
                            if (!mob || mob->getHealth() <= 0) break;
                            mob->setDamage(10);
                            //mob->mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_HIT, 0);
                            Sound::getSingleton().playStream(Sound::TENTACLE_HIT);
                            ParticleManager::getSingleton().addFreeObject(mob->getSceneNode()->getPosition(), "Particle/Hit", 0.8);
                        }
                    }
                    mAttacking = ATTACK_NONE;
                    if (!mIndex) ObjectManager::getSingleton().targetObjectAttackNPC(HERO);
                }
                break;

            default:
                break;
        }
    }
    return true;
}

//================================================================================================
// Move to a new tile pos.
//================================================================================================
void ObjectNPC::movePosition(int deltaX, int deltaZ)
{
    mActPos.x += deltaX;
    mActPos.z += deltaZ;
    setPosition(mActPos);
    if (mActHP <=0) mNode->translate(Vector3(0, -mSpawnSize, 0));
}

//================================================================================================
// Add damage to an object.
//================================================================================================
void ObjectNPC::setDamage(int damage)
{
    if (mActHP <=0) return;

    mActHP-= damage;
    Real health = getHealthPercentage();
    if (!mIndex)
    {
        GuiManager::getSingleton().sendMessage(GUI_WIN_PLAYERCONSOLE, GUI_MSG_BAR_CHANGED,
                                               GUI_STATUSBAR_PLAYER_HEALTH , (void*)&health);
    }
    else
    {
        ObjectVisuals::getSingleton().setLifebar(health);
    }
    if (mActHP <= 0)
    {
        mAttacking = ATTACK_NONE;
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_DEATH, 0);
        // mDeltaDegree is not used any longer. So we can misuse it here ;)
        mDeltaDegree = 0.0;
        mSpawnSize = 0.0;
        Sound::getSingleton().playStream(Sound::MALE_BOUNTY_01);
    }
}

//================================================================================================
// Cast a spell.
//================================================================================================
void ObjectNPC::castSpell(int spell)
{
    //  if (!askServer.AllowedToCast(spell)) return;
    SpellManager::getSingleton().addObject(spell, mIndex);
}

//================================================================================================
// Turn the Object.
//================================================================================================
void ObjectNPC::turning(Real facing, bool cursorTurn)
{
    if (cursorTurn)
    {
        mCursorTurning = facing;
    }
    else
    {
        mDeltaDegree = mFacing.valueDegrees() - facing;
        // We want a range from 0...359�.
        if      (mDeltaDegree <   0) mDeltaDegree += 360;
        else if (mDeltaDegree >=360) mDeltaDegree -= 360;
        if (mDeltaDegree < 180)
            mAutoTurning = TURN_RIGHT;
        else
            mAutoTurning = TURN_LEFT;
    }
}

//================================================================================================
// Turn the Object until it faces the given tile.
//================================================================================================
void ObjectNPC::faceToTile(TilePos pos)
{
    float deltaZ = (pos.z - mActPos.z) * SUM_SUBTILES  +  (pos.subZ - mActPos.subZ);
    float deltaX = (pos.x - mActPos.x) * SUM_SUBTILES  +  (pos.subX - mActPos.subX);

    // We need the Distance (in sub-Tiles) for a constant walk speed.
    mDistance = Math::Abs(deltaZ) > Math::Abs(deltaX)?Math::Abs(deltaZ)/8:Math::Abs(deltaX)/8;

    if (!deltaZ)
    {
        if (!deltaX) return; // Already standing on this position.
        if      (deltaX >0) mDeltaDegree = mFacing.valueDegrees() - 90;
        else if (deltaX <0) mDeltaDegree = mFacing.valueDegrees() -270;
    }
    else
    {
        mDeltaDegree = mFacing.valueDegrees() - (Math::ATan(deltaX/deltaZ)).valueDegrees();
    }
    if (deltaZ <0) mDeltaDegree -= 180;

    // We want a range from 0...359�.
    while (mDeltaDegree <   0) mDeltaDegree += 360;
    while (mDeltaDegree >=360) mDeltaDegree -= 360;

    // Do we need to turn ?
    if (mDeltaDegree >= 1 && mDeltaDegree <= 359)
    {
        if (mDeltaDegree < 180)
            mAutoTurning = TURN_RIGHT;
        else
            mAutoTurning = TURN_LEFT;
    }
}

//================================================================================================
// Attack an enemy.
//================================================================================================
void ObjectNPC::attackShortRange(ObjectNPC *EnemyObject)
{
    if (!mAnim->isIdle() || !EnemyObject) return;
    if (this == EnemyObject) return; // No Harakiri! (this is not needed, if hero is ALWAYS friendly).
    // Move in front of the enemy.
    if (!isPrimaryWeaponReady())
    {
        readyPrimaryWeapon(true);
        return;
    }
    if (mEnemyObject != EnemyObject)
    {
        mEnemyObject = EnemyObject;
        moveToDistantTile(mEnemyObject->getTilePos(), mEnemyObject->getBoundingRadius());
        mAttacking = ATTACK_APPROACH;
    }
    // Enemy is already in attack range.
    else
    {
        mAttacking = ATTACK_ANIM_START;
    }
}

//================================================================================================
// Add a new npc to the map.
//================================================================================================
void ObjectNPC::addToMap()
{}
