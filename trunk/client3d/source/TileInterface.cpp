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

#include "TileInterface.h"
#include "TileManager.h"


TileSelection::TileSelection(TileManager* TileManager)
{
  m_TileManager = TileManager;
  m_vdata = NULL;
  m_vdataCorner1 = NULL;
  m_vdataCorner2 = NULL;
  m_vdataCorner3 = NULL;
  m_vdataCorner4 = NULL;
  m_SquareSize = 1;
  // reset();
}

TileSelection::~TileSelection()
{

  m_vdata = NULL;
  m_vdataCorner1 = NULL;
  m_vdataCorner2 = NULL;
  m_vdataCorner3 = NULL;
  m_vdataCorner4 = NULL;

  if (m_vdata) delete m_vdata;
  if (m_vdataCorner1) delete m_vdataCorner1;
  if (m_vdataCorner2) delete m_vdataCorner2;
  if (m_vdataCorner3) delete m_vdataCorner3;
  if (m_vdataCorner4) delete m_vdataCorner4;
}
Vector3 TileSelection::get_Selection()
{
  Vector3 tmp;
  tmp.x = m_x;
  tmp.y = (Real) (m_TileManager->Get_Map_Height(m_x, m_y));
  tmp.z = m_y;
  return tmp;
}

void TileSelection::save_Selection()
{
  m_x_old = m_x;
  m_y_old = m_y;
}
void TileSelection::reset()
{
  m_distance = 1000000; // something big
  m_x = -1;
  m_y = -1;
}

void TileSelection::select()
{
  static int counter = 0;
  ++counter;
  if (counter == 1) create_Entity();

  change_Selection();

  if (m_x != m_x_old && m_y != m_y_old)
  {
    // note: insert actions here

    /*char name[50];
    sprintf( name, "Light %d", counter);
    Light* light = m_TileManager->Get_pSceneManager()->createLight( name );
    light->setType( Light::LT_SPOTLIGHT );
    light->setPosition( Vector3
     ((m_x + .5) * TILE_SIZE, this->m_TileManager->Get_Map_Height(m_x, m_y) + 150 , (m_y+.5) * TILE_SIZE) );
    light->setDiffuseColour( .8, .8, 1.0 );
    light->setDirection(0,-1,0);
    light->setSpecularColour( 0.0, 1.0, 0.0 );
    light->setSpotlightRange( Degree(45), Degree(120) );
    */
  }
}

