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
{
}

//================================================================================================
// Destructor.
//================================================================================================
ObjectStatic::~ObjectStatic()
{
    delete mAnim;
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
    mActPos.x = obj.posX;
    mActPos.z = obj.posY;
    mNickName = obj.nickName;
    mFloor    = obj.level;
    mCentred  = obj.centred;

    mFacing   = Degree(obj.facing);
    Logger::log().info()  << "Adding object: " << obj.meshName << ".";
    mEntity =mSceneMgr->createEntity("Obj_"+StringConverter::toString(obj.type, 2, '0')+"_" + StringConverter::toString(mIndex, 5, '0'), obj.meshName);
    mEntity->setQueryFlags(QUERY_ENVIRONMENT_MASK);
    Vector3 pos;
    const AxisAlignedBox &AABB = mEntity->getBoundingBox();
    mBoundingBox.x = TILE_SIZE_X/2 - (AABB.getMaximum().x + AABB.getMinimum().x)/2;
    mBoundingBox.z = TILE_SIZE_Z/2 - (AABB.getMaximum().z + AABB.getMinimum().z)/2;

    // If this is a plane - make sure it has higher y than the ground (avoid flickering):
    if (!AABB.getMaximum().y  && !AABB.getMinimum().y)
        mBoundingBox.y = -0.01;
    else
        mBoundingBox.y = AABB.getMinimum().y;

    if (mCentred)
    {
        pos.x = mActPos.x * TILE_SIZE_X + mBoundingBox.x;
        pos.z = mActPos.z * TILE_SIZE_Z + mBoundingBox.z;
    }
    else
    {
        pos.x = mActPos.x * TILE_SIZE_X;
        pos.z = mActPos.z * TILE_SIZE_Z;
    }
    pos.y = (Real) (TileManager::getSingleton().getAvgMapHeight(mActPos.x, mActPos.z)) - mBoundingBox.y;
    if (mFloor)
    {
        pos.y+= AABB.getMaximum().y * mFloor;
    }

    mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos);
    mNode->attachObject(mEntity);
    mNode->yaw(mFacing);

    if (Option::getSingleton().getIntValue(Option::CMDLINE_SHOW_BOUNDING_BOX))
    {
        mNode->showBoundingBox(true);
    }

    // Create the animations.
    mAnim = new ObjectAnimate(mEntity);
}

//================================================================================================
// Update object.
//================================================================================================
void ObjectStatic::update(const FrameEvent& event)
{
    mAnim->update(event);
}

//================================================================================================
// Move the object to the given position.
//================================================================================================
void ObjectStatic::move(Vector3 &pos)
{
    mNode->setPosition(mNode->getPosition() + pos);
}

//================================================================================================
// Move the object to the given position.
//================================================================================================
SubPos2D ObjectStatic::getTileScrollPos()
{
    static SubPos2D pos;
    TileManager::getSingleton().getMapScroll(pos.x, pos.z);
    pos.x+= mActPos.x;
    pos.z+= mActPos.z;
    pos.subX =0;
    pos.subZ =0;
    return pos;
}



