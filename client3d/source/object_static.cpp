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

#include "object_static.h"
#include "object_manager.h"
#include "tile_manager.h"
#include "option.h"
#include "sound.h"
#include "events.h"
#include "logger.h"
#include "network.h"

using namespace Ogre;

//================================================================================================
// Init all static Elemnts.
//================================================================================================
SceneManager *ObjectStatic::mSceneMgr =0;

//================================================================================================
// Free all recources.
//================================================================================================
void ObjectStatic::freeRecources()
{}

//================================================================================================
// Destructor.
//================================================================================================
ObjectStatic::~ObjectStatic()
{
    delete mAnim;
    mNode->getParentSceneNode()->removeAndDestroyChild(mNode->getName());
}

//================================================================================================
// Init the object.
//================================================================================================
ObjectStatic::ObjectStatic(sObject &obj)
{
    static unsigned int index =0;
    if (!mSceneMgr)
    {
        mSceneMgr = Events::getSingleton().GetSceneManager();
        Logger::log().headline("Init Actor Models");
    }
    mIndex    = index++;
    mNickName = obj.nickName;
    mFloor    = obj.level;
    mFacing   = Degree(obj.facing);
    Logger::log().info()  << "Adding object: " << obj.meshName << ".";

    String strObj = ObjectManager::ObjectID[obj.type];
    strObj+= "#Obj_";
    strObj+= StringConverter::toString(mIndex, 10, '0');
    mEntity =mSceneMgr->createEntity(strObj, obj.meshName);
    mEntity->setRenderQueueGroup(RENDER_QUEUE_7);
    switch (obj.type)
    {
        case ObjectManager::OBJECT_CONTAINER:
            mEntity->setQueryFlags(ObjectManager::QUERY_CONTAINER);
            break;

        case ObjectManager::OBJECT_NPC:
        case ObjectManager::OBJECT_PLAYER:
            mEntity->setQueryFlags(ObjectManager::QUERY_NPC_MASK);
            break;

        default:
            mEntity->setQueryFlags(ObjectManager::QUERY_ENVIRONMENT_MASK);
    }

    const AxisAlignedBox &AABB = mEntity->getBoundingBox();
    mBoundingBox = (AABB.getMaximum() + AABB.getMinimum())/2;

    mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    mNode->attachObject(mEntity);
    mNode->yaw(mFacing);
    setPosition(obj.pos);

    if (Option::getSingleton().getIntValue(Option::CMDLINE_SHOW_BOUNDING_BOX))
        mNode->showBoundingBox(true);

    // Create the animations.
    mAnim = new ObjectAnimate(mEntity);

    // Set the walkable bits for the tile on which this object is placed.
    if (obj.type < ObjectManager::OBJECT_NPC)
    {
        for (int row =0; row< 8; ++row)
        {
            TileManager::getSingleton().setWalkablePos(mActTilePos, row, obj.walkable[row]);
        }
    }
    mAction = ACTION_NONE;
    mOpen = false;
    mWaitForHero = false;
}

//================================================================================================
// Moves the object (instantly) onto a new positon.
//================================================================================================
bool ObjectStatic::movePosition(int deltaX, int deltaZ)
{
    mActTilePos.x += deltaX;
    mActTilePos.z += deltaZ;
    setPosition(mActTilePos);
    // if (pos is out of playfield) return false;
    return true;
}

//================================================================================================
// Put the object onto a new position.
//================================================================================================
void ObjectStatic::setPosition(TilePos pos)
{
    mActTilePos = pos;
    Vector3 posV;
    posV = TileManager::getSingleton().getTileInterface()->tileToWorldPos(pos);
    if (mFloor)
        posV.y += mBoundingBox.y * mFloor;
    mNode->setPosition(posV);
}

//================================================================================================
// Update object.
//================================================================================================
bool ObjectStatic::update(const FrameEvent& event)
{
    mAnim->update(event);
    if (mAction == ACTION_NONE)
        return true;
    // First wait for our hero to come to the objects position.
    if (mWaitForHero)
    {
        if (!ObjectManager::getSingleton().isMoving(ObjectNPC::HERO))
            mWaitForHero = false;
    }
    else
    {
        if (mAction == ACTION_OPEN)
        {
            mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ABILITY, 0, false, true, false, true);
            mOpen = true;
            Network::getSingleton().send_command("/apply", -1, Network::SC_NORMAL);
        }
        else if (mAction == ACTION_CLOSE)
        {
            mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_ABILITY, 1, false, true, false, true);
            mOpen = false;
            Network::getSingleton().send_command("/apply", -1, Network::SC_NORMAL);
        }
        mAction = ACTION_NONE;
    }
    return true;
}

//================================================================================================
// Activate an object.
//================================================================================================
void ObjectStatic::activate(bool waitForHero)
{
    mWaitForHero = waitForHero;
    if (mOpen)
        mAction = ACTION_CLOSE;
    else
        mAction = ACTION_OPEN;
}
