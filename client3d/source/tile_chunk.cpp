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

#include <OgreEntity.h>
#include <OgreSubMesh.h>
#include <OgreSubEntity.h>
#include <OgreMeshManager.h>
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include <OgreStringConverter.h> // needed for Ogre 1.4.9.
#include "logger.h"
#include "tile_manager.h"

#define FAKE_SHADOWS 1 // Use the normal vector to send a shadow value to the shader.
using namespace Ogre;

const unsigned int SUM_CAMERA_POS  = 7;
const Real HALF_TILE_SIZE  = 128.0          / TileManager::MAX_TEXTURE_SIZE; // Size of a subtile.
const Real HALF_TILE_SPACE = (128.0 + 16.0) / TileManager::MAX_TEXTURE_SIZE; // Space between 2 subtiles.
const Real FULL_TILE_SPACE = (256.0 + 16.0) / TileManager::MAX_TEXTURE_SIZE; // Space between 2 tiles.

//================================================================================================
// Holds the x-offset for each row of tiles.
// This prevents the drawing of tiles which are outside the field of view.
// Every camera position must have the same count of tiles, because that way the indexbuffer
// of the mesh doesn't need to be changed.
//================================================================================================
/*
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
*/
//================================================================================================
// Sum of tiles in a row (Caluclated from CHUNK_START_OFFSET)
//================================================================================================
int CHUNK_X_LENGTH[TileManager::CHUNK_SIZE_Z];

