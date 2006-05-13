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
  if (mAnim) delete mAnim;
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
  Logger::log().info()  << "Adding object: " << mesh_filename << ".";
  mEntity =mSceneMgr->createEntity("ObjStatic_" + StringConverter::toString(thisStatic, 3, '0'), mesh_filename);
  mEntity->setQueryFlags(QUERY_ENVIRONMENT_MASK);
  mPosX = posX;
  mPosZ = posZ;
  const AxisAlignedBox &AABB = mEntity->getBoundingBox();
  Vector3 pos;
  mBoundingBox.x = TILE_SIZE_X/2 - (AABB.getMaximum().x + AABB.getMinimum().x)/2;
  mBoundingBox.z = TILE_SIZE_Z/2 - (AABB.getMaximum().z + AABB.getMinimum().z)/2;
  mBoundingBox.y = AABB.getMinimum().y;
  pos.x = mPosX * TILE_SIZE_X + mBoundingBox.x;
  pos.z = mPosZ * TILE_SIZE_Z + mBoundingBox.z;
  pos.y = (Real) (Event->getTileManager()->Get_Avg_Map_Height(mPosX, mPosZ)) - mBoundingBox.y;
  mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos);
  mNode->attachObject(mEntity);

  //mNode->scale(10,10,10);

  mAnim = new Animate(mEntity);
}

///================================================================================================
/// Update npc.
///================================================================================================
void ObjStatic::update(const FrameEvent& event)
{
  //    Logger::log().info() << "hier";
    mAnim->update(event);
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
