/*-----------------------------------------------------------------------------
This source file is part of Code-Black (http://www.code-black.org)
Copyright (c) 2005 by the Code-Black Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------*/

#ifdef LOG_TIMING
#include <ctime>
#endif
#include <cmath>
#include <iostream>
#include "OgreHardwarePixelBuffer.h"
#include "TileChunk.h"
#include "TileManager.h"
#include "logger.h"
#include "option.h"

///================================================================================================
/// Constructor
///================================================================================================
TileManager::TileManager()
{
    m_Interface = NULL;
}

///================================================================================================
/// Destructor
///================================================================================================
TileManager::~TileManager()
{
    for(int x = 0; x < TILES_SUM_X + 1; ++x)
    {
        delete[] m_Map[x];
    }
    delete[] m_Map;

    delete m_Interface;
}

///================================================================================================
/// Init the TileEngine.
///================================================================================================
void TileManager::Init(SceneManager* SceneMgr, int tileTextureSize, int tileStretchZ)
{
    Logger::log().headline("Init TileEngine:");
    m_SceneManager = SceneMgr;
    m_StretchZ = tileStretchZ;
    m_TileTextureSize = tileTextureSize;
    mGrid = false;
    m_Interface = new TileInterface(this);
    m_Interface->Init();
    mTileTexture.setNull();

    srand(1);
    Logger::log().info() << "Creating map";
    m_Map = new WorldMap*[TILES_SUM_X+1];
    for (int x = 0; x < TILES_SUM_X + 1; ++x)
    {
        m_Map[x] = new WorldMap[TILES_SUM_Z + 1];
    }
    Load_Map(FILE_HEIGHT_MAP);
    /// ////////////////////////////////////////////////////////////////////
    /// Create all TextureGroups.
    /// ////////////////////////////////////////////////////////////////////
    std::string strTextureGroup = "terrain";
    Logger::log().info() << "Creating texture group " << strTextureGroup;
    if (Option::getSingleton().getIntValue(Option::CREATE_TILE_TEXTURES))
    { /// only needed after a tile-texture has changed.
        CreateTextureGroup(strTextureGroup);
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Create TileChunks.
    /// ////////////////////////////////////////////////////////////////////
    Logger::log().info() << "Creating tile-chunks";
    CreateChunks();
    setMaterialLOD(tileTextureSize);

    /// ////////////////////////////////////////////////////////////////////
    /// Init is done.
    /// ////////////////////////////////////////////////////////////////////
    Logger::log().info() << "Init done.";
    Logger::log().headline("Starting TileEngine:");
}

///================================================================================================
/// Create the worldmap.
///================================================================================================
void TileManager::Load_Map(const std::string &png_filename)
{
    Image image;
    if (!LoadImage(image, png_filename))
    {
        Logger::log().error() << "Heightmap '" << png_filename << "' was not found.";
        return;
    }
    uchar* heightdata_temp = image.getData();
    size_t dimx = image.getWidth();
    size_t dimy = image.getHeight();
    unsigned int posX = 0, posY;
    short Map[TILES_SUM_X+2][TILES_SUM_Z+2];
    /// ////////////////////////////////////////////////////////////////////
    /// Fill the heightdata buffer with the image-color.
    /// ////////////////////////////////////////////////////////////////////
    for(int x = 0; x < TILES_SUM_X+2; ++x)
    {
        posY =0;
        for(int y = 0; y < TILES_SUM_Z+2; ++y)
        {
            if ( x && x != TILES_SUM_X && y && y != TILES_SUM_Z)
            {
                Map[x][y] = heightdata_temp[posY * dimx + posX];
            }
            else
            {
                Map[x][y] = 0;
            }
            if (++posY >= dimy)
                posY =0; // if necessary, repeat the image.
        }
        if (++posX >= dimx)
            posX =0; // if necessary, repeat the image.
    }

    // TODO : put this loop in the upper loop.
    for (int x = 0; x < TILES_SUM_X+1; ++x)
    {
        for (int y = 0; y < TILES_SUM_Z+1; ++y)
        {
            m_Map[x][y].height = (Map[x][y] + Map[x][y+1] + Map[x+1][y] + Map[x+1][y+1]) / 4;
        }
    }
    Set_Map_Textures();
}

///================================================================================================
/// Saves the map.
///================================================================================================
void TileManager::Save_Map(const std::string &png_filename)
{
    uchar *data = new uchar[TILES_SUM_X * TILES_SUM_Z];
    for(int x = 0; x < TILES_SUM_X; ++x)
    {
        for(int y = 0; y < TILES_SUM_Z; ++y)
        {
            data[y * TILES_SUM_X + x] = uchar  (m_Map[x][y].height);
        }
    }
    DataStreamPtr image_chunk(new MemoryDataStream ((void*)data,TILES_SUM_X * TILES_SUM_Z,false));
    Image img;
    img.loadRawData(image_chunk,TILES_SUM_X,TILES_SUM_Z, 1, PF_A8);
    /// save as a PNG
    std::stringstream tmp_FileName;
    tmp_FileName <<PATH_TILE_TEXTURES << png_filename;
    img.save(tmp_FileName.str());
    delete data;
}

///================================================================================================
/// Set the textures for the given height.
///================================================================================================
void TileManager::Set_Map_Textures()
{
    short height;
    for (int x = 0; x < TILES_SUM_X; ++x)
    {
        for (int y = 0; y < TILES_SUM_Z; ++y)
        {
            height = m_Map[x][y].height;
            /// ////////////////////////////////////////////////////////////////////
            // Highland.
            /// ////////////////////////////////////////////////////////////////////
            if (height > LEVEL_MOUNTAIN_TOP)
            {
                m_Map[x][y].terrain_col = 0;
                m_Map[x][y].terrain_row = 1;
            }
            else if (height > LEVEL_MOUNTAIN_MID)
            {
                {
                    m_Map[x][y].terrain_col = 0;
                    m_Map[x][y].terrain_row = 0;
                }
            }
            else if (height > LEVEL_MOUNTAIN_DWN)
            {
                m_Map[x][y].terrain_col = 3;
                m_Map[x][y].terrain_row = 2;
            }
            /// ////////////////////////////////////////////////////////////////////
            /// Plain.
            /// ////////////////////////////////////////////////////////////////////
            else if (height > LEVEL_PLAINS_TOP)
            { // Plain
                m_Map[x][y].terrain_col = 2;
                m_Map[x][y].terrain_row = 2;
            }
            else if (height > LEVEL_PLAINS_MID)
            {
                m_Map[x][y].terrain_col = 6;
                m_Map[x][y].terrain_row = 3;
            }
            else if (height > LEVEL_PLAINS_DWN)
            {
                m_Map[x][y].terrain_col = 0;
                m_Map[x][y].terrain_row = 4;
            }
            else if (height > LEVEL_PLAINS_SUB)
            {
                m_Map[x][y].terrain_col = 3;
                m_Map[x][y].terrain_row = 3;
            }
            /// ////////////////////////////////////////////////////////////////////
            /// Sea-Ground.
            /// ////////////////////////////////////////////////////////////////////
            else
            {
                m_Map[x][y].terrain_col = 3;
                m_Map[x][y].terrain_row = 3;
            }
        }
    }
}

///================================================================================================
/// Create all chunks.
///================================================================================================
void TileManager::CreateChunks()
{
#ifdef LOG_TIMING
    long time = clock();
#endif

    TileChunk::m_TileManagerPtr = this;
    TileChunk::m_bounds = new AxisAlignedBox(
                              -TILE_SIZE * CHUNK_SIZE_X, 0               , -TILE_SIZE * CHUNK_SIZE_Z,
                              +TILE_SIZE * CHUNK_SIZE_X, 100 * m_StretchZ,  TILE_SIZE * CHUNK_SIZE_Z);
    for (short x = 0; x < CHUNK_SUM_X; ++x)
    {
        for (short y = 0; y < CHUNK_SUM_Z; ++y)
        {
            m_mapchunk[x][y].Create(x, y, m_TileTextureSize);
        }
    }
    delete TileChunk::m_bounds;
#ifdef LOG_TIMING

    Logger::log().info() << "Time to create Chunks: " << clock()-time << " ms";
#endif
}

///================================================================================================
/// Change all Chunks.
///================================================================================================
void TileManager::ChangeChunks()
{
#ifdef LOG_TIMING
    long time = clock();
#endif

    TileChunk::m_TileManagerPtr = this;
    TileChunk::m_bounds = new AxisAlignedBox(
                              -TILE_SIZE * CHUNK_SIZE_X, 0               , -TILE_SIZE * CHUNK_SIZE_Z,
                              TILE_SIZE * CHUNK_SIZE_X, 100 * m_StretchZ,  TILE_SIZE * CHUNK_SIZE_Z);
/*
    // Test start
    unsigned char value;
    for (int a = 0 ; a < TILES_SUM_X; ++a)
    {
        for (int b = 0; b < TILES_SUM_Z; ++b)
        {
            value = Get_Map_Height(a, b)+ 1;
            if (value > 220)
                value = 0;
            Set_Map_Height(a, b, value);
        }
    }
    // Test stop
*/
    Set_Map_Textures();
    for (short x = 0; x < CHUNK_SUM_X; ++x)
    {
        for (short y = 0; y < CHUNK_SUM_Z; ++y)
        {
            m_mapchunk[x][y].Change();
        }
    }
    delete TileChunk::m_bounds;

#ifdef LOG_TIMING

    Logger::log().info() << "Time to change Chunks: " << clock()-time << " ms";
#endif
}

///================================================================================================
/// Change Tile and Environmet textures.
///================================================================================================
void TileManager::ChangeTexture()
{
    static bool once = false;
    const std::string strFilename = "terrain_032_texture.png";
    if (once)
        return;
    else
        once=true;
#ifdef LOG_TIMING

    long time = clock();
#endif

    Image tMap;
    if (!LoadImage(tMap, strFilename))
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

    Logger::log().info() << "Time to change Texture: " << clock()-time << " ms";
#endif
}

///================================================================================================
/// +/- 5 Chunks around the camera are drawn in high quality.
///================================================================================================
void TileManager::ControlChunks(Vector3 vector)
{
    // only for testing...
    if (Option::getSingleton().getIntValue(Option::CMDLINE_FALLBACK))
        return;

    /// ////////////////////////////////////////////////////////////////////
    /// Just for testing...
    /// ////////////////////////////////////////////////////////////////////
    // ChangeChunks();

    int x = (int)vector.x / (TILE_SIZE * CHUNK_SIZE_X)+1;
    int y = (int)vector.z / (TILE_SIZE * CHUNK_SIZE_Z)+1;
    if ( x > CHUNK_SUM_X || y > CHUNK_SUM_Z)
    {
        return;
    }

    for(int cx = 0; cx < CHUNK_SUM_X; ++cx)
    {
        for (int cy = 0; cy < CHUNK_SUM_Z; ++cy)
        {
#ifndef LOW_QUALITY_RENDERING
            if (cx >= x - HIGH_QUALITY_RANGE && cx <= x + HIGH_QUALITY_RANGE
                    && cy >= y - HIGH_QUALITY_RANGE && cy <= y + HIGH_QUALITY_RANGE)
            {
                m_mapchunk[cx][cy].Attach(QUALITY_HIGH);
            }
            else
#endif

            {
                m_mapchunk[cx][cy].Attach(QUALITY_LOW);
            }
        }
    }
}

///================================================================================================
/// If the file exists load it into an image.
///================================================================================================
bool TileManager::LoadImage(Image &image, const std::string &strFilename)
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

///================================================================================================
/// Create the texture-file out of the single textures + filter texture.
///================================================================================================
bool TileManager::CreateTextureGroup(const std::string &terrain_type)
{
    std::string strFilename;
    /// ////////////////////////////////////////////////////////////////////
    /// Create grid texture.
    /// ////////////////////////////////////////////////////////////////////
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
    /// ////////////////////////////////////////////////////////////////////
    /// Shrink all filter-textures.
    /// ////////////////////////////////////////////////////////////////////
    shrinkFilter();
    /// ////////////////////////////////////////////////////////////////////
    /// Shrink all tile-textures.
    /// ////////////////////////////////////////////////////////////////////
    shrinkTexture(terrain_type);
    /// ////////////////////////////////////////////////////////////////////
    /// Create group-texture in various sizes.
    /// ////////////////////////////////////////////////////////////////////
#ifdef LOG_TIMING

    long time = clock();
#endif

    int i, tx, ty;
    int pix = PIXEL_PER_TILE;
    uchar* TextureGroup_data;
    Image Filter, Texture, TextureGroup;
    while (pix >= MIN_TEXTURE_PIXEL)
    {
        TextureGroup_data = new uchar[PIXEL_PER_ROW * PIXEL_PER_ROW *4];
        TextureGroup.loadDynamicImage(TextureGroup_data, pix * 8, pix * 8,PF_A8B8G8R8);
        strFilename = "filter_" + StringConverter::toString(pix/2, 3, '0') + ".png";
        if (!LoadImage(Filter, strFilename))
        {
            Logger::log().error() << "Filter texture '" << strFilename << "' was not found.";
            return false;
        }
        uchar* Filter_data = Filter.getData();
        i =-1;
        tx = 0;
        ty = 0;
        while(1)
        {
            strFilename = terrain_type;
            strFilename+= "_"+ StringConverter::toString(++i, 2, '0');
            strFilename+= "_"+ StringConverter::toString(pix, 3, '0') + ".png";
            if (!LoadImage(Texture, strFilename))
                break;
            addToGroupTexture(TextureGroup_data, Filter_data, &Texture, pix, tx, ty);
            if (++tx == TEXTURES_PER_ROW)
            {
                if (++ty == TEXTURES_PER_ROW)
                    break;
                tx = 0;
            }
        }
        CreateTextureGroupBorders(TextureGroup_data, pix);
        strFilename = PATH_TILE_TEXTURES + terrain_type + "_texture";
        strFilename+= "_"+ StringConverter::toString(pix, 3, '0')+".png";
        TextureGroup.save(strFilename);

        delete[] TextureGroup_data;
        pix /= 2;
    }
#ifdef LOG_TIMING
    Logger::log().info() << "Time to Create Texture-Groups: " << clock()-time << " ms";
#endif

    delete[] grid_data;
    return true;
}

///================================================================================================
/// Create MipMaps for tile textures
///================================================================================================
void TileManager::CreateMipMaps(const std::string &terrain_type, int pixel)
{
    /*
        // Load tile texture images in all resolutions
        enum
        {
            PIX_032, PIX_064, PIX_128, PIX_SUM
        };
        Image TileImage[PIX_SUM];
        std::string strFilename;

        if (!mTileTexture.isNull()) TextureManager::getSingleton().remove((ResourcePtr&)mTileTexture);

        /// Load all tile textures-sets.
        for (int i = 0; i < PIX_SUM; ++i)
        {
            strFilename = terrain_type;
            strFilename+= "_texture_"+ StringConverter::toString((int)pow(2.0, i+4), 3, '0') + ".png";
            if (!LoadImage(TileImage[i], strFilename))
            {
                Logger::log().error() << "Ground texture '" << strFilename << "' was not found.";
                return;
            }
        }
        if (pixel == 128)
        {
            /// create mipmaps for 128x128 pixel textures
            mTileTexture = TextureManager::getSingleton().createManual(terrain_type+"_texture_128.png",
                          "General",TEX_TYPE_2D, 1024,1024, 2, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
            mTileTexture->getBuffer(0,0)->blitFromMemory(TileImage[PIX_128].getPixelBox());
            mTileTexture->getBuffer(0,1)->blitFromMemory(TileImage[PIX_064].getPixelBox());
            mTileTexture->getBuffer(0,2)->blitFromMemory(TileImage[PIX_032].getPixelBox());
        }
        else if (pixel == 64)
        {
            /// create mipmaps for 64x64 pixel textures
            mTileTexture = TextureManager::getSingleton().createManual(terrain_type+"_texture_064.png",
                          "General",TEX_TYPE_2D, 512, 512, 1, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
            mTileTexture->getBuffer(0,0)->blitFromMemory(TileImage[PIX_064].getPixelBox());
            mTileTexture->getBuffer(0,1)->blitFromMemory(TileImage[PIX_032].getPixelBox());
        }
        else if (pixel == 32)
        {
            /// create mipmaps for 32x32 pixel textures
            mTileTexture = TextureManager::getSingleton().createManual(terrain_type+"_texture_032.png",
                          "General",TEX_TYPE_2D, 256, 256, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
            mTileTexture->getBuffer(0,0)->blitFromMemory(TileImage[PIX_032].getPixelBox());
        }
        else
        {
            /// create mipmaps for 32x32 pixel textures
            mTileTexture = TextureManager::getSingleton().createManual(terrain_type+"_texture_016.png",
                          "General",TEX_TYPE_2D, 128, 128, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
            mTileTexture->getBuffer(0,0)->blitFromMemory(TileImage[PIX_032].getPixelBox());
        }
      */
}

///================================================================================================
/// Shrink Texture.
///================================================================================================
void TileManager::shrinkTexture(const std::string &terrain_type)
{
    int sum=0, pix = PIXEL_PER_TILE / 2;
    Image image;
    std::string strFilename;
    while (pix >= MIN_TEXTURE_PIXEL)
    {
        sum =0;
        while(1)
        {
            /// Load the Image.
            strFilename = terrain_type;
            strFilename+= "_"+ StringConverter::toString(sum, 2, '0');
            strFilename+= "_"+ StringConverter::toString(pix+pix, 3, '0') + ".png";
            if (!LoadImage(image, strFilename))
                break;
            /// Resize the Image.
            image.resize((Ogre::ushort)image.getWidth()/2, (Ogre::ushort)image.getHeight()/2, Image::FILTER_BILINEAR);
            /// Save the Image.
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

///================================================================================================
/// Shrink the filter.
///================================================================================================
void TileManager::shrinkFilter()
{
    int pix = PIXEL_PER_TILE / 4;
    Image image;
    std::string strFilename;
    while (pix > MIN_TEXTURE_PIXEL/4)
    {
        strFilename = "filter_" + StringConverter::toString(pix+pix, 3, '0') + ".png";
        if (!LoadImage(image, strFilename))
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

///================================================================================================
/// Add a texture to the terrain-texture.
///================================================================================================
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
    /// ////////////////////////////////////////////////////////////////////
    /// Copy the tile into the texture.
    /// ////////////////////////////////////////////////////////////////////
    /// Group-texture : 32 bit (RGBA)
    /// Tile-texture  : 24 bit (RGB)
    /// Filter-texture: 24 bit (RGB)
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
            TextureGroup_data[  index1                ] = Texture_data[  index2                ]; /// Top Left  subTile.
            TextureGroup_data[  index1+dstOffX        ] = Texture_data[  index2+srcOffX        ]; /// Top Right subTile.
            TextureGroup_data[  index1+dstOffY        ] = Texture_data[  index2+srcOffY        ]; /// Bot Left subTile.
            TextureGroup_data[  index1+dstOffX+dstOffY] = Texture_data[  index2+srcOffX+srcOffY]; /// Bot Righr subTile.

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

///================================================================================================
/// .
///================================================================================================
inline void TileManager::CreateTextureGroupBorders(uchar* TextureGroup_data, short pix)
{
    if (pix <= MIN_TEXTURE_PIXEL)
        return;
    /// ////////////////////////////////////////////////////////////////////
    /// Vertical border creation
    /// ////////////////////////////////////////////////////////////////////
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
    /// ////////////////////////////////////////////////////////////////////
    /// Horizontal border creation
    /// ////////////////////////////////////////////////////////////////////
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

///================================================================================================
/// Toggle Material.
///================================================================================================
void TileManager::setMaterialLOD(int pixel)
{
    if (pixel != 128 && pixel !=64 && pixel != 32 && pixel != 16)
        return;

    std::string matWater, matLand;
    int lod;
    m_TileTextureSize = pixel;
    if (pixel >16)
    {
        matLand = "Land_HighDetails" + StringConverter::toString(m_TileTextureSize, 3, '0');
        matWater= "Water_HighDetails";
        lod = QUALITY_HIGH;
    }
    else
    {
        matLand = "Land_LowDetails";
        matWater= "Water_LowDetails";
        lod = QUALITY_LOW;
    }
    if (mGrid)
    {
        matLand +="_Grid";
        matWater+="_Grid";
    }
    CreateMipMaps("terrain", m_TileTextureSize);
    for (int x = 0; x < CHUNK_SUM_X; ++x)
    {
        for (int y = 0; y < CHUNK_SUM_Z; ++y)
        {
            if (m_mapchunk[x][y].Get_Land_entity() ) m_mapchunk[x][y].Get_Land_entity() ->setMaterialName(matLand);
            if (m_mapchunk[x][y].Get_Water_entity()) m_mapchunk[x][y].Get_Water_entity()->setMaterialName(matWater);
            m_mapchunk[x][y].Attach(lod);
        }
    }
}

///================================================================================================
/// Switch Grid.
///================================================================================================
void TileManager::ToggleGrid()
{
    mGrid = !mGrid;
    setMaterialLOD(m_TileTextureSize);
}

