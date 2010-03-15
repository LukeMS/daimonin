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

#include <OgreSceneManager.h>
#include <OgreManualObject.h>
#include <OgreConfigFile.h>
#include <sys/stat.h>
#include "logger.h"
#include "tile_chunk.h"
#include "tile_decal.h"
#include "tile_manager.h"

using namespace Ogre;

#define LOG_TIMING

#ifdef LOG_TIMING
#include <OgreRoot.h>
#include <OgreTimer.h>
#endif

String TileManager::LAND_PREFIX    = "Land";
String TileManager::WATER_PREFIX   = "Water";
String TileManager::ATLAS_PREFIX   = "Atlas_Tiles";
String TileManager::MATERIAL_PREFIX= "Terrain/";
const unsigned int RGB  = 3; /**< Pixelsize. **/
const unsigned int RGB_A= 4; /**< Pixelsize. **/

//////// Only for TESTING
#include <stdio.h>
void TileManager::loadLvl()
{
    FILE *stream = fopen("client3d.lvl", "rb");
    fread(&mMap, sizeof(mMap), 1, stream);
    fclose(stream);
    updateChunks();
}

void TileManager::saveLvl()
{
    FILE *stream = fopen("client3d.lvl", "wb");
    fwrite(&mMap, sizeof(mMap), 1, stream);
    fclose(stream);
}
//////////////////////////

SceneManager *TileManager::mSceneManager = 0;

//================================================================================================
// Constructor.
//================================================================================================
TileManager::TileManager()
{
    mMap = 0;
}

//================================================================================================
// Destructor.
//================================================================================================
TileManager::~TileManager()
{
    if (mMap ) Logger::log().error() << "TileManager::freeRecources() was not called!";
}

//================================================================================================
// Free all resources.
//================================================================================================
void TileManager::freeRecources()
{
    if (!mMap) return; // Init(...) was never called.
    mSceneManager->destroyQuery(mRaySceneQuery);
    delete[] mMap; mMap = 0;
    int count = TileDecal::getSumDecals();
    if (count)
    {
        Logger::log().error() << count << " Decal(s) were created by 'new' but not destroyed by 'delete'. "
        "(All decals must be deleted before TileManager::freeRecources() is called)";
    }
}

//================================================================================================
// Init the TileEngine.
//================================================================================================
void TileManager::Init(SceneManager *SceneMgr, int queryMaskLand, int queryMaskWater, int lod, bool createAtlas)
{
    Logger::log().headline() << "Init TileEngine";
    mSceneManager = SceneMgr;
    if (createAtlas)
    {
        Logger::log().info() << "Creating atlas-texture...";
        if (setResourcePath("tiles", mPathGfxTiles))
        {
            createMaskTemplate();
            createAtlasTexture(MAX_TEXTURE_SIZE, 0);
        }
    }
    mRaySceneQuery = mSceneManager->createRayQuery(Ray());
    // Create the world map.
    mMapSizeX = 1; while (mMapSizeX < CHUNK_SIZE_X*2+4) mMapSizeX <<= 1; // Map size mst be power of 2.
    mMapSizeZ = 1; while (mMapSizeZ < CHUNK_SIZE_Z*2+4) mMapSizeZ <<= 1; // Map size mst be power of 2.
    mMapMaskX = mMapSizeX -1;
    mMapMaskZ = mMapSizeZ -1;
    mMapSPosX = 1; // Ringbuffer start pos x.
    mMapSPosZ = 1; // Ringbuffer start pos z.
    Logger::log().info() << "Map size: " << mMapSizeX     << " * " << mMapSizeZ      << " Subtiles.";
    Logger::log().info() << "Visible: " << CHUNK_SIZE_X*2 << " * " << CHUNK_SIZE_Z*2 << " Subtiles.";
    mMap = new mapStruct[mMapSizeX*mMapSizeZ];
    mQueryMaskLand = queryMaskLand;
    mEditorActSelectedGfx = 0;
    mSelectedVertexX = mSelectedVertexZ = 0; // Tile picking.
    int Lod = lod &(SUM_ATLAS_RESOLUTIONS-1);
    mTextureSize = MAX_TEXTURE_SIZE >> Lod;
    Logger::log().info() << "Setting LoD to " << Lod << ". Atlas size is " << mTextureSize << "x" << mTextureSize<< ".";
    Logger::log().info() << "Creating tile chunk...";
    mMapchunk.init(queryMaskLand, queryMaskWater, mSceneManager);
    setMapset(0, 0);
    //setLight(1.0f);
    setLight(0.6f);
    setWave(0.5, HEIGHT_STRETCH, 1.5);
    setGrid(false);
    setRenderOptions(false);
    Logger::log().success(true);
}

//================================================================================================
// Change all Chunks.
//================================================================================================
void TileManager::updateChunks()
{
#ifdef LOG_TIMING
    unsigned long time = Root::getSingleton().getTimer()->getMicroseconds();
#endif
    mMapchunk.update();
#ifdef LOG_TIMING
    Logger::log().error() << "Time to change terrain: " << (double)(Root::getSingleton().getTimer()->getMicroseconds() - time)/1000 << " ms";
#endif
}

//================================================================================================
// Set the values for a map position.
// Within a tile there are only 4 possible positions for a spotlight (marked by a 'S' here).
// +--S--+
// | \|/ |
// S--+--S
// | /|\ |
// +--S--+
//================================================================================================
void TileManager::setMap(unsigned int x, unsigned int z, uchar heightLand, uchar gfxLayer0, uchar heightWater, uchar shadow, uchar gfxLayer1, bool spotLight)
{
    int ringBufferPos = ((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX);
    mMap[ringBufferPos].gfxLayer0  = gfxLayer0;
    mMap[ringBufferPos].gfxLayer1  = gfxLayer1;
    mMap[ringBufferPos].heightLand = heightLand;
    mMap[ringBufferPos].heightWater= heightWater;
    mMap[ringBufferPos].shadow     = shadow;
    mMap[ringBufferPos].spotLight  = spotLight;
}

//================================================================================================
// Returns the height of a tile-vertex.
//================================================================================================
Ogre::ushort TileManager::getMapHeight(unsigned int x, unsigned int z)
{
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].heightLand * HEIGHT_STRETCH;
}

//================================================================================================
// Returns the water level
//================================================================================================
Ogre::ushort TileManager::getMapWater(unsigned int x, unsigned int z)
{
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].heightWater;
}

//================================================================================================
// Returns the gfx of a tile-vertex.
//================================================================================================
uchar TileManager::getMapLayer0(unsigned int x, unsigned int z)
{
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].gfxLayer0;
}

//================================================================================================
// Returns the gfx of a tile-vertex.
//================================================================================================
uchar TileManager::getMapLayer1(unsigned int x, unsigned int z)
{
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].gfxLayer1;
}

