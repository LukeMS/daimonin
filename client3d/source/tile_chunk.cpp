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

//================================================================================================
// The map has its zero-position in the top-left corner:
// (x,z)
//  0,0 1,0 2,0 3,0
//  0,1 1,1 2,1 3,1
//  0,2 1,2 2,2 3,2
//  0,3 1,3 2,3 3,3
//================================================================================================

//================================================================================================
// Init static elements.
//================================================================================================
AxisAlignedBox *TileChunk::mBounds = 0;

//================================================================================================
// Constructor.
//================================================================================================
TileChunk::TileChunk()
{
    mEntityLand= 0;
    mEntityLand= 0;
}

//================================================================================================
// Destructor.
//================================================================================================
TileChunk::~TileChunk()
{}

//================================================================================================
// .
//================================================================================================
void TileChunk::freeRecources()
{
    mMeshWater.setNull();
    mMeshLand.setNull();
}

//================================================================================================
// Set a new Material.
//================================================================================================
void TileChunk::setMaterial(String matLand, String matWater)
{
    if (mEntityLand)  mEntityLand->setMaterialName(matLand);
    if (mEntityWater) mEntityWater->setMaterialName(matWater);
}

//================================================================================================
// Create a new Chunk.
//================================================================================================
void TileChunk::create(int tileTextureSize)
{
    createLand(tileTextureSize);
    mNodeLand= TileManager::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();
    mNodeLand->setPosition(0, 0, 0);
    mNodeLand->attachObject(mEntityLand);
    createWater();
    mNodeWater= TileManager::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();
    mNodeWater->setPosition(0, 0, 0);
    mNodeWater->attachObject(mEntityWater);
    // mNodeWater->showBoundingBox(true);
}

//================================================================================================
// Change a Chunk.
//================================================================================================
void TileChunk::change()
{
    changeLand();
    changeWater();
}

