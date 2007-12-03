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

#include "logger.h"
#include "option.h"
#include "events.h"
#include "object_manager.h"
#include "object_missile.h"
#include "sound.h"

using namespace Ogre;

//================================================================================================
// Init all static Elemnts.
//================================================================================================
SceneManager *ObjectMissile::msSceneMgr =0;
unsigned int ObjectMissile::msUnique =0;

//================================================================================================
// Free all recources.
//================================================================================================
void ObjectMissile::freeRecources()
{}

//================================================================================================
// Destructor.
//================================================================================================
ObjectMissile::~ObjectMissile()
{
    //delete particles;
    mNode->getParentSceneNode()->removeAndDestroyChild(mNode->getName());
}

//================================================================================================
// Init the object.
//================================================================================================
ObjectMissile::ObjectMissile(int type, ObjectNPC *srcMob, ObjectNPC *dstMob)
{
    // Create a missle.
    mType = type;
    mNode = Events::getSingleton().GetSceneManager()->getRootSceneNode()->createChildSceneNode();
    Entity *mEntity = Events::getSingleton().GetSceneManager()->createEntity(
                          "Mob"+ StringConverter::toString(++msUnique, 6, '0'),
                          "Arrow.mesh");
    mEntity->setQueryFlags(ObjectManager::QUERY_ENVIRONMENT_MASK);
    mNode->attachObject(mEntity);
    // Set the start position.
    Vector3 pos = srcMob->getPosition();
    pos.y +=srcMob->getHeight();
    mNode->setPosition(pos);
    mNode->setOrientation(srcMob->getSceneNode()->getOrientation());
    // Set the destination.
    mDestPosition = dstMob->getPosition();
    mDestPosition.y += dstMob->getHeight()/2;
    mSpeed = (mDestPosition-pos)*3;

    // If we use ballistic depends on the missle type.
    mHasBallistic = false;
}

//================================================================================================
// Update object.
//================================================================================================
bool ObjectMissile::update(const FrameEvent& event)
{
    if (mHasBallistic)
    {
    }
    else
    {
        mNode->translate(event.timeSinceLastFrame * mSpeed);
    }
    // Missle has reached the destination.
    if ((mDestPosition - mNode->getPosition()).squaredLength() < 1.0f)
    {
        return false;
    }
    return true;
}

