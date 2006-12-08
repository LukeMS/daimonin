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

#ifndef ObjectNPC_H
#define ObjectNPC_H

#include "define.h"
#include "object_static.h"
#include "object_animate.h"
#include "object_equipment.h"

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
        HERO /**< ME (mIndex == 0) is our Hero. **/
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
    virtual bool update(const FrameEvent& event);
    void movePosition(int dx, int dz);
    void moveToDistantTile(TilePos pos, int precision =0);
    void faceToTile(TilePos pos);
    void turning(Real turn, bool cursorTurn);
    void attackObjectOnTile(TilePos pos);
    void addToMap();
    void setEnemy();
    const unsigned char getBoundingRadius() const
    {
        return mBoundingRadius;
    }
    int getHealth()
    {
        return mActHP;
    }
    Real getHealthPercentage()
    {
        return Real(mActHP) / Real(mMaxHP);
    }
    bool isMoving()
    {
        return mAutoMoving;
    }
    void setDamage(int hp);
    void attackShortRange(ObjectNPC *mEnemyObject);
    void castSpell(int spell);
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
    const TilePos &getDestMapPos() const
    {
        return mDestStepPos;
    }

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
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
        ATTACK_ANIM_RUNNUNG,
        ATTACK_ANIM_STOP,
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

    unsigned char mBoundingRadius; /**< The radius of subtiles, the NPC stands on. Used for pathfinding. **/
    Real mCursorTurning;
    bool mAutoMoving;
    bool mTalking;
    Real mSpawnSize;
    int mReadyWeaponStatus;
    int mType;
    int mAttack;
    int mDefend;
    int mMaxHP,    mActHP;
    int mMaxMana,  mActMana;
    int mMaxGrace, mActGrace;
    TilePos mDestStepPos;   /**< The next tile pos of a multi tile walk. **/
    TilePos mDestWalkPos;   /**< The destination pos (as tile).   **/
    Vector3  mDestWalkVec;  /**< The destination pos (as vector). **/
    Vector3  mWalkSpeed;
    int mOffX, mOffZ;
    ObjectNPC *mEnemyObject;
    Real mDeltaDegree, mDistance;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectNPC(const ObjectNPC&); // disable copy-constructor.
};

#endif