//================================================================================================
// Create Water.
// +---+
// |  /|
// | / |
// |/  |
// +---+
//================================================================================================
void TileChunk::createWater()
{
    // ////////////////////////////////////////////////////////////////////
    // Create Mesh with a SubMesh.
    // ////////////////////////////////////////////////////////////////////
    mMeshWater = MeshManager::getSingleton().createManual("Mesh_Water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMeshWater = mMeshWater->createSubMesh("SubMesh_Water");
    createWater_Buffers();
    mMeshWater->_setBounds(*mBounds); // Rendering is only done when Camera looks into this quad.
    mMeshWater->load();
    mEntityWater = TileManager::getSingleton().getSceneManager()->createEntity("Entity_Water", "Mesh_Water");
    mEntityWater->setQueryFlags(ObjectManager::QUERY_TILES_WATER_MASK);
    mEntityWater->setRenderQueueGroup(RENDER_QUEUE_8);
}

//================================================================================================
// Change Water.
//================================================================================================
void TileChunk::changeWater()
{
    delete mSubMeshWater->vertexData;
    createWater_Buffers();
}

//================================================================================================
// Create Hardware Buffers for high quality water.
//================================================================================================
void TileChunk::createWater_Buffers()
{
    int mapX, mapZ;
    // ////////////////////////////////////////////////////////////////////
    // Count the Vertices in this chunk.
    // ////////////////////////////////////////////////////////////////////
    unsigned int numVertices = 0;
    for (int x = 0; x < TileManager::CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE_Z; ++z)
        {
            if (TileManager::getSingleton().getMapHeight(x, z  ) <= LEVEL_WATER_TOP || TileManager::getSingleton().getMapHeight(x+1, z  ) <= LEVEL_WATER_TOP ||
                    TileManager::getSingleton().getMapHeight(x, z+1) <= LEVEL_WATER_TOP || TileManager::getSingleton().getMapHeight(x+1, z+1) <= LEVEL_WATER_TOP)
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
        createDummy(mSubMeshWater);
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Create VertexData.
    // ////////////////////////////////////////////////////////////////////
    VertexData* vdata = new VertexData();
    vdata->vertexCount = numVertices;
    VertexDeclaration* vdec = vdata->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement( 0, offset, VET_FLOAT3, VES_POSITION );
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement( 0, offset, VET_FLOAT3, VES_NORMAL );
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1);
    offset += VertexElement::getTypeSize(VET_FLOAT2);

    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
                offset, // size of one whole vertex
                numVertices, // number of vertices
                HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                false); // no shadow buffer
    vdata->vertexBufferBinding->setBinding(0, vbuf0);

    long o = 0;

    static Real offsetWave = 0.03;
    static Real WaveHigh = 0;
    WaveHigh+= offsetWave;
    if (WaveHigh >1.7 || WaveHigh < -1.7) offsetWave*=-1;

    Real* pReal = static_cast<Real*>(vbuf0->lock (HardwareBuffer::HBL_DISCARD));
    Real q1, q2;
    for (int x = 0; x < TileManager::CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE_Z; ++z)
        {
            if (TileManager::getSingleton().getMapHeight(x, z  ) <= LEVEL_WATER_TOP ||TileManager::getSingleton().getMapHeight(x+1, z  ) <= LEVEL_WATER_TOP ||
                    TileManager::getSingleton().getMapHeight(x, z+1) <= LEVEL_WATER_TOP || TileManager::getSingleton().getMapHeight(x+1, z+1) <= LEVEL_WATER_TOP)
            {
                // ////////////////////////////////////////////////////////////////////
                // Position.
                // ////////////////////////////////////////////////////////////////////
                TileManager::getSingleton().getMapScroll(mapX, mapZ);
                mapX-=x;
                mapZ-=z;
                if ((mapX&1) != (mapZ&1))
                {
                    q1 = LEVEL_WATER_CLP + WaveHigh;
                    q2 = LEVEL_WATER_CLP - WaveHigh;
                }
                else
                {
                    q1 = LEVEL_WATER_CLP - WaveHigh;
                    q2 = LEVEL_WATER_CLP + WaveHigh;
                }
                // 1. Triangle
                pReal[o   ] = TileManager::TILE_SIZE * x;
                pReal[o+ 1] = q1;
                pReal[o+ 2] = TileManager::TILE_SIZE * z;
                pReal[o+10] = TileManager::TILE_SIZE * (x+1);
                pReal[o+11] = q2;
                pReal[o+12] = TileManager::TILE_SIZE * (z);
                pReal[o+20] = TileManager::TILE_SIZE * (x);
                pReal[o+21] = q2;
                pReal[o+22] = TileManager::TILE_SIZE * (z+1);
                // 2. Triangle
                pReal[o+30] = TileManager::TILE_SIZE * x;
                pReal[o+31] = q2;
                pReal[o+32] = TileManager::TILE_SIZE * (z+1);
                pReal[o+40] = TileManager::TILE_SIZE * (x+1);
                pReal[o+41] = q2;
                pReal[o+42] = TileManager::TILE_SIZE * (z);
                pReal[o+50] = TileManager::TILE_SIZE * (x+1);
                pReal[o+51] = q1;
                pReal[o+52] = TileManager::TILE_SIZE * (z+1);

                // ////////////////////////////////////////////////////////////////////
                // Normalvektoren
                // ////////////////////////////////////////////////////////////////////
                // 1. Triangle
                pReal[o+ 3] = 0;
                pReal[o+ 4] = 1;
                pReal[o+ 5] = 0;
                pReal[o+13] = 0;
                pReal[o+14] = 1;
                pReal[o+15] = 0;
                pReal[o+23] = 0;
                pReal[o+24] = 1;
                pReal[o+25] = 0;
                // 2. Triangle
                pReal[o+33] = 0;
                pReal[o+34] = 1;
                pReal[o+35] = 0;
                pReal[o+43] = 0;
                pReal[o+44] = 1;
                pReal[o+45] = 0;
                pReal[o+53] = 0;
                pReal[o+54] = 1;
                pReal[o+55] = 0;

                // ////////////////////////////////////////////////////////////////////
                // Texture.
                // ////////////////////////////////////////////////////////////////////
                Real offX = 1-(Real)((mapX)&3) * 0.25;
                Real offZ = 1-(Real)((mapZ)&3) * 0.25;
                // 1. Triangle
                pReal[o+ 6] = offX;
                pReal[o+ 7] = offZ;
                pReal[o+16] = offX + 0.25;
                pReal[o+17] = offZ;
                pReal[o+26] = offX;
                pReal[o+27] = offZ + 0.25;
                // 2. Triangle
                pReal[o+36] = offX;
                pReal[o+37] = offZ + 0.25;
                pReal[o+46] = offX + 0.25;
                pReal[o+47] = offZ;
                pReal[o+56] = offX + 0.25;
                pReal[o+57] = offZ + 0.25;

                // ////////////////////////////////////////////////////////////////////
                // Grid-Texture.
                // ////////////////////////////////////////////////////////////////////
                // 1. Triangle
                pReal[o+ 8] = 0.0;
                pReal[o+ 9] = 0.0;
                pReal[o+18] = 1.0;
                pReal[o+19] = 0.0;
                pReal[o+28] = 0.0;
                pReal[o+29] = 1.0;
                // 2. Triangle
                pReal[o+38] = 0.0;
                pReal[o+39] = 1.0;
                pReal[o+48] = 1.0;
                pReal[o+49] = 0.0;
                pReal[o+58] = 1.0;
                pReal[o+59] = 1.0;

                o += 60;
            }
        }
    }
    vbuf0->unlock();
    // ////////////////////////////////////////////////////////////////////
    // Create Index-buffer
    // ////////////////////////////////////////////////////////////////////
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
                                            HardwareIndexBuffer::IT_16BIT, // type of index
                                            numVertices, // number of indexes
                                            HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                                            false); // no shadow buffer
    IndexData* idata = mSubMeshWater->indexData;
    idata->indexBuffer = ibuf;
    idata->indexStart = 0;
    idata->indexCount = numVertices;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock (HardwareBuffer::HBL_DISCARD));
    for (unsigned short p=0; numVertices; ++p)
        pIdx[--numVertices] = p;
    ibuf->unlock();

    mSubMeshWater->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshWater->useSharedVertices = false;
    mSubMeshWater->vertexData = vdata;
}

