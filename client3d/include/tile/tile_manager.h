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

#include "tile/tile_chunk.h"

/// @brief This singleton class handles all tile related stuff in the worldmap.
/// @details A tile is build out of 8 trianles (=4 subtiles) arranged in this way:
/// <pre>
/// +--+--+
/// |\ | /|
/// | \|/ |
/// +--+--+
/// | /|\ |
/// |/ | \|
/// +--+--+
/// </pre>
/// The path, holding the tile graphics, must be defined in the resources.cfg
/// file as a 'tiles' entry (e.g.: 'FileSystem=media/textures/tiles').
/// @todo
/// - Use low poly/texture tiles for greater distances.
/// - Add a class to handle undergrow (gras, small stones).
/// - Don't draw tiles outside the visible range.
///   (Remember that every camera angle needs differnt tile arrangements).
/// - Use a shadow buffer to speed up the update of the vertex buffer.
///   (Remember that this buffer can only be updated after all changes are done
///   in the map, because every tile needs the complete data of all its
///   Neighbours).

class TileManager
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static Ogre::String LAND_PREFIX;
    static Ogre::String WATER_PREFIX;
    static Ogre::String UNDERGROWTH_PREFIX;
    static Ogre::String ATLAS_PREFIX;
    static Ogre::String MATERIAL_PREFIX;

    enum {CHUNK_SIZE_X     = 12                 /**< Size of the visible part of the world on x-axis. **/ };
    enum {CHUNK_SIZE_Z     = 12                 /**< Size of the visible part of the world on z-axis. **/ };
    enum {HEIGHT_STRETCH   =  2                 /**< Stretch the map height by this factor.**/ };
    enum {TILE_RENDER_SIZE = 1<< 6              /**< Rendersize of a tile.      **/ };
    enum {HALF_RENDER_SIZE = TILE_RENDER_SIZE/2 /**< Rendersize of a sub-tile.  **/ };
    enum {MAX_TEXTURE_SIZE = 2048 /**< Rendersize of a sub-tile.  **/ };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /// Init the tileengine.
    /// @param SceneManager   The ogre scenemanager.
    /// @param queryMaskLand  The query mask for land entity.
    /// @param queryMaskWater The query mask for water entity.
    /// @param lod            Level of detail for the atlastexture (0: 2048x2048 to 3: 256x256).
    /// @param createAtlas    Create the atlas texture(s).
    void Init(Ogre::SceneManager *SceneManager, int queryMaskLand, int queryMaskWater, int lod = 1, bool createAtlas = true);

    /// Must be called before the destructor. Frees all ogre resources.
    void freeRecources();
    static TileManager &getSingleton()
    {
        static TileManager Singleton; return Singleton;
    }
    /// Handle the mouseclick on a tile.
    /// @param posX The x-pos.
    /// @param posZ The z-pos.
    void tileClick(float mouseX, float mouseY);

    /// Set the map data.
    /// @param x           The x-pos within the map.
    /// @param z           The z-pos within the map.
    /// @param height      The new height of the top/left vertex of this map pos.
    /// @param gfx         The tile-gfx number (0...41, while 41 is always a complete black gfx for fog of war).
    /// @param waterLvl    The height of the water surface. 0 means no water on this tile.
    /// @param gfxHardEdge The tile-gfx number with hard edge e.g. for indoor tiles.
    /// @param spotLight   True for a spotlight (Spotlights can only be placed in the middle of the border to a
    ///                    horizontal or vertical neighbour tile).
    void setMap(unsigned int x, unsigned int z, Ogre::uchar height, Ogre::uchar gfxLayer0, Ogre::uchar waterLvl =0,
                Ogre::uchar gfxHardEdge = 0, bool spotLight = false);
    Ogre::uchar  getMapLayer0(unsigned int x, unsigned int z);
    Ogre::uchar  getMapLayer1(unsigned int x, unsigned int z);
    Ogre::ushort getMapWater (unsigned int x, unsigned int z);
    Ogre::Real   getMapShadow(unsigned int x, unsigned int z);
    bool         getMapSpotLight(unsigned int x, unsigned int z);

    /// Get the height of the Top/Left vertex of a subtile.
    /// @param posX The x-pos.
    /// @param posZ The z-pos.
    Ogre::ushort getMapHeight(unsigned int x, unsigned int z);

    void setMapset(int landGroup, int waterGroup);

    /// Scroll the map by n tiles (Subtile scrolling is not possible!).
    /// @param posX The sroll amount in x direction.
    /// @param posZ The sroll amount in z direction.
    void scrollMap(int x, int z);

    void updateChunks();
    void rotateCamera(Ogre::Real cameraAngle)
    {
        mMapchunk.setCameraRotation(cameraAngle);
    }

    /// Get the height of a position within the terrain.
    /// Used to place objects on the terrain ground.
    /// @param posX The x-pos.
    /// @param posZ The z-pos.
    short getTileHeight(int posX, int posZ);
    void updateHeighlightVertexPos(int deltaX, int deltaZ);
    void updateTileHeight(int deltaHeight);
    void updateTileGfx(int deltaGfxNr);
    void setTileGfx();
    void setWave(Ogre::Real alpha, Ogre::Real amplitude, Ogre::Real speed)
    {
        mMapchunk.setWave(alpha, amplitude, speed);
    }
    void setUndergrowth(Ogre::Real alpha, Ogre::Real amplitude, Ogre::Real speed)
    {
        mMapchunk.setUndergrowth(alpha, amplitude, speed);
    }
    void setGrid(bool visible)
    {
        mMapchunk.setGrid(visible);
    }
    void setLight(Ogre::Real brightness)
    {
        mMapchunk.setLight(brightness);
    }
    void setRenderOptions(bool drawGrass)
    {
        mMapchunk.setRenderOptions(drawGrass);
    }
    Ogre::SceneManager *getSceneManager()
    {
        return mSceneManager;
    }