void TileSelection::create_Entity()
{
  unsigned long numVertices = 6;
  float StretchZ = m_TileManager->Get_StretchZ();

  //Selected Tile Marker
  if (m_vdata) delete m_vdata;
  m_vdata = new VertexData();
  IndexData* m_idata;

  char name[50];
  char name2[50];

  sprintf( name, "Selection");
  m_Mesh = MeshManager::getSingleton().createManual( name,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  sprintf( name2, "SubSelection");
  m_SubMesh= m_Mesh->createSubMesh(name2);
  m_idata = m_SubMesh->indexData;

  m_vdata->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent

  VertexDeclaration* vdec = m_vdata->vertexDeclaration;

  size_t offset = 0;
  vdec->addElement( 0, offset, VET_FLOAT3, VES_POSITION );
  offset += VertexElement::getTypeSize(VET_FLOAT3);
  vdec->addElement( 0, offset, VET_FLOAT3, VES_NORMAL );
  offset += VertexElement::getTypeSize(VET_FLOAT3);
  vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
  offset += VertexElement::getTypeSize(VET_FLOAT2);

  m_vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
              offset, // size of one whole vertex
              numVertices, // number of vertices
              HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
              false); // no shadow buffer

  VertexBufferBinding* vbbind = m_vdata->vertexBufferBinding;
  vbbind->setBinding(0, m_vbuf0);

  Real* pReal;
  pReal = static_cast<Real*>(m_vbuf0->lock(HardwareBuffer::HBL_NORMAL));

  Real height[4];

  height[0] = m_TileManager->Get_Map_Height(m_x    , m_y    ) * StretchZ;
  height[1] = m_TileManager->Get_Map_Height(m_x + 1, m_y    ) * StretchZ;
  height[2] = m_TileManager->Get_Map_Height(m_x    , m_y + 1) * StretchZ;
  height[3] = m_TileManager->Get_Map_Height(m_x + 1, m_y + 1) * StretchZ;

  for (int a = 0; a != 4; ++a)
  {
    if (height[a] < LEVEL_WATER_CLP * StretchZ)
      height[a] = LEVEL_WATER_CLP * StretchZ;
  }

  // 1st triangle
  pReal[ 0] =   0; pReal[ 1] =   height[0] ; pReal[2] = 0;
  pReal[ 3] =   0; pReal[ 4] =   0; pReal[5] = 1;
  pReal[ 6] =   0; pReal[ 7] =   0;

  pReal[ 8] =   0; pReal[ 9] =   height[2] ; pReal[10] = TILE_SIZE;
  pReal[11] =   0; pReal[12] =   0; pReal[13] = 1;
  pReal[14] =   0; pReal[15] =   1;

  pReal[16] =   TILE_SIZE; pReal[17] =  height[1] ; pReal[18] = 0;
  pReal[19] =   0; pReal[20] =   0; pReal[21] = 1;
  pReal[22] =   1; pReal[23] =   0;

  // 2nd triangle
  pReal[24] =   TILE_SIZE; pReal[25] =   height[1] ; pReal[26] = 0;
  pReal[27] =   0; pReal[28] =   0; pReal[29] = 1;
  pReal[30] =   1; pReal[31] =   0;

  pReal[32] =   TILE_SIZE; pReal[33] =  height[2] ; pReal[34] = TILE_SIZE;
  pReal[35] =   0; pReal[36] =   0; pReal[37] = 1;
  pReal[38] =   0; pReal[39] =   1;

  pReal[40] =   TILE_SIZE; pReal[41] =   height[3] ; pReal[42] = TILE_SIZE;
  pReal[43] =   0; pReal[44] =   0; pReal[45] = 1;
  pReal[46] =   1; pReal[47] =   1;

  m_vbuf0->unlock();

  HardwareIndexBufferSharedPtr m_ibuf = HardwareBufferManager::getSingleton().
                                        createIndexBuffer(
                                          HardwareIndexBuffer::IT_16BIT, // type of index
                                          numVertices, // number of indexes
                                          HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                                          false); // no shadow buffer

  m_idata->indexBuffer = m_ibuf;
  m_idata->indexStart = 0;
  m_idata->indexCount = numVertices;

  unsigned short* pIdx = static_cast<unsigned short*>(m_ibuf->lock(HardwareBuffer::HBL_DISCARD));

  for (unsigned short a = 0; a < (unsigned short)numVertices; a++)
    pIdx[a] = a;

  m_ibuf->unlock();

  m_SubMesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
  m_SubMesh->useSharedVertices = false;
  m_SubMesh->vertexData = m_vdata;

  // Setzen des Sichtbarkeits-Quaders. Fällt dieser Quader außerhalb des Sichtbereits der
  // Kamera, so wird das Kartenstück nicht gerendert.

  AxisAlignedBox* bounds = new AxisAlignedBox(
                             0, 0 , 0,
                             TILE_SIZE, 255 * StretchZ , TILE_SIZE);
  m_Mesh->_setBounds( *bounds );

  delete bounds;

  m_SubMesh->setMaterialName("Selection");
  sprintf( name2, "Selection Entity");
  m_Entity = m_TileManager->Get_pSceneManager()->createEntity(name2, name);

  m_SceneNode = m_TileManager->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
  m_SceneNode->attachObject( m_Entity );
  m_SceneNode->setPosition(m_x * TILE_SIZE,0,m_y * TILE_SIZE);
  m_Entity->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);

  //CORNER 1
  if (m_vdataCorner1) delete m_vdataCorner1;
  m_vdataCorner1 = new VertexData();
  IndexData* m_idataCorner1;

  char nameCorner1[50];
  char name2Corner1[50];

  sprintf( nameCorner1, "Selection Corner1");
  m_MeshCorner1 = MeshManager::getSingleton().createManual( nameCorner1,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  sprintf( name2Corner1, "SubSelection Corner1");
  m_SubMeshCorner1= m_MeshCorner1->createSubMesh(name2Corner1);
  m_idataCorner1 = m_SubMeshCorner1->indexData;

  m_vdataCorner1->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent

  VertexDeclaration* vdecCorner1 = m_vdataCorner1->vertexDeclaration;

  size_t offsetCorner1 = 0;
  vdecCorner1->addElement( 0, offsetCorner1, VET_FLOAT3, VES_POSITION );
  offsetCorner1 += VertexElement::getTypeSize(VET_FLOAT3);
  vdecCorner1->addElement( 0, offsetCorner1, VET_FLOAT3, VES_NORMAL );
  offsetCorner1 += VertexElement::getTypeSize(VET_FLOAT3);
  vdecCorner1->addElement( 0, offsetCorner1, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
  offsetCorner1 += VertexElement::getTypeSize(VET_FLOAT2);

  m_vbuf0Corner1 = HardwareBufferManager::getSingleton().createVertexBuffer(
                     offsetCorner1, // size of one whole vertex
                     numVertices, // number of vertices
                     HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                     false); // no shadow buffer

  VertexBufferBinding* vbbindCorner1 = m_vdataCorner1->vertexBufferBinding;
  vbbindCorner1->setBinding(0, m_vbuf0Corner1);

  Real* pRealCorner1;
  pRealCorner1 = static_cast<Real*>(m_vbuf0Corner1->lock(HardwareBuffer::HBL_NORMAL));

  Real heightCorner1[4];

  heightCorner1[0] = m_TileManager->Get_Map_Height(m_x    , m_y    ) * StretchZ;
  heightCorner1[1] = m_TileManager->Get_Map_Height(m_x + 1, m_y    ) * StretchZ;
  heightCorner1[2] = m_TileManager->Get_Map_Height(m_x    , m_y + 1) * StretchZ;
  heightCorner1[3] = m_TileManager->Get_Map_Height(m_x + 1, m_y + 1) * StretchZ;

  for (int a = 0; a != 4; ++a)
  {
    if (heightCorner1[a] < LEVEL_WATER_CLP * StretchZ)
      heightCorner1[a] = LEVEL_WATER_CLP * StretchZ;
  }

  // 1st triangle
  pRealCorner1[ 0] =   0; pRealCorner1[ 1] =   heightCorner1[0] ; pRealCorner1[2] = 0;
  pRealCorner1[ 3] =   0; pRealCorner1[ 4] =   0; pRealCorner1[5] = 1;
  pRealCorner1[ 6] =   0; pRealCorner1[ 7] =   0;

  pRealCorner1[ 8] =   0; pRealCorner1[ 9] =   heightCorner1[2] ; pRealCorner1[10] = TILE_SIZE;
  pRealCorner1[11] =   0; pRealCorner1[12] =   0; pRealCorner1[13] = 1;
  pRealCorner1[14] =   0; pRealCorner1[15] =   1;

  pRealCorner1[16] =   TILE_SIZE; pRealCorner1[17] =  heightCorner1[1] ; pRealCorner1[18] = 0;
  pRealCorner1[19] =   0; pRealCorner1[20] =   0; pRealCorner1[21] = 1;
  pRealCorner1[22] =   1; pRealCorner1[23] =   0;

  // 2nd triangle
  pRealCorner1[24] =   TILE_SIZE; pRealCorner1[25] =   heightCorner1[1] ; pRealCorner1[26] = 0;
  pRealCorner1[27] =   0; pRealCorner1[28] =   0; pRealCorner1[29] = 1;
  pRealCorner1[30] =   1; pRealCorner1[31] =   0;

  pRealCorner1[32] =   TILE_SIZE; pRealCorner1[33] =  heightCorner1[2] ; pRealCorner1[34] = TILE_SIZE;
  pRealCorner1[35] =   0; pRealCorner1[36] =   0; pRealCorner1[37] = 1;
  pRealCorner1[38] =   0; pRealCorner1[39] =   1;

  pRealCorner1[40] =   TILE_SIZE; pRealCorner1[41] =   heightCorner1[3] ; pRealCorner1[42] = TILE_SIZE;
  pRealCorner1[43] =   0; pRealCorner1[44] =   0; pRealCorner1[45] = 1;
  pRealCorner1[46] =   1; pRealCorner1[47] =   1;

  m_vbuf0Corner1->unlock();

  HardwareIndexBufferSharedPtr m_ibufCorner1 = HardwareBufferManager::getSingleton().
      createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, // type of index
        numVertices, // number of indexes
        HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
        false); // no shadow buffer

  m_idataCorner1->indexBuffer = m_ibufCorner1;
  m_idataCorner1->indexStart = 0;
  m_idataCorner1->indexCount = numVertices;

  unsigned short* pIdxCorner1 = static_cast<unsigned short*>(m_ibufCorner1->lock(HardwareBuffer::HBL_DISCARD));

  for (unsigned short a = 0; a < (unsigned short)numVertices; a++)
    pIdxCorner1[a] = a;

  m_ibufCorner1->unlock();

  m_SubMeshCorner1->operationType = RenderOperation::OT_TRIANGLE_LIST;
  m_SubMeshCorner1->useSharedVertices = false;
  m_SubMeshCorner1->vertexData = m_vdataCorner1;

  AxisAlignedBox* boundsCorner1 = new AxisAlignedBox(
                                    0, 0 , 0,
                                    TILE_SIZE, 255 * StretchZ , TILE_SIZE);
  m_MeshCorner1->_setBounds( *boundsCorner1 );

  delete boundsCorner1;

  m_SubMeshCorner1->setMaterialName("Selection");
  sprintf( name2Corner1, "Selection Entity Corner1");
  m_EntityCorner1 = m_TileManager->Get_pSceneManager()->createEntity(name2Corner1, nameCorner1);

  m_SceneNodeCorner1 = m_TileManager->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
  m_SceneNodeCorner1->attachObject( m_EntityCorner1 );
  m_SceneNodeCorner1->setPosition(m_x * TILE_SIZE,0,m_y * TILE_SIZE);
  m_EntityCorner1->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);

  //CORNER 2
  if (m_vdataCorner2) delete m_vdataCorner2;
  m_vdataCorner2 = new VertexData();
  IndexData* m_idataCorner2;

  char nameCorner2[50];
  char name2Corner2[50];

  sprintf( nameCorner2, "Selection Corner2");
  m_MeshCorner2 = MeshManager::getSingleton().createManual( nameCorner2,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  sprintf( name2Corner2, "SubSelection Corner2");
  m_SubMeshCorner2= m_MeshCorner2->createSubMesh(name2Corner2);
  m_idataCorner2 = m_SubMeshCorner2->indexData;

  m_vdataCorner2->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent

  VertexDeclaration* vdecCorner2 = m_vdataCorner2->vertexDeclaration;

  size_t offsetCorner2 = 0;
  vdecCorner2->addElement( 0, offsetCorner2, VET_FLOAT3, VES_POSITION );
  offsetCorner2 += VertexElement::getTypeSize(VET_FLOAT3);
  vdecCorner2->addElement( 0, offsetCorner2, VET_FLOAT3, VES_NORMAL );
  offsetCorner2 += VertexElement::getTypeSize(VET_FLOAT3);
  vdecCorner2->addElement( 0, offsetCorner2, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
  offsetCorner2 += VertexElement::getTypeSize(VET_FLOAT2);

  m_vbuf0Corner2 = HardwareBufferManager::getSingleton().createVertexBuffer(
                     offsetCorner2, // size of one whole vertex
                     numVertices, // number of vertices
                     HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                     false); // no shadow buffer

  VertexBufferBinding* vbbindCorner2 = m_vdataCorner2->vertexBufferBinding;
  vbbindCorner2->setBinding(0, m_vbuf0Corner2);

  Real* pRealCorner2;
  pRealCorner2 = static_cast<Real*>(m_vbuf0Corner2->lock(HardwareBuffer::HBL_NORMAL));

  Real heightCorner2[4];

  heightCorner2[0] = m_TileManager->Get_Map_Height(m_x    , m_y    ) * StretchZ;
  heightCorner2[1] = m_TileManager->Get_Map_Height(m_x + 1, m_y    ) * StretchZ;
  heightCorner2[2] = m_TileManager->Get_Map_Height(m_x    , m_y + 1) * StretchZ;
  heightCorner2[3] = m_TileManager->Get_Map_Height(m_x + 1, m_y + 1) * StretchZ;

  for (int a = 0; a != 4; ++a)
  {
    if (heightCorner2[a] < LEVEL_WATER_CLP * StretchZ)
      heightCorner2[a] = LEVEL_WATER_CLP * StretchZ;
  }

  // 1st triangle
  pRealCorner2[ 0] =   0; pRealCorner2[ 1] =   heightCorner2[0] ; pRealCorner2[2] = 0;
  pRealCorner2[ 3] =   0; pRealCorner2[ 4] =   0; pRealCorner2[5] = 1;
  pRealCorner2[ 6] =   0; pRealCorner2[ 7] =   0;

  pRealCorner2[ 8] =   0; pRealCorner2[ 9] =   heightCorner2[2] ; pRealCorner2[10] = TILE_SIZE;
  pRealCorner2[11] =   0; pRealCorner2[12] =   0; pRealCorner2[13] = 1;
  pRealCorner2[14] =   0; pRealCorner2[15] =   1;

  pRealCorner2[16] =   TILE_SIZE; pRealCorner2[17] =  heightCorner2[1] ; pRealCorner2[18] = 0;
  pRealCorner2[19] =   0; pRealCorner2[20] =   0; pRealCorner2[21] = 1;
  pRealCorner2[22] =   1; pRealCorner2[23] =   0;

  // 2nd triangle
  pRealCorner2[24] =   TILE_SIZE; pRealCorner2[25] =   heightCorner2[1] ; pRealCorner2[26] = 0;
  pRealCorner2[27] =   0; pRealCorner2[28] =   0; pRealCorner2[29] = 1;
  pRealCorner2[30] =   1; pRealCorner2[31] =   0;

  pRealCorner2[32] =   TILE_SIZE; pRealCorner2[33] =  heightCorner2[2] ; pRealCorner2[34] = TILE_SIZE;
  pRealCorner2[35] =   0; pRealCorner2[36] =   0; pRealCorner2[37] = 1;
  pRealCorner2[38] =   0; pRealCorner2[39] =   1;

  pRealCorner2[40] =   TILE_SIZE; pRealCorner2[41] =   heightCorner2[3] ; pRealCorner2[42] = TILE_SIZE;
  pRealCorner2[43] =   0; pRealCorner2[44] =   0; pRealCorner2[45] = 1;
  pRealCorner2[46] =   1; pRealCorner2[47] =   1;

  m_vbuf0Corner2->unlock();

  HardwareIndexBufferSharedPtr m_ibufCorner2 = HardwareBufferManager::getSingleton().
      createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, // type of index
        numVertices, // number of indexes
        HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
        false); // no shadow buffer

  m_idataCorner2->indexBuffer = m_ibufCorner2;
  m_idataCorner2->indexStart = 0;
  m_idataCorner2->indexCount = numVertices;

  unsigned short* pIdxCorner2 = static_cast<unsigned short*>(m_ibufCorner2->lock(HardwareBuffer::HBL_DISCARD));

  for (unsigned short a = 0; a < (unsigned short)numVertices; a++)
    pIdxCorner2[a] = a;

  m_ibufCorner2->unlock();

  m_SubMeshCorner2->operationType = RenderOperation::OT_TRIANGLE_LIST;
  m_SubMeshCorner2->useSharedVertices = false;
  m_SubMeshCorner2->vertexData = m_vdataCorner2;

  AxisAlignedBox* boundsCorner2 = new AxisAlignedBox(
                                    0, 0 , 0,
                                    TILE_SIZE, 255 * StretchZ , TILE_SIZE);
  m_MeshCorner2->_setBounds( *boundsCorner2 );

  delete boundsCorner2;

  m_SubMeshCorner2->setMaterialName("Selection");
  sprintf( name2Corner2, "Selection Entity Corner2");
  m_EntityCorner2 = m_TileManager->Get_pSceneManager()->createEntity(name2Corner2, nameCorner2);

  m_SceneNodeCorner2 = m_TileManager->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
  m_SceneNodeCorner2->attachObject( m_EntityCorner2 );
  m_SceneNodeCorner2->setPosition(m_x * TILE_SIZE,0,m_y * TILE_SIZE);
  m_EntityCorner2->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);

  //CORNER 3
  if (m_vdataCorner3) delete m_vdataCorner3;
  m_vdataCorner3 = new VertexData();
  IndexData* m_idataCorner3;

  char nameCorner3[50];
  char name2Corner3[50];

  sprintf( nameCorner3, "Selection Corner3");
  m_MeshCorner3 = MeshManager::getSingleton().createManual( nameCorner3,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  sprintf( name2Corner3, "SubSelection Corner3");
  m_SubMeshCorner3= m_MeshCorner3->createSubMesh(name2Corner3);
  m_idataCorner3 = m_SubMeshCorner3->indexData;

  m_vdataCorner3->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent

  VertexDeclaration* vdecCorner3 = m_vdataCorner3->vertexDeclaration;

  size_t offsetCorner3 = 0;
  vdecCorner3->addElement( 0, offsetCorner3, VET_FLOAT3, VES_POSITION );
  offsetCorner3 += VertexElement::getTypeSize(VET_FLOAT3);
  vdecCorner3->addElement( 0, offsetCorner3, VET_FLOAT3, VES_NORMAL );
  offsetCorner3 += VertexElement::getTypeSize(VET_FLOAT3);
  vdecCorner3->addElement( 0, offsetCorner3, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
  offsetCorner3 += VertexElement::getTypeSize(VET_FLOAT2);

  m_vbuf0Corner3 = HardwareBufferManager::getSingleton().createVertexBuffer(
                     offsetCorner3, // size of one whole vertex
                     numVertices, // number of vertices
                     HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                     false); // no shadow buffer

  VertexBufferBinding* vbbindCorner3 = m_vdataCorner3->vertexBufferBinding;
  vbbindCorner3->setBinding(0, m_vbuf0Corner3);

  Real* pRealCorner3;
  pRealCorner3 = static_cast<Real*>(m_vbuf0Corner3->lock(HardwareBuffer::HBL_NORMAL));

  Real heightCorner3[4];

  heightCorner3[0] = m_TileManager->Get_Map_Height(m_x    , m_y    ) * StretchZ;
  heightCorner3[1] = m_TileManager->Get_Map_Height(m_x + 1, m_y    ) * StretchZ;
  heightCorner3[2] = m_TileManager->Get_Map_Height(m_x    , m_y + 1) * StretchZ;
  heightCorner3[3] = m_TileManager->Get_Map_Height(m_x + 1, m_y + 1) * StretchZ;

  for (int a = 0; a != 4; ++a)
  {
    if (heightCorner3[a] < LEVEL_WATER_CLP * StretchZ)
      heightCorner3[a] = LEVEL_WATER_CLP * StretchZ;
  }

  // 1st triangle
  pRealCorner3[ 0] =   0; pRealCorner3[ 1] =   heightCorner3[0] ; pRealCorner3[2] = 0;
  pRealCorner3[ 3] =   0; pRealCorner3[ 4] =   0; pRealCorner3[5] = 1;
  pRealCorner3[ 6] =   0; pRealCorner3[ 7] =   0;

  pRealCorner3[ 8] =   0; pRealCorner3[ 9] =   heightCorner3[2] ; pRealCorner3[10] = TILE_SIZE;
  pRealCorner3[11] =   0; pRealCorner3[12] =   0; pRealCorner3[13] = 1;
  pRealCorner3[14] =   0; pRealCorner3[15] =   1;

  pRealCorner3[16] =   TILE_SIZE; pRealCorner3[17] =  heightCorner3[1] ; pRealCorner3[18] = 0;
  pRealCorner3[19] =   0; pRealCorner3[20] =   0; pRealCorner3[21] = 1;
  pRealCorner3[22] =   1; pRealCorner3[23] =   0;

  // 2nd triangle
  pRealCorner3[24] =   TILE_SIZE; pRealCorner3[25] =   heightCorner3[1] ; pRealCorner3[26] = 0;
  pRealCorner3[27] =   0; pRealCorner3[28] =   0; pRealCorner3[29] = 1;
  pRealCorner3[30] =   1; pRealCorner3[31] =   0;

  pRealCorner3[32] =   TILE_SIZE; pRealCorner3[33] =  heightCorner3[2] ; pRealCorner3[34] = TILE_SIZE;
  pRealCorner3[35] =   0; pRealCorner3[36] =   0; pRealCorner3[37] = 1;
  pRealCorner3[38] =   0; pRealCorner3[39] =   1;

  pRealCorner3[40] =   TILE_SIZE; pRealCorner3[41] =   heightCorner3[3] ; pRealCorner3[42] = TILE_SIZE;
  pRealCorner3[43] =   0; pRealCorner3[44] =   0; pRealCorner3[45] = 1;
  pRealCorner3[46] =   1; pRealCorner3[47] =   1;

  m_vbuf0Corner3->unlock();

  HardwareIndexBufferSharedPtr m_ibufCorner3 = HardwareBufferManager::getSingleton().
      createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, // type of index
        numVertices, // number of indexes
        HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
        false); // no shadow buffer

  m_idataCorner3->indexBuffer = m_ibufCorner3;
  m_idataCorner3->indexStart = 0;
  m_idataCorner3->indexCount = numVertices;

  unsigned short* pIdxCorner3 = static_cast<unsigned short*>(m_ibufCorner3->lock(HardwareBuffer::HBL_DISCARD));

  for (unsigned short a = 0; a < (unsigned short)numVertices; a++)
    pIdxCorner3[a] = a;

  m_ibufCorner3->unlock();

  m_SubMeshCorner3->operationType = RenderOperation::OT_TRIANGLE_LIST;
  m_SubMeshCorner3->useSharedVertices = false;
  m_SubMeshCorner3->vertexData = m_vdataCorner3;

  AxisAlignedBox* boundsCorner3 = new AxisAlignedBox(
                                    0, 0 , 0,
                                    TILE_SIZE, 255 * StretchZ , TILE_SIZE);
  m_MeshCorner3->_setBounds( *boundsCorner3 );

  delete boundsCorner3;

  m_SubMeshCorner3->setMaterialName("Selection");
  sprintf( name2Corner3, "Selection Entity Corner3");
  m_EntityCorner3 = m_TileManager->Get_pSceneManager()->createEntity(name2Corner3, nameCorner3);

  m_SceneNodeCorner3 = m_TileManager->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
  m_SceneNodeCorner3->attachObject( m_EntityCorner3 );
  m_SceneNodeCorner3->setPosition(m_x * TILE_SIZE,0,m_y * TILE_SIZE);
  m_EntityCorner3->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);

  //CORNER 4
  if (m_vdataCorner4) delete m_vdataCorner4;
  m_vdataCorner4 = new VertexData();
  IndexData* m_idataCorner4;

  char nameCorner4[50];
  char name2Corner4[50];

  sprintf( nameCorner4, "Selection Corner4");
  m_MeshCorner4 = MeshManager::getSingleton().createManual( nameCorner4,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  sprintf( name2Corner4, "SubSelection Corner4");
  m_SubMeshCorner4= m_MeshCorner4->createSubMesh(name2Corner4);
  m_idataCorner4 = m_SubMeshCorner4->indexData;

  m_vdataCorner4->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent

  VertexDeclaration* vdecCorner4 = m_vdataCorner4->vertexDeclaration;

  size_t offsetCorner4 = 0;
  vdecCorner4->addElement( 0, offsetCorner4, VET_FLOAT3, VES_POSITION );
  offsetCorner4 += VertexElement::getTypeSize(VET_FLOAT3);
  vdecCorner4->addElement( 0, offsetCorner4, VET_FLOAT3, VES_NORMAL );
  offsetCorner4 += VertexElement::getTypeSize(VET_FLOAT3);
  vdecCorner4->addElement( 0, offsetCorner4, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
  offsetCorner4 += VertexElement::getTypeSize(VET_FLOAT2);

  m_vbuf0Corner4 = HardwareBufferManager::getSingleton().createVertexBuffer(
                     offsetCorner4, // size of one whole vertex
                     numVertices, // number of vertices
                     HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
                     false); // no shadow buffer

  VertexBufferBinding* vbbindCorner4 = m_vdataCorner4->vertexBufferBinding;
  vbbindCorner4->setBinding(0, m_vbuf0Corner4);

  Real* pRealCorner4;
  pRealCorner4 = static_cast<Real*>(m_vbuf0Corner4->lock(HardwareBuffer::HBL_NORMAL));

  Real heightCorner4[4];

  heightCorner4[0] = m_TileManager->Get_Map_Height(m_x    , m_y    ) * StretchZ;
  heightCorner4[1] = m_TileManager->Get_Map_Height(m_x + 1, m_y    ) * StretchZ;
  heightCorner4[2] = m_TileManager->Get_Map_Height(m_x    , m_y + 1) * StretchZ;
  heightCorner4[3] = m_TileManager->Get_Map_Height(m_x + 1, m_y + 1) * StretchZ;

  for (int a = 0; a != 4; ++a)
  {
    if (heightCorner4[a] < LEVEL_WATER_CLP * StretchZ)
      heightCorner4[a] = LEVEL_WATER_CLP * StretchZ;
  }

  // 1st triangle
  pRealCorner4[ 0] =   0; pRealCorner4[ 1] =   heightCorner4[0] ; pRealCorner4[2] = 0;
  pRealCorner4[ 3] =   0; pRealCorner4[ 4] =   0; pRealCorner4[5] = 1;
  pRealCorner4[ 6] =   0; pRealCorner4[ 7] =   0;

  pRealCorner4[ 8] =   0; pRealCorner4[ 9] =   heightCorner4[2] ; pRealCorner4[10] = TILE_SIZE;
  pRealCorner4[11] =   0; pRealCorner4[12] =   0; pRealCorner4[13] = 1;
  pRealCorner4[14] =   0; pRealCorner4[15] =   1;

  pRealCorner4[16] =   TILE_SIZE; pRealCorner4[17] =  heightCorner4[1] ; pRealCorner4[18] = 0;
  pRealCorner4[19] =   0; pRealCorner4[20] =   0; pRealCorner4[21] = 1;
  pRealCorner4[22] =   1; pRealCorner4[23] =   0;

  // 2nd triangle
  pRealCorner4[24] =   TILE_SIZE; pRealCorner4[25] =   heightCorner4[1] ; pRealCorner4[26] = 0;
  pRealCorner4[27] =   0; pRealCorner4[28] =   0; pRealCorner4[29] = 1;
  pRealCorner4[30] =   1; pRealCorner4[31] =   0;

  pRealCorner4[32] =   TILE_SIZE; pRealCorner4[33] =  heightCorner4[2] ; pRealCorner4[34] = TILE_SIZE;
  pRealCorner4[35] =   0; pRealCorner4[36] =   0; pRealCorner4[37] = 1;
  pRealCorner4[38] =   0; pRealCorner4[39] =   1;

  pRealCorner4[40] =   TILE_SIZE; pRealCorner4[41] =   heightCorner4[3] ; pRealCorner4[42] = TILE_SIZE;
  pRealCorner4[43] =   0; pRealCorner4[44] =   0; pRealCorner4[45] = 1;
  pRealCorner4[46] =   1; pRealCorner4[47] =   1;

  m_vbuf0Corner4->unlock();

  HardwareIndexBufferSharedPtr m_ibufCorner4 = HardwareBufferManager::getSingleton().
      createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, // type of index
        numVertices, // number of indexes
        HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
        false); // no shadow buffer

  m_idataCorner4->indexBuffer = m_ibufCorner4;
  m_idataCorner4->indexStart = 0;
  m_idataCorner4->indexCount = numVertices;

  unsigned short* pIdxCorner4 = static_cast<unsigned short*>(m_ibufCorner4->lock(HardwareBuffer::HBL_DISCARD));

  for (unsigned short a = 0; a < (unsigned short)numVertices; a++)
    pIdxCorner4[a] = a;

  m_ibufCorner4->unlock();

  m_SubMeshCorner4->operationType = RenderOperation::OT_TRIANGLE_LIST;
  m_SubMeshCorner4->useSharedVertices = false;
  m_SubMeshCorner4->vertexData = m_vdataCorner4;

  AxisAlignedBox* boundsCorner4 = new AxisAlignedBox(
                                    0, 0 , 0,
                                    TILE_SIZE, 255 * StretchZ , TILE_SIZE);
  m_MeshCorner4->_setBounds( *boundsCorner4 );

  delete boundsCorner4;

  m_SubMeshCorner4->setMaterialName("Selection");
  sprintf( name2Corner4, "Selection Entity Corner4");
  m_EntityCorner4 = m_TileManager->Get_pSceneManager()->createEntity(name2Corner4, nameCorner4);

  m_SceneNodeCorner4 = m_TileManager->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
  m_SceneNodeCorner4->attachObject( m_EntityCorner4 );
  m_SceneNodeCorner4->setPosition(m_x * TILE_SIZE,0,m_y * TILE_SIZE);
  m_EntityCorner4->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);
}