//================================================================================================
// Constructor.
//================================================================================================
void TileChunk::init(int queryMaskLand, int queryMaskWater, SceneManager *sceneManager)
{
    AxisAlignedBox aab(AxisAlignedBox(-10000, -10000, -10000, 10000, 10000, 10000));
    int cameraStandardPos = 3; // 0° rotation of the camera in CHUNK_START_OFFSET[][] table.
    mCameraRotation = cameraStandardPos;
    int sumVertices = 0;
    /*
        for (int i=0; i < TileManager::CHUNK_SIZE_Z; ++i)
        {
            CHUNK_X_LENGTH[i] = TileManager::CHUNK_SIZE_X - 2*CHUNK_START_OFFSET[cameraStandardPos][i];
            sumVertices+= CHUNK_X_LENGTH[i];
        }
        sumVertices*= SUM_QUAD_VERTICES;
    */
    sumVertices= 4*6*TileManager::CHUNK_SIZE_X*TileManager::CHUNK_SIZE_Z; // 4 Subtiles/tile, 6 verticels/rectangle
    // ////////////////////////////////////////////////////////////////////
    // Build the land-tiles.
    // ////////////////////////////////////////////////////////////////////
    MeshPtr MeshLand = MeshManager::getSingleton().createManual("Mesh_Land", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMeshLand = MeshLand->createSubMesh();
    mSubMeshLand->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshLand->useSharedVertices = false;
    VertexData *vData = new VertexData(); // The 'delete' will be done by Ogre::Submesh.
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
    mSubMeshLand->vertexData = vData;
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, sumVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    mSubMeshLand->indexData->indexBuffer = ibuf;
    mSubMeshLand->indexData->indexStart = 0;
    mSubMeshLand->indexData->indexCount = sumVertices;
    unsigned short *pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
    // There is no chance to optimize the vertex count, because every vertex needs its own filter.
    for (unsigned short p=0; p < sumVertices;) *pIdx++ = p++;
    ibuf->unlock();
    MeshLand->_setBounds(aab);
    //MeshLand->_setBoundingSphereRadius(Real radius);
    MeshLand->load();
    Entity *EntityLand = sceneManager->createEntity("Entity_Land", "Mesh_Land");
    EntityLand->setMaterialName(TileManager::MATERIAL_PREFIX + TileManager::LAND_PREFIX);
    EntityLand->setQueryFlags(queryMaskLand);
    EntityLand->setRenderQueueGroup(RENDER_QUEUE_1); // See OgreRenderQueue.h
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::LAND_PREFIX);
    // ////////////////////////////////////////////////////////////////////
    // Build the water-tiles.
    // ////////////////////////////////////////////////////////////////////
    MeshPtr MeshWater = MeshManager::getSingleton().createManual("Mesh_Water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMeshWater = MeshWater->createSubMesh();
    mSubMeshWater->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshWater->useSharedVertices = false;
    vData = new VertexData(); // The 'delete' will be done by Ogre::Submesh.
    vData->vertexCount = 0;
    vData->vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    offset = VertexElement::getTypeSize(VET_FLOAT3);
    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(offset, sumVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vData->vertexBufferBinding->setBinding(0, vbuf);
    mSubMeshWater->vertexData = vData;
    sumVertices = TileManager::CHUNK_SIZE_X*TileManager::CHUNK_SIZE_Z;
    ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, sumVertices*6, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    mSubMeshWater->indexData->indexBuffer = ibuf;
    mSubMeshWater->indexData->indexStart = 0;
    mSubMeshWater->indexData->indexCount = 0;
    pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
    // This could be optimized by using triangle lists.
    for (unsigned short p=0; sumVertices; --sumVertices)
    {
        *pIdx++ = p+0;
        *pIdx++ = p+1;
        *pIdx++ = p+2;
        *pIdx++ = p+2;
        *pIdx++ = p+3;
        *pIdx++ = p+0;
        p+= 4;
    }
    ibuf->unlock();
    MeshWater->_setBounds(aab);
    //MeshWater->_setBoundingSphereRadius(Real radius);
    MeshWater->load();
    Entity *EntityWater = sceneManager->createEntity("Entity_Water", "Mesh_Water");
    EntityWater->setMaterialName(TileManager::MATERIAL_PREFIX + TileManager::WATER_PREFIX);
    EntityWater->setQueryFlags(queryMaskWater);
    EntityWater->setRenderQueueGroup(RENDER_QUEUE_8); // See OgreRenderQueue.h
    // ////////////////////////////////////////////////////////////////////
    // Attach the tiles to a scenenode.
    // ////////////////////////////////////////////////////////////////////
    SceneNode *node= sceneManager->getRootSceneNode()->createChildSceneNode();
    node->setPosition(0,0,0);
    node->attachObject(EntityLand);
    node->attachObject(EntityWater);
    setWaveParameter(0.5, TileManager::HEIGHT_STRETCH, 1.0);
    setDaylight(1.0f);
    setGrid(false);
}

//================================================================================================
// Set the shader parameter for the grid.
//================================================================================================
void TileChunk::setGrid(bool visible)
{
    mGrid = visible;
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::LAND_PREFIX);
    GpuProgramParametersSharedPtr para = tmpMaterial->getBestTechnique()->getPass(0)->getVertexProgramParameters();
    if (para->_findNamedConstantDefinition("para"))
        para->setNamedConstant("para", Vector4(mDaylight,mGrid?1.0f:0.0f,1,1));
}

//================================================================================================
// Set the shader parameter for the level of darkness.
//================================================================================================
void TileChunk::setDaylight(Real brightness)
{
    mDaylight = brightness;
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::LAND_PREFIX);
    GpuProgramParametersSharedPtr para = tmpMaterial->getBestTechnique()->getPass(0)->getVertexProgramParameters();
    if (para->_findNamedConstantDefinition("para"))
        para->setNamedConstant("para", Vector4(mDaylight,mGrid?1.0f:0.0f,1,1));
    tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::WATER_PREFIX);
    para = tmpMaterial->getBestTechnique()->getPass(0)->getVertexProgramParameters();
    if (para->_findNamedConstantDefinition("para"))
        para->setNamedConstant("para", Vector4(mDaylight, mWaveParam.x, mWaveParam.y, mWaveParam.z));
}

//================================================================================================
// Set the parameter for the waves on the water.
//================================================================================================
void TileChunk::setWaveParameter(Real alpha, Real amplitude, Real speed)
{
    mWaveParam.x = alpha;
    mWaveParam.y = amplitude;
    mWaveParam.z = speed;
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::WATER_PREFIX);
    GpuProgramParametersSharedPtr para = tmpMaterial->getBestTechnique()->getPass(0)->getVertexProgramParameters();
    if (para->_findNamedConstantDefinition("para"))
        para->setNamedConstant("para", Vector4(mDaylight, mWaveParam.x, mWaveParam.y, mWaveParam.z));
}

