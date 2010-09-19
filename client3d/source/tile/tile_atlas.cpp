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
#include <OgreConfigFile.h>
#include "logger.h"
#include "profiler.h"
#include "tile/tile_atlas.h"

using namespace Ogre;

static const unsigned int RGB  = 3; ///< Pixelsize.
static const unsigned int RGB_A= 4; ///< Pixelsize.

//================================================================================================
// Collect all tiles and masks into a single RGB-image.
//================================================================================================
void TileAtlas::createAtlasTexture(String &filenamePrefix, uint32 startGroup)
{
    PROFILE()
    Logger::log().info() << Logger::ICON_CLIENT << "Creating atlas-texture...";
    // ////////////////////////////////////////////////////////////////////
    // Create the resource path.
    // ////////////////////////////////////////////////////////////////////
    if (!setResourcePath("tiles", mPathGfxTiles)) return;
    // ////////////////////////////////////////////////////////////////////
    // Check for the correct size of the tiles.
    // ////////////////////////////////////////////////////////////////////
    Image image;
    if (!loadImage(image, "terrain_00_00.png", false))
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Tile graphic 'terrain_00_00.png' was not found!";
        return;
    }
    mTileSize = image.getWidth();
    if ((mTileSize != image.getHeight()) || (mTileSize != 256 && mTileSize != 512))
    {
        Logger::log().error() << Logger::ICON_CLIENT << "The graphics for the tiles must have a size of 256x256 or 512x512 pixel.";
        return;
    }
    mBorderSize = mTileSize/32;
    mMaxTextureSize = mTileSize * (ATLAS_LAND_ROWS+1);
    // ////////////////////////////////////////////////////////////////////
    // Create the mask template.
    // ////////////////////////////////////////////////////////////////////
    createMaskTemplate();
    // ////////////////////////////////////////////////////////////////////
    // Create the atlas texture.
    // ////////////////////////////////////////////////////////////////////
    const uint32 MAX_MAP_SETS = 99; // 2 bytes are used in the filename, so max 99.
    int stopGroup = startGroup+1;
    if (startGroup >= MAX_MAP_SETS)
    {
        startGroup = 0;
        stopGroup  = MAX_MAP_SETS;
    }
    uchar *dstBuf = OGRE_ALLOC_T(uchar, mMaxTextureSize * mMaxTextureSize * RGB_A, MEMCATEGORY_GENERAL);
    for (int nr = startGroup; nr < stopGroup; ++nr)
    {
        if (!copyTileToAtlas(dstBuf)) break;
        copyFlowToAtlas(dstBuf);
        copySpotToAtlas(dstBuf);
        copyMaskToAtlas(dstBuf);
        // Save the Atlastexture.
        image.loadDynamicImage(dstBuf, mMaxTextureSize, mMaxTextureSize, 1, PF_A8R8G8B8, true);
        String dstFilename = mPathGfxTiles + filenamePrefix + "_" + StringConverter::toString(nr,2,'0') + "_";
        for (unsigned short s = mMaxTextureSize; s >= 512; s/=2)
        {
            image.save(dstFilename + StringConverter::toString(s, 4, '0') + ".png");
            image.resize(s/2, s/2, Image::FILTER_BILINEAR);
        }
    }
    //OGRE_FREE(data, MEMCATEGORY_GENERAL); // Will be done by Ogre because autoDelete was set.
}

