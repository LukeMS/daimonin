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

#include "logger.h"
#include "tile_manager.h"
#include "resourceloader.h"

using namespace Ogre;

const char SRC_TEXTURE_NAME[]     = "Atlas";
const char TEXTURE_LAND_NAME[]    = "TileEngine/TexLand";
const char TEXTURE_WATER_NAME[]   = "water_00_128.png";
const char MATERIAL_LAND_NAME[]   = "TileEngine/MatLand";
const char MATERIAL_WATER_NAME[]  = "TileEngine/MatWater";
const int  SUM_QUAD_VERTICES = 6;  /**< A quadrat is build out of 2 triangles with 3 vertices each. **/
const unsigned int SUM_CAMERA_POS  = 7;
const Real TEX_SIZE = 1.0f/((float)TileManager::COLS_SRC_TILES*4.0f);
const Real SHADOW_SIZE  = TEX_SIZE/2;
const Real SHADOW_SPACE = SHADOW_SIZE/8.0f;

//================================================================================================
// The map has its zero-position in the top-left corner:
// (x,z)
//  0,0 1,0 2,0 3,0
//  0,1 1,1 2,1 3,1
//================================================================================================

//================================================================================================
// Holds the vertex positions for mirroring on X and Z axis in the atlastexture.
//================================================================================================
const Real MIRROR[4][4][2]=
{
    {{0.0f,0.0f}, {0.0f,1.0f}, {1.0f,0.0f}, {1.0f,1.0f}}, // No mirror
    {{1.0f,0.0f}, {1.0f,1.0f}, {0.0f,0.0f}, {0.0f,1.0f}}, // Mirror on x-axis.
    {{0.0f,1.0f}, {0.0f,0.0f}, {1.0f,1.0f}, {1.0f,0.0f}}, // Mirror on z-axis.
    {{1.0f,1.0f}, {1.0f,0.0f}, {0.0f,1.0f}, {0.0f,0.0f}}  // Mirror on x and z-axis.
};

//================================================================================================
// Holds the x-offset for each row of tiles.
// This prevents the drawing of tiles which are outside the field of view.
// Each line holds a range of the camera rotation.
//================================================================================================
const int CHUNK_START_OFFSET[SUM_CAMERA_POS][TileManager::CHUNK_SIZE_Z]=
{
    { 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8,12,16,14,12}, // -45°
    { 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8,10,10}, // -30°
    { 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8}, // -15°
    { 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8,10,10,10,10}, //   0°
    { 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8}, //  15°
    { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5}, //  30°
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //  45°
};

//================================================================================================
// Sum of tiles in a row (Caluclated from CHUNK_START_OFFSET)
//================================================================================================
int CHUNK_X_LENGTH[TileManager::CHUNK_SIZE_Z];