#ifndef TILEENGINE_SKIP_LEVELLOADING
    // Just for testing. Will be removed soon!
    void loadLvl();
    void saveLvl();
#endif

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {ATLAS_LAND_COLS      =     6}; ///< Cols of Landtiles in the altlastexture.
    ///  TileEngine struct which holds the worldmap.
    typedef struct _mapStruct
    {
        Ogre::uchar gfxLayer0;   ///< Graphic of the Top/Left vertex. Layer0 uses blending.
        Ogre::uchar gfxLayer1;   ///< Graphic of the Top/Left vertex. Layer1 is drawn on top of Layer0 without blending.
        Ogre::uchar heightLand;  ///< Height  of the Top/Left vertex.
        Ogre::uchar heightWater; ///< Height of the water surface (0 -> no water).
        Ogre::Vector3 normal;    ///< The normal vector. Used for lightning.
        bool spotLight;          ///< If >0 the subtile is lighten by by a spotlight.
    } mapStruct;
    mapStruct *mMap;
    TileChunk mMapchunk;
    static Ogre::SceneManager *mSceneManager;
    Ogre::RaySceneQuery *mRaySceneQuery;
    Ogre::Vector3 mVertex[3];  ///< Used for tile clicking.
    Ogre::String mPathGfxTiles;
    int mQueryMaskLand;
    int mMapSizeX, mMapMaskX, mMapSPosX;
    int mMapSizeZ, mMapMaskZ, mMapSPosZ;
    unsigned int mTextureSize;
    unsigned int mSelectedVertexX, mSelectedVertexZ; ///< Editor feature. Stores the actual selected VERTEX_TL of a tile.
    int mEditorActSelectedGfx;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileManager();
    ~TileManager();
    TileManager(const TileManager&);            ///< disable copy-constructor.
    TileManager &operator=(const TileManager&); ///< disable assignment operator.

    bool loadImage(Ogre::Image &image, const Ogre::String &filename, bool logErrors);
    bool vertexPick(Ogre::Ray *mouseRay, int x, int z, int pos);
    int  calcHeight(int vert0, int vert1, int vert2, int posX, int posZ);
    void highlightVertex(int x, int z);
};

#endif
