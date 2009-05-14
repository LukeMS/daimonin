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

#ifndef ObjectNPC_H
#define ObjectNPC_H

#include "define.h"
#include "object_static.h"
#include "object_animate.h"
#include "object_equipment.h"

/**
 ** This is an extended object class.
 ** It handles additional functions like movement and fighting.
 *****************************************************************************/
class ObjectNPC : public ObjectStatic
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        short w, h;             /**< width and height of the image. **/
        short dstX, dstY;       /**< pos of the image in the model-texture. **/
        short srcX, srcY;       /**< pos of the image in the race-template-texture. **/
        short offsetX, offsetY; /**< offset for the next source image. **/
    }
    sPicture;
    enum
    {
        HERO /**< HERO (mIndex == 0) is our Hero. **/
    };
    enum
    {
        TEXTURE_POS_SKIN, TEXTURE_POS_FACE, TEXTURE_POS_HAIR,
        TEXTURE_POS_LEGS, TEXTURE_POS_BODY,
        TEXTURE_POS_BELT, TEXTURE_POS_SHOES, TEXTURE_POS_HANDS
    };

    class ObjectEquipment *mEquip;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectNPC(sObject &obj, bool spawn);
    virtual ~ObjectNPC();
    virtual void freeRecources();
    virtual bool update(const Ogre::FrameEvent& event);
    bool movePosition(int dx, int dz);
    void faceToTile(Ogre::Vector3 pos);
    void turning(Ogre::Real turn, bool cursorTurn);
    void walking(Ogre::Real turn, bool cursorWalk);
    //void attackObjectOnTile(TilePosOLD pos);
    void addToMap();
    void setEnemy();
    const Ogre::uchar getBoundingRadius() const
    {
        return mBoundingRadius;
    }
    int getHealth()
    {
        return mActHP;
    }
    Ogre::Real getHealthPercentage()
    {
        return Ogre::Real(mActHP) / Ogre::Real(mMaxHP);
    }
    bool isMoving()
    {
        return mAutoMoving;
    }
    void setDamage(int hp);
    void attackShortRange(ObjectNPC *mEnemyObject);
    void attackLongRange(ObjectNPC *mEnemyObject);
    void castSpell(int spell);
    void shoot(int missle, ObjectNPC *srcMob, ObjectNPC *dstMob);
    void stopMovement();
    void talkToNpc();
    void setPrimaryWeapon(int weapon);
    void readyPrimaryWeapon(bool ready);
    void readySecondaryWeapon(bool ready);
    bool isPrimaryWeaponReady()
    {
        return (mReadyWeaponStatus & READY_WEAPON_PRIMARY_READY) >0;
    }
    bool isSecondaryWeaponReady()
    {
        return (mReadyWeaponStatus & READY_WEAPON_SECONDARY_READY) >0;
    }
    void attack()
    {
        mAttacking = ATTACK_APPROACH;
    }
    void moveToNeighbourTile(int precision =0);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        ATTACK_NONE,
        ATTACK_APPROACH,
        ATTACK_ANIM_START,
        ATTACK_ANIM_RUNNING,
        ATTACK_ANIM_ENDS,
        ATTACK_ANIM_STOP,
        ATTACK_WAIT_FOR_MISSLE,
        ATTACK_CALC_DAMAGE,
        ATTACK_SUM
    }
    mAttacking;
    enum
    {
        TURN_NONE,
        TURN_RIGHT,
        TURN_LEFT
    }mAutoTurning;
    enum
    {
        READY_WEAPON_PRIMARY_READY  = 1 << 0, /**< Primary weapom IS in hand. **/
        READY_WEAPON_SECONDARY_READY= 1 << 1, /**< Secondary weapom IS in hand. **/
        READY_WEAPON_IN_PROGRESS    = 1 << 2, /**< Dummy. **/
        READY_WEAPON_PRIMARY_TAKE   = 1 << 3, /**< Prepare to ready the primary weapon. **/
        READY_WEAPON_PRIMARY_DROP   = 1 << 4, /**< Prepare to unready the primary weapon. **/
        READY_WEAPON_SECONDARY_TAKE = 1 << 5, /**< Prepare to ready the secondary weapon. **/
        READY_WEAPON_SECONDARY_DROP = 1 << 6, /**< Prepare to unready the secondare weapon. **/
    };

    Ogre::uchar mBoundingRadius; /**< The radius of subtiles, the NPC stands on. Used for pathfinding. **/
    bool mAutoMoving;
    bool mTalking;
    void moveByCursor(Ogre::Real dTime);
    Ogre::Real mSpawnSize;
    Ogre::Real mCursorTurning, mCursorWalking;
    Ogre::Real mDeltaDegree, mDistance;
    int mReadyWeaponStatus;
    int mType;
    int mAttack;
    int mDefend;
    int mMaxHP, mActHP;
    int mMaxMana,  mActMana;
    int mMaxGrace, mActGrace;
    int mOffX, mOffZ;
    ObjectNPC *mEnemyObject;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectNPC(const ObjectNPC&); // disable copy-constructor.
};

#endif
