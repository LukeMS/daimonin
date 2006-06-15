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

#include "object_equipment.h"
#include "object_manager.h"
#include "sound.h"
#include "events.h"
#include "option.h"
#include "logger.h"

///================================================================================================
/// Init all static Elemnts.
///================================================================================================
unsigned int ObjectEquipment::mWeaponID =0;
unsigned int ObjectEquipment::mArmorID  =0;

///================================================================================================
/// .
///================================================================================================
ObjectEquipment::~ObjectEquipment()
{}

///================================================================================================
/// Free all recources.
///================================================================================================
void ObjectEquipment::freeRecources()
{}

///================================================================================================
/// Init the model from the description file.
///================================================================================================
ObjectEquipment::ObjectEquipment(unsigned int type, const char *meshName, const char *particleName)
{
    Logger::log().info()  << "Adding object: " << meshName << ".";
    switch (type)
    {
        case ObjectManager::ATTACHED_OBJECT_WEAPON:
        {
            String tmpName = "Weapon_" + StringConverter::toString(mWeaponID++, 4, '0');
            mEntity= Event->GetSceneManager()->createEntity(tmpName, meshName);
            mEntity->setQueryFlags(QUERY_EQUIPMENT_MASK);
            if (particleName) mStrParticleName = particleName;
            break;
        }
        case ObjectManager::ATTACHED_OBJECT_ARMOR:
        {
            String tmpName = "Armor_" + StringConverter::toString(mWeaponID++, 4, '0');
            mEntity= Event->GetSceneManager()->createEntity(tmpName, meshName);
            mEntity->setQueryFlags(QUERY_EQUIPMENT_MASK);
            if (particleName) mStrParticleName = particleName;
            break;
        }
    }
}

///================================================================================================
/// .
///================================================================================================
const Entity *ObjectEquipment::getEntity()
{
    return mEntity;
}
