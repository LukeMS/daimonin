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

#include "logger.h"
#include "option.h"
#include "events.h"
#include "object_manager.h"
#include "object_missle.h"
#include "sound.h"

//================================================================================================
// Init all static Elemnts.
//================================================================================================
SceneManager *ObjectMissle::msSceneMgr =0;
unsigned int ObjectMissle::msUnique =0;

//================================================================================================
// Free all recources.
//================================================================================================
void ObjectMissle::freeRecources()
{}

//================================================================================================
// Destructor.
//================================================================================================
ObjectMissle::~ObjectMissle()
{
    //delete particles;
    mNode->getParentSceneNode()->removeAndDestroyChild(mNode->getName());
}

//================================================================================================
// Init the object.
//================================================================================================
ObjectMissle::ObjectMissle(int type, ObjectNPC *srcMob, ObjectNPC *dstMob)
{
    // Create a missle.
    mType = type;
    mNode = Events::getSingleton().GetSceneManager()->getRootSceneNode()->createChildSceneNode();
    Entity *mEntity = Events::getSingleton().GetSceneManager()->createEntity(
                          "Mob"+ StringConverter::toString(++msUnique, 6, '0'),
                          "Arrow.mesh");
    mNode->attachObject(mEntity);
    mNode->scale(1,4,1);
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
bool ObjectMissle::update(const FrameEvent& event)
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

