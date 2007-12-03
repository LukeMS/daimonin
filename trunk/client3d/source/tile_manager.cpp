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
#include "option.h"

using namespace Ogre;

//#define LOG_TIMING

/** Pixel per tile in the terrain-texture. */
const int PIXEL_PER_TILE =  128;

/** Pixel per col/row in the terrain-texture. */
const int PIXEL_PER_ROW  = 1024;

/** Numbers of tile textures in one row/col of the terrain texture. */
const int TEXTURES_PER_ROW = 7;

//================================================================================================
// Constructor.
//================================================================================================
TileManager::TileManager()
{
    mMapScrollX =0;
    mMapScrollZ =0;
    for (int z =0; z <= CHUNK_SIZE_Z; ++z)
        clsRowOfWalls(z);
}

//================================================================================================
// Destructor.
//================================================================================================
TileManager::~TileManager()
{}

//================================================================================================
//
//================================================================================================
void TileManager::freeRecources()
{
    for (int z =0; z <= CHUNK_SIZE_Z; ++z)
        delRowOfWalls(z);
    mMapchunk.freeRecources();
}

//================================================================================================
// Init the TileEngine.
//================================================================================================
void TileManager::Init(SceneManager* SceneMgr, int tileTextureSize)
{
    Logger::log().headline("Init TileEngine");
    mSceneManager = SceneMgr;
    mTileTextureSize = tileTextureSize;
    mGrid = false;
    // ////////////////////////////////////////////////////////////////////
    // Create all TextureGroups.
    // ////////////////////////////////////////////////////////////////////
    std::string strTextureGroup = "terrain";
    Logger::log().info() << "Creating texture group " << strTextureGroup;
    if (Option::getSingleton().getIntValue(Option::CMDLINE_CREATE_TILE_TEXTURES))
    {
        // only needed after a tile-texture has changed.
        createTextureGroup(strTextureGroup);
    }
    // ////////////////////////////////////////////////////////////////////
    // Create TileChunks.
    // ////////////////////////////////////////////////////////////////////
    Logger::log().info() << "Creating tile-chunks";
    createChunks();
    setMaterialLOD(tileTextureSize);

    // ////////////////////////////////////////////////////////////////////
    // Init is done.
    // ////////////////////////////////////////////////////////////////////
    Logger::log().info() << "Init done.";
    Logger::log().headline("Starting TileEngine");
}

//================================================================================================
// .
//================================================================================================
void TileManager::calcVertexHeight()
{
    for (int z=0; z < CHUNK_SIZE_Z; ++z)
        for (int x=0; x < CHUNK_SIZE_X; ++x)
        {
/*
            mMap[x][z].height[VERTEX_BL] = (getMapHeight(x,z) + getMapHeight(x,z+1) + getMapHeight(x-1,z) + getMapHeight(x-1,z+1)) /4;
            mMap[x][z].height[VERTEX_TL] = (getMapHeight(x,z) + getMapHeight(x,z-1) + getMapHeight(x-1,z) + getMapHeight(x-1,z-1)) /4;
            mMap[x][z].height[VERTEX_TR] = (getMapHeight(x,z) + getMapHeight(x,z-1) + getMapHeight(x+1,z) + getMapHeight(x+1,z-1)) /4;
            mMap[x][z].height[VERTEX_BR] = (getMapHeight(x,z) + getMapHeight(x,z+1) + getMapHeight(x+1,z) + getMapHeight(x+1,z+1)) /4;
*/
            mMap[x][z].height[VERTEX_BL] = mMap[x  ][z+1].height[VERTEX_MID];
            mMap[x][z].height[VERTEX_TL] = mMap[x  ][z  ].height[VERTEX_MID];
            mMap[x][z].height[VERTEX_TR] = mMap[x+1][z  ].height[VERTEX_MID];
            mMap[x][z].height[VERTEX_BR] = mMap[x+1][z+1].height[VERTEX_MID];
            mMap[x][z].height[VERTEX_AVG]=(mMap[x][z].height[VERTEX_BL] + mMap[x][z].height[VERTEX_TL] + mMap[x][z].height[VERTEX_TR] + mMap[x][z].height[VERTEX_BR])/4;
        }
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
        for (int x = 0; x < CHUNK_SIZE_X; ++x)
            for (int y = 0; y <= CHUNK_SIZE_Z; ++y)
                mMap[x][y] = mMap[x+1][y];
        clsColOfWalls(CHUNK_SIZE_X); // Set all Entities to 0.
    }
    else if (dx <0)
    {
        ++mMapScrollX;
        delColOfWalls(CHUNK_SIZE_X); // Delete walls leaving the view.
        for (int x = CHUNK_SIZE_X; x >0; --x)
            for (int y = 0; y <= CHUNK_SIZE_Z; ++y)
                mMap[x][y] = mMap[x-1][y];
        clsColOfWalls(0); // Set all Entities to 0.
    }
    if (dz >0)
    {
        --mMapScrollZ;
        delRowOfWalls(0); // Delete walls leaving the view.
        for (int x = 0; x <= CHUNK_SIZE_X; ++x)
            for (int y = 0; y < CHUNK_SIZE_Z; ++y)
                mMap[x][y] = mMap[x][y+1];
        clsRowOfWalls(CHUNK_SIZE_Z); // Set all Entities to 0.
    }
    else if (dz <0)
    {
        ++mMapScrollZ;
        delRowOfWalls(CHUNK_SIZE_Z); // Delete walls leaving the view.
        for (int x = 0; x <= CHUNK_SIZE_X; ++x)
            for (int y = CHUNK_SIZE_Z; y > 0; --y)
                mMap[x][y] = mMap[x][y-1];
        clsRowOfWalls(0); // Set all Entities to 0.
    }
    mMapchunk.change();
    syncWalls(-dx, -dz);
}

