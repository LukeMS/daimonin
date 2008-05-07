/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
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

using namespace Ogre;

const int   WALK_SPEED   =  70;
const int   TURN_SPEED   = 400;
const Real  WALK_PRECISON= 1.0f;
const float BIG_LAGGING  = 0.04f;
const float TIME_BEFORE_CORPSE_VANISHES = 2.0f;
const Real  STATUS_BAR_FILLTIME = 1.2;  // Time to draw the statusbar from 0 to 100%.

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
    mDrawnHP = obj.maxHP;
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
        cNode->attachObject(Events::getSingleton().getCamera());
        cNode->setInheritOrientation(false); // Camera needs no turning.
    }
    // ////////////////////////////////////////////////////////////////////
    // Attach the blob shadow to the npc.
    // ////////////////////////////////////////////////////////////////////
    ManualObject* blob = static_cast<ManualObject*>(Events::getSingleton().GetSceneManager()->createMovableObject("Mob_"+ StringConverter::toString(mIndex, 10, '0'), ManualObjectFactory::FACTORY_TYPE_NAME));
    blob->begin("Material_blob_shadow");
    const AxisAlignedBox &AABB = mEntity->getBoundingBox();
    float sizeX = (AABB.getMaximum().x -AABB.getMinimum().x);
    float sizeY = 0.5;
    float sizeZ = (AABB.getMaximum().z -AABB.getMinimum().z);
    if (sizeX < sizeZ) sizeX = sizeZ;
    blob->position(-sizeX, sizeY,  sizeX); blob->normal(0,0,1); blob->textureCoord(0.0, 0.0);
    blob->position( sizeX, sizeY,  sizeX); blob->normal(0,0,1); blob->textureCoord(0.0, 1.0);
    blob->position(-sizeX, sizeY, -sizeX); blob->normal(0,0,1); blob->textureCoord(1.0, 0.0);
    blob->position( sizeX, sizeY, -sizeX); blob->normal(0,0,1); blob->textureCoord(1.0, 1.0);
    blob->triangle(0, 1, 2);
    blob->triangle(3, 2, 1);
    blob->end();
    blob->convertToMesh("Blob_"+ StringConverter::toString(mIndex, 10, '0'));
    blob->setQueryFlags(ObjectManager::QUERY_NPC_SELECT_MASK);
    blob->setRenderQueueGroup(RENDER_QUEUE_6);
    mNode->attachObject(blob);

    mCursorTurning =0;
    mCursorWalking =0;
    mAutoTurning = TURN_NONE;
    mAutoMoving = false;
    mEnemyObject = 0;
    mAttacking = ATTACK_NONE;
    // mNode->showBoundingBox(true); // Remove Me!!!!
    mOffX =0;
    mOffZ =0;
}

