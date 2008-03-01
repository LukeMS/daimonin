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

#include "define.h"
#include "option.h"
#include "logger.h"
#include "object_manager.h"
#include "tile_manager.h"

using namespace Ogre;

#define SUM_TILES 21

const char SRC_TEXTURE_NAME[]     = "terrain_group_00_1024.png";
const char TEXTURE_LAND_NAME[]    = "TileEngine/TexLand";
const char TEXTURE_WATER_NAME[]   = "TileEngine/TexWater";
const char MATERIAL_LAND_NAME[]   = "TileEngine/MatLand";
const char MATERIAL_WATER_NAME[]  = "TileEngine/MatWater";
const char MATERIAL_PAINTER_NAME[]= "TileEngine/MatPainter";
const int  GRID_SHADOW_MASK_NR    = 72; // Number of the shadow filter used to draw a grid.

//================================================================================================
// The map has its zero-position in the top-left corner:
// (x,z)
//  0,0 1,0 2,0 3,0
//  0,1 1,1 2,1 3,1
//  0,2 1,2 2,2 3,2
//  0,3 1,3 2,3 3,3
//================================================================================================

const char srcTexturePos[SUM_TILES][SUM_TILES][3]=
{
    //       pos
    //       X, Y, size (1 = small 2 = big)
    //          0         1         2         3         4         5         6         7         8         9        10        11        12        13        14        15        16        17        18        19         20
    /* 0*/{{26,26,1},{26,26,1},{ 0, 0,1},{ 1, 0,1},{ 2, 0,1},{ 3, 0,1},{ 4, 0,1},{ 5, 0,1},{ 6, 0,1},{ 7, 0,1},{ 8, 0,1},{ 9, 0,1},{10, 0,1},{11, 0,1},{12, 0,1},{13, 0,1},{14, 0,1},{15, 0,1},{16, 0,1},{26,26,1},{26,26,1}},
    /* 1*/{{26,26,1},{17, 0,1},{18, 0,1},{19, 0,1},{20, 0,1},{21, 0,1},{22, 0,1},{23, 0,1},{24, 0,1},{25, 0,1},{26, 0,1},{27, 0,1},{ 0, 1,1},{ 1, 1,1},{ 2, 1,1},{ 3, 1,1},{ 4, 1,1},{ 5, 1,1},{ 6, 1,1},{ 7, 1,1},{26,26,1}},
    /* 2*/{{ 8, 1,1},{ 9, 1,1},{10, 1,1},{11, 1,1},{12, 1,1},{13, 1,1},{14, 1,1},{15, 1,1},{16, 1,1},{17, 1,1},{18, 1,1},{19, 1,1},{20, 1,1},{21, 1,1},{22, 1,1},{23, 1,1},{24, 1,1},{25, 1,1},{26, 1,1},{27, 1,1},{ 0, 2,1}},
    /* 3*/{{ 1, 2,1},{ 2, 2,1},{ 3, 2,1},{ 4, 2,1},{ 5, 2,1},{ 6, 2,1},{ 7, 2,1},{ 8, 2,1},{ 9, 2,1},{10, 2,1},{11, 2,1},{12, 2,1},{13, 2,1},{14, 2,1},{15, 2,1},{16, 2,1},{17, 2,1},{18, 2,1},{19, 2,1},{20, 2,1},{21, 2,1}},
    /* 4*/{{22, 2,1},{23, 2,1},{24, 2,1},{25, 2,1},{26, 2,1},{27, 2,1},{ 0, 3,1},{ 1, 3,1},{ 2, 3,1},{ 3, 3,1},{ 4, 3,1},{ 5, 3,1},{ 6, 3,1},{ 7, 3,1},{ 8, 3,1},{ 9, 3,1},{10, 3,1},{11, 3,1},{12, 3,1},{13, 3,1},{14, 3,1}},
    /* 5*/{{15, 3,1},{16, 3,1},{17, 3,1},{18, 3,1},{19, 3,1},{20, 3,1},{ 8, 6,2},{10, 6,2},{12, 6,2},{14, 6,2},{16, 6,2},{18, 6,2},{20, 6,2},{22, 6,2},{24, 6,2},{21, 3,1},{22, 3,1},{23, 3,1},{24, 3,1},{25, 3,1},{26, 3,1}},
    /* 6*/{{27, 3,1},{ 0, 4,1},{ 1, 4,1},{ 2, 4,1},{ 3, 4,1},{ 6, 8,2},{ 8, 8,2},{10, 8,2},{12, 8,2},{14, 8,2},{16, 8,2},{18, 8,2},{20, 8,2},{22, 8,2},{24, 8,2},{26, 8,2},{ 4, 4,1},{ 5, 4,1},{ 6, 4,1},{ 7, 4,1},{ 8, 4,1}},
    /* 7*/{{ 9, 4,1},{10, 4,1},{11, 4,1},{12, 4,1},{13, 4,1},{ 6,10,2},{ 8,10,2},{10,10,2},{12,10,2},{14,10,2},{16,10,2},{18,10,2},{20,10,2},{22,10,2},{24,10,2},{26,10,2},{14, 4,1},{15, 4,1},{16, 4,1},{17, 4,1},{18, 4,1}},
    /* 8*/{{19, 4,1},{20, 4,1},{21, 4,1},{22, 4,1},{23, 4,1},{ 6,12,2},{ 8,12,2},{10,12,2},{12,12,2},{14,12,2},{16,12,2},{18,12,2},{20,12,2},{22,12,2},{24,12,2},{26,12,2},{24, 4,1},{25, 4,1},{26, 4,1},{27, 4,1},{ 0, 5,1}},
    /* 9*/{{ 1, 5,1},{ 2, 5,1},{ 3, 5,1},{ 4, 5,1},{ 5, 5,1},{ 6,14,2},{ 8,14,2},{10,14,2},{12,14,2},{14,14,2},{16,14,2},{18,14,2},{20,14,2},{22,14,2},{24,14,2},{26,14,2},{ 6, 5,1},{ 7, 5,1},{ 8, 5,1},{ 9, 5,1},{10, 5,1}},
    /*10*/{{11, 5,1},{12, 5,1},{13, 5,1},{14, 5,1},{15, 5,1},{ 6,16,2},{ 8,16,2},{10,16,2},{12,16,2},{14,16,2},{16,16,2},{18,16,2},{20,16,2},{22,16,2},{24,16,2},{26,16,2},{16, 5,1},{17, 5,1},{18, 5,1},{19, 5,1},{20, 5,1}},
    /*11*/{{21, 5,1},{22, 5,1},{23, 5,1},{24, 5,1},{25, 5,1},{ 6,18,2},{ 8,18,2},{10,18,2},{12,18,2},{14,18,2},{16,18,2},{18,18,2},{20,18,2},{22,18,2},{24,18,2},{26,18,2},{26, 5,1},{27, 5,1},{ 0, 6,1},{ 1, 6,1},{ 2, 6,1}},
    /*12*/{{ 3, 6,1},{ 4, 6,1},{ 5, 6,1},{ 6, 6,1},{ 7, 6,1},{ 6,20,2},{ 8,20,2},{10,20,2},{12,20,2},{14,20,2},{16,20,2},{18,20,2},{20,20,2},{22,20,2},{24,20,2},{26,20,2},{26, 6,1},{27, 6,1},{ 0, 7,1},{ 1, 7,1},{ 2, 7,1}},
    /*13*/{{ 3, 7,1},{ 4, 7,1},{ 5, 7,1},{ 6, 7,1},{ 7, 7,1},{ 6,22,2},{ 8,22,2},{10,22,2},{12,22,2},{14,22,2},{16,22,2},{18,22,2},{20,22,2},{22,22,2},{24,22,2},{26,22,2},{26, 7,1},{27, 7,1},{ 0, 8,1},{ 1, 8,1},{ 2, 8,1}},
    /*14*/{{ 3, 8,1},{ 4, 8,1},{ 5, 8,1},{ 0, 9,1},{ 1, 9,1},{ 6,24,2},{ 8,24,2},{10,24,2},{12,24,2},{14,24,2},{16,24,2},{18,24,2},{20,24,2},{22,24,2},{24,24,2},{26,24,2},{ 2, 9,1},{ 3, 9,1},{ 4, 9,1},{ 5, 9,1},{ 0,10,1}},
    /*15*/{{ 1,10,1},{ 2,10,1},{ 3,10,1},{ 4,10,1},{ 5,10,1},{ 0,11,1},{ 8,26,2},{10,26,2},{12,26,2},{14,26,2},{16,26,2},{18,26,2},{20,26,2},{22,26,2},{24,26,2},{ 1,11,1},{ 2,11,1},{ 3,11,1},{ 4,11,1},{ 5,11,1},{ 0,12,1}},
    /*16*/{{ 1,12,1},{ 2,12,1},{ 3,12,1},{ 4,12,1},{ 5,12,1},{ 0,13,1},{ 1,13,1},{ 2,13,1},{ 3,13,1},{ 4,13,1},{ 5,13,1},{ 0,14,1},{ 1,14,1},{ 2,14,1},{ 3,14,1},{ 4,14,1},{ 5,14,1},{ 0,15,1},{ 1,15,1},{ 2,15,1},{ 3,15,1}},
    /*17*/{{ 4,15,1},{ 5,15,1},{ 0,16,1},{ 1,16,1},{ 2,16,1},{ 3,16,1},{ 4,16,1},{ 5,16,1},{ 0,17,1},{ 1,17,1},{ 2,17,1},{ 3,17,1},{ 4,17,1},{ 5,17,1},{ 0,18,1},{ 1,18,1},{ 2,18,1},{ 3,18,1},{ 4,18,1},{ 5,18,1},{ 0,19,1}},
    /*18*/{{ 1,19,1},{ 2,19,1},{ 3,19,1},{ 4,19,1},{ 5,19,1},{ 0,20,1},{ 1,20,1},{ 2,20,1},{ 3,20,1},{ 4,20,1},{ 5,20,1},{ 0,21,1},{ 1,21,1},{ 2,21,1},{ 3,21,1},{ 4,21,1},{ 5,21,1},{ 0,22,1},{ 1,22,1},{ 2,22,1},{ 3,22,1}},
    /*19*/{{26,26,1},{ 4,22,1},{ 5,22,1},{ 0,23,1},{ 1,23,1},{ 2,23,1},{ 3,23,1},{ 4,23,1},{ 5,23,1},{ 0,24,1},{ 1,24,1},{ 2,24,1},{ 3,24,1},{ 4,24,1},{ 5,24,1},{ 0,25,1},{ 1,25,1},{ 2,25,1},{ 3,25,1},{ 4,25,1},{26,26,1}},
    /*20*/{{26,26,1},{26,26,1},{ 5,25,1},{ 0,26,1},{ 1,26,1},{ 2,26,1},{ 3,26,1},{ 4,26,1},{ 5,26,1},{ 6,26,1},{ 7,26,1},{ 0,27,1},{ 1,27,1},{ 2,27,1},{ 3,27,1},{ 4,27,1},{ 5,27,1},{ 6,27,1},{ 7,27,1},{26,26,1},{26,26,1}},
};

