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

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <time.h>
#include "TileChunk.h"
#include "TileManager.h"
#include "logger.h"


//#define LOW_QUALITY_RENDERING

//=================================================================================================
// Constructor
//=================================================================================================
CTileManager::CTileManager() {}

//=================================================================================================
// Destructor
//=================================================================================================
CTileManager::~CTileManager()
{
	for(int x = 0; x < TILES_SUM_X + 1; ++x) { delete[] m_Map[x]; }
	delete[] m_Map;
	m_Kartentextur.setNull();
}

//=================================================================================================
//
//=================================================================================================
void CTileManager::Init(SceneManager* SceneMgr)
{
	m_SceneManager = SceneMgr;
	m_StretchZ = 2;
	srand(1);
	m_Map = new _Map*[TILES_SUM_X+1];
	for (int x = 0; x < TILES_SUM_X + 1; ++x) { m_Map[x] = new _Map[TILES_SUM_Z + 1]; }
	Create_Map();
	CreateTextureGroup("terrain"); // only used to create a new texture group.
	CreateChunks();
}

//=================================================================================================
//
//=================================================================================================
void CTileManager::Create_Map()
{
	Image image;
	image.load(FILE_HEIGHT_MAP, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	uchar* heightdata_temp = image.getData();
	int dimx = image.getWidth(); 
	int dimy = image.getHeight();
	int posX = 0, posY;
	short Map[TILES_SUM_X+1][TILES_SUM_Z+1];
	///////////////////////////////////////////////////////////////////////// 
	// Fill the heightdata buffer with the image-color.
	///////////////////////////////////////////////////////////////////////// 
	for(int x = 0; x < TILES_SUM_X+1; ++x)
	{
		posY =0;
		for(int y = 0; y < TILES_SUM_Z+1; ++y)
		{
			if ( x && x != TILES_SUM_X && y && y != TILES_SUM_Z)
			{
				Map[x][y] = heightdata_temp[posY * dimx + posX];
			}
			else
			{
				Map[x][y] = 0;
			}
			if (++posY > dimy) posY =0; // if necessary, repeat the image.
		}
		if (++posX > dimx) posX =0; // if necessary, repeat the image.
	}

	for (int x = 0; x < TILES_SUM_X; ++x)
	{
		for (int y = 0; y < TILES_SUM_Z; ++y)
		{
			m_Map[x][y].height = (Map[x][y] + Map[x][y+1] + Map[x+1][y] + Map[x+1][y+1]) / 4;
		}
	}
	Set_Map_Textures();
}

	///////////////////////////////////////////////////////////////////////// 
	// Set the textures for the given height.
	///////////////////////////////////////////////////////////////////////// 
void CTileManager::Set_Map_Textures()
{
	short height;
	for (int x = 0; x < TILES_SUM_X; ++x)
	{
		for (int y = 0; y < TILES_SUM_Z; ++y)
		{
			height = m_Map[x][y].height;
			///////////////////////////////////////////////////////////////////////// 
			// Highland.
			///////////////////////////////////////////////////////////////////////// 
			if (height > LEVEL_MOUNTAIN_TOP)
			{
				m_Map[x][y].terrain_col = 0;
				m_Map[x][y].terrain_row = 1;
			}
			else if (height > LEVEL_MOUNTAIN_MID)
			{
				if (rand() % 2)
				{
					m_Map[x][y].terrain_col =6;
					m_Map[x][y].terrain_row =0;
				}
				else
				{
					m_Map[x][y].terrain_col = 0;
					m_Map[x][y].terrain_row = 0;
				}
			}
			else if (height > LEVEL_MOUNTAIN_DWN)
			{
				m_Map[x][y].terrain_col = rand() % 2 + 4;
				m_Map[x][y].terrain_row = 2;
			}
			///////////////////////////////////////////////////////////////////////// 
			// Plain.
			///////////////////////////////////////////////////////////////////////// 
			else if (height > LEVEL_PLAINS_TOP)
			{ // Plain
				m_Map[x][y].terrain_col = rand() % 2;
				m_Map[x][y].terrain_row = 2;
			}
			else if (height > LEVEL_PLAINS_MID)
			{ 
				m_Map[x][y].terrain_col = 6;
				m_Map[x][y].terrain_row = 3;
			}
			
			else if (height > LEVEL_PLAINS_DWN)
			{ 
				m_Map[x][y].terrain_col = 0;
				m_Map[x][y].terrain_row = 4;
			}
			else if (height > LEVEL_PLAINS_SUB)
			{ 
				m_Map[x][y].terrain_col = 3;
				m_Map[x][y].terrain_row = 3;
			}
			///////////////////////////////////////////////////////////////////////// 
			// Sea-Ground.
			///////////////////////////////////////////////////////////////////////// 
			else
			{
				m_Map[x][y].terrain_col = 3;
				m_Map[x][y].terrain_row = 3;
			}
		}
	}

}

//=================================================================================================
//
//=================================================================================================
void CTileManager::CreateChunks()
{
	long time = clock();
		CChunk::m_TileManagerPtr = this;
	CChunk::m_bounds = new AxisAlignedBox(
	 - TILE_SIZE * CHUNK_SIZE_X, 0               , -TILE_SIZE * CHUNK_SIZE_Z,
		 TILE_SIZE * CHUNK_SIZE_X, 100 * m_StretchZ,  TILE_SIZE * CHUNK_SIZE_Z);

	for (short x = 0; x < CHUNK_SUM_X; ++x)
	{
		for (short y = 0; y < CHUNK_SUM_Z; ++y)
		{
			m_mapchunk[x][y].Create(x, y);
		}
	}
	delete CChunk::m_bounds;
	Logger::log().info() << "Time to create Chunks: " << clock()-time << " ms";
}

//=================================================================================================
// Change Tile and Environmet textures.
//=================================================================================================
void CTileManager::ChangeTexture()
{
	static bool once = false;
	if (once) return;

	long time = clock();
	Image tMap;
	tMap.load("terrain_128_texture_2.png", "General");
	MaterialPtr mMaterial = MaterialManager::getSingleton().getByName("Land_HighDetails");
	std::string texName = "testMat";
	TexturePtr mTexture = TextureManager::getSingleton().loadImage(texName, "General", tMap, TEX_TYPE_2D, 3,1.0f);
	mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texName);
	mMaterial->load();
	Logger::log().info() << "Time to change Texture: " << clock()-time << " ms";
	once=true;
}

