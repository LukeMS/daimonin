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
#include "logger.h"
#include "profiler.h"
#include "tile/tile_manager.h"

using namespace Ogre;

static const unsigned int SUM_CAMERA_POS  = 7;
static const int SUM_NEAR_GRASS_ROWS = 5;
static const Real HALF_TILE_SIZE  = 128.0          / (Real)TileManager::MAX_TEXTURE_SIZE; // Size of a subtile.
static const Real HALF_TILE_SPACE = (128.0 + 16.0) / (Real)TileManager::MAX_TEXTURE_SIZE; // Space between 2 subtiles.
static const Real FULL_TILE_SPACE = (256.0 + 16.0) / (Real)TileManager::MAX_TEXTURE_SIZE; // Space between 2 tiles.
static const Real RENDER_SIZE = (Real)TileManager::TILE_RENDER_SIZE;

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
// Create index buffer for the max tile count. That way the buffer never needs to be updated.
//================================================================================================
void TileChunk::init(int queryMaskLand, int queryMaskWater, SceneManager *sceneManager)
{
    PROFILE()
    mUndergrowth = true;
    AxisAlignedBox aab(AxisAlignedBox(-10000, -10000, -10000, 10000, 10000, 10000));
    mCameraRotation = 3; // 0° rotation of the camera in CHUNK_START_OFFSET[][] table.
    size_t sumIndices, sumVertices = 0;
    /*
        for (int i=0; i < TileManager::CHUNK_SIZE_Z; ++i)
        {
            CHUNK_X_LENGTH[i] = TileManager::CHUNK_SIZE_X - 2*CHUNK_START_OFFSET[mCameraRotation][i];
            sumVertices+= CHUNK_X_LENGTH[i];
        }
        sumVertices*= SUM_QUAD_VERTICES;
    */
    // ////////////////////////////////////////////////////////////////////
    // Build the land-tiles.
    // There is no chance to optimize the vertex count, because every vertex needs its own mask.
    // ////////////////////////////////////////////////////////////////////
    MeshPtr MeshLand = MeshManager::getSingleton().createManual("Mesh_Land", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMeshLand = MeshLand->createSubMesh();
    mSubMeshLand->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshLand->useSharedVertices = false;
    VertexData *vData = new VertexData(); // The 'delete' will be done by Ogre::Submesh.
    vData->vertexCount = 4*6*TileManager::CHUNK_SIZE_X*TileManager::CHUNK_SIZE_Z; // 4 Subtiles/tile, 6 vertices/subtile;
    VertexDeclaration *vdec = vData->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);               offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT4, VES_DIFFUSE);                offset+= VertexElement::getTypeSize(VET_FLOAT4);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 2); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 3); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(offset, vData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vData->vertexBufferBinding->setBinding(0, vbuf);
    mSubMeshLand->vertexData = vData;
    mSubMeshLand->indexData->indexStart = 0;
    sumIndices = vData->vertexCount;
    mSubMeshLand->indexData->indexCount = sumIndices;
    HardwareIndexBufferSharedPtr ibuf;
    if (sumIndices > 65526)
    {
        Logger::log().warning() << "You want to create a HardwareBuffer with " << sumIndices << " entries. Switching to 32bit index buffer. This can crash older gfx-cards!";
        ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_32BIT, sumIndices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        unsigned int *pIdx = static_cast<unsigned int*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
        for (unsigned int p=0; p < sumIndices;) *pIdx++ = p++;
    }
    else
    {
        ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, sumIndices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        unsigned short *pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
        for (unsigned short p=0; p < sumIndices;) *pIdx++ = p++;
    }
    ibuf->unlock();
    mSubMeshLand->indexData->indexBuffer = ibuf;
    MeshLand->_setBounds(aab);
    //MeshLand->_setBoundingSphereRadius(Real radius);
    MeshLand->load();
    Entity *EntityLand = sceneManager->createEntity("Entity_Land", "Mesh_Land");
    EntityLand->setMaterialName(TileManager::MATERIAL_PREFIX + TileManager::LAND_PREFIX);
    EntityLand->setQueryFlags(queryMaskLand);
    EntityLand->setRenderQueueGroup(RENDER_QUEUE_1); // See OgreRenderQueue.h
    // ////////////////////////////////////////////////////////////////////
    // Build the water-tiles.
    // This could be optimized by using triangle lists.
    // ////////////////////////////////////////////////////////////////////
    MeshPtr MeshWater = MeshManager::getSingleton().createManual("Mesh_Water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMeshWater = MeshWater->createSubMesh();
    mSubMeshWater->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshWater->useSharedVertices = false;
    vData = new VertexData(); // The 'delete' will be done by Ogre::Submesh.
    sumVertices = 4*TileManager::CHUNK_SIZE_X*TileManager::CHUNK_SIZE_Z; // 4 Subtile/tile
    vData->vertexCount = sumVertices;
    vdec = vData->vertexDeclaration;
    offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);               offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(offset, sumVertices*4, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vData->vertexBufferBinding->setBinding(0, vbuf);
    mSubMeshWater->vertexData = vData;
    if (sumVertices > 65526)
    {
        Logger::log().warning() << "You want to create a HardwareBuffer with " << sumVertices << " entries. Switching to 32bit index buffer. This can crash older gfx-cards!";
        ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_32BIT, sumVertices*6, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        unsigned int *pIdx = static_cast<unsigned int*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
        for (unsigned int p=0; sumVertices; --sumVertices)
        {
            *pIdx++ = p+0;
            *pIdx++ = p+1;
            *pIdx++ = p+2;
            *pIdx++ = p+2;
            *pIdx++ = p+3;
            *pIdx++ = p+0;
            p+= 4;
        }
    }
    else
    {
        ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, sumVertices*6, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        unsigned short *pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
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
    }
    ibuf->unlock();
    mSubMeshWater->indexData->indexBuffer = ibuf;
    mSubMeshWater->indexData->indexStart  = 0;
    mSubMeshWater->indexData->indexCount  = 0;
    mSubMeshWater->vertexData->vertexCount = 0;
    MeshWater->_setBounds(aab);
    //MeshWater->_setBoundingSphereRadius(Real radius);
    MeshWater->load();
    Entity *EntityWater = sceneManager->createEntity("Entity_Water", "Mesh_Water");
    EntityWater->setMaterialName(TileManager::MATERIAL_PREFIX + TileManager::WATER_PREFIX);
    EntityWater->setQueryFlags(queryMaskWater);
    EntityWater->setRenderQueueGroup(RENDER_QUEUE_8); // See OgreRenderQueue.h
    // ////////////////////////////////////////////////////////////////////
    // Build the Undergrowth (Far).
    // ////////////////////////////////////////////////////////////////////
    MeshPtr MeshGrassFar = MeshManager::getSingleton().createManual("Mesh_GrassFar", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMeshGrassFar = MeshGrassFar->createSubMesh();
    mSubMeshGrassFar->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshGrassFar->useSharedVertices = false;
    vData = new VertexData(); // The 'delete' will be done by Ogre::Submesh.
    sumVertices = 4*TileManager::CHUNK_SIZE_X*TileManager::CHUNK_SIZE_Z; // 4 Subtile/tile
    vData->vertexCount = sumVertices;
    vdec = vData->vertexDeclaration;
    offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);               offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(offset, sumVertices*4, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vData->vertexBufferBinding->setBinding(0, vbuf);
    mSubMeshGrassFar->vertexData = vData;
    if (sumVertices > 65526)
    {
        Logger::log().warning() << "You want to create a HardwareBuffer with " << sumVertices << " entries. Switching to 32bit index buffer. This can crash older gfx-cards!";
        ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_32BIT, sumVertices*6, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        unsigned int *pIdx = static_cast<unsigned int*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
        for (unsigned int p=0; sumVertices; --sumVertices)
        {
            *pIdx++ = p+0;
            *pIdx++ = p+1;
            *pIdx++ = p+2;
            *pIdx++ = p+2;
            *pIdx++ = p+3;
            *pIdx++ = p+0;
            p+= 4;
        }
    }
    else
    {
        ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, sumVertices*6, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        unsigned short *pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
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
    }
    ibuf->unlock();
    mSubMeshGrassFar->indexData->indexBuffer = ibuf;
    mSubMeshGrassFar->indexData->indexStart  = 0;
    mSubMeshGrassFar->indexData->indexCount  = 0;
    mSubMeshGrassFar->vertexData->vertexCount = 0;
    MeshGrassFar->_setBounds(aab);
    //MeshGrassFar->_setBoundingSphereRadius(Real radius);
    MeshGrassFar->load();
    mEntityGrassFar = sceneManager->createEntity("Entity_GrassFar", "Mesh_GrassFar");
    //mEntityGrassFar->setMaterialName(TileManager::MATERIAL_PREFIX + TileManager::GrassFar_PREFIX);
    mEntityGrassFar->setMaterialName("Terrain/Sprite");
    mEntityGrassFar->setQueryFlags(queryMaskWater);
    mEntityGrassFar->setRenderQueueGroup(RENDER_QUEUE_8); // See OgreRenderQueue.h
    // ////////////////////////////////////////////////////////////////////
    // Build the Undergrowth (Near).
    // ////////////////////////////////////////////////////////////////////
    MeshPtr MeshGrassNear = MeshManager::getSingleton().createManual("Mesh_GrassNear", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMeshGrassNear = MeshGrassNear->createSubMesh();
    mSubMeshGrassNear->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshGrassNear->useSharedVertices = false;
    vData = new VertexData(); // The 'delete' will be done by Ogre::Submesh.
    sumVertices = 4*TileManager::CHUNK_SIZE_X*TileManager::CHUNK_SIZE_Z; // 4 Subtile/tile
    vData->vertexCount = sumVertices;
    vdec = vData->vertexDeclaration;
    offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);
    offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset+= VertexElement::getTypeSize(VET_FLOAT2);
    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(offset, sumVertices*4, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vData->vertexBufferBinding->setBinding(0, vbuf);
    mSubMeshGrassNear->vertexData = vData;
    if (sumVertices > 65526)
    {
        Logger::log().warning() << "You want to create a HardwareBuffer with " << sumVertices << " entries. Switching to 32bit index buffer. This can crash older gfx-cards!";
        ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_32BIT, sumVertices*6, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        unsigned int *pIdx = static_cast<unsigned int*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
        for (unsigned int p=0; sumVertices; --sumVertices)
        {
            *pIdx++ = p+0;
            *pIdx++ = p+1;
            *pIdx++ = p+2;
            *pIdx++ = p+2;
            *pIdx++ = p+3;
            *pIdx++ = p+0;
            p+= 4;
        }
    }
    else
    {
        ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, sumVertices*6, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        unsigned short *pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
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
    }
    ibuf->unlock();
    mSubMeshGrassNear->indexData->indexBuffer = ibuf;
    mSubMeshGrassNear->indexData->indexStart  = 0;
    mSubMeshGrassNear->indexData->indexCount  = 0;
    mSubMeshGrassNear->vertexData->vertexCount = 0;
    MeshGrassNear->_setBounds(aab);
    //MeshGrassNear->_setBoundingSphereRadius(Real radius);
    MeshGrassNear->load();
    mEntityGrassNear = sceneManager->createEntity("Entity_GrassNear", "Mesh_GrassNear");
    //mEntityGrassNear->setMaterialName(TileManager::MATERIAL_PREFIX + TileManager::GrassNear_PREFIX);
    mEntityGrassNear->setMaterialName("Terrain/GrassNear");
    mEntityGrassNear->setQueryFlags(queryMaskWater);
    mEntityGrassNear->setRenderQueueGroup(RENDER_QUEUE_8); // See OgreRenderQueue.h
    // ////////////////////////////////////////////////////////////////////
    // Attach the tiles to a scenenode.
    // ////////////////////////////////////////////////////////////////////
    SceneNode *node= sceneManager->getRootSceneNode()->createChildSceneNode("snTileChunk");
    node->attachObject(EntityLand);
    node->attachObject(EntityWater);
    node->attachObject(mEntityGrassFar);
    node->attachObject(mEntityGrassNear);
}

//================================================================================================
// Set the shader parameter for the grid.
//================================================================================================
void TileChunk::setGrid(bool visible)
{
    PROFILE()
    mGrid = visible;
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::LAND_PREFIX);
    GpuProgramParametersSharedPtr para = tmpMaterial->getBestTechnique()->getPass(0)->getVertexProgramParameters();
    if (para->_findNamedConstantDefinition("para"))
        para->setNamedConstant("para", Vector4(mDaylight,mGrid?0.0f:1.0f,1,1));
}

//================================================================================================
// Set the shader parameter for the level of darkness.
//================================================================================================
void TileChunk::setLight(Real brightness)
{
    PROFILE()
    mDaylight = brightness;
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::LAND_PREFIX);
    GpuProgramParametersSharedPtr para = tmpMaterial->getBestTechnique()->getPass(0)->getVertexProgramParameters();
    if (para->_findNamedConstantDefinition("para"))
        para->setNamedConstant("para", Vector4(mDaylight,mGrid?0.0f:1.0f,1,1));
    tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::WATER_PREFIX);
    para = tmpMaterial->getBestTechnique()->getPass(0)->getVertexProgramParameters();
    if (para->_findNamedConstantDefinition("para"))
        para->setNamedConstant("para", Vector4(mDaylight, mWaveParam.x, mWaveParam.y, mWaveParam.z));
}

