/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include <iostream>
#include "OgreHardwarePixelBuffer.h"
#include "tile_chunk.h"
#include "tile_manager.h"
#include "logger.h"
#include "option.h"

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
    mInterface = 0;
    mMapScrollX = 0;
    mMapScrollZ = 0;
}

//================================================================================================
// Destructor.
//================================================================================================
TileManager::~TileManager()
{}

//================================================================================================
///
//================================================================================================
void TileManager::freeRecources()
{
    mMapchunk.freeRecources();
    delete mInterface;
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
    mInterface = new TileInterface(SceneMgr);

    Logger::log().info() << "Creating map";
    loadMap(FILE_HEIGHT_MAP);
    // ////////////////////////////////////////////////////////////////////
    // Create all TextureGroups.
    // ////////////////////////////////////////////////////////////////////
    std::string strTextureGroup = "terrain";
    Logger::log().info() << "Creating texture group " << strTextureGroup;
    if (Option::getSingleton().getIntValue(Option::CMDLINE_CREATE_TILE_TEXTURES))
    { // only needed after a tile-texture has changed.
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
// Create the worldmap.
//================================================================================================
void TileManager::loadMap(const std::string &png_filename)
{
    Image image;
    if (!loadImage(image, png_filename))
    {
        Logger::log().error() << "Heightmap '" << png_filename << "' was not found.";
        return;
    }
    uchar* heightdata_temp = image.getData();
    size_t dimx = image.getWidth();
    size_t dimy = image.getHeight();
    unsigned int posX = 0, posY;
    short Map[CHUNK_SIZE_X+3][CHUNK_SIZE_Z+3];
    // ////////////////////////////////////////////////////////////////////
    // Fill the heightdata buffer with the image-color.
    // ////////////////////////////////////////////////////////////////////
    for (int x = 0; x < CHUNK_SIZE_X+3; ++x)
    {
        posY =0;
        for (int y = 0; y < CHUNK_SIZE_Z+3; ++y)
        {
            Map[x][y] = heightdata_temp[posY * dimx + posX];
            if (++posY >= dimy)
                posY =0; // if necessary, repeat the image.
        }
        if (++posX >= dimx)
            posX =0; // if necessary, repeat the image.
    }

    for (int x = 0; x < CHUNK_SIZE_X+1; ++x)
    {
        for (int y = 0; y < CHUNK_SIZE_Z+1; ++y)
        {
            mMap[x][y].height = (Map[x+1][y+1] + Map[x+1][y+2] + Map[x+2][y+1] + Map[x+2][y+2]) / 4;
            mMap[x][y].indoorTris =0;
            for (int i =0; i< SUM_SUBTILES; ++i) mMap[x][y].walkable[i] =0;
            /*
            // Building ground.
            if (x>3 && x < 9 && y>1 && y<11)
            {
                mMap[x][y].height = 60;
                mMap[x][y].indoor = true;
            }
            */
            //   mMap[x][y].height=60;
        }
    }

    /*
         if (WallType = INNER_TOP_LEFT)  WallType = TRIANGLE_LEFT + TRIANGLE_TOP;
    else if (WallType = INNER_BOT_LEFT)  WallType = TRIANGLE_LEFT + TRIANGLE_BOTTOM;
    else if (WallType = INNER_TOP_RIGHT) WallType = TRIANGLE_RIGHT+ TRIANGLE_TOP;
    else if (WallType = INNER_BOT_RIGHT) WallType = TRIANGLE_RIGHT+ TRIANGLE_BOTTOM;
    else if (WallType = INNER_ALL)       WallType = TRIANGLE_RIGHT+ TRIANGLE_LEFT + TRIANGLE_TOP + TRIANGLE_BOTTOM;
    else logger << "Wrong wall type!"
    */

    // Building ground 2x2.
    //   x  z
    mMap[4][5].height = 60;
    mMap[4][5].indoor_col = 2;
    mMap[4][5].indoor_row = 4;
    mMap[4][5].indoorTris = TRIANGLE_RIGHT + TRIANGLE_BOTTOM;

    mMap[4][6].height = 60;
    mMap[4][6].indoor_col = 2;
    mMap[4][6].indoor_row = 4;
    mMap[4][6].indoorTris = TRIANGLE_RIGHT + TRIANGLE_TOP;
    //-----------------------//
    mMap[5][4].height = 60;
    mMap[5][4].indoor_col = 2;
    mMap[5][4].indoor_row = 4;
    mMap[5][4].indoorTris = TRIANGLE_RIGHT + TRIANGLE_BOTTOM;

    mMap[5][5].height = 60;
    mMap[5][5].indoor_col = 2;
    mMap[5][5].indoor_row = 4;
    mMap[5][5].indoorTris = TRIANGLE_LEFT + TRIANGLE_RIGHT + TRIANGLE_TOP + TRIANGLE_BOTTOM;

    mMap[5][6].height = 60;
    mMap[5][6].indoor_col = 2;
    mMap[5][6].indoor_row = 4;
    mMap[5][6].indoorTris = TRIANGLE_LEFT + TRIANGLE_TOP;

    //-----------------------//
    mMap[6][4].height = 60;
    mMap[6][4].indoor_col = 2;
    mMap[6][4].indoor_row = 4;
    mMap[6][4].indoorTris = TRIANGLE_LEFT + TRIANGLE_BOTTOM;

    mMap[6][5].height = 60;
    mMap[6][5].indoor_col = 2;
    mMap[6][5].indoor_row = 4;
    mMap[6][5].indoorTris = TRIANGLE_LEFT + TRIANGLE_RIGHT + TRIANGLE_TOP + TRIANGLE_BOTTOM;

    mMap[6][6].height = 60;
    mMap[6][6].indoor_col = 2;
    mMap[6][6].indoor_row = 4;
    mMap[6][6].indoorTris = TRIANGLE_RIGHT + TRIANGLE_TOP;

    //-----------------------//
    mMap[7][5].height = 60;
    mMap[7][5].indoor_col = 2;
    mMap[7][5].indoor_row = 4;
    mMap[7][5].indoorTris = TRIANGLE_LEFT + TRIANGLE_BOTTOM;

    mMap[7][6].height = 60;
    mMap[7][6].indoor_col = 2;
    mMap[7][6].indoor_row = 4;
    mMap[7][6].indoorTris = TRIANGLE_LEFT + TRIANGLE_TOP;

    setMapTextures();
}

//================================================================================================
// Saves the map.
//================================================================================================
void TileManager::scrollMap(int dx, int dz)
{
    // Quick hack. server will send us the map later.
    if (dx)
    {
        struct WorldMap bufferH[CHUNK_SIZE_Z];
        if (dx <0)
        {
            for (int y = 0; y < CHUNK_SIZE_Z; ++y)
            {
                bufferH[y] = mMap[0][y];
            }
            for (int x = 0; x < CHUNK_SIZE_X-1; ++x)
            {
                for (int y = 0; y < CHUNK_SIZE_Z; ++y)
                {
                    mMap[x][y] = mMap[x+1][y];
                }
            }
            for (int y = 0; y < CHUNK_SIZE_Z; ++y)
            {
                mMap[CHUNK_SIZE_X-1][y] = bufferH[y] ;
            }
        }
        else
        {
            for (int y = 0; y < CHUNK_SIZE_Z; ++y)
            {
                bufferH[y] = mMap[CHUNK_SIZE_X-1][y];
            }
            for (int x = CHUNK_SIZE_X-1; x >0; --x)
            {
                for (int y = 0; y < CHUNK_SIZE_Z; ++y)
                {
                    mMap[x][y] = mMap[x-1][y];
                }
            }
            for (int y = 0; y < CHUNK_SIZE_Z; ++y)
            {
                mMap[0][y] = bufferH[y];
            }
        }
    }
    if (dz)
    {
        struct WorldMap bufferH[CHUNK_SIZE_Z];
        if (dz <0)
        {
            for (int x = 0; x < CHUNK_SIZE_X; ++x)
            {
                bufferH[x] = mMap[x][0] ;
            }

            for (int x = 0; x < CHUNK_SIZE_X; ++x)
            {
                for (int y = 0; y < CHUNK_SIZE_Z-1; ++y)
                {
                    mMap[x][y] = mMap[x][y+1];
                }
            }

            for (int x = 0; x < CHUNK_SIZE_X; ++x)
            {
                mMap[x][CHUNK_SIZE_Z-1] = bufferH[x] ;
            }
        }
        else
        {
            for (int x = 0; x < CHUNK_SIZE_X; ++x)
            {
                bufferH[x] = mMap[x][CHUNK_SIZE_Z-1] ;
            }
            for (int x = 0; x < CHUNK_SIZE_X; ++x)
            {
                for (int y = CHUNK_SIZE_Z-1; y > 0; --y)
                {
                    mMap[x][y] = mMap[x][y-1];
                }
            }
            for (int x = 0; x < CHUNK_SIZE_X; ++x)
            {
                mMap[x][0]  = bufferH[x] ;
            }
        }
    }
    mMapScrollX+= dx;
    mMapScrollZ+= dz;
    changeChunks();
}

//================================================================================================
// Set the walkable status for a COMPLETE row of subtiles.
//================================================================================================
void TileManager::setWalkablePos(const SubPos2D &pos, int row, unsigned char walkables)
{
    mMap[pos.x][pos.z].walkable[row] |= walkables;
}

//================================================================================================
// Get the walkable status of a SINGLE subtile.
//================================================================================================
bool TileManager::getWalkablePos(int x, int y)
{
    return (mMap[x >> 3][y >> 3].walkable[y&7] & (1 << (x&7))) ==0;
}


//================================================================================================
// .
//================================================================================================
void TileManager::getMapScroll(int &x, int &z)
{
    x = mMapScrollX;
    z = mMapScrollZ;
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
            height = mMap[x][y].height;
            // ////////////////////////////////////////////////////////////////////
            // Highland.
            // ////////////////////////////////////////////////////////////////////
            /*
                        if (mMap[x][y].indoor)
                        {
                            mMap[x][y].terrain_col = INDOOR_COL;
                            mMap[x][y].terrain_row = INDOOR_ROW;
                        }
                        else
            */
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
            { // Plain
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
    TileChunk::mBounds = new AxisAlignedBox(
                             0, 0, 0,
                             TILE_SIZE_X * CHUNK_SIZE_X, 100, TILE_SIZE_Z * CHUNK_SIZE_Z);
    mMapchunk.create(mTileTextureSize);
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
    /*
        TileChunk::mBounds = new AxisAlignedBox(
                                 0, 0, 0,
                                 TILE_SIZE_X * CHUNK_SIZE_X, 100, TILE_SIZE_Z * CHUNK_SIZE_Z);
    */
    /*
        // Test start
        unsigned char value;
        for (int a = 0 ; a < TILES_SUmX; ++a)
        {
            for (int b = 0; b < TILES_SUmZ; ++b)
            {
                value = Get_Map_Height(a, b)+ 1;
                if (value > 220)
                    value = 0;
                Set_Map_Height(a, b, value);
            }
        }
        // Test stop
    */
    setMapTextures();
    mMapchunk.change();
    //    delete TileChunk::mBounds;

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
bool TileManager::loadImage(Image &image, const std::string &strFilename)
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
    grid.loadDynamicImage((uchar*)grid_data, PIXEL_PER_TILE, PIXEL_PER_TILE, PF_R8G8B8A8);
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
    uchar* TextureGroup_data;
    Image Filter, Texture, TextureGroup;
    while (pix >= MIN_TEXTURE_PIXEL)
    {
        TextureGroup_data = new uchar[PIXEL_PER_ROW * PIXEL_PER_ROW *4];
        TextureGroup.loadDynamicImage(TextureGroup_data, pix * 8, pix * 8,PF_A8B8G8R8);
        strFilename = "filter_" + StringConverter::toString(pix/2, 3, '0') + ".png";
        if (!loadImage(Filter, strFilename))
        {
            Logger::log().error() << "Filter texture '" << strFilename << "' was not found.";
            return false;
        }
        uchar* Filter_data = Filter.getData();
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
inline void TileManager::addToGroupTexture(uchar* TextureGroup_data, uchar *Filter_data, Image* Texture, short pix, short x, short y)
{
    const int RGBA = 4;
    const int RGB  = 3;
    unsigned long index1, index2, index3;
    uchar* Texture_data = Texture->getData();
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
            TextureGroup_data[  index1]                    = TextureGroup_data[index1+RGBA*8*pix*(pix)];
            TextureGroup_data[  index1+RGBA*8*pix*(pix+1)] = TextureGroup_data[index1+RGBA*8*pix];

            TextureGroup_data[++index1]                    = TextureGroup_data[index1+RGBA*8*pix*(pix)];
            TextureGroup_data[  index1+RGBA*8*pix*(pix+1)] = TextureGroup_data[index1+RGBA*8*pix];

            TextureGroup_data[++index1]                    = TextureGroup_data[index1+RGBA*8*pix*(pix)];
            TextureGroup_data[  index1+RGBA*8*pix*(pix+1)] = TextureGroup_data[index1+RGBA*8*pix];

            TextureGroup_data[++index1]                    = 255;
            TextureGroup_data[  index1+RGBA*8*pix*(pix+1)] = 255;

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
    uchar alpha = 255;

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
inline void TileManager::createTextureGroupBorders(uchar* TextureGroup_data, short pix)
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

