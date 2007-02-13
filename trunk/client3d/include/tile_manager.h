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

/**
 * TileEngine class which manages all tiles related stuff in the worldmap.
 *****************************************************************************/
class TileManager
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { CHUNK_SIZE_X = 17 };      /**< Number of tiles in the worldmap (on x-axis). */
    enum { CHUNK_SIZE_Z = 17 };      /**< Number of tiles in the worldmap (on z-axis). */
    enum { MIN_TEXTURE_PIXEL = 16 }; /**< Minimal size of tile in the terrain texture. */

    /** Size of a tile. */
    enum { TILE_SIZE_X  = 48 };
    enum { TILE_SIZE_Z  = 48 };
    enum { SUM_SUBTILES =  8 };

    int map_update_flag;
    int map_transfer_flag;
    bool map_new_flag;

    enum
    {
        TRIANGLE_LEFT  = 1 << 0,
        TRIANGLE_TOP   = 1 << 1,
        TRIANGLE_RIGHT = 1 << 2,
        TRIANGLE_BOTTOM= 1 << 3
    };

    enum
    {   // Pos of a wall within a tile.
        WALL_POS_BOTTOM,
        WALL_POS_TOP,
        WALL_POS_RIGHT,
        WALL_POS_LEFT,
        WALL_POS_SUM
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void freeRecources();
    static TileManager &getSingleton()
    {
        static TileManager Singleton; return Singleton;
    }
    Ogre::SceneManager* getSceneManager()
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
    char getMapTextureRow(short x, short z)
    {
        if ((unsigned short)x > CHUNK_SIZE_X) x = CHUNK_SIZE_X;
        if ((unsigned short)z > CHUNK_SIZE_Z) z = CHUNK_SIZE_Z;
        return mMap[x][z].terrain_row;
    }
    char getMapTextureCol(short x, short z)
    {
        if ((unsigned short)x > CHUNK_SIZE_X) x = CHUNK_SIZE_X;
        if ((unsigned short)z > CHUNK_SIZE_Z) z = CHUNK_SIZE_Z;
        return mMap[x][z].terrain_col;
    }
    bool getIndoor(short x, short z)
    {
        if ((unsigned short)x > CHUNK_SIZE_X) x = CHUNK_SIZE_X;
        if ((unsigned short)z > CHUNK_SIZE_Z) z = CHUNK_SIZE_Z;
        // ATM only map position 2,4 is an indoor tile.
        if (mMap[x][z].terrain_col == 2 && mMap[x][z].terrain_row == 4) return true;
        return false;
    }
    TileInterface* getTileInterface()
    {
        return mInterface;
    }
    void getMapScroll(int &x, int &z)
    {
        x = mMapScrollX;
        z = mMapScrollZ;
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
    void setMap(int x, int y, int height, int row, int col)
    {
        mMap[x][y].height      = (unsigned char)height;
        mMap[x][y].terrain_row = (unsigned char)row;
        mMap[x][y].terrain_col = (unsigned char)col;
    }
    void setMapTextures();
    bool loadImage(Ogre::Image &image, const Ogre::String &filename);

    Ogre::AxisAlignedBox *GetBounds();
    void Init(Ogre::SceneManager* SceneManager, int tileTextureSize = 128);

    void createChunks();
    void changeChunks();

    void createTexture();
    void changeTexture();
    /** Create a terrain-texture out of tile textures. **/
    bool createTextureGroup(const Ogre::String &terrain_type);
    void createTextureGroupBorders(unsigned char* TextureGroup_data, short pix);
    void shrinkFilter();
    void shrinkTexture(const Ogre::String &terrain_type);
    void scrollMap(int x, int z);
    void setMaterialLOD(int pix);
    void toggleGrid();
    void addToGroupTexture(unsigned char* TextureGroup_data, unsigned char *Filter_data, Ogre::Image* Texture, short pixel, short x, short y);
    void setWalkablePos(const TilePos &pos, int row, unsigned char walkables);
    bool getWalkablePos(int x, int y);
    void addWall(int level, int tileX, int tileZ, int pos, const char *meshName);
    void syncWalls(Ogre::Vector3 &deltaPos);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    /**  TileEngine struct which holds the worldmap. **/
    struct _mMap
    {
        unsigned char height;                     /**< Average height. **/
        unsigned char walkable[SUM_SUBTILES];     /**< Walkable status for each subtile (8 bit * 8 rows)  **/
        char terrain_col;                         /**< Column of the tile-texture in the terrain-texture. **/
        char terrain_row;                         /**< Row    of the tile-texture in the terrain-texture. **/
        Ogre::Entity *entity[WALL_POS_SUM];
    }
    mMap[CHUNK_SIZE_X+1][CHUNK_SIZE_Z+1];
    Ogre::SceneManager *mSceneManager;
    TileChunk mMapchunk;
    TileInterface *mInterface;
    int mTileTextureSize;
    int mMapScrollX, mMapScrollZ;
    bool mGrid;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileManager();
    ~TileManager();
    TileManager(const TileManager&); /**< disable copy-constructor. **/
    void delRowOfWalls(int row);     /**< Delete all walls that are scrolling out of the tile map.**/
    void delColOfWalls(int col);     /**< Delete all walls that are scrolling out of the tile map.**/
    void clsRowOfWalls(int row);     /**< Set all walls to 0 that are scrolling into the tile map.**/
    void clsColOfWalls(int col);     /**< Set all walls to 0 that are scrolling into the tile map.**/
};

#endif
