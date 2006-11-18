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
#include "option.h"
#include "sound.h"
#include "events.h"
#include "logger.h"

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
    if (!mSceneMgr)
    {
        mSceneMgr = Event->GetSceneManager();
        Logger::log().headline("Init Actor Models");
    }
    mIndex    = obj.index;
    mActPos   = obj.pos;
    mNickName = obj.nickName;
    mFloor    = obj.level;
    mFacing   = Degree(obj.facing);
    Logger::log().info()  << "Adding object: " << obj.meshName << ".";

    string strObj = ObjectManager::ObjectID[obj.type];
    strObj+= "#Obj_";
    strObj+= StringConverter::toString(mIndex, 8, '0');
    mEntity =mSceneMgr->createEntity(strObj, obj.meshName);
    switch (obj.type)
    {

        case ObjectManager::OBJECT_CONTAINER:
        case ObjectManager::OBJECT_ENVIRONMENT:
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
    mBoundingBox.x = (AABB.getMaximum().x + AABB.getMinimum().x)/2;
    mBoundingBox.z = (AABB.getMaximum().z + AABB.getMinimum().z)/2;

    mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    mNode->attachObject(mEntity);
    mNode->yaw(mFacing);
    setPosition(mActPos);

    if (Option::getSingleton().getIntValue(Option::CMDLINE_SHOW_BOUNDING_BOX))
    {
        mNode->showBoundingBox(true);
    }

    // Create the animations.
    mAnim = new ObjectAnimate(mEntity);

    // Set the walkable bits for the tile on which this object is placed.
    if (obj.type < ObjectManager::OBJECT_NPC)
    {
        for (int row =0; row< 8; ++row)
        {
            TileManager::getSingleton().setWalkablePos(mActPos, row, obj.walkable[row]);
        }
    }
}

//================================================================================================
// Moves the object (instantly) onto a new positon.
//================================================================================================
void ObjectStatic::movePosition(int deltaX, int deltaZ)
{
    mActPos.x += deltaX;
    mActPos.z += deltaZ;
    setPosition(mActPos);
}

//================================================================================================
// Put the object onto a new position.
//================================================================================================
void ObjectStatic::setPosition(TilePos pos)
{
    mActPos = pos;
    Vector3 posV = TileManager::getSingleton().getTileInterface()->tileToWorldPos(pos);
    if (mFloor)
    {
        posV.y += mBoundingBox.y * mFloor;
    }
    mNode->setPosition(posV);
}

//================================================================================================
// Update object.
//================================================================================================
bool ObjectStatic::update(const FrameEvent& event)
{
    mAnim->update(event);
	return true;
}

//================================================================================================
// Move the object to the given position.
//================================================================================================
void ObjectStatic::move(Vector3 &pos)
{
    mNode->setPosition(mNode->getPosition() + pos);
}

