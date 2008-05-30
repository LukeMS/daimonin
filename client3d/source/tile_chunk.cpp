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

/*
=================
* Documentation *
=================
defines:
TileRGB: The RGB data of a tile.
Filter:  The alpha part of a tile.

All tiles are arranged in an Alastexture the following way:
Tile1RGB with Filter0, TileRGB with Filter1, ...
That means on x-axis every tile is twice in the Atlastexture.
Each with different alpha values.

Texture Unit 0: Uses Tile1 (Filter0) alpha off
Texture Unit 1: Uses Tile1 (Filter0) alpha on
Texture Unit 2: Uses Tile1 (Filter1) alpha on
Texture Unit 3: Uses the Shadow-Atlastexture.

To avoid the extrem slow down on older Hardware when using 4 TU we use only
1 TU most of the time. We draw the Tiles with the 4 Layers into a RenderTexture
every time the player moves over a tile border.
The RenderTexture can then be drawn with only 1 TU, so we are only using 4 TU
for a very short time.

To save another few fps, we draw only the tiles that are currently seen.
In the distance you see more tiles, so looking at tbe terrain from above
will end up like this:
X: Player
1: Visible
-: Not visible

11111111111
-111111111-
--1111111--
---11111---
----1X1----

When using an AtlasTexture, you get filtering errors at the tile borders
(strange colored lines around the tile).
But we are using a Rendertexture, so we can arrange the tiles the correct
way and don't get filtering errors. For this the tiles must be arranged the
way they are drawn on the screen. But as we can see above, we need more
tiles in the distance, so it doesnt fit into a squared rendertexture!?
To avoid this we make it this way:

X: Player
1: Inside  the RT
2: Outside the RT

11111112222
-111111222-
--1111122--
---11112---
----1X1----

When mirroring the outside part on y-axis and put it into the invisible part
of the map it will lokk like that.

1111111
2111111
2211111
2221111
22221X1

And voila, no more filtering errors.
*/

#include "define.h"
#include "logger.h"
#include "object_manager.h"
#include "tile_manager.h"

using namespace Ogre;

const char SRC_TEXTURE_NAME[]     = "Atlas_00_";
const char SHADOW_TEX_NAME[]      = "Shadows_";
const char TEXTURE_LAND_NAME[]    = "TileEngine/TexLand";
const char TEXTURE_WATER_NAME[]   = "TileEngine/TexWater";
const char MATERIAL_LAND_NAME[]   = "TileEngine/MatLand";
const char MATERIAL_WATER_NAME[]  = "TileEngine/MatWater";
const char MATERIAL_PAINTER_NAME[]= "TileEngine/MatPainter";
const int  COLS_RENDERTEXTURE = 32; /**< Tiles which not fit into the Rendertuxture are mirrored and drawn into the left part which is empty. **/
//================================================================================================
// The map has its zero-position in the top-left corner:
// (x,z)
//  0,0 1,0 2,0 3,0
//  0,1 1,1 2,1 3,1
//  0,2 1,2 2,2 3,2
//  0,3 1,3 2,3 3,3
//================================================================================================

//================================================================================================
// Holds the vertex positions for mirroring on X and Z axis.
//================================================================================================
const char MIRROR_TABLE[4][4][2]=
{
    {{0,0}, {0,1}, {1,0}, {1,1}}, // No mirror
    {{1,0}, {1,1}, {0,0}, {0,1}}, // Mirror on x-axis.
    {{0,1}, {0,0}, {1,1}, {1,0}}, // Mirror on z-axis.
    {{1,1}, {1,0}, {0,1}, {0,0}}  // Mirror on x and z-axis.
};

//================================================================================================
// .
//================================================================================================
const int CHUNK_START_OFFSET[][TileManager::CHUNK_SIZE_Z]=
{
    {0,0,0,0, 2,2,2,2,2,2, 4,4,4,4,4,4, 6,6,6,6,6,6, 8,8,8,8,8,8, 10,10,10,10}, //   0° camera rotation.
    //{6, 6, 7, 7, 8, 8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17}, //  15° camera rotation.
    //{5, 5, 6, 6, 7, 7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16}, //  30° camera rotation.
};