//================================================================================================
// Add a wall to a tile quadrant.
// Walls are the only non-moveable objects taht are not centered on a subtile.
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
    //tpos.y = TileManager::getSingleton().getTileHeight((int)tpos.x, (int)tpos.z);
    tpos.y = 30; // JUST A HACK! (server may send objects before tiles - so the height may be undefined).
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
    for (int x = 0; x <= CHUNK_SIZE_X; ++x)
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
    for (int x = 0; x <= CHUNK_SIZE_X; ++x)
        for (int pos = 0; pos < WALL_POS_SUM; ++pos)
            mMap[x][row].entity[pos] =0;
}

//================================================================================================
// Delete all walls that are scrolling out of the tile map.
//================================================================================================
void TileManager::delColOfWalls(int col)
{
    for (int z = 0; z <= CHUNK_SIZE_X; ++z)
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
    for (int z = 0; z <= CHUNK_SIZE_X; ++z)
        for (int pos = 0; pos < WALL_POS_SUM; ++pos)
            mMap[col][z].entity[pos] =0;
}

//================================================================================================
// When player moves over a tile border, the world scrolls.
//================================================================================================
void TileManager::syncWalls(int dx, int dz)
{
    for (int z =0; z <= CHUNK_SIZE_Z; ++z)
        for (int x =0; x <= CHUNK_SIZE_X; ++x)
            for (int pos = 0; pos < WALL_POS_SUM; ++pos)
                if (mMap[x][z].entity[pos])
                    mMap[x][z].entity[pos]->getParentSceneNode()->translate(dx*TILE_SIZE, 0 , dz*TILE_SIZE);
}

//================================================================================================
// Set the textures for the given height.
//================================================================================================
void TileManager::setMapTextures()
{
    short height;
    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int y = 0; y < CHUNK_SIZE_Z; ++y)
        {
            height = mMap[x][y].height[VERTEX_MID];
            // ////////////////////////////////////////////////////////////////////
            // Highland.
            // ////////////////////////////////////////////////////////////////////
            if (height > TileChunk::LEVEL_MOUNTAIN_TOP)
            {
                mMap[x][y].terrain_col = 0;
                mMap[x][y].terrain_row = 1;
            }
            else if (height > TileChunk::LEVEL_MOUNTAIN_MID)
            {
                {
                    mMap[x][y].terrain_col = 0;
                    mMap[x][y].terrain_row = 0;
                }
            }
            else if (height > TileChunk::LEVEL_MOUNTAIN_DWN)
            {
                mMap[x][y].terrain_col = 3;
                mMap[x][y].terrain_row = 2;
            }
            // ////////////////////////////////////////////////////////////////////
            // Plain.
            // ////////////////////////////////////////////////////////////////////
            else if (height > TileChunk::LEVEL_PLAINS_TOP)
            {
                // Plain
                mMap[x][y].terrain_col = 2;
                mMap[x][y].terrain_row = 2;
            }
            else if (height > TileChunk::LEVEL_PLAINS_MID)
            {
                mMap[x][y].terrain_col = 6;
                mMap[x][y].terrain_row = 3;
            }
            else if (height > TileChunk::LEVEL_PLAINS_DWN)
            {
                mMap[x][y].terrain_col = 0;
                mMap[x][y].terrain_row = 4;
            }
            else if (height > TileChunk::LEVEL_PLAINS_SUB)
            {
                mMap[x][y].terrain_col = 3;
                mMap[x][y].terrain_row = 3;
            }
            // ////////////////////////////////////////////////////////////////////
            // Sea-Ground.
            // ////////////////////////////////////////////////////////////////////
            else
            {
                mMap[x][y].terrain_col = 3;
                mMap[x][y].terrain_row = 3;
            }
        }
    }
}

