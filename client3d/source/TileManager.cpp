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

#ifdef LOG_TIMING
#include <ctime>
#endif
#include <iostream>
#include "OgreHardwarePixelBuffer.h"
#include "TileChunk.h"
#include "TileManager.h"
#include "logger.h"

//#define LOW_QUALITY_RENDERING

///=================================================================================================
/// Constructor
///=================================================================================================
TileManager::TileManager() 
{
}

///=================================================================================================
/// Destructor
///=================================================================================================
TileManager::~TileManager()
	{
	for(int x = 0; x < TILES_SUM_X + 1; ++x) { delete[] m_Map[x]; }
	delete[] m_Map;
	}

///=================================================================================================
/// Init the TileEngine.
///=================================================================================================
void TileManager::Init(SceneManager* SceneMgr, int tileTextureSize, int tileStretchZ)
{
	Logger::log().headline("Starting TileEngine:");
	m_SceneManager = SceneMgr;
	m_StretchZ = tileStretchZ;
	m_TileTextureSize = tileTextureSize;
	srand(1);
	Logger::log().info() << "Creating map";
	m_Map = new WorldMap*[TILES_SUM_X+1];
	for (int x = 0; x < TILES_SUM_X + 1; ++x) { m_Map[x] = new WorldMap[TILES_SUM_Z + 1]; }
	Load_Map(FILE_HEIGHT_MAP);
	/////////////////////////////////////////////////////////////////////////
	/// Create all TextureGroups.
	/////////////////////////////////////////////////////////////////////////
	std::string strTextureGroup = "terrain";
	Logger::log().info() << "Creating texture group " << strTextureGroup;
	CreateTextureGroup(strTextureGroup); // only used once, to create a new texture group.
	/////////////////////////////////////////////////////////////////////////
	/// Create TileChunks.
	/////////////////////////////////////////////////////////////////////////
	Logger::log().info() << "Creating tile-chunks";
	CreateChunks();
	/////////////////////////////////////////////////////////////////////////
	/// Init is done.
	/////////////////////////////////////////////////////////////////////////
	Logger::log().info() << "Init done.";
	Logger::log().headline("Running TileEngine:");
}

///=================================================================================================
/// Create the worldmap.
///=================================================================================================
void TileManager::Load_Map(const std::string &png_filename)
{
	Image image;
	image.load(png_filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	uchar* heightdata_temp = image.getData();
	int dimx = image.getWidth();
	int dimy = image.getHeight();
	int posX = 0, posY;
	short Map[TILES_SUM_X+2][TILES_SUM_Z+2];
	/////////////////////////////////////////////////////////////////////////
	/// Fill the heightdata buffer with the image-color.
	/////////////////////////////////////////////////////////////////////////
	for(int x = 0; x < TILES_SUM_X+2; ++x)
	{
		posY =0;
		for(int y = 0; y < TILES_SUM_Z+2; ++y)
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

	for (int x = 0; x < TILES_SUM_X+1; ++x)
	{
		for (int y = 0; y < TILES_SUM_Z+1; ++y)
		{
			m_Map[x][y].height = (Map[x][y] + Map[x][y+1] + Map[x+1][y] + Map[x+1][y+1]) / 4;
		}
	}
	Set_Map_Textures();
}

///=================================================================================================
/// Set the textures for the given height.
///=================================================================================================
void TileManager::Set_Map_Textures()
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
//				if (rand() % 2)
				{
					m_Map[x][y].terrain_col =6;
					m_Map[x][y].terrain_row =0;
				}
/*
				else
				{
					m_Map[x][y].terrain_col = 0;
					m_Map[x][y].terrain_row = 0;
				}
			*/}
			else if (height > LEVEL_MOUNTAIN_DWN)
			{
				m_Map[x][y].terrain_col = 5;//rand() % 2 + 4;
				m_Map[x][y].terrain_row = 2;
			}
			/////////////////////////////////////////////////////////////////////////
			/// Plain.
			/////////////////////////////////////////////////////////////////////////
			else if (height > LEVEL_PLAINS_TOP)
			{ // Plain
				m_Map[x][y].terrain_col = 2;//rand() % 2;
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
			/// Sea-Ground.
			/////////////////////////////////////////////////////////////////////////
			else
			{
				m_Map[x][y].terrain_col = 3;
				m_Map[x][y].terrain_row = 3;
			}
		}
	}
}