//================================================================================================
// v0/5+-+ v4
//     |\|
// v1  +-+ v2/3
//
// v0  +-+ v2/3
//     |/|
// v1/4+-+ v5
//
// Layers:
//   1  2.2  1
//     +-+-+
//     |\|/|
// 2.1 +-0-+ 2.1
//     |/|\|
//     +-+-+
//   1  2.2  1
//================================================================================================
void TilePainter::updateVertexBuffer(int rotation)
{
    VertexBufferBinding *vBind = mRenderOp.vertexData->vertexBufferBinding;
    HardwareVertexBufferSharedPtr tvbuf = vBind->getBuffer(1);
    float sizeTile = 1.0f/(float)TileManager::COLS_SUB_TILES;
    float szShadow = sizeTile*2.0;
    Real colLayer0, rowLayer0, colLayer21, rowLayer21, colLayer1, rowLayer1, colLayer22, rowLayer22;
    Real offX, offZ, colShadow, rowShadow;
    int scrollX, scrollZ, m2, m3, m4;
    int tLayer0, tLayer21, tLayer1, tLayer22, tShadow, tLayerMirror, tShadowMirror;
    float *pTex = static_cast<float*>(tvbuf->lock(HardwareBuffer::HBL_DISCARD));
    TileManager::getSingleton().getMapScroll(scrollX, scrollZ);
    for (int z = 0; z < TileManager::CHUNK_SIZE_Z; ++z)
    {
        offZ = ((z+scrollZ)&3) * sizeTile;
        for (int x = CHUNK_START_OFFSET[rotation][z]; x < TileManager::CHUNK_SIZE_X - CHUNK_START_OFFSET[rotation][z]; ++x) // TODO::: add the offset to draw the correct tiles.
        {
            // Tile gfx.
            if ((x-scrollX)&1)
            {
                if (z&1) // bottom-right
                {
                    tLayer0 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TL);
                    tLayer1 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BR);
                    tLayer21= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TR);
                    tLayer22= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BL);
                }
                else  // top-right
                {
                    tLayer0 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BL);
                    tLayer1 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TR);
                    tLayer21= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BR);
                    tLayer22= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TL);
                }
            }
            else
            {
                if (z&1) // bottom-left
                {
                    tLayer0 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TR);
                    tLayer1 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BL);
                    tLayer21= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BR);
                    tLayer22= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TL);
                }
                else  // top-left
                {
                    tLayer0 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BR);
                    tLayer1 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TL);
                    tLayer21= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TR);
                    tLayer22= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BL);
                }
            }
            // Gfx for layer 1+3 are taken from the right half of the src gfx.
            offX = ((x-scrollX)&3) * sizeTile;
            rowLayer0 = (Real)(tLayer0 /4) * sizeTile*4.0f + offZ;
            rowLayer1 = (Real)(tLayer1 /4) * sizeTile*4.0f + offZ;
            rowLayer21= (Real)(tLayer21/4) * sizeTile*4.0f + offZ;
            rowLayer22= (Real)(tLayer22/4) * sizeTile*4.0f + offZ;
            colLayer0 = (Real)(tLayer0 &3) * sizeTile*8.0f + offX;
            colLayer1 = (Real)(tLayer1 &3) * sizeTile*8.0f + offX;
            colLayer21= (Real)(tLayer21&3) * sizeTile*8.0f + offX + 4* sizeTile;
            colLayer22= (Real)(tLayer22&3) * sizeTile*8.0f + offX + 4* sizeTile;

            tShadow = TileManager::getSingleton().getMapShadow(x, z);
            colShadow = ((Real) (tShadow&15)) * szShadow;
            rowShadow = ((Real)((tShadow&127)/16)) * szShadow;
            tShadowMirror = tShadow >> 14;
            if (x < COLS_RENDERTEXTURE)
                tLayerMirror = 0;
            else
            {
                tLayerMirror = 2;
                tShadowMirror = (tShadowMirror+2)&3;
            }
            if ((x+z+tLayerMirror/2)&1)
            {
                m2 = 2;
                m3 = 1;
                m4 = 3;
            }
            else
            {
                m3 = 2;
                m4 = 0;
                m2 = 3;
            }
            // Vertex 0
            *pTex++ = colLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][0][0];
            *pTex++ = rowLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][0][1];
            *pTex++ = colLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][0][0];
            *pTex++ = rowLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][0][1];
            *pTex++ = colLayer22+ sizeTile * MIRROR_TABLE[tLayerMirror][0][0];
            *pTex++ = rowLayer22+ sizeTile * MIRROR_TABLE[tLayerMirror][0][1];
            *pTex++ = colShadow + szShadow * MIRROR_TABLE[tShadowMirror][0][0];
            *pTex++ = rowShadow + szShadow * MIRROR_TABLE[tShadowMirror][0][1];
            // Vertex 1
            *pTex++ = colLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][1][0];
            *pTex++ = rowLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][1][1];
            *pTex++ = colLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][1][0];
            *pTex++ = rowLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][1][1];
            *pTex++ = colLayer22+ sizeTile * MIRROR_TABLE[tLayerMirror][1][0];
            *pTex++ = rowLayer22+ sizeTile * MIRROR_TABLE[tLayerMirror][1][1];
            *pTex++ = colShadow + szShadow * MIRROR_TABLE[tShadowMirror][1][0];
            *pTex++ = rowShadow + szShadow * MIRROR_TABLE[tShadowMirror][1][1];
            // Vertex 2
            *pTex++ = colLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][m2][0];
            *pTex++ = rowLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][m2][1];
            *pTex++ = colLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][m2][0];
            *pTex++ = rowLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][m2][1];
            *pTex++ = colLayer22+ sizeTile * MIRROR_TABLE[tLayerMirror][m2][0];
            *pTex++ = rowLayer22+ sizeTile * MIRROR_TABLE[tLayerMirror][m2][1];
            *pTex++ = colShadow + szShadow * MIRROR_TABLE[tShadowMirror][m2][0];
            *pTex++ = rowShadow + szShadow * MIRROR_TABLE[tShadowMirror][m2][1];
            ///////////
            // Vertex 3
            *pTex++ = colLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][m2][0];
            *pTex++ = rowLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][m2][1];
            *pTex++ = colLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][m2][0];
            *pTex++ = rowLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][m2][1];
            *pTex++ = colLayer21+ sizeTile * MIRROR_TABLE[tLayerMirror][m2][0];
            *pTex++ = rowLayer21+ sizeTile * MIRROR_TABLE[tLayerMirror][m2][1];
            *pTex++ = colShadow + szShadow * MIRROR_TABLE[tShadowMirror][m2][0];
            *pTex++ = rowShadow + szShadow * MIRROR_TABLE[tShadowMirror][m2][1];
            // Vertex 4
            *pTex++ = colLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][m3][0];
            *pTex++ = rowLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][m3][1];
            *pTex++ = colLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][m3][0];
            *pTex++ = rowLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][m3][1];
            *pTex++ = colLayer21+ sizeTile * MIRROR_TABLE[tLayerMirror][m3][0];
            *pTex++ = rowLayer21+ sizeTile * MIRROR_TABLE[tLayerMirror][m3][1];
            *pTex++ = colShadow + szShadow * MIRROR_TABLE[tShadowMirror][m3][0];
            *pTex++ = rowShadow + szShadow * MIRROR_TABLE[tShadowMirror][m3][1];
            // Vertex 5
            *pTex++ = colLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][m4][0];
            *pTex++ = rowLayer0 + sizeTile * MIRROR_TABLE[tLayerMirror][m4][1];
            *pTex++ = colLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][m4][0];
            *pTex++ = rowLayer1 + sizeTile * MIRROR_TABLE[tLayerMirror][m4][1];
            *pTex++ = colLayer21+ sizeTile * MIRROR_TABLE[tLayerMirror][m4][0];
            *pTex++ = rowLayer21+ sizeTile * MIRROR_TABLE[tLayerMirror][m4][1];
            *pTex++ = colShadow + szShadow * MIRROR_TABLE[tShadowMirror][m4][0];
            *pTex++ = rowShadow + szShadow * MIRROR_TABLE[tShadowMirror][m4][1];
        }
    }
    tvbuf->unlock();
}

