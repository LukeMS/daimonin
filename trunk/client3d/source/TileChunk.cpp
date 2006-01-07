/*-----------------------------------------------------------------------------
This source file is part of Code-Black (http://www.code-black.org)
Copyright (c) 2005 by the Code-Black Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "define.h"
#include "logger.h"
#include "TileInterface.h"
#include "TileManager.h"

///=================================================================================================
/// Init static elements.
///=================================================================================================
TileManager   *TileChunk::m_TileManagerPtr = NULL;
AxisAlignedBox *TileChunk::m_bounds = NULL;
char TileChunk::MeshName[NAME_BUFFER_SIZE];
char TileChunk::TempName[NAME_BUFFER_SIZE];

///=================================================================================================
/// Constructor.
///=================================================================================================
TileChunk::TileChunk()
{
  m_posX = -1;
  m_posZ = -1;
  m_Land_subMesh_high  = NULL;
  m_Land_subMesh_low   = NULL;
  m_Land_entity_high   = NULL;
  m_Land_entity_low    = NULL;
  m_Water_subMesh_high = NULL;
  m_Water_subMesh_low  = NULL;
  m_Water_entity_high  = NULL;
  m_Water_entity_low   = NULL;
}

///=================================================================================================
/// Destructor.
///=================================================================================================
TileChunk::~TileChunk()
{
  m_Water_Mesh_high.setNull();
  m_Water_Mesh_low.setNull();
  m_Land_Mesh_high.setNull();
  m_Land_Mesh_low.setNull();
}

///=================================================================================================
/// Create Scene-Nodes.
///=================================================================================================
void TileChunk::CreateSceneNode()
{
  m_Land  = m_TileManagerPtr->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
  m_Land ->setPosition(m_posX * TILE_SIZE * CHUNK_SIZE_X, 0, m_posZ * TILE_SIZE * CHUNK_SIZE_Z);
  m_Water = m_TileManagerPtr->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
  m_Water->setPosition(m_posX * TILE_SIZE * CHUNK_SIZE_X, 0, m_posZ * TILE_SIZE * CHUNK_SIZE_Z);
}

///=================================================================================================
/// Attach.
///=================================================================================================
void TileChunk::Attach(short quality)
{
  if (m_Land)  m_Land->detachAllObjects();
  if (m_Water) m_Water->detachAllObjects();
  if (quality == QUALITY_LOW)
  {
    if (m_Land_entity_low != NULL) m_Land->attachObject(m_Land_entity_low);
    if (m_Water_entity_low != NULL) m_Water->attachObject(m_Water_entity_low);
  }
  else if (quality == QUALITY_HIGH)
  {
    if (m_Land_entity_high != NULL)  m_Land->attachObject(m_Land_entity_high);
    if (m_Water_entity_high != NULL) m_Water->attachObject(m_Water_entity_high);
  }
}

///=================================================================================================
/// Detach.
///=================================================================================================
void TileChunk::Detach()
{
  if (m_IsAttached == true)
  {
    // tile->detachObject(TempName);
    // TileManager->Get_pSceneManager()->removeEntity(custom);
    // TileManager->Get_pSceneManager()->getRootSceneNode()->removeChild(tile);
    //String texname = m_Kartentextur->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
    //TextureManager::getSingleton().getBym_name(texm_name)->unload();
    m_IsAttached = false;
  }
}

///=================================================================================================
/// Create a new Chunk.
///=================================================================================================
void TileChunk::Create(short &x, short &z)
{
  Set_Tile(x, z);
  CreateLandLow();
  CreateLandHigh();
  CreateWaterLow();
  CreateWaterHigh();
  CreateSceneNode();
}

///=================================================================================================
/// Change a Chunk.
///=================================================================================================
void TileChunk::Change()
{
  ChangeLandHigh();
  ChangeWaterHigh();
  ChangeLandLow();
  ChangeWaterLow();
}

///=================================================================================================
/// Change low qality water.
///=================================================================================================
void TileChunk::ChangeWaterLow()
{
  delete m_Water_subMesh_low->vertexData;
  CreateWaterLow_Buffers();
}

///=================================================================================================
/// Create low qality water.
///=================================================================================================
void TileChunk::CreateWaterLow()
{
  int x = m_posX * CHUNK_SIZE_X;
  int z = m_posZ * CHUNK_SIZE_Z;

  sprintf( MeshName, "Water[%d,%d] Low", x, z );
  m_Water_Mesh_low = MeshManager::getSingleton().createManual( MeshName,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  sprintf( TempName, "SubWater[%d,%d] Low", x, z );
  m_Water_subMesh_low = m_Water_Mesh_low->createSubMesh(TempName);
  CreateWaterLow_Buffers();
  m_Water_Mesh_low->_setBounds( *m_bounds );
  m_Water_Mesh_low->load();
  m_Water_subMesh_low->setMaterialName("Water_LowDetails");
  sprintf( TempName, "Water[%d,%d] Low Entity", m_posX, m_posZ );
  m_Water_entity_low = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);
}

///=================================================================================================
/// Create Hardware Buffers for low quality water.
///=================================================================================================
void TileChunk::CreateWaterLow_Buffers()
{
  int x = m_posX * CHUNK_SIZE_X;
  int z = m_posZ * CHUNK_SIZE_Z;
  float StretchZ = m_TileManagerPtr->Get_StretchZ();
  unsigned long numVertices = 0;

  // Bestimmung der Anzahl der Geometriepunkte
  for (short a = x; a < x + CHUNK_SIZE_X; ++a)
  {
    for (short b = z; b < z + CHUNK_SIZE_Z; ++b)
    {
      if (m_TileManagerPtr->Get_Map_Height(a, b  ) <= LEVEL_WATER_TOP && m_TileManagerPtr->Get_Map_Height(a+1, b  ) <= LEVEL_WATER_TOP &&
              m_TileManagerPtr->Get_Map_Height(a, b+1) <= LEVEL_WATER_TOP && m_TileManagerPtr->Get_Map_Height(a+1, b+1) <= LEVEL_WATER_TOP)
      {
        goto hasWater;
      }
    }
  }
  if (numVertices == 0)
  {
    Create_Dummy(m_Water_subMesh_low); return;
  }

hasWater:
  VertexData* vdata = new VertexData();
  IndexData* idata = m_Water_subMesh_low->indexData;
  numVertices = 6;
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

  VertexBufferBinding* vbbind = vdata->vertexBufferBinding;
  vbbind->setBinding(0, vbuf0);

  Real *pReal = static_cast<Real*>(vbuf0->lock(HardwareBuffer::HBL_NORMAL));

  /////////////////////////////////////////////////////////////////////////
  // 1. Triangle
  /////////////////////////////////////////////////////////////////////////
  pReal[0] = 0;
  pReal[1] = LEVEL_WATER_TOP * StretchZ;
  pReal[2] = 0;
  pReal[3] = 0; pReal[4] = 1; pReal[5] = 0;
  pReal[6] = 0; pReal[7] =  0;
  pReal[8] = 0; pReal[9] = 0;
  /////////////////////////////////////////////////////////////////////////
  pReal[10] = TILE_SIZE * CHUNK_SIZE_X;
  pReal[11] = LEVEL_WATER_TOP * StretchZ;
  pReal[12] = 0;
  pReal[13] = 0; pReal[14] = 1; pReal[15] = 0;
  pReal[16] = CHUNK_SIZE_X /4; pReal[17] = 0;
  pReal[18] = 1; pReal[19] = 0;
  /////////////////////////////////////////////////////////////////////////
  pReal[20] = 0;
  pReal[21] = LEVEL_WATER_TOP * StretchZ;
  pReal[22] = TILE_SIZE * CHUNK_SIZE_Z;
  pReal[23] = 0; pReal[24] = 1; pReal[25] = 0;
  pReal[26] = 0; pReal[27] = CHUNK_SIZE_Z /4;
  pReal[28] = 0; pReal[29] = 1;
  /////////////////////////////////////////////////////////////////////////
  // 2. Triangle
  /////////////////////////////////////////////////////////////////////////
  pReal[30] = 0;
  pReal[31] = LEVEL_WATER_TOP * StretchZ;
  pReal[32] = TILE_SIZE * CHUNK_SIZE_Z;
  pReal[33] = 0; pReal[34] = 1; pReal[35] = 0;
  pReal[36] = 0; pReal[37] = CHUNK_SIZE_Z /4;
  pReal[38] = 0; pReal[39] = 1;
  /////////////////////////////////////////////////////////////////////////
  pReal[40] = TILE_SIZE * CHUNK_SIZE_X;
  pReal[41] = LEVEL_WATER_TOP * StretchZ;
  pReal[42] = 0;
  pReal[43] = 0; pReal[44] = 1; pReal[45] = 0;
  pReal[46] = CHUNK_SIZE_X /4; pReal[47] = 0;
  pReal[48] = 1; pReal[49] = 0;
  /////////////////////////////////////////////////////////////////////////
  pReal[50] = TILE_SIZE * CHUNK_SIZE_X;
  pReal[51] = LEVEL_WATER_TOP * StretchZ;
  pReal[52] = TILE_SIZE * CHUNK_SIZE_Z;
  pReal[53] = 0; pReal[54] = 1; pReal[55] = 0;
  pReal[56] = CHUNK_SIZE_X /4; pReal[57] = CHUNK_SIZE_Z /4;
  pReal[58] = 1; pReal[59] = 1;

  vbuf0->unlock();

  HardwareIndexBufferSharedPtr ibuf;
  ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
           HardwareIndexBuffer::IT_16BIT, // type of index
           numVertices, // number of indexes
           HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
           false); // no shadow buffer

  idata->indexBuffer = ibuf;
  idata->indexStart = 0;
  idata->indexCount = numVertices;

  unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
  for (unsigned short p=0; numVertices; ++p) pIdx[--numVertices] = p;
  ibuf->unlock();

  m_Water_subMesh_low->operationType = RenderOperation::OT_TRIANGLE_LIST;
  m_Water_subMesh_low->useSharedVertices = false;
  m_Water_subMesh_low->vertexData = vdata;

  m_IsAttached = false;
  // m_Water->attachObject( m_Water_entity );
}

///=================================================================================================
/// Create Water in high Quality
///=================================================================================================
void TileChunk::CreateWaterHigh()
{
  int x = m_posX * CHUNK_SIZE_X;
  int z = m_posZ * CHUNK_SIZE_Z;
  /////////////////////////////////////////////////////////////////////////
  /// Create Mesh with a SubMesh.
  /////////////////////////////////////////////////////////////////////////
  sprintf( MeshName, "Water[%d,%d] High", x, z );
  m_Water_Mesh_high = MeshManager::getSingleton().createManual( MeshName,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  sprintf( TempName, "SubWater[%d,%d] High", x, z );
  m_Water_subMesh_high = m_Water_Mesh_high->createSubMesh(TempName);
  m_Water_subMesh_high->setMaterialName("Water_HighDetails");

  CreateWaterHigh_Buffers();

  m_Water_Mesh_high->_setBounds( *m_bounds ); // Rendering is only done when Camera looks into this quad.
  m_Water_Mesh_high->load();

  sprintf( TempName, "Water[%d,%d] High Entity", m_posX, m_posZ );
  m_Water_entity_high = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);
  m_IsAttached = false;
}

///=================================================================================================
/// Change high Quality Water.
///=================================================================================================
void TileChunk::ChangeWaterHigh()
{
  delete m_Water_subMesh_high->vertexData;
  CreateWaterHigh_Buffers();
}

///=================================================================================================
/// Create Hardware Buffers for high quality water.
///=================================================================================================
void TileChunk::CreateWaterHigh_Buffers()
{
  int x = m_posX * CHUNK_SIZE_X;
  int z = m_posZ * CHUNK_SIZE_Z;

  /////////////////////////////////////////////////////////////////////////
  /// Count the Vertices in this chunk.
  /////////////////////////////////////////////////////////////////////////
  unsigned int numVertices = 0;
  for (short a = x; a < x + CHUNK_SIZE_X; ++a)
  {
    for (short b = z; b < z + CHUNK_SIZE_Z; ++b)
    {
      if (m_TileManagerPtr->Get_Map_Height(a, b  ) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) <= LEVEL_WATER_TOP ||
              m_TileManagerPtr->Get_Map_Height(a, b+1) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) <= LEVEL_WATER_TOP)
      {
        numVertices += 6;
      }
    }
  }
  if (numVertices == 0)
  {
    Create_Dummy(m_Water_subMesh_high); return;
  }

  /////////////////////////////////////////////////////////////////////////
  // Create VertexData.
  /////////////////////////////////////////////////////////////////////////
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
  Real StretchZ = m_TileManagerPtr->Get_StretchZ();
  Real* pReal = static_cast<Real*>(vbuf0->lock(HardwareBuffer::HBL_NORMAL));
  for (short a = x; a < x+ CHUNK_SIZE_X; ++a)
  {
    for (short b = z; b < z +CHUNK_SIZE_Z; ++b)
    {
      if (m_TileManagerPtr->Get_Map_Height(a, b  ) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) <= LEVEL_WATER_TOP ||
              m_TileManagerPtr->Get_Map_Height(a, b+1) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) <= LEVEL_WATER_TOP)
      {
        /////////////////////////////////////////////////////////////////////////
        // Position.
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal[o   ] = TILE_SIZE * (a-x);
        pReal[o+ 1] = LEVEL_WATER_CLP * StretchZ;
        pReal[o+ 2] = TILE_SIZE * (b-z);
        pReal[o+10] = TILE_SIZE * ((a-x)+1);
        pReal[o+11] = LEVEL_WATER_CLP* StretchZ;
        pReal[o+12] = TILE_SIZE * (b-z);
        pReal[o+20] = TILE_SIZE * (a-x);
        pReal[o+21] = LEVEL_WATER_CLP* StretchZ;
        pReal[o+22] = TILE_SIZE * ((b-z)+1);
        // 2. Triangle
        pReal[o+30] = TILE_SIZE * (a-x);
        pReal[o+31] = LEVEL_WATER_CLP* StretchZ;
        pReal[o+32] = TILE_SIZE * ((b-z)+1);
        pReal[o+40] = TILE_SIZE * ((a-x)+1);
        pReal[o+41] = LEVEL_WATER_CLP * StretchZ;
        pReal[o+42] = TILE_SIZE * (b-z);
        pReal[o+50] = TILE_SIZE * ((a-x)+1);
        pReal[o+51] = LEVEL_WATER_CLP * StretchZ;
        pReal[o+52] = TILE_SIZE * ((b-z)+1);

        /////////////////////////////////////////////////////////////////////////
        // Normalvektoren
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal[o+ 3] = 0; pReal[o+ 4] = 1; pReal[o+ 5] = 0;
        pReal[o+13] = 0; pReal[o+14] = 1; pReal[o+15] = 0;
        pReal[o+23] = 0; pReal[o+24] = 1; pReal[o+25] = 0;
        // 2. Triangle
        pReal[o+33] = 0; pReal[o+34] = 1; pReal[o+35] = 0;
        pReal[o+43] = 0; pReal[o+44] = 1; pReal[o+45] = 0;
        pReal[o+53] = 0; pReal[o+54] = 1; pReal[o+55] = 0;
        /////////////////////////////////////////////////////////////////////////
        // Texture.
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal[o+ 6] = (a-x  ) / 4.0; pReal[o+ 7] = (b-z  ) / 4.0;
        pReal[o+16] = (a+1-x) / 4.0; pReal[o+17] = (b-z  ) / 4.0;
        pReal[o+26] = (a-x  ) / 4.0; pReal[o+27] = (b+1-z) / 4.0;
        // 2. Triangle
        pReal[o+36] = (a-x  ) / 4.0; pReal[o+37] = (b+1-z) / 4.0;
        pReal[o+46] = (a+1-x) / 4.0; pReal[o+47] = (b-z  ) / 4.0;
        pReal[o+56] = (a+1-x) / 4.0; pReal[o+57] = (b+1-z) / 4.0;
#ifdef USE_LIGHTMAP
        /////////////////////////////////////////////////////////////////////////
        // Lightmap
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal[o+ 8] = a / 256.0; pReal[o+ 9] = b / 256.0;
        pReal[o+18] = (a + 1)/ 256.0; pReal[o+19] = b / 256.0;
        pReal[o+28] = a / 256.0; pReal[o+29] = (b + 1)/ 256.0;
        // 2. Triangle
        pReal[o+38] = a / 256.0; pReal[o+39] = (b + 1)/ 256.0;
        pReal[o+48] = (a + 1)/ 256.0; pReal[o+49] = b / 256.0;
        pReal[o+58] = (a + 1)/ 256.0; pReal[o+59] = (b + 1)/ 256.0;
#else
        /////////////////////////////////////////////////////////////////////////
        // Grid-Texture.
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal[o+ 8] = 0.0; pReal[o+ 9] = 0.0;
        pReal[o+18] = 1.0; pReal[o+19] = 0.0;
        pReal[o+28] = 0.0; pReal[o+29] = 1.0;
        // 2. Triangle
        pReal[o+38] = 0.0; pReal[o+39] = 1.0;
        pReal[o+48] = 1.0; pReal[o+49] = 0.0;
        pReal[o+58] = 1.0; pReal[o+59] = 1.0;
#endif
        o += 60;
      } // if
    } // y
  } // x
  vbuf0->unlock();
  /////////////////////////////////////////////////////////////////////////
  /// Create Index-buffer
  /////////////////////////////////////////////////////////////////////////
  HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
                                        HardwareIndexBuffer::IT_16BIT, // type of index
                                        numVertices, // number of indexes
                                        HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                                        false); // no shadow buffer
  IndexData* idata = m_Water_subMesh_high->indexData;
  idata->indexBuffer = ibuf;
  idata->indexStart = 0;
  idata->indexCount = numVertices;
  unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
  for (unsigned short p=0; numVertices; ++p) pIdx[--numVertices] = p;
  ibuf->unlock();

  m_Water_subMesh_high->operationType = RenderOperation::OT_TRIANGLE_LIST;
  m_Water_subMesh_high->useSharedVertices = false;
  m_Water_subMesh_high->vertexData = vdata;
}

///=================================================================================================
/// Change low quality Land.
///=================================================================================================
void TileChunk::ChangeLandLow()
{
  delete m_Land_subMesh_low->vertexData;
  CreateLandLow_Buffers();
}

///=================================================================================================
/// Create low quality Land. 1 Tile = 2 triangles.
/// +----+
/// |\ 2 |
/// | \  |
/// |  \ |
/// | 1 \|
/// +----+
///=================================================================================================
void TileChunk::CreateLandLow()
{
  int x = m_posX * CHUNK_SIZE_X;
  int z = m_posZ * CHUNK_SIZE_Z;
  /////////////////////////////////////////////////////////////////////////
  // Create Mesh with a SubMesh.
  /////////////////////////////////////////////////////////////////////////
  sprintf( MeshName, "Land[%d,%d] Low", x, z );
  m_Land_Mesh_low = MeshManager::getSingleton().createManual( MeshName,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  sprintf( TempName, "SubLand[%d,%d] Low", x, z );
  m_Land_subMesh_low = m_Land_Mesh_low->createSubMesh(TempName);

  CreateLandLow_Buffers();

  m_Land_Mesh_low->_setBounds( *m_bounds );
  m_Land_subMesh_low->setMaterialName("Land_LowDetails");
  sprintf( TempName, "Land[%d,%d] Low Entity", m_posX, m_posZ );
  m_Land_entity_low = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);
  // m_Land->attachObject( m_Land_entity_low );
  m_IsAttached = false;
}

///=================================================================================================
/// Create Hardware Buffers for low quality land.
///=================================================================================================
void TileChunk::CreateLandLow_Buffers()
{
  int x = m_posX * CHUNK_SIZE_X;
  int z = m_posZ * CHUNK_SIZE_Z;
  float StretchZ = m_TileManagerPtr->Get_StretchZ();
  unsigned int numVertices = 0;
  /////////////////////////////////////////////////////////////////////////
  // Bestimmung der Anzahl der Geometriepunkte
  /////////////////////////////////////////////////////////////////////////
  for (int a = x; a < x + CHUNK_SIZE_X; a += 2)
  {
    for (int b = z; b < z + CHUNK_SIZE_Z; b += 2)
    {
      if (m_TileManagerPtr->Get_Map_Height(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) > LEVEL_WATER_TOP ||
              m_TileManagerPtr->Get_Map_Height(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) > LEVEL_WATER_TOP)
      {
        numVertices += 6;
      }
    }
  }
  if (numVertices == 0)
  {
    Create_Dummy(m_Land_subMesh_low); return;
  }
  VertexData* vdata = new VertexData();

  IndexData* idata = m_Land_subMesh_low->indexData;
  vdata->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent
  VertexDeclaration* vdec = vdata->vertexDeclaration;
  size_t offset = 0;
  vdec->addElement( 0, offset, VET_FLOAT3, VES_POSITION );
  offset += VertexElement::getTypeSize(VET_FLOAT3);
  vdec->addElement( 0, offset, VET_FLOAT3, VES_NORMAL );
  offset += VertexElement::getTypeSize(VET_FLOAT3);
  vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
  offset += VertexElement::getTypeSize(VET_FLOAT2);
  vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1);  // Gittertextur
  offset += VertexElement::getTypeSize(VET_FLOAT2);

  HardwareVertexBufferSharedPtr m_vbuf0;
  m_vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
              offset, // size of one whole vertex
              numVertices, // number of vertices
              HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
              false // no shadow buffer
            );

  vdata->vertexBufferBinding->setBinding(0, m_vbuf0);

  Real* pReal1 = static_cast<Real*>(m_vbuf0->lock(HardwareBuffer::HBL_NORMAL));

  int o = 0;
  Real g,h,d,f;
  short row, col;
  for (int a = x; a < x+ CHUNK_SIZE_X; a += 2)
  {
    for (int b = z; b < z +CHUNK_SIZE_Z; b += 2)
    {
      if (m_TileManagerPtr->Get_Map_Height(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) > LEVEL_WATER_TOP ||
              m_TileManagerPtr->Get_Map_Height(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) > LEVEL_WATER_TOP)
      {
        g = m_TileManagerPtr->Get_Map_Height(a  ,b  ) * StretchZ;
        h = m_TileManagerPtr->Get_Map_Height(a+2,b  ) * StretchZ;
        d = m_TileManagerPtr->Get_Map_Height(a  ,b+2) * StretchZ;
        f = m_TileManagerPtr->Get_Map_Height(a+2,b+2) * StretchZ;
        /////////////////////////////////////////////////////////////////////////
        // Position.
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal1[o   ] = TILE_SIZE * (a-x);
        pReal1[o+ 1] = g;
        pReal1[o+ 2] = TILE_SIZE * (b-z);
        pReal1[o+10] = TILE_SIZE * (a-x+2);
        pReal1[o+11] = h;
        pReal1[o+12] = TILE_SIZE * (b-z);
        pReal1[o+20] = TILE_SIZE * (a-x);
        pReal1[o+21] = d;
        pReal1[o+22] = TILE_SIZE * (b-z+2);
        // 2. Triangle
        pReal1[o+30] = TILE_SIZE * (a-x);
        pReal1[o+31] = d;
        pReal1[o+32] = TILE_SIZE * (b-z+2);
        pReal1[o+40] = TILE_SIZE * (a-x+2);
        pReal1[o+41] = h;
        pReal1[o+42] = TILE_SIZE * (b-z);
        pReal1[o+50] = TILE_SIZE * (a-x+2);
        pReal1[o+51] = f;
        pReal1[o+52] = TILE_SIZE * (b-z+2);
        /////////////////////////////////////////////////////////////////////////
        // Normals.
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal1[o+ 3] = 0; pReal1[o+ 4] = 1; pReal1[o+ 5] = 0;
        pReal1[o+13] = 0; pReal1[o+14] = 1; pReal1[o+15] = 0;
        pReal1[o+23] = 0; pReal1[o+24] = 1; pReal1[o+25] = 0;
        // 2. Triangle
        pReal1[o+33] = 0; pReal1[o+34] = 1; pReal1[o+35] = 0;
        pReal1[o+43] = 0; pReal1[o+44] = 1; pReal1[o+45] = 0;
        pReal1[o+53] = 0; pReal1[o+54] = 1; pReal1[o+55] = 0;
        /////////////////////////////////////////////////////////////////////////
        // Textures.
        /////////////////////////////////////////////////////////////////////////
        col = m_TileManagerPtr->Get_Map_Texture_Col(a,b);
        row = m_TileManagerPtr->Get_Map_Texture_Row(a,b);
        // 1. triangle
        pReal1[o+ 6] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
        pReal1[o+ 7] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
        pReal1[o+16] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+17] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
        pReal1[o+26] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
        pReal1[o+27] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
        // 2. Triangle
        pReal1[o+36] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
        pReal1[o+37] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+46] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+47] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
        pReal1[o+56] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+57] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
        /////////////////////////////////////////////////////////////////////////
        // Grid.
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal1[o+ 8] = 0.0; pReal1[o+ 9] = 0.0;
        pReal1[o+18] = 1.0; pReal1[o+19] = 0.0;
        pReal1[o+28] = 0.0; pReal1[o+29] = 1.0;
        // 2. Triangle
        pReal1[o+38] = 0.0; pReal1[o+39] = 1.0;
        pReal1[o+48] = 1.0; pReal1[o+49] = 0.0;
        pReal1[o+58] = 1.0; pReal1[o+59] = 1.0;
        o += 60;
      }
    }
  }
  m_vbuf0->unlock();

  HardwareIndexBufferSharedPtr m_ibuf;
  m_ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
             HardwareIndexBuffer::IT_16BIT, // type of index
             numVertices, // number of indexes
             HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
             false // no shadow buffer
           );
  idata->indexBuffer = m_ibuf;
  idata->indexStart = 0;
  idata->indexCount = numVertices;

  unsigned short* pIdx = static_cast<unsigned short*>(m_ibuf->lock(HardwareBuffer::HBL_DISCARD));
  for (unsigned short p=0; numVertices; ++p) pIdx[--numVertices] = p;
  m_ibuf->unlock();

  m_Land_subMesh_low->operationType = RenderOperation::OT_TRIANGLE_LIST;
  m_Land_subMesh_low->useSharedVertices = false;
  m_Land_subMesh_low->vertexData = vdata;

  m_IsAttached = false;
  //m_Land->attachObject( m_Land_entity_low );
}

///=================================================================================================
/// Create Land in high Quality. 1 Tile = 4 triangles. We need this for the filter.
/// +------+
/// |\  4 /|
/// | \  / |
/// |  \/  |
/// |1 /\ 3|
/// | /  \ |
/// |/  2 \|
/// +------+
///=================================================================================================
void TileChunk::CreateLandHigh()
{
  int x = m_posX * CHUNK_SIZE_X;
  int z = m_posZ * CHUNK_SIZE_Z;
  /////////////////////////////////////////////////////////////////////////
  // Create Mesh with a SubMesh.
  /////////////////////////////////////////////////////////////////////////
  sprintf(MeshName, "Land[%d,%d] High", x, z);
  m_Land_Mesh_high = MeshManager::getSingleton().createManual(MeshName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  sprintf(TempName, "SubLand[%d,%d] High", x, z);
  m_Land_subMesh_high = m_Land_Mesh_high->createSubMesh(TempName);
  m_Land_subMesh_high->setMaterialName("Land_HighDetails128");

  CreateLandHigh_Buffers();

  m_Land_Mesh_high->_setBounds( *m_bounds ); // Rendering is only done when Camera looks into this quad.
  m_Land_Mesh_high->load();
  sprintf( TempName, "Land[%d,%d] High Entity", m_posX, m_posZ );
  m_Land_entity_high = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);
  // m_Land->attachObject( m_Land_entity_high );
  m_IsAttached = false;
}

///=================================================================================================
/// Change Land in high Quality
///=================================================================================================
void TileChunk::ChangeLandHigh()
{
  delete m_Land_subMesh_high->vertexData;
  CreateLandHigh_Buffers();
}

///=================================================================================================
/// Create Hardware Buffers for high quality land.
///=================================================================================================
void TileChunk::CreateLandHigh_Buffers()
{
  int x = m_posX * CHUNK_SIZE_X;
  int z = m_posZ * CHUNK_SIZE_Z;

  /////////////////////////////////////////////////////////////////////////
  /// Count the Vertices in this chunk.
  /////////////////////////////////////////////////////////////////////////
  unsigned int numVertices = 0;
  for (int a = x; a < x + CHUNK_SIZE_X; ++a)
  {
    for (int b = z; b < z + CHUNK_SIZE_Z; ++b)
    {
      if (m_TileManagerPtr->Get_Map_Height(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) > LEVEL_WATER_TOP ||
              m_TileManagerPtr->Get_Map_Height(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) > LEVEL_WATER_TOP)
      {
        numVertices += 12;
      }
    }
  }
  if (numVertices == 0)
  {
    Create_Dummy(m_Land_subMesh_high); return;
  }

  /////////////////////////////////////////////////////////////////////////
  /// Create VertexData.
  /////////////////////////////////////////////////////////////////////////
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

  long o = 0;
  Real g, h, d, f, row, col;
  Real average, StretchZ = m_TileManagerPtr->Get_StretchZ();
  Real* pReal1 = static_cast<Real*>(vbuf0->lock(HardwareBuffer::HBL_NORMAL));
  for (short a = x; a < x+ CHUNK_SIZE_X; ++a)
  {
    for (short b = z; b < z +CHUNK_SIZE_Z; ++b)
    {
      if (m_TileManagerPtr->Get_Map_Height(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) > LEVEL_WATER_TOP ||
              m_TileManagerPtr->Get_Map_Height(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) > LEVEL_WATER_TOP)
      {
        g = m_TileManagerPtr->Get_Map_Height(a  , b  ) * StretchZ;
        h = m_TileManagerPtr->Get_Map_Height(a+1, b  ) * StretchZ;
        d = m_TileManagerPtr->Get_Map_Height(a  , b+1) * StretchZ;
        f = m_TileManagerPtr->Get_Map_Height(a+1, b+1) * StretchZ;
        average = ( g + h + d + f ) / 4.0;
        /////////////////////////////////////////////////////////////////////////
        // Position.
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal1[o   ] = TILE_SIZE * (a-x);
        pReal1[o+ 1] = d;
        pReal1[o+ 2] = TILE_SIZE * (b-z+1);

        pReal1[o+12] = TILE_SIZE * (a-x );
        pReal1[o+13] = g;
        pReal1[o+14] = TILE_SIZE * (b-z );

        pReal1[o+24] = TILE_SIZE * (a-x+.5);
        pReal1[o+25] = average;
        pReal1[o+26] = TILE_SIZE * (b-z+.5);

        // 2. Triangle
        pReal1[o+36] = TILE_SIZE * (a-x);
        pReal1[o+37] = g;
        pReal1[o+38] = TILE_SIZE * (b-z);

        pReal1[o+48] = TILE_SIZE * (a-x+1);
        pReal1[o+49] = h;
        pReal1[o+50] = TILE_SIZE * (b-z);

        pReal1[o+60] = TILE_SIZE * (a-x +.5);
        pReal1[o+61] = average;
        pReal1[o+62] = TILE_SIZE * (b-z +.5);

        // 3. Triangle
        pReal1[o+72] = TILE_SIZE * (a-x+1);
        pReal1[o+73] = h;
        pReal1[o+74] = TILE_SIZE * (b-z);

        pReal1[o+84] = TILE_SIZE * (a-x +1);
        pReal1[o+85] = f;
        pReal1[o+86] = TILE_SIZE * (b-z +1);

        pReal1[o+96] = TILE_SIZE * (a-x+.5);
        pReal1[o+97] = average;
        pReal1[o+98] = TILE_SIZE * (b-z+.5);

        // 4. Triangle
        pReal1[o+108] = TILE_SIZE * (a-x +1);
        pReal1[o+109] = f;
        pReal1[o+110] = TILE_SIZE * (b-z +1);

        pReal1[o+120] = TILE_SIZE * (a-x   );
        pReal1[o+121] = d;
        pReal1[o+122] = TILE_SIZE * (b-z +1);

        pReal1[o+132] = TILE_SIZE * (a-x+.5);
        pReal1[o+133] = average;
        pReal1[o+134] = TILE_SIZE * (b-z+.5);

        /////////////////////////////////////////////////////////////////////////
        // Normalvektoren
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal1[o+  3] = 0.0; pReal1[o+  4] = 1.0; pReal1[o+  5] = 0.0;
        pReal1[o+ 15] = 0.0; pReal1[o+ 16] = 1.0; pReal1[o+ 17] = 0.0;
        pReal1[o+ 27] = 0.0; pReal1[o+ 28] = 1.0; pReal1[o+ 29] = 0.0;
        // 2. Triangle
        pReal1[o+ 39] = 0.0; pReal1[o+ 40] = 1.0; pReal1[o+ 41] = 0.0;
        pReal1[o+ 51] = 0.0; pReal1[o+ 52] = 1.0; pReal1[o+ 53] = 0.0;
        pReal1[o+ 63] = 0.0; pReal1[o+ 64] = 1.0; pReal1[o+ 65] = 0.0;
        // 3. Triangle
        pReal1[o+ 75] = 0.0; pReal1[o+ 76] = 1.0; pReal1[o+ 77] = 0.0;
        pReal1[o+ 87] = 0.0; pReal1[o+ 88] = 1.0; pReal1[o+ 89] = 0.0;
        pReal1[o+ 99] = 0.0; pReal1[o+100] = 1.0; pReal1[o+101] = 0.0;
        // 4. Triangle
        pReal1[o+111] = 0.0; pReal1[o+112] = 1.0; pReal1[o+113] = 0.0;
        pReal1[o+123] = 0.0; pReal1[o+124] = 1.0; pReal1[o+125] = 0.0;
        pReal1[o+135] = 0.0; pReal1[o+136] = 1.0; pReal1[o+137] = 0.0;
        /////////////////////////////////////////////////////////////////////////
        // Ground-Texture.
        /////////////////////////////////////////////////////////////////////////
        col = (1.0 / 8.0 + 2 * 1.0 / 128.0) * m_TileManagerPtr->Get_Map_Texture_Col(a,b)+ 1.0 / 128.0;
        row = (1.0 / 8.0 + 2 * 1.0 / 128.0) * m_TileManagerPtr->Get_Map_Texture_Row(a,b)+ 1.0 / 128.0;
        // 1. Triangle
        pReal1[o+  6] = col;
        pReal1[o+  7] = row;
        pReal1[o+ 18] = col;
        pReal1[o+ 19] = row + 1.0 / 8.0;
        pReal1[o+ 30] = col + 1.0 / 16.0;
        pReal1[o+ 31] = row + 1.0 / 16.0;
        // 2. Triangle
        pReal1[o+ 42] = col;
        pReal1[o+ 43] = row + 1.0 / 8.0;
        pReal1[o+ 54] = col + 1.0 / 8.0;
        pReal1[o+ 55] = row + 1.0 / 8.0;
        pReal1[o+ 66] = col + 1.0 / 16.0;
        pReal1[o+ 67] = row + 1.0 / 16.0;
        // 3. Triangle
        pReal1[o+ 78] = col + 1.0 / 8.0;
        pReal1[o+ 79] = row + 1.0 / 8.0;
        pReal1[o+ 90] = col + 1.0 / 8.0;
        pReal1[o+ 91] = row;
        pReal1[o+102] = col + 1.0 / 16.0;
        pReal1[o+103] = row + 1.0 / 16.0;
        // 4. Triangle
        pReal1[o+114] = col + 1.0 / 8.0;
        pReal1[o+115] = row;
        pReal1[o+126] = col;
        pReal1[o+127] = row;
        pReal1[o+138] = col + 1.0 / 16.0;
        pReal1[o+139] = row + 1.0 / 16.0;
        /////////////////////////////////////////////////////////////////////////
        // Filter-Texture.
        /////////////////////////////////////////////////////////////////////////

        Real temp = 1.0 / 8.0 + 2 * 1.0 / 128.0; // size of one tile texture plus two mipmapping spaces in the terrain texture

        if (a)
        {
          col = m_TileManagerPtr->Get_Map_Texture_Col(a-1,b);
          row = m_TileManagerPtr->Get_Map_Texture_Row(a-1,b);
        }
        else
        {
          col = 0;
          row = 0;
        }
        // 1. Triangle
        pReal1[o+ 8] = temp * col + 1.0 / 128.0;
        pReal1[o+ 9] = temp * row + 1.0 / 128.0;
        pReal1[o+20] = temp * col + 1.0 / 128.0;
        pReal1[o+21] = temp * row + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+32] = temp * col + 1.0 / 128.0 + 1.0 / 16.0;
        pReal1[o+33] = temp * row + 1.0 / 128.0 + 1.0 / 16.0;
        // 2. Triangle
        if (b)
        {
          col = m_TileManagerPtr->Get_Map_Texture_Col(a,b-1);
          row = m_TileManagerPtr->Get_Map_Texture_Row(a,b-1);
        }
        else
        {
          col = 0;
          row = 0;
        }
        pReal1[o+44] = temp * col + 1.0 / 128.0;
        pReal1[o+45] = temp * row + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+56] = temp * col + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+57] = temp * row + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+68] = temp * col + 1.0 / 128.0 + 1.0 / 16.0;
        pReal1[o+69] = temp * row + 1.0 / 128.0 + 1.0 / 16.0;
        // 3. Triangle
        if ( a != TILES_SUM_X - 1)
        {
          col = m_TileManagerPtr->Get_Map_Texture_Col(a+1,b);
          row = m_TileManagerPtr->Get_Map_Texture_Row(a+1,b);
        }
        else
        {
          col = 0;
          row = 0;
        }
        pReal1[o+80] = temp * col + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+81] = temp * row + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+92] = temp * col + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+93] = temp * row + 1.0 / 128.0;
        pReal1[o+104] = temp * col + 1.0 / 128.0 + 1.0 / 16.0;
        pReal1[o+105] = temp * row + 1.0 / 128.0 + 1.0 / 16.0;
        // 4. Triangle
        if ( b != TILES_SUM_Z -1)
        {
          col = m_TileManagerPtr->Get_Map_Texture_Col(a,b+1);
          row = m_TileManagerPtr->Get_Map_Texture_Row(a,b+1);
        }
        else
        {
          col = 0;
          row = 0;
        }
        pReal1[o+116] = temp * col + 1.0 / 128.0 + 1.0 / 8.0;
        pReal1[o+117] = temp * row + 1.0 / 128.0;
        pReal1[o+128] = temp * col + 1.0 / 128.0;
        pReal1[o+129] = temp * row + 1.0 / 128.0;
        pReal1[o+140] = temp * col + 1.0 / 128.0 + 1.0 / 16.0;
        pReal1[o+141] = temp * row + 1.0 / 128.0 + 1.0 / 16.0;

#ifdef USE_LIGHTMAP
        /////////////////////////////////////////////////////////////////////////
        // Lightmap-Texture
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal1[o+ 10] = a / 256.0; pReal1[o+ 11] = (b + 1) / 256.0;
        pReal1[o+ 22] = a / 256.0; pReal1[o+ 23] = b / 256.0;
        pReal1[o+ 34] = (a + .5) / 256.0; pReal1[o+ 35] = (b +.5) / 256.0;
        // 2. Triangle
        pReal1[o+ 46] = a / 256.0; pReal1[o+ 47] = b / 256.0;
        pReal1[o+ 58] = (a + 1) / 256.0; pReal1[o+ 59] = b / 256.0;
        pReal1[o+ 70] = (a + .5) / 256.0; pReal1[o+ 71] = (b +.5) / 256.0;
        // 3. Triangle
        pReal1[o+ 82] = (a + 1) / 256.0; pReal1[o+ 83] = b / 256.0;
        pReal1[o+ 94] = (a + 1)/ 256.0; pReal1[o+ 95] = (b + 1) / 256.0;
        pReal1[o+106] = (a + .5) / 256.0; pReal1[o+107] = (b +.5) / 256.0;
        // 4. Triangle
        pReal1[o+118] = (a + 1)/ 256.0; pReal1[o+119] = (b + 1) / 256.0;
        pReal1[o+130] = a / 256.0; pReal1[o+131] = (b + 1) / 256.0;
        pReal1[o+142] = (a + .5) / 256.0; pReal1[o+143] = (b +.5) / 256.0;
#else
        /////////////////////////////////////////////////////////////////////////
        // Grid-Texture.
        /////////////////////////////////////////////////////////////////////////
        // 1. Triangle
        pReal1[o+ 10] = 0.0; pReal1[o+ 11] = 1.0; pReal1[o+ 22] = 0.0;
        pReal1[o+ 23] = 1.0; pReal1[o+ 34] = 0.5; pReal1[o+ 35] = 0.5;
        // 2. Triangle
        pReal1[o+ 46] = 0.0; pReal1[o+ 47] = 1.0; pReal1[o+ 58] = 1.0;
        pReal1[o+ 59] = 1.0; pReal1[o+ 70] = 0.5; pReal1[o+ 71] = 0.5;
        // 3. Triangle
        pReal1[o+ 82] = 1.0; pReal1[o+ 83] = 1.0; pReal1[o+ 94] = 1.0;
        pReal1[o+ 95] = 0.0; pReal1[o+106] = 0.5; pReal1[o+107] = 0.5;
        // 4. Triangle
        pReal1[o+118] = 1.0; pReal1[o+119] = 0.0; pReal1[o+130] = 0.0;
        pReal1[o+131] = 0.0; pReal1[o+142] = 0.5; pReal1[o+143] = 0.5;
#endif
        o += 144;
      }
    }
  }

  vbuf0->unlock();
  /////////////////////////////////////////////////////////////////////////
  // Create Index-buffer
  /////////////////////////////////////////////////////////////////////////
  HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
                                        HardwareIndexBuffer::IT_16BIT, // type of index
                                        numVertices, // number of indexes
                                        HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                                        false); // no shadow buffer
  IndexData* idata = m_Land_subMesh_high->indexData;
  idata->indexBuffer = ibuf;
  idata->indexStart = 0;
  idata->indexCount = numVertices;
  unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
  for (unsigned short p=0; numVertices; ++p) pIdx[--numVertices] = p;
  ibuf->unlock();

  m_Land_subMesh_high->operationType = RenderOperation::OT_TRIANGLE_LIST;
  m_Land_subMesh_high->useSharedVertices = false;
  m_Land_subMesh_high->vertexData = vdata;
}

///=================================================================================================
/// Creates a dummy submesh containing only 1 Triangle.
///=================================================================================================
void TileChunk::Create_Dummy(SubMesh* submesh)
{
  const int numVertices = 3;
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
  HardwareVertexBufferSharedPtr vbuf0;
  vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
            offset, // size of one whole vertex
            numVertices, // number of vertices
            HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
            false); // no shadow buffer
  vdata->vertexBufferBinding->setBinding(0, vbuf0);
  Real* pReal = static_cast<Real*>(vbuf0->lock(HardwareBuffer::HBL_NORMAL));
  // Triangle 1
  pReal[ 0] = 0; pReal[ 1] =-100; pReal[ 2] = 0;
  pReal[ 3] = 0; pReal[ 4] =   0; pReal[ 5] = 0;
  pReal[ 6] = 0; pReal[ 7] =   0;
  pReal[ 8] = 5; pReal[ 9] =-100; pReal[10] = 0;
  pReal[11] = 0; pReal[12] =   0; pReal[13] = 0;
  pReal[14] = 0; pReal[15] =   0;
  pReal[16] = 5; pReal[17] =-100; pReal[18] =0;
  pReal[19] = 0; pReal[20] =   0; pReal[21] = 0;
  pReal[22] = 0; pReal[23] =   0;
  vbuf0->unlock();

  HardwareIndexBufferSharedPtr ibuf;
  ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
           HardwareIndexBuffer::IT_16BIT, // type of index
           numVertices, // number of indexes
           HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
           false); // no shadow buffer
  IndexData* idata = submesh->indexData;
  idata->indexBuffer = ibuf;
  idata->indexStart = 0;
  idata->indexCount = numVertices;
  unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
  pIdx[0] = 2; pIdx[1] = 1; pIdx[2] = 0;
  ibuf->unlock();

  submesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
  submesh->useSharedVertices = false;
  submesh->vertexData = vdata;
}