//================================================================================================
// Set a new material.
//================================================================================================
void TileChunk::setMaterial(bool land, int groupNr, int texSize)
{
    String strTypPrefix = (land)?TileManager::LAND_PREFIX:TileManager::WATER_PREFIX;
    String strMatFile = strTypPrefix;
    strMatFile+= "_"+ StringConverter::toString(groupNr, 2, '0');
    strMatFile+= "_"+ StringConverter::toString(texSize, 4, '0') + ".png";
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + strTypPrefix);
    for (int i=0; i < tmpMaterial->getBestTechnique()->getPass(0)->getNumTextureUnitStates(); ++i)
        tmpMaterial->getBestTechnique()->getPass(0)->getTextureUnitState(i)->setTextureName(strMatFile);
}

//================================================================================================
//
//================================================================================================
void TileChunk::setCameraRotation(Real cameraAngle)
{
    unsigned int rotation = (unsigned int)((cameraAngle + 45.0f) / 15.0f);
    if (mCameraRotation != rotation && rotation < SUM_CAMERA_POS)
    {
        mCameraRotation = rotation;
        update();
    }
}

//================================================================================================
// Set all data for a vertex
//================================================================================================
void TileChunk::setVertex(Vector3 &pos, int maskNr, Real offsetU, Real offsetV, Vector3 &normal)
{
    static const int TEXTURE_UNIT_SORT[]= {0,2,4, 0,4,2, 2,0,4, 2,4,0, 4,0,2, 4,2,0, 4,4,4};
    int sorting = maskNr * 3;
    // if (maskNr == 6) maskNr = 0;
    *mPosVBuf++ = pos.x;
    *mPosVBuf++ = pos.y;
    *mPosVBuf++ = pos.z;
    *mPosVBuf++ = normal.x;
    *mPosVBuf++ = normal.y;
    *mPosVBuf++ = normal.z;
    // Sort the texture units (the higher the gfxNr, the higher the used texture unit)
    // Pos in Atlastexture for Texture #0
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[  sorting]+0] * FULL_TILE_SPACE + offsetU;
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[  sorting]+1] * FULL_TILE_SPACE + offsetV;
    // Pos in Atlastexture for Texture #1
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[++sorting]+0] * FULL_TILE_SPACE + offsetU;
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[  sorting]+1] * FULL_TILE_SPACE + offsetV;
    // Pos in Atlastexture for Texture #2
    *mPosVBuf++ = mTexPosInAtlas[TEXTURE_UNIT_SORT[++sorting]+0] * FULL_TILE_SPACE + offsetU;
    Real mask_Y = mTexPosInAtlas[TEXTURE_UNIT_SORT[  sorting]+1];
    *mPosVBuf++ = mask_Y * FULL_TILE_SPACE + offsetV;
    // Atlastexture position for Mask.
    // Every tile row in the atlas texture has its own mask-set. The highest gfx selects the mask set.
    *mPosVBuf++ = (maskNr%3) * HALF_TILE_SPACE + offsetU/2 + 6*FULL_TILE_SPACE;
    *mPosVBuf++ = (maskNr/3) * HALF_TILE_SPACE + offsetV/2 + 2*mask_Y*HALF_TILE_SPACE;
}

