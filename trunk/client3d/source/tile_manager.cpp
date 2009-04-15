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
#include "define.h"
#include "tile_chunk.h"
#include "tile_manager.h"

using namespace Ogre;

//#define LOG_TIMING

Ogre::SceneManager *TileManager::mSceneManager = 0;

//////// Only for TESTING
#include <stdio.h>
void TileManager::loadLvl()
{
    FILE *stream = fopen("client3d.lvl", "rb");
    fread(&mMap, sizeof(mMap), 1, stream);
    fclose(stream);
    changeChunks();
}

void TileManager::saveLvl()
{
    FILE *stream = fopen("client3d.lvl", "wb");
    fwrite(&mMap, sizeof(mMap), 1, stream);
    fclose(stream);
}
//////////////////////////

//================================================================================================
// Free all resources.
//================================================================================================
void TileManager::freeRecources()
{
    mMapchunk.freeRecources();
    if (mSceneManager)
        mSceneManager->destroyQuery(mRaySceneQuery);
}

//================================================================================================
// Init the TileEngine.
//================================================================================================
void TileManager::Init(SceneManager* SceneMgr, int queryMaskLand, int queryMaskWater, int lod, bool createAtlas)
{
    Logger::log().headline() << "Init TileEngine";
    mShowGrid = false;
    mQueryMaskLand = queryMaskLand;
    mEditorActSelectedGfx = 0;
    mMapScrollX = mMapScrollZ = 0;
    mSelectedVertexX = mSelectedVertexZ = 0; // Tile picking.
    mSceneManager = SceneMgr;
    mRaySceneQuery = mSceneManager->createRayQuery(Ray());
    mLod = lod&3;
    //mLod = 3;
    int textureSize = MAX_TEXTURE_SIZE >> mLod;
    Logger::log().info() << "Setting LoD to " << mLod << ". Atlas size is " << textureSize << "x" << textureSize<< ".";
    if (createAtlas)
    {
        Logger::log().info() << "Creating atlas-texture...";
        createFilterTemplate();
        createAtlasTexture(MAX_TEXTURE_SIZE, true);
        Logger::log().success(true);
    }
    Logger::log().info() << "Creating tile chunk...";
    mMapchunk.init(textureSize, queryMaskLand, queryMaskWater);
    Logger::log().success(true);
    Logger::log().info() << "Init done.";
}

//================================================================================================
// Return the shadow number.
//================================================================================================
int TileManager::getMapShadow(unsigned int x, unsigned int z)
{
    if (!mShowGrid) return mMap[x][z].shadow;
    return SHADOW_GRID + ((x&1)?SHADOW_MIRROX_X:0) + ((z&1)?0:SHADOW_MIRROX_Z);
}

//================================================================================================
// Returns the gfx number of the shadow.
// This MUST be done in the editor. Its only here because of testing...
//================================================================================================
unsigned short TileManager::calcShadow(int x, int z)
{


    return 0 + SHADOW_MIRROX_X; // delete me! (Mirror test)


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

    // ////////////////////////////////////////////////////////////////////
    // No shadow.
    // ////////////////////////////////////////////////////////////////////
    return 66;
}

//================================================================================================
// Copy a shadow into the atlastexture.
//================================================================================================
void TileManager::copyShadowToAtlas(uchar *dstBuf)
{
    unsigned int size = MAX_TEXTURE_SIZE/32;
    unsigned int fixFilterSize = size/16;
    uint32 *dst, *src;
    Image srcImage;
    String srcFilename;
    int sumImages = 0;
    for (int y = 0; y < 15; ++y)
    {
        dst = ((uint32*)dstBuf) + y*size*2*MAX_TEXTURE_SIZE + size*3/2*MAX_TEXTURE_SIZE + MAX_TEXTURE_SIZE/2+size*3/2;
        for (int x = 0; x < 7; ++x)
        {
            srcFilename = "shadow_" + StringConverter::toString(sumImages++, 3, '0') + ".png";
            if (!loadImage(srcImage, srcFilename)) return;
            if ((srcImage.getWidth() != size-2*fixFilterSize) || (srcImage.getHeight() != size-2*fixFilterSize))
            {
                Logger::log().error() << "Gfx " << srcFilename << " has the wrong size! Only "
                << size-2*fixFilterSize << "x" << size-2*fixFilterSize << " is supported";
                return;
            }
            if (srcImage.getFormat()!=PF_A8R8G8B8)
            {
                Logger::log().error() << "Gfx " << srcFilename << " has the wrong pixelformat. Only A8R8G8B8 is supported for shadows.";
                return;
            }
            src= (uint32*)srcImage.getData();
            for (unsigned int posY = 0; posY < size; ++posY)
            {
                if (posY == size-fixFilterSize) src-= size-2*fixFilterSize;
                for (unsigned int posX = 0; posX < size; ++posX)
                {
                    dst[posY*MAX_TEXTURE_SIZE + posX] = *src;
                    if (posX >= fixFilterSize && posX < size-fixFilterSize-1) ++src;
                }
                ++src;
                if (posY < fixFilterSize || posY >= size-fixFilterSize) src-= size-2*fixFilterSize;
            }
            dst+= 2*size;
        }
    }
}

