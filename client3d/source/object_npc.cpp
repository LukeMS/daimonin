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
#include "object_manager.h"
#include "particle_manager.h"
#include "events.h"

// #define WRITE_MODELTEXTURE_TO_FILE

const Real WALK_PRECISON = 1.0;
const int TURN_SPEED    = 200;

///================================================================================================
/// Init all static Elemnts.
///================================================================================================

///================================================================================================
/// Destructor.
///================================================================================================
ObjectNPC::~ObjectNPC()
{}

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
    mAttacking = ATTACK_NONE;
    mAutoTurning= false;
    mAutoMoving = false;
    mFriendly= obj.friendly;
    mAttack  = obj.attack;
    mDefend  = obj.defend;
    mMaxHP   = obj.maxHP;
    mMaxMana = obj.maxMana;
    mMaxGrace=obj.maxGrace;
    ParticleManager::getSingleton().addNodeObject(mNode, "Particle/JoinGame", 4.8);
}

///================================================================================================
/// Move to the currently selected object.
///================================================================================================
void ObjectNPC::attackObjectOnTile(SubPos2D pos)
{}

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
        if (deltaDegree <= .5)
        {
            mAutoTurning = false;
            if (mAttacking == ATTACK_APPROACH) mAttacking = ATTACK_ANIM_START;
        }
    }
    else if (mAutoMoving)
    {
        ;
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
                    Vector3 pos = ObjectManager::getSingleton().getTargetedWorldPos();
                    Sound::getSingleton().playStream(Sound::PLAYER_HIT);
                    //ParticleManager::getSingleton().addFreeObject(pos, "Particle/Hit", 0.8);
                    mAttacking = ATTACK_NONE;
                    ObjectManager::getSingleton().targetObjectAttackPlayer();
                }
                break;

            default:
                break;
        }
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
/// Turn the Object.
///================================================================================================
void ObjectNPC::turning(Real facing)
{
    mNewFacing = Radian(facing) - Degree(90);
    mAutoTurning = true;
}

///================================================================================================
/// Turn the Object until it faces the given tile.
///================================================================================================
void ObjectNPC::faceToTile(SubPos2D pos)
{
    float deltaX = pos.x - mActPos.x;
    float deltaZ = pos.z - mActPos.z;

    /// This is the position of the player.
    if (deltaX ==0 && deltaZ ==0) return;

    mNewFacing = Radian(Math::ATan(deltaX/deltaZ));
    if      (deltaZ <0) mNewFacing+=Degree(180);
    else if (deltaX <0) mNewFacing+=Degree(360);
    mAutoTurning = true;
}

///================================================================================================
/// Move the Object to a neighbour tile.
///================================================================================================
void ObjectNPC::moveToNeighbourTile()
{
    if ((mActPos.x == mDestWalkPos.x) && (mActPos.z == mDestWalkPos.z))
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0);
        mAutoMoving = false;
        return;
    }
    mDstPos = mDestWalkPos;
    if (mDstPos.x > mActPos.x+1) mDstPos.x = mActPos.x+1;
    if (mDstPos.x < mActPos.x-1) mDstPos.x = mActPos.x-1;
    if (mDstPos.z > mActPos.z+1) mDstPos.z = mActPos.z+1;
    if (mDstPos.z < mActPos.z-1) mDstPos.z = mActPos.z-1;
    /// Turn the head into the moving direction.
    faceToTile(mDstPos);
    /// Walk 1 tile.
    mWalkToPos.x = mDstPos.x * TILE_SIZE_X + mBoundingBox.x;
    mWalkToPos.y = (Real) (TileManager::getSingleton().getAvgMapHeight(mDstPos.x, mDstPos.z) - mBoundingBox.y);
    mWalkToPos.z = mDstPos.z * TILE_SIZE_Z + mBoundingBox.z;
    mDeltaPos = mNode->getPosition() - mWalkToPos;
    if (!mIndex) Event->setWorldPos(mDeltaPos, 0, 0, CEvent::WSYNC_INIT);
}

///================================================================================================
/// Move the Object to the given tile.
///================================================================================================
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

///================================================================================================
/// Attack an enemy.
///================================================================================================
void ObjectNPC::attackShortRange(const SceneNode *node)
{
    if (mAnim->isAttack()) return; /// Finish the attack before starting a new one.
    /// Move in front of the enemy.
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

///================================================================================================
/// Add a new npc to the map.
///================================================================================================
void ObjectNPC::addToMap()
{}

