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

#ifndef OBJECT_H
#define OBJECT_H

#include <map>
#include <OgreFrameListener.h>
#include "object/object_element.h"

/// @brief This is the main class for an object.
/// @details
class Object
{
public:
    typedef enum
    {
        FAMILY_CONTROLS,    ///< All interactive user controls (mouse, keyboard, etc).
        FAMILY_EQUIP3D,     ///< Equipment for a 3d object (sword, shoes, etc).
        FAMILY_VISUAL3D,    ///< Renders a 3d object.
        FAMILY_ANIMATION3D, ///< Skeleton animation for a 3d object.
        FAMILY_AVATAR,      ///< All stuff related to your avatar.
        FAMILY_VISUAL2D,    ///< Renders a 2d object (also 2d graphics of a 3d object).
        FAMILY_PHYSICAL,    ///< Physical stats (life, speed, att, def, etc).
        FAMILY_AI,          ///< Artificial intelligence.
        FAMILY_SKILLS,      ///< Skills of a creature.
        FAMILY_SPELLS,      ///< Spells of a creature.
        // to be continued...
    } familyID;

    /// @brief Default constructor.
    Object() {}

    /// @brief Default destructor.
    ~Object();

    /// @brief Get an element of this object.
    /// @param familyID The type of element.
    /// @return A pointer to the element, or 0 when no such element exists.
    class ObjectElement *getElement(familyID id);

    /// @brief Add an element to this object.
    void addElement(familyID id, class ObjectElement *element);

    /// @brief Update all elements of this object.
    /// @param event Ogre frame event. Used to get time since last frame.
    bool update(const Ogre::FrameEvent &event);

private:

    /// @brief All elements are stored in a std::map.
    std::map<familyID, class ObjectElement*> mElementMap;
};

#endif