//================================================================================================
// Collect all tiles and filters into a single RGBA-image.
//================================================================================================
void TileManager::createAtlasTexture(int textureSize, bool fixFilteringErrors, unsigned int startGroup)
{
    int stopGroup = startGroup+1;
    if (startGroup >= (unsigned int) MAX_MAP_SETS)
    {
        startGroup = 0;
        stopGroup  = MAX_MAP_SETS;
    }
    Image dstImage;
    uchar *dstBuf = new uchar[textureSize * textureSize * RGBA];
    for (int nr = startGroup; nr < stopGroup; ++nr)
    {
        if (!copyTileToAtlas(dstBuf)) break;
        copyFilterToAtlas(dstBuf);
        copyShadowToAtlas(dstBuf);
        // Save the Atlastexture.
        dstImage.loadDynamicImage(dstBuf, textureSize, textureSize, 1, PF_A8R8G8B8, true);
        String dstFilename = PATH_GFX_TILES;
        dstFilename+= "Atlas_"+ StringConverter::toString(nr,2,'0') + "_";
        for (unsigned short s = textureSize; s >= textureSize/4; s/=2)
        {
            dstImage.save(dstFilename + StringConverter::toString(s, 4, '0') + ".png");
            dstImage.resize(s/2, s/2, Image::FILTER_BILINEAR);
        }
    }
    //delete[] dstBuf; // Will be done by Ogre because autoDelete was set.
}

