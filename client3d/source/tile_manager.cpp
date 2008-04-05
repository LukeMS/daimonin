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
#include "gui_manager.h"

#include "logger.h"

using namespace Ogre;

//#define LOG_TIMING

//================================================================================================
// Constructor.
//================================================================================================
TileManager::TileManager()
{
    mShowGrid = false;
    mMapScrollX =0;
    mMapScrollZ =0;
    mSelectedVertexX =0;
    mSelectedVertexZ =0;
    for (int z =0; z <= CHUNK_SIZE_Z; ++z)
        clsRowOfWalls(z);
}

//================================================================================================
// Free all resources.
//================================================================================================
void TileManager::freeRecources()
{
    for (int z =0; z <= CHUNK_SIZE_Z; ++z)
        delRowOfWalls(z);
    mMapchunk.freeRecources();
    mSceneManager->destroyQuery(mRaySceneQuery);
}

//================================================================================================
// A tile was clicked.
//================================================================================================
void TileManager::tileClick(float mouseX, float mouseY)
{
    Ray mouseRay = mSceneManager->getCamera("PlayerCam")->getCameraToViewportRay(mouseX, mouseY);
    mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setQueryMask(ObjectManager::QUERY_TILES_LAND_MASK);
    // Perform the scene query.
    RaySceneQueryResult &result = mRaySceneQuery->execute();
    if (result.size() >1)
    {
        Logger::log().error() << "BUG in tileClick(...): RaySceneQuery returned more than 1 result.";
        Logger::log().error() << "(Perhaps you created Entities without setting a setQueryFlags(...) on them)";
        return;
    }

    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z)
        {
            if ((x+z)&1)
            {
                // +-+
                // |/
                // +
                mTris[0].x = (x+0)*TILE_SIZE;
                mTris[0].y = getMapHeight(x, z, VERTEX_TL);
                mTris[0].z = (z+0)*TILE_SIZE;
                mTris[1].x = (x+0)*TILE_SIZE;
                mTris[1].y = getMapHeight(x, z, VERTEX_BL);
                mTris[1].z = (z+1)*TILE_SIZE;
                mTris[2].x = (x+1)*TILE_SIZE;
                mTris[2].y = getMapHeight(x, z, VERTEX_TR);
                mTris[2].z = (z+0)*TILE_SIZE;
                if (vertexPick(&mouseRay, x, z, 0)) return; // We got a hit.
                //   +
                //  /|
                // +-+
                mTris[0].x = (x+1)*TILE_SIZE;
                mTris[0].y = getMapHeight(x, z, VERTEX_TR);
                mTris[0].z = (z+0)*TILE_SIZE;
                mTris[1].x = (x+0)*TILE_SIZE;
                mTris[1].y = getMapHeight(x, z, VERTEX_BL);
                mTris[1].z = (z+1)*TILE_SIZE;
                mTris[2].x = (x+1)*TILE_SIZE;
                mTris[2].y = getMapHeight(x, z, VERTEX_BR);
                mTris[2].z = (z+1)*TILE_SIZE;
                if (vertexPick(&mouseRay, x, z, 1)) return; // We got a hit.
            }
            else
            {
                //
                //   +
                //   |\.
                //   +-+
                mTris[0].x = (x+0.0)*TILE_SIZE;
                mTris[0].y = getMapHeight(x, z, VERTEX_TL);
                mTris[0].z = (z+0.0)*TILE_SIZE;
                mTris[1].x = (x+0.0)*TILE_SIZE;
                mTris[1].y = getMapHeight(x, z, VERTEX_BL);
                mTris[1].z = (z+1.0)*TILE_SIZE;
                mTris[2].x = (x+1.0)*TILE_SIZE;
                mTris[2].y = getMapHeight(x, z, VERTEX_BR);
                mTris[2].z = (z+1.0)*TILE_SIZE;
                if (vertexPick(&mouseRay, x, z, 2)) return; // We got a hit.
                // +-+
                //  \|
                //   +
                mTris[0].x = (x+1.0)*TILE_SIZE;
                mTris[0].y = getMapHeight(x, z, VERTEX_BR);
                mTris[0].z = (z+1.0)*TILE_SIZE;
                mTris[1].x = (x+1.0)*TILE_SIZE;
                mTris[1].y = getMapHeight(x, z, VERTEX_TR);
                mTris[1].z = (z+0.0)*TILE_SIZE;
                mTris[2].x = (x+0.0)*TILE_SIZE;
                mTris[2].y = getMapHeight(x, z, VERTEX_TL);
                mTris[2].z = (z+0.0)*TILE_SIZE;
                if (vertexPick(&mouseRay, x, z, 3))  return; // We got a hit.
            }
        }
    }
}