//================================================================================================
// Holds the vertex positions for mirroring on X and Z axis.
//================================================================================================
const char mirrorTable[4][6][2]=
{
    {{0,0}, {0,1}, {1,0}, {1,0}, {0,1}, {1,1}}, // No mirror
    {{1,0}, {1,1}, {0,0}, {0,0}, {1,1}, {0,1}}, // Mirror on x-axis.
    {{0,1}, {0,0}, {1,1}, {1,1}, {0,0}, {1,0}}, // Mirror on z-axis.
    {{1,1}, {1,0}, {0,1}, {0,1}, {1,0}, {0,0}}  // Mirror on x and z-axis.
};

//================================================================================================
// v0  +--+ v2/3
//     | /|
//     |/ |
// v1/4+--+ v5
//================================================================================================
void TilePainter::updateVertexBuffer()
{
    VertexBufferBinding *vBind = mRenderOp.vertexData->vertexBufferBinding;
    HardwareVertexBufferSharedPtr tvbuf = vBind->getBuffer(1);
    float* pTex = static_cast<float*>(tvbuf->lock(HardwareBuffer::HBL_DISCARD));
    int tilesPerRow  = TileManager::TEXTURE_SIZE /TileManager::ATLAS_TILE_SIZE;
    int filtersPerRow= TileManager::TEXTURE_SIZE /TileManager::ATLAS_FILTER_SIZE;
    float sizeTile   = 1.0f/(float) tilesPerRow;
    float sizeFilter = 1.0f/(float) filtersPerRow;
    Real offX, offZ;
    Real colLayer0, rowLayer0;
    Real colLayer1, rowLayer1;
    Real colFilter, rowFilter;
    Real colShadow, rowShadow;
    int tLayer0, tLayer1, tFilter, tShadow;
    int tShadowMirror;
    int mapX, mapZ, odd;
    TileManager::getSingleton().getMapScroll(mapX, mapZ);
    odd = mapX + (mapZ&1);
    for (int z = 0; z < SUM_TILES; ++z)
    {
        offZ = (Real)((mapZ-z)&1) * sizeTile/2;
        for (int x = 0; x < SUM_TILES; ++x)
        {
            offX = (Real)((mapX-x)&1) * sizeTile/2;
            tLayer0 = TileManager::getSingleton().getMapTile(x,z,0);
            tLayer1 = TileManager::getSingleton().getMapTile(x,z,1);
            tFilter = 4;
            colLayer0 = offX + (Real)(tLayer0&3) * sizeTile;
            rowLayer0 = offZ + ((Real)(tLayer0/tilesPerRow)) * sizeTile;
            colLayer1 = offX + (Real)(tLayer1&3) * sizeTile;
            rowLayer1 = offZ + ((Real)(tLayer1/tilesPerRow)) * sizeTile;
            colFilter = ((Real)(tFilter&15)) * sizeFilter;
            rowFilter = ((Real)(tFilter/filtersPerRow)) * sizeFilter;
            if (mShowGrid)
            {
                colShadow = ((Real)(GRID_SHADOW_MASK_NR&15)) * sizeFilter;
                rowShadow =(((Real)(GRID_SHADOW_MASK_NR/filtersPerRow))+ filtersPerRow/2) * sizeFilter;
                tShadowMirror = ((++odd)&1);
            }
            else
            {
                int c = TileManager::getSingleton().getDeltaHeightClass(x-1, z);
                int h = TileManager::getSingleton().getDeltaHeightBottom(x-1, z+1);
                if (c == 1)
                {
                    if      (h > 64) tShadow =  3;
                    else if (h > 10) tShadow =  2;
                    else             tShadow =  1;
                }
                else if (c == 2)
                {
                    if      (h > 64) tShadow = 13;
                    else if (h > 10) tShadow = 12;
                    else             tShadow = 11;
                }
                else if (c == 3)
                {
                    if      (h > 64) tShadow = 23;
                    else if (h > 10) tShadow = 22;
                    else             tShadow = 21;
                }
                else if (c == 4)
                {
                    if      (h > 64) tShadow = 33;
                    else if (h > 10) tShadow = 32;
                    else             tShadow = 31;
                }
                else if (c == 5)
                {
                    if      (h > 64) tShadow = 43;
                    else if (h > 10) tShadow = 42;
                    else             tShadow = 41;
                }
                else tShadow = 71; // No shadow.

                //Logger::log().error() << "Tile x: " << x << " z: " << y << " shadowNr: " << tShadow;

                //tShadow       = 71;//TileManager::getSingleton().getMapShadow(x,y);
                tShadowMirror = 0;//TileManager::getSingleton().getMapShadowMirror(x,y);
                colShadow = ((Real)(tShadow&15)) * sizeFilter;
                rowShadow =(((Real)(tShadow/filtersPerRow))+ filtersPerRow/2) * sizeFilter;
            }
            // Vertex 0
            *pTex++ = colLayer0;
            *pTex++ = rowLayer0;
            *pTex++ = colFilter;
            *pTex++ = rowFilter;
            *pTex++ = colLayer1;
            *pTex++ = rowLayer1;
            *pTex++ = colShadow + sizeFilter * mirrorTable[tShadowMirror][0][0];
            *pTex++ = rowShadow + sizeFilter * mirrorTable[tShadowMirror][0][1];
            // Vertex 1
            *pTex++ = colLayer0;
            *pTex++ = rowLayer0 + sizeTile/2;
            *pTex++ = colFilter;
            *pTex++ = rowFilter + sizeFilter;
            *pTex++ = colLayer1;
            *pTex++ = rowLayer1 + sizeTile/2;
            *pTex++ = colShadow + sizeFilter * mirrorTable[tShadowMirror][1][0];
            *pTex++ = rowShadow + sizeFilter * mirrorTable[tShadowMirror][1][1];
            // Vertex 2
            *pTex++ = colLayer0 + sizeTile/2;
            *pTex++ = rowLayer0;
            *pTex++ = colFilter + sizeFilter;
            *pTex++ = rowFilter;
            *pTex++ = colLayer1 + sizeTile/2;
            *pTex++ = rowLayer1;
            *pTex++ = colShadow + sizeFilter * mirrorTable[tShadowMirror][2][0];
            *pTex++ = rowShadow + sizeFilter * mirrorTable[tShadowMirror][2][1];
            // Vertex 3
            *pTex++ = colLayer0 + sizeTile/2;
            *pTex++ = rowLayer0;
            *pTex++ = colFilter + sizeFilter;
            *pTex++ = rowFilter;
            *pTex++ = colLayer1 + sizeTile/2;
            *pTex++ = rowLayer1;
            *pTex++ = colShadow + sizeFilter * mirrorTable[tShadowMirror][3][0];
            *pTex++ = rowShadow + sizeFilter * mirrorTable[tShadowMirror][3][1];
            // Vertex 4
            *pTex++ = colLayer0;
            *pTex++ = rowLayer0 + sizeTile/2;
            *pTex++ = colFilter;
            *pTex++ = rowFilter + sizeFilter;
            *pTex++ = colLayer1;
            *pTex++ = rowLayer1 + sizeTile/2;
            *pTex++ = colShadow + sizeFilter * mirrorTable[tShadowMirror][4][0];
            *pTex++ = rowShadow + sizeFilter * mirrorTable[tShadowMirror][4][1];
            // Vertex 5
            *pTex++ = colLayer0 + sizeTile/2;
            *pTex++ = rowLayer0 + sizeTile/2;
            *pTex++ = colFilter + sizeFilter;
            *pTex++ = rowFilter + sizeFilter;
            *pTex++ = colLayer1 + sizeTile/2;
            *pTex++ = rowLayer1 + sizeTile/2;
            *pTex++ = colShadow + sizeFilter * mirrorTable[tShadowMirror][5][0];
            *pTex++ = rowShadow + sizeFilter * mirrorTable[tShadowMirror][5][1];
        }
    }
    tvbuf->unlock();
}