//================================================================================================
// Create the painter used to draw the blended tiles from alpha-texture to a render-texture.
//================================================================================================
TilePainter::TilePainter(int sumVertices, int absSize)
{
    mUseIdentityProjection = true;
    mUseIdentityView = true;
    mRenderOp.useIndexes = false;
    mRenderOp.vertexData = new VertexData();
    mRenderOp.vertexData->vertexCount = sumVertices;
    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST ;
    VertexDeclaration   *decl = mRenderOp.vertexData->vertexDeclaration;
    VertexBufferBinding *bind = mRenderOp.vertexData->vertexBufferBinding;
    size_t offset = 0;
    decl->addElement(0, offset, VET_FLOAT3, VES_POSITION); offset += VertexElement::getTypeSize(VET_FLOAT3);
    HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(VertexElement::getTypeSize(VET_FLOAT3), mRenderOp.vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    bind->setBinding(0, vbuf);
    float* pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    Real top = 1.0f;
    Real left=-1.0f;
    Real size = 2.0f/TileManager::COLS_SUB_TILES;
    int mirror;
    for (int z = 0; z < TileManager::CHUNK_SIZE_Z; ++z)
    {
        top = 1.0f - z * size;
        for (int x = CHUNK_START_OFFSET[0][z]; x < TileManager::CHUNK_SIZE_X - CHUNK_START_OFFSET[0][z]; ++x)
        {
            if (x>= COLS_RENDERTEXTURE)
            {
                mirror=1;
                top = -1+(z+1)*size;
                left= -1+(x-COLS_RENDERTEXTURE)*size;
            }
            else
            {
                mirror=0;
                left= -1+x*size;
            }
            *pFloat++ = left;
            *pFloat++ = top;
            *pFloat++ = -1;
            *pFloat++ = left;
            *pFloat++ = top - size;
            *pFloat++ = -1;

            if ((x+z+mirror)&1)
            {
                *pFloat++ = left+ size;
                *pFloat++ = top;
                *pFloat++ = -1;
                *pFloat++ = left + size;
                *pFloat++ = top;
                *pFloat++ = -1;
                *pFloat++ = left;
                *pFloat++ = top - size;
                *pFloat++ = -1;
                *pFloat++ = left+ size;
                *pFloat++ = top - size;
                *pFloat++ = -1;
            }
            else
            {
                *pFloat++ = left+ size;
                *pFloat++ = top - size;
                *pFloat++ = -1;
                *pFloat++ = left+ size;
                *pFloat++ = top - size;
                *pFloat++ = -1;
                *pFloat++ = left+ size;
                *pFloat++ = top;
                *pFloat++ = -1;
                *pFloat++ = left;
                *pFloat++ = top;
                *pFloat++ = -1;
            }
        }
    }
    vbuf->unlock();
    const int SUM_TEXTURE_UNITS = 4;
    offset = 0;
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,0); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,1); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,2); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,3); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(VertexElement::getTypeSize(VET_FLOAT2)*SUM_TEXTURE_UNITS, sumVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    bind->setBinding(1, vbuf);
    updateVertexBuffer(0);
    MaterialPtr tmpMaterial;
    tmpMaterial= MaterialManager::getSingleton().getByName(MATERIAL_PAINTER_NAME);
    String strTextureFile= SRC_TEXTURE_NAME + StringConverter::toString(absSize*8*4, 4, '0') + ".png";
    String strShadowFile = SHADOW_TEX_NAME  + StringConverter::toString(absSize*8*2, 4, '0') + ".png";
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(strTextureFile);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName(strTextureFile);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName(strTextureFile);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(3)->setTextureName(strShadowFile);
    setMaterial(MATERIAL_PAINTER_NAME);
    AxisAlignedBox aabInf;
    aabInf.setInfinite();
    setBoundingBox(aabInf);
    /*
            mRenderOp.useIndexes = true;
            mRenderOp.indexData = new IndexData();
            mRenderOp.indexData->indexCount = sumVertices;
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
//
//================================================================================================
LandTiles::LandTiles(int sumVertices)
{
    mRenderOp.useIndexes = false;
    mRenderOp.vertexData = new VertexData();
    mRenderOp.vertexData->vertexCount = sumVertices;
    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST ;
    VertexDeclaration   *vdec = mRenderOp.vertexData->vertexDeclaration;
    VertexBufferBinding *bind = mRenderOp.vertexData->vertexBufferBinding;
    size_t offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);            offset+= VertexElement::getTypeSize(VET_FLOAT3);
//    vdec->addElement(0, offset, VET_FLOAT3, VES_NORMAL);              offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(offset, sumVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    bind->setBinding(0, vbuf);
    setQueryFlags(ObjectManager::QUERY_TILES_LAND_MASK);
    setRenderQueueGroup(RENDER_QUEUE_1);
    setMaterial(MATERIAL_LAND_NAME);
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_LAND_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_LAND_NAME);
    updateVertexBuffer();
    AxisAlignedBox aabInf;
    aabInf.setInfinite();
    setBoundingBox(aabInf);
}

//================================================================================================
//
//================================================================================================
void LandTiles::setVertex(int x, int y, int z, Ogre::Real left, Ogre::Real top, Ogre::Real *pReal)
{
    *pReal++ = TileManager::TILE_SIZE * x;
    *pReal++ = y;
    *pReal++ = TileManager::TILE_SIZE * z;
    *pReal++ = left;
    *pReal++ = top;
/*
    // Calc normals.
    // v1-v3 vertices of a triangle
    Vector3 n1, normal;
    n1 = v2-v1;
    normal = n1.crossProduct(v3-v1);
    normal.normalise();
*/
}