///=================================================================================================
/// Create all chunks.
///=================================================================================================
void TileManager::CreateChunks()
{
	#ifdef LOG_TIMING
	long time = clock();
	#endif
	TileChunk::m_TileManagerPtr = this;
	TileChunk::m_bounds = new AxisAlignedBox(
		 -TILE_SIZE * CHUNK_SIZE_X, 0             , -TILE_SIZE * CHUNK_SIZE_Z,
			TILE_SIZE * CHUNK_SIZE_X, 100 * m_StretchZ,  TILE_SIZE * CHUNK_SIZE_Z);

	for (short x = 0; x < CHUNK_SUM_X; ++x)
	{
		for (short y = 0; y < CHUNK_SUM_Z; ++y)
		{
			m_mapchunk[x][y].Create(x, y);
		}
	}
	delete TileChunk::m_bounds;
	#ifdef LOG_TIMING
	Logger::log().info() << "Time to create Chunks: " << clock()-time << " ms";
	#endif
}

///=================================================================================================
/// Change all Chunks.
///=================================================================================================
void TileManager::ChangeChunks()
	{
	#ifdef LOG_TIMING
	long time = clock();
	#endif
	TileChunk::m_TileManagerPtr = this;
	TileChunk::m_bounds = new AxisAlignedBox(
		 -TILE_SIZE * CHUNK_SIZE_X, 0               , -TILE_SIZE * CHUNK_SIZE_Z,
			TILE_SIZE * CHUNK_SIZE_X, 100 * m_StretchZ,  TILE_SIZE * CHUNK_SIZE_Z);
/*
	unsigned char value;
	for (int a = 0 ; a < TILES_SUM_X; ++a)
	{
		for (int b = 0; b < TILES_SUM_Z; ++b)
		{
			value = Get_Map_Height(a, b)+ 1;
			if (value > 220) value = 0;
			Set_Map_Height(a, b, value);
		}
	}
*/
	for (short x = 0; x < CHUNK_SUM_X; ++x)
	{
		for (short z = 0; z < CHUNK_SUM_Z; ++z)
		{
			m_mapchunk[x][z].Change();
		}
	}
	delete TileChunk::m_bounds;
	Set_Map_Textures();
	#ifdef LOG_TIMING
	Logger::log().info() << "Time to change Chunks: " << clock()-time << " ms";
	#endif
}

///=================================================================================================
/// Change Tile and Environmet textures.
///=================================================================================================
void TileManager::ChangeTexture()
{
	static bool once = false;
	if (once) return; else once=true;
	#ifdef LOG_TIMING
	long time = clock();
	#endif
	Image tMap;
	tMap.load("terrain_016_texture.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	MaterialPtr mMaterial = MaterialManager::getSingleton().getByName("Land_HighDetails128");
	std::string texName = "testMat";
	TexturePtr mTexture = TextureManager::getSingleton().loadImage(texName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, tMap, TEX_TYPE_2D, 3,1.0f);
	mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texName);
//	mMaterial->unload();
	mMaterial->load();
	#ifdef LOG_TIMING
	Logger::log().info() << "Time to change Texture: " << clock()-time << " ms";
	#endif
}

///=================================================================================================
/// +/- 5 Chunks around the camera are drawn in high quality.
///=================================================================================================
void TileManager::ControlChunks(Vector3 vector)
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

