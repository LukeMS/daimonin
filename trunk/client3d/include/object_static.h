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

#ifndef OBJ_STATIC_H
#define OBJ_STATIC_H

#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include "object_animate.h"

/**
 ** This is the basic object class.
 ** It handles static (=non moveable) objects like environment stuff.
 ** @todo Static objects support particle effects.
 *****************************************************************************/
class ObjectStatic
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        Ogre::String nickName;      /**< Ingame-Name. **/
        Ogre::String meshName;      /**< Name of the ogre3d mesh. **/
        Ogre::Vector3 pos;          /**< Tile-pos. **/
        Ogre::uchar boundingRadius; /**< The radius of subtiles, the NPC stands on. **/
        Ogre::Real facing;
        unsigned int index;         /**< Unique number for this object. **/
        int type;                   /**< Type: e.g. static, npc, ... **/
        int particleNr;             /**< Number of the particle effect. **/
        int level;                  /**< Floor-level. **/
        int friendly;
        int attack;
        int defend;
        int maxHP;
        int maxMana;
        int maxGrace;
        //char walkable[8];           /**< 8x8 bit for the walkable status of a tile. **/
    }
    sObject;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectStatic(sObject &obj);
    virtual ~ObjectStatic();
    virtual void freeRecources();
    virtual bool update(const Ogre::FrameEvent& event);
    bool movePosition(int dx, int dz);
    const Ogre::Vector3 &getPosition()
    {
        return mNode->getPosition();
    }
    Ogre::SceneNode *getSceneNode()
    {
        return mNode;
    }
    Ogre::Real getFacing()
    {
        return mFacing.valueDegrees();
    }
    void toggleAnimation(int animGroup, int animNr)
    {
        mAnim->toggleAnimation(animGroup, animNr);
    }
    const Ogre::String &getNickName() const
    {
        return mNickName;
    }
    void setNickName(Ogre::String name)
    {
        mNickName = name;
    }
    int getFriendly() const
    {
        return mFriendly;
    }
    Ogre::Entity *getEntity()
    {
        return mEntity;
    }
    Ogre::Vector3 getTilePos() const
    {
        return mTilePos;
    }
    void setPosition(Ogre::Vector3 pos);
    void activate(bool waitForHero = true);
    unsigned int getIndex() const
    {
        return mIndex;
    }
    Ogre::Real getHeight() const
    {
        return mBoundingBox.y;
    }
protected:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static Ogre::SceneManager *mSceneMgr;
    ObjectAnimate *mAnim;
    Ogre::SceneNode *mNode;
    Ogre::Entity *mEntity;
    Ogre::Vector3 mTilePos;   /**< the actual pos in the map. **/
    Ogre::String mNickName;
    Ogre::Vector3 mBoundingBox;
    Ogre::Degree mFacing;
    unsigned int mIndex;
    int mFriendly;
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
    ObjectStatic(const ObjectStatic&);            /**< disable copy-constructor. **/
    ObjectStatic &operator=(const ObjectStatic&); /**< disable assignment operator. **/
};

#endif