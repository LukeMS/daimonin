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

#include "OgreParticleSystem.h"
#include "object_static.h"
#include "particle.h"
#include "sound.h"
#include "option.h"
#include "logger.h"
#include "spell_manager.h"
#include "events.h"
#include "TileManager.h"
#include "gui_manager.h"

///================================================================================================
/// Init all static Elemnts.
///================================================================================================
unsigned int  ObjStatic::mInstanceNr = 0;
SceneManager *ObjStatic::mSceneMgr =0;

///================================================================================================
/// Free all recources.
///================================================================================================
void ObjStatic::freeRecources()
{
  mTexture.setNull();
}

///================================================================================================
/// Init the model from the description file.
///================================================================================================
ObjStatic::ObjStatic(const char *mesh_filename, int posX, int posZ, float Facing)
{
  if (!mSceneMgr) mSceneMgr = Event->GetSceneManager();
  mFacing = Degree(Facing);
  thisStatic = mInstanceNr++;
  /// ////////////////////////////////////////////////////////////////////
  /// Build the mesh name.
  /// ////////////////////////////////////////////////////////////////////
  mEntity =mSceneMgr->createEntity("ObjStatic_" + StringConverter::toString(thisStatic, 3, '0'), mesh_filename);
  mEntity->setQueryFlags(QUERY_ENVIRONMENT_MASK);
  mPosX = posX;
  mPosZ = posZ;
  const AxisAlignedBox &AABB = mEntity->getBoundingBox();
  Vector3 pos;
  mBoundingBox.x = Math::Abs(AABB.getMaximum().x) - Math::Abs(AABB.getMinimum().x) + TILE_SIZE/2;
  mBoundingBox.y = Math::Abs(AABB.getMinimum().y);
  mBoundingBox.z = Math::Abs(AABB.getMaximum().z) - Math::Abs(AABB.getMinimum().z) + TILE_SIZE/2;
  pos.x = mPosX * TILE_SIZE + mBoundingBox.x;
  pos.y = (Real) (Event->getTileManager()->Get_Map_StretchedHeight(mPosX, mPosZ) + mBoundingBox.y);
  pos.z = mPosZ * TILE_SIZE + mBoundingBox.z;
  mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos, Quaternion(1.0,0.0,0.0,0.0));
  mNode->attachObject(mEntity);
}


///================================================================================================
/// Update npc.
///================================================================================================
void ObjStatic::update(const FrameEvent&)
{
}

///================================================================================================
/// Turn the mob until it faces the given tile.
///================================================================================================
void ObjStatic::faceToTile(int, int)
{
}

///================================================================================================
/// Move the mob to the given tile.
///================================================================================================
void ObjStatic::moveToTile(int, int)
{
}

///================================================================================================
/// Select a new texture.
///================================================================================================
void ObjStatic::setTexture(int, int, int)
{
}