void TileSelection::change_Selection()
{

  // only change if value differs from old one
  //if (m_x_old == m_x && m_y_old == m_y) return;

  // Tile Selection Marker
  float StretchZ = m_TileManager->Get_StretchZ();

  Real* pReal;
  pReal = static_cast<Real*>(m_vbuf0->lock(HardwareBuffer::HBL_NORMAL));

  Real height[4];
  height[0] = m_TileManager->Get_Map_Height(m_x    , m_y    ) * StretchZ;
  height[1] = m_TileManager->Get_Map_Height(m_x + 1, m_y    ) * StretchZ;
  height[2] = m_TileManager->Get_Map_Height(m_x    , m_y + 1) * StretchZ;
  height[3] = m_TileManager->Get_Map_Height(m_x + 1, m_y + 1) * StretchZ;

  for (int a = 0; a != 4; ++a)
  {
    if (height[a] < LEVEL_WATER_CLP * StretchZ)
      height[a] = LEVEL_WATER_CLP * StretchZ;
  }

  // 1st triangle
  pReal[ 0] =   0; pReal[ 1] =   height[0] ; pReal[2] = 0;
  pReal[ 3] =   0; pReal[ 4] =   0; pReal[5] = 1;
  pReal[ 6] =   0; pReal[ 7] =   0;

  pReal[ 8] =   0; pReal[ 9] =   height[2] ; pReal[10] = TILE_SIZE;
  pReal[11] =   0; pReal[12] =   0; pReal[13] = 1;
  pReal[14] =   0; pReal[15] =   1;

  pReal[16] =   TILE_SIZE; pReal[17] =  height[1] ; pReal[18] = 0;
  pReal[19] =   0; pReal[20] =   0; pReal[21] = 1;
  pReal[22] =   1; pReal[23] =   0;

  // 2nd triangle
  pReal[24] =   TILE_SIZE; pReal[25] =   height[1] ; pReal[26] = 0;
  pReal[27] =   0; pReal[28] =   0; pReal[29] = 1;
  pReal[30] =   1; pReal[31] =   0;

  pReal[32] =   0; pReal[33] =  height[2] ; pReal[34] = TILE_SIZE;
  pReal[35] =   0; pReal[36] =   0; pReal[37] = 1;
  pReal[38] =   0; pReal[39] =   1;

  pReal[40] =   TILE_SIZE; pReal[41] =   height[3] ; pReal[42] = TILE_SIZE;
  pReal[43] =   0; pReal[44] =   0; pReal[45] = 1;
  pReal[46] =   1; pReal[47] =   1;

  m_vbuf0->unlock();

  m_SceneNode->setPosition(m_x * TILE_SIZE,0,m_y * TILE_SIZE);

  //Corner1
  if (m_SquareSize > 1)
  {
    Real* pRealCorner1;
    pRealCorner1 = static_cast<Real*>(m_vbuf0Corner1->lock(HardwareBuffer::HBL_NORMAL));

    Real heightCorner1[4];
    heightCorner1[0] = m_TileManager->Get_Map_Height(m_x + (m_SquareSize-1)/2    , m_y + (m_SquareSize-1)/2   ) * StretchZ;
    heightCorner1[1] = m_TileManager->Get_Map_Height(m_x + (m_SquareSize-1)/2 +1 , m_y + (m_SquareSize-1)/2   ) * StretchZ;
    heightCorner1[2] = m_TileManager->Get_Map_Height(m_x + (m_SquareSize-1)/2    , m_y + (m_SquareSize-1)/2+ 1) * StretchZ;
    heightCorner1[3] = m_TileManager->Get_Map_Height(m_x + (m_SquareSize-1)/2 + 1, m_y + (m_SquareSize-1)/2+ 1) * StretchZ;

    for (int a = 0; a != 4; ++a)
    {
      if (heightCorner1[a] < LEVEL_WATER_CLP * StretchZ)
        heightCorner1[a] = LEVEL_WATER_CLP * StretchZ;
    }

    // 1st triangle
    pRealCorner1[ 0] =   0; pRealCorner1[ 1] =   heightCorner1[0] ; pRealCorner1[2] = 0;
    pRealCorner1[ 3] =   0; pRealCorner1[ 4] =   0; pRealCorner1[5] = 1;
    pRealCorner1[ 6] =   0; pRealCorner1[ 7] =   0;

    pRealCorner1[ 8] =   0; pRealCorner1[ 9] =   heightCorner1[2] ; pRealCorner1[10] = TILE_SIZE;
    pRealCorner1[11] =   0; pRealCorner1[12] =   0; pRealCorner1[13] = 1;
    pRealCorner1[14] =   0; pRealCorner1[15] =   1;

    pRealCorner1[16] =   TILE_SIZE; pRealCorner1[17] =  heightCorner1[1] ; pRealCorner1[18] = 0;
    pRealCorner1[19] =   0; pRealCorner1[20] =   0; pRealCorner1[21] = 1;
    pRealCorner1[22] =   1; pRealCorner1[23] =   0;

    // 2nd triangle
    pRealCorner1[24] =   TILE_SIZE; pRealCorner1[25] =   heightCorner1[1] ; pRealCorner1[26] = 0;
    pRealCorner1[27] =   0; pRealCorner1[28] =   0; pRealCorner1[29] = 1;
    pRealCorner1[30] =   1; pRealCorner1[31] =   0;

    pRealCorner1[32] =   0; pRealCorner1[33] =  heightCorner1[2] ; pRealCorner1[34] = TILE_SIZE;
    pRealCorner1[35] =   0; pRealCorner1[36] =   0; pRealCorner1[37] = 1;
    pRealCorner1[38] =   0; pRealCorner1[39] =   1;

    pRealCorner1[40] =   TILE_SIZE; pRealCorner1[41] =   heightCorner1[3] ; pRealCorner1[42] = TILE_SIZE;
    pRealCorner1[43] =   0; pRealCorner1[44] =   0; pRealCorner1[45] = 1;
    pRealCorner1[46] =   1; pRealCorner1[47] =   1;

    m_vbuf0Corner1->unlock();

    m_SceneNodeCorner1->setPosition((m_x + (m_SquareSize-1)/2 )* TILE_SIZE,0,(m_y + + (m_SquareSize-1)/2)* TILE_SIZE);

    //Corner2
    Real* pRealCorner2;
    pRealCorner2 = static_cast<Real*>(m_vbuf0Corner2->lock(HardwareBuffer::HBL_NORMAL));

    Real heightCorner2[4];
    heightCorner2[0] = m_TileManager->Get_Map_Height(m_x - (m_SquareSize-1)/2    , m_y + (m_SquareSize-1)/2   ) * StretchZ;
    heightCorner2[1] = m_TileManager->Get_Map_Height(m_x - (m_SquareSize-1)/2 +1 , m_y + (m_SquareSize-1)/2   ) * StretchZ;
    heightCorner2[2] = m_TileManager->Get_Map_Height(m_x - (m_SquareSize-1)/2    , m_y + (m_SquareSize-1)/2+ 1) * StretchZ;
    heightCorner2[3] = m_TileManager->Get_Map_Height(m_x - (m_SquareSize-1)/2 + 1, m_y + (m_SquareSize-1)/2+ 1) * StretchZ;

    for (int a = 0; a != 4; ++a)
    {
      if (heightCorner2[a] < LEVEL_WATER_CLP * StretchZ)
        heightCorner2[a] = LEVEL_WATER_CLP * StretchZ;
    }

    // 1st triangle
    pRealCorner2[ 0] =   0; pRealCorner2[ 1] =   heightCorner2[0] ; pRealCorner2[2] = 0;
    pRealCorner2[ 3] =   0; pRealCorner2[ 4] =   0; pRealCorner2[5] = 1;
    pRealCorner2[ 6] =   0; pRealCorner2[ 7] =   0;

    pRealCorner2[ 8] =   0; pRealCorner2[ 9] =   heightCorner2[2] ; pRealCorner2[10] = TILE_SIZE;
    pRealCorner2[11] =   0; pRealCorner2[12] =   0; pRealCorner2[13] = 1;
    pRealCorner2[14] =   0; pRealCorner2[15] =   1;

    pRealCorner2[16] =   TILE_SIZE; pRealCorner2[17] =  heightCorner2[1] ; pRealCorner2[18] = 0;
    pRealCorner2[19] =   0; pRealCorner2[20] =   0; pRealCorner2[21] = 1;
    pRealCorner2[22] =   1; pRealCorner2[23] =   0;

    // 2nd triangle
    pRealCorner2[24] =   TILE_SIZE; pRealCorner2[25] =   heightCorner2[1] ; pRealCorner2[26] = 0;
    pRealCorner2[27] =   0; pRealCorner2[28] =   0; pRealCorner2[29] = 1;
    pRealCorner2[30] =   1; pRealCorner2[31] =   0;

    pRealCorner2[32] =   0; pRealCorner2[33] =  heightCorner2[2] ; pRealCorner2[34] = TILE_SIZE;
    pRealCorner2[35] =   0; pRealCorner2[36] =   0; pRealCorner2[37] = 1;
    pRealCorner2[38] =   0; pRealCorner2[39] =   1;

    pRealCorner2[40] =   TILE_SIZE; pRealCorner2[41] =   heightCorner2[3] ; pRealCorner2[42] = TILE_SIZE;
    pRealCorner2[43] =   0; pRealCorner2[44] =   0; pRealCorner2[45] = 1;
    pRealCorner2[46] =   1; pRealCorner2[47] =   1;

    m_vbuf0Corner2->unlock();

    m_SceneNodeCorner2->setPosition((m_x - (m_SquareSize-1)/2 )* TILE_SIZE,0,(m_y + + (m_SquareSize-1)/2)* TILE_SIZE);

    //Corner 3
    Real* pRealCorner3;
    pRealCorner3 = static_cast<Real*>(m_vbuf0Corner3->lock(HardwareBuffer::HBL_NORMAL));

    Real heightCorner3[4];
    heightCorner3[0] = m_TileManager->Get_Map_Height(m_x + (m_SquareSize-1)/2    , m_y - (m_SquareSize-1)/2   ) * StretchZ;
    heightCorner3[1] = m_TileManager->Get_Map_Height(m_x + (m_SquareSize-1)/2 +1 , m_y - (m_SquareSize-1)/2   ) * StretchZ;
    heightCorner3[2] = m_TileManager->Get_Map_Height(m_x + (m_SquareSize-1)/2    , m_y - (m_SquareSize-1)/2+ 1) * StretchZ;
    heightCorner3[3] = m_TileManager->Get_Map_Height(m_x + (m_SquareSize-1)/2 + 1, m_y - (m_SquareSize-1)/2+ 1) * StretchZ;

    for (int a = 0; a != 4; ++a)
    {
      if (heightCorner3[a] < LEVEL_WATER_CLP * StretchZ)
        heightCorner3[a] = LEVEL_WATER_CLP * StretchZ;
    }

    // 1st triangle
    pRealCorner3[ 0] =   0; pRealCorner3[ 1] =   heightCorner3[0] ; pRealCorner3[2] = 0;
    pRealCorner3[ 3] =   0; pRealCorner3[ 4] =   0; pRealCorner3[5] = 1;
    pRealCorner3[ 6] =   0; pRealCorner3[ 7] =   0;

    pRealCorner3[ 8] =   0; pRealCorner3[ 9] =   heightCorner3[2] ; pRealCorner3[10] = TILE_SIZE;
    pRealCorner3[11] =   0; pRealCorner3[12] =   0; pRealCorner3[13] = 1;
    pRealCorner3[14] =   0; pRealCorner3[15] =   1;

    pRealCorner3[16] =   TILE_SIZE; pRealCorner3[17] =  heightCorner3[1] ; pRealCorner3[18] = 0;
    pRealCorner3[19] =   0; pRealCorner3[20] =   0; pRealCorner3[21] = 1;
    pRealCorner3[22] =   1; pRealCorner3[23] =   0;

    // 2nd triangle
    pRealCorner3[24] =   TILE_SIZE; pRealCorner3[25] =   heightCorner3[1] ; pRealCorner3[26] = 0;
    pRealCorner3[27] =   0; pRealCorner3[28] =   0; pRealCorner3[29] = 1;
    pRealCorner3[30] =   1; pRealCorner3[31] =   0;

    pRealCorner3[32] =   0; pRealCorner3[33] =  heightCorner3[2] ; pRealCorner3[34] = TILE_SIZE;
    pRealCorner3[35] =   0; pRealCorner3[36] =   0; pRealCorner3[37] = 1;
    pRealCorner3[38] =   0; pRealCorner3[39] =   1;

    pRealCorner3[40] =   TILE_SIZE; pRealCorner3[41] =   heightCorner3[3] ; pRealCorner3[42] = TILE_SIZE;
    pRealCorner3[43] =   0; pRealCorner3[44] =   0; pRealCorner3[45] = 1;
    pRealCorner3[46] =   1; pRealCorner3[47] =   1;

    m_vbuf0Corner3->unlock();

    m_SceneNodeCorner3->setPosition((m_x + (m_SquareSize-1)/2 )* TILE_SIZE,0,(m_y - (m_SquareSize-1)/2)* TILE_SIZE);

    //Corner 4
    Real* pRealCorner4;
    pRealCorner4 = static_cast<Real*>(m_vbuf0Corner4->lock(HardwareBuffer::HBL_NORMAL));

    Real heightCorner4[4];
    heightCorner4[0] = m_TileManager->Get_Map_Height(m_x - (m_SquareSize-1)/2    , m_y - (m_SquareSize-1)/2   ) * StretchZ;
    heightCorner4[1] = m_TileManager->Get_Map_Height(m_x - (m_SquareSize-1)/2 +1 , m_y - (m_SquareSize-1)/2   ) * StretchZ;
    heightCorner4[2] = m_TileManager->Get_Map_Height(m_x - (m_SquareSize-1)/2    , m_y - (m_SquareSize-1)/2+ 1) * StretchZ;
    heightCorner4[3] = m_TileManager->Get_Map_Height(m_x - (m_SquareSize-1)/2 + 1, m_y - (m_SquareSize-1)/2+ 1) * StretchZ;

    for (int a = 0; a != 4; ++a)
    {
      if (heightCorner4[a] < LEVEL_WATER_CLP * StretchZ)
        heightCorner4[a] = LEVEL_WATER_CLP * StretchZ;
    }

    // 1st triangle
    pRealCorner4[ 0] =   0; pRealCorner4[ 1] =   heightCorner4[0] ; pRealCorner4[2] = 0;
    pRealCorner4[ 3] =   0; pRealCorner4[ 4] =   0; pRealCorner4[5] = 1;
    pRealCorner4[ 6] =   0; pRealCorner4[ 7] =   0;

    pRealCorner4[ 8] =   0; pRealCorner4[ 9] =   heightCorner4[2] ; pRealCorner4[10] = TILE_SIZE;
    pRealCorner4[11] =   0; pRealCorner4[12] =   0; pRealCorner4[13] = 1;
    pRealCorner4[14] =   0; pRealCorner4[15] =   1;

    pRealCorner4[16] =   TILE_SIZE; pRealCorner4[17] =  heightCorner4[1] ; pRealCorner4[18] = 0;
    pRealCorner4[19] =   0; pRealCorner4[20] =   0; pRealCorner4[21] = 1;
    pRealCorner4[22] =   1; pRealCorner4[23] =   0;

    // 2nd triangle
    pRealCorner4[24] =   TILE_SIZE; pRealCorner4[25] =   heightCorner4[1] ; pRealCorner4[26] = 0;
    pRealCorner4[27] =   0; pRealCorner4[28] =   0; pRealCorner4[29] = 1;
    pRealCorner4[30] =   1; pRealCorner4[31] =   0;

    pRealCorner4[32] =   0; pRealCorner4[33] =  heightCorner4[2] ; pRealCorner4[34] = TILE_SIZE;
    pRealCorner4[35] =   0; pRealCorner4[36] =   0; pRealCorner4[37] = 1;
    pRealCorner4[38] =   0; pRealCorner4[39] =   1;

    pRealCorner4[40] =   TILE_SIZE; pRealCorner4[41] =   heightCorner4[3] ; pRealCorner4[42] = TILE_SIZE;
    pRealCorner4[43] =   0; pRealCorner4[44] =   0; pRealCorner4[45] = 1;
    pRealCorner4[46] =   1; pRealCorner4[47] =   1;

    m_vbuf0Corner4->unlock();

    m_SceneNodeCorner4->setPosition((m_x - (m_SquareSize-1)/2 )* TILE_SIZE,0,(m_y - (m_SquareSize-1)/2)* TILE_SIZE);

    m_Entity->setVisible (false);//hide the middle tile selection mark
    m_EntityCorner1->setVisible (true);//and show the corners
    m_EntityCorner2->setVisible (true);
    m_EntityCorner3->setVisible (true);
    m_EntityCorner4->setVisible (true);

  }//if squaresize > 1
  else
  {
    m_Entity->setVisible (true);//show the middle tile selection mark
    m_EntityCorner1->setVisible (false);//and hide the corners
    m_EntityCorner2->setVisible (false);
    m_EntityCorner3->setVisible (false);
    m_EntityCorner4->setVisible (false);
  }



}