//================================================================================================
// Copy a tile into the color part of the atlastexture.
//================================================================================================
bool TileAtlas::copyTileToAtlas(uchar *dstBuf)
{
    PROFILE()
    static int nr = -1;
    const unsigned int OFFSET = mBorderSize*2+mTileSize;
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
            if ((srcImage.getWidth() != mTileSize) || (srcImage.getHeight() != mTileSize))
            {
                Logger::log().error() << Logger::ICON_CLIENT << "Gfx " << srcFilename << " has the wrong size! Alle tiles must have the same size than terrain_00_00.png";
                return true;
            }
            int srcAlpha = (srcImage.getFormat()==PF_A8R8G8B8)?1:0; // Ignore alpha.
            uchar *src = srcImage.getData();
            uchar *dst = dstBuf + (y*OFFSET*mMaxTextureSize + x*OFFSET)*RGB_A;
            uchar *dst1= x?dst-mBorderSize*RGB_A:dst;
            for (unsigned int ty = 0; ty < mTileSize; ++ty)
            {
                // Tile
                for (unsigned int tx = 0; tx < mTileSize; ++tx)
                {
                    *dst++= *src++; // R
                    *dst++= *src++; // G
                    *dst++= *src++; // B
                    *dst++= 255;
                    src+= srcAlpha; // A
                }
                // Right and left mask borders.
                for (unsigned int tx = 0; tx < mBorderSize; ++tx)
                {
                    for (unsigned int color = 0; color < RGB_A; ++color)
                    {
                        *dst = *(dst-mTileSize*RGB_A); // Right mask border
                        if (x)
                            *(dst-(mTileSize+2*tx+1)*RGB_A) = *(dst-(2*tx+1)*RGB_A); // Left mask border
                        ++dst;
                    }
                }
                dst+=(mMaxTextureSize-mTileSize-mBorderSize)*RGB_A;
            }
            // Top and bottom mask borders.
            for (unsigned int ty = 0; ty < mBorderSize; ++ty)
            {
                maxX = x?OFFSET*RGB_A:(OFFSET-mBorderSize)*RGB_A;
                for (unsigned int tx = 0; tx < maxX; ++tx)
                    dst1[mTileSize*mMaxTextureSize*RGB_A+tx] = dst1[tx]; // Bottom mask border
                dst1+= mMaxTextureSize*RGB_A;
            }
            if (y)
            {
                dst1-= 2*mBorderSize*mMaxTextureSize*RGB_A;
                for (unsigned int ty = 0; ty < mBorderSize; ++ty)
                {
                    maxX = x?OFFSET*RGB_A:(OFFSET-mBorderSize)*RGB_A;
                    for (unsigned int tx = 0; tx < maxX; ++tx)
                        dst1[tx] = dst1[(mTileSize*mMaxTextureSize)*RGB_A+tx]; // Top mask border
                    dst1+= mMaxTextureSize*RGB_A;
                }
            }
        }
    }
    return true;
}

