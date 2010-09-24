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

#ifndef OBJECT_ELEMENT_PHYSICAL_H
#define OBJECT_ELEMENT_PHYSICAL_H

#include "object/object.h"
#include "object/object_element.h"

/// @brief This class handles all physical stuff (live, mana, grace, etc) of an object.
/// @details
class ObjectElementPhysical: public ObjectElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /// @brief Default constructor.
    ObjectElementPhysical(Object *parent);

    /// @brief Default destructor.
    ~ObjectElementPhysical() {}

    /// @brief Get the familyID of the element.
    Object::familyID getFamilyID() const { return Object::FAMILY_PHYSICAL; }

    /// @brief Update this element.
    /// @param event Ogre frame event. Used to get time since last frame.
    bool update(const Ogre::FrameEvent &event);

private:
    int mMaxLife, mActLife;
    int mMaxMana, mActMana;
    int mMaxGrace, mActGrace;
    int mExperience, mExpLevel;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectElementPhysical(const ObjectElementPhysical&);            ///< disable copy-constructor.
    ObjectElementPhysical &operator=(const ObjectElementPhysical&); ///< disable assignment operator.
};

#endif