//================================================================================================
//
//================================================================================================
void LandTiles::updateVertexBuffer()
{
    VertexBufferBinding *vBind = mRenderOp.vertexData->vertexBufferBinding;
    HardwareVertexBufferSharedPtr vbuf = vBind->getBuffer(0);
    Real *pReal = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    Real left, top, sizeY, size= 32.0f/1024.0f;
    for (int z = 0; z < TileManager::CHUNK_SIZE_Z; ++z)
    {
        top = z * size;
        sizeY = size;
        for (int x = CHUNK_START_OFFSET[0][z]; x < TileManager::CHUNK_SIZE_X - CHUNK_START_OFFSET[0][z]; ++x)
        {
            if (x >=COLS_RENDERTEXTURE)
            {
                top = 1.0f - z*size;
                left= (x-COLS_RENDERTEXTURE)*size;
                sizeY= -size;
            }
            else
            {
                left= x*size;
            }
            if ((x+z)&1)
            {
                // v0  +-+ v2/3
                //     |/|
                // v1/4+-+ v5
                // 1. Triangle
                setVertex(x  , TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TL), z  , left     , top      , pReal+ 0);
                setVertex(x  , TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BL), z+1, left     , top+sizeY, pReal+ 5);
                setVertex(x+1, TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TR), z  , left+size, top      , pReal+10);
                // 2. Triangle
                setVertex(x+1, TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TR), z  , left+size, top      , pReal+15);
                setVertex(x  , TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BL), z+1, left     , top+sizeY, pReal+20);
                setVertex(x+1, TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BR), z+1, left+size, top+sizeY, pReal+25);
            }
            else
            {
                // v0/5+-+ v4
                //     |\|
                // v1  +-+ v2/3
                // 1. Triangle
                setVertex(x  , TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TL), z  , left     , top      , pReal+ 0);
                setVertex(x  , TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BL), z+1, left     , top+sizeY, pReal+ 5);
                setVertex(x+1, TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BR), z+1, left+size, top+sizeY, pReal+10);
                // 2. Triangle
                setVertex(x+1, TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BR), z+1, left+size, top+sizeY, pReal+15);
                setVertex(x+1, TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TR), z  , left+size, top      , pReal+20);
                setVertex(x  , TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TL), z  , left     , top      , pReal+25);
            }
            pReal += 2*15;
        }
    }
    vbuf->unlock();
}

