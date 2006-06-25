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

    void moveToTile(int x, int z);
    void faceToTile(int x, int z);
    void attackObjectOnTile(int posX, int posZ);
    void walking(Real walk)
    {
        mWalking = walk;
    }
    void turning(Real turn)
    {
        mTurning = turn;
    }
    const Pos2D &getDestMapPos()
    {
        return mActPos;
    }
    void castSpell(int spell);
    void toggleMesh(int pos, int WeaponNr);

protected:
    Real mWalking, mTurning;
    bool mAutoTurning, mAutoMoving, mAttacking;
    Pos2D mDstPos;   /**< the destination pos in the map. **/
    int maxHealth, actHealth;
    int maxMana  , actMana;
    Vector3 mWalkToPos, mDeltaPos;
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