//================================================================================================
// Set all vertex data.
//================================================================================================
void TileChunk::setTriangle(int x, int z, Vector3 v1, Vector3 v2, Vector3 v3,int maskNr)
{
    Real offsetU1 = v1.x*HALF_TILE_SIZE;
    Real offsetV1 = v1.z*HALF_TILE_SIZE;
    Real offsetU2 = v2.x*HALF_TILE_SIZE;
    Real offsetV2 = v2.z*HALF_TILE_SIZE;
    Real offsetU3 = v3.x*HALF_TILE_SIZE;
    Real offsetV3 = v3.z*HALF_TILE_SIZE;
#ifdef FAKE_SHADOWS
    v1.y = TileManager::getSingleton().getMapHeight((int)(x+v1.x), (int)(z+v1.z));
    v2.y = TileManager::getSingleton().getMapHeight((int)(x+v2.x), (int)(z+v2.z));
    v3.y = TileManager::getSingleton().getMapHeight((int)(x+v3.x), (int)(z+v3.z));
    Vector3 normal;
    if ((v1.y <= v2.y) && (v1.y <= v3.y))
    {
        normal.x = v1.y;
        normal.y = TileManager::getSingleton().getMapShadow((int)(x+v1.x), (int)(z+v1.z));
        if (v2.y >= v3.y)
            normal.z = (v2.y == normal.x)?0:(TileManager::getSingleton().getMapShadow((int)(x+v2.x), (int)(z+v2.z)) - normal.y)/ (v2.y-normal.x);
        else
            normal.z = (v3.y == normal.x)?0:(TileManager::getSingleton().getMapShadow((int)(x+v3.x), (int)(z+v3.z)) - normal.y)/ (v3.y-normal.x);
    }
    else if ((v2.y <= v1.y) && (v2.y <= v3.y))
    {
        normal.x = v2.y;
        normal.y = TileManager::getSingleton().getMapShadow((int)(x+v2.x), (int)(z+v2.z));
        if (v1.y >= v3.y)
            normal.z = (v1.y == normal.x)?0:(TileManager::getSingleton().getMapShadow((int)(x+v1.x), (int)(z+v1.z)) - normal.y)/ (v1.y-normal.x);
        else
            normal.z = (v3.y == normal.x)?0:(TileManager::getSingleton().getMapShadow((int)(x+v3.x), (int)(z+v3.z)) - normal.y)/ (v3.y-normal.x);
    }
    else if ((v3.y <= v1.y) && (v3.y <= v2.y))
    {
        normal.x = v3.y;
        normal.y = TileManager::getSingleton().getMapShadow((int)(x+v3.x), (int)(z+v3.z));
        if (v1.y >= v2.y)
            normal.z = (v1.y == normal.x)?0:(TileManager::getSingleton().getMapShadow((int)(x+v1.x), (int)(z+v1.z)) - normal.y)/ (v1.y-normal.x);
        else
            normal.z = (v2.y == normal.x)?0:(TileManager::getSingleton().getMapShadow((int)(x+v3.x), (int)(z+v2.z)) - normal.y)/ (v2.y-normal.x);
    }
    v3.x = (x+v3.x) * TileManager::TILE_RENDER_SIZE;
    v3.z = (z+v3.z) * TileManager::TILE_RENDER_SIZE;
    v1.x = (x+v1.x) * TileManager::TILE_RENDER_SIZE;
    v1.z = (z+v1.z) * TileManager::TILE_RENDER_SIZE;
    v2.x = (x+v2.x) * TileManager::TILE_RENDER_SIZE;
    v2.z = (z+v2.z) * TileManager::TILE_RENDER_SIZE;
#else
    v1.y = TileManager::getSingleton().getMapHeight((int)(x+v1.x), (int)(z+v1.z));
    v1.x = (x+v1.x) * TileManager::TILE_RENDER_SIZE;
    v1.z = (z+v1.z) * TileManager::TILE_RENDER_SIZE;
    v2.y = TileManager::getSingleton().getMapHeight((int)(x+v2.x), (int)(z+v2.z));
    v2.x = (x+v2.x) * TileManager::TILE_RENDER_SIZE;
    v2.z = (z+v2.z) * TileManager::TILE_RENDER_SIZE;
    v3.y = TileManager::getSingleton().getMapHeight((int)(x+v3.x), (int)(z+v3.z));
    v3.x = (x+v3.x) * TileManager::TILE_RENDER_SIZE;
    v3.z = (z+v3.z) * TileManager::TILE_RENDER_SIZE;
    Vector3 normal = v2 - v1;
    normal = normal.crossProduct(v3 - v1);
    normal.normalise();
#endif
    setVertex(v1, maskNr, offsetU1, offsetV1, normal);
    setVertex(v2, maskNr, offsetU2, offsetV2, normal);
    setVertex(v3, maskNr, offsetU3, offsetV3, normal);
}

//================================================================================================
// Choose the blending mask for the texture units.
//================================================================================================
int TileChunk::getMask(int gfxVertex0, int gfxVertex1, int gfxVertex2)
{
    // Set the texture units with the default gfxNr.
    // After we have choosen the filter, they are sorted.
    mTexPosInAtlas[0] = (gfxVertex0%6);
    mTexPosInAtlas[1] = (gfxVertex0/6);
    mTexPosInAtlas[2] = (gfxVertex1%6);
    mTexPosInAtlas[3] = (gfxVertex1/6);
    mTexPosInAtlas[4] = (gfxVertex2%6);
    mTexPosInAtlas[5] = (gfxVertex2/6);
    // Calculate the needed filter.
    if (gfxVertex0 <= gfxVertex1)
    {
        if (gfxVertex0 <= gfxVertex2)
            return (gfxVertex1 <= gfxVertex2)?0:1;
        return 4;
    }
    if (gfxVertex0 <= gfxVertex2) return 2;
    if (gfxVertex1 <= gfxVertex2) return 3;
    return 5;
}