//================================================================================================
// Moving by cursor keys..
//================================================================================================
void ObjectNPC::moveByCursor(Ogre::Real dTime)
{
    static Vector3 oldPos = mTilePos;
    Real distance = WALK_SPEED * dTime;
    mTilePos.x+= Math::Sin(Degree(mFacing)) * distance;
    mTilePos.z+= Math::Cos(Degree(mFacing)) * distance;
    mTilePos.y = TileManager::getSingleton().getTileHeight((int)mTilePos.x, (int)mTilePos.z);
    int dx = (int)(oldPos.x - mTilePos.x) / (TileManager::TILE_SIZE*2);
    int dz = (int)(oldPos.z - mTilePos.z) / (TileManager::TILE_SIZE*2);
    // Player moved over a tile border.
    if (!mIndex && (dx || dz))
    {
        mTilePos.x+= dx * TileManager::TILE_SIZE*2;
        mTilePos.z+= dz * TileManager::TILE_SIZE*2;
        TileManager::getSingleton().scrollMap(dx, dz);
        ObjectManager::getSingleton().synchToWorldPos(dx*2, dz*2);
        oldPos = mTilePos;
        //ParticleManager::getSingleton().syncToWorldPos(deltaPos);
    }
    mNode->setPosition(mTilePos);
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
    else if (isPrimaryWeaponReady() != ready)
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
// Update ObjectNPC.
// Returns false if the object needs to be deleted.
//================================================================================================
bool ObjectNPC::update(const FrameEvent& event)
{
    mAnim->update(event);
    //  Finish the current (non movement) anim first.
    //  if (!mAnim->isMovement()) return;
    // ////////////////////////////////////////////////////////////////////
    // Draw the Stats.
    // Status changes are drawn step by step just because it looks better.
    // ////////////////////////////////////////////////////////////////////
    if (mDrawnHP != mActHP)
    {
        if (mDrawnHP < mActHP)
        {
            mDrawnHP += ((event.timeSinceLastFrame/STATUS_BAR_FILLTIME) * mMaxHP);
            if (mDrawnHP > mActHP) mDrawnHP = mActHP;
        }
        else
        {
            mDrawnHP -= ((event.timeSinceLastFrame/STATUS_BAR_FILLTIME) * mMaxHP);
            if (mDrawnHP < mActHP) mDrawnHP = mActHP;
        }
        Real health = Ogre::Real(mDrawnHP) / Ogre::Real(mMaxHP);
        if (!mIndex)
        {
            GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_PLAYERCONSOLE, GuiManager::GUI_MSG_BAR_CHANGED,
                                                   GuiImageset::GUI_STATUSBAR_PLAYER_HEALTH , (void*)&health);
        }
        else
        {
            ObjectVisuals::getSingleton().setLifebar(health);
        }
    }
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
    if (mCursorTurning)
    {
        mEnemyObject = 0; // We are no longer looking at the enemy.
        mNode->yaw(Degree(event.timeSinceLastFrame * TURN_SPEED * mCursorTurning));
        mFacing += Degree(event.timeSinceLastFrame * TURN_SPEED * mCursorTurning);
        if (mFacing.valueDegrees() >= 360) mFacing -= Degree(360);
        if (mFacing.valueDegrees() <    0) mFacing += Degree(360);
    }
    // ////////////////////////////////////////////////////////////////////
    // Walking by cursor keys.
    // ////////////////////////////////////////////////////////////////////
    if (mCursorWalking)
    {
        mEnemyObject = 0; // We are no longer looking at the enemy.
        moveByCursor(event.timeSinceLastFrame * mCursorWalking);
    }

    // ////////////////////////////////////////////////////////////////////
    // Auto movement.
    // ////////////////////////////////////////////////////////////////////
    if (mAutoMoving && !waitForTurning)
    {
        /*
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_WALK, 0, true, true);
        // On slow systems it will happen that distance goes negative.
        // So we look if the squared length is bigger than the previous squared length.
        static float squaredLengthPrev = 1000.0; // Somthing big.
        Vector3 dist = mDestWalkVec - mNode->getPosition();
        float squaredLength = dist.squaredLength();
        // We have reached a waypoint || we went too far.
        if (squaredLength < WALK_PRECISON || squaredLength > squaredLengthPrev)
        {
            mActTilePos = mDestStepPos;
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
        */
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
                if (isSecondaryWeaponReady())
                    mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ATTACK, 6);
                else
                    mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ATTACK, 0);
                mAttacking = ATTACK_ANIM_RUNNING;
                break;

            case ATTACK_ANIM_RUNNING:
                if (mAnim->getTimeLeft() < 0.5 || mAnim->isIdle())
                    mAttacking = ATTACK_ANIM_ENDS;
                break;

            case ATTACK_ANIM_ENDS:
                if (!isSecondaryWeaponReady())
                {
                    mAttacking = ATTACK_CALC_DAMAGE;
                    break;
                }
                ObjectManager::getSingleton().shoot(ObjectManager::MISSLE_ARROW,
                                                    ObjectManager::getSingleton().getObjectNPC(HERO),
                                                    ObjectManager::getSingleton().getSelectedNPC());
                mAttacking = ATTACK_WAIT_FOR_MISSLE;
                break;

            case ATTACK_WAIT_FOR_MISSLE:
                mAttacking = ATTACK_CALC_DAMAGE;
                break;

            case ATTACK_CALC_DAMAGE:
                // Just a quick hack to get some action on the screen...
                if (mAnim->isAttack())
                {
                    // Player is getting hurt.
                    if (mIndex)
                    {
                        // Quick hack. Later we will check the distance to enmey here...
                        if (!READY_WEAPON_SECONDARY_READY)
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
bool ObjectNPC::movePosition(int deltaX, int deltaZ)
{
    mTilePos.x += deltaX * TileManager::TILE_SIZE;
    mTilePos.z += deltaZ * TileManager::TILE_SIZE;
    setPosition(mTilePos);
    if (mActHP <=0) mNode->translate(Vector3(0, -mSpawnSize, 0));
    // if (pos out of playfield) return false;
    return true;
}

//================================================================================================
// Add damage to an object.
//================================================================================================
void ObjectNPC::setDamage(int damage)
{
    if (mActHP <=0) return;
    mDrawnHP = mActHP; /**< The health shown in the statusbar. **/
    mActHP-= damage;
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
// Walk by cursor keys..
//================================================================================================
void ObjectNPC::walking(Ogre::Real dir, bool cursorWalk)
{
    if (cursorWalk)
    {
        mCursorWalking = dir;
        if (dir)
            mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_WALK, 0, true, true);
        else
            mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0, true);
    }
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
        // We want a range from 0...359°.
        while (mDeltaDegree <   0) mDeltaDegree += 360;
        while (mDeltaDegree >=360) mDeltaDegree -= 360;
        //Logger::log().error() << "mDeltaDegree: " << mDeltaDegree;
        // Do we need to turn ?
        if (mDeltaDegree >= 1 && mDeltaDegree <= 358)
        {
            if (mDeltaDegree < 180)
                mAutoTurning = TURN_RIGHT;
            else
                mAutoTurning = TURN_LEFT;
        }
        else
        {
            mAutoTurning = TURN_NONE;
        }
    }
}

