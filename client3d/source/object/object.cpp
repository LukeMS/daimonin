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

#include "object/object.h"

//================================================================================================
// Default Destructor.
//================================================================================================
Object::~Object()
{
    for (std::map<familyID, class ObjectElement*>::const_iterator i=mElementMap.begin(); i!=mElementMap.end(); ++i)
    {
        if ((*i).second)
            delete (*i).second;
    }
    mElementMap.clear();
}

//================================================================================================
// Add the element to the given family.
//================================================================================================
void Object::addElement(familyID id, class ObjectElement *element)
{
    mElementMap.insert(std::pair<familyID, ObjectElement*>(id, element));
}

//================================================================================================
// Removes the element from this object, but doesn't delete the element.
//================================================================================================
void Object::delElement(familyID id)
{
    mElementMap.erase(id);
}

//================================================================================================
// Get the element of this family.
//================================================================================================
class ObjectElement *Object::getElement(familyID id)
{
    std::map<familyID, class ObjectElement*>::const_iterator i= mElementMap.find(id);
    return (i!= mElementMap.end()) ? (*i).second : 0;
}

//================================================================================================
// Update all elements in this object.
//================================================================================================
bool Object::update(const Ogre::FrameEvent &event)
{
    for (std::map<familyID, class ObjectElement*>::const_iterator i=mElementMap.begin(); i!=mElementMap.end(); ++i)
    {
        if ((*i).second)
            (*i).second->update(event);
    }
    return true;
}
