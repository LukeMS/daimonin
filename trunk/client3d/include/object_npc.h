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

using namespace Ogre;

/// Defines:
/// Mob => (M)oveable (Ob)ject.

class ObjectNPC : public ObjectStatic
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectNPC(sObject &obj);
    virtual ~ObjectNPC();
    virtual void freeRecources();
    virtual void update(const FrameEvent& event);
    void moveToDistantTile(SubPos2D pos);
    void faceToTile(SubPos2D pos);
    void turning(Real turn);
    void attackObjectOnTile(SubPos2D pos);
    void addToMap();
    void setEnemy();
    void attackShortRange(const SceneNode *node);
    void attack()
    {
        mAttacking = ATTACK_APPROACH;
    }

    const SubPos2D &getDestMapPos()
    {
        return mDstPos;
    }
    void castSpell(int spell);

protected:
    enum
    {
        ATTACK_NONE,
        ATTACK_APPROACH,
        ATTACK_ANIM_START,
        ATTACK_ANIM_RUNNUNG,
        ATTACK_ANIM_STOP,
        ATTACK_CALC_DAMAGE,
        ATTACK_SUM
    }mAttacking;

    SubPos2D mDestWalkPos;
    SceneNode *mEnemyNode;
    bool mAutoTurning;
    bool mAutoMoving;
    bool mTalking;
    int maxHealth, actHealth;
    int maxMana  , actMana;
    SubPos2D mDstPos;   /**< the destination pos in the map. **/
    Vector3 mWalkToPos, mDeltaPos;

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    void moveToNeighbourTile();

private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    int mAttack;
    int mDefend;
    int mMaxHP,    mActHP;
    int mMaxMana,  mActMana;
    int mMaxGrace, mActGrace;
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectNPC(const ObjectNPC&); // disable copy-constructor.
};

#endif
