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
#include "logger.h"
#include "tile_chunk.h"
#include "tile_manager.h"

#include "object_manager.h"

using namespace Ogre;

//#define LOG_TIMING

//================================================================================================
// Free all resources.
//================================================================================================
void TileManager::freeRecources()
{
    mMapchunk.freeRecources();
    mSceneManager->destroyQuery(mRaySceneQuery);
}

//================================================================================================
// Init the TileEngine.
//================================================================================================
void TileManager::Init(SceneManager* SceneMgr, int lod, bool createAtlas)
{
    Logger::log().headline("Init TileEngine");
    mShowGrid = false;
    mMapScrollX = mMapScrollZ = 0;
    mSelectedVertexX = mSelectedVertexZ = 0; // Tile picking.
    mSceneManager = SceneMgr;
    mRaySceneQuery = mSceneManager->createRayQuery(Ray());
    mLod = lod&3;
    int textureSize = MAX_TEXTURE_SIZE >> mLod;
    Logger::log().info() << "Setting LoD to " << mLod << ". Atlas size is " << textureSize << "x" << textureSize<< ".";
    //if (createAtlas)
    {
        Logger::log().info() << "Creating atlas-texture...";
        createAtlasTexture(MAX_TEXTURE_SIZE);
        //createShadowAtlas("shadow");
        Logger::log().success(true);
    }
    Logger::log().info() << "Creating tile chunk...";
    mMapchunk.init(textureSize);
    Logger::log().success(true);
    Logger::log().info() << "Init done.";
}

//================================================================================================
// Collect all tiles and filters into a single RGBA-image.
// The upper half of the atlastexture is used for tile graphics.
// The lower half of the atlastexture is used for environment graphics (stones, bushes, trees,...)
//================================================================================================
void TileManager::createAtlasTexture(int textureSize, unsigned int groupNr)
{
/*
    // Only for creating dummy filters. DELETE ME!
    {
        Image srcImage;
        String srcFilename;
        for (int filter =0; filter < 4; filter++)
        {
            srcFilename = "filter_00_";
            srcFilename+= 'a' + filter;
            srcFilename+= ".png";
            loadImage(srcImage, srcFilename);
            for (int i =1; i < 32; ++i)
            {
                srcFilename = "c:\\filter_" + StringConverter::toString(i, 2, '0') + "_";
                srcFilename+= 'a' + filter;
                srcFilename+= ".png";
                srcImage.save(srcFilename);
            }
        }
    }
*/
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
    uchar *dstBuf = new uchar[textureSize * textureSize * sizeof(uint32)];
    // Buffer will not be cleared, so previous drawn tiles/filters are still there.
    for (int nr = startGroup; nr < stopGroup; ++nr)
    {
        if (!copyTileToAtlas(dstBuf)) break;
        for (int i=0; i < 4; ++i) copyFilterToAtlas(dstBuf, i);

        // Save the Atlastexture.
        dstImage.loadDynamicImage(dstBuf, textureSize, textureSize, PF_A8R8G8B8);
        String dstFilename = PATH_TILE_TEXTURES;
        dstFilename+= "Atlas_"+ StringConverter::toString(nr,2,'0') + "_";
        for (unsigned short s = textureSize; s >= textureSize/8; s/=2)
        {
            dstImage.save(dstFilename + StringConverter::toString(s, 4, '0') + ".png");
            dstImage.resize(s/2, s/2, Image::FILTER_BILINEAR); // crashes on linux.
        }
    }
    delete[] dstBuf;
}

//================================================================================================
// Copy a tile into the color part of the atlastexture.
//================================================================================================
bool TileManager::copyTileToAtlas(uchar *dstBuf)
{
    static int nr = -1;
    int sumImages = 0;
    unsigned int tileSize = MAX_TEXTURE_SIZE / COLS_SRC_TILES;
    Image srcImage;
    String srcFilename;
    ++nr;
    int lineSkip = (MAX_TEXTURE_SIZE - tileSize) * sizeof(uint32);
    for (int y = 0; y < COLS_SRC_TILES; ++y)
    {
        for (int x = 0; x < COLS_SRC_TILES; x+=2)
        {
            srcFilename = "terrain_" + StringConverter::toString(nr, 2, '0') + "_" + StringConverter::toString(sumImages++, 2, '0') + ".png";
            if (!loadImage(srcImage, srcFilename))
            {
                if (sumImages==1) return false; // No Tiles found for this group.
                return true;
            }
            if ((srcImage.getWidth() != tileSize) || (srcImage.getHeight() != tileSize))
            {
                Logger::log().error() << "Gfx " << srcFilename << " has the wrong size! Only " << tileSize << "x" << tileSize << " is supported";
                return true;
            }
            bool srcAlpha = (srcImage.getFormat()==PF_A8R8G8B8);
            uchar *src = srcImage.getData();
            uchar *dst1= dstBuf + (y * tileSize * MAX_TEXTURE_SIZE + x * tileSize) * sizeof(uint32);
            uchar *dst2= dst1 + tileSize * sizeof(uint32);
            for (unsigned int y = 0; y < tileSize; ++y)
            {
                for (unsigned int x = 0; x < tileSize; ++x)
                {
                    *dst2++ = *src;   // R
                    *dst1++ = *src++; // R
                    *dst2++ = *src;   // G
                    *dst1++ = *src++; // G
                    *dst2++ = *src;   // B
                    *dst1++ = *src++; // B
                    *dst2++ = *src;   // A
                    *dst1++ = 0xff;   // A
                    if (srcAlpha) ++src; // Ignore alpha.
                }
                dst1+= lineSkip;
                dst2+= lineSkip;
            }
        }
    }
    return true; }

