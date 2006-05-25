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

#include "define.h"
#include "option.h"
#include "logger.h"
#include "events.h"
#include "object_visuals.h"

///================================================================================================
/// Init all static Elemnts.
///================================================================================================


///================================================================================================
/// Free all recources.
///================================================================================================
void ObjectVisuals::freeRecources()
{}

///================================================================================================
/// .
///================================================================================================
ObjectVisuals::~ObjectVisuals()
{}

///================================================================================================
/// .
///================================================================================================
ObjectVisuals::ObjectVisuals()
{
    mSelectionVsible = false;
    mNodeSelection = 0;

    /// ////////////////////////////////////////////////////////////////////
    /// Create texture.
    /// ////////////////////////////////////////////////////////////////////
    // todo:
    // build texture from font + Imageset.png (*.xml)

    /// ////////////////////////////////////////////////////////////////////
    /// Create selection entity..
    /// ////////////////////////////////////////////////////////////////////
    const Real SIZE = 10.0;
    ManualObject* mob = static_cast<ManualObject*>(Event->GetSceneManager()->createMovableObject("mob", ManualObjectFactory::FACTORY_TYPE_NAME));
    mob->begin("selection"); // MAterial name;
    mob->position(-SIZE, 0.0, -SIZE);
    mob->normal(0,0,1);
    mob->textureCoord(0.0, 0.0);
    mob->position(-SIZE, 0.0, SIZE);
    mob->normal(0,0,1);
    mob->textureCoord(0.0, 1.0);
    mob->position(SIZE, 0.0, -SIZE);
    mob->normal(0,0,1);
    mob->textureCoord(1.0, 0.0);
    mob->position(SIZE, 0.0, SIZE);
    mob->normal(0,0,1);
    mob->textureCoord(1.0, 1.0);
    mob->triangle( 0, 1, 2);
    mob->triangle( 3, 2, 1);
    mob->end();
    mob->convertToMesh("selection.mesh");
    mEntitySelection=Event->GetSceneManager()->createEntity("Selection","selection.mesh");
    mEntitySelection->setQueryFlags(QUERY_NPC_SELECT_MASK);

    /// ////////////////////////////////////////////////////////////////////
    /// Create lifebar entity.
    /// ////////////////////////////////////////////////////////////////////
/*
    const Real SIZE = 100.0;
    ManualObject* mob = static_cast<ManualObject*>(Event->GetSceneManager()->createMovableObject("mob", ManualObjectFactory::FACTORY_TYPE_NAME));
    mob->begin("selection"); // MAterial name;
    mob->position(-SIZE, 0.0, -SIZE);
    mob->normal(0,0,1);
    mob->textureCoord(0.0, 0.0);
    mob->position(-SIZE, 0.0, SIZE);
    mob->normal(0,0,1);
    mob->textureCoord(0.0, 1.0);
    mob->position(SIZE, 0.0, -SIZE);
    mob->normal(0,0,1);
    mob->textureCoord(1.0, 0.0);
    mob->position(SIZE, 0.0, SIZE);
    mob->normal(0,0,1);
    mob->textureCoord(1.0, 1.0);
    mob->triangle( 0, 1, 2);
    mob->triangle( 3, 2, 1);
    mob->end();
    mob->convertToMesh("selection.mesh");
    mEntitySelection=Event->GetSceneManager()->createEntity("Selection","selection.mesh");
    mEntitySelection->setQueryFlags(QUERY_NPC_SELECT_MASK);
*/
}

///================================================================================================
/// .
///================================================================================================
void ObjectVisuals::setPosLifebar(Vector3 pos)
{}

///================================================================================================
/// .
///================================================================================================
void ObjectVisuals::setLengthLifebar(int maxLength, int currentLength)
{
    if (!maxLength) return; // prevent division by zero.
    Real filling = (mWidthLifebarGFX * currentLength) / maxLength;
    // subMesh3->setScale(filling, 1.0f, 1.0f);
}

///================================================================================================
/// Select a NPC.
///================================================================================================
void ObjectVisuals::selectNPC(MovableObject *mob)
{
    if (mNodeSelection) mNodeSelection->getParentSceneNode()->removeAndDestroyChild("SelNode");
    mNodeSelection = mob->getParentSceneNode()->createChildSceneNode("SelNode");
    mNodeSelection->attachObject(mEntitySelection);

    const AxisAlignedBox &AABB = mob->getBoundingBox();
    Vector3 pos = mNodeSelection->getPosition();
    mNodeSelection->setPosition(pos.x, AABB.getMinimum().y +3, pos.z);
}