//================================================================================================
// Create all chunks.
//================================================================================================
void TileManager::createChunks()
{
#ifdef LOG_TIMING
    unsigned long time = Root::getSingleton().getTimer()->getMicroseconds();
#endif
    TileChunk::mBounds = new AxisAlignedBox(0, 0, 0, TILE_SIZE * CHUNK_SIZE_X, 100, TILE_SIZE * CHUNK_SIZE_Z);
    mMapchunk.create(mTileTextureSize);
    //calcVertexHeight();
    delete TileChunk::mBounds;
#ifdef LOG_TIMING
    Logger::log().info() << "Time to create Chunks: "
    << (double)(Root::getSingleton().getTimer()->getMicroseconds() - time)/1000 << " ms";
#endif
}

//================================================================================================
// Change all Chunks.
//================================================================================================
void TileManager::changeChunks()
{
#ifdef LOG_TIMING
    unsigned long time = Root::getSingleton().getTimer()->getMicroseconds();
#endif
    //TileChunk::mBounds = new AxisAlignedBox(0, 0, 0, TILE_SIZE * CHUNK_SIZE_X, 100, TILE_SIZE * CHUNK_SIZE_Z);
    //setMapTextures();
    mMapchunk.change();
    calcVertexHeight();
    //delete TileChunk::mBounds;
#ifdef LOG_TIMING
    Logger::log().info() << "Time to change Chunks: "
    << (double)(Root::getSingleton().getTimer()->getMicroseconds() - time)/1000 << " ms";
#endif
}

//================================================================================================
// Change Tile and Environmet textures.
//================================================================================================
void TileManager::changeTexture()
{
    static bool once = false;
    const std::string strFilename = "terrain_032_texture.png";
    if (once)
        return;
    else
        once=true;
#ifdef LOG_TIMING
    unsigned long time = Root::getSingleton().getTimer()->getMicroseconds();
#endif
    Image tMap;
    if (!loadImage(tMap, strFilename))
    {
        Logger::log().error() << "Group texture '" << strFilename << "' was not found.";
        return;
    }
    MaterialPtr mMaterial = MaterialManager::getSingleton().getByName("Land_HighDetails128");
    std::string texName = "testMat";
    TexturePtr mTexture = TextureManager::getSingleton().loadImage(texName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, tMap, TEX_TYPE_2D, 3,1.0f);
    mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texName);
    // mMaterial->unload();
    mMaterial->load();
#ifdef LOG_TIMING
    Logger::log().info() << "Time to change Chunks: "
    << (double)(Root::getSingleton().getTimer()->getMicroseconds() - time)/1000 << " ms";
#endif
}

