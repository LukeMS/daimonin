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

#include <iostream>
#include <OgreHardwarePixelBuffer.h>
#include "tile_chunk.h"
#include "tile_manager.h"

#include "object_manager.h"
#include "logger.h"

using namespace Ogre;

//#define LOG_TIMING

//================================================================================================
// Constructor.
//================================================================================================
TileManager::TileManager()
{
    mMapScrollX =0;
    mMapScrollZ =0;
    for (int z =0; z <= CHUNK_SIZE; ++z)
        clsRowOfWalls(z);
}

//================================================================================================
// Free all resources.
//================================================================================================
void TileManager::freeRecources()
{
    for (int z =0; z <= CHUNK_SIZE; ++z)
        delRowOfWalls(z);
    mMapchunk.freeRecources();
}

//================================================================================================
// Returns the height of a tile-vertex.
//================================================================================================
Ogre::uchar TileManager::getMapHeight(unsigned int x, unsigned int z, int vertex)
{
    if (x >= MAP_SIZE || z >= MAP_SIZE) return 0;
    if (vertex == VERTEX_TL)  return mMap[x][z].height;
    if (vertex == VERTEX_TR)
    {
        if (x < MAP_SIZE-1) return mMap[x+1][z].height;
    }
    else if (vertex == VERTEX_BL)
    {
        if (z < MAP_SIZE-1) return mMap[x][z+1].height;
    }
    else if (vertex == VERTEX_BR)
    {
        if ((x < MAP_SIZE-1) && (z < MAP_SIZE-1)) return mMap[x+1][z+1].height;
    }
    return mMap[x][z].height;
}

//================================================================================================
//
//================================================================================================
int TileManager::getDeltaHeightClass(int x, int z)
{
    /*
    int a = Ogre::Math::IAbs(mMap[x][z].height[VERTEX_TL] - mMap[x][z].height[VERTEX_TR]);
    int b = Ogre::Math::IAbs(mMap[x][z].height[VERTEX_BL] - mMap[x][z].height[VERTEX_BR]);
    if (a >=220 && b >= 220) return 5;
    if (a >=180 && b >= 180) return 4;
    if (a >=140 && b >= 140) return 3;
    if (a >=100 && b >= 100) return 2;
    if (a >= 65 && b >=  65) return 1;
    */
    return 0;
}

//================================================================================================
// Init the TileEngine.
//================================================================================================
void TileManager::Init(SceneManager* SceneMgr, int sumTilesX, int sumTilesZ, int zeroX, int zeroZ, bool highDetails)
{
    Logger::log().headline("Init TileEngine");
    mSceneManager = SceneMgr;
    mHighDetails  = highDetails;
    // ////////////////////////////////////////////////////////////////////
    // Create all TextureGroups.
    // ////////////////////////////////////////////////////////////////////
    //if (Option::getSingleton().getIntValue(Option::CMDLINE_CREATE_TILE_TEXTURES))
    {
        Logger::log().info() << "Creating texture groups";
        createAtlasTexture("terrain", "filter", "shadow");
    }
    mMapchunk.init();
    Logger::log().info() << "Init done.";
}

//================================================================================================
// Scroll the map.
//================================================================================================
void TileManager::scrollMap(int dx, int dz)
{
    if (dx >0)
    {
        --mMapScrollX;
        delColOfWalls(0); // Delete walls leaving the view.
        for (int x = 0; x < CHUNK_SIZE; ++x)
            for (int y = 0; y <= CHUNK_SIZE; ++y)
                mMap[x][y] = mMap[x+1][y];
        clsColOfWalls(CHUNK_SIZE); // Set all Entities to 0.
    }
    else if (dx <0)
    {
        ++mMapScrollX;
        delColOfWalls(CHUNK_SIZE); // Delete walls leaving the view.
        for (int x = CHUNK_SIZE; x >0; --x)
            for (int y = 0; y <= CHUNK_SIZE; ++y)
                mMap[x][y] = mMap[x-1][y];
        clsColOfWalls(0); // Set all Entities to 0.
    }
    if (dz >0)
    {
        --mMapScrollZ;
        delRowOfWalls(0); // Delete walls leaving the view.
        for (int x = 0; x <= CHUNK_SIZE; ++x)
            for (int y = 0; y < CHUNK_SIZE; ++y)
                mMap[x][y] = mMap[x][y+1];
        clsRowOfWalls(CHUNK_SIZE); // Set all Entities to 0.
    }
    else if (dz <0)
    {
        ++mMapScrollZ;
        delRowOfWalls(CHUNK_SIZE); // Delete walls leaving the view.
        for (int x = 0; x <= CHUNK_SIZE; ++x)
            for (int y = CHUNK_SIZE; y > 0; --y)
                mMap[x][y] = mMap[x][y-1];
        clsRowOfWalls(0); // Set all Entities to 0.
    }


    mMapchunk.change();
    syncWalls(-dx, -dz);
}