//================================================================================================
// Copy a terrain-mask into the atlastexture.
//================================================================================================
void TileAtlas::copyFlowToAtlas(uchar *dstBuf)
{
    PROFILE()
    static int nr = -1;
    Image srcImage;
    uchar *src, *src2, *dst;
    dstBuf+= ATLAS_LAND_ROWS*(mTileSize+2*mBorderSize) * mMaxTextureSize * RGB_A;
    ++nr;
    for (int sumFlowTiles = 0; sumFlowTiles < ATLAS_LAND_COLS/2; ++sumFlowTiles)
    {
        String srcFilename = "terrain_" + StringConverter::toString(nr, 2, '0') + "_F" + StringConverter::toString(sumFlowTiles) + ".png";
        if (!loadImage(srcImage, srcFilename, false))
        {
            // FlowTile was not found, so we use the default one.
            if (!loadImage(srcImage, "terrain_00_F0.png", false))
            {
                Logger::log().error() << Logger::ICON_CLIENT << "The default flow-tile (terrain_00_F0.png) was not found.";
                return;
            }
        }
        if ((srcImage.getWidth() != mTileSize) || (srcImage.getHeight() != mTileSize))
        {
            Logger::log().error() << Logger::ICON_CLIENT << "Gfx " << srcFilename << " has the wrong size! Alle tiles must have the same size than terrain_00_00.png";
            return;
        }
        if (srcImage.getFormat()!=PF_R8G8B8)
        {
            Logger::log().error() << Logger::ICON_CLIENT << "Gfx " << srcFilename << " has the wrong pixelformat. Only RGB is supported for tiles.";
            return;
        }
        // Left horizontal Border
        src = srcImage.getData() + (mTileSize-mBorderSize)*mTileSize * RGB;
        src2= srcImage.getData() + (mTileSize/2)*mTileSize * RGB;
        dst = dstBuf - mBorderSize*mMaxTextureSize*RGB_A;
        for (unsigned int y = 0; y < mBorderSize; ++y)
        {
            for (unsigned int x = 0; x < mTileSize; ++x)
            {
                dst[x*RGB_A+0] = src[x*RGB+0];
                dst[x*RGB_A+1] = src[x*RGB+1];
                dst[x*RGB_A+2] = src[x*RGB+2];
                dst[x*RGB_A+3] = 255;
                dst[((mTileSize/2+mBorderSize)*mMaxTextureSize + x)*RGB_A+0] = src2[x*RGB+0];
                dst[((mTileSize/2+mBorderSize)*mMaxTextureSize + x)*RGB_A+1] = src2[x*RGB+1];
                dst[((mTileSize/2+mBorderSize)*mMaxTextureSize + x)*RGB_A+2] = src2[x*RGB+2];
                dst[((mTileSize/2+mBorderSize)*mMaxTextureSize + x)*RGB_A+3] = 255;
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mTileSize*RGB;
            src2+= mTileSize*RGB;
        }
        // Right horizontal Border
        src = srcImage.getData() + (mTileSize/2-mBorderSize)*mTileSize * RGB;
        src2= srcImage.getData();
        dst = dstBuf - mBorderSize*mMaxTextureSize*RGB_A + (256+mBorderSize*2) * RGB_A;
        for (unsigned int y = 0; y < mBorderSize; ++y)
        {
            for (unsigned int x = 0; x < mTileSize; ++x)
            {
                dst[x*RGB_A+0] = src[x*RGB+0];
                dst[x*RGB_A+1] = src[x*RGB+1];
                dst[x*RGB_A+2] = src[x*RGB+2];
                dst[x*RGB_A+3] = 255;
                dst[((mTileSize/2+mBorderSize)*mMaxTextureSize + x)*RGB_A+0] = src2[x*RGB+0];
                dst[((mTileSize/2+mBorderSize)*mMaxTextureSize + x)*RGB_A+1] = src2[x*RGB+1];
                dst[((mTileSize/2+mBorderSize)*mMaxTextureSize + x)*RGB_A+2] = src2[x*RGB+2];
                dst[((mTileSize/2+mBorderSize)*mMaxTextureSize + x)*RGB_A+3] = 255;
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mTileSize*RGB;
            src2+= mTileSize*RGB;
        }
        // Upper Tile half.
        src = srcImage.getData();
        dst = dstBuf;
        for (unsigned int y = 0; y < mTileSize/2; ++y)
        {
            for (unsigned int x = 0; x < mTileSize; ++x)
            {
                dst[x*RGB_A+0] = src[x*RGB+0];
                dst[x*RGB_A+1] = src[x*RGB+1];
                dst[x*RGB_A+2] = src[x*RGB+2];
                dst[x*RGB_A+3] = 255;
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mTileSize*RGB;
        }
        // Lower Tile half.
        dst = dstBuf + (mTileSize+mBorderSize*2) * RGB_A;
        for (unsigned int y = 0; y < mTileSize/2; ++y)
        {
            for (unsigned int x = 0; x < mTileSize; ++x)
            {
                dst[x*RGB_A+0] = src[x*RGB+0];
                dst[x*RGB_A+1] = src[x*RGB+1];
                dst[x*RGB_A+2] = src[x*RGB+2];
                dst[x*RGB_A+3] = 255;
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mTileSize*RGB;
        }
        // Vertical Borders
        src = dstBuf - (mBorderSize*mMaxTextureSize) * RGB_A;
        dst = dstBuf - (mBorderSize*mMaxTextureSize + mBorderSize) * RGB_A;
        const unsigned int OFFSET = (mBorderSize*2+mTileSize)*RGB_A;
        for (unsigned int y = 0; y < mTileSize/2+2*mBorderSize; ++y)
        {
            for (unsigned int x = 0; x < mBorderSize; ++x)
            {
                if (sumFlowTiles)
                {
                    dst[x*RGB_A+0] = dst[(mTileSize+x)*RGB_A+0];
                    dst[x*RGB_A+1] = dst[(mTileSize+x)*RGB_A+1];
                    dst[x*RGB_A+2] = dst[(mTileSize+x)*RGB_A+2];
                    dst[x*RGB_A+3] = dst[(mTileSize+x)*RGB_A+3];
                }
                dst[x*RGB_A+OFFSET+0] = dst[(mTileSize+x)*RGB_A+OFFSET+0];
                dst[x*RGB_A+OFFSET+1] = dst[(mTileSize+x)*RGB_A+OFFSET+1];
                dst[x*RGB_A+OFFSET+2] = dst[(mTileSize+x)*RGB_A+OFFSET+2];
                dst[x*RGB_A+OFFSET+3] = dst[(mTileSize+x)*RGB_A+OFFSET+3];

                dst[(mTileSize+mBorderSize+ x)*RGB_A+0] = src[x*RGB_A+0];
                dst[(mTileSize+mBorderSize+ x)*RGB_A+1] = src[x*RGB_A+1];
                dst[(mTileSize+mBorderSize+ x)*RGB_A+2] = src[x*RGB_A+2];
                dst[(mTileSize+mBorderSize+ x)*RGB_A+3] = 255;
                dst[(mTileSize+mBorderSize+ x)*RGB_A+OFFSET+0] = src[x*RGB_A+OFFSET+0];
                dst[(mTileSize+mBorderSize+ x)*RGB_A+OFFSET+1] = src[x*RGB_A+OFFSET+1];
                dst[(mTileSize+mBorderSize+ x)*RGB_A+OFFSET+2] = src[x*RGB_A+OFFSET+2];
                dst[(mTileSize+mBorderSize+ x)*RGB_A+OFFSET+3] = 255;
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mMaxTextureSize*RGB_A;
        }
        dstBuf+= (2*mTileSize+4*mBorderSize) * RGB_A;
    }
}

//================================================================================================
// Copy a terrain-mask into the atlastexture.
//================================================================================================
void TileAtlas::copyMaskToAtlas(uchar *dstBuf)
{
    PROFILE()
    static int nr = -1;
    const unsigned int OFFSET = mBorderSize*2+mTileSize/2;
    Image srcImage;
    String srcFilename;
    uchar *src, *dst;
    dstBuf+= 6*(mBorderSize*2+mTileSize)* RGB_A;
    ++nr;
    for (int sumMask = 0; sumMask < ATLAS_LAND_ROWS; ++sumMask)
    {
        String srcFilename = "terrain_" + StringConverter::toString(nr, 2, '0') + "_M" + StringConverter::toString(sumMask) + ".png";
        if (!loadImage(srcImage, srcFilename, false))
        {
            // Mask was not found, so we use the default mask.
            if (!loadImage(srcImage, "terrain_00_M0.png", false))
            {
                Logger::log().error() << Logger::ICON_CLIENT << "The default tile-mask (terrain_00_M0.png) was not found.";
                return;
            }
        }
        if ((srcImage.getWidth() != mTileSize) || (srcImage.getHeight() != mTileSize))
        {
            Logger::log().error() << Logger::ICON_CLIENT << "Gfx " << srcFilename << " has the wrong size! Alle tiles must have the same size than terrain_00_00.png";
            return;
        }
        if (srcImage.getFormat()!=PF_R8G8B8)
        {
            Logger::log().error() << Logger::ICON_CLIENT << "Gfx " << srcFilename << " has the wrong pixelformat. Only RGB is supported for masks.";
            return;
        }
        // Erase the help lines from the mask template.
        src = srcImage.getData();
        for (unsigned int i = 0; i < mTileSize*mTileSize; ++i)
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
        for (unsigned int y = 0; y < mTileSize/2; ++y)
        {
            for (unsigned int x = 0; x < mTileSize/2; ++x)
            {
                dst[(                         2*OFFSET + x)*RGB_A+2] = src[x*RGB]; // Mask 2 - red.
                dst[(OFFSET*mMaxTextureSize + 0*OFFSET + x)*RGB_A+1] = src[x*RGB]; // Mask 3 - green.
                dst[(OFFSET*mMaxTextureSize + 1*OFFSET + x)*RGB_A+2] = src[x*RGB]; // Mask 4 - red.
                dst[(OFFSET*mMaxTextureSize + 2*OFFSET + x)*RGB_A+1] = src[x*RGB]; // Mask 5 - green.
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mTileSize*RGB;
        }
        // Copy mask 1 to the atlas-texture.
        src = srcImage.getData();
        dst = dstBuf;
        for (unsigned int y = 0; y < mTileSize/4; ++y)
        {
            for (unsigned int x = 0; x < mTileSize/4; ++x)
            {
                dst[(x                                         )*RGB_A+2] = src[(64*mTileSize + 192 + x)*RGB]; // R (1 of 4)
                dst[(x+mTileSize/4                             )*RGB_A+2] = src[(64*mTileSize + 128 + x)*RGB]; // R (2 of 4)
                dst[(x             +mTileSize/4*mMaxTextureSize)*RGB_A+2] = src[(               192 + x)*RGB]; // R (1 of 4)
                dst[(x+mTileSize/4 +mTileSize/4*mMaxTextureSize)*RGB_A+2] = src[(               128 + x)*RGB]; // R (2 of 4)
                dst[(1*OFFSET +x                                         )*RGB_A+1] = src[(64*mTileSize + 192 + x)*RGB]; // G (1 of 4)
                dst[(1*OFFSET +x+mTileSize/4                             )*RGB_A+1] = src[(64*mTileSize + 128 + x)*RGB]; // G (2 of 4)
                dst[(1*OFFSET +x             +mTileSize/4*mMaxTextureSize)*RGB_A+1] = src[(               192 + x)*RGB]; // G (1 of 4)
                dst[(1*OFFSET +x+mTileSize/4 +mTileSize/4*mMaxTextureSize)*RGB_A+1] = src[(               128 + x)*RGB]; // G (2 of 4)

                dst[(OFFSET*mMaxTextureSize+ 1*OFFSET +x                                        )*RGB_A+1] = src[(64*mTileSize + 192 + x)*RGB]; // G (1 of 4)
                dst[(OFFSET*mMaxTextureSize+ 1*OFFSET +x+mTileSize/4                            )*RGB_A+1] = src[(64*mTileSize + 128 + x)*RGB]; // G (2 of 4)
                dst[(OFFSET*mMaxTextureSize+ 1*OFFSET +x            +mTileSize/4*mMaxTextureSize)*RGB_A+1] = src[(               192 + x)*RGB]; // G (1 of 4)
                dst[(OFFSET*mMaxTextureSize+ 1*OFFSET +x+mTileSize/4+mTileSize/4*mMaxTextureSize)*RGB_A+1] = src[(               128 + x)*RGB]; // G (2 of 4)

                dst[(OFFSET*mMaxTextureSize+ 2*OFFSET +x                                        )*RGB_A+2] = src[(64*mTileSize + 192 + x)*RGB]; // R (1 of 4)
                dst[(OFFSET*mMaxTextureSize+ 2*OFFSET +x+mTileSize/4                            )*RGB_A+2] = src[(64*mTileSize + 128 + x)*RGB]; // R (2 of 4)
                dst[(OFFSET*mMaxTextureSize+ 2*OFFSET +x            +mTileSize/4*mMaxTextureSize)*RGB_A+2] = src[(               192 + x)*RGB]; // R (1 of 4)
                dst[(OFFSET*mMaxTextureSize+ 2*OFFSET +x+mTileSize/4+mTileSize/4*mMaxTextureSize)*RGB_A+2] = src[(               128 + x)*RGB]; // R (2 of 4)
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mTileSize*RGB;
        }
        // Copy mask 2 (horizontal) to the atlas-texture.
        src = srcImage.getData();
        dst = dstBuf;
        for (unsigned int y = 0; y < mTileSize/4; ++y)
        {
            for (unsigned int x = 0; x < mTileSize/2; ++x)
            {
                dst[(                                                    x)*RGB_A+1] = src[(192*mTileSize + x)*RGB]; // G (1 of 4)
                dst[(                       mTileSize/4*mMaxTextureSize +x)*RGB_A+1] = src[(128*mTileSize + x)*RGB]; // G (1 of 4)
                dst[(OFFSET                                             +x)*RGB_A+2] = src[(192*mTileSize + x)*RGB]; // R (1 of 4)
                dst[(OFFSET                +mTileSize/4*mMaxTextureSize +x)*RGB_A+2] = src[(128*mTileSize + x)*RGB]; // R (1 of 4)
                dst[(OFFSET*2                                           +x)*RGB_A+1] = src[(192*mTileSize + x)*RGB]; // G (1 of 4)
                dst[(OFFSET*2              +mTileSize/4*mMaxTextureSize +x)*RGB_A+1] = src[(128*mTileSize + x)*RGB]; // G (1 of 4)
                dst[(OFFSET*mMaxTextureSize                             +x)*RGB_A+2] = src[(192*mTileSize + x)*RGB]; // R (1 of 4)
                dst[(OFFSET*mMaxTextureSize+mTileSize/4*mMaxTextureSize +x)*RGB_A+2] = src[(128*mTileSize + x)*RGB]; // R (1 of 4)
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mTileSize*RGB;
        }
        // Copy mask 2 (vertical) to the atlas-texture.
        src = srcImage.getData();
        dst = dstBuf;
        for (unsigned int y = 0; y < mTileSize/2; ++y)
        {
            for (unsigned int x = 0; x < mTileSize/4; ++x)
            {
                dst[(                                    x)*RGB_A+1]+= src[(128*mTileSize +192+ x)*RGB]; // G (1 of 4)
                dst[(                        mTileSize/4+x)*RGB_A+1]+= src[(128*mTileSize +128+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET                             +x)*RGB_A+2]+= src[(128*mTileSize +192+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET                 +mTileSize/4+x)*RGB_A+2]+= src[(128*mTileSize +128+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET*2                           +x)*RGB_A+1]+= src[(128*mTileSize +192+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET*2               +mTileSize/4+x)*RGB_A+1]+= src[(128*mTileSize +128+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET*mMaxTextureSize             +x)*RGB_A+2]+= src[(128*mTileSize +192+ x)*RGB]; // G (1 of 4)
                dst[(OFFSET*mMaxTextureSize+mTileSize/4 +x)*RGB_A+2]+= src[(128*mTileSize +128+ x)*RGB]; // G (1 of 4)
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mTileSize*RGB;
        }
        // Draw the grid.
        int gridColor;
        src = srcImage.getData();
        dst = dstBuf;
        for (int i = 0; i < 2; ++i)
        {
            for (unsigned int y = 0; y < mTileSize/2; ++y)
            {
                for (unsigned int x = 0; x < mTileSize/2; ++x)
                {
                    gridColor = (!x || !y || x == mTileSize/4 || y == mTileSize/4 || x == y || x == mTileSize/2-y)?0x00:0xff;
                    dst[(0*OFFSET + x)*RGB_A+0] = gridColor; // B
                    dst[(1*OFFSET + x)*RGB_A+0] = gridColor; // B
                    dst[(2*OFFSET + x)*RGB_A+0] = gridColor; // B
                }
                dst+= mMaxTextureSize*RGB_A;
            }
            dst+= mBorderSize*2*mMaxTextureSize*RGB_A;
        }
        // Create mask borders
        // Vertical
        for (int i =0; i < 3; ++i)
        {
            dst = dstBuf + i*(mTileSize/2+mBorderSize*2)* RGB_A;
            for (unsigned int y = 0; y < mBorderSize*2+mTileSize; ++y)
            {
                for (unsigned int x = 1; x <= mBorderSize; ++x)
                {
                    // Left border
                    *(dst-x*RGB_A+0) = dst[0];
                    *(dst-x*RGB_A+1) = dst[1];
                    *(dst-x*RGB_A+2) = dst[2];
                    *(dst-x*RGB_A+3) = dst[3];
                    // Right border
                    if (i < 2)
                    {
                        *(dst+(mTileSize/2+mBorderSize-x)*RGB_A+0) = dst[(mTileSize/2-1)*RGB_A+0];
                        *(dst+(mTileSize/2+mBorderSize-x)*RGB_A+1) = dst[(mTileSize/2-1)*RGB_A+1];
                        *(dst+(mTileSize/2+mBorderSize-x)*RGB_A+2) = dst[(mTileSize/2-1)*RGB_A+2];
                        *(dst+(mTileSize/2+mBorderSize-x)*RGB_A+3) = dst[(mTileSize/2-1)*RGB_A+3];
                    }
                }
                dst+= mMaxTextureSize*RGB_A;
            }
        }
        // Horizontal
        dst = dstBuf + (mTileSize/2*mMaxTextureSize-mBorderSize)* RGB_A;
        for (int i =0; i < 2; ++i)
        {
            for (unsigned int y = 0; y < mBorderSize; ++y)
            {
                for (unsigned int x = 0; x < mBorderSize*5+mTileSize/2*3; ++x)
                {
                    // Bottom border
                    dst[(y*mMaxTextureSize+x)*RGB_A+0]= *(dst-(mMaxTextureSize-x)*RGB_A+0);
                    dst[(y*mMaxTextureSize+x)*RGB_A+1]= *(dst-(mMaxTextureSize-x)*RGB_A+1);
                    dst[(y*mMaxTextureSize+x)*RGB_A+2]= *(dst-(mMaxTextureSize-x)*RGB_A+2);
                    dst[(y*mMaxTextureSize+x)*RGB_A+3]= *(dst-(mMaxTextureSize-x)*RGB_A+3);
                    // Top border
                    if (i+sumMask)
                    {
                        *(dst-((y+mTileSize/2+1)*mMaxTextureSize-x)*RGB_A+0) = *(dst-((mTileSize/2)*mMaxTextureSize-x)*RGB_A+0);
                        *(dst-((y+mTileSize/2+1)*mMaxTextureSize-x)*RGB_A+1) = *(dst-((mTileSize/2)*mMaxTextureSize-x)*RGB_A+1);
                        *(dst-((y+mTileSize/2+1)*mMaxTextureSize-x)*RGB_A+2) = *(dst-((mTileSize/2)*mMaxTextureSize-x)*RGB_A+2);
                        *(dst-((y+mTileSize/2+1)*mMaxTextureSize-x)*RGB_A+3) = *(dst-((mTileSize/2)*mMaxTextureSize-x)*RGB_A+3);
                    }
                }
            }
            dst+= (mTileSize/2 + mBorderSize*2)*mMaxTextureSize * RGB_A;
        }
        dstBuf+= (mTileSize + mBorderSize*4)*mMaxTextureSize * RGB_A;
    }
}

//================================================================================================
// Copy a spotlight-mask into the atlastexture.
//================================================================================================
void TileAtlas::copySpotToAtlas(uchar *dstBuf)
{
    PROFILE()
    const unsigned int OFFSET = mBorderSize*2+mTileSize/2;
    Image srcImage;
    dstBuf+= 6*(mBorderSize*2+mTileSize)* RGB_A;
    String srcFilename = "Spotlight.png";
    if (!loadImage(srcImage, srcFilename, false))
    {
        Logger::log().error() << Logger::ICON_CLIENT << "The spotlight-mask was not found.";
        return;
    }
    if ((srcImage.getWidth() != mTileSize/2) || (srcImage.getHeight() != mTileSize/2))
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Gfx " << srcFilename << " has the wrong size! Alle tiles must have the same size than terrain_00_00.png";
        return;
    }
    if (srcImage.getFormat()!=PF_R8G8B8)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Gfx " << srcFilename << " has the wrong pixelformat. Only RGB is supported for masks.";
        return;
    }
    uchar *dst = dstBuf;
    for (int ty = 0; ty < ATLAS_LAND_ROWS*2; ++ty)
    {
        uchar *src = srcImage.getData();
        for (unsigned int y = 0; y < mTileSize/2; ++y)
        {
            for (unsigned int x = 0; x < mTileSize/2; ++x)
            {
                dst[(0*OFFSET+x)*RGB_A+3] = src[x*RGB];
                dst[(1*OFFSET+x)*RGB_A+3] = src[x*RGB];
                dst[(2*OFFSET+x)*RGB_A+3] = src[x*RGB];
            }
            dst+= mMaxTextureSize*RGB_A;
            src+= mTileSize/2*RGB;
        }
        dst+= mBorderSize*2*mMaxTextureSize*RGB_A;
    }
}

//================================================================================================
// Create a template for the mask.
//================================================================================================
void TileAtlas::createMaskTemplate()
{
    PROFILE()
    const unsigned char color[3] = {0xC6, 0x38, 0xDB};
    int lineSkip = mTileSize * RGB;
    const int UNUSED_SIZE = 16;
    uchar *dstBuf = new uchar[mTileSize * mTileSize * RGB];
    memset(dstBuf, 0x00, mTileSize * mTileSize * RGB);
    Image dstImage;
    dstImage.loadDynamicImage(dstBuf, mTileSize, mTileSize, 1, PF_R8G8B8);
    // vertical borderline
    for (uint32 y = 0; y < mTileSize/2; ++y)
    {
        for (int c = 0; c < 3; ++c)
        {
            dstBuf[(y*mTileSize                )*RGB +c] = color[c];
            dstBuf[(y*mTileSize + mTileSize-1  )*RGB +c] = color[c];
            dstBuf[(y*mTileSize + mTileSize/2  )*RGB +c] = color[c];
            dstBuf[(y*mTileSize + mTileSize/2-1)*RGB +c] = color[c];
        }
    }
    // Horizontal borderlines for karos
    uchar *p = dstBuf + mTileSize/2*lineSkip;
    for (int i = 0; i < 2; ++i)
    {
        for (int height= 0; height < UNUSED_SIZE; ++height)
        {
            for (uint32 x = 0; x < mTileSize/2; ++x)
            {
                for (int c = 0; c < 3; ++c)
                {
                    p[((height              )*mTileSize+x)*RGB +c] = color[c]; // Top pos
                    p[((mTileSize/2-height-2)*mTileSize+x)*RGB +c] = color[c]; // Bottom pos
                }
            }
        }
        p+= lineSkip;
    }
    // Vertical borderlines for karos
    p = dstBuf + mTileSize/2*lineSkip + mTileSize/2*RGB;
    for (int i = 0; i < 2; ++i)
    {
        for (int width= 0; width < UNUSED_SIZE; ++width)
        {
            for (uint32 y = 0; y < mTileSize/2; ++y)
            {
                for (int c = 0; c < 3; ++c)
                {
                    p[(y*mTileSize + width                )*RGB +c] = color[c]; // Left pos
                    p[(y*mTileSize - width + mTileSize/2-2)*RGB +c] = color[c];// Right pos
                }
            }
        }
        p+= lineSkip;
    }
    // Karos
    int offset = 0;
    p = dstBuf + lineSkip * mTileSize/2;
    int karoSize= mTileSize/2;
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
// Load an existing image. Returns true on success.
// (Used to prevent ogre from spamming if image.load() fails).
//================================================================================================
bool TileAtlas::loadImage(Image &image, const Ogre::String &strFilename, bool logErrors)
{
    PROFILE()
    try
    {
        image.load(strFilename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
    catch (Exception &)
    {
        if (logErrors)
            Logger::log().error() << Logger::ICON_CLIENT << "Error on opening file " << mPathGfxTiles + strFilename;
        return false;
    }
    return true;
}

//================================================================================================
//
//================================================================================================
int TileAtlas::setResourcePath(String key, String &refPath)
{
    PROFILE()
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
                    Logger::log().error() << Logger::ICON_CLIENT << "The '"<< key << "' entry given in 'resources.cfg' does not exist in the media folder!";
                    return false;
                }
                refPath = i->second + "/";
                return true;
            }
        }
    }
    Logger::log().error() << Logger::ICON_CLIENT << "The 'resources.cfg' is missing a '"<< key << "' entry!";
    return false;
}
