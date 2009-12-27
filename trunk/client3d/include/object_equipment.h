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

#ifndef ObjectEquipment_H
#define ObjectEquipment_H

#include <Ogre.h>

/**
 ** This class handles all equipment of an object like weapon, armour, etc.
 ** Equipment supports particle-effects and color change of the bodyparts.
 *****************************************************************************/
class ObjectEquipment
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        PARTICLE_FX_FIRE,
        PARTICLE_FX_SUM
    };
    enum
    {
        ITEM_WEAPON,
        ITEM_ARMOR_SHIELD,
        ITEM_ARMOR_HEAD,
        ITEM_ARMOR_BODY,
        ITEM_ARMOR_LEGS,
        ITEM_SUM
    };
    enum
    {
        BONE_PELVIS,
        BONE_CENTER,
        BONE_HEAD,
        BONE_NECK,
        BONE_SPINE,       BONE_SPINE1,
        BONE_LCLAVICLE,   BONE_RCLAVICLE,
        BONE_LUPPER_ARM,  BONE_RUPPER_ARM,
        BONE_LFORE_ARM,   BONE_RFORE_ARM,
        BONE_L_HAND,      BONE_R_HAND,
        BONE_WEAPON_HAND, BONE_SHIELD_HAND,
        BONE_LTHIGH,      BONE_RTHIGH,
        BONE_LCALF,       BONE_RCALF,
        BONE_LFOOT,       BONE_RFOOT,
        BONE_LTOES,       BONE_RTOES,
        BONE_SUM
    };
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectEquipment(Ogre::Entity *parent);
    ~ObjectEquipment() {}
    void dropItem(int bone);
    void equipItem(unsigned int bone, int type, int itemID, int particleID =-1);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    struct _mItem
    {
        Ogre::Entity *entity;
        Ogre::ParticleSystem *particle;
    }
    mItem[BONE_SUM];
    Ogre::Entity *mParentEntity;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectEquipment(const ObjectEquipment&); // disable copy-constructor.
};
#endif