//================================================================================================
// Every subtile holds the map-data for the top/left vertex.
//================================================================================================
void TileChunk::updateLand()
{
    HardwareVertexBufferSharedPtr vbuf = mSubMeshLand->vertexData->vertexBufferBinding->getBuffer(0);
    mPosVBuf = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    int maskNr;
    int gfxNrVert0, gfxNrVert1, gfxNrIndoor;
    for (int z = 0; z < TileManager::CHUNK_SIZE_Z*2; z+=2)
    {
        for (int x = 0; x < TileManager::CHUNK_SIZE_X*2; x+=2)
        {
            // Top/Left Subtile
            // Position   Mask
            // 2+-+3--+   1+-+2+-+
            //  |\|   |    |\|   |
            // 1+-+0  |   2+-+0  |
            //  |     |    |     |
            //  +-----+    +-----+
            gfxNrVert0 = TileManager::getSingleton().getMapLayer0(x+1, z+1);
            gfxNrIndoor = TileManager::getSingleton().getMapLayer1(x  , z+2);
            if (gfxNrIndoor)
            {   // Indoor tile -> No blending with the neighbour tiles.
                mTexPosInAtlas[0] = (gfxNrIndoor%6);
                mTexPosInAtlas[1] = (gfxNrIndoor/6);
                mTexPosInAtlas[2] = mTexPosInAtlas[0];
                mTexPosInAtlas[3] = mTexPosInAtlas[1];
                mTexPosInAtlas[4] = mTexPosInAtlas[0];
                mTexPosInAtlas[5] = mTexPosInAtlas[1];
                setTriangle(x, z, Vector3(1,0,1), Vector3(0,0,1), Vector3(0,0,2), 6); // Vertex 0,1,2
                setTriangle(x, z, Vector3(0,0,2), Vector3(1,0,2), Vector3(1,0,1), 6); // Vertex 2,3,0
            }
            else
            {
                gfxNrVert1 = TileManager::getSingleton().getMapLayer0(x  , z+2);
                maskNr = getMask(gfxNrVert0, gfxNrVert1, TileManager::getSingleton().getMapLayer0(x  , z+1));
                setTriangle(x, z, Vector3(1,0,1), Vector3(0,0,1), Vector3(0,0,2), maskNr); // Vertex 0,1,2
                maskNr = getMask(gfxNrVert0, gfxNrVert1, TileManager::getSingleton().getMapLayer0(x+1, z+2));
                setTriangle(x, z, Vector3(0,0,2), Vector3(1,0,2), Vector3(1,0,1), maskNr); // Vertex 2,3,0
            }
            // Top/Right Subtile
            // Position    Mask
            //  +--1+-+2   +--2+-+1
            //  |   |/|    |   |/|
            //  |  0+-+3   |  0+-+2
            //  |     |    |     |
            //  +-----+    +-----+
            gfxNrIndoor = TileManager::getSingleton().getMapLayer1(x+1, z+2);
            if (gfxNrIndoor)
            {   // Indoor tile -> No blending with the neighbour tiles.
                mTexPosInAtlas[0] = (gfxNrIndoor%6);
                mTexPosInAtlas[1] = (gfxNrIndoor/6);
                mTexPosInAtlas[2] = mTexPosInAtlas[0];
                mTexPosInAtlas[3] = mTexPosInAtlas[1];
                mTexPosInAtlas[4] = mTexPosInAtlas[0];
                mTexPosInAtlas[5] = mTexPosInAtlas[1];
                setTriangle(x, z, Vector3(1,0,1), Vector3(1,0,2), Vector3(2,0,2), 6); // Vertex 0,1,2
                setTriangle(x, z, Vector3(2,0,2), Vector3(2,0,1), Vector3(1,0,1), 6); // Vertex 2,3,0
            }
            else
            {
                gfxNrVert1 = TileManager::getSingleton().getMapLayer0(x+2, z+2);
                maskNr = getMask(gfxNrVert0, gfxNrVert1, TileManager::getSingleton().getMapLayer0(x+1, z+2));
                setTriangle(x, z, Vector3(1,0,1), Vector3(1,0,2), Vector3(2,0,2), maskNr); // Vertex 0,1,2
                maskNr = getMask(gfxNrVert0, gfxNrVert1, TileManager::getSingleton().getMapLayer0(x+2, z+1));
                setTriangle(x, z, Vector3(2,0,2), Vector3(2,0,1), Vector3(1,0,1), maskNr); // Vertex 2,3,0
            }
            // Bottom/Left Subtile
            // Position    Mask
            //  +-----+    +-----+
            //  |     |    |     |
            // 1+-+2  |   2+-+0  |
            //  |/|   |    |/|   |
            // 0+-+3--+   1+-+2--+
            gfxNrIndoor = TileManager::getSingleton().getMapLayer1(x, z+1);
            if (gfxNrIndoor)
            {   // Indoor tile -> No blending with the neighbour tiles.
                mTexPosInAtlas[0] = (gfxNrIndoor%6);
                mTexPosInAtlas[1] = (gfxNrIndoor/6);
                mTexPosInAtlas[2] = mTexPosInAtlas[0];
                mTexPosInAtlas[3] = mTexPosInAtlas[1];
                mTexPosInAtlas[4] = mTexPosInAtlas[0];
                mTexPosInAtlas[5] = mTexPosInAtlas[1];
                setTriangle(x, z, Vector3(0,0,0), Vector3(0,0,1), Vector3(1,0,1), 6); // Vertex 0,1,2
                setTriangle(x, z, Vector3(1,0,1), Vector3(1,0,0), Vector3(0,0,0), 6); // Vertex 2,3,0
            }
            else
            {
                gfxNrVert1 = TileManager::getSingleton().getMapLayer0(x  , z  );
                maskNr = getMask(gfxNrVert0, gfxNrVert1, TileManager::getSingleton().getMapLayer0(x  , z+1));
                setTriangle(x, z, Vector3(0,0,0), Vector3(0,0,1), Vector3(1,0,1), maskNr); // Vertex 0,1,2
                maskNr = getMask(gfxNrVert0, gfxNrVert1, TileManager::getSingleton().getMapLayer0(x+1, z  ));
                setTriangle(x, z, Vector3(1,0,1), Vector3(1,0,0), Vector3(0,0,0), maskNr); // Vertex 2,3,0
            }
            // Bottom/Right Subtile
            // Position    Mask
            //  +-----+    +-----+
            //  |     |    |     |
            //  |  2+-+3   |  0+-+2
            //  |   |\|    |   |\|
            //  +--1+-+0   +--2+-+1
            gfxNrIndoor = TileManager::getSingleton().getMapLayer1(x+1, z+1);
            if (gfxNrIndoor)
            {   // Indoor tile -> No blending with the neighbour tiles.
                mTexPosInAtlas[0] = (gfxNrIndoor%6);
                mTexPosInAtlas[1] = (gfxNrIndoor/6);
                mTexPosInAtlas[2] = mTexPosInAtlas[0];
                mTexPosInAtlas[3] = mTexPosInAtlas[1];
                mTexPosInAtlas[4] = mTexPosInAtlas[0];
                mTexPosInAtlas[5] = mTexPosInAtlas[1];
                setTriangle(x, z, Vector3(2,0,0), Vector3(1,0,0), Vector3(1,0,1), 6); // Vertex 0,1,2
                setTriangle(x, z, Vector3(1,0,1), Vector3(2,0,1), Vector3(2,0,0), 6); // Vertex 2,3,0
            }
            else
            {
                gfxNrVert1 = TileManager::getSingleton().getMapLayer0(x+2, z  );
                maskNr = getMask(gfxNrVert0, gfxNrVert1, TileManager::getSingleton().getMapLayer0(x+1, z  ));
                setTriangle(x, z, Vector3(2,0,0), Vector3(1,0,0), Vector3(1,0,1), maskNr); // Vertex 0,1,2
                maskNr = getMask(gfxNrVert0, gfxNrVert1, TileManager::getSingleton().getMapLayer0(x+2, z+1));
                setTriangle(x, z, Vector3(1,0,1), Vector3(2,0,1), Vector3(2,0,0), maskNr); // Vertex 2,3,0
            }
        }
    }
    vbuf->unlock();
}

