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
#include "tile_pos.h"
#include "object_animate.h"

using namespace Ogre;

class ObjectStatic
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        int type;                     /**< Type: e.g. static, npc, ... **/
        String nickName;              /**< Ingame-Name. **/
        String meshName;              /**< Name of the ogre3d mesh. **/
        int particleNr;               /**< Number of the particle effect. **/
        unsigned int index;           /**< Unique number for this object. **/
        TilePos pos;                 /**< Tile-pos. **/
        unsigned char boundingRadius; /**< The radius of subtiles, the NPC stands on. **/
        int level;                    /**< Floor-level. **/
        char walkable[8];             /**< 8x8 bit for the walkable status of a tile. **/
        Real facing;
        int friendly;
        int attack;
        int defend;
        int maxHP;
        int maxMana;
        int maxGrace;
    }
    sObject;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectStatic(sObject &obj);
    virtual ~ObjectStatic();
    virtual void freeRecources();
    virtual bool update(const FrameEvent& event);
    void movePosition(int dx, int dz);
    void move(Vector3 &pos);
    const Vector3 &getPosition()
    {
        return mNode->getPosition();
    }
    SceneNode *getSceneNode()
    {
        return mNode;
    }
    Real getFacing()
    {
        return mFacing.valueDegrees();
    }
    void toggleAnimation(int animGroup, int animNr)
    {
        mAnim->toggleAnimation(animGroup, animNr);
    }
    const String &getNickName()
    {
        return mNickName;
    }
    void setNickName(String name)
    {
        mNickName = name;
    }
    int getFriendly()
    {
        return mFriendly;
    }
    Entity *getEntity()
    {
        return mEntity;
    }
    TilePos getTilePos()
    {
        return mActPos;
    }
    void setPosition(TilePos pos);
    void activate(bool waitForHero = true);
    unsigned int getIndex()
    {
        return mIndex;
    }
    Real getHeight()
    {
        return mBoundingBox.y;
    }
protected:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static SceneManager *mSceneMgr;
    Vector3 mBoundingBox;
    ObjectAnimate *mAnim;
    Degree mFacing;
    int mFriendly;
    unsigned int mIndex;
    SceneNode *mNode;
    Entity *mEntity;
    TilePos mActPos;   /**< the actual pos in the map. **/
    String mNickName;
    int mFloor;
    bool mWaitForHero;

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { ACTION_NONE, ACTION_OPEN, ACTION_CLOSE };
    bool mOpen;
    int  mAction;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectStatic(const ObjectStatic&); // disable copy-constructor.
};

#endif
