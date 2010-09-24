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
#include <OgreParticleSystem.h>
#include <OgreStringConverter.h>
#include "logger.h"
#include "profiler.h"
#include "object/object_manager.h"
#include "object/object_element_equip3d.h"

using namespace Ogre;

static const String boneName[ObjectElementEquip3d::BONE_SUM]=
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

static const char *particleName[]=
{
    "Particle/SwordGlow"
};
static const unsigned int SUM_PARTICLES = sizeof(particleName) / sizeof(char*);

// Todo: use pointer to vector for this. and read meshnames from a xml-file.
static const char *meshName[]=
{
    // ITEM_WEAPON
    "Sword_Short_01.mesh",
    "Mace_Small_01.mesh",
    "Short_Bow.mesh",
    // ITEM_ARMOR_SHIELD
    "Shield_Round_01.mesh",
    "Shield_Round_02.mesh"
};
static const unsigned int SUM_ITEM = sizeof(meshName) / sizeof(char*);

//================================================================================================
// Init the model from the description file.
//================================================================================================
ObjectElementEquip3d::ObjectElementEquip3d(Object *parent, Entity *parentEntity):ObjectElement(parent)
{
    PROFILE()
    Logger::log().list() << Logger::ICON_CLIENT << "Adding Equipment.";
    parent->addElement(getFamilyID(), this);
    mParentEntity = parentEntity;
    for (int bone=0; bone < BONE_SUM; ++bone)
    {
        mItem[bone].entity  = 0;
        mItem[bone].particle= 0;
    }
}

//================================================================================================
//
//================================================================================================
ObjectElementEquip3d::~ObjectElementEquip3d()
{
    for (int bone =0; bone < BONE_SUM; ++bone)
        dropItem(bone);
}

//================================================================================================
//
//================================================================================================
bool ObjectElementEquip3d::update(const Ogre::FrameEvent &/*event*/)
{
    return true;
}

//================================================================================================
// Adds an item (with optionl particle effects) to a bone.
//================================================================================================
void ObjectElementEquip3d::equipItem(unsigned int bone, int itemID, int particleID)
{
    PROFILE()
    static unsigned long itemIndex =0;
    if (bone >= BONE_SUM || (unsigned int)itemID >= SUM_ITEM) return;
    dropItem(bone);
    String tmpName = "Item_" + StringConverter::toString(++itemIndex, 8, '0');
    try
    {
        // Add an entity.
        mItem[bone].entity= mParentEntity->getParentSceneNode()->getCreator()->createEntity(tmpName, meshName[itemID]);
        mItem[bone].entity->setQueryFlags(ObjectManager::QUERY_MASK_NPC);
        mItem[bone].entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_7);
        mParentEntity->attachObjectToBone(boneName[bone], mItem[bone].entity);
        // Add a particle system.
        if ((unsigned int)particleID < SUM_PARTICLES)
        {
            mItem[bone].particle = mParentEntity->getParentSceneNode()->getCreator()->createParticleSystem(tmpName+"_p", particleName[particleID]);
            mItem[bone].particle->setBoundsAutoUpdated(false);
            mItem[bone].particle->setKeepParticlesInLocalSpace(true);
            mItem[bone].particle->setQueryFlags(ObjectManager::QUERY_MASK_NPC);
            mItem[bone].particle->setRenderQueueGroup(RENDER_QUEUE_7);
            mParentEntity->attachObjectToBone(boneName[bone], mItem[bone].particle);
        }
    }
    catch (Exception &/*e*/) {}
}

//================================================================================================
// Detach and destroy an equipment item from a bone.
//================================================================================================
void ObjectElementEquip3d::dropItem(int bone)
{
    PROFILE()
    if ((unsigned int)bone >= BONE_SUM || !mItem[bone].entity) return;
    //mParentEntity->detachObjectFromBone(mItem[bone].entity);
    mParentEntity->getParentSceneNode()->getCreator()->destroyEntity(mItem[bone].entity);
    mItem[bone].entity =0;
    if (mItem[bone].particle)
    {
        //mParentEntity->detachObjectFromBone(mItem[bone].particle);
        mParentEntity->getParentSceneNode()->getCreator()->destroyParticleSystem(mItem[bone].particle);
        mItem[bone].particle =0;
    }
}