//================================================================================================
// Create the painter used to draw the blended tiles from alpha-texture to a render-texture.
//================================================================================================
TilePainter::TilePainter()
{
    mShowGrid = false;
    int SUM_VERTICES = SUM_TILES * SUM_TILES * 6;
    mUseIdentityProjection = true;
    mUseIdentityView = true;
    mRenderOp.useIndexes = false;
    mRenderOp.vertexData = new VertexData();
    mRenderOp.vertexData->vertexCount = SUM_VERTICES;
    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST ;
    VertexDeclaration   *decl = mRenderOp.vertexData->vertexDeclaration;
    VertexBufferBinding *bind = mRenderOp.vertexData->vertexBufferBinding;
    size_t offset = 0;
    decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);  offset += VertexElement::getTypeSize(VET_FLOAT3);
    HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(VertexElement::getTypeSize(VET_FLOAT3), mRenderOp.vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    bind->setBinding(0, vbuf);
    float* pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    Real left, top;
    Real size = 1.0f/14.0f; // 28 tile positions fit into the texture 7 for coordinates -1...0 and 7 for 0...1.
    for (int y = 0; y < SUM_TILES; ++y)
    {
        for (int x = 0; x < SUM_TILES; ++x)
        {
            Real tile_size = size * srcTexturePos[y][x][2];
            left = -1.0f + size * srcTexturePos[y][x][0];
            top  =  1.0f - size * srcTexturePos[y][x][1];
            *pFloat++ = left;
            *pFloat++ = top;
            *pFloat++ = -1;
            *pFloat++ = left;
            *pFloat++ = top - tile_size;
            *pFloat++ = -1;
            *pFloat++ = left+ tile_size;
            *pFloat++ = top;
            *pFloat++ = -1;
            *pFloat++ = left + tile_size;
            *pFloat++ = top;
            *pFloat++ = -1;
            *pFloat++ = left;
            *pFloat++ = top - tile_size;
            *pFloat++ = -1;
            *pFloat++ = left+ tile_size;
            *pFloat++ = top - tile_size;
            *pFloat++ = -1;
        }
    }
    vbuf->unlock();
    const int SUM_TEXTURE_UNITS = 4;
    offset = 0;
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,0);  offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,1);  offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,2);  offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,3);  offset += VertexElement::getTypeSize(VET_FLOAT2);

    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(VertexElement::getTypeSize(VET_FLOAT2)*SUM_TEXTURE_UNITS, SUM_VERTICES, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    bind->setBinding(1, vbuf);
    updateVertexBuffer();

    MaterialPtr tmpMaterial;
    tmpMaterial= MaterialManager::getSingleton().getByName(MATERIAL_PAINTER_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(SRC_TEXTURE_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName(SRC_TEXTURE_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName(SRC_TEXTURE_NAME);
    //tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(3)->setTextureName("terrain_shadow.png");
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(3)->setTextureName(SRC_TEXTURE_NAME);
    setMaterial(MATERIAL_PAINTER_NAME);

    AxisAlignedBox aabInf;
    aabInf.setInfinite();
    setBoundingBox(aabInf);
    /*
            mRenderOp.useIndexes = true;
            mRenderOp.indexData = new IndexData();
            mRenderOp.indexData->indexCount = SUM_VERTICES;
            mRenderOp.indexData->indexStart = 0;
            mRenderOp.indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, mRenderOp.indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY,false);
            unsigned short* pIdx = static_cast<unsigned short*>(mRenderOp.indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
            unsigned short numVertices = mRenderOp.indexData->indexCount;
            for (unsigned short p=0; p < numVertices; ++p)
                *pIdx++ = p;
            mRenderOp.indexData->indexBuffer->unlock();
    */
}

//================================================================================================
// Switch the grid on/off.
//================================================================================================
void TileChunk::toggleGrid()
{
    mPainter->toggleGrid();
    updatePainter();
}

//================================================================================================
// Constructor.
//================================================================================================
void TileChunk::init()
{
    MaterialPtr tmpMaterial;
    // ////////////////////////////////////////////////////////////////////
    // Build the land-tiles.
    // ////////////////////////////////////////////////////////////////////
    mMeshLand = MeshManager::getSingleton().createManual("Mesh_Land", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
    mSubMeshLand = mMeshLand->createSubMesh();
    createLand();
    mMeshLand->_setBounds(AxisAlignedBox(0,0,0,TileManager::TILE_SIZE * SUM_TILES,MAX_TERRAIN_HEIGHT,TileManager::TILE_SIZE * SUM_TILES));
    mMeshLand->load();
    mEntityLand = TileManager::getSingleton().getSceneManager()->createEntity("Entity_Land", "Mesh_Land");
    mEntityLand->setQueryFlags(ObjectManager::QUERY_TILES_LAND_MASK);
    mEntityLand->setRenderQueueGroup(RENDER_QUEUE_1);
    mTexLand = TextureManager::getSingleton().createManual(TEXTURE_LAND_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 1024, 1024, 0, PF_R8G8B8, TU_RENDERTARGET);
    tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_LAND_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_LAND_NAME);
    mEntityLand->setMaterialName(MATERIAL_LAND_NAME);
    // ////////////////////////////////////////////////////////////////////
    // Build the water-tiles.
    // ////////////////////////////////////////////////////////////////////
    mMeshWater = MeshManager::getSingleton().createManual("Mesh_Water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMeshWater = mMeshWater->createSubMesh();
    createDummySubMesh(mSubMeshWater); // We don't have the tile datas yet, so no need to waste time in creating the tiles.
    mMeshWater->_setBounds(AxisAlignedBox(0,0,0,TileManager::TILE_SIZE * SUM_TILES,MAX_TERRAIN_HEIGHT,TileManager::TILE_SIZE * SUM_TILES));
    mMeshWater->load();
    mEntityWater = TileManager::getSingleton().getSceneManager()->createEntity("Entity_Water", "Mesh_Water");
    mEntityWater->setQueryFlags(ObjectManager::QUERY_TILES_WATER_MASK);
    mEntityWater->setRenderQueueGroup(RENDER_QUEUE_8);
    mTexWater = TextureManager::getSingleton().createManual(TEXTURE_WATER_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, TileManager::ATLAS_TILE_SIZE, TileManager::ATLAS_TILE_SIZE, 0, PF_A8R8G8B8, TU_STATIC);
    tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_WATER_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_WATER_NAME);
    mEntityWater->setMaterialName(MATERIAL_WATER_NAME);
    // ////////////////////////////////////////////////////////////////////
    // Attach the tiles to a scenenode.
    // ////////////////////////////////////////////////////////////////////
    mNode= TileManager::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();
    mNode->setPosition(0, 0, 0);
    mNode->attachObject(mEntityLand);
    mNode->attachObject(mEntityWater);
    mPainter= new TilePainter();
    loadAtlasTexture("terrain_group_00_1024.png");
    updatePainter();
}

//================================================================================================
// Set a new material.
//================================================================================================
void TileChunk::loadAtlasTexture(String newAtlasTexture)
{
    Image image;
    image.load(newAtlasTexture, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    uint32 *src = (uint32 *) image.getData();
    // ////////////////////////////////////////////////////////////////////
    // Copy the water tile to a separate texture
    // (Water needs own texture for the scroll animation).
    // ////////////////////////////////////////////////////////////////////
    PixelBox pb;
    uint32 *dst;
    int size = (int)image.getWidth();
    int sizeTile = size/4;

    pb = mTexWater->getBuffer()->lock(Box(0, 0, TileManager::ATLAS_TILE_SIZE, TileManager::ATLAS_TILE_SIZE), HardwareBuffer::HBL_DISCARD);
    dst = (uint32 *)pb.data;
    for (int y = 0; y<sizeTile; ++y)
    {
        for (int x = 0; x<sizeTile; ++x)
            *dst++ = *src++ | 0xff000000;
        src+=(size - sizeTile);
    }
    mTexWater->getBuffer()->unlock();
    // ////////////////////////////////////////////////////////////////////
    // Copy the new atlastexture over the old one.
    // ////////////////////////////////////////////////////////////////////
    MaterialPtr tmpMaterial = mPainter->getMaterial();
    TexturePtr texPainter = TextureManager::getSingleton().getByName(tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName());
    pb = texPainter->getBuffer()->lock(Box(0, 0, TileManager::ATLAS_TILE_SIZE, TileManager::ATLAS_TILE_SIZE), HardwareBuffer::HBL_DISCARD);
    src = (uint32 *) image.getData();
    dst = (uint32 *)pb.data;
    for (int i=0; i< size*size; ++i)
        *dst++ = *src++;
    texPainter->getBuffer()->unlock();
    updatePainter();
}

//================================================================================================
// Update the rendertexture (e.g after a mapscroll).
//================================================================================================
void TileChunk::updatePainter()
{
    mPainter->updateVertexBuffer();
    SceneNode *node = TileManager::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode("nodeTilePainter");
    node->attachObject(mPainter);
    RenderTexture *renderTarget = mTexLand->getBuffer()->getRenderTarget();
    renderTarget->setAutoUpdated(false);
    Camera *camera = TileManager::getSingleton().getSceneManager()->createCamera("tmpCamera");
    camera->setUseIdentityView(true);
    camera->setUseIdentityProjection(true);
    Viewport *viewport = renderTarget->addViewport(camera);
    viewport->setOverlaysEnabled(false);
    viewport->setClearEveryFrame(false);
    viewport->setDimensions(0.0f, 0.0f, 896.0f/1024.0f, 896.0f/1024.0f);
    // Render only the queues in the special case list.
    TileManager::getSingleton().getSceneManager()->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_INCLUDE);
    TileManager::getSingleton().getSceneManager()->addSpecialCaseRenderQueue(RENDER_QUEUE_MAIN);
    mPainter->setRenderQueueGroup(RENDER_QUEUE_MAIN);
    renderTarget->update();
//renderTarget->writeContentsToFile("Animation2d_.png");
    TileManager::getSingleton().getSceneManager()->removeSpecialCaseRenderQueue(RENDER_QUEUE_MAIN);
    //Render all except the queues in the special case list.
    TileManager::getSingleton().getSceneManager()->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
    renderTarget->removeViewport(0);
    node->detachAllObjects();
    TileManager::getSingleton().getSceneManager()->destroyCamera(camera);
    TileManager::getSingleton().getSceneManager()->destroySceneNode("nodeTilePainter");
}

//================================================================================================
// Free all resources.
//================================================================================================
void TileChunk::freeRecources()
{
    mMeshLand.setNull();
    mMeshWater.setNull();
    mTexWater.setNull();
    mTexLand.setNull();
    mMeshPainter.setNull();
    delete mPainter;
}

//================================================================================================
// Change a chunk.
//================================================================================================
void TileChunk::change()
{
    long time = Root::getSingleton().getTimer()->getMicroseconds();
    changeLand();
    createWater();
    updatePainter();
    Logger::log().error() << "Time to change terrain: " << (double)(Root::getSingleton().getTimer()->getMicroseconds() - time)/1000 << " ms";
}

//================================================================================================
// Create hardware buffers for water.
//================================================================================================
void TileChunk::createWater()
{
    // ////////////////////////////////////////////////////////////////////
    // Count the Vertices in this chunk.
    // ////////////////////////////////////////////////////////////////////
    unsigned int numVertices = 0;
    for (int x = 0; x < TileManager::CHUNK_SIZE; ++x)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE; ++z)
        {
            if (TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_TL) < WATERLEVEL || TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_TR) < WATERLEVEL ||
                    TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_BL) < WATERLEVEL || TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_BR) < WATERLEVEL)
            {
                numVertices+= 6;
            }
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // When we don't have any water on the map, we create a dummy mesh.
    // That way we won't run in any trouble because of uninitialized stuff.
    // ////////////////////////////////////////////////////////////////////
    if (!numVertices)
    {
        createDummySubMesh(mSubMeshWater);
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Create VertexData.
    // ////////////////////////////////////////////////////////////////////
    int mapX, mapZ;
    delete mSubMeshWater->vertexData;
    VertexData* vdata = new VertexData();
    vdata->vertexCount = numVertices;
    VertexDeclaration* vdec = vdata->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement( 0, offset, VET_FLOAT3, VES_POSITION );  offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement( 0, offset, VET_FLOAT3, VES_NORMAL   );  offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset += VertexElement::getTypeSize(VET_FLOAT2);

    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(offset, numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vdata->vertexBufferBinding->setBinding(0, vbuf0);

    static Real offsetWave = 0.03;
    static Real WaveHigh = 0;
    WaveHigh+= offsetWave;
    if (WaveHigh >1.7 || WaveHigh < -1.7) offsetWave*=-1;

    Real* pReal = static_cast<Real*>(vbuf0->lock (HardwareBuffer::HBL_DISCARD));
    Real q1, q2, offX, offZ;
    for (int x = 0; x < TileManager::CHUNK_SIZE; ++x)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE; ++z)
        {
            if (TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_TL) < WATERLEVEL || TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_TR) < WATERLEVEL ||
                    TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_BL) < WATERLEVEL || TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_BR) < WATERLEVEL)
            {
                // ////////////////////////////////////////////////////////////////////
                // Position.
                // ////////////////////////////////////////////////////////////////////
                TileManager::getSingleton().getMapScroll(mapX, mapZ);
                mapX-=x;
                mapZ-=z;
                if ((mapX&1) != (mapZ&1))
                {
                    q1 = WATERLEVEL + WaveHigh;
                    q2 = WATERLEVEL - WaveHigh;
                }
                else
                {
                    q1 = WATERLEVEL - WaveHigh;
                    q2 = WATERLEVEL + WaveHigh;
                }
                offX = 1-(Real)(mapX&3) * 0.25;
                offZ = 1-(Real)(mapZ&3) * 0.25;
                // 1. Triangle
                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = q1;
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX;
                *pReal++ = offZ;

                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX;
                *pReal++ = offZ + 0.25;

                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX + 0.25;
                *pReal++ = offZ;
                // 2. Triangle
                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX + 0.25;
                *pReal++ = offZ;

                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX;
                *pReal++ = offZ + 0.25;

                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++= q1;
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX + 0.25;
                *pReal++ = offZ + 0.25;
            }
        }
    }
    vbuf0->unlock();
    // ////////////////////////////////////////////////////////////////////
    // Create Index-buffer
    // (SubMeshes always use indexes)
    // ////////////////////////////////////////////////////////////////////
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    IndexData* idata = mSubMeshWater->indexData;
    idata->indexBuffer = ibuf;
    idata->indexStart = 0;
    idata->indexCount = numVertices;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock (HardwareBuffer::HBL_DISCARD));
    for (unsigned short p=0; numVertices; --numVertices) *pIdx++ = p++;
    ibuf->unlock();

    mSubMeshWater->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshWater->useSharedVertices = false;
    mSubMeshWater->vertexData = vdata;
}