//================================================================================================
// Add a wall to a tile quadrant.
// Walls are the only non-moveable objects that are not centered on a subtile.
//================================================================================================
void TileManager::addWall(int level, int x, int z, int pos, const char *meshName)
{
    if (mMap[x][z].entity[pos]) return; // Add only 1 wall per tile quadrant.
    static unsigned int index = 0;
    String strObj = "Wall_" + StringConverter::toString(++index);
    mMap[x][z].entity[pos] = mSceneManager->createEntity(strObj, meshName);
    mMap[x][z].entity[pos]->setQueryFlags(ObjectManager::QUERY_ENVIRONMENT_MASK);
    mMap[x][z].entity[pos]->setRenderQueueGroup(RENDER_QUEUE_7);
    Vector3 tpos;
    tpos.x = x * TILE_SIZE + (pos==WALL_POS_LEFT?0:TILE_SIZE);
    tpos.z = z * TILE_SIZE + (pos==WALL_POS_TOP?0:TILE_SIZE);
    //tpos.y = getTileHeight((int)tpos.x, (int)tpos.z);
    tpos.y = 30; // JUST A HACK! (server may send objects before tiles - so the height could be undefined).
    SceneNode *sceneNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
    sceneNode->attachObject(mMap[x][z].entity[pos]);
    sceneNode->yaw(Degree((pos >WALL_POS_TOP )?90:180));
    sceneNode->setPosition(tpos);
}

//================================================================================================
// Delete all walls that are scrolling out of the tile map.
//================================================================================================
void TileManager::delRowOfWalls(int row)
{
    for (int x = 0; x <= CHUNK_SIZE; ++x)
        for (int pos = 0; pos < WALL_POS_SUM; ++pos)
            if (mMap[x][row].entity[pos])
            {
                mSceneManager->destroySceneNode(mMap[x][row].entity[pos]->getParentSceneNode()->getName());
                mSceneManager->destroyEntity(mMap[x][row].entity[pos]);
                mMap[x][row].entity[pos] =0;
            }
}

//================================================================================================
// Set all wall-entities, that are scrolling into the tile map, to 0.
//================================================================================================
void TileManager::clsRowOfWalls(int row)
{
    for (int x = 0; x <= CHUNK_SIZE; ++x)
        for (int pos = 0; pos < WALL_POS_SUM; ++pos)
            mMap[x][row].entity[pos] =0;
}

//================================================================================================
// Delete all walls that are scrolling out of the tile map.
//================================================================================================
void TileManager::delColOfWalls(int col)
{
    for (int z = 0; z <= CHUNK_SIZE; ++z)
        for (int pos = 0; pos < WALL_POS_SUM; ++pos)
            if (mMap[col][z].entity[pos])
            {
                mSceneManager->destroySceneNode(mMap[col][z].entity[pos]->getParentSceneNode()->getName());
                mSceneManager->destroyEntity(mMap[col][z].entity[pos]);
                mMap[col][z].entity[pos] =0;
            }
}

//================================================================================================
// Set all wall-entities, that are scrolling into the tile map, to 0.
//================================================================================================
void TileManager::clsColOfWalls(int col)
{
    for (int z = 0; z <= CHUNK_SIZE; ++z)
        for (int pos = 0; pos < WALL_POS_SUM; ++pos)
            mMap[col][z].entity[pos] =0;
}

//================================================================================================
// When player moves over a tile border, the world scrolls.
//================================================================================================
void TileManager::syncWalls(int dx, int dz)
{
    for (int z =0; z <= CHUNK_SIZE; ++z)
        for (int x =0; x <= CHUNK_SIZE; ++x)
            for (int pos = 0; pos < WALL_POS_SUM; ++pos)
                if (mMap[x][z].entity[pos])
                    mMap[x][z].entity[pos]->getParentSceneNode()->translate(dx*TILE_SIZE, 0 , dz*TILE_SIZE);
}

//================================================================================================
// Set the values for a map position.
//================================================================================================
void TileManager::setMap(int x, int y, uchar heightVertexTL, uchar tileLayer0, uchar tileLayer1, uchar filterLayer, uchar filterShadow)
{
    mMap[x][y].height       = heightVertexTL *10;
    mMap[x][y].tileLayer[0] = tileLayer0;
    mMap[x][y].tileLayer[1] = tileLayer1;
    mMap[x][y].filterLayer  = filterLayer;
    mMap[x][y].filterShadow = filterShadow;
}

