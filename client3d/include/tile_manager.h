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
 * A tile is build out of 8 trinagles (=4 suvbtiles) arranged in this way:
 * +--+--+
 * |\ | /|
 * | \|/ |
 * +--+--+
 * | /|\ |
 * |/ | \|
 * +--+--+
 *
 * For a better looking terrain, every tile has 4 source graphics.
 * This way the terrain doesn't repeat too much.
 * +--+--+
 * |  |  |
 * |  |  |
 * +--+--+
 * |  |  |
 * |  |  |
 * +--+--+
 *****************************************************************************/

class TileManager
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { RGB = 3, RGBA = 4       }; /**< Pixelsize. */
    enum { SHADOW_NONE      =    0 }; /**< Shadow gfx number for no shadow. */
    enum { SHADOW_GRID      =    1 }; /**< Shadow gfx number for grid-gfx. */
    enum { SHADOW_MIRROX_X  = 1<<14}; /**< Mirror the shadow on X-Axis. */
    enum { SHADOW_MIRROX_Z  = 1<<15}; /**< Mirror the shadow on Z-Axis. */
    enum { TILE_SIZE        = 1<<6 }; /**< Rendersize of a tile. */
    enum { MAX_TEXTURE_SIZE = 2048 }; /**< Atlas- and Rendertexture size for highest quality. */
    enum { COLS_SRC_TILES   =    8 }; /**< Number of tile columns in the atlastexture. */
    enum { MAP_SIZE         =   42 }; /**< Number of tiles in the worldmap (on x ynd z axis). */
    enum { CHUNK_SIZE_X     =   42 }; /**< . */
    enum { CHUNK_SIZE_Z     =   32 }; /**< . */
    enum { MAX_MAP_SETS     =   16 }; /**< The maximum numbers of AtlasTextures to be created by createAtlasTexture(...). */
    enum
    {
        VERTEX_TL,  // Top/Left.
        VERTEX_TR,  // Top/Right.
        VERTEX_BL,  // Bottom/Left.
        VERTEX_BR,  // Bottom/Right.
        VERTEX_SUM  // Numer of vertices
    };
    int map_transfer_flag;
    bool map_new_flag;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void Init(Ogre::SceneManager* SceneManager, int lod = 1, bool createAtlasTexture = true);
    void freeRecources();
    static TileManager &getSingleton()
    {
        static TileManager Singleton;
        return Singleton;
    }
    Ogre::SceneManager* getSceneManager()
    {
        return mSceneManager;
    }
    void tileClick(float mouseX, float mouseYt);
    int getMapShadow(unsigned int x, unsigned int z);
    void getMapScroll(int &x, int &z)
    {
        x = mMapScrollX;
        z = mMapScrollZ;
    }
    void setMap(unsigned int x, unsigned int z, short height, char gfx, char shadow=0);
    char getMapGfx(unsigned int x, unsigned int z, int vertex);
    short getMapHeight(unsigned int x, unsigned int z, int vertex);
    void changeMapset(Ogre::String filenameTileTexture, Ogre::String filenameEnvTexture);
    void toggleGrid()
    {
        mShowGrid = !mShowGrid;
        mMapchunk.change();
    }
    void scrollMap(int x, int z);
    void changeChunks();
    bool loadImage(Ogre::Image &image, const Ogre::String &filename);
    short getTileHeight(int posX, int posZ);
    void updateHeighlightVertexPos(int deltaX, int deltaZ);
    void updateTileHeight(int deltaHeight);
    void updateTileGfx(int deltaGfxNr);
    void setTileGfx();
    //////// Only for TESTING
    void loadLvl();
    void saveLvl();
    /////////////////////////

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    /**  TileEngine struct which holds the worldmap. **/
    typedef struct
    {
        unsigned char  gfx;    /**< Graphic of VERTEX_TL. **/
        unsigned short height; /**< Height of VERTEX_TL. **/
        unsigned short shadow; /**< Shadow that VERTEX_TL is casting.
                                         0: No shadow.
                                         1: Grid gfx.
                                     2-127: Shadow gfx.
                                    Bit 14: Mirror shadow on X-Axis (SHADOW_MIRROX_X).
                                    Bit 15: Mirror shadow on Z-Axis (SHADOW_MIRROX_Z). **/
    }mapStruct;
    mapStruct mMap[MAP_SIZE+2][MAP_SIZE+2];
    Ogre::SceneManager *mSceneManager;
    Ogre::RaySceneQuery *mRaySceneQuery;
    TileChunk mMapchunk;
    Ogre::Vector3 mTris[4];
    int mLod;
    unsigned int mSelectedVertexX, mSelectedVertexZ; /**< Editor feature. Stores the actual selected VERTEX_TL of a tile. **/
    int mEditorActSelectedGfx;
    int mMapScrollX, mMapScrollZ;
    bool mShowGrid;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileManager() {}
    ~TileManager() {}
    TileManager(const TileManager&); /**< disable copy-constructor. **/
    /** **************************************************************************
     **  groupNr -1: create all groups found in the media folder.
     ** **************************************************************************/
    void createAtlasTexture(int textureSize, bool fixFilteringErrors, unsigned int groupNr = MAX_MAP_SETS);
    bool copyTileToAtlas  (Ogre::uchar *dstBuf);
    void copyFilterToAtlas(Ogre::uchar *dstBuf);
    void copyShadowToAtlas(Ogre::uchar *dstBuf);
    void copySubTile(Ogre::uchar* src, int srcX, int srcY, Ogre::uchar *dst, int dstX, int dstY, int size, bool srcAlpha);
    bool vertexPick(Ogre::Ray *mouseRay, int x, int z, int pos);
    void highlightVertex(int x, int z);
    int  calcHeight(int vert0, int vert1, int vert2, int posX, int posZ);
    void createFilterTemplate();
    unsigned short calcShadow(int x, int z);
};

#endif
