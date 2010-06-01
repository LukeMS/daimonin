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

#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <OgreStringConverter.h>
#include "object_manager.h"
#include "object_equipment.h"
#include "particle_manager.h"
#include "sound.h"
#include "events.h"
#include "logger.h"
#include "profiler.h"

using namespace Ogre;

const char *FILE_SHADOW_IMAGE = "shadow.png";
String boneName[ObjectEquipment::BONE_SUM]=
{
    "Pelvis",
    "Center",
    "Head",
    "Neck",
    "Spine",     "Spine1",
    "LClavicle", "R_Clavicle",
    "LUpperArm", "RUpperArm",
    "LForeArm",  "RForeArm",
    "LHand",     "RHand",
    "RFingers",  "LFingers",
    "LThigh",    "RThigh",
    "LCalf",     "RCalf",
    "LFoot",     "RFoot",
    "LToes",     "RToes"
};

const char *particleName[ObjectEquipment::PARTICLE_FX_SUM]=
{
    "Particle/SwordGlow"
};

// Todo: use pointer to vector for this. and read meshnames from a xml-file.
const char *meshName[][ObjectEquipment::ITEM_SUM]=
{
    {
        // ITEM_WEAPON
        "Sword_Short_01.mesh",
        "Mace_Small_01.mesh",
        "Short_Bow.mesh",
    },
    {
        // ITEM_ARMOR_SHIELD
        "Shield_Round_01.mesh",
        "Shield_Round_02.mesh"
    },
    {
        // ITEM_ARMOR_HEAD
        0,
        0
    },
    {
        // ITEM_ARMOR_BODY
        0,
        0
    },
    {
        // ITEM_ARMOR_LEGS
        0,
        0
    }
};

//================================================================================================
// Init the model from the description file.
//================================================================================================
ObjectEquipment::ObjectEquipment(Entity *parentEntity)
{
    PROFILE()
    Logger::log().list() << "Adding Equipment.";
    mParentEntity = parentEntity;
    for (int bone=0; bone < BONE_SUM; ++bone)
    {
        mItem[bone].entity  = 0;
        mItem[bone].particle= 0;
    }
}

//================================================================================================
// Create and attach an equipment item to bone.
//================================================================================================
void ObjectEquipment::equipItem(unsigned int bone, int type, int itemID, int particleID)
{
    PROFILE()
    if (bone >= BONE_SUM) return;
    dropItem(bone);
    // Add a particle system.
    if (particleID < 0 || particleID >= PARTICLE_FX_SUM)
    {
        mItem[bone].particle= 0;
    }
    else
    {
        mItem[bone].particle = ParticleManager::getSingleton().addBoneObject(mParentEntity, boneName[bone].c_str(), particleName[particleID], -1);
    }
    // Add a entity.
    if (itemID < 0 || itemID >= ITEM_SUM)
    {
        mItem[bone].entity = 0;
    }
    else
    {
        static unsigned long itemIndex =0;
        String tmpName = "Item_" + StringConverter::toString(++itemIndex, 8, '0');
        mItem[bone].entity= Events::getSingleton().getSceneManager()->createEntity(tmpName, meshName[type][itemID]);
        mItem[bone].entity->setQueryFlags(ObjectManager::QUERY_EQUIPMENT_MASK);
        mParentEntity->attachObjectToBone(boneName[bone], mItem[bone].entity);
    }
}

//================================================================================================
// Detach and destroy an equipment item from bone.
//================================================================================================
void ObjectEquipment::dropItem(int bone)
{
    PROFILE()
    if (mItem[bone].entity)
    {
        mParentEntity->detachObjectFromBone(mItem[bone].entity);
        Events::getSingleton().getSceneManager()->destroyEntity(mItem[bone].entity);
        mItem[bone].entity =0;
    }
    if (mItem[bone].particle)
    {
        mParentEntity->detachObjectFromBone(mItem[bone].particle);
        ParticleManager::getSingleton().delObject(mItem[bone].particle);
        mItem[bone].particle =0;
    }
}