//================================================================================================
// Copy a terrain-filter/shadow-filter into the alpha part of the atlastexture.
//=========================================================== =====================================
void TileManager::copyFilterToAtlas(uchar *dstBuf, int filter)
{
    int sumImages = 0;
    unsigned int tileSize = MAX_TEXTURE_SIZE / COLS_SRC_TILES/2;
    Image srcImage;
    String srcFilename;
    int lineSkip = (MAX_TEXTURE_SIZE - tileSize) * sizeof(uint32);
    for (int y = 0; y < COLS_SRC_TILES; ++y)
    {
        for (int x = 0; x < COLS_SRC_TILES; x+=2)
        {
            srcFilename = "filter_" + StringConverter::toString(sumImages++, 2, '0') + "_";
            srcFilename+= 'a' + filter;
            srcFilename+= ".png";
            if (!loadImage(srcImage, srcFilename)) return;
            if ((srcImage.getWidth() != tileSize) || (srcImage.getHeight() != tileSize))
            {
                Logger::log().error() << "Filter " << srcFilename << " has the wrong size! Only " << tileSize << "x" << tileSize << " is supported";
                return;
            }
            bool srcAlpha = (srcImage.getFormat()==PF_A8R8G8B8);
            uchar *src = srcImage.getData();
            uchar *dst1= dstBuf + (y*2 * tileSize * MAX_TEXTURE_SIZE + x*2 * tileSize + filter *tileSize) * sizeof(uint32);
            uchar *dst2= dst1 + (tileSize * MAX_TEXTURE_SIZE + tileSize)* sizeof(uint32);
            if (filter==1 || filter ==3) dst2-= (2*tileSize)* sizeof(uint32);
            for (unsigned int y = 0; y < tileSize; ++y)
            {
                for (unsigned int x = 0; x < tileSize; ++x)
                {
                    dst1+=3;
                    dst2+=3;
                    if (srcAlpha)
                    {
                        src+=3;
                        *dst2++ = *src;
                        *dst1++ = *src++;
                    }
                    else
                    {
                        *dst2++ = *src;
                        *dst1++ = *src; // R
                        src+=3;
                    }
                }
                dst1+= lineSkip;
                dst2+= lineSkip;
            }
        }
    }
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
    if (vertex == VERTEX_TR) return mMap[x+1][z  ].height;
    if (vertex == VERTEX_BL) return mMap[x  ][z+1].height;
    if (vertex == VERTEX_BR) return mMap[x+1][z+1].height;
    return mMap[x][z].height;
}

//================================================================================================
// Returns the gfx of a tile-vertex.
//================================================================================================
char TileManager::getMapGfx(unsigned int x, unsigned int z, int vertex)
{
    if (x >= MAP_SIZE || z >= MAP_SIZE) return 0;
    if (vertex == VERTEX_TR) return mMap[x+1][z  ].gfx;
    if (vertex == VERTEX_BL) return mMap[x  ][z+1].gfx;
    if (vertex == VERTEX_BR) return mMap[x+1][z+1].gfx;
    return mMap[x][z].gfx;
}

//================================================================================================
// Scroll the map.
//================================================================================================
void TileManager::scrollMap(int dx, int dz)
{
    if (dx <0)
    {
        mMapScrollX-=2;
        for (int x = 0; x < CHUNK_SIZE_X; ++x)
            for (int y = 0; y <= CHUNK_SIZE_Z; ++y)
                mMap[x][y] = mMap[x+2][y];
    }
    else if (dx >0)
    {
        mMapScrollX+=2;
        for (int x = CHUNK_SIZE_X; x >0; --x)
            for (int y = 0; y <= CHUNK_SIZE_Z; ++y)
                mMap[x][y] = mMap[x-2][y];
    }
    if (dz <0)
    {
        mMapScrollZ-=2;
        for (int x = 0; x <= CHUNK_SIZE_X; ++x)
            for (int y = 0; y < CHUNK_SIZE_Z; ++y)
                mMap[x][y] = mMap[x][y+2];
    }
    else if (dz >0)
    {
        mMapScrollZ+=2;
        for (int x = 0; x <= CHUNK_SIZE_X; ++x)
            for (int y = CHUNK_SIZE_Z; y > 0; --y)
                mMap[x][y] = mMap[x][y-2];
    }
    mMapchunk.change();
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
// .
//================================================================================================
void TileManager::updateTileGfx(int deltaGfxNr)
{
    mMap[mSelectedVertexX][mSelectedVertexZ].gfx+= deltaGfxNr;
    mMapchunk.change();
    highlightVertex(mSelectedVertexX, mSelectedVertexZ);
}

//================================================================================================
// Set the values for a map position.
//================================================================================================
void TileManager::setMap(unsigned int x, unsigned int y, short height, char gfx, char shadow, char mirror)
{
    mMap[x][y].height = height *10;
    mMap[x][y].gfx = gfx;
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
    mMapchunk.loadAtlasTexture(0);
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
        return calcHeight(getMapHeight(TileX, TileZ, VERTEX_BR), v1, v2, TILE_SIZE-posZ, TILE_SIZE-posX);
    }
    // v1+-+
    //   |\|
    //   +-+v2
    int v1 = getMapHeight(TileX, TileZ, VERTEX_TL);
    int v2 = getMapHeight(TileX, TileZ, VERTEX_BR);
    if (posX < posZ)
        return calcHeight(getMapHeight(TileX, TileZ, VERTEX_BL), v1, v2, posX, TILE_SIZE-posZ);
    return calcHeight(getMapHeight(TileX, TileZ, VERTEX_TR), v1, v2, posZ, TILE_SIZE-posX);
}