//=================================================================================================
//
//=================================================================================================
void CTileManager::ChangeChunks()
{
	long time = clock();
		CChunk::m_TileManagerPtr = this;
	CChunk::m_bounds = new AxisAlignedBox(
	 - TILE_SIZE * CHUNK_SIZE_X, 0               , -TILE_SIZE * CHUNK_SIZE_Z,
		 TILE_SIZE * CHUNK_SIZE_X, 100 * m_StretchZ,  TILE_SIZE * CHUNK_SIZE_Z);

	for (short x = 0; x < CHUNK_SUM_X; ++x)
	{
		for (short y = 0; y < CHUNK_SUM_Z; ++y)
		{
			m_mapchunk[x][y].Change(x, y);
		}
	}
	delete CChunk::m_bounds;
	Set_Map_Textures();
	Logger::log().info() << "Time to change Chunks: " << clock()-time << " ms";
}


//=================================================================================================
// +/- 5 Chunks around the camera are drawn in high quality.
//=================================================================================================
void CTileManager::ControlChunks(Vector3 vector)
{
	/////////////////////////////////////////////////////////////////////////
	/// Just for testing...
	/////////////////////////////////////////////////////////////////////////
//	ChangeChunks();

	int x = (int)vector.x / (TILE_SIZE * CHUNK_SIZE_X)+1;
	int y = (int)vector.z / (TILE_SIZE * CHUNK_SIZE_Z)+1;
	if ( x > CHUNK_SUM_X || y > CHUNK_SUM_Z) { return; }

	for(int cx = 0; cx < CHUNK_SUM_X; ++cx)
	{
		for (int cy = 0; cy < CHUNK_SUM_Z; ++cy)
		{
			#ifndef LOW_QUALITY_RENDERING
			if (cx >= x - HIGH_QUALITY_RANGE && cx <= x + HIGH_QUALITY_RANGE
			 && cy >= y - HIGH_QUALITY_RANGE && cy <= y + HIGH_QUALITY_RANGE)
			{
				m_mapchunk[cx][cy].Attach(QUALITY_HIGH);
			}
			else
			#endif
			{
				m_mapchunk[cx][cy].Attach(QUALITY_LOW);
			}
		}
	}
}

