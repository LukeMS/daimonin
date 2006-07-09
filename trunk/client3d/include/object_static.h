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

#ifndef OBJ_STATIC_H
#define OBJ_STATIC_H

#include <Ogre.h>
#include "define.h"
#include "object_animate.h"

using namespace Ogre;

class ObjectStatic
{
public:
    enum
    {
        RACE_HUMAN_MALE,
        RACE_HUMAN_FEMALE,
        RACE_ELF_MALE,
        RACE_ELF_FEMALE,
        RACE_DWARF_MALE,
        RACE_DWARF_FEMALE,
        RACE_TENTACLE,
        RACE_SUM
    }Race;

    typedef struct
    {
        int type;               /**< Type: e.g. static, npc, ... **/
        int Race;
        String nickName;        /**< Ingame-Name **/
        String meshName;
        int particleNr;
        unsigned int index;
        int posX, posY;         /**< Tile-pos. **/
        int level;              /**< Floor-level. **/
        int centred;            /**< Is the object centred on the tile? **/
        Real facing;
        int friendly;
        int attack;
        int defend;
        int maxHP;
        int maxMana;
        int maxGrace;
    }
    sObject;

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectStatic(sObject &obj);
    virtual ~ObjectStatic();
    virtual void freeRecources();
    virtual void update(const FrameEvent& event);

    void move(Vector3 &pos);
    const Vector3 &getPos()
    {
        return mNode->getPosition();
    }
    const Vector3 &getWorldPos()
    {
        return mTranslateVector;
    }
    const SceneNode *getNode()
    {
        return mNode;
    }
    Real getFacing()
    {
        return mFacing.valueRadians();
    }
    void toggleAnimation(int animGroup, int animNr)
    {
        mAnim->toggleAnimation(animGroup, animNr);
    }
    const String &getNickName()
    {
        return mNickName;
    }
    const int getFriendly()
    {
        return mFriendly;
    }
    SubPos2D getTilePos()
    {
        return mActPos;
    }
    SubPos2D getTileScrollPos();

protected:
    static SceneManager *mSceneMgr;
    Vector3 mTranslateVector, mBoundingBox;
    ObjectAnimate *mAnim;
    Degree mFacing, mNewFacing;
    int mFriendly;
    unsigned int mIndex;
    SceneNode *mNode;
    Entity *mEntity;
    SubPos2D mActPos;   /**< the actual pos in the map. **/
    String mNickName;
    int mFloor;
    int mCentred;

private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectStatic(const ObjectStatic&); // disable copy-constructor.
};

#endif