//================================================================================================
// .
//================================================================================================
bool TileManager::vertexPick(Ray *mouseRay, int x, int z, int pos)
{
    std::pair<bool, Real> Test;
    Test = Math::intersects(*mouseRay, mTris[0], mTris[1], mTris[2]);
    if (!Test.first) return false;  // This tile piece was not clicked.
    // Test for Vertex 0
    Test = Math::intersects(*mouseRay, mTris[0], (mTris[0] + mTris[2])/2, (mTris[0] + mTris[1])/2);
    if (Test.first)
    {
        if      (pos == 0) highlightVertex(x  , z  );
        else if (pos == 1) highlightVertex(x+1, z  );
        else if (pos == 2) highlightVertex(x  , z  );
        else if (pos == 3) highlightVertex(x+1, z+1);
        return true;
    }
    // Test for Vertex 1
    Test = Math::intersects(*mouseRay, mTris[2], (mTris[0] + mTris[2])/2, (mTris[1] + mTris[2])/2);
    if (Test.first)
    {
        if      (pos == 0) highlightVertex(x+1, z  );
        else if (pos == 1) highlightVertex(x+1, z+1);
        else if (pos == 2) highlightVertex(x+1, z+1);
        else if (pos == 3) highlightVertex(x  , z  );
        return true;
    }
    // Test for Vertex 2
    Test = Math::intersects(*mouseRay, mTris[1], (mTris[0] + mTris[1])/2, (mTris[1] + mTris[2])/2);
    if (Test.first)
    {
        if      (pos == 0) highlightVertex(x  , z+1);
        else if (pos == 1) highlightVertex(x  , z+1);
        else if (pos == 2) highlightVertex(x  , z+1);
        else if (pos == 3) highlightVertex(x+1, z  );
        return true;
    }
    return true;
}

//================================================================================================
// .
//================================================================================================
void TileManager::highlightVertex(int x, int z)
{
    static SceneNode *tcNode = 0;
    mSelectedVertexX = x;
    mSelectedVertexZ = z;
    if (tcNode)
    {
        tcNode->detachAllObjects();
        mSceneManager->destroyManualObject("SelTest");
        mSceneManager->destroySceneNode("N1");
    }
    int y = getMapHeight(x, z, VERTEX_TL)+1;
    x*= TILE_SIZE;
    z*= TILE_SIZE;
    int size = TILE_SIZE/8;
    ManualObject* mob = static_cast<ManualObject*>(mSceneManager->createMovableObject("SelTest", ManualObjectFactory::FACTORY_TYPE_NAME));
    mob->begin("TileEngine/VertexSelection");
    mob->position(x-size, y, z-size); mob->normal(0,1,0); mob->textureCoord(0.0, 0.0);
    mob->position(x-size, y, z+size); mob->normal(0,1,0); mob->textureCoord(0.0, 1.0);
    mob->position(x+size, y, z-size); mob->normal(0,1,0); mob->textureCoord(1.0, 0.0);
    mob->position(x+size, y, z+size); mob->normal(0,1,0); mob->textureCoord(1.0, 1.0);
    mob->quad(0, 1, 3, 2);
    mob->end();
    tcNode = mSceneManager->getRootSceneNode()->createChildSceneNode("N1");
    tcNode->attachObject(mob);
}

//================================================================================================
// Returns the height of a tile-vertex.
//================================================================================================
short TileManager::getMapHeight(unsigned int x, unsigned int z, int vertex)
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
// Init the TileEngine.
//================================================================================================
void TileManager::Init(SceneManager* SceneMgr, int sumTilesX, int sumTilesZ, int zeroX, int zeroZ, int highDetails)
{
    Logger::log().headline("Init TileEngine");
    mSceneManager = SceneMgr;
    mRaySceneQuery = mSceneManager->createRayQuery(Ray());
    mLod  = highDetails;
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
        for (int x = 0; x < CHUNK_SIZE_X; ++x)
            for (int y = 0; y <= CHUNK_SIZE_Z; ++y)
                mMap[x][y] = mMap[x+1][y];
    }
    else if (dx <0)
    {
        ++mMapScrollX;
        for (int x = CHUNK_SIZE_X; x >0; --x)
            for (int y = 0; y <= CHUNK_SIZE_Z; ++y)
                mMap[x][y] = mMap[x-1][y];
    }
    if (dz >0)
    {
        --mMapScrollZ;
        for (int x = 0; x <= CHUNK_SIZE_X; ++x)
            for (int y = 0; y < CHUNK_SIZE_Z; ++y)
                mMap[x][y] = mMap[x][y+1];
    }
    else if (dz <0)
    {
        ++mMapScrollZ;
        for (int x = 0; x <= CHUNK_SIZE_X; ++x)
            for (int y = CHUNK_SIZE_Z; y > 0; --y)
                mMap[x][y] = mMap[x][y-1];
    }
    mMapchunk.change();
}