///=================================================================================================
/// If the file exists load it into an image.
///=================================================================================================
bool TileManager::LoadImage(Image &image, const std::string &strFilename)
{
	std::string strTemp = PATH_TILE_TEXTURES + strFilename;
	std::ifstream chkFile;
	chkFile.open(strTemp.data());
	if (!chkFile){ chkFile.close(); return false; }
	chkFile.close();
	image.load(strFilename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	return true;
}

///=================================================================================================
/// Create the texture-file out of the single textures + filter texture.
///=================================================================================================
bool TileManager::CreateTextureGroup(const std::string &terrain_type)
{
	std::string strFilename;
	bool hasFilter = true;
	/////////////////////////////////////////////////////////////////////////
	/// Create grid texture.
	/////////////////////////////////////////////////////////////////////////
	Image grid;
	uchar* grid_data = new uchar[PIXEL_PER_TILE * PIXEL_PER_TILE * 4];
	grid.loadDynamicImage(grid_data, PIXEL_PER_TILE, PIXEL_PER_TILE, PF_R8G8B8A8);
	for (int x = 0; x < PIXEL_PER_TILE; ++x)
	{
		for (int y = 0; y < PIXEL_PER_TILE; ++y)
		{
			if ( x == 0 || y == 0 || x == PIXEL_PER_TILE - 1 || y == PIXEL_PER_TILE -1)
			{
				grid_data[4*(PIXEL_PER_TILE*y + x) + 0] = 255;
				grid_data[4*(PIXEL_PER_TILE*y + x) + 1] = 50;
				grid_data[4*(PIXEL_PER_TILE*y + x) + 2] = 50;
				grid_data[4*(PIXEL_PER_TILE*y + x) + 3] = 50;
			}
			else
			{
				grid_data[4*(PIXEL_PER_TILE*y + x) + 0] = 0;
				grid_data[4*(PIXEL_PER_TILE*y + x) + 1] = 0;
				grid_data[4*(PIXEL_PER_TILE*y + x) + 2] = 0;
				grid_data[4*(PIXEL_PER_TILE*y + x) + 3] = 0;
			}
		}
	}
	strFilename = PATH_TILE_TEXTURES;
	strFilename+= "grid_" + StringConverter::toString(PIXEL_PER_TILE, 3, '0') + ".png";
	grid.save(strFilename);
	Image Texture, Filter;
	/////////////////////////////////////////////////////////////////////////
	/// Shrink all filter-textures.
	/////////////////////////////////////////////////////////////////////////
	strFilename = "filter_" + StringConverter::toString(PIXEL_PER_TILE, 3, '0') + ".png";
	if (!LoadImage(Filter, strFilename))
	{
		Logger::log().error() << "Filter texture '" << strFilename << "' was not found.";
		return false;
	}
	shrinkFilter(Filter);
	/////////////////////////////////////////////////////////////////////////
	/// Shrink all tile-textures.
	/////////////////////////////////////////////////////////////////////////
	int i=-1, x=0, y = 0;
	while(1)
	{
		strFilename = terrain_type;
		strFilename+= "_"+ StringConverter::toString(++i, 2, '0');
		strFilename+= "_"+ StringConverter::toString(PIXEL_PER_TILE, 3, '0') + ".png";
		if (!LoadImage(Texture, strFilename)) { break; }
		shrinkTexture(Texture, i, terrain_type);
	}
	Logger::log().info() << "Found " << StringConverter::toString(i,2,'0') << " textures for group '" << terrain_type << "'.";
	/////////////////////////////////////////////////////////////////////////
	/// Create group-texture in various sizes.
	/////////////////////////////////////////////////////////////////////////
	int pix = PIXEL_PER_TILE;
	Image TextureGroup; // Will be created.
	while (pix >= MIN_TEXTURE_PIXEL)
	{
			uchar* TextureGroup_data = new uchar[PIXEL_PER_ROW * PIXEL_PER_ROW *4];
		TextureGroup.loadDynamicImage(TextureGroup_data, pix * 8, pix * 8,PF_A8B8G8R8);
		strFilename = "filter_" + StringConverter::toString(pix, 3, '0') + ".png";
		if (!LoadImage(Filter, strFilename))
		{
			Logger::log().error() << "Filter texture '" << strFilename << "' was not found.";
			return false;
		}
		uchar* Filter_data = Filter.getData();
		i = -1; x=0, y = 0;
		while(1)
		{
			strFilename = terrain_type;
			strFilename+= "_"+ StringConverter::toString(++i, 2, '0');
			strFilename+= "_"+ StringConverter::toString(pix, 3, '0') + ".png";
			if (!LoadImage(Texture, strFilename)) { break; }
			addToGroupTexture(TextureGroup_data, Filter_data, &Texture, pix, TEXTURES_PER_ROW, x, y);
			if (++x == TEXTURES_PER_ROW)
			{
				if (++y == TEXTURES_PER_ROW) { break; }
				x = 0;
			}
		}
		strFilename = PATH_TILE_TEXTURES + terrain_type + "_texture";
		strFilename+= "_"+ StringConverter::toString(pix, 3, '0')+".png";
		TextureGroup.save(strFilename);
		
		delete []TextureGroup_data;
		pix /= 2;
	}

	MaterialPtr mMaterial128 = MaterialManager::getSingleton().getByName("Land_HighDetails128");
	MaterialPtr mMaterial64 = MaterialManager::getSingleton().getByName("Land_HighDetails064");
	MaterialPtr mMaterial32 = MaterialManager::getSingleton().getByName("Land_HighDetails032");
	MaterialPtr mMaterial16 = MaterialManager::getSingleton().getByName("Land_HighDetails016");
	MaterialPtr mMaterial8 = MaterialManager::getSingleton().getByName("Land_HighDetails008");

	mMaterial128->load();
	mMaterial64->load();
	mMaterial32->load();
	mMaterial16->load();
	mMaterial8->load();

	String texname = mMaterial128->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
	TexturePtr mTexture128 = (TexturePtr) TextureManager::getSingleton().getByName(texname);

	texname = mMaterial64->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
	TexturePtr mTexture64 = (TexturePtr) TextureManager::getSingleton().getByName(texname);

	texname = mMaterial32->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
	TexturePtr mTexture32 = (TexturePtr) TextureManager::getSingleton().getByName(texname);

	texname = mMaterial16->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
	TexturePtr mTexture16 = (TexturePtr) TextureManager::getSingleton().getByName(texname);

	texname = mMaterial8->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
	TexturePtr mTexture8 = (TexturePtr) TextureManager::getSingleton().getByName(texname);

	HardwarePixelBufferSharedPtr mBuffer64 = mTexture64->getBuffer(0,0);
	HardwarePixelBufferSharedPtr mBuffer32 = mTexture32->getBuffer(0,0);
	HardwarePixelBufferSharedPtr mBuffer16 = mTexture16->getBuffer(0,0);
	HardwarePixelBufferSharedPtr mBuffer8 = mTexture8->getBuffer(0,0);

	HardwarePixelBufferSharedPtr mBuffer128_64 = mTexture128->getBuffer(0,1);
	HardwarePixelBufferSharedPtr mBuffer128_32 = mTexture128->getBuffer(0,2);
	HardwarePixelBufferSharedPtr mBuffer128_16 = mTexture128->getBuffer(0,3);
	HardwarePixelBufferSharedPtr mBuffer128_8 = mTexture128->getBuffer(0,4);

	mBuffer128_64->blit(mBuffer64.getPointer());
	mBuffer128_32->blit(mBuffer32.getPointer());
	mBuffer128_16->blit(mBuffer16.getPointer());
	mBuffer128_8->blit(mBuffer8.getPointer());

	HardwarePixelBufferSharedPtr mBuffer64_32 = mTexture64->getBuffer(0,1);
	HardwarePixelBufferSharedPtr mBuffer64_16 = mTexture64->getBuffer(0,2);
	HardwarePixelBufferSharedPtr mBuffer64_8 = mTexture64->getBuffer(0,3);

	mBuffer64_32->blit(mBuffer32.getPointer());
	mBuffer64_16->blit(mBuffer16.getPointer());
	mBuffer64_8->blit(mBuffer8.getPointer());

	HardwarePixelBufferSharedPtr mBuffer32_16 = mTexture32->getBuffer(0,1);
	HardwarePixelBufferSharedPtr mBuffer32_8 = mTexture32->getBuffer(0,2);

	mBuffer32_16->blit(mBuffer16.getPointer());
	mBuffer32_8->blit(mBuffer8.getPointer());

	HardwarePixelBufferSharedPtr mBuffer16_8 = mTexture16->getBuffer(0,1);

	mBuffer16_8->blit(mBuffer8.getPointer());

	return true;
}

///=================================================================================================
/// Shrink Texture.
///=================================================================================================
void TileManager::shrinkTexture(const Image& Texture, const int num, const std::string &terrain_type)
{
	const uchar* Texture_data = Texture.getData();
	int pix = PIXEL_PER_TILE / 2;
	Image Texture_shrink;
	Image Texture_previous;
	uchar* Texture_shrink_data = new uchar[pix * pix *3];
	std::string strFilename;
	while (pix >= MIN_TEXTURE_PIXEL)
	{
		strFilename = terrain_type;
		strFilename+= "_"+ StringConverter::toString(num, 2, '0');
		strFilename+= "_"+ StringConverter::toString(pix+pix, 3, '0') + ".png";
		Texture_previous.load(strFilename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
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
					Texture_shrink_data  [3* (pix * y+ x) + z] = (
					Texture_previous_data[3* (2*pix * 2*y + 2* x) + z] +
					Texture_previous_data[3* (2*pix * 2*y + 2* x+1) + z] +
					Texture_previous_data[3* (2*pix * 2*y + 2*pix + 2* x) + z] +
					Texture_previous_data[3* (2*pix * 2*y + 2*pix + 2* x+1) + z]) / 4;
				}
			}
		}
		strFilename = PATH_TILE_TEXTURES;
		strFilename+= terrain_type;
		strFilename+= "_"+ StringConverter::toString(num, 2, '0');
		strFilename+= "_"+ StringConverter::toString(pix, 3, '0') + ".png";
		Texture_shrink.save(strFilename);
		pix /= 2;
	}
}