//================================================================================================
// v0  +--+ v2/3
//     | /|
//     |/ |
// v1/4+--+ v5
//================================================================================================
void TileChunk::changeLand()
{
    VertexData *vData = mSubMeshLand->vertexData;
    VertexBufferBinding *vBind = vData->vertexBufferBinding;
    HardwareVertexBufferSharedPtr vbuf = vBind->getBuffer(0);
    Real *pReal = static_cast<Real*>(vbuf->lock (HardwareBuffer::HBL_DISCARD));
    int mapX, mapZ;
    TileManager::getSingleton().getMapScroll(mapX, mapZ);
    mapX += (mapZ&1);
    Real detail, size = 32.0 /1024.0;
    for (int z = 0; z < TileManager::CHUNK_SIZE; ++z)
    {
        for (int x= 0; x < TileManager::CHUNK_SIZE; ++x)
        {
            detail = srcTexturePos[z][x][2] * size;
            if (mapX&1)
            {
                // 1. Triangle
                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TL);
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0.0;
                *pReal++ = 1.0;
                *pReal++ = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size;
                *pReal++ = srcTexturePos[z][x][1] * size;

                *pReal++  = TileManager::TILE_SIZE * x;
                *pReal++  = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BL);
                *pReal++  = TileManager::TILE_SIZE * (z+1);
                *pReal++  = 0.0;
                *pReal++  = 1.0;
                *pReal++  = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size;
                *pReal++ = srcTexturePos[z][x][1] * size + detail;

                *pReal++  = TileManager::TILE_SIZE * (x+1);
                *pReal++  = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TR);
                *pReal++  = TileManager::TILE_SIZE * z;
                *pReal++  = 0.0;
                *pReal++  = 1.0;
                *pReal++  = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size + detail;
                *pReal++ = srcTexturePos[z][x][1] * size;

                // 2. Triangle
                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TR);
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0.0;
                *pReal++ = 1.0;
                *pReal++ = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size + detail;
                *pReal++ = srcTexturePos[z][x][1] * size;

                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BL);
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0.0;
                *pReal++ = 1.0;
                *pReal++ = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size;
                *pReal++ = srcTexturePos[z][x][1] * size + detail;

                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BR);
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0.0;
                *pReal++ = 1.0;
                *pReal++ = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size + detail;
                *pReal++ = srcTexturePos[z][x][1] * size + detail;

            }
            else
            {
                // 1. Triangle
                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TL);
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0.0;
                *pReal++ = 1.0;
                *pReal++ = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size;
                *pReal++ = srcTexturePos[z][x][1] * size;

                *pReal++  = TileManager::TILE_SIZE * x;
                *pReal++  = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BL);
                *pReal++  = TileManager::TILE_SIZE * (z+1);
                *pReal++  = 0.0;
                *pReal++  = 1.0;
                *pReal++  = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size;
                *pReal++ = srcTexturePos[z][x][1] * size + detail;

                *pReal++  = TileManager::TILE_SIZE * (x+1);
                *pReal++  = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BR);
                *pReal++  = TileManager::TILE_SIZE * (z+1);
                *pReal++  = 0.0;
                *pReal++  = 1.0;
                *pReal++  = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size + detail;
                *pReal++ = srcTexturePos[z][x][1] * size + detail;

                // 2. Triangle
                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BR);
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0.0;
                *pReal++ = 1.0;
                *pReal++ = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size + detail;
                *pReal++ = srcTexturePos[z][x][1] * size + detail;

                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TR);
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0.0;
                *pReal++ = 1.0;
                *pReal++ = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size + detail;
                *pReal++ = srcTexturePos[z][x][1] * size;

                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TL);
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0.0;
                *pReal++ = 1.0;
                *pReal++ = 0.0;
                *pReal++ = srcTexturePos[z][x][0] * size;
                *pReal++ = srcTexturePos[z][x][1] * size;
            }
            --mapX;
        }
    }
    vbuf->unlock();
}

