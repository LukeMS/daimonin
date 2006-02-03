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

#include "Ogre.h"
#include "define.h"
#include "TileChunk.h"
#include "TileInterface.h"

using namespace Ogre;

/** Pixel per tile in the terrain-texture. */
const int PIXEL_PER_TILE =  128;
/** Pixel per col/row in the terrain-texture. */
const int PIXEL_PER_ROW  = 1024;
/** Numbers of tile textures in one row/col of the terrain texture. */
const int TEXTURES_PER_ROW = 7;
/** Size of a tile. */
const int TILE_SIZE = 32;

/** Number of chunks in worldmap */
#ifdef SINGLE_CHUNK
const int CHUNK_SUM_X  = 1;
const int CHUNK_SUM_Z  = 1;
#else
const int CHUNK_SUM_X  = 4;
const int CHUNK_SUM_Z  = 4;
#endif


/** Number of tiles in a chunk. */
const int CHUNK_SIZE_X = 13;
const int CHUNK_SIZE_Z = 21;

/** Number of tiles in the worldmap (on x-axis). */
const int TILES_SUM_X  = CHUNK_SUM_X * CHUNK_SIZE_X;
/** Number of tiles in the worldmap (on z-axis). */
const int TILES_SUM_Z  = CHUNK_SUM_Z * CHUNK_SIZE_Z;
/** Radius of the area where the tiles are drawn in high quality. */
const int HIGH_QUALITY_RANGE = 5;
/** Minimal size of tile in the shrinked terrain texture. */
const int MIN_TEXTURE_PIXEL = 16;
/** LOD for the chunks. */
enum
{
  QUALITY_LOW, QUALITY_HIGH
};


/**
 * TileEngine class which manages the chunks in the worldmap.
 * Because of speed reasons, the TileEngine divides the worldmap into chunks.
 * Each chunk controls a number of tiles.
 *****************************************************************************/
class TileManager
{
private:
  /**  TileEngine struct which holds the worldmap. **/
  struct WorldMap
  {
    /** Average height. **/
    unsigned char height;
    /** Column of the texture in the terrain-texture. **/
    unsigned char terrain_col;
    /** Row of the texture in the terrain-texture. **/
    unsigned char terrain_row;
  }
  ** m_Map;

  TileChunk m_mapchunk[CHUNK_SUM_X][CHUNK_SUM_Z];
  SceneManager* m_SceneManager;
  TileInterface* m_Interface;
  AxisAlignedBox* bounds;
  MaterialPtr m_Kartentextur;
  /** The z-stretching of the tiles. **/
  float m_StretchZ;
  int m_TileTextureSize;
  bool mHighDetails;
  bool mGrid;

public:
  TileManager();
  ~TileManager();
  TileChunk* get_TileChunk(int x, int y)
  {
    return &m_mapchunk[x][y];
  }
  SceneManager* Get_pSceneManager()
  {
    return m_SceneManager;
  }
  float Get_StretchZ()
  {
    return m_StretchZ;
  }
  unsigned char Get_Map_Height(short x, short y)
  {
    return m_Map[x][y].height;
  }
  float Get_Avg_Map_Height(short x, short y)
  {
    return ((m_Map[x  ][y].height*Get_StretchZ() + m_Map[x  ][y+1].height*Get_StretchZ() +
             m_Map[x+1][y].height*Get_StretchZ() + m_Map[x+1][y+1].height*Get_StretchZ()) /4);
  }
  unsigned char Get_Map_StretchedHeight(short x, short y)
  {
    return (unsigned char) (m_Map[x][y].height*Get_StretchZ());
  }
  unsigned char Get_Map_Texture_Row(short x, short y)
  {
    return m_Map[x][y].terrain_row;
  }
  unsigned char Get_Map_Texture_Col(short x, short y)
  {
    return m_Map[x][y].terrain_col;
  }
  TileInterface* get_TileInterface()
  {
    return m_Interface;
  }
  void Set_Map_Height(short x, short y, short value)
  {
    m_Map[x][y].height = value;
  }
  void Set_Map_Texture_Row(short x, short y, unsigned char value)
  {
    m_Map[x][y].terrain_row = value;
  }
  void Set_Map_Texture_Col(short x, short y, unsigned char value)
  {
    m_Map[x][y].terrain_col = value;
  }
  void Set_Map_Textures();
  bool LoadImage(Image &image, const std::string &filename);

  AxisAlignedBox *GetBounds();
  void Init(SceneManager* SceneManager, int tileTextureSize = 128, int tileStretchZ = 2);

  void CreateChunks();
  void ChangeChunks();
  void ControlChunks(Vector3 vector);

  void CreateTexture();
  void ChangeTexture();
  /** Create a terrain-texture out of tile textures. **/
  bool CreateTextureGroup(const std::string &terrain_type);
  void SetTextureSize(int pixels);
  void shrinkFilter();
  void shrinkTexture(const std::string &terrain_type);
  /** Import a 8bit png file as heightmap **/
  void Load_Map(const std::string &png_filename);
  /** Import a heightmap. **/
  void Load_Map(char *mapData);
  void Save_Map(const std::string &png_filename);
  void ToggleMaterial();
  void ToggleGrid();
  void addToGroupTexture(uchar* TextureGroup_data, uchar *Filter_data, Image* Texture, short pixel, short x, short y);
  void CreateMipMaps(const std::string &terrain_type);
};

#endif
