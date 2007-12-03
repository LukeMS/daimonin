/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include <Ogre.h>
#include "define.h"
#include "tile_chunk.h"

/**
 * TileEngine class which manages all tiles related stuff in the worldmap.
 *****************************************************************************/
class TileManager
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { TILE_SIZE = 1 << 6};      /**< Size of a tile in pixel. */
    enum { HALF_SIZE = TILE_SIZE/2}; /**< Size of a tile quadrandt in pixel. */
    enum { CHUNK_SIZE_X = 17 };      /**< Number of tiles in the worldmap (on x-axis). */
    enum { CHUNK_SIZE_Z = 17 };      /**< Number of tiles in the worldmap (on z-axis). */
    enum { MIN_TEXTURE_PIXEL = 16 }; /**< Minimal size of tile in the terrain texture. */
    enum { SUM_SUBTILES =  8 };      /**< Obsolete - don't use! */

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
    {
        VERTEX_BL,  // Bottom/Left.
        VERTEX_TL,  // Top/Left.
        VERTEX_TR,  // Top/Right.
        VERTEX_BR,  // Bottom/Right.
        VERTEX_MID, // Height given by the server.
        VERTEX_AVG, // Average Height.
        VERTEX_SUM, // Number of vertixes.
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
    unsigned char getMapHeight(unsigned int x, unsigned int z, int vertex = VERTEX_MID)
    {
        if (x >= CHUNK_SIZE_X || z >= CHUNK_SIZE_Z)
            return 0;
        return mMap[x][z].height[vertex];
    }
    char getMapTextureRow(short x, short z)
    {
        return mMap[x][z].terrain_row;
    }
    char getMapTextureCol(short x, short z)
    {
        return mMap[x][z].terrain_col;
    }
    bool getIndoor(short x, short z)
    {
        // ATM only map position 2,4 is an indoor tile.
        if (mMap[x][z].terrain_col == 2 && mMap[x][z].terrain_row == 4) return true;
        return false;
    }
    void getMapScroll(int &x, int &z)
    {
        x = mMapScrollX;
        z = mMapScrollZ;
    }
    void setMapHeight(short x, short y, short height)
    {
        mMap[x][y].height[VERTEX_MID] = height;
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
        mMap[x][y].height[VERTEX_MID] = (unsigned char)height;
        mMap[x][y].terrain_row        = (unsigned char)row;
        mMap[x][y].terrain_col        = (unsigned char)col;
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
    void addWall(int level, int tileX, int tileZ, int pos, const char *meshName);
    void syncWalls(int dx, int dy);
    void calcVertexHeight();
    int getTileHeight(int posX, int posZ);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    /**  TileEngine struct which holds the worldmap. **/
    struct _mMap
    {
        unsigned char height[VERTEX_SUM];      /**< Height of every vertex of a tile. **/
        unsigned char walkable[SUM_SUBTILES];  /**< Walkable status for each subtile (8 bit * 8 rows)  **/  // Obsolete - don't use!
        char terrain_col;                      /**< Column of the tile-texture in the terrain-texture. **/
        char terrain_row;                      /**< Row    of the tile-texture in the terrain-texture. **/
        Ogre::Entity *entity[WALL_POS_SUM];
    }
    mMap[CHUNK_SIZE_X+1][CHUNK_SIZE_Z+1];
    Ogre::SceneManager *mSceneManager;
    TileChunk mMapchunk;
    int mTileTextureSize;
    int mMapScrollX, mMapScrollZ;
    bool mGrid;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileManager();
    ~TileManager();
    TileManager(const TileManager&); /**< disable copy-constructor. **/
    int calcHeight(int vert0, int vert1, int vertMid, int posX, int posZ);
    void delRowOfWalls(int row);     /**< Delete all walls that are scrolling out of the tile map.**/
    void delColOfWalls(int col);     /**< Delete all walls that are scrolling out of the tile map.**/
    void clsRowOfWalls(int row);     /**< Set all walls to 0 that are scrolling into the tile map.**/
    void clsColOfWalls(int col);     /**< Set all walls to 0 that are scrolling into the tile map.**/
};

#endif