//=================================================================================================
// Create the texture-file out of the single textures + filter texture.
//=================================================================================================
void CTileManager::CreateTextureGroup(const char *terrain_type)
{
	const int PIXEL = (int)PIXEL_PER_TILE;
	const int TEXTURES_PER_ROW = 7; // Texture is a quad, so we have (x^2)-1 textures in the main-texture.
	const int SIZE = (int) PIXEL_PER_ROW;
	const int BUFFER_SIZE =80;
	char filename[BUFFER_SIZE+1];

	/////////////////////////////////////////////////////////////////////////
	// Create grid texture.
	/////////////////////////////////////////////////////////////////////////
	Image grid;
	uchar* grid_data = new uchar[PIXEL * PIXEL * 4];
	grid.loadDynamicImage(grid_data, PIXEL, PIXEL, PF_R8G8B8A8);

	for (int x = 0; x != PIXEL; ++x)
	{
		for (int y = 0; y != PIXEL; ++y)
		{
			if ( x == 0 || y == 0 || x == PIXEL - 1 || y == PIXEL -1)
			{
				grid_data[4*(PIXEL*y + x) + 0] = 255;
				grid_data[4*(PIXEL*y + x) + 1] = 50;
				grid_data[4*(PIXEL*y + x) + 2] = 50;
				grid_data[4*(PIXEL*y + x) + 3] = 50;
			}
			else
			{
				grid_data[4*(PIXEL*y + x) + 0] = 0;
				grid_data[4*(PIXEL*y + x) + 1] = 0;
				grid_data[4*(PIXEL*y + x) + 2] = 0;
				grid_data[4*(PIXEL*y + x) + 3] = 0;
			}
		}
	}
	sprintf(filename,"%s%s%.3d%s",PATH_TILE_TEXTURES, "grid_", PIXEL, ".png");
	grid.save(filename);

	/////////////////////////////////////////////////////////////////////////
	// Create a ground texture.
	/////////////////////////////////////////////////////////////////////////
	
	Image Filter, Texture; // Needed Textures.
	
	int i=-1, x=0, y = 0;

	// shrink filter

	snprintf(filename, BUFFER_SIZE, "filter_%.3d.png", PIXEL);
	Filter.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	shrinkFilter(Filter, PIXEL);

	// shrink all textures
	while(1)
	{
		// Check if the texture-file exists.
		sprintf(filename, "%s%s_%.2d_%.3d.png", PATH_TILE_TEXTURES, terrain_type, ++i, PIXEL);
		if (access(filename, 0) == -1) { break; }
		// Load the texture-file.
		sprintf(filename, "%s_%.2d_%.3d.png", terrain_type, i, PIXEL);
		Texture.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		shrinkTexture(Texture, i, PIXEL, terrain_type);
	}

	int pix = PIXEL;

	Image TextureGroup; // Will be created.

	while (pix >= MIN_TEXTURE_PIXEL)
	{
		uchar* TextureGroup_data = new uchar[SIZE * SIZE *4];
		TextureGroup.loadDynamicImage(TextureGroup_data, pix * 8, pix * 8,PF_A8B8G8R8);

		snprintf(filename, BUFFER_SIZE, "filter_%.3d.png", pix);
		Filter.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		uchar* Filter_data = Filter.getData();

		i = -1; x=0, y = 0;

		while(1)
		{
			// Check if the texture-file exists.
			sprintf(filename, "%s%s_%.2d_%.3d.png", PATH_TILE_TEXTURES, terrain_type, ++i, pix);
			if (access(filename, 0) == -1) { break; }
			// Load the texture-file.
			sprintf(filename, "%s_%.2d_%.3d.png", terrain_type, i, pix);
			Texture.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

			addToGroupTexture(TextureGroup_data, Filter_data, &Texture, pix, TEXTURES_PER_ROW, x, y);
		
			if (++x == TEXTURES_PER_ROW)
			{
				if (++y == TEXTURES_PER_ROW) { break; }
				x = 0;
			}
		}
	
		sprintf(filename,"%s%s_%.3d_texture.png",PATH_TILE_TEXTURES, terrain_type, pix);
		TextureGroup.save(filename);
		delete []TextureGroup_data;

		pix /= 2;
	}
}

