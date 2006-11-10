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

#include "define.h"
#include "option.h"
#include "logger.h"
#include "object_manager.h"
#include "tile_interface.h"
#include "tile_manager.h"

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
    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z)
        {
            if (TileManager::getSingleton().getMapHeight(x, z  ) <= LEVEL_WATER_TOP || TileManager::getSingleton().getMapHeight(x+1, z  ) <= LEVEL_WATER_TOP ||
                    TileManager::getSingleton().getMapHeight(x, z+1) <= LEVEL_WATER_TOP || TileManager::getSingleton().getMapHeight(x+1, z+1) <= LEVEL_WATER_TOP)
            {
                numVertices += 6;
            }
        }
    }
    if (numVertices == 0)
    {
        createDummy(mSubMeshWater);
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Create VertexData.
    // ////////////////////////////////////////////////////////////////////
    VertexData* vdata = new VertexData();
    vdata->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent
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

    static Real offsetWave = 0.05;
    static Real WaveHigh = 0;
    WaveHigh+= offsetWave;
    if (WaveHigh >1.5 || WaveHigh < -1.5) offsetWave*=-1;

    Real* pReal = static_cast<Real*>(vbuf0->lock (HardwareBuffer::HBL_DISCARD));
    Real q1, q2;
    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z)
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
                pReal[o   ] = TILE_SIZE_X * x;
                pReal[o+ 1] = q1;
                pReal[o+ 2] = TILE_SIZE_Z * z;
                pReal[o+10] = TILE_SIZE_X * (x+1);
                pReal[o+11] = q2;
                pReal[o+12] = TILE_SIZE_Z * (z);
                pReal[o+20] = TILE_SIZE_X * (x);
                pReal[o+21] = q2;
                pReal[o+22] = TILE_SIZE_Z * (z+1);
                // 2. Triangle
                pReal[o+30] = TILE_SIZE_X * x;
                pReal[o+31] = q2;
                pReal[o+32] = TILE_SIZE_Z * (z+1);
                pReal[o+40] = TILE_SIZE_X * (x+1);
                pReal[o+41] = q2;
                pReal[o+42] = TILE_SIZE_Z * (z);
                pReal[o+50] = TILE_SIZE_X * (x+1);
                pReal[o+51] = q1;
                pReal[o+52] = TILE_SIZE_Z * (z+1);

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
// +------+
// |\  2 /|
// | \  / |
// |  \/  |
// |1 /\ 3|
// | /  \ |
// |/  4 \|
// +------+
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
    unsigned int numVertices = CHUNK_SIZE_X * CHUNK_SIZE_Z * 12;

    // ////////////////////////////////////////////////////////////////////
    // Create VertexData.
    // ////////////////////////////////////////////////////////////////////
    VertexData* vdata = new VertexData();
    vdata->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent
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

    char indoorTris;
    long o = 0;
    Real g, h, d, f, row, col, rowIndoor =0, colIndoor=0;
    Real average;
    Real* pReal1 = static_cast<Real*>(vbuf0->lock (HardwareBuffer::HBL_DISCARD));
    const Real MIPMAP_SPACE = 4;
    // We divide a tile into 4 (2x2) subtiles.
    // On odd x positions a subtile from right half (even -> left half) of the tile is drawn.
    // On odd z positions a subtile from lower half (even ->upper half) of the tile is drawn.
    const Real SUB_POS = (64.0 + 2 * MIPMAP_SPACE) / 1024.0;

    for (int x= 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z)
        {
            g = TileManager::getSingleton().getMapHeight(x  , z  );
            h = TileManager::getSingleton().getMapHeight(x+1, z  );
            d = TileManager::getSingleton().getMapHeight(x  , z+1);
            f = TileManager::getSingleton().getMapHeight(x+1, z+1);
            average = (g + h + d + f) / 4.0;
            // ////////////////////////////////////////////////////////////////////
            // Position.
            // ////////////////////////////////////////////////////////////////////
            // 1. Triangle
            pReal1[o   ] = TILE_SIZE_X * x;
            pReal1[o+ 1] = d;
            pReal1[o+ 2] = TILE_SIZE_Z * (z+1);
            pReal1[o+12] = TILE_SIZE_X * x;
            pReal1[o+13] = g;
            pReal1[o+14] = TILE_SIZE_Z * z;
            pReal1[o+24] = TILE_SIZE_X * (x+.5);
            pReal1[o+25] = average;
            pReal1[o+26] = TILE_SIZE_Z * (z+.5);
            // 2. Triangle
            pReal1[o+36] = TILE_SIZE_X * x;
            pReal1[o+37] = g;
            pReal1[o+38] = TILE_SIZE_Z * z;
            pReal1[o+48] = TILE_SIZE_X * (x+1);
            pReal1[o+49] = h;
            pReal1[o+50] = TILE_SIZE_Z * z;
            pReal1[o+60] = TILE_SIZE_X * (x +.5);
            pReal1[o+61] = average;
            pReal1[o+62] = TILE_SIZE_Z * (z +.5);
            // 3. Triangle
            pReal1[o+72] = TILE_SIZE_X * (x+1);
            pReal1[o+73] = h;
            pReal1[o+74] = TILE_SIZE_Z * z;
            pReal1[o+84] = TILE_SIZE_X * (x +1);
            pReal1[o+85] = f;
            pReal1[o+86] = TILE_SIZE_Z * (z +1);
            pReal1[o+96] = TILE_SIZE_X * (x+.5);
            pReal1[o+97] = average;
            pReal1[o+98] = TILE_SIZE_Z * (z+.5);
            // 4. Triangle
            pReal1[o+108] = TILE_SIZE_X * (x +1);
            pReal1[o+109] = f;
            pReal1[o+110] = TILE_SIZE_Z * (z +1);
            pReal1[o+120] = TILE_SIZE_X * x;
            pReal1[o+121] = d;
            pReal1[o+122] = TILE_SIZE_Z * (z +1);
            pReal1[o+132] = TILE_SIZE_X * (x+.5);
            pReal1[o+133] = average;
            pReal1[o+134] = TILE_SIZE_Z * (z+.5);

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
            if ((indoorTris = TileManager::getSingleton().getIndoorTris(x,z)))
            {
                colIndoor = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorCol(x,z) + MIPMAP_SPACE/1024;
                rowIndoor = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorRow(x,z) + MIPMAP_SPACE/1024;
                if ((mapX&1)) colIndoor+= SUB_POS;
                if ((mapZ&1)) rowIndoor+= SUB_POS;
            }
            col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureCol(x,z)+ MIPMAP_SPACE/1024;
            row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapTextureRow(x,z)+ MIPMAP_SPACE/1024;
            if ((mapX&1)) col+= SUB_POS;
            if ((mapZ&1)) row+= SUB_POS;
            // 1. Triangle
            if (indoorTris & TileManager::TRIANGLE_LEFT)
            {   // Inodoor
                pReal1[o+  6] = colIndoor;
                pReal1[o+  7] = rowIndoor;
                pReal1[o+ 18] = colIndoor;
                pReal1[o+ 19] = rowIndoor + 64.0 /1024.0;
                pReal1[o+ 30] = colIndoor + 32.0 /1024.0;
                pReal1[o+ 31] = rowIndoor + 32.0 /1024.0;
            }
            else
            {
                pReal1[o+  6] = col;
                pReal1[o+  7] = row;
                pReal1[o+ 18] = col;
                pReal1[o+ 19] = row + 64.0 /1024.0;
                pReal1[o+ 30] = col + 32.0 /1024.0;
                pReal1[o+ 31] = row + 32.0 /1024.0;
            }
            // 2. Triangle
            if (indoorTris & TileManager::TRIANGLE_TOP)
            {   // Inodoor
                pReal1[o+ 42] = colIndoor;
                pReal1[o+ 43] = rowIndoor + 64.0 /1024.0;
                pReal1[o+ 54] = colIndoor + 64.0 /1024.0;
                pReal1[o+ 55] = rowIndoor + 64.0 /1024.0;
                pReal1[o+ 66] = colIndoor + 32.0 /1024.0;
                pReal1[o+ 67] = rowIndoor + 32.0 /1024.0;
            }
            else
            {
                pReal1[o+ 42] = col;
                pReal1[o+ 43] = row + 64.0 /1024.0;
                pReal1[o+ 54] = col + 64.0 /1024.0;
                pReal1[o+ 55] = row + 64.0 /1024.0;
                pReal1[o+ 66] = col + 32.0 /1024.0;
                pReal1[o+ 67] = row + 32.0 /1024.0;
            }
            // 3. Triangle (right)
            if (indoorTris & TileManager::TRIANGLE_RIGHT)
            {   // Inodoor
                pReal1[o+ 78] = colIndoor + 64.0 /1024.0;
                pReal1[o+ 79] = rowIndoor + 64.0 /1024.0;
                pReal1[o+ 90] = colIndoor + 64.0 /1024.0;
                pReal1[o+ 91] = rowIndoor;
                pReal1[o+102] = colIndoor + 32.0 /1024.0;
                pReal1[o+103] = rowIndoor + 32.0 /1024.0;
            }
            else
            {
                pReal1[o+ 78] = col + 64.0 /1024.0;
                pReal1[o+ 79] = row + 64.0 /1024.0;
                pReal1[o+ 90] = col + 64.0 /1024.0;
                pReal1[o+ 91] = row;
                pReal1[o+102] = col + 32.0 /1024.0;
                pReal1[o+103] = row + 32.0 /1024.0;
            }
            // 4. Triangle (bottom)
            if (indoorTris & TileManager::TRIANGLE_BOTTOM)
            {   // Inodoor
                pReal1[o+114] = colIndoor + 64.0 /1024.0;
                pReal1[o+115] = rowIndoor;
                pReal1[o+126] = colIndoor;
                pReal1[o+127] = rowIndoor;
                pReal1[o+138] = colIndoor + 32.0 /1024.0;
                pReal1[o+139] = rowIndoor + 32.0 /1024.0;
            }
            else
            {
                pReal1[o+114] = col + 64.0 /1024.0;
                pReal1[o+115] = row;
                pReal1[o+126] = col;
                pReal1[o+127] = row;
                pReal1[o+138] = col + 32.0 /1024.0;
                pReal1[o+139] = row + 32.0 /1024.0;
            }
            // ////////////////////////////////////////////////////////////////////
            // Filter-Texture.
            // ////////////////////////////////////////////////////////////////////
            // 1. Triangle (left)
            indoorTris = TileManager::getSingleton().getIndoorTris(x,z);
            if (indoorTris & TileManager::TRIANGLE_LEFT)
            {   // Inodoor
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorCol(x, z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorRow(x, z)+ MIPMAP_SPACE/1024;
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
            indoorTris = TileManager::getSingleton().getIndoorTris(x,z);
            if (indoorTris & TileManager::TRIANGLE_TOP)
            {   // Inodoor
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorCol(x, z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorRow(x, z)+ MIPMAP_SPACE/1024;
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
            indoorTris = TileManager::getSingleton().getIndoorTris(x,z);
            if (indoorTris & TileManager::TRIANGLE_RIGHT)
            {   // Inodoor
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorCol(x, z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorRow(x, z)+ MIPMAP_SPACE/1024;
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
            indoorTris = TileManager::getSingleton().getIndoorTris(x,z);
            if (indoorTris & TileManager::TRIANGLE_BOTTOM)
            {   // Inodoor
                col = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorCol(x, z)+ MIPMAP_SPACE/1024;
                row = (128 + 4 * MIPMAP_SPACE) /1024.0 * TileManager::getSingleton().getMapIndoorRow(x, z)+ MIPMAP_SPACE/1024;
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