//================================================================================================
// Constructor.
//================================================================================================
void TileChunk::init(int textureSize)
{
    MaterialPtr tmpMaterial;
    mTextureSize = textureSize;
    mSubTileSize = mTextureSize/TileManager::COLS_SRC_TILES/4;
    mSumVertices = 0;
    for (int i=0; i < TileManager::CHUNK_SIZE_Z; ++i)
        mSumVertices+= TileManager::CHUNK_SIZE_X - 2*CHUNK_START_OFFSET[0][i];
    mSumVertices *= 6;
    // ////////////////////////////////////////////////////////////////////
    // Build the land-tiles.
    // ////////////////////////////////////////////////////////////////////
    mTexLand = TextureManager::getSingleton().createManual(TEXTURE_LAND_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, textureSize, textureSize, 3, PF_R8G8B8, TU_RENDERTARGET);
    tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_LAND_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_LAND_NAME);
    mLandTiles= new LandTiles(mSumVertices);
    // ////////////////////////////////////////////////////////////////////
    // Build the water-tiles.
    // ////////////////////////////////////////////////////////////////////
    mMeshWater = MeshManager::getSingleton().createManual("Mesh_Water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    createDummySubMesh(mMeshWater->createSubMesh()); // We don't have the tile datas yet, so no need to waste time in creating the tiles.
    mMeshWater->_setBounds(AxisAlignedBox(0,0,0,TileManager::TILE_SIZE * TileManager::CHUNK_SIZE_X,MAX_TERRAIN_HEIGHT,TileManager::TILE_SIZE * TileManager::CHUNK_SIZE_Z));
    mMeshWater->load();
    mEntityWater = TileManager::getSingleton().getSceneManager()->createEntity("Entity_Water", "Mesh_Water");
    mEntityWater->setQueryFlags(ObjectManager::QUERY_TILES_WATER_MASK);
    mEntityWater->setRenderQueueGroup(RENDER_QUEUE_8);
    mTexWater = TextureManager::getSingleton().createManual(TEXTURE_WATER_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, mTextureSize/TileManager::COLS_SRC_TILES, mTextureSize/TileManager::COLS_SRC_TILES, 0, PF_A8R8G8B8, TU_STATIC);
    tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_WATER_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_WATER_NAME);
    mEntityWater->setMaterialName(MATERIAL_WATER_NAME);
    // ////////////////////////////////////////////////////////////////////
    // Attach the tiles to a scenenode.
    // ////////////////////////////////////////////////////////////////////
    SceneNode *node= TileManager::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();
    node->setPosition(0, 0, 0);
    node->attachObject(mLandTiles);
    node->attachObject(mEntityWater);
    mPainter= new TilePainter(mSumVertices, mSubTileSize);
    loadAtlasTexture(0);
    updatePainter();
}