//================================================================================================
// Returns the gfx of a tile-vertex.
//================================================================================================
Real TileManager::getMapShadow(unsigned int x, unsigned int z)
{
    return Real(mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].shadow) / 255.0f;
}

//================================================================================================
// Returns the gfx of a tile-vertex.
//================================================================================================
bool TileManager::getMapSpotLight(unsigned int x, unsigned int z)
{
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].spotLight;
}

//================================================================================================
// Scroll the map.
//================================================================================================
void TileManager::scrollMap(int dx, int dz)
{
    mMapSPosX= (mMapSPosX-dx*2)&mMapMaskX;
    mMapSPosZ= (mMapSPosZ-dz*2)&mMapMaskZ;
    mMapchunk.update();
}

//================================================================================================
// Change Tile textures.
//================================================================================================
void TileManager::setMapset(int landGroup, int waterGroup)
{
    mMapchunk.setMaterial(landGroup,  mTextureSize);
}

//================================================================================================
// A tile was clicked.
//================================================================================================
void TileManager::tileClick(float mouseX, float mouseY)
{
    Ray mouseRay = mSceneManager->getCamera("PlayerCam")->getCameraToViewportRay(mouseX, mouseY);
    mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setQueryMask(mQueryMaskLand);
    // Perform the scene query.
    RaySceneQueryResult &result = mRaySceneQuery->execute();
    if (result.size() >1)
    {
        Logger::log().error() << "BUG in tileClick(...): RaySceneQuery returned more than 1 result.";
        Logger::log().error() << "(Perhaps you created Entities without setting a setQueryFlags() on them)";
        return;
    }
    for (int x = 0; x < CHUNK_SIZE_X*2; ++x)
    {
        for (int z = 0; z < CHUNK_SIZE_Z*2; ++z)
        {
            if ((x+z)&1)
            {
                // +-+
                // |/
                // +
                mVertex[0].x = (x+0.0f)*TILE_RENDER_SIZE;
                mVertex[0].y = getMapHeight(x, z);
                mVertex[0].z = (z+0.0f)*TILE_RENDER_SIZE;
                mVertex[1].x = (x+0.0f)*TILE_RENDER_SIZE;
                mVertex[1].y = getMapHeight(x, z-1);
                mVertex[1].z = (z+1.0f)*TILE_RENDER_SIZE;
                mVertex[2].x = (x+1.0f)*TILE_RENDER_SIZE;
                mVertex[2].y = getMapHeight(x-1, z);
                mVertex[2].z = (z+0.0f)*TILE_RENDER_SIZE;
                if (vertexPick(&mouseRay, x, z, 0)) return; // We got a hit.
                //   +
                //  /|
                // +-+
                mVertex[0].x = (x+1.0f)*TILE_RENDER_SIZE;
                mVertex[0].y = getMapHeight(x+1, z);
                mVertex[0].z = (z+0.0f)*TILE_RENDER_SIZE;
                mVertex[1].x = (x+0.0f)*TILE_RENDER_SIZE;
                mVertex[1].y = getMapHeight(x, z-1);
                mVertex[1].z = (z+1.0f)*TILE_RENDER_SIZE;
                mVertex[2].x = (x+1.0f)*TILE_RENDER_SIZE;
                mVertex[2].y = getMapHeight(x+1, z-1);
                mVertex[2].z = (z+1.0f)*TILE_RENDER_SIZE;
                if (vertexPick(&mouseRay, x, z, 1)) return; // We got a hit.
            }
            else
            {
                //
                //   +
                //   |\.
                //   +-+
                mVertex[0].x = (x+0.0f)*TILE_RENDER_SIZE;
                mVertex[0].y = getMapHeight(x, z);
                mVertex[0].z = (z+0.0f)*TILE_RENDER_SIZE;
                mVertex[1].x = (x+0.0f)*TILE_RENDER_SIZE;
                mVertex[1].y = getMapHeight(x, z-1);
                mVertex[1].z = (z+1.0f)*TILE_RENDER_SIZE;
                mVertex[2].x = (x+1.0f)*TILE_RENDER_SIZE;
                mVertex[2].y = getMapHeight(x+1, z-1);
                mVertex[2].z = (z+1.0f)*TILE_RENDER_SIZE;
                if (vertexPick(&mouseRay, x, z, 2)) return; // We got a hit.
                // +-+
                //  \|
                //   +
                mVertex[0].x = (x+1.0f)*TILE_RENDER_SIZE;
                mVertex[0].y = getMapHeight(x+1, z-1);
                mVertex[0].z = (z+1.0f)*TILE_RENDER_SIZE;
                mVertex[1].x = (x+1.0f)*TILE_RENDER_SIZE;
                mVertex[1].y = getMapHeight(x+1, z);
                mVertex[1].z = (z+0.0f)*TILE_RENDER_SIZE;
                mVertex[2].x = (x+0.0f)*TILE_RENDER_SIZE;
                mVertex[2].y = getMapHeight(x, z);
                mVertex[2].z = (z+0.0f)*TILE_RENDER_SIZE;
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
    Test = Math::intersects(*mouseRay, mVertex[0], mVertex[1], mVertex[2]);
    if (!Test.first) return false;  // This tile piece was not clicked.
    // Test for Vertex 0
    Test = Math::intersects(*mouseRay, mVertex[0], (mVertex[0] + mVertex[2])/2, (mVertex[0] + mVertex[1])/2);
    if (Test.first)
    {
        if      (pos == 0) highlightVertex(x  , z  );
        else if (pos == 1) highlightVertex(x+1, z  );
        else if (pos == 2) highlightVertex(x  , z  );
        else if (pos == 3) highlightVertex(x+1, z+1);
        return true;
    }
    // Test for Vertex 1
    Test = Math::intersects(*mouseRay, mVertex[2], (mVertex[0] + mVertex[2])/2, (mVertex[1] + mVertex[2])/2);
    if (Test.first)
    {
        if      (pos == 0) highlightVertex(x+1, z  );
        else if (pos == 1) highlightVertex(x+1, z+1);
        else if (pos == 2) highlightVertex(x+1, z+1);
        else if (pos == 3) highlightVertex(x  , z  );
        return true;
    }
    // Test for Vertex 2
    Test = Math::intersects(*mouseRay, mVertex[1], (mVertex[0] + mVertex[1])/2, (mVertex[1] + mVertex[2])/2);
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
void TileManager::updateHeighlightVertexPos(int deltaX, int deltaZ)
{
    mSelectedVertexX+= deltaX; if (mSelectedVertexX > CHUNK_SIZE_X) mSelectedVertexX = 0;
    mSelectedVertexZ+= deltaZ; if (mSelectedVertexZ > CHUNK_SIZE_Z) mSelectedVertexZ = 0;
    highlightVertex(mSelectedVertexX, mSelectedVertexZ);
}

//================================================================================================
// .
//================================================================================================
void TileManager::highlightVertex(int x, int z)
{
    static SceneNode *tcNode = 0;
    if (!tcNode)
    {
        int size = TILE_RENDER_SIZE/6;
        ManualObject *mob = static_cast<ManualObject*>(mSceneManager->createMovableObject("VertexHighlight", ManualObjectFactory::FACTORY_TYPE_NAME));
        mob->begin("Terrain/VertexHighlight");
        mob->position(-1.0f*size, 1.5f*size,-0.80f*size);
        mob->position( 0.0f*size, 0.0f*size, 0.00f*size);
        mob->position( 1.0f*size, 1.5f*size,-0.80f*size);
        mob->position( 0.0f*size, 1.5f*size, 0.94f*size);
        mob->triangle( 0,  1,  2);
        mob->triangle( 2,  1,  3);
        mob->triangle( 3,  1,  0);
        mob->triangle( 0,  2,  3);
        mob->position(-0.5f*size, 1.5f*size,-0.30f*size);
        mob->position( 0.5f*size, 1.5f*size,-0.30f*size);
        mob->position( 0.0f*size, 1.5f*size, 0.20f*size);
        mob->position(-0.5f*size, 3.0f*size,-0.30f*size);
        mob->position( 0.5f*size, 3.0f*size,-0.30f*size);
        mob->position( 0.0f*size, 3.0f*size, 0.20f*size);
        mob->triangle( 7,  8,  9);
        mob->quad( 4,  7,  8,  5);
        mob->quad( 8,  5,  6,  9);
        mob->quad( 9,  6,  4,  7);
        mob->setQueryFlags(0);
        //mob->setRenderQueueGroup(RENDER_QUEUE_6); // See OgreRenderQueue.h
        mob->end();
        tcNode = mSceneManager->getRootSceneNode()->createChildSceneNode("Node/VertexHighlight");
        tcNode->attachObject(mob);
    }
    mSelectedVertexX = x;
    mSelectedVertexZ = z;
    x*= TILE_RENDER_SIZE;
    z*= TILE_RENDER_SIZE;
    tcNode->setPosition((Real)x, getTileHeight(x, z), (Real)z);
}

//================================================================================================
// .
//================================================================================================
void TileManager::updateTileHeight(int deltaHeight)
{
    mMap[((mMapSPosZ + mSelectedVertexZ)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + mSelectedVertexX)&mMapMaskX)].heightLand+= deltaHeight;
    mMapchunk.update();
    highlightVertex(mSelectedVertexX, mSelectedVertexZ);
}

//================================================================================================
// .
//================================================================================================
void TileManager::updateTileGfx(int deltaGfxNr)
{
    mMap[((mMapSPosZ + mSelectedVertexZ)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + mSelectedVertexX)&mMapMaskX)].gfxLayer0+= deltaGfxNr;
    mEditorActSelectedGfx = mMap[((mMapSPosZ + mSelectedVertexZ)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + mSelectedVertexX)&mMapMaskX)].gfxLayer0;
    mMapchunk.update();
    highlightVertex(mSelectedVertexX, mSelectedVertexZ);
}

//================================================================================================
// .
//================================================================================================
void TileManager::setTileGfx()
{
    mMap[((mMapSPosZ + mSelectedVertexZ)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + mSelectedVertexX)&mMapMaskX)].gfxLayer0 = mEditorActSelectedGfx;
    mMapchunk.update();
}