//================================================================================================
// If the file exists load it into an image.
//================================================================================================
bool TileManager::loadImage(Image &image, const Ogre::String &strFilename)
{
    std::string strTemp = PATH_TILE_TEXTURES + strFilename;
    std::ifstream chkFile;
    chkFile.open(strTemp.c_str());
    if (!chkFile)
        return false;
    chkFile.close();
    image.load(strFilename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    return true;
}

//================================================================================================
// Create the texture-file out of the single textures + filter texture.
//================================================================================================
bool TileManager::createTextureGroup(const std::string &terrain_type)
{
    std::string strFilename;
    // ////////////////////////////////////////////////////////////////////
    // Create grid texture.
    // ////////////////////////////////////////////////////////////////////
    Image grid;
    uint32 *grid_data = new uint32[PIXEL_PER_TILE * PIXEL_PER_TILE];
    grid.loadDynamicImage((unsigned char*)grid_data, PIXEL_PER_TILE, PIXEL_PER_TILE, PF_R8G8B8A8);
    for (int x = 0; x < PIXEL_PER_TILE; ++x)
    {
        for (int y = 0; y < PIXEL_PER_TILE; ++y)
        {
            if ( x < 2 || y < 2 )
                grid_data[PIXEL_PER_TILE * y + x] = 0xffffffff;
            else
                grid_data[PIXEL_PER_TILE * y + x] = 0x00000000;
        }
    }
    strFilename = PATH_TILE_TEXTURES;
    strFilename+= "grid_" + StringConverter::toString(PIXEL_PER_TILE, 3, '0') + ".png";
    grid.save(strFilename);
    // ////////////////////////////////////////////////////////////////////
    // Shrink all filter-textures.
    // ////////////////////////////////////////////////////////////////////
    shrinkFilter();
    // ////////////////////////////////////////////////////////////////////
    // Shrink all tile-textures.
    // ////////////////////////////////////////////////////////////////////
    shrinkTexture(terrain_type);
    // ////////////////////////////////////////////////////////////////////
    // Create group-texture in various sizes.
    // ////////////////////////////////////////////////////////////////////
    int i, tx, ty;
    int pix = PIXEL_PER_TILE;
    unsigned char* TextureGroup_data;
    Image Filter, Texture, TextureGroup;
    while (pix >= MIN_TEXTURE_PIXEL)
    {
        TextureGroup_data = new unsigned char[PIXEL_PER_ROW * PIXEL_PER_ROW *4];
        TextureGroup.loadDynamicImage(TextureGroup_data, pix * 8, pix * 8,PF_A8B8G8R8);
        strFilename = "filter_" + StringConverter::toString(pix/2, 3, '0') + ".png";
        if (!loadImage(Filter, strFilename))
        {
            Logger::log().error() << "Filter texture '" << strFilename << "' was not found.";
            return false;
        }
        unsigned char* Filter_data = Filter.getData();
        i =-1;
        tx = 0;
        ty = 0;
        while (1)
        {
            strFilename = terrain_type;
            strFilename+= "_"+ StringConverter::toString(++i, 2, '0');
            strFilename+= "_"+ StringConverter::toString(pix, 3, '0') + ".png";
            if (!loadImage(Texture, strFilename))
                break;
            addToGroupTexture(TextureGroup_data, Filter_data, &Texture, pix, tx, ty);
            if (++tx == TEXTURES_PER_ROW)
            {
                if (++ty == TEXTURES_PER_ROW)
                    break;
                tx = 0;
            }
        }
        createTextureGroupBorders(TextureGroup_data, pix);
        strFilename = PATH_TILE_TEXTURES + terrain_type + "_texture";
        strFilename+= "_"+ StringConverter::toString(pix, 3, '0')+".png";
        TextureGroup.save(strFilename);

        delete[] TextureGroup_data;
        pix /= 2;
    }
    delete[] grid_data;
    return true;
}

//================================================================================================
// Shrink Texture.
//================================================================================================
void TileManager::shrinkTexture(const std::string &terrain_type)
{
    int sum=0, pix = PIXEL_PER_TILE / 2;
    Image image;
    std::string strFilename;
    while (pix >= MIN_TEXTURE_PIXEL)
    {
        sum =0;
        while (1)
        {
            // Load the Image.
            strFilename = terrain_type;
            strFilename+= "_"+ StringConverter::toString(sum, 2, '0');
            strFilename+= "_"+ StringConverter::toString(pix+pix, 3, '0') + ".png";
            if (!loadImage(image, strFilename))
                break;
            // Resize the Image.
            image.resize((Ogre::ushort)image.getWidth()/2, (Ogre::ushort)image.getHeight()/2, Image::FILTER_BILINEAR);
            // Save the Image.
            strFilename = PATH_TILE_TEXTURES;
            strFilename+= terrain_type;
            strFilename+= "_"+ StringConverter::toString(sum, 2, '0');
            strFilename+= "_"+ StringConverter::toString(pix, 3, '0') + ".png";
            image.save(strFilename);
            ++sum;
        }
        pix /= 2;
    }
    Logger::log().info() << "Found " << StringConverter::toString(sum,2,'0') << " textures for group '" << terrain_type << "'.";
}

//================================================================================================
// Shrink the filter.
//================================================================================================
void TileManager::shrinkFilter()
{
    int pix = PIXEL_PER_TILE / 4;
    Image image;
    std::string strFilename;
    while (pix > MIN_TEXTURE_PIXEL/4)
    {
        strFilename = "filter_" + StringConverter::toString(pix+pix, 3, '0') + ".png";
        if (!loadImage(image, strFilename))
        {
            Logger::log().error() << "Filter texture '" << strFilename << "' was not found.";
            return;
        }
        image.resize((Ogre::ushort)image.getWidth()/2, (Ogre::ushort)image.getHeight()/2, Image::FILTER_BILINEAR);
        strFilename = PATH_TILE_TEXTURES;
        strFilename+= "filter_" + StringConverter::toString(pix, 3, '0') + ".png";
        image.save(strFilename);
        pix /= 2;
    }
}

//================================================================================================
// Add a texture to the terrain-texture.
//================================================================================================
inline void TileManager::addToGroupTexture(unsigned char* TextureGroup_data, unsigned char *Filter_data, Image* Texture, short pix, short x, short y)
{
    const int RGBA = 4;
    const int RGB  = 3;
    unsigned long index1, index2, index3;
    unsigned char* Texture_data = Texture->getData();
    int SPACE;
    if (pix <= MIN_TEXTURE_PIXEL)
    {
        SPACE = 1;
        index2 = 0;
        index1 = RGBA* (pix * 8)* (pix + 2*SPACE) * y +
                 RGBA* (pix * 8) * SPACE +
                 RGBA* x * (pix + 2*SPACE) +
                 RGBA* SPACE;

        for (int i = 0; i < pix; ++i)
        {
            TextureGroup_data[index1-4] = Texture_data[index2+ RGB*(pix-1)+0];
            TextureGroup_data[index1-3] = Texture_data[index2+ RGB*(pix-1)+1];
            TextureGroup_data[index1-2] = Texture_data[index2+ RGB*(pix-1)+2];
            TextureGroup_data[index1-1] = 255;
            for (int posX = 0; posX < pix; ++posX)
            {
                TextureGroup_data[  index1] = Texture_data[  index2];
                TextureGroup_data[++index1] = Texture_data[++index2];
                TextureGroup_data[++index1] = Texture_data[++index2];
                TextureGroup_data[++index1] = 255;
                ++index1;
                ++index2;
            }
            TextureGroup_data[index1+0] = Texture_data[index2-RGB*(pix-1)+ 0];
            TextureGroup_data[index1+1] = Texture_data[index2-RGB*(pix-1)+ 1];
            TextureGroup_data[index1+2] = Texture_data[index2-RGB*(pix-1)+ 2];
            TextureGroup_data[index1+3] = 255;
            index1+= RGBA* (pix * 7);
        }
        index1 = RGBA* (pix * 8)* (pix + 2*SPACE) * y +
                 RGBA* x * (pix + 2*SPACE);

        for (int i = 0; i < pix+2*SPACE; ++i)
        {
            TextureGroup_data[index1]                    = TextureGroup_data[index1+RGBA*8*pix*(pix)];
            TextureGroup_data[index1+RGBA*8*pix*(pix+1)] = TextureGroup_data[index1+RGBA*8*pix];
            ++index1;
            TextureGroup_data[index1]                    = TextureGroup_data[index1+RGBA*8*pix*(pix)];
            TextureGroup_data[index1+RGBA*8*pix*(pix+1)] = TextureGroup_data[index1+RGBA*8*pix];
            ++index1;
            TextureGroup_data[index1]                    = TextureGroup_data[index1+RGBA*8*pix*(pix)];
            TextureGroup_data[index1+RGBA*8*pix*(pix+1)] = TextureGroup_data[index1+RGBA*8*pix];
            ++index1;
            TextureGroup_data[index1]                    = 255;
            TextureGroup_data[index1+RGBA*8*pix*(pix+1)] = 255;
            ++index1;
        }
        return;
    }
    SPACE = pix / 32; // mipmap space.
    // ////////////////////////////////////////////////////////////////////
    // Copy the tile into the texture.
    // ////////////////////////////////////////////////////////////////////
    // Group-texture : 32 bit (RGBA)
    // Tile-texture  : 24 bit (RGB)
    // Filter-texture: 24 bit (RGB)
    int dstOffX = RGBA* (pix/2 + 2*SPACE);
    int dstOffY = RGBA* (pix/2 + 2*SPACE) * pix * 8;
    int srcOffX = RGB * (pix/2);
    int srcOffY = RGB * (pix/2) * pix;
    unsigned char alpha = 255;

    index2 = index3 =0;
    index1 = RGBA* (pix * 8) * (pix + 4 * SPACE) * y +
             RGBA* (pix * 8) *SPACE +
             RGBA*  x * (pix + 4* SPACE) +
             RGBA* SPACE;
    for (int posY = 0; posY < pix/2; ++posY)
    {
        for (int posX = 0; posX < pix/2; ++posX)
        {
            TextureGroup_data[  index1                ] = Texture_data[  index2                ]; // Top Left  subTile.
            TextureGroup_data[  index1+dstOffX        ] = Texture_data[  index2+srcOffX        ]; // Top Right subTile.
            TextureGroup_data[  index1+dstOffY        ] = Texture_data[  index2+srcOffY        ]; // Bot Left subTile.
            TextureGroup_data[  index1+dstOffX+dstOffY] = Texture_data[  index2+srcOffX+srcOffY]; // Bot Righr subTile.

            TextureGroup_data[++index1                ] = Texture_data[++index2                ];
            TextureGroup_data[  index1+dstOffX        ] = Texture_data[  index2+srcOffX        ];
            TextureGroup_data[  index1+dstOffY        ] = Texture_data[  index2+srcOffY        ];
            TextureGroup_data[  index1+dstOffX+dstOffY] = Texture_data[  index2+srcOffX+srcOffY];

            TextureGroup_data[++index1                ] = Texture_data[++index2                ];
            TextureGroup_data[  index1+dstOffX        ] = Texture_data[  index2+srcOffX        ];
            TextureGroup_data[  index1+dstOffY        ] = Texture_data[  index2+srcOffY        ];
            TextureGroup_data[  index1+dstOffX+dstOffY] = Texture_data[  index2+srcOffX+srcOffY];

            if (pix > MIN_TEXTURE_PIXEL)
                alpha = Filter_data[index3];
            TextureGroup_data[++index1                ] = alpha;
            TextureGroup_data[  index1+dstOffX        ] = alpha;
            TextureGroup_data[  index1+dstOffY        ] = alpha;
            TextureGroup_data[  index1+dstOffX+dstOffY] = alpha;

            ++index1;
            ++index2;
            index3+= 3;
        }
        index1+= 4* (pix * 8 - pix/2);
        index2+= 3* (pix/2);
    }
}

//================================================================================================
// .
//================================================================================================
inline void TileManager::createTextureGroupBorders(unsigned char* TextureGroup_data, short pix)
{
    if (pix <= MIN_TEXTURE_PIXEL)
        return;
    // ////////////////////////////////////////////////////////////////////
    // Vertical border creation
    // ////////////////////////////////////////////////////////////////////
    const int RGBA = 4;
    const int SPACE = pix / 32; // mipmap space.
    long index1;
    for (int col = 0; col < 7; ++col)
    {
        index1 = col * RGBA* (pix + 4 *SPACE);
        for (int posY = 0; posY < pix *8; ++posY)
        {
            for (int posX = 0; posX < SPACE; ++posX)
            {
                TextureGroup_data[index1]                        = TextureGroup_data[index1+ RGBA*(pix   + 2*SPACE)];
                TextureGroup_data[index1+ RGBA*(pix/2 + 1*SPACE)]= TextureGroup_data[index1+ RGBA*(pix/2 + 3*SPACE)];
                TextureGroup_data[index1+ RGBA*(pix/2 + 2*SPACE)]= TextureGroup_data[index1+ RGBA*(pix/2)];
                TextureGroup_data[index1+ RGBA*(pix   + 3*SPACE)]= TextureGroup_data[index1+ RGBA*(SPACE)];
                ++index1;
                TextureGroup_data[index1]                        = TextureGroup_data[index1+ RGBA*(pix + 2*SPACE)];
                TextureGroup_data[index1+ RGBA*(pix/2 + 1*SPACE)]= TextureGroup_data[index1+ RGBA*(pix/2 + 3*SPACE)];
                TextureGroup_data[index1+ RGBA*(pix/2 + 2*SPACE)]= TextureGroup_data[index1+ RGBA*(pix/2)];
                TextureGroup_data[index1+ RGBA*(pix   + 3*SPACE)]= TextureGroup_data[index1+ RGBA*(SPACE)];
                ++index1;
                TextureGroup_data[index1]                        = TextureGroup_data[index1+ RGBA*(pix + 2*SPACE)];
                TextureGroup_data[index1+ RGBA*(pix/2 + 1*SPACE)]= TextureGroup_data[index1+ RGBA*(pix/2 + 3*SPACE)];
                TextureGroup_data[index1+ RGBA*(pix/2 + 2*SPACE)]= TextureGroup_data[index1+ RGBA*(pix/2)];
                TextureGroup_data[index1+ RGBA*(pix   + 3*SPACE)]= TextureGroup_data[index1+ RGBA*(SPACE)];
                ++index1;
                TextureGroup_data[index1]                        = 255 -TextureGroup_data[index1+ RGBA*(pix + 2*SPACE)];
                TextureGroup_data[index1+ RGBA*(pix/2 + 1*SPACE)]= 255 -TextureGroup_data[index1+ RGBA*(pix/2 + 3*SPACE)];
                TextureGroup_data[index1+ RGBA*(pix/2 + 2*SPACE)]= 255 -TextureGroup_data[index1+ RGBA*(pix/2)];
                TextureGroup_data[index1+ RGBA*(pix   + 3*SPACE)]= 255 -TextureGroup_data[index1+ RGBA*(SPACE)];
                ++index1;
            }
            index1 += RGBA* (pix * 8 - SPACE);
        }
    }

    const int ROW_SKIP = RGBA* (pix * 8);
    // ////////////////////////////////////////////////////////////////////
    // Horizontal border creation
    // ////////////////////////////////////////////////////////////////////
    index1 =0;
    for (int row = 0; row < 7; ++row) // ersetzen durch x/y pos.
    {
        for (int posY = 0; posY < SPACE; ++posY)
        {
            for (int posX = 0; posX < pix*8; ++posX)
            {
                TextureGroup_data[index1]                           = TextureGroup_data[index1+ ROW_SKIP* (pix   + 2*SPACE)];
                TextureGroup_data[index1+ ROW_SKIP* (pix/2+  SPACE)]= TextureGroup_data[index1+ ROW_SKIP* (pix/2 + 3*SPACE)];
                TextureGroup_data[index1+ ROW_SKIP* (pix/2+2*SPACE)]= TextureGroup_data[index1+ ROW_SKIP* (pix/2)];
                TextureGroup_data[index1+ ROW_SKIP* (pix  +3*SPACE)]= TextureGroup_data[index1+ ROW_SKIP* SPACE];
                ++index1;
                TextureGroup_data[index1]                           = TextureGroup_data[index1+ ROW_SKIP* (pix + 2 * SPACE)];
                TextureGroup_data[index1+ ROW_SKIP* (pix/2+  SPACE)]= TextureGroup_data[index1+ ROW_SKIP* (pix/2 + 3*SPACE)];
                TextureGroup_data[index1+ ROW_SKIP* (pix/2+2*SPACE)]= TextureGroup_data[index1+ ROW_SKIP* (pix/2)];
                TextureGroup_data[index1+ ROW_SKIP* (pix  +3*SPACE)]= TextureGroup_data[index1+ ROW_SKIP* (SPACE)];
                ++index1;
                TextureGroup_data[index1]                           = TextureGroup_data[index1+ ROW_SKIP* (pix + 2 * SPACE)];
                TextureGroup_data[index1+ ROW_SKIP* (pix/2+  SPACE)]= TextureGroup_data[index1+ ROW_SKIP* (pix/2 + 3*SPACE)];
                TextureGroup_data[index1+ ROW_SKIP* (pix/2+2*SPACE)]= TextureGroup_data[index1+ ROW_SKIP* (pix/2)];
                TextureGroup_data[index1+ ROW_SKIP* (pix  +3*SPACE)]= TextureGroup_data[index1+ ROW_SKIP* (SPACE)];
                ++index1;
                TextureGroup_data[index1]                           = 255 -TextureGroup_data[index1+ ROW_SKIP* (pix + 2 * SPACE)];
                TextureGroup_data[index1+ ROW_SKIP* (pix/2+  SPACE)]= 255 -TextureGroup_data[index1+ ROW_SKIP* (pix/2 + 3*SPACE)];
                TextureGroup_data[index1+ ROW_SKIP* (pix/2+2*SPACE)]= 255 -TextureGroup_data[index1+ ROW_SKIP* (pix/2)];
                TextureGroup_data[index1+ ROW_SKIP* (pix  +3*SPACE)]= 255 -TextureGroup_data[index1+ ROW_SKIP* (SPACE)];
                ++index1;
            }
        }
        index1 += RGBA* (pix * 8) * (pix + 3*SPACE);
    }
}

//================================================================================================
// Toggle Material.
//================================================================================================
void TileManager::setMaterialLOD(int pixel)
{
    if (pixel != 128 && pixel !=64 && pixel != 32 && pixel != 16)
        return;
    String matWater, matLand;
    mTileTextureSize = pixel;
    matLand = "LandTiles" + StringConverter::toString(mTileTextureSize, 3, '0');
    matWater= "WaterTiles";
    //matWater= "Ocean2_Cg";
    //matWater= "Fresnel";
    if (mGrid)
    {
        matLand +="_Grid";
        matWater+="_Grid";
    }
    mMapchunk.setMaterial(matLand, matWater);
}

//================================================================================================
// Switch Grid.
//================================================================================================
void TileManager::toggleGrid()
{
    mGrid = !mGrid;
    setMaterialLOD(mTileTextureSize);
}

//================================================================================================
// Returns the exact height of a position within a triangle.
//================================================================================================
int TileManager::calcHeight(int vert0, int vert1, int vertMid, int posX, int posZ)
{
    if (posZ == HALF_SIZE) return vert1;
    int h1 = ((vert1 - vert0) * posZ) / HALF_SIZE + vert0;
    int h2 = ((vert1 - vertMid) * posZ) / HALF_SIZE + vertMid;
    int maxX = HALF_SIZE - posZ;
    return ((h2 - h1) * posX) / maxX + h1;
}

//================================================================================================
// Return the exact height of a position within a tile.
//================================================================================================
int TileManager::getTileHeight(int posX, int posZ)
{
    // ////////////////////////////////////////////////////////////////////
    // Get the vertex heights of the tile.
    // ////////////////////////////////////////////////////////////////////
    int TileX = posX / TileManager::TILE_SIZE;  // Get the Tile position within the map.
    int TileZ = posZ / TileManager::TILE_SIZE;  // Get the Tile position within the map.
    posX&= (TileManager::TILE_SIZE-1);          // Lower part is the position within the tile.
    posZ&= (TileManager::TILE_SIZE-1);          // Lower part is the position within the tile.
    unsigned int vert0  = TileManager::getSingleton().getMapHeight(TileX, TileZ, TileManager::VERTEX_BL);
    unsigned int vert1  = TileManager::getSingleton().getMapHeight(TileX, TileZ, TileManager::VERTEX_TL);
    unsigned int vert2  = TileManager::getSingleton().getMapHeight(TileX, TileZ, TileManager::VERTEX_TR);
    unsigned int vert3  = TileManager::getSingleton().getMapHeight(TileX, TileZ, TileManager::VERTEX_BR);
    unsigned int vertMid= TileManager::getSingleton().getMapHeight(TileX, TileZ, TileManager::VERTEX_AVG);
    // ////////////////////////////////////////////////////////////////////
    // Divide the tile into 8 trinagles and translate the tris positions
    // for calcHeight(...) to get always the same triangle.
    // ////////////////////////////////////////////////////////////////////
    posZ = TileManager::TILE_SIZE - posZ;
    if (posZ >= HALF_SIZE)
    {
        // Quadrant 1
        if (posX < HALF_SIZE)
        {
            if (TileManager::TILE_SIZE - posZ > posX) // pos b
                return calcHeight((vert0 + vert1) / 2, vert1, vertMid, posX, posZ - HALF_SIZE);
            return calcHeight((vert1 + vert2) / 2, vert1, vertMid, TileManager::TILE_SIZE - posZ, HALF_SIZE - posX);
        }
        // Quadrant 2
        else
        {
            if (posZ - HALF_SIZE > posX - HALF_SIZE) // pos a
                return calcHeight((vert1 + vert2) / 2, vert2, vertMid, TileManager::TILE_SIZE - posZ, posX - HALF_SIZE);
            return calcHeight((vert3 + vert2) / 2, vert2, vertMid, TileManager::TILE_SIZE - posX, posZ - HALF_SIZE);
        }
    }
    // Quadrant 3
    if (posX < HALF_SIZE)
    {
        if (posZ > posX) // pos b
            return calcHeight((vert1 + vert0) / 2, vert0, vertMid, posX, HALF_SIZE - posZ);
        return calcHeight((vert0 + vert3) / 2, vert0, vertMid, posZ, HALF_SIZE - posX);
    }
    // Quadrant 4
    if (TileManager::TILE_SIZE - posZ > posX) // pos a
        return calcHeight((vert3 + vert0) / 2, vert3, vertMid, posZ, posX - HALF_SIZE);
    return  calcHeight((vert3 + vert2) / 2, vert3, vertMid, TileManager::TILE_SIZE - posX, HALF_SIZE - posZ);
}