//================================================================================================
// Set a new material.
//================================================================================================
void TileChunk::loadAtlasTexture(int group)
{
    String filename;
    Image image;
    filename = "Atlas_" + StringConverter::toString(group, 2, '0') + "_" + StringConverter::toString(mTextureSize, 4, '0') + ".png";
    image.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    uint32 *src = (uint32 *) image.getData();
    // ////////////////////////////////////////////////////////////////////
    // Copy the water tile to a separate texture
    // (Water needs own texture for the scroll animation).
    // ////////////////////////////////////////////////////////////////////
    int sizeTile = mTextureSize/TileManager::COLS_SRC_TILES;
    mTexWater->getBuffer()->lock(HardwareBuffer::HBL_DISCARD);
    PixelBox pb = mTexWater->getBuffer()->getCurrentLock();
    uint32 *dst = (uint32 *)pb.data;
    for (int y = 0; y < sizeTile; ++y)
    {
        for (int x = 0; x < sizeTile; ++x)
            *dst++ = *src++ | 0xff000000;
        src+=(mTextureSize - sizeTile);
    }
    mTexWater->getBuffer()->unlock();
    // ////////////////////////////////////////////////////////////////////
    // Copy the new atlastexture over the old one.
    // ////////////////////////////////////////////////////////////////////
    MaterialPtr tmpMaterial = mPainter->getMaterial();
    TexturePtr texPainter = TextureManager::getSingleton().getByName(tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName());
    pb = texPainter->getBuffer()->lock(Box(0, 0, mTextureSize, mTextureSize), HardwareBuffer::HBL_DISCARD);
    memcpy(pb.data, image.getData(), mTextureSize * mTextureSize * sizeof(uint32));
    texPainter->getBuffer()->unlock();
    updatePainter();
}