///=================================================================================================
/// Shrink the filter.
///=================================================================================================
void TileManager::shrinkFilter(const Image& Filter)
{
	const uchar* Filter_data = Filter.getData();
	int pix = PIXEL_PER_TILE / 2;
	Image Filter_shrink;
	Image Filter_previous;
	uchar* Filter_shrink_data = new uchar[pix * pix *3];
	std::string strFilename;
	while (pix >= MIN_TEXTURE_PIXEL)
	{
		strFilename = "filter_" + StringConverter::toString(pix+pix, 3, '0') + ".png";
		Filter_previous.load(strFilename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
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
					Filter_previous_data[3* (2*pix * 2*y + 2*pix + 2* x) + z] +
					Filter_previous_data[3* (2*pix * 2*y + 2*pix + 2* x+1) + z]) / 4;
				}
			}
		}
		strFilename = PATH_TILE_TEXTURES;
		strFilename+= "filter_" + StringConverter::toString(pix, 3, '0') + ".png";
		Filter_shrink.save(strFilename);
		pix /= 2;
	}
}

///=================================================================================================
/// Add a texture to the terrain-texture.
///=================================================================================================
inline void TileManager::addToGroupTexture(uchar* TextureGroup_data, uchar *Filter_data, Image* Texture, short pix, short size, short x, short y)
{
	unsigned long index1, index2;
	uchar* Texture_data = Texture->getData();
	int space = pix / 16;

	if (pix > 8)
	{
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
	}
	else
	{
		for (int i = 0; i < pix; ++i)
		{
			for (int j = 0; j < pix; ++j)
			{
				index1 = 4*(pix * 8)* (pix + 1) * y
						 + 4* (pix * 8) * (i + space)
						 + 4* x * (pix + 1)
						 + 4* (j + space);
				index2 = 3* pix * i + 3 * j;
				TextureGroup_data[  index1] = Texture_data[index2];
				TextureGroup_data[++index1] = Texture_data[index2 + 1];
				TextureGroup_data[++index1] = Texture_data[index2 + 2];
				TextureGroup_data[++index1] = Filter_data [index2];
			}
		}
		
		for (int j = 0; j < pix; ++j)
		{
			int i = pix;

			index1 = 4*(pix * 8)* (pix+1) * y
						 + 4* (pix * 8) * (i + space)
						 + 4* x * (pix + 1)
						 + 4* (j + space);
			index2 = 3* pix * (i-1) + 3 * j;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data [index2];
		}

		for (int i = 0; i < pix; ++i)
		{
			int j = pix;
			index1 = 4*(pix * 8)* (pix + 1) * y
						 + 4* (pix * 8) * (i + space)
						 + 4* x * (pix + 1)
						 + 4* (j + space);
			index2 = 3* pix * i + 3 * (j-1);
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data [index2];
		}

		int i = pix;
		int j = pix;

		index1 = 4*(pix * 8)* (pix + 1) * y
						 + 4* (pix * 8) * (i + space)
						 + 4* x * (pix + 1)
						 + 4* (j + space);
			index2 = 3* pix * (i-1) + 3 * (j-1);
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data [index2];
	}
	if (pix > 8)
	{
		/////////////////////////////////////////////////////////////////////////
		/// left border creation
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
				TextureGroup_data[++index1] = 255 - Filter_data[index2];
			}
		}
		/////////////////////////////////////////////////////////////////////////
		/// right border creation
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
				TextureGroup_data[++index1] = 255 - Filter_data[index2];
			}
		}
		/////////////////////////////////////////////////////////////////////////
		/// upper border creation
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
				TextureGroup_data[++index1] = 255 - Filter_data[index2];
			}
		}
		/////////////////////////////////////////////////////////////////////////
		/// lower border creation
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
				TextureGroup_data[++index1] = 255 - Filter_data[index2];
			}
		}
		/////////////////////////////////////////////////////////////////////////
		/// remaining 4 edges
		/////////////////////////////////////////////////////////////////////////
		// upper left
		for (int i = 0; i != space; ++i)
		{
			for (int j = 0; j!= space; ++j)
			{
				index1 = 4*(pix * 8)* (pix + 2 * space) * y
							+ 4* (pix * 8) * i
							+ 4* x * (pix + 2* space)
							+ 4* j;
				index2 = 0;
				TextureGroup_data[  index1] = Texture_data[index2];
				TextureGroup_data[++index1] = Texture_data[index2 + 1];
				TextureGroup_data[++index1] = Texture_data[index2 + 2];
				TextureGroup_data[++index1] = 255;
			}
		}
		// upper right
		for (int i = pix + space; i != pix + 2*space; ++i)
		{
			for (int j = 0; j!= space; ++j)
			{
				index1 = 4*(pix * 8)* (pix + 2 * space) * y
							+ 4* (pix * 8) * i
							+ 4* x * (pix + 2* space)
							+ 4* j;
				index2 = 0;
				TextureGroup_data[  index1] = Texture_data[index2];
				TextureGroup_data[++index1] = Texture_data[index2 + 1];
				TextureGroup_data[++index1] = Texture_data[index2 + 2];
				TextureGroup_data[++index1] = 255;
			}
		}
		// lower left
		for (int i = 0; i != space; ++i)
		{
			for (int j = pix + space; j!= pix + 2* space; ++j)
			{
				index1 = 4*(pix * 8)* (pix + 2 * space) * y
							+ 4* (pix * 8) * i
							+ 4* x * (pix + 2* space)
							+ 4* j;
				index2 = 0;
				TextureGroup_data[  index1] = Texture_data[index2];
				TextureGroup_data[++index1] = Texture_data[index2 + 1];
				TextureGroup_data[++index1] = Texture_data[index2 + 2];
				TextureGroup_data[++index1] = 255;
			}
		}
		// lower right
		for (int i = pix + space; i != pix + 2* space; ++i)
		{
			for (int j = pix + space; j!= pix + 2* space; ++j)
			{
				index1 = 4*(pix * 8)* (pix + 2 * space) * y
							+ 4* (pix * 8) * i
							+ 4* x * (pix + 2* space)
							+ 4* j;
				index2 = 0;
				TextureGroup_data[  index1] = Texture_data[index2];
				TextureGroup_data[++index1] = Texture_data[index2 + 1];
				TextureGroup_data[++index1] = Texture_data[index2 + 2];
				TextureGroup_data[++index1] = 255;
			}
		}
	}
}

