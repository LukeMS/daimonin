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

#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include "define.h"
#include "Ogre.h"

//=================================================================================================
// The World is devided into chunks. Each of this chunk holds several tiles.
//=================================================================================================

using namespace Ogre;

enum {QUALITY_LOW, QUALITY_HIGH};

const float PIXEL_PER_TILE = 128.0; // Pixel per Tile in the Ground-texture.
const float PIXEL_PER_ROW = 1024.0; // Pixel per Row  in the Ground-texture.
const int   TEXTURES_PER_ROW = (int)PIXEL_PER_ROW / (int)PIXEL_PER_TILE;
const int   HIGH_QUALITY_RANGE = 8; // Radius of the Area where the tiles are drawn in high quality.
const int   CHUNK_SUM_X = 8; // Map has x chunks on x-axis.
const int   CHUNK_SUM_Y = 8; // Map has y chunks on y-axis.
const int   TILES_PER_CHUNK = 16; // A chunk holds x^2 Tiles.
const int   HALF_CHUNK_SIZE = TILES_PER_CHUNK/2;
const int   TILE_SIZE = 30;
const int   NAME_BUFFER_SIZE = 50;

class CTileManager;
class Cworldmap;
class CTile;
class CEnvironmentManager;

class CChunk
{
private:
	bool m_IsAttached;

	#ifndef EXTERN_WATER
	MeshPtr m_Water_Mesh_high;
	MeshPtr m_Water_Mesh_low;
	SubMesh* m_Water_subMesh_high;
	SubMesh* m_Water_subMesh_low;
	Entity* m_Water_entity_high;
	Entity* m_Water_entity_low;
	SceneNode* m_Water;
	#endif

	MeshPtr m_Land_Mesh_high;
	MeshPtr m_Land_Mesh_low;
	SubMesh* m_Land_subMesh_high;
	SubMesh* m_Land_subMesh_low;
	Entity* m_Land_entity_high;
	Entity* m_Land_entity_low;
	SceneNode* m_Land;

	MaterialPtr m_Kartentextur;
	VertexData* m_vdata;
	IndexData* m_idata;
	HardwareVertexBufferSharedPtr m_vbuf0;
	HardwareIndexBufferSharedPtr m_ibuf;
	CEnvironmentManager* m_EnvironmentManagerPtr;

public:
	static char MeshName[NAME_BUFFER_SIZE];
	static char TempName[NAME_BUFFER_SIZE];
	static CTileManager* m_TileManagerPtr;
	static AxisAlignedBox* m_bounds;

	short m_posX, m_posY;

	CChunk();
	~CChunk();
	SceneNode* Get_Land(){ return m_Land; }
	#ifndef EXTERN_WATER
	SceneNode* Get_Water(){return m_Water;}
	Entity* Get_Water_entity(){return m_Water_entity_high;}
	#endif
	SubMesh* Get_Land_subMesh(){return m_Land_subMesh_high;}
	Entity* Get_Land_entity(){return m_Land_entity_high;}
	void Set_Tile(short &x, short &y) { m_posX = x; m_posY = y; }
	void Create(short &x, short &y);
	void CreateTexture();
	void CreateLandHigh();
	void CreateLandLow();
	#ifndef EXTERN_WATER
	void CreateWaterHigh();
	void CreateWaterLow();
	#endif
	void CreateSceneNode();
	void CreateEnvironmentManager();
	void Attach(short quality);
	void Detach();
};

class CTileManager
{
private:
	short (** m_Map); // Spielkarte
	CChunk m_mapchunk[CHUNK_SUM_X][CHUNK_SUM_Y]; // Kartenstücke
	Cworldmap* m_worldmap;
	SceneManager* m_SceneManager;
	AxisAlignedBox* bounds;
	float m_StretchZ;

public:
	CTileManager();
	~CTileManager();
	SceneManager* Get_pSceneManager(){ return m_SceneManager; }
	Cworldmap* Get_pworldmap() { return m_worldmap; }
	float Get_StretchZ() { return m_StretchZ;}
	short Get_Map(short x, short y) { return m_Map[x][y]; }
	AxisAlignedBox *GetBounds();
	void Init(Cworldmap* worldmap, SceneManager* SceneManager);
	void CreateChunks();
	void ControlChunks(Vector3 vector);
	void CreateTexture();
	void CreateTextureGroup(const char *terrain_type); // create the Group-Texture.
	void Create_Map(); // Errechnet ein graphisches Koordinatensystem für die Spielkarte.
	void SwitchMaterial(bool grid, bool highDetail);
	void addToGroupTexture(uchar* TextureGroup_data, uchar *Filter_data, Image* Texture, short pixel, short size, short x, short y);
};

#endif
