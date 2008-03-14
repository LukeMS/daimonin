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
    enum { TILE_SIZE        = 1<<6 }; /**< Size of a tile in pixel. */
    enum { TEXTURE_SIZE     = 1024 }; /**< The size of both textures (AtlasTexture and RenderTexture). */
    enum { ATLAS_TILE_SIZE  =  256 }; /**< The size of a tile in the AtlasTexture. */
    enum { ATLAS_FILTER_SIZE=   64 }; /**< The size of a filter in the AtlasTexture. */
    enum { MAP_SIZE         =   43 }; /**< Number of tiles in the worldmap (on x ynd z axis). */
    enum { CHUNK_SIZE       =   43 }; /**< Number of tiles in a chunk      (on x ynd z axis). */
    enum { MAX_MAP_SETS     =   16 }; /**< The maximum numbers of AtlasTextures to be created by createAtlasTexture(...). */

    int map_transfer_flag;
    bool map_new_flag;

    enum
    {
        VERTEX_TL,  // Top/Left.
        VERTEX_TR,  // Top/Right.
        VERTEX_BL,  // Bottom/Left.
        VERTEX_BR,  // Bottom/Right.
    };

    enum
    {
        // Pos of a wall within a tile.
        WALL_POS_BOTTOM,
        WALL_POS_TOP,
        WALL_POS_RIGHT,
        WALL_POS_LEFT,
        WALL_POS_SUM
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void Init(Ogre::SceneManager* SceneManager, int sumTilesX, int sumTilesZ, int zeroX, int zeroZ, int lod);
    void freeRecources();
    static TileManager &getSingleton()
    {
        static TileManager Singleton; return Singleton;
    }
    Ogre::SceneManager* getSceneManager()
    {
        return mSceneManager;
    }
    void getMapGfx(unsigned int x, unsigned int z, int &layer0, int &layer1, int &filter, int &mirror)
    {
        layer0 = mMap[x][z].layer0;
        layer1 = mMap[x][z].layer1;
        filter = mMap[x][z].filter;
        mirror = mMap[x][z].mirror&3;
    }
    void getMapShadow(unsigned int x, unsigned int z, int &shadow, int &mirror)
    {
        if (mShowGrid)
        {
            shadow = 72;
            mirror = !((mMapScrollX+mMapScrollZ+x+z)&1);
        }
        else
        {
            shadow = mMap[x][z].shadow;
            mirror =(mMap[x][z].mirror>>2)&3;
        }
    }
    void getMapScroll(int &x, int &z)
    {
        x = mMapScrollX;
        z = mMapScrollZ;
    }
    Ogre::uchar calcShadow(int x, int z);
    void calcMapShadows();
    Ogre::uchar getMapHeight(unsigned int x, unsigned int z, int vertex);

    /** Sets all parameters for a tile.
     ** @x X-pos of the tile within the map.
     ** @z Z-pos of the tile within the map.
     ** @height The height of the top-left vertex.
     ** @layer0 Position of layer0-gfx within the atlas texture.
     ** @layer1 Position of layer1-gfx within the atlas texture.
     ** @filter Position of the filter-gfx within the atlas-texture.
     **         The gfx-data is located in the alpha-channel of the atlas-texture.
     ** @shadow Position of the shadow-gfx within the atlas-texture. Used for:
     **         - terrain-shadow
     **         - darkness (day/night effect)
     **         - fogOfWar
     **         - grid
     **         The gfx-data is located in the Alpha channel of the atlas-texture.
     ** @mirror Bitarray for mirroring:
     **         bit 0: Mirror filter on x.
     **         bit 1: Mirror filter on z.
     **         bit 2: Mirror shadow on x.
     **         bit 3: Mirror shadow on z.
     *****************************************************************************/
    void setMap(int x, int z, Ogre::uchar height, Ogre::uchar layer0, Ogre::uchar layer1, Ogre::uchar filter, Ogre::uchar shadow, Ogre::uchar mirror);

    void changeMapset(Ogre::String filenameTileTexture, Ogre::String filenameEnvTexture);
    void toggleGrid()
    {
        mShowGrid = !mShowGrid;
        mMapchunk.updatePainter();
    }
    void scrollMap(int x, int z);
    void changeChunks();
    bool loadImage(Ogre::Image &image, const Ogre::String &filename);
    int  getTileHeight(int posX, int posZ);

    void addWall(int level, int tileX, int tileZ, int pos, const char *meshName);
    void syncWalls(int dx, int dy);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    /**  TileEngine struct which holds the worldmap. **/
    typedef struct
    {
        Ogre::Entity *entity[WALL_POS_SUM];
        Ogre::uchar height; /**< Height of VERTEX_TL. **/
        Ogre::uchar layer0; /**< Gfx-nr in atlas-texture for layer0. **/
        Ogre::uchar layer1; /**< Gfx-nr in atlas-texture for layer1. **/
        Ogre::uchar filter; /**< Gfx-nr in atlas-texture for filter. (used to blend the layers). **/
        Ogre::uchar shadow; /**< Gfx-nr in atlas-texture for shadow. (used for shadowds/fogOfWar/grid/etc). **/
        Ogre::uchar mirror; /**< bit 0: Mirror filter on x, bit 1: Mirror filter on z.
                                 bit 2: Mirror shadow on x, bit 3: Mirror shadow on z. **/
    }mapStruct;
    mapStruct mMap[MAP_SIZE+2][MAP_SIZE+2];
    Ogre::SceneManager *mSceneManager;
    TileChunk mMapchunk;
    int mLod;
    int mMapScrollX, mMapScrollZ;
    bool mShowGrid;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileManager();
    ~TileManager() {}
    TileManager(const TileManager&); /**< disable copy-constructor. **/
    /** **************************************************************************
     **  groupNr -1: create all groups found in the media folder.
     ** **************************************************************************/
    void createAtlasTexture(const Ogre::String filenameTiles, const Ogre::String filenameFilters, const Ogre::String filenameShadows, unsigned int groupNr = MAX_MAP_SETS+1);
    void copyFilterToAtlas(Ogre::uchar *dstBuf, Ogre::String filename, int startRow, int stopRow);
    bool copyTileToAtlas  (Ogre::uchar *dstBuf, Ogre::String filename);
    int  calcHeight(int vert0, int vert1, int vert2, int posX, int posZ);

    void delRowOfWalls(int row); /**< Delete all walls that are scrolling out of the tile map.**/
    void delColOfWalls(int col); /**< Delete all walls that are scrolling out of the tile map.**/
    void clsRowOfWalls(int row); /**< Set all walls to 0 that are scrolling into the tile map.**/
    void clsColOfWalls(int col); /**< Set all walls to 0 that are scrolling into the tile map.**/
};

#endif