//================================================================================================
// Helper function for getTileHeight(...);
//================================================================================================
int TileManager::calcHeight(int vert0, int vert1, int vert2, int posX, int posZ)
{
    if (posZ == TILE_RENDER_SIZE) return vert1;
    int h1 = ((vert1 - vert0) * posZ) / TILE_RENDER_SIZE + vert0;
    int h2 = ((vert1 - vert2) * posZ) / TILE_RENDER_SIZE + vert2;
    int maxX = TILE_RENDER_SIZE - posZ;
    return ((h2 - h1) * posX) / maxX + h1;
}

//================================================================================================
// Return the exact height of a position within a tile.
//================================================================================================
short TileManager::getTileHeight(int posX, int posZ)
{
    int TileX = posX / TILE_RENDER_SIZE; // Get the Tile position within the map.
    int TileZ = posZ / TILE_RENDER_SIZE; // Get the Tile position within the map.
    posX&= (TILE_RENDER_SIZE-1);         // Lower part is the position within the tile.
    posZ&= (TILE_RENDER_SIZE-1);         // Lower part is the position within the tile.
    //   +-+v2
    //   |/|
    // v1+-+
    if ((TileX+TileZ)&1)
    {
        int v1 = getMapHeight(TileX  , TileZ+1); // BL
        int v2 = getMapHeight(TileX+1, TileZ  ); // TR
        if (TILE_RENDER_SIZE - posX > posZ)
            return calcHeight(getMapHeight(TileX, TileZ), v1, v2, posX, posZ); // TL
        return calcHeight(getMapHeight(TileX+1, TileZ+1), v1, v2, TILE_RENDER_SIZE-posZ, TILE_RENDER_SIZE-posX); // BR
    }
    // v1+-+
    //   |\|
    //   +-+v2
    int v1 = getMapHeight(TileX  , TileZ  ); // TL
    int v2 = getMapHeight(TileX+1, TileZ+1); // BR
    if (posX < posZ)
        return calcHeight(getMapHeight(TileX  , TileZ+1), v1, v2, posX, TILE_RENDER_SIZE-posZ); // BL
    return calcHeight(getMapHeight(TileX+1, TileZ  ), v1, v2, posZ, TILE_RENDER_SIZE-posX); //TR
}

//================================================================================================
// Collect all tiles and masks into a single RGB-image.
//================================================================================================
void TileManager::createAtlasTexture(int textureSize, unsigned int startGroup)
{
    int stopGroup = startGroup+1;
    if (startGroup >= (unsigned int) MAX_MAP_SETS)
    {
        startGroup = 0;
        stopGroup  = MAX_MAP_SETS;
    }
    PixelFormat pf = PF_A8R8G8B8;
    uchar *dstBuf = OGRE_ALLOC_T(uchar, textureSize * textureSize * PixelUtil::getNumElemBytes(pf), MEMCATEGORY_GENERAL);
    Image dstImage;
    for (int nr = startGroup; nr < stopGroup; ++nr)
    {
        if (!copyTileToAtlas(dstBuf)) break;
        copyFlowToAtlas(dstBuf);
        copySpotToAtlas(dstBuf);
        copyMaskToAtlas(dstBuf);
        // Save the Atlastexture.
        dstImage.loadDynamicImage(dstBuf, textureSize, textureSize, 1, pf, true);
        String dstFilename = mPathGfxTiles + ATLAS_PREFIX + "_" + StringConverter::toString(nr,2,'0') + "_";
        for (unsigned short s = textureSize; s >= textureSize/(1<<(SUM_ATLAS_RESOLUTIONS-1)); s/=2)
        {
            dstImage.save(dstFilename + StringConverter::toString(s, 4, '0') + ".png");
            dstImage.resize(s/2, s/2, Image::FILTER_BILINEAR);
        }
    }
    //OGRE_FREE(data, MEMCATEGORY_GENERAL); // Will be done by Ogre because autoDelete was set.
}

