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

#ifndef OBJECT_ELEMENT_VISUAL2D_H
#define OBJECT_ELEMENT_VISUAL2D_H

#include "object/object.h"
#include "object/object_element.h"

/**
 ** This class handles the 2d specific part of an object.
 ** Remember that a 3d object has often a 2d component e.g:
 ** - the item gfx of the equipped weapon in the equippment window.
 ** - the face of the creature in the trading window.
 *****************************************************************************/
class ObjectElementVisual2d: public ObjectElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectElementVisual2d(Object *parent);
    ~ObjectElementVisual2d() {}
    Object::familyID getFamilyID() const { return Object::FAMILY_VISUAL2D; }
    bool update(const Ogre::FrameEvent &event);

private:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectElementVisual2d(const ObjectElementVisual2d&);            /**< disable copy-constructor. **/
    ObjectElementVisual2d &operator=(const ObjectElementVisual2d&); /**< disable assignment operator. **/
};

#endif
