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
#include "profiler.h"
#include "tile/tile_chunk.h"
#include "tile/tile_decal.h"
#include "tile/tile_atlas.h"
#include "tile/tile_manager.h"

using namespace Ogre;

String TileManager::LAND_PREFIX        = "Land";
String TileManager::WATER_PREFIX       = "Water";
String TileManager::UNDERGROWTH_PREFIX = "GrassWaving";
String TileManager::ATLAS_PREFIX       = "Atlas_Tiles";
String TileManager::MATERIAL_PREFIX    = "Terrain/";
static const unsigned int RGB  = 3; ///< Pixelsize.
static const unsigned int RGB_A= 4; ///< Pixelsize.


//================================================================================================
// Just for testing. Will be removed soon!
//================================================================================================
#ifndef TILEENGINE_SKIP_LEVELLOADING
#include <stdio.h>
#include "object/object_manager.h"
void TileManager::loadLvl()
{
    FILE *stream = fopen("client3d.lvl", "rb");
    if (!stream) return;
    fread(mMap, sizeof(mapStruct), mMapSizeX * mMapSizeZ, stream);
    fclose(stream);
    updateChunks();
    ObjectManager::getSingleton().syncToMapScroll(0, 0);
}
void TileManager::saveLvl()
{
    FILE *stream = fopen("client3d.lvl", "wb");
    fwrite(mMap, sizeof(mapStruct), mMapSizeX * mMapSizeZ, stream);
    fclose(stream);
}
#endif

SceneManager *TileManager::mSceneManager = 0;

//================================================================================================
// Constructor.
//================================================================================================
TileManager::TileManager()
{
    PROFILE()
    mMap = 0;
}

//================================================================================================
// Destructor.
//================================================================================================
TileManager::~TileManager()
{
    PROFILE()
    if (mMap) Logger::log().error() << "TileManager::freeRecources() was not called!";
}

//================================================================================================
// Free all resources.
//================================================================================================
void TileManager::freeRecources()
{
    PROFILE()
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
    PROFILE()
    Logger::log().headline() << "Init TileEngine";
    mSceneManager = SceneMgr;
    if (createAtlas)
    {
        TileAtlas::getSingleton().createAtlasTexture(ATLAS_PREFIX, 0);
    }
    mTextureSize = MAX_TEXTURE_SIZE >> lod;
    //mTextureSize = MAX_TEXTURE_SIZE;
    if (mTextureSize < 256) mTextureSize = 256;
    Logger::log().info() << "Setting LoD to " << lod << ". Atlas size is " << mTextureSize << "x" << mTextureSize<< ".";
    mRaySceneQuery = mSceneManager->createRayQuery(Ray());
    // Create the world map.
    mMapSizeX = 1; while (mMapSizeX < CHUNK_SIZE_X*2+4) mMapSizeX <<= 1; // Map size must be power of 2.
    mMapSizeZ = 1; while (mMapSizeZ < CHUNK_SIZE_Z*2+4) mMapSizeZ <<= 1; // Map size must be power of 2.
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
    Logger::log().attempt() << "Creating tile chunk...";
    mMapchunk.init(queryMaskLand, queryMaskWater, mSceneManager);
    setMapset(0, 0);
    //setLight(1.0f);
    setLight(0.6f);
    setWave(0.5f, HEIGHT_STRETCH, 1.5f);
    setUndergrowth(0.5f, 1.5f, 2.0f);
    setGrid(false);
    setRenderOptions(false);
}

//================================================================================================
// Change all Chunks.
//================================================================================================
void TileManager::updateChunks()
{
    PROFILE()
    mMapchunk.update();
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
void TileManager::setMap(unsigned int x, unsigned int z, uchar heightLand, uchar gfxLayer0, uchar heightWater, uchar gfxLayer1, bool spotLight)
{
    PROFILE()
    int ringBufferPos = ((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX);
    mMap[ringBufferPos].gfxLayer0  = gfxLayer0;
    mMap[ringBufferPos].gfxLayer1  = gfxLayer1;
    mMap[ringBufferPos].heightLand = heightLand;
    mMap[ringBufferPos].heightWater= heightWater;
    //mMap[ringBufferPos].normal     = ;
    mMap[ringBufferPos].spotLight  = spotLight;
}

//================================================================================================
// Returns the height of a tile-vertex.
//================================================================================================
Ogre::ushort TileManager::getMapHeight(unsigned int x, unsigned int z)
{
    PROFILE()
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].heightLand * HEIGHT_STRETCH;
}

//================================================================================================
// Returns the water level
//================================================================================================
Ogre::ushort TileManager::getMapWater(unsigned int x, unsigned int z)
{
    PROFILE()
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].heightWater * HEIGHT_STRETCH;
}