void TileSelection::set_Square_Size(unsigned int SquareSize)
{
  m_SquareSize = SquareSize;
}




///================================================================================================
/// Constructor.
///================================================================================================
TileInterface::TileInterface(TileManager* TileManager)
{
  m_TileManager = TileManager;
  m_SceneNode = m_TileManager->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode("Interface");
  m_Selection = new TileSelection(m_TileManager);
  m_SquareSize = 1;
}

///================================================================================================
/// Destructor.
///================================================================================================
TileInterface::~TileInterface()
{
  delete m_Selection;
  m_TileManager->Get_pSceneManager()->destroyQuery(mRaySceneQuery);

}

///================================================================================================
/// Init.
///================================================================================================
void TileInterface::Init()
{
  mRaySceneQuery = m_TileManager->Get_pSceneManager()->createRayQuery( Ray() );
}

///================================================================================================
/// Mouse picking.
///================================================================================================
void TileInterface::pick_Tile(float mouseX, float mouseY)
{
  /// save old selection to compare to new selection later
  m_Selection->save_Selection();
  /// reset Selection_Tile
  m_Selection->reset();

  Ray mouseRay = m_TileManager->Get_pSceneManager()->getCamera("PlayerCam")->getCameraToViewportRay(mouseX, mouseY);
  mRaySceneQuery->setRay( mouseRay );

  /// Perform the scene query
  RaySceneQueryResult &result = mRaySceneQuery->execute();
  RaySceneQueryResult::iterator itr = result.begin( );

  /// Get the results
  while ( itr != result.end() && itr->movable) // we only collect movable objects (our terrain is movable!)
  {
    // make sure the query doesn't contain any overlay elements like the mouse cursor (which is always hit!)
    if (itr->movable->getRenderQueueGroup() != RENDER_QUEUE_OVERLAY)
    {
      // now test if a terrain chunk is hit
      for (int a = 0; a != CHUNK_SUM_X; ++a)
      {
        for (int b = 0; b != CHUNK_SUM_Z; ++b)
        {
          if (itr->movable == this->m_TileManager->get_TileChunk(a,b)->Get_Land_entity())
          {
            // we found our chunk, now search for the correct tile
            pick_Tile(&mouseRay,a,b);
          }
        }
      }
    }
    itr++;
  }

  /// now m_Selection contains the wanted tile
  if (m_Selection->m_x != -1 && m_Selection->m_y != -1)
  {
    m_Selection->select();
  }
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::pick_Tile(Ray* mouseRay, int a, int b)
{
  // we start with a given tile chunk (coordinates a and b) and a ray and try to find the tile that was selected
  // we have to check every tile if it was hit and return the nearest one
  Real height[5], lower, higher;
  int vertex_x = m_TileManager->get_TileChunk(a,b)->get_posX() * CHUNK_SIZE_X;
  int vertex_y = m_TileManager->get_TileChunk(a,b)->get_posZ() * CHUNK_SIZE_Z;

  float StretchZ = m_TileManager->Get_StretchZ();

  for (int x = 0; x != CHUNK_SIZE_X; ++x)
  {
    for (int y = 0; y!= CHUNK_SIZE_Z; ++y)
    {
      // we have to build a bounding box for each tile and check if the ray intersects this box
      // to do this we need the height of the tile corner vertices

      height[0] = m_TileManager->Get_Map_Height(vertex_x + x    , vertex_y + y    ) * StretchZ;
      height[1] = m_TileManager->Get_Map_Height(vertex_x + x + 1, vertex_y + y    ) * StretchZ;
      height[2] = m_TileManager->Get_Map_Height(vertex_x + x    , vertex_y + y + 1) * StretchZ;
      height[3] = m_TileManager->Get_Map_Height(vertex_x + x + 1, vertex_y + y + 1) * StretchZ;
      height[4] = (height[0]+height[1]+height[2]+height[3]) /4.0;

      // now we build four bounding boxes per tile to increase picking accuracy
      // note: Ogre only allows bounding boxes with the first vector having got the lower value in every(!) component
      // so we have to check which height value is greater
      if (height[0] > height[4])
      {
        lower = height[4];
        higher = height[0];
      }
      else
      {
        lower = height[0];
        higher = height[4];
      }

      AxisAlignedBox Box1((vertex_x + x)* TILE_SIZE, lower , (vertex_y + y)* TILE_SIZE,
                          (vertex_x + x + .5)* TILE_SIZE, higher, (vertex_y + y + .5)* TILE_SIZE);

      if (height[1] > height[4])
      {
        lower = height[4];
        higher = height[1];
      }
      else
      {
        lower = height[1];
        higher = height[4];
      }

      AxisAlignedBox Box2((vertex_x + x + .5)* TILE_SIZE, lower, (vertex_y + y)* TILE_SIZE,
                          (vertex_x + x + 1)* TILE_SIZE, higher, (vertex_y + y + .5)* TILE_SIZE);

      if (height[2] > height[4])
      {
        lower = height[4];
        higher = height[2];
      }
      else
      {
        lower = height[2];
        higher = height[4];
      }

      AxisAlignedBox Box3((vertex_x + x)* TILE_SIZE, lower , (vertex_y + y + .5)* TILE_SIZE,
                          (vertex_x + x + .5)* TILE_SIZE, higher, (vertex_y + y + 1)* TILE_SIZE);

      if (height[3] > height[4])
      {
        lower = height[4];
        higher = height[3];
      }
      else
      {
        lower = height[3];
        higher = height[4];
      }

      AxisAlignedBox Box4((vertex_x + x + .5)* TILE_SIZE, lower , (vertex_y + y + .5)* TILE_SIZE,
                          (vertex_x + x + 1)* TILE_SIZE, higher, (vertex_y + y + 1)* TILE_SIZE);

      // Test box
      std::pair< bool, Real > Test1 = mouseRay->intersects(Box1);
      std::pair< bool, Real > Test2 = mouseRay->intersects(Box2);
      std::pair< bool, Real > Test3 = mouseRay->intersects(Box3);
      std::pair< bool, Real > Test4 = mouseRay->intersects(Box4);

      if (Test1.first == true)
      {
        // intersection!
        // find the closest intersection to the camera
        if (Test1.second < m_Selection->m_distance)
        {
          m_Selection->m_distance = Test1.second;
          m_Selection->m_x  = vertex_x + x;
          m_Selection->m_y  = vertex_y + y;
        }
      }

      if (Test2.first == true)
      {
        // intersection!
        // find the closest intersection to the camera
        if (Test2.second < m_Selection->m_distance)
        {
          m_Selection->m_distance = Test2.second;
          m_Selection->m_x  = vertex_x + x;
          m_Selection->m_y  = vertex_y + y;
        }
      }

      if (Test3.first == true)
      {
        // intersection!
        // find the closest intersection to the camera
        if (Test3.second < m_Selection->m_distance)
        {
          m_Selection->m_distance = Test3.second;
          m_Selection->m_x  = vertex_x + x;
          m_Selection->m_y  = vertex_y + y;
        }
      }

      if (Test4.first == true)
      {
        // intersection!
        // find the closest intersection to the camera
        if (Test4.second < m_Selection->m_distance)
        {
          m_Selection->m_distance = Test4.second;
          m_Selection->m_x  = vertex_x + x;
          m_Selection->m_y  = vertex_y + y;
        }
      }
    } // end for y
  } // end for x
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::set_Square_Size(unsigned int SquareSize)
{
  m_SquareSize = SquareSize;
  m_Selection->set_Square_Size(SquareSize);// inform the marker of the selected tile
  m_Selection->change_Selection();// update the marker of the selected tile
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::change_Tile_height(int z_direction)
{
  unsigned char value;

  for (int a = (int) m_Selection->m_x - ((int)m_SquareSize-1)/2; a <= (int) m_Selection->m_x +((int)m_SquareSize+1)/2; ++a)
  {
    for (int b = (int) m_Selection->m_y - ((int)m_SquareSize-1)/2; b <= (int) m_Selection->m_y +((int)m_SquareSize+1)/2; ++b)
    {
      value = m_TileManager->Get_Map_Height(a, b)+ z_direction;
      if (value > 220) value = 0;//cap, is his important ?? maybe delete later on
      m_TileManager->Set_Map_Height(a, b, value);
    }
  }
  m_Selection->change_Selection();// update the marker of the selected tile
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::level_Tile_Corner_height(int z_direction)
{
  int z_max = -1000;
  int z_min = 1000;
  unsigned char value;

  for (int a = (int)m_Selection->m_x - ((int)m_SquareSize-1)/2; a <= (int)m_Selection->m_x +((int)m_SquareSize+1)/2; ++a)
  {
    for (int b = (int)m_Selection->m_y - ((int)m_SquareSize-1)/2; b <= (int)m_Selection->m_y +((int)m_SquareSize+1)/2; ++b)
    {
      // find the maximal z value in area and store to z_max
      if ((m_TileManager->Get_Map_Height(a, b) > z_max)&&(z_direction > 0))
      {
        z_max = m_TileManager->Get_Map_Height(a, b);
      }
      // find the minimal z value in area and store to z_min
      if ((m_TileManager->Get_Map_Height(a, b) < z_min)&&(z_direction < 0))
      {
        z_min = m_TileManager->Get_Map_Height(a, b);
      }
    }
  }
  // perform the leveling
  for (int a = (int)m_Selection->m_x - ((int)m_SquareSize-1)/2; a <= (int)m_Selection->m_x +((int)m_SquareSize+1)/2; ++a)
  {
    for (int b = (int)m_Selection->m_y - ((int)m_SquareSize-1)/2; b <= (int)m_Selection->m_y +((int)m_SquareSize+1)/2; ++b)
    {
      // level the corners to the maximun
      if (z_direction > 0)
      {
        value = m_TileManager->Get_Map_Height(a, b);
        if (value > 220) value = 0; //cap, is his important ?? maybe delete later on
        if (value < z_max) m_TileManager->Set_Map_Height(a, b, z_max);
      }
      // or,level the corners to the minimum
      if (z_direction < 0)
      {
        value = m_TileManager->Get_Map_Height(a, b);
        if (value > 220) value = 0; //cap, is his important ?? maybe delete later on
        if (value > z_min) m_TileManager->Set_Map_Height(a, b, z_min);
      }
    }
  }
  m_Selection->change_Selection();// update the marker of the selected tile
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::level_Tile_Corner_height(int z_direction,int SquareSize)
{
  int tmpSquareSize = m_SquareSize;
  m_SquareSize = SquareSize;
  level_Tile_Corner_height(z_direction);
  m_SquareSize = tmpSquareSize;
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::level_Tile_Corner_height(int z_direction,int SquareSize,int x,int y)
{
  int z_max = -1000;
  int z_min =  1000;
  unsigned char value;

  for (int a = x - (SquareSize-1)/2; a <= x +(SquareSize+1)/2; ++a)
  {
    for (int b = y - (SquareSize-1)/2; b <= y +(SquareSize+1)/2; ++b)
    {
      // find the maximal z value in area and store to z_max
      if ((m_TileManager->Get_Map_Height(a, b) > z_max)&&(z_direction > 0))
      {
        z_max = m_TileManager->Get_Map_Height(a, b);
      }
      // find the minimal z value in area and store to z_min
      if ((m_TileManager->Get_Map_Height(a, b) < z_min)&&(z_direction < 0))
      {
        z_min = m_TileManager->Get_Map_Height(a, b);
      }
    }
  }
  // perform the leveling
  for (int a = x - (SquareSize-1)/2; a <= x +(SquareSize+1)/2; ++a)
  {
    for (int b = y - (SquareSize-1)/2; b <= y +(SquareSize+1)/2; ++b)
    {
      // level the corners to the maximun
      if (z_direction > 0)
      {
        value = m_TileManager->Get_Map_Height(a, b);
        if (value > 220) value = 0; //cap, is his important ?? maybe delete later on
        if (value < z_max) m_TileManager->Set_Map_Height(a, b, z_max);
      }
      // or,level the corners to the minimum
      if (z_direction < 0)
      {
        value = m_TileManager->Get_Map_Height(a, b);
        if (value > 220) value = 0; //cap, is his important ?? maybe delete later on
        if (value > z_min) m_TileManager->Set_Map_Height(a, b, z_min);
      }
    }
  }
}

///================================================================================================
/// .
///================================================================================================
bool TileInterface::Tile_Corner_height_is_leveled(int z_direction,int SquareSize,int x,int y)
{
  int z_max = -1000;
  int z_min = 1000;

  for (int a = x - (SquareSize-1)/2; a <= x +(SquareSize+1)/2; ++a)
  {
    for (int b = y - (SquareSize-1)/2; b <= y +(SquareSize+1)/2; ++b)
    {
      // find the maximal z value in area and store to z_max
      if ((m_TileManager->Get_Map_Height(a, b) > z_max)&&(z_direction > 0))
      {
        z_max = m_TileManager->Get_Map_Height(a, b);
      }
      // find the minimal z value in area and store to z_min
      if ((m_TileManager->Get_Map_Height(a, b) < z_min)&&(z_direction < 0))
      {
        z_min = m_TileManager->Get_Map_Height(a, b);
      }
    }
  }
  if(z_min == z_max) return true;
  return false;
}