///=================================================================================================
/// Switch Grid/Filter.
///=================================================================================================
void TileManager::SwitchMaterial(bool grid, bool filter)
{
	std::string matWater, matLand;
	if (filter == true)
	{
		matLand = "Land_HighDetails" + StringConverter::toString(m_TileTextureSize, 3, '0');
		matWater= "Water_HighDetails";
	}
	else
	{
		matLand = "Land_LowDetails";
		matWater= "Water_LowDetails";
	}
	if (grid)
	{
		matLand +="_Grid";
		matWater+="_Grid";
	}
	for (int x = 0; x < CHUNK_SUM_X; ++x)
	{
		for (int y = 0; y < CHUNK_SUM_Z; ++y)
		{
			m_mapchunk[x][y].Get_Land_entity() ->setMaterialName(matLand);
			m_mapchunk[x][y].Get_Water_entity()->setMaterialName(matWater);
		}
	}
}

///=================================================================================================
/// Change resolution of terrain texture
///=================================================================================================
void TileManager::SetTextureSize(int pixels)
{
	if (pixels != 128 && pixels !=64 && pixels != 32 && pixels != 16 && pixels != 8) return;
	m_TileTextureSize = pixels;
	std::string matLand = "Land_HighDetails" + StringConverter::toString(m_TileTextureSize, 3, '0');
	{
		for (int x = 0; x < CHUNK_SUM_X; ++x)
			{
				for (int y = 0; y < CHUNK_SUM_Z; ++y)
				{
					m_mapchunk[x][y].Get_Land_entity()->setMaterialName(matLand);
				}
			}
	}
}
