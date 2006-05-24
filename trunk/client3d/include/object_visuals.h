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

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef OBJECT_VISUALS_H
#define OBJECT_VISUALS_H

#include <vector>
#include "Ogre.h"
#include "object_npc.h"
#include "object_static.h"

using namespace Ogre;


class ObjectVisuals
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    static ObjectVisuals &getSingleton()
    {
        static ObjectVisuals Singleton; return Singleton;
    }
    void showLifebar(bool visible)
    {
        mEntityLifebar->setVisible(visible);
    }
    void setPosLifebar(Vector3 pos);
    void setLengthLifebar(int maxLength, int currentLength);

private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    Real mWidthLifebarGFX; /**< Lifebar has the same width like the NPC. **/
    Entity *mEntityLifebar;
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectVisuals();
    ~ObjectVisuals();
    ObjectVisuals(const ObjectVisuals&); // disable copy-constructor.
};

#endif