//================================================================================================
// Constructor.
//================================================================================================
void TileChunk::init(int textureSize, int queryMaskLand, int queryMaskWater)
{
    int cameraStandardPos = 3; // 0° rotation of the camera in CHUNK_START_OFFSET[][] table.
    mTextureSize = textureSize;
    mCameraRotation = cameraStandardPos;
    int sumVertices = 0;
    for (int i=0; i < TileManager::CHUNK_SIZE_Z; ++i)
    {
        CHUNK_X_LENGTH[i] = TileManager::CHUNK_SIZE_X - 2*CHUNK_START_OFFSET[cameraStandardPos][i];
        sumVertices+= CHUNK_X_LENGTH[i];
    }
    sumVertices*= SUM_QUAD_VERTICES;
    // ////////////////////////////////////////////////////////////////////
    // Build the land-tiles.
    // ////////////////////////////////////////////////////////////////////
    mMeshLand = MeshManager::getSingleton().createManual("Mesh_Land", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    SubMesh *submesh = mMeshLand->createSubMesh();
    VertexData *vData = new VertexData();
    vData->vertexCount = sumVertices;
    VertexDeclaration *vdec = vData->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);               offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT3, VES_NORMAL);                 offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 2); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 3); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(offset, sumVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vData->vertexBufferBinding->setBinding(0, vbuf);
    createIndexData(submesh, sumVertices);
    submesh->vertexData = vData;
    mMeshLand->_setBounds(AxisAlignedBox(0,0,0,TileManager::TILE_SIZE * TileManager::CHUNK_SIZE_X,MAX_TERRAIN_HEIGHT,TileManager::TILE_SIZE * TileManager::CHUNK_SIZE_Z));
    mMeshLand->load();
    mEntityLand = TileManager::getSingleton().getSceneManager()->createEntity("Entity_Land", "Mesh_Land");
    loadAtlasTexture(0, -1);
    mEntityLand->setMaterialName(MATERIAL_LAND_NAME);
    mEntityLand->setQueryFlags(queryMaskLand);
    mEntityLand->setRenderQueueGroup(RENDER_QUEUE_1);
    // ////////////////////////////////////////////////////////////////////
    // Build the water-tiles.
    // ////////////////////////////////////////////////////////////////////
    mMeshWater = MeshManager::getSingleton().createManual("Mesh_Water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    createDummySubMesh(mMeshWater->createSubMesh()); // We don't have the tile data yet, so no need to waste time in creating the tiles.
    mMeshWater->_setBounds(AxisAlignedBox(0,0,0,TileManager::TILE_SIZE * TileManager::CHUNK_SIZE_X,MAX_TERRAIN_HEIGHT,TileManager::TILE_SIZE * TileManager::CHUNK_SIZE_Z));
    mMeshWater->load();
    mEntityWater = TileManager::getSingleton().getSceneManager()->createEntity("Entity_Water", "Mesh_Water");
    loadAtlasTexture(-1, 0);
    mEntityWater->setMaterialName(MATERIAL_WATER_NAME);
    mEntityWater->setQueryFlags(queryMaskWater);
    mEntityWater->setRenderQueueGroup(RENDER_QUEUE_8);
    // ////////////////////////////////////////////////////////////////////
    // Attach the tiles to a scenenode.
    // ////////////////////////////////////////////////////////////////////
    SceneNode *node= TileManager::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();
    node->setPosition(0,0,0);
    node->attachObject(mEntityLand);
    node->attachObject(mEntityWater);
}

//================================================================================================
// Set a new material.
//================================================================================================
void TileChunk::loadAtlasTexture(int landGroup, int waterGroup)
{
    if (landGroup >=0)
    {
        String textureFile = SRC_TEXTURE_NAME;
        textureFile+= "_"+ StringConverter::toString(landGroup, 2, '0');
        textureFile+= "_"+ StringConverter::toString(mTextureSize/TileManager::COLS_SRC_TILES*8, 4, '0') + ".png";
        MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_LAND_NAME);
        for (int i=0; i < tmpMaterial->getTechnique(0)->getPass(0)->getNumTextureUnitStates(); ++i)
            tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(i)->setTextureName(textureFile);
    }
    if (waterGroup >=0)
    {
        String textureFile = TEXTURE_WATER_NAME;
        MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_WATER_NAME);
        for (int i=0; i < tmpMaterial->getTechnique(0)->getPass(0)->getNumTextureUnitStates(); ++i)
            tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(i)->setTextureName(textureFile);
    }
}

//================================================================================================
//
//================================================================================================
void TileChunk::rotate(Real cameraAngle)
{
    unsigned int rotation = (unsigned int)((cameraAngle + 45.0f) / 15);
    if (mCameraRotation != rotation && rotation < SUM_CAMERA_POS)
    {
        mCameraRotation = rotation;
        update();
    }
}

//================================================================================================
// Free all resources.
//================================================================================================
void TileChunk::freeRecources()
{
    mMeshLand.setNull();
    mMeshWater.setNull();
}

//================================================================================================
// Create Index-buffer (SubMeshes always use indexes).
//================================================================================================
void TileChunk::createIndexData(SubMesh *submesh, int sumVertices)
{
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, sumVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    IndexData* idata = submesh->indexData;
    idata->indexBuffer = ibuf;
    idata->indexStart = 0;
    idata->indexCount = sumVertices;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
    for (unsigned short p=0; sumVertices; --sumVertices) *pIdx++ = p++;
    ibuf->unlock();
    submesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
    submesh->useSharedVertices = false;
}