//================================================================================================
// Turn the Object until it faces the given tile.
//================================================================================================
void ObjectNPC::faceToTile(Vector3 pos)
{
    /*
    float deltaZ = (pos.z - mActTilePos.z) * TileManager::SUM_SUBTILES  +  (pos.subZ - mActTilePos.subZ);
    float deltaX = (pos.x - mActTilePos.x) * TileManager::SUM_SUBTILES  +  (pos.subX - mActTilePos.subX);

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

    // We want a range from 0...359°.
    while (mDeltaDegree <   0) mDeltaDegree += 360;
    while (mDeltaDegree >=360) mDeltaDegree -= 360;

    // Do we need to turn ?
    if (mDeltaDegree >= 1 && mDeltaDegree <= 358)
    {
        if (mDeltaDegree < 180)
            mAutoTurning = TURN_RIGHT;
        else
            mAutoTurning = TURN_LEFT;
    }
    else
    {
        mAutoTurning = TURN_NONE;
    }
    */
}

//================================================================================================
// Attack an enemy with a short range weapon.
//================================================================================================
void ObjectNPC::attackShortRange(ObjectNPC *EnemyObject)
{
    if (!mAnim->isIdle() || !EnemyObject) return;
    if (this == EnemyObject) return; // No Harakiri! (this is not needed, if hero is ALWAYS friendly).
    // Ready the weapon.
    if (!isPrimaryWeaponReady())
    {
        readyPrimaryWeapon(true);
        return;
    }
    // Move in front of the enemy.
    if (mEnemyObject != EnemyObject)
    {
        mEnemyObject = EnemyObject;
    //    moveToDistantTile(mEnemyObject->getTilePos(), mEnemyObject->getBoundingRadius());
        mAttacking = ATTACK_APPROACH;
    }
    // Enemy is already in attack range.
    else
    {
        mAttacking = ATTACK_ANIM_START;
    }
}

//================================================================================================
// Attack an enemy with a long range weapon.
//================================================================================================
void ObjectNPC::attackLongRange(ObjectNPC *EnemyObject)
{
    if (!mAnim->isIdle() || !EnemyObject) return;
    if (this == EnemyObject) return; // No Harakiri! (this is not needed, if hero is ALWAYS friendly).
    // Ready the weapon.
    if (!isSecondaryWeaponReady())
    {
        readySecondaryWeapon(true);
        return;
    }
    // Move in front of the enemy.
    if (mEnemyObject != EnemyObject)
    {
        mEnemyObject = EnemyObject;
//        const int WEAPON_RANGE = 16; // subtiles range of the weapon
//        moveToDistantTile(mEnemyObject->getTilePos(), WEAPON_RANGE);
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