//================================================================================================
// Create Land in high Quality. 1 Tile = 4 triangles. We need this for the filter.
//  +------+
//  |\  2 /|
//  | \  / |
//  |  \/  |
//  |1 /\ 3|
//  | /  \ |
//  |/  4 \|
//  +------+
//================================================================================================
void TileChunk::createLand(int tileTextureSize)
{
    // ////////////////////////////////////////////////////////////////////
    // Create Mesh with a SubMesh.
    // ////////////////////////////////////////////////////////////////////
    mMeshLand = MeshManager::getSingleton().createManual("Mesh_Land", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
    mSubMeshLand = mMeshLand->createSubMesh("SubMesh_Land");
    String matLand = "Land_HighDetails" + StringConverter::toString(tileTextureSize, 3, '0');
    createLand_Buffers();
    mMeshLand->_setBounds(*mBounds); // Rendering is only done when Camera looks into this quad.
    mMeshLand->load();
    mEntityLand = TileManager::getSingleton().getSceneManager()->createEntity("Entity_Land", "Mesh_Land");
    mEntityLand->setQueryFlags(ObjectManager::QUERY_TILES_LAND_MASK);
    mEntityLand->setRenderQueueGroup(RENDER_QUEUE_1);
}

//================================================================================================
// Change Land in high Quality
//================================================================================================
void TileChunk::changeLand()
{
    delete mSubMeshLand->vertexData;
    createLand_Buffers();
}

//================================================================================================
// Create Hardware Buffers for high quality land.
//================================================================================================
void TileChunk::createLand_Buffers()
{
    int mapX, mapZ;
    unsigned int numVertices = TileManager::CHUNK_SIZE_X * TileManager::CHUNK_SIZE_Z * 12;

    // ////////////////////////////////////////////////////////////////////
    // Create VertexData.
    // ////////////////////////////////////////////////////////////////////
    VertexData* vdata = new VertexData();
    vdata->vertexCount = numVertices;
    VertexDeclaration* vdec = vdata->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement( 0, offset, VET_FLOAT3, VES_POSITION );
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement( 0, offset, VET_FLOAT3, VES_NORMAL );
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0); // Ground-texture.
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1); // Filter-texture.
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 2); // Grid-texture.
    offset += VertexElement::getTypeSize(VET_FLOAT2);

    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
                offset, // size of one whole vertex
                numVertices, // number of vertices
                HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                false); // no shadow buffer

    vdata->vertexBufferBinding->setBinding(0, vbuf0);

    bool indoor;
    long o = 0;
    Real row, col;
    Real* pReal1 = static_cast<Real*>(vbuf0->lock (HardwareBuffer::HBL_DISCARD));
    const Real MIPMAP_SPACE = 4;
    // We divide a tile into 4 (2x2) subtiles.
    // On odd x positions a subtile from right half (even -> left half) of the tile is drawn.
    // On odd z positions a subtile from lower half (even ->upper half) of the tile is drawn.
    const Real SUB_POS = (64.0 + 2 * MIPMAP_SPACE) / 1024.0;

    for (int x= 0; x < TileManager::CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE_Z; ++z)
        {
            // ////////////////////////////////////////////////////////////////////
            // Position.
            // ////////////////////////////////////////////////////////////////////
            // 1. Triangle
            pReal1[o   ] = TileManager::TILE_SIZE * x;
            pReal1[o+ 1] = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BL);
            pReal1[o+ 2] = TileManager::TILE_SIZE * (z+1);
            pReal1[o+12] = TileManager::TILE_SIZE * x;
            pReal1[o+13] = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TL);
            pReal1[o+14] = TileManager::TILE_SIZE * z;
            pReal1[o+24] = TileManager::TILE_SIZE * (x+.5);
            pReal1[o+25] = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_AVG);
            pReal1[o+26] = TileManager::TILE_SIZE * (z+.5);
            // 2. Triangle
            pReal1[o+36] = TileManager::TILE_SIZE * x;
            pReal1[o+37] = pReal1[o+13];
            pReal1[o+38] = TileManager::TILE_SIZE * z;
            pReal1[o+48] = TileManager::TILE_SIZE * (x+1);
            pReal1[o+49] = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TR);
            pReal1[o+50] = TileManager::TILE_SIZE * z;
            pReal1[o+60] = TileManager::TILE_SIZE * (x +.5);
            pReal1[o+61] = pReal1[o+25];
            pReal1[o+62] = TileManager::TILE_SIZE * (z +.5);
            // 3. Triangle
            pReal1[o+72] = TileManager::TILE_SIZE * (x+1);
            pReal1[o+73] = pReal1[o+49];
            pReal1[o+74] = TileManager::TILE_SIZE * z;
            pReal1[o+84] = TileManager::TILE_SIZE * (x +1);
            pReal1[o+85] = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BR);
            pReal1[o+86] = TileManager::TILE_SIZE * (z +1);
            pReal1[o+96] = TileManager::TILE_SIZE * (x+.5);
            pReal1[o+97] = pReal1[o+25];
            pReal1[o+98] = TileManager::TILE_SIZE * (z+.5);
            // 4. Triangle
            pReal1[o+108] = TileManager::TILE_SIZE * (x +1);
            pReal1[o+109] = pReal1[o+85];
            pReal1[o+110] = TileManager::TILE_SIZE * (z +1);
            pReal1[o+120] = TileManager::TILE_SIZE * x;
            pReal1[o+121] = pReal1[o+ 1];
            pReal1[o+122] = TileManager::TILE_SIZE * (z +1);
            pReal1[o+132] = TileManager::TILE_SIZE * (x+.5);
            pReal1[o+133] = pReal1[o+25];
            pReal1[o+134] = TileManager::TILE_SIZE * (z+.5);
            // ////////////////////////////////////////////////////////////////////
            // Normalvektoren
            // ////////////////////////////////////////////////////////////////////
            // 1. Triangle
            pReal1[o+  3] = 0.0;
            pReal1[o+  4] = 1.0;
            pReal1[o+  5] = 0.0;
            pReal1[o+ 15] = 0.0;
            pReal1[o+ 16] = 1.0;
            pReal1[o+ 17] = 0.0;
            pReal1[o+ 27] = 0.0;
            pReal1[o+ 28] = 1.0;
            pReal1[o+ 29] = 0.0;
            // 2. Triangle
            pReal1[o+ 39] = 0.0;
            pReal1[o+ 40] = 1.0;
            pReal1[o+ 41] = 0.0;
            pReal1[o+ 51] = 0.0;
            pReal1[o+ 52] = 1.0;
            pReal1[o+ 53] = 0.0;
            pReal1[o+ 63] = 0.0;
            pReal1[o+ 64] = 1.0;
            pReal1[o+ 65] = 0.0;
            // 3. Triangle
            pReal1[o+ 75] = 0.0;
            pReal1[o+ 76] = 1.0;
            pReal1[o+ 77] = 0.0;
            pReal1[o+ 87] = 0.0;
            pReal1[o+ 88] = 1.0;
            pReal1[o+ 89] = 0.0;
            pReal1[o+ 99] = 0.0;
            pReal1[o+100] = 1.0;
            pReal1[o+101] = 0.0;
            // 4. Triangle
            pReal1[o+111] = 0.0;
            pReal1[o+112] = 1.0;
            pReal1[o+113] = 0.0;
            pReal1[o+123] = 0.0;
            pReal1[o+124] = 1.0;
            pReal1[o+125] = 0.0;
            pReal1[o+135] = 0.0;
            pReal1[o+136] = 1.0;
            pReal1[o+137] = 0.0;
            // ////////////////////////////////////////////////////////////////////
            // Ground-Texture.
            // ////////////////////////////////////////////////////////////////////
            TileManager::getSingleton().getMapScroll(mapX, mapZ);
            mapX-=x;
            mapZ-=z;
            col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x,z)+ MIPMAP_SPACE/1024;
            row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x,z)+ MIPMAP_SPACE/1024;
            if ((mapX&1)) col+= SUB_POS;
            if ((mapZ&1)) row+= SUB_POS;
            // 1. Triangle
            pReal1[o+  6] = col;
            pReal1[o+  7] = row;
            pReal1[o+ 18] = col;
            pReal1[o+ 19] = row + 64.0 /1024.0;
            pReal1[o+ 30] = col + 32.0 /1024.0;
            pReal1[o+ 31] = row + 32.0 /1024.0;
            // 2. Triangle
            pReal1[o+ 42] = col;
            pReal1[o+ 43] = row + 64.0 /1024.0;
            pReal1[o+ 54] = col + 64.0 /1024.0;
            pReal1[o+ 55] = row + 64.0 /1024.0;
            pReal1[o+ 66] = col + 32.0 /1024.0;
            pReal1[o+ 67] = row + 32.0 /1024.0;
            // 3. Triangle (right)
            pReal1[o+ 78] = col + 64.0 /1024.0;
            pReal1[o+ 79] = row + 64.0 /1024.0;
            pReal1[o+ 90] = col + 64.0 /1024.0;
            pReal1[o+ 91] = row;
            pReal1[o+102] = col + 32.0 /1024.0;
            pReal1[o+103] = row + 32.0 /1024.0;
            // 4. Triangle (left)
            pReal1[o+114] = col + 64.0 /1024.0;
            pReal1[o+115] = row;
            pReal1[o+126] = col;
            pReal1[o+127] = row;
            pReal1[o+138] = col + 32.0 /1024.0;
            pReal1[o+139] = row + 32.0 /1024.0;
            // ////////////////////////////////////////////////////////////////////
            // Filter-Texture.
            // ////////////////////////////////////////////////////////////////////
            // 1. Triangle (left)
            indoor = TileManager::getSingleton().getIndoor(x,z);
            if (indoor || TileManager::getSingleton().getIndoor(x-1,z))
            {   // Inodoor
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x,z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x,z)+ MIPMAP_SPACE/1024;
            }
            else
            {
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x-1, z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x-1, z)+ MIPMAP_SPACE/1024;
            }
            if ((mapX&1)) col+= SUB_POS;
            if ((mapZ&1)) row+= SUB_POS;
            pReal1[o+  8] = col;
            pReal1[o+  9] = row;
            pReal1[o+ 20] = col;
            pReal1[o+ 21] = row + 64.0 /1024.0;
            pReal1[o+ 32] = col + 32.0 /1024.0;
            pReal1[o+ 33] = row + 32.0 /1024.0;
            // 2. Triangle
            if (indoor|| TileManager::getSingleton().getIndoor(x,z-1))
            {   // Inodoor
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x,z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x,z)+ MIPMAP_SPACE/1024;
            }
            else
            {
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x, z-1)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x, z-1)+ MIPMAP_SPACE/1024;
            }
            if ((mapX&1)) col+= SUB_POS;
            if ((mapZ&1)) row+= SUB_POS;
            pReal1[o+ 44] = col;
            pReal1[o+ 45] = row + 64.0 /1024.0;
            pReal1[o+ 56] = col + 64.0 /1024.0;
            pReal1[o+ 57] = row + 64.0 /1024.0;
            pReal1[o+ 68] = col + 32.0 /1024.0;
            pReal1[o+ 69] = row + 32.0 /1024.0;
            // 3. Triangle
            if (indoor|| TileManager::getSingleton().getIndoor(x+1,z))
            {   // Inodoor
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x,z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x,z)+ MIPMAP_SPACE/1024;
            }
            else
            {
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x+1, z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x+1, z)+ MIPMAP_SPACE/1024;
            }
            if ((mapX&1)) col+= SUB_POS;
            if ((mapZ&1)) row+= SUB_POS;
            pReal1[o+ 80] = col + 64.0 /1024.0;
            pReal1[o+ 81] = row + 64.0 /1024.0;
            pReal1[o+ 92] = col + 64.0 /1024.0;
            pReal1[o+ 93] = row;
            pReal1[o+104] = col + 32.0 /1024.0;
            pReal1[o+105] = row + 32.0 /1024.0;
            // 4. Triangle
            if (indoor || TileManager::getSingleton().getIndoor(x,z+1))
            {   // Inodoor
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x,z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x,z)+ MIPMAP_SPACE/1024;
            }
            else
            {
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x, z+1)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x, z+1)+ MIPMAP_SPACE/1024;
            }
            if ((mapX&1)) col+= SUB_POS;
            if ((mapZ&1)) row+= SUB_POS;
            pReal1[o+116] = col + 64.0 /1024.0;
            pReal1[o+117] = row;
            pReal1[o+128] = col;
            pReal1[o+129] = row;
            pReal1[o+140] = col + 32.0 /1024.0;
            pReal1[o+141] = row + 32.0 /1024.0;
            // ////////////////////////////////////////////////////////////////////
            // Grid-Texture.
            // ////////////////////////////////////////////////////////////////////
            // 1. Triangle
            pReal1[o+ 10] = 0.0;
            pReal1[o+ 11] = 0.0;
            pReal1[o+ 22] = 0.0;
            pReal1[o+ 23] = 1.0;
            pReal1[o+ 34] = 0.5;
            pReal1[o+ 35] = 0.5;
            // 2. Triangle
            pReal1[o+ 46] = 0.0;
            pReal1[o+ 47] = 1.0;
            pReal1[o+ 58] = 1.0;
            pReal1[o+ 59] = 1.0;
            pReal1[o+ 70] = 0.5;
            pReal1[o+ 71] = 0.5;
            // 3. Triangle
            pReal1[o+ 82] = 1.0;
            pReal1[o+ 83] = 1.0;
            pReal1[o+ 94] = 1.0;
            pReal1[o+ 95] = 0.0;
            pReal1[o+106] = 0.5;
            pReal1[o+107] = 0.5;
            // 4. Triangle
            pReal1[o+118] = 1.0;
            pReal1[o+119] = 0.0;
            pReal1[o+130] = 0.0;
            pReal1[o+131] = 0.0;
            pReal1[o+142] = 0.5;
            pReal1[o+143] = 0.5;

            o += 144;
        }
    }
    vbuf0->unlock();
    // ////////////////////////////////////////////////////////////////////
    // Create Index-buffer
    // ////////////////////////////////////////////////////////////////////
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
                                            HardwareIndexBuffer::IT_16BIT, // type of index
                                            numVertices, // number of indexes
                                            HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                                            false); // no shadow buffer
    IndexData* idata = mSubMeshLand->indexData;
    idata->indexBuffer = ibuf;
    idata->indexStart = 0;
    idata->indexCount = numVertices;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock (HardwareBuffer::HBL_DISCARD));
    for (unsigned short p=0; numVertices; ++p)
        pIdx[--numVertices] = p;
    ibuf->unlock();

    mSubMeshLand->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshLand->useSharedVertices = false;
    mSubMeshLand->vertexData = vdata;
}