//=================================================================================================
// Create shrinked textures of a texture
//=================================================================================================

void CTileManager::shrinkTexture(const Image& Texture, const int num, const int PIXEL, const char *terrain_type)
{
	const int BUFFER_SIZE =80;
	const uchar* Texture_data = Texture.getData();

	char filename[BUFFER_SIZE+1];
	
	int pix = PIXEL / 2;

	Image Texture_shrink;
	Image Texture_previous;
	uchar* Texture_shrink_data = new uchar[pix * pix *3];

	while (pix >= MIN_TEXTURE_PIXEL)
	{
		sprintf(filename, "%s_%.2d_%.3d.png", terrain_type, num, pix * 2);
		Texture_previous.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		const uchar* Texture_previous_data = Texture_previous.getData();

		// create shrinked images, each time with half the pixel size (128x128 => 64x64 => 32x32 => ...)
		
		Texture_shrink.loadDynamicImage(Texture_shrink_data, pix, pix,PF_B8G8R8);

		// calculate arithmetic mean for new image (2x2 pixels old image => 1 pixel new image)

		for (int x = 0; x < pix; ++x)
		{
			for (int y = 0; y < pix; ++y)
			{
				for (int z = 0; z < 3; ++z)
				{
					Texture_shrink_data[3* (pix * y+ x) + z] = (
						Texture_previous_data[3* (2*pix * 2*y + 2* x) + z] +
						Texture_previous_data[3* (2*pix * 2*y + 2* x+1) + z] +
						Texture_previous_data[3* (2*pix * 2*y + 2*pix + 2* x) + z]) / 3;
				}
			}
		}

		sprintf(filename, "%s%s_%.2d_%.3d.png", PATH_TILE_TEXTURES, terrain_type, num, pix);
		Texture_shrink.save(filename);
		
		// devide pixel size by 2
		pix /= 2;
	}
}

void CTileManager::shrinkFilter(const Image& Filter, const int PIXEL)
{
	const int BUFFER_SIZE =80;
	const uchar* Filter_data = Filter.getData();

	char filename[BUFFER_SIZE+1];
	
	int pix = PIXEL / 2;

	Image Filter_shrink;
	Image Filter_previous;
	uchar* Filter_shrink_data = new uchar[pix * pix *3];

	while (pix >= MIN_TEXTURE_PIXEL)
	{
		sprintf(filename, "filter_%.3d.png", pix * 2);
		Filter_previous.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		const uchar* Filter_previous_data = Filter_previous.getData();

		// create shrinked filters, each time with half the pixel size (128x128 => 64x64 => 32x32 => ...)
		
		Filter_shrink.loadDynamicImage(Filter_shrink_data, pix, pix,PF_B8G8R8);

		// calculate arithmetic mean for new image (2x2 pixels old image => 1 pixel new image)

		for (int x = 0; x < pix; ++x)
		{
			for (int y = 0; y < pix; ++y)
			{
				for (int z = 0; z < 3; ++z)
				{
					Filter_shrink_data[3* (pix * y+ x) + z] = (
						Filter_previous_data[3* (2*pix * 2*y + 2* x) + z] +
						Filter_previous_data[3* (2*pix * 2*y + 2* x+1) + z] +
						Filter_previous_data[3* (2*pix * 2*y + 2*pix + 2* x) + z]) / 3;
				}
			}
		}

		sprintf(filename, "%sfilter_%.3d.png", PATH_TILE_TEXTURES, pix);
		Filter_shrink.save(filename);

		// devide pixel size by 2
		pix /= 2;
	}
}

