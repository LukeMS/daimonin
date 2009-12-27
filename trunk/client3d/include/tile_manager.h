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

#include "tile_chunk.h"

/**
 ** This singleton class handles all tile related stuff in the worldmap.
 ** A tile is build out of 8 trianles (=4 subtiles) arranged in this way:
 ** +--+--+
 ** |\ | /|
 ** | \|/ |
 ** +--+--+
 ** | /|\ |
 ** |/ | \|
 ** +--+--+
 ** The path, holding the tile graphics, must be defined in the resources.cfg
 ** file as a 'tiles' entry (e.g.: 'FileSystem=media/textures/tiles').
 **
 ** Todo:
 ** - Use low poly/texture tiles for greater distances.
 ** - Finish the fake shadow support.
 ** - Add a class to handle undergrow (gras, small stones).
 **   (Remember that the sprites must always face to the camera).
 ** - Don't draw tiles outside the visible range.
 **   (Remember that every camera angle needs differnt tile arrangements).
 ** - Use a shadow buffer to speed up the update of the vertex buffer.
 **   (Remember that this buffer can only be updated after all changes are done
 **   in the map, because every tile needs the complete data of all its
 **   Neighbours).
 *****************************************************************************/

class TileManager
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static Ogre::String LAND_PREFIX;
    static Ogre::String WATER_PREFIX;
    static Ogre::String ATLAS_PREFIX;
    static Ogre::String MATERIAL_PREFIX;
    enum {CHUNK_SIZE_X     =  12};                /**< Size of the visible part of the world on x-axis. **/
    enum {CHUNK_SIZE_Z     =  12};                /**< Size of the visible part of the world on z-axis. **/
    enum {HEIGHT_STRETCH   =   2};                /**< Stretch the map height by this factor. **/
    enum {TILE_RENDER_SIZE = 1<< 6};              /**< Rendersize of a tile. **/
    enum {MAX_TEXTURE_SIZE = 1<<11};              /**< Atlassize for high quality. **/
    enum {TILE_SIZE        = MAX_TEXTURE_SIZE/8}; /**< Tilesize for high quality. **/
    enum {BORDER_SIZE      = TILE_SIZE/32};       /**< Bordersize for high quality (Border is used to fix filtering errors). **/
    enum {ATLAS_LAND_ROWS  =   7};                /**< Rows of Landtiles in the altlastexture. **/
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /** Init the tileengine.
     ** @param SceneManager   The ogre scenemanager.
     ** @param queryMaskLand  The query mask for land tiles.
     ** @param queryMaskWater The query mask for water tiles.
     ** @param pathGfxTiles   The directory path of the tile graphics.
     ** @param lod            Level of detail for the atlastexture (0: 2048x2048 to 3: 256x256).
     ** @param createAtlas    Create the atlas texture(s).
     *****************************************************************************/
    void Init(Ogre::SceneManager *SceneManager, int queryMaskLand, int queryMaskWater, int lod = 1, bool createAtlas = true);
    void freeRecources();
    static TileManager &getSingleton()
    {
        static TileManager Singleton; return Singleton;
    }
    /** Handle the mouseclick on a tile.
     ** @param posX The x-pos.
     ** @param posZ The z-pos.
     *****************************************************************************/
    void tileClick(float mouseX, float mouseY);
    /** Set the map data.
     ** @param x           The x-pos within the map.
     ** @param z           The z-pos within the map.
     ** @param height      The new height of the top/left vertex of this map pos.
     ** @param gfx         The tile-gfx number (0...41, while 41 is always a complete black gfx for fog of war).
     ** @param waterLvl    The height of the water surface. 0 means no water on this tile.
     ** @param shadow      The amount of the darkening for this tile. Used to fake shadows.
     ** @param gfxHardEdge The tile-gfx number with hard edge e.g. for indoor tiles.
     ** @param spotLight   True for a spotlight (Spotlights can only be placed in the middle of the border to a
                           horizontal or vertical neighbour tile).
     *****************************************************************************/
    void setMap(unsigned int x, unsigned int z, Ogre::uchar height, Ogre::uchar gfxLayer0, Ogre::uchar waterLvl =0,
                Ogre::uchar shadow = 255, Ogre::uchar gfxHardEdge = 0, bool spotLight = false);
    Ogre::uchar  getMapLayer0(unsigned int x, unsigned int z);
    Ogre::uchar  getMapLayer1(unsigned int x, unsigned int z);
    Ogre::ushort getMapWater (unsigned int x, unsigned int z);
    Ogre::Real   getMapShadow(unsigned int x, unsigned int z);
    bool         getMapSpotLight(unsigned int x, unsigned int z);
    /** Get the height of the Top/Left vertex of a subtile.
     ** @param posX The x-pos.
     ** @param posZ The z-pos.
     *****************************************************************************/
    Ogre::ushort getMapHeight(unsigned int x, unsigned int z);
    void setMapset(int landGroup, int waterGroup);
    void scrollMap(int x, int z);
    void updateChunks();
    void rotateCamera(Ogre::Real cameraAngle)
    {
        mMapchunk.setCameraRotation(cameraAngle);
    }
    bool loadImage(Ogre::Image &image, const Ogre::String &filename, bool logErrors);
    /** Get the height of a position within the terrain.
     ** Used to place objects on the terrain ground.
     ** @param posX The x-pos.
     ** @param posZ The z-pos.
     *****************************************************************************/
    short getTileHeight(int posX, int posZ);
    void updateHeighlightVertexPos(int deltaX, int deltaZ);
    void updateTileHeight(int deltaHeight);
    void updateTileGfx(int deltaGfxNr);
    void setTileGfx();
    void setWave(Ogre::Real alpha, Ogre::Real amplitude, Ogre::Real speed)
    {
        mMapchunk.setWave(alpha, amplitude, speed);
    }
    void setGrid(bool visible)
    {
        mMapchunk.setGrid(visible);
    }
    void setLight(Ogre::Real brightness)
    {
        mMapchunk.setLight(brightness);
    }
    Ogre::SceneManager *getSceneManager()
    {
        return mSceneManager;
    }
    //////// Only for TESTING
    void loadLvl();
    void saveLvl();
    /////////////////////////

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {SUM_ATLAS_RESOLUTIONS= 1<< 2}; /**< How many resolutions of the atlastexture to create. **/
    enum {ATLAS_LAND_COLS      =     6}; /**< Cols of Landtiles in the altlastexture. **/
    enum {MAX_MAP_SETS         =    16}; /**< The maximum numbers of AtlasTextures to be created by createAtlasTexture(...).
                                              Must be < 100, because its encoded as a 2 chars wide number in the filename. **/
    /**  TileEngine struct which holds the worldmap. **/
    typedef struct _mapStruct
    {
        Ogre::uchar gfxLayer0;   /**< Graphic of the Top/Left vertex. Layer0 uses blending. **/
        Ogre::uchar gfxLayer1;   /**< Graphic of the Top/Left vertex. Layer1 is drawn on top of Layer0 without blending. **/
        Ogre::uchar heightLand;  /**< Height  of the Top/Left vertex. **/
        Ogre::uchar heightWater; /**< Height of the water surface (0 -> no water). **/
        Ogre::uchar shadow;      /**< The darkening amount to simulate terrain shadows. **/
        bool spotLight;          /**< If >0 the subtile is lighten by by a spotlight. **/
    } mapStruct;
    mapStruct *mMap;
    TileChunk mMapchunk;
    static Ogre::SceneManager *mSceneManager;
    Ogre::RaySceneQuery *mRaySceneQuery;
    Ogre::Vector3 mVertex[3];  /**< Used for tile clicking. **/
    Ogre::String mPathGfxTiles;
    int mQueryMaskLand;
    int mMapSizeX, mMapMaskX, mMapSPosX;
    int mMapSizeZ, mMapMaskZ, mMapSPosZ;
    unsigned int mTextureSize;
    unsigned int mSelectedVertexX, mSelectedVertexZ; /**< Editor feature. Stores the actual selected VERTEX_TL of a tile. **/
    int mEditorActSelectedGfx;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileManager();
    ~TileManager();
    TileManager(const TileManager&); /**< disable copy-constructor. **/

    /** Create the atlastexture in different resolutions.
     ** @param maxTextureSize The maximum size of the atlas to create.
     ** @param groupNr        How many atlas groups to create. -1 to create all groups found in the media folder.
     *****************************************************************************/
    void createAtlasTexture(int maxTextureSize, unsigned int groupNr = MAX_MAP_SETS);
    bool copyTileToAtlas(Ogre::uchar *dstBuf); /**< 41 Standard tiles. **/
    void copyFlowToAtlas(Ogre::uchar *dstBuf); /**<  3 Special tiles with a flow effect (Water/Lava/etc). **/
    void copySpotToAtlas(Ogre::uchar *dstBuf);
    void copyMaskToAtlas(Ogre::uchar *dstBuf);
    bool vertexPick(Ogre::Ray *mouseRay, int x, int z, int pos);
    int  calcHeight(int vert0, int vert1, int vert2, int posX, int posZ);
    /// Create a template to make it easier for the artists to create a maskset.
    void createMaskTemplate();
    void highlightVertex(int x, int z);
    /** Set the path string with the parsed entry of the 'resources.cfg' file.
     ** Reimplemented
     ** @param resource The directory name to be found in the resources file.
     ** @param path     The path string to be set.
     *****************************************************************************/
    bool setResourcePath(Ogre::String resource, Ogre::String &path);
};

#endif