//================================================================================================
// Calc the normals for a triangle.
//================================================================================================
void TileChunk::calcNormal(Real x1, Real z1, Real x2, Real z2, Real x3, Real z3)
{
    mVec1.x = TileManager::TILE_SIZE * x1;
    mVec1.y = TileManager::getSingleton().getMapHeight((int)x1, (int)z1, TileManager::VERTEX_TL);
    mVec1.z = TileManager::TILE_SIZE * z1;
    mVec2.x = TileManager::TILE_SIZE * x2;
    mVec2.y = TileManager::getSingleton().getMapHeight((int)x2, (int)z2, TileManager::VERTEX_TL);
    mVec2.z = TileManager::TILE_SIZE * z2;
    mVec3.x = TileManager::TILE_SIZE * x3;
    mVec3.y = TileManager::getSingleton().getMapHeight((int)x3, (int)z3, TileManager::VERTEX_TL);
    mVec3.z = TileManager::TILE_SIZE * z3;
    mNormal = mVec2 - mVec1;
    mNormal = mNormal.crossProduct(mVec3 - mVec1);
    mNormal.normalise();
}

//================================================================================================
// Set all data for a vertex
//================================================================================================
void TileChunk::setVertex(Vector3 &pos, Real posTexX, Real posTexZ, Real posShadowX, Real posShadowZ, int p)
{
    static const int TEXTURE_UNIT_SORT[][6]=
    {
        {0,1, 2,3, 4,5}, {0,1, 4,5, 2,3},
        {2,3, 0,1, 4,5}, {2,3, 4,5, 0,1},
        {4,5, 0,1, 2,3}, {4,5, 2,3, 0,1},
    };
    // Pos in the world.
    *mPosVBuf++ = pos.x;
    *mPosVBuf++ = pos.y;
    *mPosVBuf++ = pos.z;
    // Normals.
    *mPosVBuf++ = mNormal.x;
    *mPosVBuf++ = mNormal.y;
    *mPosVBuf++ = mNormal.z;
    // Texture positons in the atlastexture.
    // Sorting: Highest gfx-nr in atlas texture gets the highest texture unit.
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[p][0]] + posTexX * TEX_SIZE;
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[p][1]] + posTexZ * TEX_SIZE;
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[p][2]] + posTexX * TEX_SIZE;
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[p][3]] + posTexZ * TEX_SIZE;
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[p][4]] + posTexX * TEX_SIZE;
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[p][5]] + posTexZ * TEX_SIZE;
    // Shadows
    *mPosVBuf++ = mTexPosInAtlas[6] + posShadowX * (SHADOW_SIZE-SHADOW_SPACE)*2;
    *mPosVBuf++ = mTexPosInAtlas[7] + posShadowZ * (SHADOW_SIZE-SHADOW_SPACE)*2;
}

//================================================================================================
// TextureUnit sorting: The higher the gfx-nr, the higher the texture unit.
//================================================================================================
int TileChunk::calcTextureUnitSorting(int l0, int l1, int l2)
{
    if (l0 <= l1)
    {
        if (l0 <= l2)
        {
            if (l1 <= l2) return 0;
            return 1;
        }
        return 4;
    }
    if (l0 <= l2) return 2;
    if (l1 <= l2) return 3;
    return 5;
}