//================================================================================================
// Set the parameter for the waves on the water.
//================================================================================================
void TileChunk::setWave(Real alpha, Real amplitude, Real speed)
{
    PROFILE()
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
void TileChunk::setMaterial(int groupNr, int texSize)
{
    PROFILE()
    String strMatFile = "_"+ StringConverter::toString(groupNr, 2, '0')+"_"+ StringConverter::toString(texSize, 4, '0')+".png";
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::LAND_PREFIX);
    for (int i=0; i < tmpMaterial->getBestTechnique()->getPass(0)->getNumTextureUnitStates(); ++i)
        tmpMaterial->getBestTechnique()->getPass(0)->getTextureUnitState(i)->setTextureName(TileManager::ATLAS_PREFIX + strMatFile);
    tmpMaterial = MaterialManager::getSingleton().getByName(TileManager::MATERIAL_PREFIX + TileManager::WATER_PREFIX);
    for (int i=0; i < tmpMaterial->getBestTechnique()->getPass(0)->getNumTextureUnitStates(); ++i)
        tmpMaterial->getBestTechnique()->getPass(0)->getTextureUnitState(i)->setTextureName(TileManager::ATLAS_PREFIX + strMatFile);
}

//================================================================================================
// Set the camera rotation.
//================================================================================================
void TileChunk::setCameraRotation(Real cameraAngle)
{
    PROFILE()
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
void TileChunk::setVertex(Vector3 &pos, int maskNr, Real offsetU, Real offsetV, Vector4 &params)
{
    PROFILE()
    static const int TEXTURE_UNIT_SORT[]= {0,2,4, 0,4,2, 2,0,4, 2,4,0, 4,0,2, 4,2,0, 4,4,4};
    int sorting = maskNr * 3;
    // if (maskNr == 6) maskNr = 0;
    *mPosVBuf++ = pos.x * TileManager::TILE_RENDER_SIZE;
    *mPosVBuf++ = pos.y;
    *mPosVBuf++ = pos.z * TileManager::TILE_RENDER_SIZE;
    *mPosVBuf++ = params.x; // Faked shadow
    *mPosVBuf++ = params.x; // Reserved
    *mPosVBuf++ = params.x; // Reserved
    *mPosVBuf++ = params.w; // Spotlight
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
    PROFILE()
    Vector4 params;
    params.w = TileManager::getSingleton().getMapSpotLight((int)(x+v2.x), (int)(z+v2.z))?1.0f:0.0f;
    // Vertex 1
    Real offsetU = v1.x*HALF_TILE_SIZE;
    Real offsetV = v1.z*HALF_TILE_SIZE;
    v1.x+= x;
    v1.z+= z;
    v1.y = TileManager::getSingleton().getMapHeight((int)v1.x, (int)v1.z);
    params.x = TileManager::getSingleton().getMapShadow((int)v1.x, (int)v1.z);
    setVertex(v1, maskNr, offsetU, offsetV, params);
    // Vertex 2
    offsetU = v2.x*HALF_TILE_SIZE;
    offsetV = v2.z*HALF_TILE_SIZE;
    v2.x+= x;
    v2.z+= z;
    v2.y = TileManager::getSingleton().getMapHeight((int)v2.x, (int)v2.z);
    params.x = TileManager::getSingleton().getMapShadow((int)v2.x, (int)v2.z);
    setVertex(v2, maskNr, offsetU, offsetV, params);
    // Vertex 3
    offsetU = v3.x*HALF_TILE_SIZE;
    offsetV = v3.z*HALF_TILE_SIZE;
    v3.x+= x;
    v3.z+= z;
    v3.y = TileManager::getSingleton().getMapHeight((int)v3.x, (int)v3.z);
    params.x = TileManager::getSingleton().getMapShadow((int)v3.x, (int)v3.z);
    setVertex(v3, maskNr, offsetU, offsetV, params);
}

//================================================================================================
// Choose the blending mask for the texture units.
//================================================================================================
int TileChunk::getMask(int gfxVertex0, int gfxVertex1, int gfxVertex2)
{
    PROFILE()
    // Set the texture units with the default gfxNr.
    // After we have choosen the mask, they are sorted.
    mTexPosInAtlas[0] = (Real)(gfxVertex0%6);
    mTexPosInAtlas[1] = (Real)(gfxVertex0/6);
    mTexPosInAtlas[2] = (Real)(gfxVertex1%6);
    mTexPosInAtlas[3] = (Real)(gfxVertex1/6);
    mTexPosInAtlas[4] = (Real)(gfxVertex2%6);
    mTexPosInAtlas[5] = (Real)(gfxVertex2/6);
    // Calculate the needed mask.
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
// Update hardware buffers for land.
// (Every subtile holds the map-data for the top/left vertex).
//================================================================================================
void TileChunk::updateLand()
{
    PROFILE()
    HardwareVertexBufferSharedPtr vbuf = mSubMeshLand->vertexData->vertexBufferBinding->getBuffer(0);
    mPosVBuf = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    int maskNr, gfxNrVert0, gfxNrVert1, gfxNrNoBlending;
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
            gfxNrNoBlending = TileManager::getSingleton().getMapLayer1(x  , z+2);
            if (gfxNrNoBlending)
            {
                // Indoor tile -> No blending with the neighbour tiles.
                mTexPosInAtlas[0] = (Real)(gfxNrNoBlending%6);
                mTexPosInAtlas[1] = (Real)(gfxNrNoBlending/6);
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
            gfxNrNoBlending = TileManager::getSingleton().getMapLayer1(x+1, z+2);
            if (gfxNrNoBlending)
            {
                // Indoor tile -> No blending with the neighbour tiles.
                mTexPosInAtlas[0] = (Real)(gfxNrNoBlending%6);
                mTexPosInAtlas[1] = (Real)(gfxNrNoBlending/6);
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
            gfxNrNoBlending = TileManager::getSingleton().getMapLayer1(x, z+1);
            if (gfxNrNoBlending)
            {
                // Indoor tile -> No blending with the neighbour tiles.
                mTexPosInAtlas[0] = (Real)(gfxNrNoBlending%6);
                mTexPosInAtlas[1] = (Real)(gfxNrNoBlending/6);
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
            gfxNrNoBlending = TileManager::getSingleton().getMapLayer1(x+1, z+1);
            if (gfxNrNoBlending)
            {
                // Indoor tile -> No blending with the neighbour tiles.
                mTexPosInAtlas[0] = (Real)(gfxNrNoBlending%6);
                mTexPosInAtlas[1] = (Real)(gfxNrNoBlending/6);
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
// Update hardware buffers for water.
//================================================================================================
void TileChunk::updateWater()
{
    PROFILE()
    HardwareVertexBufferSharedPtr vbuf = mSubMeshWater->vertexData->vertexBufferBinding->getBuffer(0);
    Real height;
    Real *pReal = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    const Real START_Z = TileManager::ATLAS_LAND_ROWS*FULL_TILE_SPACE;
    unsigned int numVertices = 0;
    for (int z = 0; z < TileManager::CHUNK_SIZE_Z*2; ++z)
    {
        Real offsetZ = (z&1)?FULL_TILE_SPACE:0.0f;
        for (int x = 0; x < TileManager::CHUNK_SIZE_X*2; ++x)
        {
            // Only draw water if this tile or a neighbour-tile has water on it.
            if (!(height = TileManager::getSingleton().getMapWater(x  , z  )))
                if (!(height = TileManager::getSingleton().getMapWater(x+1, z  )))
                    if (!(height = TileManager::getSingleton().getMapWater(x  , z+1)))
                        if (!(height = TileManager::getSingleton().getMapWater(x+1, z+1)))
                            continue;
            {
                Real offsetX = (x&1)?offsetZ+HALF_TILE_SIZE:offsetZ;
                *pReal++ = RENDER_SIZE * x + RENDER_SIZE;
                *pReal++ = height;
                *pReal++ = RENDER_SIZE * z;
                *pReal++ = offsetX+HALF_TILE_SIZE;
                *pReal++ = START_Z;
                *pReal++ = RENDER_SIZE * x;
                *pReal++ = height;
                *pReal++ = RENDER_SIZE * z;
                *pReal++ = offsetX;
                *pReal++ = START_Z;
                *pReal++ = RENDER_SIZE * x;
                *pReal++ = height;
                *pReal++ = RENDER_SIZE * z + RENDER_SIZE;
                *pReal++ = offsetX;
                *pReal++ = START_Z+HALF_TILE_SIZE;
                *pReal++ = RENDER_SIZE * x + RENDER_SIZE;
                *pReal++ = height;
                *pReal++ = RENDER_SIZE * z + RENDER_SIZE;
                *pReal++ = offsetX+HALF_TILE_SIZE;
                *pReal++ = START_Z+HALF_TILE_SIZE;
                ++numVertices;
            }
        }
    }
    vbuf->unlock();
    mSubMeshWater->indexData->indexCount = numVertices*2*3; // 2 tris/subtile * 3 vertices/triangle.
    mSubMeshWater->vertexData->vertexCount = numVertices*4; // 4 vertices/subtile.
}

//================================================================================================
// Update hardware buffers for sprites.
//================================================================================================
void TileChunk::updateUndergrowth()
{
    PROFILE()
    if (!mUndergrowth) return;
    HardwareVertexBufferSharedPtr vbuf = mSubMeshGrassFar->vertexData->vertexBufferBinding->getBuffer(0);
    Real height;
    Real *pReal = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    unsigned int numVertices = 0;
    for (int z = 0; z < TileManager::CHUNK_SIZE_Z*2-SUM_NEAR_GRASS_ROWS; ++z)
    {
        for (int x = 0; x < TileManager::CHUNK_SIZE_X*2; ++x)
        {
            height = TileManager::getSingleton().getMapHeight(x, z) + RENDER_SIZE;
            *pReal++ = RENDER_SIZE * x- 1.0f;
            *pReal++ = height;
            *pReal++ = RENDER_SIZE * z;
            *pReal++ = 1.0f;
            *pReal++ = 0.0f;

            *pReal++ = RENDER_SIZE * x;
            *pReal++ = height;
            *pReal++ = RENDER_SIZE * z;
            *pReal++ = 0.0f;
            *pReal++ = 0.0f;

            *pReal++ = RENDER_SIZE * x;
            *pReal++ = height+1;
            *pReal++ = RENDER_SIZE * z;
            *pReal++ = 0.0f;
            *pReal++ = 1.0f;

            *pReal++ = RENDER_SIZE * x- 1.0f;
            *pReal++ = height+1;
            *pReal++ = RENDER_SIZE * z;
            *pReal++ = 1.0f;
            *pReal++ = 1.0f;
            ++numVertices;
        }
    }
    vbuf->unlock();
    mSubMeshGrassFar->indexData->indexCount = numVertices*2*3; // 2 tris/subtile * 3 vertices/triangle.
    mSubMeshGrassFar->vertexData->vertexCount = numVertices*4; // 4 vertices/subtile.

    vbuf = mSubMeshGrassNear->vertexData->vertexBufferBinding->getBuffer(0);
    pReal = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    numVertices = 0;
    const Real GRASS_SIZE = 70.0;
    Real z = 0;
    for (int zz = TileManager::CHUNK_SIZE_Z*2-SUM_NEAR_GRASS_ROWS; zz < TileManager::CHUNK_SIZE_Z*2; ++zz, ++z)
    {
        for (int x = 0; x < TileManager::CHUNK_SIZE_X*2; ++x)
        {
            height = TileManager::getSingleton().getMapHeight(x, zz) + TileManager::TILE_RENDER_SIZE/2.0f;
//#define STYLE_1
#ifdef STYLE_1
            /*    \/    */
            /*    /\    */
            /*  _/__\_  */
            /*  /    \  */
            // Front
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/2;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/2;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/2;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/2;
            *pReal++ = 0.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/2;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/2;
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/2;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/2;
            *pReal++ = 1.0;
            *pReal++ = 1.0;
            ++numVertices;
            // Right
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/2;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/1.5;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/8;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z - GRASS_SIZE/4;
            *pReal++ = 0.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/8;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z - GRASS_SIZE/4;
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/2;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/1.5;
            *pReal++ = 1.0;
            *pReal++ = 1.0;
            ++numVertices;
            // Left
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/8;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z - GRASS_SIZE/4;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/2;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/1.5;
            *pReal++ = 0.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/2;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/1.5;
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/8;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z - GRASS_SIZE/4;
            *pReal++ = 1.0;
            *pReal++ = 1.0;
            ++numVertices;
#else
            /*  _\/_  */
            /*   /\   */
            // Horizontal
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/2.5f;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE *z;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/2.5f;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE *z;
            *pReal++ = 0.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/2.5f;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE *z;
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/2.5f;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE *z;
            *pReal++ = 1.0;
            *pReal++ = 1.0;
            ++numVertices;
            // Vertical 1
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/4.5f;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/3.0f;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/4.5f;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z - GRASS_SIZE/3.0f;
            *pReal++ = 0.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/4.5f;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z - GRASS_SIZE/3.0f;
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/4.5f;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/3.0f;
            *pReal++ = 1.0;
            *pReal++ = 1.0;
            ++numVertices;
            // Vertical 2
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/4.5f;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/3.0f;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/4.5f;
            *pReal++ = height;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z - GRASS_SIZE/3.0f;
            *pReal++ = 0.0;
            *pReal++ = 0.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x + GRASS_SIZE/4.5f;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z - GRASS_SIZE/3.0f;
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = TileManager::TILE_RENDER_SIZE * x - GRASS_SIZE/4.5f;
            *pReal++ = height-TileManager::TILE_RENDER_SIZE/2;
            *pReal++ = TileManager::TILE_RENDER_SIZE * z + GRASS_SIZE/3.0f;
            *pReal++ = 1.0;
            *pReal++ = 1.0;
            ++numVertices;
#endif
        }
    }
    vbuf->unlock();
    mSubMeshGrassNear->indexData->indexCount = numVertices*2*3; // 2 tris/subtile * 3 vertices/triangle.
    mSubMeshGrassNear->vertexData->vertexCount = numVertices*4; // 4 vertices/subtile.
}

//================================================================================================
//
//================================================================================================
void TileChunk::setRenderOptions(bool drawUndergrowth)
{
    PROFILE()
    if (drawUndergrowth == mUndergrowth) return;
    if (!drawUndergrowth)
    {
        mEntityGrassFar->getParentSceneNode()->detachObject(mEntityGrassFar);
    }
    else
    {
        SceneNode *sn = static_cast<SceneNode*> (TileManager::getSingleton().getSceneManager()->getRootSceneNode()->getChild("snTileChunk"));
        sn->attachObject(mEntityGrassFar);
    }
    mUndergrowth = drawUndergrowth;
}