//================================================================================================
//  Create hardware buffers for land.
//================================================================================================
void TileChunk::createLand()
{
    unsigned int numVertices = TileManager::CHUNK_SIZE * TileManager::CHUNK_SIZE * 6;
    // ////////////////////////////////////////////////////////////////////
    // Create VertexData.
    // ////////////////////////////////////////////////////////////////////
    VertexData* vdata = new VertexData();
    vdata->vertexCount = numVertices;
    VertexDeclaration* vdec = vdata->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION           );  offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT3, VES_NORMAL             );  offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);  offset += VertexElement::getTypeSize(VET_FLOAT2);
    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(offset, numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vdata->vertexBufferBinding->setBinding(0, vbuf0);
    // ////////////////////////////////////////////////////////////////////
    // Create Index-buffer (SubMeshes always use indexes)
    // ////////////////////////////////////////////////////////////////////
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    IndexData* idata = mSubMeshLand->indexData;
    idata->indexBuffer = ibuf;
    idata->indexStart = 0;
    idata->indexCount = numVertices;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock (HardwareBuffer::HBL_DISCARD));
    for (unsigned short p=0; numVertices; --numVertices) *pIdx++ = p++;
    ibuf->unlock();

    mSubMeshLand->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshLand->useSharedVertices = false;
    mSubMeshLand->vertexData = vdata;
}

//================================================================================================
// Creates a dummy submesh containing only 1 Triangle.
// Used when there is no water in the scene.
//================================================================================================
void TileChunk::createDummySubMesh(SubMesh* submesh)
{
    delete submesh->vertexData;
    VertexData* vdata = new VertexData();
    vdata->vertexCount = 3;
    vdata->vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(VertexElement::getTypeSize(VET_FLOAT3), vdata->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vdata->vertexBufferBinding->setBinding(0, vbuf0);
    Real *pReal = static_cast<Real*>(vbuf0->lock (HardwareBuffer::HBL_DISCARD));
    for (unsigned int i =0; i < vdata->vertexCount * 3; ++i) *pReal++= 0;
    vbuf0->unlock();
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, vdata->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    IndexData* idata = submesh->indexData;
    idata->indexBuffer= ibuf;
    idata->indexStart = 0;
    idata->indexCount = vdata->vertexCount;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock (HardwareBuffer::HBL_DISCARD));
    *pIdx++ = 2;
    *pIdx++ = 1;
    *pIdx++ = 0;
    ibuf->unlock();
    submesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
    submesh->useSharedVertices = false;
    submesh->vertexData = vdata;
}