//================================================================================================
// Copy a tile into the color part of the atlastexture.
//================================================================================================
bool TileManager::copyTileToAtlas(uchar *dstBuf)
{
    static int nr = -1;
    unsigned int tileSize = MAX_TEXTURE_SIZE / COLS_SRC_TILES;
    unsigned int subSize = tileSize/2;
    int pixelSize, sumImages= 0;
    Image srcImage;
    String srcFilename;
    ++nr;
    for (int y = 0; y < COLS_SRC_TILES; ++y)
    {
        for (int x = 0; x <= 2; x+=2)
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

            bool srcHasAlpha = srcImage.getFormat()==PF_A8R8G8B8;
            pixelSize = (srcImage.getFormat()==PF_A8R8G8B8)?RGBA:RGB;
            uchar *src = srcImage.getData();
            uchar *dst = dstBuf + (y*tileSize*MAX_TEXTURE_SIZE + x*tileSize)*RGBA;
            // ////////////////////////////////////////////////////////////////////
            // Texture unit 0
            // ////////////////////////////////////////////////////////////////////
            copySubTile(src, 0, 0, dst + 0*tileSize*RGBA, 0, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 0, dst + 0*tileSize*RGBA, 1, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 0, dst + 0*tileSize*RGBA, 2, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 0, dst + 0*tileSize*RGBA, 3, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 1, dst + 0*tileSize*RGBA, 0, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 1, dst + 0*tileSize*RGBA, 1, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 1, dst + 0*tileSize*RGBA, 2, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 1, dst + 0*tileSize*RGBA, 3, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 2, dst + 0*tileSize*RGBA, 0, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 2, dst + 0*tileSize*RGBA, 1, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 2, dst + 0*tileSize*RGBA, 2, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 2, dst + 0*tileSize*RGBA, 3, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 3, dst + 0*tileSize*RGBA, 0, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 3, dst + 0*tileSize*RGBA, 1, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 3, dst + 0*tileSize*RGBA, 2, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 3, dst + 0*tileSize*RGBA, 3, 3, subSize/2, srcHasAlpha);
            // ////////////////////////////////////////////////////////////////////
            // Texture unit 1
            // ////////////////////////////////////////////////////////////////////
            copySubTile(src, 0, 0, dst + 1*tileSize*RGBA, 1, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 0, dst + 1*tileSize*RGBA, 2, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 0, dst + 1*tileSize*RGBA, 3, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 0, dst + 1*tileSize*RGBA, 0, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 1, dst + 1*tileSize*RGBA, 1, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 1, dst + 1*tileSize*RGBA, 2, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 1, dst + 1*tileSize*RGBA, 3, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 1, dst + 1*tileSize*RGBA, 0, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 2, dst + 1*tileSize*RGBA, 1, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 2, dst + 1*tileSize*RGBA, 2, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 2, dst + 1*tileSize*RGBA, 3, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 2, dst + 1*tileSize*RGBA, 0, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 3, dst + 1*tileSize*RGBA, 1, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 3, dst + 1*tileSize*RGBA, 2, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 3, dst + 1*tileSize*RGBA, 3, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 3, dst + 1*tileSize*RGBA, 0, 0, subSize/2, srcHasAlpha);
            // ////////////////////////////////////////////////////////////////////
            // Texture unit 2.1 (Horizontal filters)
            // ////////////////////////////////////////////////////////////////////
            copySubTile(src, 0, 0, dst + 4*tileSize*RGBA, 0, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 0, dst + 4*tileSize*RGBA, 1, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 0, dst + 4*tileSize*RGBA, 2, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 0, dst + 4*tileSize*RGBA, 3, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 1, dst + 4*tileSize*RGBA, 0, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 1, dst + 4*tileSize*RGBA, 1, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 1, dst + 4*tileSize*RGBA, 2, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 1, dst + 4*tileSize*RGBA, 3, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 2, dst + 4*tileSize*RGBA, 0, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 2, dst + 4*tileSize*RGBA, 1, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 2, dst + 4*tileSize*RGBA, 2, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 2, dst + 4*tileSize*RGBA, 3, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 3, dst + 4*tileSize*RGBA, 0, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 3, dst + 4*tileSize*RGBA, 1, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 3, dst + 4*tileSize*RGBA, 2, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 3, dst + 4*tileSize*RGBA, 3, 0, subSize/2, srcHasAlpha);
            // ////////////////////////////////////////////////////////////////////
            // Texture unit 2.2 (Vertical filters)
            // ////////////////////////////////////////////////////////////////////
            copySubTile(src, 0, 0, dst + 5*tileSize*RGBA, 1, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 0, dst + 5*tileSize*RGBA, 2, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 0, dst + 5*tileSize*RGBA, 3, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 0, dst + 5*tileSize*RGBA, 0, 0, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 1, dst + 5*tileSize*RGBA, 1, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 1, dst + 5*tileSize*RGBA, 2, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 1, dst + 5*tileSize*RGBA, 3, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 1, dst + 5*tileSize*RGBA, 0, 1, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 2, dst + 5*tileSize*RGBA, 1, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 2, dst + 5*tileSize*RGBA, 2, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 2, dst + 5*tileSize*RGBA, 3, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 2, dst + 5*tileSize*RGBA, 0, 2, subSize/2, srcHasAlpha);
            copySubTile(src, 0, 3, dst + 5*tileSize*RGBA, 1, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 1, 3, dst + 5*tileSize*RGBA, 2, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 2, 3, dst + 5*tileSize*RGBA, 3, 3, subSize/2, srcHasAlpha);
            copySubTile(src, 3, 3, dst + 5*tileSize*RGBA, 0, 3, subSize/2, srcHasAlpha);
        }
    }
    return true;
}

//================================================================================================
//
//================================================================================================
void TileManager::copySubTile(uchar* src, int srcX, int srcY, uchar *dst, int dstX, int dstY, int size, bool alpha)
{
    int alphaPixel = alpha?RGBA:RGB;
    src+= (srcY*size*size*4 + srcX*size) *alphaPixel;
    dst+= (MAX_TEXTURE_SIZE * size * dstY + size*dstX) * RGBA;
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = (alpha)?*src++:0xff;;
        }
        src+= size*3*alphaPixel;
        dst+= (MAX_TEXTURE_SIZE-size) * RGBA;
    }
}