//================================================================================================
// Create hardware buffers for water.
//================================================================================================
void TileChunk::updateWater()
{
    // ////////////////////////////////////////////////////////////////////
    // Count the Vertices in this chunk.
    // ////////////////////////////////////////////////////////////////////
    unsigned int numVertices = 0;
    for (int x = 0; x < TileManager::CHUNK_SIZE_X*2; x+=2)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE_Z*2; z+=2)
        {
            if (TileManager::getSingleton().getMapWater(x  , z  )) { ++numVertices; continue; }
            if (TileManager::getSingleton().getMapWater(x+1, z  )) { ++numVertices; continue; }
            if (TileManager::getSingleton().getMapWater(x+2, z  )) { ++numVertices; continue; }
            if (TileManager::getSingleton().getMapWater(x  , z+1)) { ++numVertices; continue; }
            if (TileManager::getSingleton().getMapWater(x+1, z+1)) { ++numVertices; continue; }
            if (TileManager::getSingleton().getMapWater(x+2, z+1)) { ++numVertices; continue; }
            if (TileManager::getSingleton().getMapWater(x  , z+2)) { ++numVertices; continue; }
            if (TileManager::getSingleton().getMapWater(x+1, z+2)) { ++numVertices; continue; }
            if (TileManager::getSingleton().getMapWater(x+2, z+2)) { ++numVertices; continue; }
        }
    }
    if (!numVertices)
    {
        mSubMeshWater->indexData->indexCount = 0;
        mSubMeshWater->vertexData->vertexCount =  0;
        return;
    }
    mSubMeshWater->indexData->indexCount = numVertices*6;
    // ////////////////////////////////////////////////////////////////////
    // Update VertexData.
    // ////////////////////////////////////////////////////////////////////
    VertexData *vdata = mSubMeshWater->vertexData;
    vdata->vertexCount = numVertices*4;
    VertexDeclaration *vdec = vdata->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);
    offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset+= VertexElement::getTypeSize(VET_FLOAT2);
    HardwareVertexBufferSharedPtr vbuf;
    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(offset, vdata->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vdata->vertexBufferBinding->setBinding(0, vbuf);
    Real height;
    Real *pReal = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    for (int z = 0; z < TileManager::CHUNK_SIZE_Z*2; z+=2) {
        for (int x = 0; x < TileManager::CHUNK_SIZE_X*2; x+=2) {
            if (!(height = TileManager::getSingleton().getMapWater(x  , z  )))
                if (!(height = TileManager::getSingleton().getMapWater(x+1, z  )))
                    if (!(height = TileManager::getSingleton().getMapWater(x+2, z  )))
                        if (!(height = TileManager::getSingleton().getMapWater(x  , z+1)))
                            if (!(height = TileManager::getSingleton().getMapWater(x+1, z+1)))
                                if (!(height = TileManager::getSingleton().getMapWater(x+2, z+1)))
                                    if (!(height = TileManager::getSingleton().getMapWater(x  , z+2)))
                                        if (!(height = TileManager::getSingleton().getMapWater(x+1, z+2)))
                                            if (!(height = TileManager::getSingleton().getMapWater(x+2, z+2)))
                                                continue;
            {
                *pReal++ = TileManager::TILE_RENDER_SIZE * x + 2*TileManager::TILE_RENDER_SIZE;
                *pReal++ = height;
                *pReal++ = TileManager::TILE_RENDER_SIZE * z;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = TileManager::TILE_RENDER_SIZE * x;
                *pReal++ = height;
                *pReal++ = TileManager::TILE_RENDER_SIZE * z;
                *pReal++ = 0;
                *pReal++ = 0;
                *pReal++ = TileManager::TILE_RENDER_SIZE * x;
                *pReal++ = height;
                *pReal++ = TileManager::TILE_RENDER_SIZE * z+ 2*TileManager::TILE_RENDER_SIZE;
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = TileManager::TILE_RENDER_SIZE * x+ 2*TileManager::TILE_RENDER_SIZE;
                *pReal++ = height;
                *pReal++ = TileManager::TILE_RENDER_SIZE * z+ 2*TileManager::TILE_RENDER_SIZE;
                *pReal++ = 1;
                *pReal++ = 1;
            }
        }
    }
    vbuf->unlock();
}

