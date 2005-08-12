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
#include "EnvironmentManager.h"

using namespace Ogre;

const float PIXEL_PER_TILE =  128.0; // Pixel per Tile in the Ground-texture.
const float PIXEL_PER_ROW  = 1024.0; // Pixel per Row  in the Ground-texture.

const int   TILE_SIZE = 30;
const int   CHUNK_SUM_X  =  1; // Map has x chunks on x-axis.
const int   CHUNK_SUM_Z  =  1; // Map has y chunks on y-axis.
const int   CHUNK_SIZE_X = 22; // Chunk has x tiles on x-axis & y-axis. Must be even.
const int   CHUNK_SIZE_Z = 22; // Chunk has x tiles on x-axis & y-axis. Must be even.
const int   TILES_SUM_X  = CHUNK_SUM_X * CHUNK_SIZE_X;
const int   TILES_SUM_Z  = CHUNK_SUM_Z * CHUNK_SIZE_Z;

const int   TEXTURES_PER_ROW = (int)PIXEL_PER_ROW / (int)PIXEL_PER_TILE;
const int   HIGH_QUALITY_RANGE = 5; // Radius of the Area where the tiles are drawn in high quality.
const int   NAME_BUFFER_SIZE = 50;
const int   MIN_TEXTURE_PIXEL = 16;

enum {QUALITY_LOW, QUALITY_HIGH};

// Height levels.
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
//			 TERRAIN_BEACH,  TERRAIN_WATER};
// Spezical Effects: Snopw, Rain, Lava, 

class CChunk
{
private:
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

	CEnvironmentManager* m_EnvironmentManagerPtr;

public:
	static char MeshName[NAME_BUFFER_SIZE];
	static char TempName[NAME_BUFFER_SIZE];
	static CTileManager* m_TileManagerPtr;
	static AxisAlignedBox* m_bounds;

	short m_posX, m_posZ;

	CChunk();
	~CChunk();
	SceneNode* Get_Land(){ return m_Land; }
	SceneNode* Get_Water(){return m_Water;}
	Entity* Get_Water_entity(){return m_Water_entity_high;}
	SubMesh* Get_Land_subMesh(){return m_Land_subMesh_high;}
	Entity* Get_Land_entity(){return m_Land_entity_high;}
	void Set_Tile(short &x, short &z) { m_posX = x; m_posZ = z; }
	void Create(short &x, short &z);
	void Change(short &x, short &z);

	void Create_Dummy(SubMesh* submesh); // No Land/Water on this chunk. Lets make a dummy.
	void CreateLandHigh();
	void ChangeLandHigh();
	void CreateLandHigh_Buffers();

	void CreateLandLow();
	void ChangeLandLow();
	void CreateLandLow_Buffers();

	void CreateWaterHigh();
	void ChangeWaterHigh();
	void CreateWaterHigh_Buffers();

	void CreateWaterLow();
	void ChangeWaterLow();
	void CreateWaterLow_Buffers();

	void CreateTexture();
	void CreateSceneNode();
	void CreateEnvironmentManager();
	void Attach(short quality);
	void Detach();
};

#endif