//================================================================================================
// Copy a terrain-filter/shadow-filter into the alpha part of the atlastexture.
//================================================================================================
void TileManager::copyFilterToAtlas(uchar *dstBuf)
{
    int sumImages = 0;
    unsigned int size = MAX_TEXTURE_SIZE / COLS_SRC_TILES * 2;
    Image srcImage;
    String srcFilename;
    for (int y = 0; y < COLS_SRC_TILES; ++y)
    {
        for (int x = 0; x < 2; ++x)
        {
            srcFilename = "filter_" + StringConverter::toString(sumImages++, 2, '0') + ".png";
            if (!loadImage(srcImage, srcFilename))
            {
                // Filter was not found, so we use the default filter.
                if (!loadImage(srcImage, "filter_00.png"))
                {
                    Logger::log().error() << "The default tile-filter (filter_00.png) was not found.";
                    return;
                }
            }
            if ((srcImage.getWidth() != size) || (srcImage.getHeight() != size))
            {
                Logger::log().error() << "Gfx " << srcFilename << " has the wrong size! Only " << size << "x" << size << " is supported.";
                return;
            }
            if (srcImage.getFormat()!=PF_R8G8B8)
            {
                Logger::log().error() << "Gfx " << srcFilename << " has the wrong pixelformat. Only RGB is supported for filters.";
                return;
            }
            uchar *src = srcImage.getData();
            uchar *dst = dstBuf+(y*size/2*MAX_TEXTURE_SIZE + x *size) * RGBA;
            for (unsigned int posY = 0; posY < size/2; ++posY)
            {
                for (unsigned int posX = 0; posX < size; ++posX)
                {
                    dst[0*size*RGBA+3] = *src; // alpha part of the atlastexture = red part of the filter.
                    dst[2*size*RGBA+3] = src[size/2*size*RGB];
                    dst+=RGBA;
                    src+=RGB;
                }
                dst+= (MAX_TEXTURE_SIZE-size)*RGBA;
            }
        }
    }
}

//================================================================================================
// Load an existing image.
//================================================================================================
bool TileManager::loadImage(Image &image, const Ogre::String &strFilename)
{
    std::string strTemp = PATH_GFX_TILES + strFilename;
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
    mRaySceneQuery->setQueryMask(mQueryMaskLand);
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
    mMapchunk.update();
}

//================================================================================================
// .
//================================================================================================
void TileManager::updateTileHeight(int deltaHeight)
{
    mMap[mSelectedVertexX][mSelectedVertexZ].height+= deltaHeight;
    mMapchunk.update();
    highlightVertex(mSelectedVertexX, mSelectedVertexZ);
}

//================================================================================================
// .
//================================================================================================
void TileManager::updateTileGfx(int deltaGfxNr)
{
    mMap[mSelectedVertexX][mSelectedVertexZ].gfx+= deltaGfxNr;
    mEditorActSelectedGfx = mMap[mSelectedVertexX][mSelectedVertexZ].gfx;
    mMapchunk.update();
    highlightVertex(mSelectedVertexX, mSelectedVertexZ);
}

//================================================================================================
// .
//================================================================================================
void TileManager::setTileGfx()
{
    mMap[mSelectedVertexX][mSelectedVertexZ].gfx = mEditorActSelectedGfx;
    mMapchunk.update();
}

//================================================================================================
// Set the values for a map position.
//================================================================================================
void TileManager::setMap(unsigned int x, unsigned int y, short height, char gfx, char shadow)
{
    mMap[x][y].height = height *10;
    mMap[x][y].gfx    = gfx;
    mMap[x][y].shadow = shadow;
}

//================================================================================================
// Change all Chunks.
//================================================================================================
void TileManager::changeChunks()
{
    unsigned long time = Root::getSingleton().getTimer()->getMicroseconds();
    // Shadow calculation is part of the editor. This is just for testing here.
    {
        for (int z = 0; z <= CHUNK_SIZE_Z; ++z)
            for (int x = 0; x <= CHUNK_SIZE_X; ++x)
                mMap[x][z].shadow = calcShadow(x, z);
    }
    mMapchunk.update();
    Logger::log().error() << "Time to change terrain: " << (double)(Root::getSingleton().getTimer()->getMicroseconds() - time)/1000 << " ms";
}