//================================================================================================
// Creates a dummy submesh containing only 1 Triangle.
//================================================================================================
void TileChunk::createDummy(SubMesh* submesh)
{
    const int numVertices = 3;
    VertexData* vdata = new VertexData();
    vdata->vertexCount = numVertices;
    VertexDeclaration* vdec = vdata->vertexDeclaration;
    vdec->addElement(0, 0, VET_FLOAT3, VES_POSITION );
    size_t offset = VertexElement::getTypeSize(VET_FLOAT3);

    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
                offset, // size of one whole vertex
                numVertices, // number of vertices
                HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                false); // no shadow buffer
    vdata->vertexBufferBinding->setBinding(0, vbuf0);
    Real* pReal = static_cast<Real*>(vbuf0->lock (HardwareBuffer::HBL_DISCARD));
    // Triangle 1
    pReal[0] = 0;
    pReal[1] = 0;
    pReal[2] = 0;
    pReal[3] = 0;
    pReal[4] = 0;
    pReal[5] = 0;
    pReal[6] = 0;
    pReal[7] = 0;
    pReal[8] = 0;
    vbuf0->unlock();

    HardwareIndexBufferSharedPtr ibuf;
    ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
               HardwareIndexBuffer::IT_16BIT, // type of index
               numVertices, // number of indexes
               HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
               false); // no shadow buffer
    IndexData* idata = submesh->indexData;
    idata->indexBuffer= ibuf;
    idata->indexStart = 0;
    idata->indexCount = numVertices;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock (HardwareBuffer::HBL_DISCARD));
    pIdx[0] = 2;
    pIdx[1] = 1;
    pIdx[2] = 0;
    ibuf->unlock();

    submesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
    submesh->useSharedVertices = false;
    submesh->vertexData = vdata;
}