//================================================================================================
// Change all Chunks.
//================================================================================================
void TileManager::changeChunks()
{
    mMapchunk.change();
}

//================================================================================================
// Change Tile and Environmet textures.
//================================================================================================
void TileManager::changeMapset(String filenameTileTexture, String filenameEnvTexture)
{
    mMapchunk.loadAtlasTexture(filenameTileTexture);
}

//================================================================================================
// Load an existing image.
//================================================================================================
bool TileManager::loadImage(Image &image, const Ogre::String &strFilename)
{
    std::string strTemp = PATH_TILE_TEXTURES + strFilename;
    std::ifstream chkFile;
    chkFile.open(strTemp.c_str());
    if (!chkFile) return false;
    chkFile.close();
    image.load(strFilename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    return true;
}

//================================================================================================
// Copy a terrain-filter/shadow-filter into the alpha part of the atlastexture.
//================================================================================================
void TileManager::copyFilterToAtlas(uchar *dstBuf, String filename, int startRow, int stopRow)
{
    Image srcImage;
    String srcFilename;
    int sumImages = 0;
    const int FILTERS_PER_ROW = TEXTURE_SIZE / ATLAS_FILTER_SIZE;
    int lineSkip = ATLAS_FILTER_SIZE * (FILTERS_PER_ROW-1) * sizeof(uint32);
    for (int y = startRow; y < stopRow; ++y)
    {
        for (int x = 0; x < FILTERS_PER_ROW; ++x)
        {
            srcFilename = filename  + "_" + StringConverter::toString(sumImages++, 2, '0') + ".png";
            if (!loadImage(srcImage, srcFilename)) return;
            if ((srcImage.getWidth() != ATLAS_FILTER_SIZE) || (srcImage.getHeight() != ATLAS_FILTER_SIZE))
            {
                Logger::log().error() << "Graphic " << srcFilename << " has wrong size! Only "
                << (int)ATLAS_FILTER_SIZE << "x" << (int) ATLAS_FILTER_SIZE << " is supported";
                return;
            }
            bool srcAlpha = (srcImage.getFormat()==PF_A8R8G8B8);
            uchar *src = srcImage.getData();
            uchar *dst = dstBuf + (y*ATLAS_FILTER_SIZE*TEXTURE_SIZE + x*ATLAS_FILTER_SIZE) * sizeof(uint32);
            for (int y = 0; y < ATLAS_FILTER_SIZE; ++y)
            {
                for (int x = 0; x < ATLAS_FILTER_SIZE; ++x)
                {
                    dst+=3;
                    if (srcAlpha)
                    {
                        src+=3;
                        *dst++ = *src++;
                    }
                    else
                    {
                        *dst++ = *src; // R
                        src+=3;
                    }
                }
                dst+= lineSkip;
            }
        }
    }
}

//================================================================================================
// Copy a tile into the color part of the atlastexture.
//================================================================================================
bool TileManager::copyTileToAtlas(uchar *dstBuf, String filename)
{
    static int nr = -1;
    int sumImages = 0;
    const int TILES_PER_ROW = TEXTURE_SIZE / ATLAS_TILE_SIZE;
    Image srcImage;
    String srcFilename;
    ++nr;
    int lineSkip = ATLAS_TILE_SIZE * (TILES_PER_ROW-1) * sizeof(uint32);
    for (int y = 0; y < TILES_PER_ROW; ++y)
    {
        for (int x = 0; x < TILES_PER_ROW; ++x)
        {
            srcFilename = filename + "_" + StringConverter::toString(nr, 2, '0') + "_" + StringConverter::toString(sumImages++, 2, '0') + ".png";
            if (!loadImage(srcImage, srcFilename))
            {
                if (sumImages==1) return false; // No Tiles found for this group.
                return true;
            }
            if ((srcImage.getWidth() != ATLAS_TILE_SIZE) || (srcImage.getHeight() != ATLAS_TILE_SIZE))
            {
                Logger::log().error() << "Graphic " << srcFilename << " has wrong size! Only "
                << (int)ATLAS_TILE_SIZE << "x" << (int) ATLAS_TILE_SIZE << " is supported";
                return true;
            }
            bool srcAlpha = (srcImage.getFormat()==PF_A8R8G8B8);
            uchar *src = srcImage.getData();
            uchar *dst = dstBuf + (y*ATLAS_TILE_SIZE*TEXTURE_SIZE + x*ATLAS_TILE_SIZE) * sizeof(uint32);
            for (int y = 0; y < ATLAS_TILE_SIZE; ++y)
            {
                for (int x = 0; x < ATLAS_TILE_SIZE; ++x)
                {
                    *dst++ = *src++; // R
                    *dst++ = *src++; // G
                    *dst++ = *src++; // B
                    *dst++ = 0xff;   // A
                    if (srcAlpha) ++src; // Ignore alpha.
                }
                dst+= lineSkip;
            }
        }
    }
    return true;
}

//================================================================================================
// Collect all tiles and filters into a single image.
//================================================================================================
void TileManager::createAtlasTexture(const String filenameTiles, const String filenameFilters, const String filenameShadows,unsigned int groupNr)
{
    const int FILTERS_PER_ROW = TEXTURE_SIZE / ATLAS_FILTER_SIZE;
    int startGroup, stopGroup;
    if (groupNr >= (unsigned int) MAX_MAP_SETS)
    {
        startGroup = 0;
        stopGroup  = MAX_MAP_SETS;
    }
    else
    {
        startGroup = groupNr;
        stopGroup  = startGroup+1;
    }
    Image dstImage;
    uchar *dstBuf = new uchar[TEXTURE_SIZE * TEXTURE_SIZE * sizeof(uint32)];
    // Buffer will not be cleared, so previous drawn tiles/filters are still there.
    for (int nr = startGroup; nr < stopGroup; ++nr)
    {
        // Copy the tiles into the atlastexture.
        if (!copyTileToAtlas(dstBuf, filenameTiles)) break;
        // Copy the filters into the atlastexture.
        copyFilterToAtlas(dstBuf, filenameFilters, 0, FILTERS_PER_ROW/2);
        copyFilterToAtlas(dstBuf, filenameShadows, FILTERS_PER_ROW/2, FILTERS_PER_ROW);
        // Save the Atlastexture.
        dstImage.loadDynamicImage(dstBuf, TEXTURE_SIZE, TEXTURE_SIZE, PF_A8R8G8B8);
        String dstFilename = PATH_TILE_TEXTURES + filenameTiles;
        dstFilename+= "_group_"+ StringConverter::toString(nr,2,'0') + "_";
        for (unsigned short s = TEXTURE_SIZE; s >= TEXTURE_SIZE/2; s/=2)
        {
            dstImage.save(dstFilename + StringConverter::toString(s, 4, '0') + ".png");
            //dstImage.resize(s/2, s/2, Image::FILTER_BILINEAR); // crashes on linux.
        }
    }
    delete[] dstBuf;
}

//================================================================================================
// Helper function for getTileHeight(...);
//================================================================================================
int TileManager::calcHeight(int vert0, int vert1, int vert2, int posX, int posZ)
{
    if (posZ == TILE_SIZE) return vert1;
    int h1 = ((vert1 - vert0) * posZ) / TILE_SIZE + vert0;
    int h2 = ((vert1 - vert2) * posZ) / TILE_SIZE + vert2;
    int maxX = TILE_SIZE - posZ;
    return ((h2 - h1) * posX) / maxX + h1;
}

//================================================================================================
// Return the exact height of a position within a tile.
//================================================================================================
int TileManager::getTileHeight(int posX, int posZ)
{
    int mapX, mapZ;
    int TileX = posX / TILE_SIZE; // Get the Tile position within the map.
    int TileZ = posZ / TILE_SIZE; // Get the Tile position within the map.
    posX&= (TILE_SIZE-1);         // Lower part is the position within the tile.
    posZ&= (TILE_SIZE-1);         // Lower part is the position within the tile.
    getMapScroll(mapX, mapZ);
    mapX += (mapZ&1);
    //   +-+v2
    //   |/|
    // v1+-+
    if (mapX&1)
    {
        int v1 = getMapHeight(TileX, TileZ, VERTEX_BL);
        int v2 = getMapHeight(TileX, TileZ, VERTEX_TR);
        if (TILE_SIZE - posX > posZ)
            return calcHeight(getMapHeight(TileX, TileZ, VERTEX_TL), v1, v2, posX, posZ);
        return calcHeight(getMapHeight(TileX, TileZ, VERTEX_BR), v1, v2, TILE_SIZE-posZ, TILE_SIZE-posX);;
    }
    // v1+-+
    //   |\|
    //   +-+v2
    int v1 = getMapHeight(TileX, TileZ, VERTEX_TL);
    int v2 = getMapHeight(TileX, TileZ, VERTEX_BR);
    if (posX < posZ)
        return calcHeight(getMapHeight(TileX, TileZ, VERTEX_BL), v1, v2, posX, TILE_SIZE-posZ);
    return calcHeight(getMapHeight(TileX, TileZ, VERTEX_TR), v1, v2, posZ, TILE_SIZE-posX);;
}