//================================================================================================
// Returns the gfx number of the shadow.
//================================================================================================
uchar TileManager::calcShadow(int x, int z)
{
    int tl, tr, bl, br;
    // ////////////////////////////////////////////////////////////////////
    // I - III
    // ////////////////////////////////////////////////////////////////////
    bl = getMapHeight(x-1, z, VERTEX_BL);
    br = getMapHeight(x-1, z, VERTEX_BR);
    if (bl-br > 10 && bl-br < 100)
    {
        tl = getMapHeight(x-1, z, VERTEX_TL);
        tr = getMapHeight(x-1, z, VERTEX_TR);
        // ////////////////////////////////////////////////////////////////////
        // II
        // ////////////////////////////////////////////////////////////////////
        if (bl-br < 64)
        {
            if (tl-tr >= 220) return 42;
            if (tl-tr >= 180) return 32;
            if (tl-tr >= 140) return 22;
            if (tl-tr >= 100) return 12;
            if (tl-tr >=  65) return  2;
        }
        bl = getMapHeight(x-1, z+1, VERTEX_BL);
        br = getMapHeight(x-1, z+1, VERTEX_BR);
        // ////////////////////////////////////////////////////////////////////
        // I
        // ////////////////////////////////////////////////////////////////////
        if (bl-br < 10)
        {
            if (tl-tr >= 220) return 41;
            if (tl-tr >= 180) return 31;
            if (tl-tr >= 140) return 21;
            if (tl-tr >= 100) return 11;
            if (tl-tr >=  65) return  1;
        }
        // ////////////////////////////////////////////////////////////////////
        // III
        // ////////////////////////////////////////////////////////////////////
        if (bl-br > 64)
        {
            if (tl-tr >= 220) return 43;
            if (tl-tr >= 180) return 33;
            if (tl-tr >= 140) return 23;
            if (tl-tr >= 100) return 13;
            if (tl-tr >=  65) return  3;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // IV
    // ////////////////////////////////////////////////////////////////////
    bl = getMapHeight(x, z+2, VERTEX_BL);
    br = getMapHeight(x, z+2, VERTEX_BR);
    tl = getMapHeight(x, z+2, VERTEX_TL);
    tr = getMapHeight(x, z+2, VERTEX_TR);
    if (bl < 64 && br < 64 && tl < 64 && tr < 64)
    {
        br =      getMapHeight(x-1, z+1, VERTEX_BR);
        tl = br - getMapHeight(x-1, z+1, VERTEX_TL);
        tr = br - getMapHeight(x-1, z+1, VERTEX_TR);
        bl = br - getMapHeight(x-1, z+1, VERTEX_BL);
        if (tl >= 220 && tr >= 220 && bl >= 220) return 44;
        if (tl >= 180 && tr >= 180 && bl >= 180) return 34;
        if (tl >= 140 && tr >= 140 && bl >= 140) return 24;
        if (tl >= 100 && tr >= 100 && bl >= 100) return 14;
        if (tl >= 65  && tr >=  65 && bl >=  65) return  4;
    }
    // ////////////////////////////////////////////////////////////////////
    // V
    // ////////////////////////////////////////////////////////////////////
    // typo from red ????

    // ////////////////////////////////////////////////////////////////////
    // VI
    // ////////////////////////////////////////////////////////////////////
    bl = getMapHeight(x-2, z, VERTEX_BL);
    br = getMapHeight(x-2, z, VERTEX_BR);
    tl = getMapHeight(x-2, z, VERTEX_TL);
    tr = getMapHeight(x-2, z, VERTEX_TR);
    if (bl < 64 && br < 64 && tl < 64 && tr < 64)
    {
        br =      getMapHeight(x-1, z+1, VERTEX_BR);
        tl = br - getMapHeight(x-1, z+1, VERTEX_TL);
        tr = br - getMapHeight(x-1, z+1, VERTEX_TR);
        bl = br - getMapHeight(x-1, z+1, VERTEX_BL);
        if (tl >= 220 && tr >= 220 && bl >= 220) return 46;
        if (tl >= 180 && tr >= 180 && bl >= 180) return 36;
        if (tl >= 140 && tr >= 140 && bl >= 140) return 26;
        if (tl >= 100 && tr >= 100 && bl >= 100) return 16;
        if (tl >= 65  && tr >=  65 && bl >=  65) return  6;
    }
    // ////////////////////////////////////////////////////////////////////
    // VII
    // ////////////////////////////////////////////////////////////////////
    bl = getMapHeight(x-1, z+1, VERTEX_BL);
    tl = getMapHeight(x-1, z+1, VERTEX_TL);
    if (bl-tl > 64)
    {
        bl = getMapHeight(x, z+1, VERTEX_BL);
        br = getMapHeight(x, z+1, VERTEX_BR);
        tl = getMapHeight(x, z+1, VERTEX_TL);
        tr = getMapHeight(x, z+1, VERTEX_TR);
        if (bl-tl >= 220 && br-tr >= 220) return 47;
        if (bl-tl >= 180 && br-tr >= 180) return 37;
        if (bl-tl >= 140 && br-tr >= 140) return 27;
        if (bl-tl >= 100 && br-tr >= 100) return 17;
        if (bl-tl >=  65 && br-tr >=  65) return  7;
    }
    // ////////////////////////////////////////////////////////////////////
    // VIII
    // ////////////////////////////////////////////////////////////////////
    else if (bl-bl < 10)
    {
        bl = getMapHeight(x, z+1, VERTEX_BL);
        br = getMapHeight(x, z+1, VERTEX_BR);
        tl = getMapHeight(x, z+1, VERTEX_TL);
        tr = getMapHeight(x, z+1, VERTEX_TR);
        if (bl-tl >= 220 && br-tr >= 220) return 48;
        if (bl-tl >= 180 && br-tr >= 180) return 38;
        if (bl-tl >= 140 && br-tr >= 140) return 28;
        if (bl-tl >= 100 && br-tr >= 100) return 18;
        if (bl-tl >=  65 && br-tr >=  65) return  8;
    }
    // ////////////////////////////////////////////////////////////////////
    // IX
    // ////////////////////////////////////////////////////////////////////
    bl = getMapHeight(x+1, z+2, VERTEX_BL);
    br = getMapHeight(x+1, z+2, VERTEX_BR);
    if (bl-tl > 10 && bl-tl < 65)
    {
        tl = getMapHeight(x, z+1, VERTEX_TL);
        tr = getMapHeight(x, z+1, VERTEX_TR);
        if (tl-tr >= 220) return 49;
        if (tl-tr >= 180) return 39;
        if (tl-tr >= 140) return 29;
        if (tl-tr >= 100) return 19;
        if (tl-tr >=  65) return  9;
    }
    // ////////////////////////////////////////////////////////////////////
    // X
    // ////////////////////////////////////////////////////////////////////
    //From here to end is still not possible to code becau.




    // ////////////////////////////////////////////////////////////////////
    // No shadow.
    // ////////////////////////////////////////////////////////////////////
    return 66;
}

//================================================================================================
//
//================================================================================================
void TileManager::calcMapShadows()
{
    for (int z = 0; z <= CHUNK_SIZE_Z; ++z)
    {
        for (int x = 0; x <= CHUNK_SIZE_X; ++x)
        {
            mMap[x][z].shadow = calcShadow(x, z);
        }
    }
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
}

//================================================================================================
// Set all wall-entities, that are scrolling into the tile map, to 0.
//================================================================================================
void TileManager::clsRowOfWalls(int row)
{
}

//================================================================================================
// Delete all walls that are scrolling out of the tile map.
//================================================================================================
void TileManager::delColOfWalls(int col)
{
}

//================================================================================================
// Set all wall-entities, that are scrolling into the tile map, to 0.
//================================================================================================
void TileManager::clsColOfWalls(int col)
{
}

//================================================================================================
// When player moves over a tile border, the world scrolls.
//================================================================================================
void TileManager::syncWalls(int dx, int dz)
{
}

//================================================================================================
// .
//================================================================================================
void TileManager::updateTileHeight(int deltaHeight)
{
    mMap[mSelectedVertexX][mSelectedVertexZ].height+= deltaHeight;
    mMapchunk.change();
    highlightVertex(mSelectedVertexX, mSelectedVertexZ);
}

//================================================================================================
// Set the values for a map position.
//================================================================================================
void TileManager::setMap(int x, int y, short height, uchar layer0, uchar layer1, uchar filter, uchar shadow, uchar mirror)
{
    mMap[x][y].height = height *10;
    mMap[x][y].layer0 = layer0;
    mMap[x][y].layer1 = layer1;
    mMap[x][y].filter = filter;
    mMap[x][y].shadow = shadow;
    mMap[x][y].mirror = mirror;
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
short TileManager::getTileHeight(int posX, int posZ)
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