//================================================================================================
// Copy a tile into the color part of the atlastexture.
//================================================================================================
bool TileManager::copyTileToAtlas(uchar *dstBuf)
{
    static int nr = -1;
    const unsigned int OFFSET = BORDER_SIZE*2+TILE_SIZE;
    int sumImages= 0;
    unsigned int maxX;
    Image srcImage;
    String srcFilename;
    ++nr;
    for (int y = 0; y < ATLAS_LAND_ROWS; ++y)
    {
        for (unsigned int x = 0; x < ATLAS_LAND_COLS; ++x)
        {
            srcFilename = "terrain_" + StringConverter::toString(nr, 2, '0') + "_" + StringConverter::toString(sumImages++, 2, '0') + ".png";
            if (!loadImage(srcImage, srcFilename, false))
            {
                if (sumImages==1) return false; // No Tiles found for this group.
                return true;
            }
            if ((srcImage.getWidth() != TILE_SIZE) || (srcImage.getHeight() != TILE_SIZE))
            {
                Logger::log().error() << "Gfx " << srcFilename << " has the wrong size! Only " << (int)TILE_SIZE << "^2 is supported.";
                return true;
            }
            int srcAlpha = (srcImage.getFormat()==PF_A8R8G8B8)?1:0; // Ignore alpha.
            uchar *src = srcImage.getData();
            uchar *dst = dstBuf + (y*OFFSET*MAX_TEXTURE_SIZE + x*OFFSET)*RGB_A;
            uchar *dst1= x?dst-BORDER_SIZE*RGB_A:dst;
            for (unsigned int ty = 0; ty < TILE_SIZE; ++ty)
            {
                // Tile
                for (unsigned int tx = 0; tx < TILE_SIZE; ++tx)
                {
                    *dst++= *src++; // R
                    *dst++= *src++; // G
                    *dst++= *src++; // B
                    *dst++= 255;
                    src+= srcAlpha; // A
                }
                // Right and left mask borders.
                for (unsigned int tx = 0; tx < BORDER_SIZE; ++tx)
                {
                    for (unsigned int color = 0; color < RGB_A; ++color)
                    {
                        *dst = *(dst-TILE_SIZE*RGB_A); // Right mask border
                        if (x)
                            *(dst-(TILE_SIZE+2*tx+1)*RGB_A) = *(dst-(2*tx+1)*RGB_A); // Left mask border
                        ++dst;
                    }
                }
                dst+=(MAX_TEXTURE_SIZE-TILE_SIZE-BORDER_SIZE)*RGB_A;
            }
            // Top and bottom mask borders.
            for (unsigned int ty = 0; ty < BORDER_SIZE; ++ty)
            {
                maxX = x?OFFSET*RGB_A:(OFFSET-BORDER_SIZE)*RGB_A;
                for (unsigned int tx = 0; tx < maxX; ++tx)
                    dst1[TILE_SIZE*MAX_TEXTURE_SIZE*RGB_A+tx] = dst1[tx]; // Bottom mask border
                dst1+= MAX_TEXTURE_SIZE*RGB_A;
            }
            if (y)
            {
                dst1-= 2*BORDER_SIZE*MAX_TEXTURE_SIZE*RGB_A;
                for (unsigned int ty = 0; ty < BORDER_SIZE; ++ty)
                {
                    maxX = x?OFFSET*RGB_A:(OFFSET-BORDER_SIZE)*RGB_A;
                    for (unsigned int tx = 0; tx < maxX; ++tx)
                        dst1[tx] = dst1[(TILE_SIZE*MAX_TEXTURE_SIZE)*RGB_A+tx]; // Top mask border
                    dst1+= MAX_TEXTURE_SIZE*RGB_A;
                }
            }
        }
    }
    return true;
}

