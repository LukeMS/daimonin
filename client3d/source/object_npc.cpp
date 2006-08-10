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

const Real WALK_PRECISON = 1.0;
const int TURN_SPEED    = 200;

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
        Equip->freeRecources();
        delete Equip;
    }
}

//================================================================================================
// Init the model from the description file.
//================================================================================================
ObjectNPC::ObjectNPC(sObject &obj, bool spawn):ObjectStatic(obj)
{
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
    mEntity->setQueryFlags(QUERY_NPC_MASK);
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
        Equip = new ObjectEquipment(mEntity);
        Equip->equipItem(0, 0, 0, -1);  // Just for test
    }
    // ////////////////////////////////////////////////////////////////////
    // The first Object is our Hero.
    // ////////////////////////////////////////////////////////////////////
    if (!mIndex)
    {
        Vector3 pos = mNode->getPosition();
        Event->setWorldPos(pos, 0, 0, CEvent::WSYNC_MOVE);
    }
    mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0, true);
    mCursorTurning =0;
    mAutoTurning = TURN_NONE;
    mAutoMoving = false;
    mEnemyNode = 0;
    mAttacking = ATTACK_NONE;
    // mNode->showBoundingBox(true); // Remove Me!!!!
}

//================================================================================================
// Is player currently moving?
//================================================================================================
bool ObjectNPC::isMoving()
{
    return mAutoMoving;
}

//================================================================================================
// Move to the currently selected object.
//================================================================================================
void ObjectNPC::attackObjectOnTile(SubPos2D pos)
{}

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
void ObjectNPC::faceToTile(SubPos2D pos)
{
    float deltaZ = pos.z - mActPos.z;
    float deltaX = pos.x - mActPos.x;
    if (!deltaZ)
    {
        if (!deltaX) return; // Already facing this direction.
        if      (deltaX >0) mDeltaDegree = mFacing.valueDegrees() - 90;
        else if (deltaX <0) mDeltaDegree = mFacing.valueDegrees() -270;
    }
    else
    {
        mDeltaDegree = mFacing.valueDegrees() - (Math::ATan(deltaX/deltaZ)).valueDegrees();
    }
    if (deltaZ <0) mDeltaDegree -= 180;
    // We want a range from 0...359�.
    if      (mDeltaDegree <   0) mDeltaDegree += 360;
    else if (mDeltaDegree >=360) mDeltaDegree -= 360;
    if (mDeltaDegree < 180)
        mAutoTurning = TURN_RIGHT;
    else
        mAutoTurning = TURN_LEFT;
}

//================================================================================================
// Update ObjectNPC.
//================================================================================================
void ObjectNPC::update(const FrameEvent& event)
{
    mAnim->update(event);
    if (mActHP <0) return;
    //  Finish the current (non movement) anim first.
    //  if (!mAnim->isMovement()) return;

    if (mSpawnSize != 1.0)
    {
        if (mSpawnSize == 0.0)
        {
            ParticleManager::getSingleton().addNodeObject(mNode, "Particle/JoinGame", 4.8);
            mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_SPAWN, 0, false, true);
        }
        mSpawnSize+= event.timeSinceLastFrame;
        if (mSpawnSize > 1.0) mSpawnSize =1.0;
        mNode->setScale(mSpawnSize,mSpawnSize,mSpawnSize);
    }

    if (mAutoTurning)
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0, true);
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
        // After the turning is complete, we can attack.
        if (!mAutoTurning && mAttacking == ATTACK_APPROACH)
            mAttacking = ATTACK_ANIM_START;
    }
    else if (mAutoMoving)
    {
        // We are very close to destination.
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_WALK, 0, true);
        Vector3 dist = mWalkToPos - mNode->getPosition();
        dist.y =0;
        if(dist.squaredLength() < WALK_PRECISON)
        {
            // Set the exact destination pos.
            mWalkToPos.x = mBoundingBox.x + mActPos.x * TILE_SIZE_X;
            mWalkToPos.z = mBoundingBox.z + mActPos.z * TILE_SIZE_Z;
            mWalkToPos.y = TileManager::getSingleton().getAvgMapHeight(mDstPos.x, mDstPos.z) - mBoundingBox.y;
            mNode->setPosition(mWalkToPos);
            if (!mIndex) Event->setWorldPos(mWalkToPos, mActPos.x - mDstPos.x, mActPos.z - mDstPos.z, CEvent::WSYNC_MOVE);
            mDestWalkPos.x+= mActPos.x - mDstPos.x;
            mDestWalkPos.z+= mActPos.z - mDstPos.z;
            moveToNeighbourTile();
            if (mAttacking == ATTACK_APPROACH) mAttacking = ATTACK_ANIM_START;
        }
        else
        {
            // We have to move on.
            Vector3 NewTmpPosition = - event.timeSinceLastFrame * mDeltaPos;
            mNode->setPosition(mNode->getPosition() + NewTmpPosition);
            if (!mIndex) Event->setWorldPos(NewTmpPosition, 0, 0, CEvent::WSYNC_OFFSET);
            if (mEnemyNode)
            {
                AxisAlignedBox aabb = mNode->_getWorldAABB().intersection(mEnemyNode->_getWorldAABB());
                if (!aabb.isNull())
                {
                    if (mAttacking == ATTACK_APPROACH) mAttacking = ATTACK_ANIM_START;
                    mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0, true);
                    mAutoMoving = false;
                }
            }
        }
        return;
    }
    else if (mAttacking != ATTACK_NONE)
    {
        switch (mAttacking)
        {
            case ATTACK_ANIM_START:
                mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ATTACK, 0);
                mAttacking = ATTACK_ANIM_RUNNUNG;
                break;

            case ATTACK_ANIM_RUNNUNG:
                if (mAnim->getTimeLeft() < 0.5 || mAnim->isIdle())
                {
                    // Just a test...
                    if (mAnim->isAttack())
                    {
                        if (mIndex)
                        {
                            ObjectNPC *mob = ObjectManager::getSingleton().getObjectNPC(0);
                            static int oo =0;
                            if (++oo <=3)
                            {
                                mob->setDamage(3);
                                mob->mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_HIT, 0);
                                Sound::getSingleton().playStream(Sound::MALE_HIT_01);
                            }
                            if (++oo > 5) oo =0;
                        }
                        else
                        {
                            ObjectNPC *mob = ObjectManager::getSingleton().getObjectNPC(1);
                            mob->setDamage(10);
                            if (mob->getHealth() <= 0) break;
                            //mob->mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_HIT, 0);
                            Sound::getSingleton().playStream(Sound::TENTACLE_HIT);
                            ParticleManager::getSingleton().addFreeObject(mob->getNode()->getPosition(), "Particle/Hit", 0.8);
                        }
                    }
                    mAttacking = ATTACK_NONE;
                    if (!mIndex) ObjectManager::getSingleton().targetObjectAttackPlayer();
                }
                break;

            default:
                break;
        }
    }
    // Turning by cursor keys.
    if (mAnim->isIdle() && mCursorTurning)
    {
        mEnemyNode = 0; // We are no longer looking at the enemy.
        mNode->yaw(Degree(event.timeSinceLastFrame * TURN_SPEED * mCursorTurning));
        mFacing += Degree(event.timeSinceLastFrame * TURN_SPEED * mCursorTurning);
        if (mFacing.valueDegrees() >= 360) mFacing -= Degree(360);
        if (mFacing.valueDegrees() <    0) mFacing += Degree(360);
    }
}

