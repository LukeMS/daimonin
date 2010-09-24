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

#ifndef OBJECT_ELEMENT_EQUIP3D_H
#define OBJECT_ELEMENT_EQUIP3D_H

#include "object/object_element.h"

/// @brief This class handles all 3d equipment of an object.
/// @details Every bone in the skeleton can attach a single mesh item with or without an additional particleSystem.
class ObjectElementEquip3d: public ObjectElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
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
    /// @brief Default constructor.
    ObjectElementEquip3d(Object *parent, Ogre::Entity *entity);

    /// @brief Default destructor.
    ~ObjectElementEquip3d();

    /// @brief Get the familyID of the element.
    Object::familyID getFamilyID() const { return Object::FAMILY_EQUIP3D; }

    /// @brief Update this element.
    /// @param event Ogre frame event. Used to get time since last frame.
    bool update(const Ogre::FrameEvent &event);

    /// @brief Removes an item from the given bone.
    void dropItem(int bone);

    /// @brief Append an item to the given bone.
    /// @param bone       The bone to append the item.
    /// @param itemID     The id of the item.
    /// @param particleID The id of the particleSystem or -1 for none.
    void equipItem(unsigned int bone, int itemID, int particleID =-1);

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
    ObjectElementEquip3d(const ObjectElementEquip3d&);            /**< disable copy-constructor. **/
    ObjectElementEquip3d &operator=(const ObjectElementEquip3d&); /**< disable assignment operator. **/
};

#endif