//=================================================================================================
// Add a texture to the all group-texture.
//=================================================================================================
inline void CTileManager::addToGroupTexture(uchar* TextureGroup_data, uchar *Filter_data, Image* Texture, short pix, short size, short x, short y)
{
	unsigned long index1;
	unsigned long index2;

	uchar* Texture_data = Texture->getData();
	
	int space = pix / 16;


	for (int i = 0; i < pix; ++i)
	{
		for (int j = 0; j < pix; ++j)
		{
		
			index1 = 4*(pix * 8)* (pix + 2 * space) * y 
				+ 4* (pix * 8) * (i + space) 
				+ 4* x * (pix + 2* space) 
				+ 4* (j + space);

			index2 = 3* pix * i + 3 * j;
	
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data [index2];
		}
	}

	
	/////////////////////////////////////////////////////////////////////////
	// left border creation
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != pix; ++i)
	{
		for (int j = 0; j!= space ; ++j)
		{
			index1 = 4* (pix * 8) * (pix + 2 * space) * y
				+ 4* (pix * 8) * (i + space)
				+ 4* x * (pix + 2* space) + 
				4* j;
			
			index2 = 3* pix * i + 3 * (pix - space + j);
			
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2];
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// right border creation
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != pix; ++i)
	{
		for (int j = 0; j!= space ; ++j)
		{
			index1 = 4* (pix * 8) * (pix + 2 * space) * y
				+ 4* (pix * 8) * (i + space)
				+ 4* x * (pix + 2* space) + 
				4* (pix + space + j);

			index2 = 3* pix * i + 3 * j;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2];
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// upper border creation
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != space; ++i)
	{
		for (int j = 0; j!= pix; ++j)
		{
			index1 = 4* (pix * 8) * (pix + 2 * space) * y
				+ 4* (pix * 8) * i
				+ 4* x * (pix + 2* space) + 
				4* (space + j);

			index2 = 3* pix * (pix - space + i) + 3 * j;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2];
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// lower border creation
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != space; ++i)
	{
		for (int j = 0; j!= pix; ++j)
		{
			index1 = 4* (pix * 8) * (pix + 2 * space) * y
				+ 4* (pix * 8) * (pix + space + i)
				+ 4* x * (pix + 2* space) + 
				4* (space + j);

			index2 = 3* pix * i + 3 * j;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2];
		}
	}
	/*
	/////////////////////////////////////////////////////////////////////////
	// remaining 4 edges
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != pix / 16 + 1; ++i)
	{
		for (int j = 0; j!= pix / 16 + 1; ++j)
		{
			index1 = (4* (pix + 2*(pix / 16 + 1)) * size + pix / 16)* (pix + 2*(pix / 16 + 1)) * y + 
				(4* (pix + 2*(pix / 16 + 1)) * size + pix / 16) * i + 4* x * (pix + 2*(pix / 16 + 1)) 
				+ 4* j;
			index2 = 0;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}

	for (int i = 0; i != pix / 16 + 1; ++i)
	{
		for (int j = pix + pix/ 16 + 1; j!= pix + 2 * (pix / 16 + 1); ++j)
		{
			index1 = (4* (pix + 2*(pix / 16 + 1)) * size + pix / 16)* (pix + 2*(pix / 16 + 1)) * y + 
				(4* (pix + 2*(pix / 16 + 1)) * size + pix / 16) * i + 4* x * (pix + 2*(pix / 16 + 1)) 
				+ 4* j;
			index2 = 0;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}

	for (int i = pix + pix / 16 + 1; i !=  pix + 2 * (pix / 16 + 1); ++i)
	{
		for (int j = 0; j!= pix / 16 + 1; ++j)
		{
			index1 = (4* (pix + 2*(pix / 16 + 1)) * size + pix / 16)* (pix + 2*(pix / 16 + 1)) * y + 
				(4* (pix + 2*(pix / 16 + 1)) * size + pix / 16) * i + 4* x * (pix + 2*(pix / 16 + 1)) 
				+ 4* j;
			index2 = 0;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}

	for (int i = pix + pix / 16 + 1; i !=  pix + 2*(pix / 16 + 1); ++i)
	{
		for (int j = pix + pix / 16 + 1; j!= pix + 2*(pix / 16 + 1); ++j)
		{
			index1 = (4* (pix + 2*(pix / 16 + 1)) * size + pix / 16)* (pix + 2*(pix / 16 + 1)) * y + 
				(4* (pix + 2*(pix / 16 + 1)) * size + pix / 16) * i + 4* x * (pix + 2*(pix / 16 + 1)) 
				+ 4* j;
			index2 = 0;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}

	*/
}