//================================================================================================
// Change Tile and Environmet textures.
//================================================================================================
void TileManager::changeMapset(int landGroup, int waterGroup)
{
    mMapchunk.loadAtlasTexture(landGroup, waterGroup);
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

//================================================================================================
// Create a template for the filter.
//================================================================================================
void TileManager::createFilterTemplate()
{
    int size = MAX_TEXTURE_SIZE/4;
    int lineSkip = size * RGB;
    const int UNUSED_SIZE = 16;
    uchar *dstBuf = new uchar[size * size * RGB];
    memset(dstBuf, 0x00, size * size * RGB);
    Image dstImage;
    dstImage.loadDynamicImage(dstBuf, size, size, 1, PF_R8G8B8);
    // Horizontal borderlines
    for (int y= 0; y < size/2; y+= size/4)
    {
        for (int x = 0; x < size; ++x)
        {
            dstBuf[(y*size+x)*RGB +0] = 0xC6;
            dstBuf[(y*size+x)*RGB +1] = 0x38;
            dstBuf[(y*size+x)*RGB +2] = 0xDB;
        }
    }
    // vertical borderlines
    for (int x= 0; x < size; x+= size/4)
    {
        for (int y = 0; y < size/2; ++y)
        {
            dstBuf[(y*size+x)*RGB +0] = 0xC6;
            dstBuf[(y*size+x)*RGB +1] = 0x38;
            dstBuf[(y*size+x)*RGB +2] = 0xDB;
        }
    }
    // Horizontal borderlines for karos
    uchar *p = dstBuf + size/2*lineSkip;
    for (int i = 0; i < 2; ++i)
    {
        for (int height= 0; height < UNUSED_SIZE; ++height)
        {
            for (int x = 0; x < size/2; ++x)
            {
                // Top pos
                p[(height*size+x)*RGB +0] = 0xC6;
                p[(height*size+x)*RGB +1] = 0x38;
                p[(height*size+x)*RGB +2] = 0xDB;
                // Bottom pos
                p[((size/4 - height-1)*size+x)*RGB +0] = 0xC6;
                p[((size/4 - height-1)*size+x)*RGB +1] = 0x38;
                p[((size/4 - height-1)*size+x)*RGB +2] = 0xDB;
            }
        }
        p+= size/4*lineSkip;
    }
    // Vertical borderlines for karos
    p = dstBuf + size/2*lineSkip + size/2*RGB;
    for (int i = 0; i < 2; ++i)
    {
        for (int width= 0; width < UNUSED_SIZE; ++width)
        {
            for (int y = 0; y < size/2; ++y)
            {
                // Left pos
                p[(y*size + width)*RGB +0] = 0xC6;
                p[(y*size + width)*RGB +1] = 0x38;
                p[(y*size + width)*RGB +2] = 0xDB;
                // Right pos
                p[(y*size - width + size/4-1)*RGB +0] = 0xC6;
                p[(y*size - width + size/4-1)*RGB +1] = 0x38;
                p[(y*size - width + size/4-1)*RGB +2] = 0xDB;
            }
        }
        p+= size/4*RGB;
    }
    // Karos
    int offset = 0;
    p = dstBuf + lineSkip * size/2;
    size/=4;
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            if ((x == size/2 - offset))
                x+= 2*offset;
            for (int row = 0; row < 2; ++row)
            {
                for (int col = 0; col < 4; ++col)
                {
                    p[size*lineSkip*row + (x+col*size)*RGB+ 0] = 0xC6; // R
                    p[size*lineSkip*row + (x+col*size)*RGB+ 1] = 0x38; // G
                    p[size*lineSkip*row + (x+col*size)*RGB+ 2] = 0xDB; // B
                }
            }
        }
        if (y < size/2) ++offset; else --offset;
        p+=4*size*RGB;
    }
    String filename = PATH_GFX_TILES;
    filename+= "TemplateFilter.png";
    dstImage.save(filename);
    delete[] dstBuf;
}