//================================================================================================
// Returns the gfx of a tile-vertex.
//================================================================================================
uchar TileManager::getMapLayer0(unsigned int x, unsigned int z)
{
    PROFILE()
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].gfxLayer0;
}

//================================================================================================
// Returns the gfx of a tile-vertex.
//================================================================================================
uchar TileManager::getMapLayer1(unsigned int x, unsigned int z)
{
    PROFILE()
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].gfxLayer1;
}

//================================================================================================
// Returns the shadow value of the given tile.
//================================================================================================
Real TileManager::getMapShadow(unsigned int x, unsigned int z)
{
    PROFILE()
    float h = getMapHeight(x, z);
    if (getMapHeight(x+1, z) > h || getMapHeight(x-1, z) > h || getMapHeight(x, z+1) > h || getMapHeight(x, z-1) > h ||
            getMapHeight(x-1, z+1) > h || getMapHeight(x+1, z+1) >h || getMapHeight(x-1, z-1) > h || getMapHeight(x+1, z-1) > h)
        return 0.75f;
    return 1.00f;
}

//================================================================================================
// Returns true if the given tile is illuminated by a spot light.
//================================================================================================
bool TileManager::getMapSpotLight(unsigned int x, unsigned int z)
{
    PROFILE()
    return mMap[((mMapSPosZ + z)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + x)&mMapMaskX)].spotLight;
}

//================================================================================================
// Scroll the map by 1 tile (subtile scrolling is not possible!).
//================================================================================================
void TileManager::scrollMap(int dx, int dz)
{
    PROFILE()
    mMapSPosX= (mMapSPosX-dx*2)&mMapMaskX;
    mMapSPosZ= (mMapSPosZ-dz*2)&mMapMaskZ;
    mMapchunk.update();
}

//================================================================================================
// Change Tile textures.
//================================================================================================
void TileManager::setMapset(int landGroup, int /*waterGroup*/)
{
    PROFILE()
    mMapchunk.setMaterial(landGroup,  mTextureSize);
}

//================================================================================================
// A tile was clicked.
//================================================================================================
void TileManager::tileClick(float mouseX, float mouseY)
{
    PROFILE()
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
                mVertex[0].x = (x+0.0f)*HALF_RENDER_SIZE;
                mVertex[0].y = getMapHeight(x, z);
                mVertex[0].z = (z+0.0f)*HALF_RENDER_SIZE;
                mVertex[1].x = (x+0.0f)*HALF_RENDER_SIZE;
                mVertex[1].y = getMapHeight(x, z-1);
                mVertex[1].z = (z+1.0f)*HALF_RENDER_SIZE;
                mVertex[2].x = (x+1.0f)*HALF_RENDER_SIZE;
                mVertex[2].y = getMapHeight(x-1, z);
                mVertex[2].z = (z+0.0f)*HALF_RENDER_SIZE;
                if (vertexPick(&mouseRay, x, z, 0)) return; // We got a hit.
                //   +
                //  /|
                // +-+
                mVertex[0].x = (x+1.0f)*HALF_RENDER_SIZE;
                mVertex[0].y = getMapHeight(x+1, z);
                mVertex[0].z = (z+0.0f)*HALF_RENDER_SIZE;
                mVertex[1].x = (x+0.0f)*HALF_RENDER_SIZE;
                mVertex[1].y = getMapHeight(x, z-1);
                mVertex[1].z = (z+1.0f)*HALF_RENDER_SIZE;
                mVertex[2].x = (x+1.0f)*HALF_RENDER_SIZE;
                mVertex[2].y = getMapHeight(x+1, z-1);
                mVertex[2].z = (z+1.0f)*HALF_RENDER_SIZE;
                if (vertexPick(&mouseRay, x, z, 1)) return; // We got a hit.
            }
            else
            {
                //
                //   +
                //   |\.
                //   +-+
                mVertex[0].x = (x+0.0f)*HALF_RENDER_SIZE;
                mVertex[0].y = getMapHeight(x, z);
                mVertex[0].z = (z+0.0f)*HALF_RENDER_SIZE;
                mVertex[1].x = (x+0.0f)*HALF_RENDER_SIZE;
                mVertex[1].y = getMapHeight(x, z-1);
                mVertex[1].z = (z+1.0f)*HALF_RENDER_SIZE;
                mVertex[2].x = (x+1.0f)*HALF_RENDER_SIZE;
                mVertex[2].y = getMapHeight(x+1, z-1);
                mVertex[2].z = (z+1.0f)*HALF_RENDER_SIZE;
                if (vertexPick(&mouseRay, x, z, 2)) return; // We got a hit.
                // +-+
                //  \|
                //   +
                mVertex[0].x = (x+1.0f)*HALF_RENDER_SIZE;
                mVertex[0].y = getMapHeight(x+1, z-1);
                mVertex[0].z = (z+1.0f)*HALF_RENDER_SIZE;
                mVertex[1].x = (x+1.0f)*HALF_RENDER_SIZE;
                mVertex[1].y = getMapHeight(x+1, z);
                mVertex[1].z = (z+0.0f)*HALF_RENDER_SIZE;
                mVertex[2].x = (x+0.0f)*HALF_RENDER_SIZE;
                mVertex[2].y = getMapHeight(x, z);
                mVertex[2].z = (z+0.0f)*HALF_RENDER_SIZE;
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
    PROFILE()
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
    PROFILE()
    mSelectedVertexX+= deltaX; if (mSelectedVertexX > CHUNK_SIZE_X) mSelectedVertexX = 0;
    mSelectedVertexZ+= deltaZ; if (mSelectedVertexZ > CHUNK_SIZE_Z) mSelectedVertexZ = 0;
    highlightVertex(mSelectedVertexX, mSelectedVertexZ);
}