//=================================================================================================
//
//=================================================================================================
void CTileManager::SwitchMaterial(bool grid, bool filter)
{
	Entity *entity;
	/////////////////////////////////////////////////////////////////////////
	// Display Grid.
	/////////////////////////////////////////////////////////////////////////
	if (grid)
	{
		if (filter == true)
		{
			for (int x = 0; x < CHUNK_SUM_X; ++x)
			{
				for (int y = 0; y < CHUNK_SUM_Z; ++y)
				{
					entity = m_mapchunk[x][y].Get_Land_entity();
					if (entity) { entity->setMaterialName("Land_HighDetails_Grid"); }
					entity = m_mapchunk[x][y].Get_Water_entity();
					if (entity) { entity->setMaterialName("Water_HighDetails_Grid"); }
				}
			}
		}
		else
		{
			for (int x = 0; x < CHUNK_SUM_X; ++x)
			{
				for (int y = 0; y < CHUNK_SUM_Z; ++y)
				{
					entity = m_mapchunk[x][y].Get_Land_entity();
					if (entity) { entity->setMaterialName("Land_LowDetails_Grid"); }
					entity = m_mapchunk[x][y].Get_Water_entity();
					if (entity) { entity->setMaterialName("Water_LowDetails_Grid"); }
				}
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////
	// Don't display grid.
	/////////////////////////////////////////////////////////////////////////
	else
	{
		if (filter == true)
		{
			for (int x = 0; x < CHUNK_SUM_X; ++x)
			{
				for (int y = 0; y < CHUNK_SUM_Z; ++y)
				{
					entity = m_mapchunk[x][y].Get_Land_entity();
					if (entity) { entity->setMaterialName("Land_HighDetails"); }
					entity = m_mapchunk[x][y].Get_Water_entity();
					if (entity) { entity->setMaterialName("Water_HighDetails"); }
				}
			}
		}
		else
		{
			for (int x = 0; x < CHUNK_SUM_X; ++x)
			{
				for (int y = 0; y < CHUNK_SUM_Z; ++y)
				{
					entity = m_mapchunk[x][y].Get_Land_entity();
					if (entity) { entity->setMaterialName("Land_LowDetails"); }
					entity = m_mapchunk[x][y].Get_Water_entity();
					if (entity) { entity->setMaterialName("Water_LowDetails"); }
				}
			}
		}
	}
}