//================================================================================================
// Copy a terrain-mask into the atlastexture.
//================================================================================================
void TileManager::copyFlowToAtlas(uchar *dstBuf)
{
    static int nr = -1;
    Image srcImage;
    uchar *src, *src2, *dst;
    dstBuf+= ATLAS_LAND_ROWS*(TILE_SIZE+2*BORDER_SIZE) * MAX_TEXTURE_SIZE * RGB_A;
    ++nr;
    for (int sumFlowTiles = 0; sumFlowTiles < ATLAS_LAND_COLS/2; ++sumFlowTiles)
    {
        String srcFilename = "terrain_" + StringConverter::toString(nr, 2, '0') + "_F" + StringConverter::toString(sumFlowTiles) + ".png";
        if (!loadImage(srcImage, srcFilename, false))
        {
            // FlowTile was not found, so we use the default one.
            if (!loadImage(srcImage, "terrain_00_F0.png", false))
            {
                Logger::log().error() << "The default flow-tile (terrain_00_F0.png) was not found.";
                return;
            }
        }
        if ((srcImage.getWidth() != TILE_SIZE) || (srcImage.getHeight() != TILE_SIZE))
        {
            Logger::log().error() << "Gfx " << srcFilename << " has the wrong size! Only " << (int)TILE_SIZE << "^2 is supported.";
            return;
        }
        if (srcImage.getFormat()!=PF_R8G8B8)
        {
            Logger::log().error() << "Gfx " << srcFilename << " has the wrong pixelformat. Only RGB is supported for tiles.";
            return;
        }
        // Left horizontal Border
        src = srcImage.getData() + (TILE_SIZE-BORDER_SIZE)*TILE_SIZE * RGB;
        src2= srcImage.getData() + (TILE_SIZE/2)*TILE_SIZE * RGB;
        dst = dstBuf - BORDER_SIZE*MAX_TEXTURE_SIZE*RGB_A;
        for (unsigned int y = 0; y < BORDER_SIZE; ++y)
        {
            for (unsigned int x = 0; x < TILE_SIZE; ++x)
            {
                dst[x*RGB_A+0] = src[x*RGB+0];
                dst[x*RGB_A+1] = src[x*RGB+1];
                dst[x*RGB_A+2] = src[x*RGB+2];
                dst[x*RGB_A+3] = 255;
                dst[((TILE_SIZE/2+BORDER_SIZE)*MAX_TEXTURE_SIZE + x)*RGB_A+0] = src2[x*RGB+0];
                dst[((TILE_SIZE/2+BORDER_SIZE)*MAX_TEXTURE_SIZE + x)*RGB_A+1] = src2[x*RGB+1];
                dst[((TILE_SIZE/2+BORDER_SIZE)*MAX_TEXTURE_SIZE + x)*RGB_A+2] = src2[x*RGB+2];
                dst[((TILE_SIZE/2+BORDER_SIZE)*MAX_TEXTURE_SIZE + x)*RGB_A+3] = 255;
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= TILE_SIZE*RGB;
            src2+= TILE_SIZE*RGB;
        }
        // Right horizontal Border
        src = srcImage.getData() + (TILE_SIZE/2-BORDER_SIZE)*TILE_SIZE * RGB;
        src2= srcImage.getData();
        dst = dstBuf - BORDER_SIZE*MAX_TEXTURE_SIZE*RGB_A + (256+BORDER_SIZE*2) * RGB_A;
        for (unsigned int y = 0; y < BORDER_SIZE; ++y)
        {
            for (unsigned int x = 0; x < TILE_SIZE; ++x)
            {
                dst[x*RGB_A+0] = src[x*RGB+0];
                dst[x*RGB_A+1] = src[x*RGB+1];
                dst[x*RGB_A+2] = src[x*RGB+2];
                dst[x*RGB_A+3] = 255;
                dst[((TILE_SIZE/2+BORDER_SIZE)*MAX_TEXTURE_SIZE + x)*RGB_A+0] = src2[x*RGB+0];
                dst[((TILE_SIZE/2+BORDER_SIZE)*MAX_TEXTURE_SIZE + x)*RGB_A+1] = src2[x*RGB+1];
                dst[((TILE_SIZE/2+BORDER_SIZE)*MAX_TEXTURE_SIZE + x)*RGB_A+2] = src2[x*RGB+2];
                dst[((TILE_SIZE/2+BORDER_SIZE)*MAX_TEXTURE_SIZE + x)*RGB_A+3] = 255;
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= TILE_SIZE*RGB;
            src2+= TILE_SIZE*RGB;
        }
        // Upper Tile half.
        src = srcImage.getData();
        dst = dstBuf;
        for (unsigned int y = 0; y < TILE_SIZE/2; ++y)
        {
            for (unsigned int x = 0; x < TILE_SIZE; ++x)
            {
                dst[x*RGB_A+0] = src[x*RGB+0];
                dst[x*RGB_A+1] = src[x*RGB+1];
                dst[x*RGB_A+2] = src[x*RGB+2];
                dst[x*RGB_A+3] = 255;
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= TILE_SIZE*RGB;
        }
        // Lower Tile half.
        dst = dstBuf + (TILE_SIZE+BORDER_SIZE*2) * RGB_A;
        for (unsigned int y = 0; y < TILE_SIZE/2; ++y)
        {
            for (unsigned int x = 0; x < TILE_SIZE; ++x)
            {
                dst[x*RGB_A+0] = src[x*RGB+0];
                dst[x*RGB_A+1] = src[x*RGB+1];
                dst[x*RGB_A+2] = src[x*RGB+2];
                dst[x*RGB_A+3] = 255;
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= TILE_SIZE*RGB;
        }
        // Vertical Borders
        src = dstBuf - (BORDER_SIZE*MAX_TEXTURE_SIZE) * RGB_A;
        dst = dstBuf - (BORDER_SIZE*MAX_TEXTURE_SIZE + BORDER_SIZE) * RGB_A;
        const unsigned int OFFSET = (BORDER_SIZE*2+TILE_SIZE)*RGB_A;
        for (unsigned int y = 0; y < TILE_SIZE/2+2*BORDER_SIZE; ++y)
        {
            for (unsigned int x = 0; x < BORDER_SIZE; ++x)
            {
                if (sumFlowTiles)
                {
                    dst[x*RGB_A+0] = dst[(TILE_SIZE+x)*RGB_A+0];
                    dst[x*RGB_A+1] = dst[(TILE_SIZE+x)*RGB_A+1];
                    dst[x*RGB_A+2] = dst[(TILE_SIZE+x)*RGB_A+2];
                    dst[x*RGB_A+3] = dst[(TILE_SIZE+x)*RGB_A+3];
                }
                dst[x*RGB_A+OFFSET+0] = dst[(TILE_SIZE+x)*RGB_A+OFFSET+0];
                dst[x*RGB_A+OFFSET+1] = dst[(TILE_SIZE+x)*RGB_A+OFFSET+1];
                dst[x*RGB_A+OFFSET+2] = dst[(TILE_SIZE+x)*RGB_A+OFFSET+2];
                dst[x*RGB_A+OFFSET+3] = dst[(TILE_SIZE+x)*RGB_A+OFFSET+3];

                dst[(TILE_SIZE+BORDER_SIZE+ x)*RGB_A+0] = src[x*RGB_A+0];
                dst[(TILE_SIZE+BORDER_SIZE+ x)*RGB_A+1] = src[x*RGB_A+1];
                dst[(TILE_SIZE+BORDER_SIZE+ x)*RGB_A+2] = src[x*RGB_A+2];
                dst[(TILE_SIZE+BORDER_SIZE+ x)*RGB_A+3] = 255;
                dst[(TILE_SIZE+BORDER_SIZE+ x)*RGB_A+OFFSET+0] = src[x*RGB_A+OFFSET+0];
                dst[(TILE_SIZE+BORDER_SIZE+ x)*RGB_A+OFFSET+1] = src[x*RGB_A+OFFSET+1];
                dst[(TILE_SIZE+BORDER_SIZE+ x)*RGB_A+OFFSET+2] = src[x*RGB_A+OFFSET+2];
                dst[(TILE_SIZE+BORDER_SIZE+ x)*RGB_A+OFFSET+3] = 255;
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= MAX_TEXTURE_SIZE*RGB_A;
        }
        dstBuf+= (2*TILE_SIZE+4*BORDER_SIZE) * RGB_A;
    }
}

//================================================================================================
// Copy a terrain-mask into the atlastexture.
//================================================================================================
void TileManager::copyMaskToAtlas(uchar *dstBuf)
{
    static int nr = -1;
    const unsigned int OFFSET = BORDER_SIZE*2+TILE_SIZE/2;
    Image srcImage;
    String srcFilename;
    uchar *src, *dst;
    dstBuf+= 6*(BORDER_SIZE*2+TILE_SIZE)* RGB_A;
    ++nr;
    for (int sumMask = 0; sumMask < ATLAS_LAND_ROWS; ++sumMask)
    {
        String srcFilename = "terrain_" + StringConverter::toString(nr, 2, '0') + "_M" + StringConverter::toString(sumMask) + ".png";
        if (!loadImage(srcImage, srcFilename, false))
        {
            // Mask was not found, so we use the default mask.
            if (!loadImage(srcImage, "terrain_00_M0.png", false))
            {
                Logger::log().error() << "The default tile-mask (terrain_00_M0.png) was not found.";
                return;
            }
        }
        if ((srcImage.getWidth() != TILE_SIZE) || (srcImage.getHeight() != TILE_SIZE))
        {
            Logger::log().error() << "Gfx " << srcFilename << " has the wrong size! Only " << (int)TILE_SIZE << "^2 is supported.";
            return;
        }
        if (srcImage.getFormat()!=PF_R8G8B8)
        {
            Logger::log().error() << "Gfx " << srcFilename << " has the wrong pixelformat. Only RGB is supported for masks.";
            return;
        }
        // Erase the help lines from the mask template.
        src = srcImage.getData();
        for (unsigned int i = 0; i < TILE_SIZE*TILE_SIZE; ++i)
        {
            if (src[0] != src[1])
            {
                src[0] = 0x00; // R
                src[1] = 0x00; // G
                src[2] = 0x00; // B
            }
            src+=RGB;
        }
        // Copy mask 0 to the atlas-texture.
        src = srcImage.getData();
        dst = dstBuf;
        for (unsigned int y = 0; y < TILE_SIZE/2; ++y)
        {
            for (unsigned int x = 0; x < TILE_SIZE/2; ++x)
            {
                dst[(                          2*OFFSET + x)*RGB_A+2] = src[x*RGB]; // Mask 2 - red.
                dst[(OFFSET*MAX_TEXTURE_SIZE + 0*OFFSET + x)*RGB_A+1] = src[x*RGB]; // Mask 3 - green.
                dst[(OFFSET*MAX_TEXTURE_SIZE + 1*OFFSET + x)*RGB_A+2] = src[x*RGB]; // Mask 4 - red.
                dst[(OFFSET*MAX_TEXTURE_SIZE + 2*OFFSET + x)*RGB_A+1] = src[x*RGB]; // Mask 5 - green.
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= TILE_SIZE*RGB;
        }
        // Copy mask 1 to the atlas-texture.
        src = srcImage.getData();
        dst = dstBuf;
        for (unsigned int y = 0; y < TILE_SIZE/4; ++y)
        {
            for (unsigned int x = 0; x < TILE_SIZE/4; ++x)
            {
                dst[(x                                          )*RGB_A+2] = src[(64*TILE_SIZE + 192 + x)*RGB]; // R (1 of 4)
                dst[(x+TILE_SIZE/4                              )*RGB_A+2] = src[(64*TILE_SIZE + 128 + x)*RGB]; // R (2 of 4)
                dst[(x             +TILE_SIZE/4*MAX_TEXTURE_SIZE)*RGB_A+2] = src[(               192 + x)*RGB]; // R (1 of 4)
                dst[(x+TILE_SIZE/4 +TILE_SIZE/4*MAX_TEXTURE_SIZE)*RGB_A+2] = src[(               128 + x)*RGB]; // R (2 of 4)
                dst[(1*OFFSET +x                                          )*RGB_A+1] = src[(64*TILE_SIZE + 192 + x)*RGB]; // G (1 of 4)
                dst[(1*OFFSET +x+TILE_SIZE/4                              )*RGB_A+1] = src[(64*TILE_SIZE + 128 + x)*RGB]; // G (2 of 4)
                dst[(1*OFFSET +x             +TILE_SIZE/4*MAX_TEXTURE_SIZE)*RGB_A+1] = src[(               192 + x)*RGB]; // G (1 of 4)
                dst[(1*OFFSET +x+TILE_SIZE/4 +TILE_SIZE/4*MAX_TEXTURE_SIZE)*RGB_A+1] = src[(               128 + x)*RGB]; // G (2 of 4)

                dst[(OFFSET*MAX_TEXTURE_SIZE+ 1*OFFSET +x                                          )*RGB_A+1] = src[(64*TILE_SIZE + 192 + x)*RGB]; // G (1 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE+ 1*OFFSET +x+TILE_SIZE/4                              )*RGB_A+1] = src[(64*TILE_SIZE + 128 + x)*RGB]; // G (2 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE+ 1*OFFSET +x             +TILE_SIZE/4*MAX_TEXTURE_SIZE)*RGB_A+1] = src[(               192 + x)*RGB]; // G (1 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE+ 1*OFFSET +x+TILE_SIZE/4 +TILE_SIZE/4*MAX_TEXTURE_SIZE)*RGB_A+1] = src[(               128 + x)*RGB]; // G (2 of 4)

                dst[(OFFSET*MAX_TEXTURE_SIZE+ 2*OFFSET +x                                          )*RGB_A+2] = src[(64*TILE_SIZE + 192 + x)*RGB]; // R (1 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE+ 2*OFFSET +x+TILE_SIZE/4                              )*RGB_A+2] = src[(64*TILE_SIZE + 128 + x)*RGB]; // R (2 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE+ 2*OFFSET +x             +TILE_SIZE/4*MAX_TEXTURE_SIZE)*RGB_A+2] = src[(               192 + x)*RGB]; // R (1 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE+ 2*OFFSET +x+TILE_SIZE/4 +TILE_SIZE/4*MAX_TEXTURE_SIZE)*RGB_A+2] = src[(               128 + x)*RGB]; // R (2 of 4)
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= TILE_SIZE*RGB;
        }
        // Copy mask 2 (horizontal) to the atlas-texture.
        src = srcImage.getData();
        dst = dstBuf;
        for (unsigned int y = 0; y < TILE_SIZE/4; ++y)
        {
            for (unsigned int x = 0; x < TILE_SIZE/2; ++x)
            {
                dst[(                                                       x)*RGB_A+1] = src[(192*TILE_SIZE + x)*RGB]; // G (1 of 4)
                dst[(                         TILE_SIZE/4*MAX_TEXTURE_SIZE +x)*RGB_A+1] = src[(128*TILE_SIZE + x)*RGB]; // G (1 of 4)
                dst[(OFFSET                                                +x)*RGB_A+2] = src[(192*TILE_SIZE + x)*RGB]; // R (1 of 4)
                dst[(OFFSET                  +TILE_SIZE/4*MAX_TEXTURE_SIZE +x)*RGB_A+2] = src[(128*TILE_SIZE + x)*RGB]; // R (1 of 4)
                dst[(OFFSET*2                                              +x)*RGB_A+1] = src[(192*TILE_SIZE + x)*RGB]; // G (1 of 4)
                dst[(OFFSET*2                +TILE_SIZE/4*MAX_TEXTURE_SIZE +x)*RGB_A+1] = src[(128*TILE_SIZE + x)*RGB]; // G (1 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE                               +x)*RGB_A+2] = src[(192*TILE_SIZE + x)*RGB]; // R (1 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE +TILE_SIZE/4*MAX_TEXTURE_SIZE +x)*RGB_A+2] = src[(128*TILE_SIZE + x)*RGB]; // R (1 of 4)
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= TILE_SIZE*RGB;
        }
        // Copy mask 2 (vertical) to the atlas-texture.
        src = srcImage.getData();
        dst = dstBuf;
        for (unsigned int y = 0; y < TILE_SIZE/2; ++y)
        {
            for (unsigned int x = 0; x < TILE_SIZE/4; ++x)
            {
                dst[(                                      x)*RGB_A+1]+= src[(128*TILE_SIZE +192+ x)*RGB]; // G (1 of 4)
                dst[(                         TILE_SIZE/4 +x)*RGB_A+1]+= src[(128*TILE_SIZE +128+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET                               +x)*RGB_A+2]+= src[(128*TILE_SIZE +192+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET                  +TILE_SIZE/4 +x)*RGB_A+2]+= src[(128*TILE_SIZE +128+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET*2                             +x)*RGB_A+1]+= src[(128*TILE_SIZE +192+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET*2                +TILE_SIZE/4 +x)*RGB_A+1]+= src[(128*TILE_SIZE +128+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE              +x)*RGB_A+2]+= src[(128*TILE_SIZE +192+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET*MAX_TEXTURE_SIZE +TILE_SIZE/4 +x)*RGB_A+2]+= src[(128*TILE_SIZE +128+ x)*RGB]; // G (1 of 4)
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= TILE_SIZE*RGB;
        }
        // Draw the grid.
        int gridColor;
        src = srcImage.getData();
        dst = dstBuf;
        for (int i = 0; i < 2; ++i)
        {
            for (unsigned int y = 0; y < TILE_SIZE/2; ++y)
            {
                for (unsigned int x = 0; x < TILE_SIZE/2; ++x)
                {
                    gridColor = (!x || !y || x == TILE_SIZE/4 || y == TILE_SIZE/4 || x == y || x == TILE_SIZE/2-y)?0x00:0xff;
                    dst[(0*OFFSET + x)*RGB_A+0] = gridColor; // B
                    dst[(1*OFFSET + x)*RGB_A+0] = gridColor; // B
                    dst[(2*OFFSET + x)*RGB_A+0] = gridColor; // B
                }
                dst+= MAX_TEXTURE_SIZE*RGB_A;
            }
            dst+= BORDER_SIZE*2*MAX_TEXTURE_SIZE*RGB_A;
        }
        // Create mask borders
        // Vertical
        for (int i =0; i < 3; ++i)
        {
            dst = dstBuf + i*(TILE_SIZE/2+BORDER_SIZE*2)* RGB_A;
            for (unsigned int y = 0; y < BORDER_SIZE*2+TILE_SIZE; ++y)
            {
                for (unsigned int x = 1; x <= BORDER_SIZE; ++x)
                {
                    // Left border
                    *(dst-x*RGB_A+0) = dst[0];
                    *(dst-x*RGB_A+1) = dst[1];
                    *(dst-x*RGB_A+2) = dst[2];
                    *(dst-x*RGB_A+3) = dst[3];
                    // Right border
                    if (i < 2)
                    {
                        *(dst+(TILE_SIZE/2+BORDER_SIZE-x)*RGB_A+0) = dst[(TILE_SIZE/2-1)*RGB_A+0];
                        *(dst+(TILE_SIZE/2+BORDER_SIZE-x)*RGB_A+1) = dst[(TILE_SIZE/2-1)*RGB_A+1];
                        *(dst+(TILE_SIZE/2+BORDER_SIZE-x)*RGB_A+2) = dst[(TILE_SIZE/2-1)*RGB_A+2];
                        *(dst+(TILE_SIZE/2+BORDER_SIZE-x)*RGB_A+3) = dst[(TILE_SIZE/2-1)*RGB_A+3];
                    }
                }
                dst+= MAX_TEXTURE_SIZE*RGB_A;
            }
        }
        // Horizontal
        dst = dstBuf + (TILE_SIZE/2*MAX_TEXTURE_SIZE-BORDER_SIZE)* RGB_A;
        for (int i =0; i < 2; ++i)
        {
            for (unsigned int y = 0; y < BORDER_SIZE; ++y)
            {
                for (unsigned int x = 0; x < BORDER_SIZE*5+TILE_SIZE/2*3; ++x)
                {
                    // Bottom border
                    dst[(y*MAX_TEXTURE_SIZE+x)*RGB_A+0]= *(dst-(MAX_TEXTURE_SIZE-x)*RGB_A+0);
                    dst[(y*MAX_TEXTURE_SIZE+x)*RGB_A+1]= *(dst-(MAX_TEXTURE_SIZE-x)*RGB_A+1);
                    dst[(y*MAX_TEXTURE_SIZE+x)*RGB_A+2]= *(dst-(MAX_TEXTURE_SIZE-x)*RGB_A+2);
                    dst[(y*MAX_TEXTURE_SIZE+x)*RGB_A+3]= *(dst-(MAX_TEXTURE_SIZE-x)*RGB_A+3);
                    // Top border
                    if (i+sumMask)
                    {
                        *(dst-((y+TILE_SIZE/2+1)*MAX_TEXTURE_SIZE-x)*RGB_A+0) = *(dst-((TILE_SIZE/2)*MAX_TEXTURE_SIZE-x)*RGB_A+0);
                        *(dst-((y+TILE_SIZE/2+1)*MAX_TEXTURE_SIZE-x)*RGB_A+1) = *(dst-((TILE_SIZE/2)*MAX_TEXTURE_SIZE-x)*RGB_A+1);
                        *(dst-((y+TILE_SIZE/2+1)*MAX_TEXTURE_SIZE-x)*RGB_A+2) = *(dst-((TILE_SIZE/2)*MAX_TEXTURE_SIZE-x)*RGB_A+2);
                        *(dst-((y+TILE_SIZE/2+1)*MAX_TEXTURE_SIZE-x)*RGB_A+3) = *(dst-((TILE_SIZE/2)*MAX_TEXTURE_SIZE-x)*RGB_A+3);
                    }
                }
            }
            dst+= (TILE_SIZE/2 + BORDER_SIZE*2)*MAX_TEXTURE_SIZE * RGB_A;
        }
        dstBuf+= (TILE_SIZE + BORDER_SIZE*4)*MAX_TEXTURE_SIZE * RGB_A;
    }
}

//================================================================================================
// Copy a spotlight-mask into the atlastexture.
//================================================================================================
void TileManager::copySpotToAtlas(uchar *dstBuf)
{
    const unsigned int OFFSET = BORDER_SIZE*2+TILE_SIZE/2;
    Image srcImage;
    dstBuf+= 6*(BORDER_SIZE*2+TILE_SIZE)* RGB_A;
    String srcFilename = "Spotlight.png";
    if (!loadImage(srcImage, srcFilename, false))
    {
        Logger::log().error() << "The spotlight-mask was not found.";
        return;
    }
    if ((srcImage.getWidth() != TILE_SIZE/2) || (srcImage.getHeight() != TILE_SIZE/2))
    {
        Logger::log().error() << "Gfx " << srcFilename << " has the wrong size! Only " << (int)TILE_SIZE << "^2 is supported.";
        return;
    }
    if (srcImage.getFormat()!=PF_R8G8B8)
    {
        Logger::log().error() << "Gfx " << srcFilename << " has the wrong pixelformat. Only RGB is supported for masks.";
        return;
    }
    uchar *dst = dstBuf;
    for (int ty = 0; ty < ATLAS_LAND_ROWS*2; ++ty)
    {
        uchar *src = srcImage.getData();
        for (unsigned int y = 0; y < TILE_SIZE/2; ++y)
        {
            for (unsigned int x = 0; x < TILE_SIZE/2; ++x)
            {
                dst[(0*OFFSET+x)*RGB_A+3] = src[x*RGB];
                dst[(1*OFFSET+x)*RGB_A+3] = src[x*RGB];
                dst[(2*OFFSET+x)*RGB_A+3] = src[x*RGB];
            }
            dst+= MAX_TEXTURE_SIZE*RGB_A;
            src+= TILE_SIZE/2*RGB;
        }
        dst+= BORDER_SIZE*2*MAX_TEXTURE_SIZE*RGB_A;
    }
}

//================================================================================================
// Create a template for the mask.
//================================================================================================
void TileManager::createMaskTemplate()
{
    const unsigned char color[3] = {0xC6, 0x38, 0xDB};
    int lineSkip = TILE_SIZE * RGB;
    const int UNUSED_SIZE = 16;
    uchar *dstBuf = new uchar[TILE_SIZE * TILE_SIZE * RGB];
    memset(dstBuf, 0x00, TILE_SIZE * TILE_SIZE * RGB);
    Image dstImage;
    dstImage.loadDynamicImage(dstBuf, TILE_SIZE, TILE_SIZE, 1, PF_R8G8B8);
    // vertical borderline
    for (int y = 0; y < TILE_SIZE/2; ++y)
    {
        for (int c = 0; c < 3; ++c)
        {
            dstBuf[(y*TILE_SIZE                )*RGB +c] = color[c];
            dstBuf[(y*TILE_SIZE + TILE_SIZE-1  )*RGB +c] = color[c];
            dstBuf[(y*TILE_SIZE + TILE_SIZE/2  )*RGB +c] = color[c];
            dstBuf[(y*TILE_SIZE + TILE_SIZE/2-1)*RGB +c] = color[c];
        }
    }
    // Horizontal borderlines for karos
    uchar *p = dstBuf + TILE_SIZE/2*lineSkip;
    for (int i = 0; i < 2; ++i)
    {
        for (int height= 0; height < UNUSED_SIZE; ++height)
        {
            for (int x = 0; x < TILE_SIZE/2; ++x)
            {
                for (int c = 0; c < 3; ++c)
                {
                    p[((height              )*TILE_SIZE+x)*RGB +c] = color[c]; // Top pos
                    p[((TILE_SIZE/2-height-2)*TILE_SIZE+x)*RGB +c] = color[c]; // Bottom pos
                }
            }
        }
        p+= lineSkip;
    }
    // Vertical borderlines for karos
    p = dstBuf + TILE_SIZE/2*lineSkip + TILE_SIZE/2*RGB;
    for (int i = 0; i < 2; ++i)
    {
        for (int width= 0; width < UNUSED_SIZE; ++width)
        {
            for (int y = 0; y < TILE_SIZE/2; ++y)
            {
                for (int c = 0; c < 3; ++c)
                {
                    p[(y*TILE_SIZE + width                )*RGB +c] = color[c]; // Left pos
                    p[(y*TILE_SIZE - width + TILE_SIZE/2-2)*RGB +c] = color[c];// Right pos
                }
            }
        }
        p+= lineSkip;
    }
    // Karos
    int offset = 0;
    p = dstBuf + lineSkip * TILE_SIZE/2;
    int karoSize= TILE_SIZE/2;
    for (int y = 0; y < karoSize; ++y)
    {
        for (int x = 0; x < karoSize; ++x)
        {
            if ((x == karoSize/2 - offset))
                x+= 2*offset;
            for (int col = 0; col < 2; ++col)
            {
                p[(x+col*karoSize)*RGB+ 0] = 0xC6; // R
                p[(x+col*karoSize)*RGB+ 1] = 0x38; // G
                p[(x+col*karoSize)*RGB+ 2] = 0xDB; // B
            }
        }
        if (y < karoSize/2) ++offset; else --offset;
        p+=lineSkip;
    }
    String filename = mPathGfxTiles;
    filename+= "TemplateMask.png";
    dstImage.save(filename);
    delete[] dstBuf;
}

//================================================================================================
//
//================================================================================================
bool TileManager::setResourcePath(String key, String &refPath)
{
    ConfigFile cf; cf.load("resources.cfg");
    ConfigFile::SectionIterator seci = cf.getSectionIterator();
    while (seci.hasMoreElements())
    {
        ConfigFile::SettingsMultiMap *settings = seci.getNext();
        for (ConfigFile::SettingsMultiMap::iterator i = settings->begin(); i != settings->end(); ++i)
        {
            if (StringUtil::match(i->second, "*"+key, false))
            {
                struct stat fileInfo;
                if (stat(i->second.c_str(), &fileInfo))
                {
                    Logger::log().success(false);
                    Logger::log().error() << "The '"<< key << "' entry given in 'resources.cfg' does not exist in the media folder!";
                    return false;
                }
                refPath = i->second + "/";
                Logger::log().success(true);
                return true;
            }
        }
    }
    Logger::log().success(false);
    Logger::log().error() << "The 'resources.cfg' is missing a '"<< key << "' entry!";
    return false;
}

//================================================================================================
// Load an existing image. Returns true on success.
// (Used to prevent ogre from spamming if image.load() fails).
//================================================================================================
bool TileManager::loadImage(Image &image, const Ogre::String &strFilename, bool logErrors)
{
    struct stat fileInfo;
    String strFile = mPathGfxTiles + strFilename;
    if (!stat(strFile.c_str(), &fileInfo))
    {
        try
        {
            image.load(strFilename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            return true;
        }
        catch (Exception &) {}
    }
    if (logErrors)
        Logger::log().error() << "Error on opening file " << strFile;
    return false;
}