//================================================================================================
// .
//================================================================================================
void TileManager::highlightVertex(int x, int z)
{
    PROFILE()
    static SceneNode *tcNode = 0;
    if (!tcNode)
    {
        int size = HALF_RENDER_SIZE/6;
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
    x*= HALF_RENDER_SIZE;
    z*= HALF_RENDER_SIZE;
    tcNode->setPosition((Real)x, getTileHeight(x, z), (Real)z);
}

//================================================================================================
// .
//================================================================================================
void TileManager::updateTileHeight(int deltaHeight)
{
    PROFILE()
    mMap[((mMapSPosZ + mSelectedVertexZ)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + mSelectedVertexX)&mMapMaskX)].heightLand+= deltaHeight;
    mMapchunk.update();
    highlightVertex(mSelectedVertexX, mSelectedVertexZ);
}

//================================================================================================
// .
//================================================================================================
void TileManager::updateTileGfx(int deltaGfxNr)
{
    PROFILE()
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
    PROFILE()
    mMap[((mMapSPosZ + mSelectedVertexZ)&mMapMaskZ)*mMapSizeX + ((mMapSPosX + mSelectedVertexX)&mMapMaskX)].gfxLayer0 = mEditorActSelectedGfx;
    mMapchunk.update();
}

//================================================================================================
// Helper function for getTileHeight(...);
//================================================================================================
int TileManager::calcHeight(int vert0, int vert1, int vert2, int posX, int posZ)
{
    PROFILE()
    if (posZ == HALF_RENDER_SIZE) return vert1;
    int h1 = ((vert1 - vert0) * posZ) / HALF_RENDER_SIZE + vert0;
    int h2 = ((vert1 - vert2) * posZ) / HALF_RENDER_SIZE + vert2;
    int maxX = HALF_RENDER_SIZE - posZ;
    return ((h2 - h1) * posX) / maxX + h1;
}

//================================================================================================
// Return the exact height of a position within a tile.
//================================================================================================
short TileManager::getTileHeight(int posX, int posZ)
{
    PROFILE()
    int TileX = posX / HALF_RENDER_SIZE; // Get the Tile position within the map.
    int TileZ = posZ / HALF_RENDER_SIZE; // Get the Tile position within the map.
    posX&= (HALF_RENDER_SIZE-1);         // Lower part is the position within the tile.
    posZ&= (HALF_RENDER_SIZE-1);         // Lower part is the position within the tile.
    //   +-+v2
    //   |/|
    // v1+-+
    if ((TileX+TileZ)&1)
    {
        int v1 = getMapHeight(TileX  , TileZ+1); // BL
        int v2 = getMapHeight(TileX+1, TileZ  ); // TR
        if (HALF_RENDER_SIZE - posX > posZ)
            return calcHeight(getMapHeight(TileX, TileZ), v1, v2, posX, posZ); // TL
        return calcHeight(getMapHeight(TileX+1, TileZ+1), v1, v2, HALF_RENDER_SIZE-posZ, HALF_RENDER_SIZE-posX); // BR
    }
    // v1+-+
    //   |\|
    //   +-+v2
    int v1 = getMapHeight(TileX  , TileZ  ); // TL
    int v2 = getMapHeight(TileX+1, TileZ+1); // BR
    if (posX < posZ)
        return calcHeight(getMapHeight(TileX  , TileZ+1), v1, v2, posX, HALF_RENDER_SIZE-posZ); // BL
    return calcHeight(getMapHeight(TileX+1, TileZ  ), v1, v2, posZ, HALF_RENDER_SIZE-posX); //TR
}
