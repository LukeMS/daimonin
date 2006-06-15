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

#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include <Ogre.h>
#include "define.h"
#include "tile_chunk.h"
#include "tile_interface.h"

using namespace Ogre;

/**
 * TileEngine class which manages all tiles related stuff in the worldmap.
 *****************************************************************************/

/** Number of tiles in the worldmap (on x-axis). */
const int CHUNK_SIZE_X  = 16;

/** Number of tiles in the worldmap (on z-axis). */
const int CHUNK_SIZE_Z  = 16;

/** Minimal size of tile in the shrinked terrain texture. */
const int MIN_TEXTURE_PIXEL = 16;

/** Size of a tile. */
const int TILE_SIZE_X = 32;
const int TILE_SIZE_Z = 64;

class TileManager
{

public:
    void freeRecources();
    static TileManager &getSingleton()
    {
        static TileManager Singleton; return Singleton;
    }
    SceneManager* getSceneManager()
    {
        return mSceneManager;
    }
    int getTextureSize()
    {
        return mTileTextureSize;
    }
    unsigned char getMapHeight(short x, short y)
    {
        return mMap[x][y].height;
    }
    float getAvgMapHeight(short x, short y)
    {
        return ((mMap[x  ][y].height + mMap[x  ][y+1].height +
                 mMap[x+1][y].height + mMap[x+1][y+1].height) /4);
    }
    unsigned char getMapTextureRow(short x, short y)
    {
        return mMap[x][y].terrain_row;
    }
    unsigned char getMapTextureCol(short x, short y)
    {
        return mMap[x][y].terrain_col;
    }
    TileInterface* getTileInterface()
    {
        return mInterface;
    }
    void setMapHeight(short x, short y, short value)
    {
        mMap[x][y].height = value;
    }
    void setMapTextureRow(short x, short y, unsigned char value)
    {
        mMap[x][y].terrain_row = value;
    }
    void setMapTextureCol(short x, short y, unsigned char value)
    {
        mMap[x][y].terrain_col = value;
    }
    void setMapTextures();
    bool loadImage(Image &image, const std::string &filename);

    AxisAlignedBox *GetBounds();
    void Init(SceneManager* SceneManager, int tileTextureSize = 128);

    void createChunks();
    void changeChunks();
    void getMapScroll(int &x, int &z);

    void createTexture();
    void changeTexture();
    /** Create a terrain-texture out of tile textures. **/
    bool createTextureGroup(const std::string &terrain_type);
    void createTextureGroupBorders(uchar* TextureGroup_data, short pix);
    void shrinkFilter();
    void shrinkTexture(const std::string &terrain_type);
    /** Import a 8bit png file as heightmap **/
    void loadMap(const std::string &png_filename);
    void scrollMap(int x, int z);
    void setMaterialLOD(int pix);
    void toggleGrid();
    void addToGroupTexture(uchar* TextureGroup_data, uchar *Filter_data, Image* Texture, short pixel, short x, short y);

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
    mMap[CHUNK_SIZE_X+1][CHUNK_SIZE_Z+1];

    SceneManager *mSceneManager;
    TileChunk mMapchunk;
    TileInterface *mInterface;
    AxisAlignedBox *mBounds;
    MaterialPtr mKartentextur;
    int mTileTextureSize;
    bool mGrid;
    unsigned int mapScrollX, mapScrollZ;

    TileManager();
    ~TileManager();
    TileManager(const TileManager&); // disable copy-constructor.
};

#endif