//================================================================================================
// Add damage to an object.
//================================================================================================
void ObjectNPC::setDamage(int damage)
{
    if (mActHP <0) return;
    mActHP-= damage;
    Real health = (Real)(mActHP) / Real(mMaxHP);
    if (!mIndex)
    {
        GuiManager::getSingleton().sendMessage(GUI_WIN_PLAYERINFO, GUI_MSG_BAR_CHANGED,
                                               GUI_STATUSBAR_PLAYER_HEALTH , (void*)&health);
    }
    else
    {
        ObjectVisuals::getSingleton().setLifebar(health);
    }
    if (mActHP < 0)
    {
        mAttacking = ATTACK_NONE;
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_DEATH, 0);
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
// Move the Object to a neighbour tile.
//================================================================================================
void ObjectNPC::moveToNeighbourTile()
{
    if ((mActPos.x == mDestWalkPos.x) && (mActPos.z == mDestWalkPos.z))
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0, true);
        mAutoMoving = false;
        return;
    }
    mDstPos = mDestWalkPos;
    if (mDstPos.x > mActPos.x+1) mDstPos.x = mActPos.x+1;
    if (mDstPos.x < mActPos.x-1) mDstPos.x = mActPos.x-1;
    if (mDstPos.z > mActPos.z+1) mDstPos.z = mActPos.z+1;
    if (mDstPos.z < mActPos.z-1) mDstPos.z = mActPos.z-1;
    // Turn the head into the moving direction.
    faceToTile(mDstPos);
    // Walk 1 tile.
    mWalkToPos.x = mDstPos.x * TILE_SIZE_X + mBoundingBox.x;
    mWalkToPos.y = (Real) (TileManager::getSingleton().getAvgMapHeight(mDstPos.x, mDstPos.z) - mBoundingBox.y);
    mWalkToPos.z = mDstPos.z * TILE_SIZE_Z + mBoundingBox.z;
    mDeltaPos = mNode->getPosition() - mWalkToPos;
    if (!mIndex) Event->setWorldPos(mDeltaPos, 0, 0, CEvent::WSYNC_INIT);
}

//================================================================================================
// Move the Object to the given tile.
//================================================================================================
void ObjectNPC::moveToDistantTile(SubPos2D pos)
{
    if(mActPos.x == pos.x && mActPos.z == pos.z || mAutoTurning || mAutoMoving) return;
    if (mEnemyNode)
    {
        mWalkToPos.x = mBoundingBox.x + mActPos.x * TILE_SIZE_X;
        mWalkToPos.z = mBoundingBox.z + mActPos.z * TILE_SIZE_Z;
        mWalkToPos.y = TileManager::getSingleton().getAvgMapHeight(mDstPos.x, mDstPos.z) - mBoundingBox.y;
        mNode->setPosition(mWalkToPos);
        if (!mIndex) Event->setWorldPos(mWalkToPos, mActPos.x - mDstPos.x, mActPos.z - mDstPos.z, CEvent::WSYNC_MOVE);
        mEnemyNode = 0;
    }
    mDestWalkPos = pos;
    mAutoMoving = true;
    moveToNeighbourTile();
}

//================================================================================================
// Attack an enemy.
//================================================================================================
void ObjectNPC::attackShortRange(const SceneNode *node)
{
    if (mAnim->isAttack()) return; // Finish the attack before starting a new one.
    // Move in front of the enemy.
    if (mEnemyNode != node)
    {
        moveToDistantTile(ObjectManager::getSingleton().getTargetedPos());
        mAttacking = ATTACK_APPROACH;
    }
    else
    {
        mAttacking = ATTACK_ANIM_START;
    }
    mEnemyNode = (SceneNode*) node;
}

//================================================================================================
// Add a new npc to the map.
//================================================================================================
void ObjectNPC::addToMap()
{}