//================================================================================================
// Update the rendertexture (e.g after a mapscroll).
//================================================================================================
void TileChunk::updatePainter()
{
    mPainter->updateVertexBuffer(0);
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
    viewport->setDimensions(0.0f, 0.0f, 1.0f, 1.0f);
    // Render only the queues in the special case list.
    TileManager::getSingleton().getSceneManager()->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_INCLUDE);
    TileManager::getSingleton().getSceneManager()->addSpecialCaseRenderQueue(RENDER_QUEUE_MAIN);
    mPainter->setRenderQueueGroup(RENDER_QUEUE_MAIN);
    renderTarget->update();
//renderTarget->writeContentsToFile("Painter.png");
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
    mMeshWater.setNull();
    mTexWater.setNull();
    mTexLand.setNull();
    delete mPainter;
    delete mLandTiles;
}

//================================================================================================
// Change a chunk.
//================================================================================================
void TileChunk::change()
{
    long time = Root::getSingleton().getTimer()->getMicroseconds();
    updatePainter();
    mLandTiles->updateVertexBuffer();
    createWater();
    Logger::log().error() << "Time to change terrain: " << (double)(Root::getSingleton().getTimer()->getMicroseconds() - time)/1000 << " ms";
}

//================================================================================================
// Create hardware buffers for water.
//================================================================================================
void TileChunk::createWater()
{
    SubMesh *submesh = mMeshWater->getSubMesh(0);
    createDummySubMesh(submesh);

    return;

    // ////////////////////////////////////////////////////////////////////
    // Count the Vertices in this chunk.
    // ////////////////////////////////////////////////////////////////////
    unsigned int numVertices = 0;
    for (int x = 0; x < TileManager::CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE_Z; ++z)
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
        createDummySubMesh(submesh);
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Create VertexData.
    // ////////////////////////////////////////////////////////////////////
    int mapX, mapZ;
    delete submesh->vertexData;
    VertexData* vdata = new VertexData();
    vdata->vertexCount = numVertices;
    VertexDeclaration* vdec = vdata->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);               offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT3, VES_NORMAL);                 offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(offset, numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vdata->vertexBufferBinding->setBinding(0, vbuf0);
    static Real offsetWave = 0.03;
    static Real WaveHigh = 0;
    WaveHigh+= offsetWave;
    if (WaveHigh >1.7 || WaveHigh < -1.7) offsetWave*=-1;
    Real* pReal = static_cast<Real*>(vbuf0->lock(HardwareBuffer::HBL_DISCARD));
    Real q1, q2, offX, offZ;
    for (int x = 0; x < TileManager::CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE_Z; ++z)
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
                *pReal++ = 0.25;

                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = 0.25;
                *pReal++ = offZ;
                // 2. Triangle
                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = 0.25;
                *pReal++ = offZ;

                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX;
                *pReal++ = 0.25;

                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++= q1;
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = 0.25;
                *pReal++ = 0.25;
            }
        }
    }
    vbuf0->unlock();
    // ////////////////////////////////////////////////////////////////////
    // Create Index-buffer
    // (SubMeshes always use indexes)
    // ////////////////////////////////////////////////////////////////////
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    IndexData* idata = submesh->indexData;
    idata->indexBuffer = ibuf;
    idata->indexStart = 0;
    idata->indexCount = numVertices;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
    for (unsigned short p=0; numVertices; --numVertices) *pIdx++ = p++;
    ibuf->unlock();
    submesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
    submesh->useSharedVertices = false;
    submesh->vertexData = vdata;
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
    Real *pReal = static_cast<Real*>(vbuf0->lock(HardwareBuffer::HBL_DISCARD));
    for (unsigned int i =0; i < vdata->vertexCount * 3; ++i) *pReal++= 0;
    vbuf0->unlock();
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, vdata->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    IndexData* idata = submesh->indexData;
    idata->indexBuffer= ibuf;
    idata->indexStart = 0;
    idata->indexCount = vdata->vertexCount;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
    *pIdx++ = 2;
    *pIdx++ = 1;
    *pIdx++ = 0;
    ibuf->unlock();
    submesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
    submesh->useSharedVertices = false;
    submesh->vertexData = vdata;
}