//================================================================================================
//  Triangles:             Layers:
// v0/5+-+ v4             1   2   1
//     |\|                  +-+-+
// v1  +-+ v2/3             |\|/|
//                        2 +-0-+ 2
// v0  +-+ v2/3             |/|\|
//     |/|                  +-+-+
// v1/4+-+ v5             1   2   1
//================================================================================================
void TileChunk::changeLand()
{
    // Positions of the texture-units in the atlas texture.
    static const Real TU_POS[8][4]=
    {
        // X-Positions
        {(0+ 0) * TEX_SIZE, (1+ 0) * TEX_SIZE, (2+ 0) * TEX_SIZE, (3+ 0) * TEX_SIZE}, // Texture Unit 0
        {(1+ 4) * TEX_SIZE, (2+ 4) * TEX_SIZE, (3+ 4) * TEX_SIZE, (0+ 4) * TEX_SIZE}, // Texture Unit 1
        {(0+16) * TEX_SIZE, (1+16) * TEX_SIZE, (2+16) * TEX_SIZE, (3+16) * TEX_SIZE}, // Texture Unit 2.1
        {(1+20) * TEX_SIZE, (2+20) * TEX_SIZE, (3+20) * TEX_SIZE, (0+20) * TEX_SIZE}, // Texture Unit 2.2
        //Y-Positions
        {0 * TEX_SIZE, 1 * TEX_SIZE, 2 * TEX_SIZE, 3 * TEX_SIZE}, // Texture Unit 0
        {1 * TEX_SIZE, 2 * TEX_SIZE, 3 * TEX_SIZE, 0 * TEX_SIZE}, // Texture Unit 1
        {1 * TEX_SIZE, 2 * TEX_SIZE, 3 * TEX_SIZE, 0 * TEX_SIZE}, // Texture Unit 2.1 (vertical)
        {0 * TEX_SIZE, 1 * TEX_SIZE, 2 * TEX_SIZE, 3 * TEX_SIZE}, // Texture Unit 2.2 (horizontal)
    };
    int scrollX, scrollZ, texUnit0, texUnit1, texUnit2, texUnitShadow, mirror, sortPos, offX, offZ;

    HardwareVertexBufferSharedPtr vbuf = mMeshLand->getSubMesh(0)->vertexData->vertexBufferBinding->getBuffer(0);
    mPosVBuf = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    TileManager::getSingleton().getMapScroll(scrollX, scrollZ);
    for (int z = 0; z < TileManager::CHUNK_SIZE_Z; ++z)
    {
        offZ = (z+scrollZ)&3;
        for (int x = CHUNK_START_OFFSET[mCameraRotation][z]; x < CHUNK_START_OFFSET[mCameraRotation][z]+CHUNK_X_LENGTH[z]; ++x)
        {
            offX = (x-scrollX)&3;
            // Shadow gfx
            texUnitShadow = TileManager::getSingleton().getMapShadow(x, z);
            mTexPosInAtlas[7] = (Real) ((texUnitShadow&127)/7);
            mTexPosInAtlas[6] = (Real)(((texUnitShadow&127) - mTexPosInAtlas[7]*7)*4*SHADOW_SIZE +0.5f + 3*SHADOW_SIZE +  SHADOW_SPACE);
            mTexPosInAtlas[7] = mTexPosInAtlas[7]*4*SHADOW_SIZE + 3*SHADOW_SIZE + SHADOW_SPACE;
            mirror = texUnitShadow >> 14;

            // Tile gfx.
            if ((x-scrollX)&1)
            {
                if (z&1) // bottom-right subtile.
                {
                    texUnit0 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TL);
                    mTexPosInAtlas[0]= (Real)(texUnit0 &1) * TEX_SIZE*8.0f + TU_POS[0][offX];
                    mTexPosInAtlas[1]= (Real)(texUnit0 /2) * TEX_SIZE*4.0f + TU_POS[4][offZ];
                    texUnit1 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BR);
                    mTexPosInAtlas[2]= (Real)(texUnit1 &1) * TEX_SIZE*8.0f + TU_POS[1][offX];
                    mTexPosInAtlas[3]= (Real)(texUnit1 /2) * TEX_SIZE*4.0f + TU_POS[5][offZ];
                    // Triangle 1
                    texUnit2= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BL);
                    mTexPosInAtlas[4]= (Real)(texUnit2&1) * TEX_SIZE*8.0f + TU_POS[2][offX];
                    mTexPosInAtlas[5]= (Real)(texUnit2/2) * TEX_SIZE*4.0f + TU_POS[6][offZ];
                    sortPos = calcTextureUnitSorting(texUnit0, texUnit1, texUnit2);
                    calcNormal(x+MIRROR[0][0][X], z+MIRROR[0][0][Z],  x+MIRROR[0][1][X], z+MIRROR[0][1][Z],  x+MIRROR[0][3][X], z+MIRROR[0][3][Z]);
                    setVertex(mVec1, MIRROR[0][0][X], MIRROR[0][0][Z], MIRROR[mirror][0][X], MIRROR[mirror][0][Z], sortPos);
                    setVertex(mVec2, MIRROR[0][1][X], MIRROR[0][1][Z], MIRROR[mirror][1][X], MIRROR[mirror][1][Z], sortPos);
                    setVertex(mVec3, MIRROR[0][3][X], MIRROR[0][3][Z], MIRROR[mirror][3][X], MIRROR[mirror][3][Z], sortPos);
                    // Triangle 2
                    texUnit2= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TR);
                    mTexPosInAtlas[4]= (Real)(texUnit2&1) * TEX_SIZE*8.0f + TU_POS[3][offX];
                    mTexPosInAtlas[5]= (Real)(texUnit2/2) * TEX_SIZE*4.0f + TU_POS[7][offZ];
                    sortPos = calcTextureUnitSorting(texUnit0, texUnit1, texUnit2);
                    calcNormal(x+MIRROR[0][3][X], z+MIRROR[0][3][Z],  x+MIRROR[0][2][X], z+MIRROR[0][2][Z],  x+MIRROR[0][0][X], z+MIRROR[0][0][Z]);
                    setVertex(mVec1, MIRROR[0][3][X], MIRROR[0][3][Z], MIRROR[mirror][3][X], MIRROR[mirror][3][Z], sortPos);
                    setVertex(mVec2, MIRROR[0][2][X], MIRROR[0][2][Z], MIRROR[mirror][2][X], MIRROR[mirror][2][Z], sortPos);
                    setVertex(mVec3, MIRROR[0][0][X], MIRROR[0][0][Z], MIRROR[mirror][0][X], MIRROR[mirror][0][Z], sortPos);
                }
                else  // top-right subtile.
                {
                    texUnit0 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BL);
                    mTexPosInAtlas[0]= (Real)(texUnit0 &1) * TEX_SIZE*8.0f + TU_POS[0][offX];
                    mTexPosInAtlas[1]= (Real)(texUnit0 /2) * TEX_SIZE*4.0f + TU_POS[4][offZ];
                    texUnit1 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TR);
                    mTexPosInAtlas[2]= (Real)(texUnit1 &1) * TEX_SIZE*8.0f + TU_POS[1][offX];
                    mTexPosInAtlas[3]= (Real)(texUnit1 /2) * TEX_SIZE*4.0f + TU_POS[5][offZ];
                    // Triangle 1
                    texUnit2= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TL);
                    mTexPosInAtlas[4]= (Real)(texUnit2&1) * TEX_SIZE*8.0f + TU_POS[2][offX];
                    mTexPosInAtlas[5]= (Real)(texUnit2/2) * TEX_SIZE*4.0f + TU_POS[6][offZ];
                    sortPos = calcTextureUnitSorting(texUnit0, texUnit1, texUnit2);
                    calcNormal(x+MIRROR[0][0][X], z+MIRROR[0][0][Z],  x+MIRROR[0][1][X], z+MIRROR[0][1][Z],  x+MIRROR[0][2][X], z+MIRROR[0][2][Z]);
                    setVertex(mVec1, MIRROR[0][0][X], MIRROR[0][0][Z], MIRROR[mirror][0][X], MIRROR[mirror][0][Z], sortPos);
                    setVertex(mVec2, MIRROR[0][1][X], MIRROR[0][1][Z], MIRROR[mirror][1][X], MIRROR[mirror][1][Z], sortPos);
                    setVertex(mVec3, MIRROR[0][2][X], MIRROR[0][2][Z], MIRROR[mirror][2][X], MIRROR[mirror][2][Z], sortPos);
                    // Triangle 2
                    texUnit2= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BR);
                    mTexPosInAtlas[4]= (Real)(texUnit2&1) * TEX_SIZE*8.0f + TU_POS[3][offX];
                    mTexPosInAtlas[5]= (Real)(texUnit2/2) * TEX_SIZE*4.0f + TU_POS[7][offZ];
                    sortPos = calcTextureUnitSorting(texUnit0, texUnit1, texUnit2);
                    calcNormal(x+MIRROR[0][2][X], z+MIRROR[0][2][Z],  x+MIRROR[0][1][X], z+MIRROR[0][1][Z],  x+MIRROR[0][3][X], z+MIRROR[0][3][Z]);
                    setVertex(mVec1, MIRROR[0][2][X], MIRROR[0][2][Z], MIRROR[mirror][2][X], MIRROR[mirror][2][Z], sortPos);
                    setVertex(mVec2, MIRROR[0][1][X], MIRROR[0][1][Z], MIRROR[mirror][1][X], MIRROR[mirror][1][Z], sortPos);
                    setVertex(mVec3, MIRROR[0][3][X], MIRROR[0][3][Z], MIRROR[mirror][3][X], MIRROR[mirror][3][Z], sortPos);
                }
            }
            else
            {
                if (z&1) // bottom-left subtile.
                {
                    texUnit0 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TR);
                    mTexPosInAtlas[0]= (Real)(texUnit0 &1) * TEX_SIZE*8.0f + TU_POS[0][offX];
                    mTexPosInAtlas[1]= (Real)(texUnit0 /2) * TEX_SIZE*4.0f + TU_POS[4][offZ];
                    texUnit1 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BL);
                    mTexPosInAtlas[2]= (Real)(texUnit1 &1) * TEX_SIZE*8.0f + TU_POS[1][offX];
                    mTexPosInAtlas[3]= (Real)(texUnit1 /2) * TEX_SIZE*4.0f + TU_POS[5][offZ];
                    // Triangle 1
                    texUnit2= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TL);
                    mTexPosInAtlas[4]= (Real)(texUnit2&1) * TEX_SIZE*8.0f + TU_POS[3][offX];
                    mTexPosInAtlas[5]= (Real)(texUnit2/2) * TEX_SIZE*4.0f + TU_POS[7][offZ];
                    sortPos = calcTextureUnitSorting(texUnit0, texUnit1, texUnit2);
                    calcNormal(x+MIRROR[0][0][X], z+MIRROR[0][0][Z],  x+MIRROR[0][1][X], z+MIRROR[0][1][Z],  x+MIRROR[0][2][X], z+MIRROR[0][2][Z]);
                    setVertex(mVec1, MIRROR[0][0][X], MIRROR[0][0][Z], MIRROR[0][0][X], MIRROR[mirror][0][Z], sortPos);
                    setVertex(mVec2, MIRROR[0][1][X], MIRROR[0][1][Z], MIRROR[0][1][X], MIRROR[mirror][1][Z], sortPos);
                    setVertex(mVec3, MIRROR[0][2][X], MIRROR[0][2][Z], MIRROR[0][2][X], MIRROR[mirror][2][Z], sortPos);
                    // Triangle 2
                    texUnit2= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BR);
                    mTexPosInAtlas[4]= (Real)(texUnit2&1) * TEX_SIZE*8.0f + TU_POS[2][offX];
                    mTexPosInAtlas[5]= (Real)(texUnit2/2) * TEX_SIZE*4.0f + TU_POS[6][offZ];
                    sortPos = calcTextureUnitSorting(texUnit0, texUnit1, texUnit2);
                    calcNormal(x+MIRROR[0][2][X], z+MIRROR[0][2][Z],  x+MIRROR[0][1][X], z+MIRROR[0][1][Z],  x+MIRROR[0][3][X], z+MIRROR[0][3][Z]);
                    setVertex(mVec1, MIRROR[0][2][X], MIRROR[0][2][Z], MIRROR[0][2][X], MIRROR[mirror][2][Z], sortPos);
                    setVertex(mVec2, MIRROR[0][1][X], MIRROR[0][1][Z], MIRROR[0][1][X], MIRROR[mirror][1][Z], sortPos);
                    setVertex(mVec3, MIRROR[0][3][X], MIRROR[0][3][Z], MIRROR[0][3][X], MIRROR[mirror][3][Z], sortPos);
                }
                else  // top-left subtile.
                {
                    texUnit0 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BR);
                    mTexPosInAtlas[0]= (Real)(texUnit0 &1) * TEX_SIZE*8.0f + TU_POS[0][offX];
                    mTexPosInAtlas[1]= (Real)(texUnit0 /2) * TEX_SIZE*4.0f + TU_POS[4][offZ];
                    texUnit1 = TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TL);
                    mTexPosInAtlas[2]= (Real)(texUnit1 &1) * TEX_SIZE*8.0f + TU_POS[1][offX];
                    mTexPosInAtlas[3]= (Real)(texUnit1 /2) * TEX_SIZE*4.0f + TU_POS[5][offZ];
                    // Triangle 1
                    texUnit2= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_BL);
                    mTexPosInAtlas[4]= (Real)(texUnit2&1) * TEX_SIZE*8.0f + TU_POS[3][offX];
                    mTexPosInAtlas[5]= (Real)(texUnit2/2) * TEX_SIZE*4.0f + TU_POS[7][offZ];
                    sortPos = calcTextureUnitSorting(texUnit0, texUnit1, texUnit2);
                    calcNormal(x+MIRROR[0][0][X], z+MIRROR[0][0][Z],  x+MIRROR[0][1][X], z+MIRROR[0][1][Z],  x+MIRROR[0][3][X], z+MIRROR[0][3][Z]);
                    setVertex(mVec1, MIRROR[0][0][X], MIRROR[0][0][Z], MIRROR[0][0][X], MIRROR[mirror][0][Z], sortPos);
                    setVertex(mVec2, MIRROR[0][1][X], MIRROR[0][1][Z], MIRROR[0][1][X], MIRROR[mirror][1][Z], sortPos);
                    setVertex(mVec3, MIRROR[0][3][X], MIRROR[0][3][Z], MIRROR[0][3][X], MIRROR[mirror][3][Z], sortPos);
                    // Triangle 2
                    texUnit2= TileManager::getSingleton().getMapGfx(x, z, TileManager::VERTEX_TR);
                    mTexPosInAtlas[4]= (Real)(texUnit2&1) * TEX_SIZE*8.0f + TU_POS[2][offX];
                    mTexPosInAtlas[5]= (Real)(texUnit2/2) * TEX_SIZE*4.0f + TU_POS[6][offZ];
                    sortPos = calcTextureUnitSorting(texUnit0, texUnit1, texUnit2);
                    calcNormal(x+MIRROR[0][3][X], z+MIRROR[0][3][Z],  x+MIRROR[0][2][X], z+MIRROR[0][2][Z],  x+MIRROR[0][0][X], z+MIRROR[0][0][Z]);
                    setVertex(mVec1, MIRROR[0][3][X], MIRROR[0][3][Z], MIRROR[0][3][X], MIRROR[mirror][3][Z], sortPos);
                    setVertex(mVec2, MIRROR[0][2][X], MIRROR[0][2][Z], MIRROR[0][2][X], MIRROR[mirror][2][Z], sortPos);
                    setVertex(mVec3, MIRROR[0][0][X], MIRROR[0][0][Z], MIRROR[0][0][X], MIRROR[mirror][0][Z], sortPos);
                }
            }
        }
    }
    vbuf->unlock();
}

//================================================================================================
// Change a chunk.
//================================================================================================
void TileChunk::update()
{
    changeLand();
    changeWater();
}

//================================================================================================
// Create hardware buffers for water.
//================================================================================================
void TileChunk::changeWater()
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
    createIndexData(submesh, numVertices);
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
