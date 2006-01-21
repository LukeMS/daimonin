/*-----------------------------------------------------------------------------
This source file is part of Code-Black (http://www.code-black.org)
Copyright (c) 2005 by the Code-Black Team
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

#ifndef TILE_CHUNK_H
#define TILE_CHUNK_H

#include "Ogre.h"



using namespace Ogre;

class TileManager;

/** Height levels for the tiles. **/
enum {
  // Mountains
  LEVEL_MOUNTAIN_TOP = 70,
  LEVEL_MOUNTAIN_MID = 60,
  LEVEL_MOUNTAIN_DWN = 40,
  // Plains
  LEVEL_PLAINS_TOP = 18,
  LEVEL_PLAINS_MID = 14,
  LEVEL_PLAINS_DWN = 12,
  LEVEL_PLAINS_SUB = 10,
  // Water
  LEVEL_WATER_CLP = LEVEL_PLAINS_SUB+1, // At this point the water clips the land-tiles.
  LEVEL_WATER_TOP = LEVEL_WATER_CLP -1,
};

// Outdoor
//enum {TERRAIN_MOUNTAIN, TERRAIN_JUNGLE, TERRAIN_PLAINS, TERRAIN_SWAMP, TERRAIN_RIFT,TERRAIN_DESERT,
//    TERRAIN_BEACH,  TERRAIN_WATER};
// Spezical Effects: Snopw, Rain, Lava,


/**
 * TileEngine class which manages the tiles in a chunk.
 *****************************************************************************/
class TileChunk
{
private:
  /** Buffersize for meshnames. **/
  static const int NAME_BUFFER_SIZE = 50;
  bool m_IsAttached;
  MeshPtr m_Water_Mesh_high;
  MeshPtr m_Water_Mesh_low;
  SubMesh* m_Water_subMesh_high;
  SubMesh* m_Water_subMesh_low;
  Entity* m_Water_entity_high;
  Entity* m_Water_entity_low;
  SceneNode* m_Water;

  MeshPtr m_Land_Mesh_high;
  MeshPtr m_Land_Mesh_low;
  SubMesh* m_Land_subMesh_high;
  SubMesh* LandSubMesh;
  SubMesh* m_Land_subMesh_low;
  Entity* m_Land_entity_high;
  Entity* m_Land_entity_low;
  SceneNode* m_Land;


public:
  /** Buffer for building meshnames. **/
  static char MeshName[NAME_BUFFER_SIZE];
  /** Buffer for temporary actions. **/
  static char TempName[NAME_BUFFER_SIZE];
  static TileManager* m_TileManagerPtr;
  static AxisAlignedBox* m_bounds;

  /** X pos of the chunk. **/
  short m_posX;
  /** Z pos of the chunk. **/
  short m_posZ;

  TileChunk();
  ~TileChunk();
  SceneNode* Get_Land()
  {
    return m_Land;
  }
  SceneNode* Get_Water()
  {
    return m_Water;
  }
  Entity* Get_Water_entity()
  {
    return m_Water_entity_high;
  }
  SubMesh* Get_Land_subMesh()
  {
    return m_Land_subMesh_high;
  }
  Entity* Get_Land_entity()
  {
    return m_Land_entity_high;
  }
  short get_posX(){
    return m_posX;}
  short get_posZ(){
    return m_posZ;}
  void Set_Tile(short &x, short &z)
  {
    m_posX = x; m_posZ = z;
  }
  void Create(short &x, short &z, int tileTextureSize);
  void Change();

  /** Every chunk must have a land- AND a waterSubmesh,
  if there is one of them missing, we make a dummy submesh. **/
  void Create_Dummy(SubMesh* submesh);
  /** Create a (high poly) land chunk. **/
  void CreateLandHigh(int tileTextureSize);
  /** Change a (high poly) land chunk. **/
  void ChangeLandHigh();
  /** Create HW buffers for (high poly) land chunk. **/
  void CreateLandHigh_Buffers();

  /** Create a (low poly) land chunk. **/
  void CreateLandLow();
  /** Change a (low poly) land chunk. **/
  void ChangeLandLow();
  /** Create HW buffers for (low poly) land chunk. **/
  void CreateLandLow_Buffers();

  /** Create a (high poly) water chunk. **/
  void CreateWaterHigh();
  /** Change a (high poly) water chunk. **/
  void ChangeWaterHigh();
  /** Create HW buffers for (high poly) water chunk. **/
  void CreateWaterHigh_Buffers();

  /** Create a (low poly) water chunk. **/
  void CreateWaterLow();
  /** Change a (low poly) water chunk. **/
  void ChangeWaterLow();
  /** Create HW buffers for (low poly) water chunk. **/
  void CreateWaterLow_Buffers();

  /** Create terrain-texture in multiple sizes. **/
  void CreateTexture();
  void CreateSceneNode();
  void Attach(short quality);
  void Detach();
};

#endif
