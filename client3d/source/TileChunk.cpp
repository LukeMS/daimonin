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
#include "TileChunk.h"
#include "TileManager.h"

/////////////////////////////////////////////////////////////////////////
// Init static elements.
/////////////////////////////////////////////////////////////////////////
CTileManager   *CChunk::m_TileManagerPtr = NULL;
AxisAlignedBox *CChunk::m_bounds = NULL;
char CChunk::MeshName[NAME_BUFFER_SIZE];
char CChunk::TempName[NAME_BUFFER_SIZE];

//=================================================================================================
// Constructor.
//=================================================================================================
CChunk::CChunk()
{
	m_posX = -1;
	m_posY = -1;
	m_Land_subMesh_high  = NULL;
	m_Land_subMesh_low   = NULL;
	m_Land_entity_high   = NULL;
	m_Land_entity_low    = NULL;
	m_Water_subMesh_high = NULL;
	m_Water_subMesh_low  = NULL;
	m_Water_entity_high  = NULL;
	m_Water_entity_low   = NULL;
}

//=================================================================================================
// Destructor.
//=================================================================================================
CChunk::~CChunk()
{
	m_Water_Mesh_high.setNull();
	m_Water_Mesh_low.setNull();
	m_Land_Mesh_high.setNull();
	m_Land_Mesh_low.setNull();
	m_Kartentextur.setNull();
}

//=================================================================================================
// Create Sctene-Nodes.
//=================================================================================================
void CChunk::CreateSceneNode()
{
	m_Land  = m_TileManagerPtr->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
	m_Land ->setPosition(
		m_posX * TILE_SIZE * CHUNK_SIZE_X + TILE_SIZE * CHUNK_SIZE_X / 2,
		0,
		m_posY * TILE_SIZE * CHUNK_SIZE_Y + TILE_SIZE * CHUNK_SIZE_Y / 2);
	m_Water = m_TileManagerPtr->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
	m_Water->setPosition(
		m_posX * TILE_SIZE * CHUNK_SIZE_X + TILE_SIZE * CHUNK_SIZE_X / 2,
		0,
		m_posY * TILE_SIZE * CHUNK_SIZE_Y + TILE_SIZE * CHUNK_SIZE_Y / 2 );
}

//=================================================================================================
// Attach.
//=================================================================================================
void CChunk::Attach(short quality)
{
	if (m_Land)		m_Land->detachAllObjects();
	if (m_Water)	m_Water->detachAllObjects();
		if (quality == QUALITY_LOW)
	{
			if (m_Land_entity_low != NULL)	m_Land->attachObject(m_Land_entity_low);
			if (m_Water_entity_low != NULL)	m_Water->attachObject(m_Water_entity_low);
	}
	else if (quality == QUALITY_HIGH)
	{
		if (m_Land_entity_high != NULL)		m_Land->attachObject(m_Land_entity_high);
		if (m_Water_entity_high != NULL)	m_Water->attachObject(m_Water_entity_high);
	}
}

//=================================================================================================
// Detach.
//=================================================================================================
void CChunk::Detach()
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

//=================================================================================================
// Create a new Chunk.
//=================================================================================================
void CChunk::Create(short &x, short &y)
{
	Set_Tile(x, y);
	CreateLandLow();
	CreateLandHigh();
	CreateWaterLow();
	CreateWaterHigh();
	CreateSceneNode();
	CreateEnvironmentManager();
}

//=================================================================================================
// Change a Chunk.
//=================================================================================================
void CChunk::Change(short &x, short &y)
{
	int x1 = x * CHUNK_SIZE_X;
	int y1 = y * CHUNK_SIZE_Y;

	unsigned char value;
	for (int a = x1; a < x1 + CHUNK_SIZE_X; ++a)
	{
		for (int b = y1; b < y1 + CHUNK_SIZE_Y; ++b)
		{
			value = m_TileManagerPtr->Get_Map_Height(a, b)+ 1;
			if (value > 220) value = 0;
			m_TileManagerPtr->Set_Map_Height(a, b, value);
		}
	}

	ChangeLandHigh();
	ChangeWaterHigh();
//	ChangeLandLow(); TODO
//	ChangeWaterLow(); TODO
//	ChangeEnvironmentManager();
}


//=================================================================================================
// Create EnvironmentManager
//=================================================================================================
void CChunk::CreateEnvironmentManager()
{
	m_EnvironmentManagerPtr = new CEnvironmentManager(m_TileManagerPtr, this);
	m_EnvironmentManagerPtr->UpdateEnvironment();
}

//=================================================================================================
// Create Water in low Quality
//=================================================================================================
void CChunk::CreateWaterLow()
	{
	int x = m_posX * CHUNK_SIZE_X;
	int y = m_posY * CHUNK_SIZE_Y;
	float StretchZ = m_TileManagerPtr->Get_StretchZ();
	unsigned long numVertices = 0;

	// Bestimmung der Anzahl der Geometriepunkte

	for (short a = x; a < x + CHUNK_SIZE_X; ++a)
			{
			for (short b = y; b < y + CHUNK_SIZE_Y; ++b)
					{
					if (m_TileManagerPtr->Get_Map_Height(a, b  ) <= LEVEL_WATER_TOP && m_TileManagerPtr->Get_Map_Height(a+1, b  ) <= LEVEL_WATER_TOP &&
							m_TileManagerPtr->Get_Map_Height(a, b+1) <= LEVEL_WATER_TOP && m_TileManagerPtr->Get_Map_Height(a+1, b+1) <= LEVEL_WATER_TOP)
							{
							numVertices += 6;
							} // endif
					} // b
			} // a

	if (numVertices == 0)
		return;

	VertexData* vdata = new VertexData();
	IndexData* idata;

	sprintf( MeshName, "Water[%d,%d] Low", x, y );
	m_Water_Mesh_low = MeshManager::getSingleton().createManual( MeshName,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	sprintf( TempName, "SubWater[%d,%d] Low", x, y );
	m_Water_subMesh_low = m_Water_Mesh_low->createSubMesh(TempName);
	idata = m_Water_subMesh_low->indexData;

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

	Real* pReal;
	pReal = static_cast<Real*>(vbuf0->lock(HardwareBuffer::HBL_NORMAL))
					;
	long z = -60;
	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[0] = - (TILE_SIZE * HALF_CHUNK_X);
	pReal[1] =  LEVEL_WATER_TOP * StretchZ;
	pReal[2] = - (TILE_SIZE * HALF_CHUNK_Y);


	pReal[3] = 0;
	pReal[4] = 1;
	pReal[5] = 0;

	pReal[6] =  0;
	pReal[7] =  0;

	pReal[8] = 0;
	pReal[9] = 0;

	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[10] = + (TILE_SIZE * HALF_CHUNK_X);
	pReal[11] = LEVEL_WATER_TOP * StretchZ;
	pReal[12] = - (TILE_SIZE * HALF_CHUNK_Y);


	pReal[13] = 0;
	pReal[14] = 1;
	pReal[15] = 0;

	pReal[16] = CHUNK_SIZE_X /4;
	pReal[17] = 0;

	pReal[18] = 1;
	pReal[19] = 0;

	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[20] = - (TILE_SIZE * HALF_CHUNK_X);
	pReal[21] = LEVEL_WATER_TOP * StretchZ;
	pReal[22] = + (TILE_SIZE * HALF_CHUNK_Y);

	pReal[23] = 0;
	pReal[24] = 1;
	pReal[25] = 0;

	pReal[26] = 0;
	pReal[27] = CHUNK_SIZE_Y /4;

	pReal[28] = 0;
	pReal[29] = 1;


	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[30] = - (TILE_SIZE * HALF_CHUNK_X);
	pReal[31] = LEVEL_WATER_TOP * StretchZ;
	pReal[32] = + (TILE_SIZE * HALF_CHUNK_Y);


	pReal[33] = 0;
	pReal[34] = 1;
	pReal[35] = 0;

	pReal[36] = 0;
	pReal[37] = CHUNK_SIZE_Y /4;

	pReal[38] = 0;
	pReal[39] = 1;


	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[40] = + (TILE_SIZE * HALF_CHUNK_X);
	pReal[41] = LEVEL_WATER_TOP * StretchZ;
	pReal[42] = - (TILE_SIZE * HALF_CHUNK_Y);


	pReal[43] = 0;
	pReal[44] = 1;
	pReal[45] = 0;

	pReal[46] = CHUNK_SIZE_X /4;
	pReal[47] = 0;

	pReal[48] = 1;
	pReal[49] = 0;


	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[50] = + (TILE_SIZE * HALF_CHUNK_X);
	pReal[51] = LEVEL_WATER_TOP * StretchZ;
	pReal[52] = + (TILE_SIZE * HALF_CHUNK_Y);

	pReal[53] = 0;
	pReal[54] = 1;
	pReal[55] = 0;

	pReal[56] = CHUNK_SIZE_X /4;
	pReal[57] = CHUNK_SIZE_Y /4;

	pReal[58] = 1;
	pReal[59] = 1;

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

	// Setzen des Sichtbarkeits-Quaders. Fällt dieser Quader außerhalb des Sichtbereits der
	// Kamera, so wird das Kartenstück nicht gerendert.
	m_Water_Mesh_low->_setBounds( *m_bounds );
	m_Water_Mesh_low->load();
	m_Water_subMesh_low->setMaterialName("Water_LowDetails");
	sprintf( TempName, "Water[%d,%d] Low Entity", m_posX, m_posY );
	m_Water_entity_low = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);

	m_IsAttached = false;

	// m_Water->attachObject( m_Water_entity );
}

//=================================================================================================
// Create Water in high Quality
//=================================================================================================
void CChunk::CreateWaterHigh()
{
	int x = m_posX * CHUNK_SIZE_X;
	int y = m_posY * CHUNK_SIZE_Y;
		/////////////////////////////////////////////////////////////////////////
	// Create Mesh with a SubMesh.
	/////////////////////////////////////////////////////////////////////////
	sprintf( MeshName, "Water[%d,%d] High", x, y );
	m_Water_Mesh_high = MeshManager::getSingleton().createManual( MeshName,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	sprintf( TempName, "SubWater[%d,%d] High", x, y );
	m_Water_subMesh_high = m_Water_Mesh_high->createSubMesh(TempName);
	m_Water_subMesh_high->setMaterialName("Water_HighDetails");

	CreateWaterHigh_Buffers();

	m_Water_Mesh_high->_setBounds( *m_bounds ); // Rendering is only done when Camera looks into this quad.
	m_Water_Mesh_high->load();

	sprintf( TempName, "Water[%d,%d] High Entity", m_posX, m_posY );
	m_Water_entity_high = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);
	m_IsAttached = false;
}

//=================================================================================================
// Change high Quality Water.
//=================================================================================================
void CChunk::ChangeWaterHigh()
{
	delete m_Water_subMesh_high->vertexData;
	CreateWaterHigh_Buffers();
}

//=================================================================================================
//
//=================================================================================================
void CChunk::CreateWaterHigh_Buffers()
{
	int x = m_posX * CHUNK_SIZE_X;
	int y = m_posY * CHUNK_SIZE_Y;

	/////////////////////////////////////////////////////////////////////////
	// Count the Vertices in this chunk.
	/////////////////////////////////////////////////////////////////////////
	unsigned int numVertices = 0;
	for (short a = x; a < x + CHUNK_SIZE_X; ++a)
	{
		for (short b = y; b < y + CHUNK_SIZE_Y; ++b)
		{
			if (m_TileManagerPtr->Get_Map_Height(a, b  ) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) <= LEVEL_WATER_TOP ||
					m_TileManagerPtr->Get_Map_Height(a, b+1) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) <= LEVEL_WATER_TOP)
			{
				numVertices += 6;
			}
		}
	}
	if (numVertices == 0)  { Create_Dummy(m_Water_subMesh_high); return; }

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

	long z = 0;
	Real StretchZ = m_TileManagerPtr->Get_StretchZ();
	Real* pReal = static_cast<Real*>(vbuf0->lock(HardwareBuffer::HBL_NORMAL));
	for (short a = x; a < x+ CHUNK_SIZE_X; ++a)
	{
		for (short b = y; b < y +CHUNK_SIZE_Y; ++b)
		{
			if (m_TileManagerPtr->Get_Map_Height(a, b  ) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) <= LEVEL_WATER_TOP ||
					m_TileManagerPtr->Get_Map_Height(a, b+1) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) <= LEVEL_WATER_TOP)
			{
				/////////////////////////////////////////////////////////////////////////
				// Position.
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal[z   ] = TILE_SIZE * (a-x - HALF_CHUNK_X);
				pReal[z+ 1] = LEVEL_WATER_CLP * StretchZ;
				pReal[z+ 2] = TILE_SIZE * (b-y - HALF_CHUNK_Y);
				pReal[z+10] = TILE_SIZE * ((a-x)+1- HALF_CHUNK_X);
				pReal[z+11] = LEVEL_WATER_CLP* StretchZ;
				pReal[z+12] = TILE_SIZE * (b-y- HALF_CHUNK_Y);
				pReal[z+20] = TILE_SIZE * (a-x- HALF_CHUNK_X);
				pReal[z+21] = LEVEL_WATER_CLP* StretchZ;
				pReal[z+22] = TILE_SIZE * ((b-y)+1- HALF_CHUNK_Y);
				// 2. Triangle
				pReal[z+30] = TILE_SIZE * (a-x- HALF_CHUNK_X);
				pReal[z+31] = LEVEL_WATER_CLP* StretchZ;
				pReal[z+32] = TILE_SIZE * ((b-y)+1- HALF_CHUNK_Y);
				pReal[z+40] = TILE_SIZE * ((a-x)+1- HALF_CHUNK_X);
				pReal[z+41] = LEVEL_WATER_CLP * StretchZ;
				pReal[z+42] = TILE_SIZE * (b-y- HALF_CHUNK_Y);
				pReal[z+50] = TILE_SIZE * ((a-x)+1- HALF_CHUNK_X);
				pReal[z+51] = LEVEL_WATER_CLP * StretchZ;
				pReal[z+52] = TILE_SIZE * ((b-y)+1- HALF_CHUNK_Y);

				/////////////////////////////////////////////////////////////////////////
				// Normalvektoren
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal[z+ 3] = 0;	pReal[z+ 4] = 1;	pReal[z+ 5] = 0;
				pReal[z+13] = 0;	pReal[z+14] = 1;	pReal[z+15] = 0;
				pReal[z+23] = 0;	pReal[z+24] = 1;	pReal[z+25] = 0;
				// 2. Triangle
				pReal[z+33] = 0;	pReal[z+34] = 1;	pReal[z+35] = 0;
				pReal[z+43] = 0;	pReal[z+44] = 1;	pReal[z+45] = 0;
				pReal[z+53] = 0;	pReal[z+54] = 1;	pReal[z+55] = 0;
				/////////////////////////////////////////////////////////////////////////
				// Texture.
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal[z+ 6] = (a-x  ) / 4.0;	pReal[z+ 7] = (b-y  ) / 4.0;
				pReal[z+16] = (a+1-x) / 4.0;	pReal[z+17] = (b-y  ) / 4.0;
				pReal[z+26] = (a-x  ) / 4.0;	pReal[z+27] = (b+1-y) / 4.0;
				// 2. Triangle
				pReal[z+36] = (a-x  ) / 4.0;	pReal[z+37] = (b+1-y) / 4.0;
				pReal[z+46] = (a+1-x) / 4.0;	pReal[z+47] = (b-y  ) / 4.0;
				pReal[z+56] = (a+1-x) / 4.0;	pReal[z+57] = (b+1-y) / 4.0;
				/////////////////////////////////////////////////////////////////////////
				// Grid-Texture.
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal[z+ 8] = 0;	pReal[z+ 9] = 0;
				pReal[z+18] = 1;	pReal[z+19] = 0;
				pReal[z+28] = 0;	pReal[z+29] = 1;
				// 2. Triangle
				pReal[z+38] = 0;	pReal[z+39] = 1;
				pReal[z+48] = 1;	pReal[z+49] = 0;
				pReal[z+58] = 1;	pReal[z+59] = 1;

				z += 60;
			} // if
		} // y
	} // x
	vbuf0->unlock();
	/////////////////////////////////////////////////////////////////////////
	// Create Index-buffer
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

//=================================================================================================
// Create Land in low Quality. 1 Tile = 2 triangles.
// +----+
// |\ 2 |
// | \  |
// |  \ |
// | 1 \|
// +----+
//=================================================================================================
void CChunk::CreateLandLow()
{
	int x = m_posX * CHUNK_SIZE_X;
	int y = m_posY * CHUNK_SIZE_Y;
	float StretchZ = m_TileManagerPtr->Get_StretchZ();
	unsigned int numVertices = 0;
	/////////////////////////////////////////////////////////////////////////
	// Bestimmung der Anzahl der Geometriepunkte
	/////////////////////////////////////////////////////////////////////////
	for (int a = x; a < x + CHUNK_SIZE_X; a += 2)
	{
		for (int b = y; b < y + CHUNK_SIZE_Y; b += 2)
		{
				if (m_TileManagerPtr->Get_Map_Height(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) > LEVEL_WATER_TOP ||
						m_TileManagerPtr->Get_Map_Height(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) > LEVEL_WATER_TOP)
			{
					numVertices += 6;
			}
		}
	}
	if (numVertices == 0) { return; }
	VertexData* vdata = new VertexData();
	sprintf( MeshName, "Land[%d,%d] Low", x, y );
	m_Land_Mesh_low = MeshManager::getSingleton().createManual( MeshName,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	sprintf( TempName, "SubLand[%d,%d] Low", x, y );
	m_Land_subMesh_low = m_Land_Mesh_low->createSubMesh(TempName);

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

	int z = 0;
	Real g,h,d,f;
	short row, col;
	for (int a = x; a < x+ CHUNK_SIZE_X; a += 2)
			{
			for (int b = y; b < y +CHUNK_SIZE_Y; b += 2)
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
							pReal1[z   ] = TILE_SIZE * (a-x - HALF_CHUNK_X);
							pReal1[z+ 1] = g;
							pReal1[z+ 2] = TILE_SIZE * (b-y - HALF_CHUNK_Y);
							pReal1[z+10] = TILE_SIZE * (a-x+2- HALF_CHUNK_X);
							pReal1[z+11] = h;
							pReal1[z+12] = TILE_SIZE * (b-y  - HALF_CHUNK_Y);
							pReal1[z+20] = TILE_SIZE * (a-x  - HALF_CHUNK_X);
							pReal1[z+21] = d;
							pReal1[z+22] = TILE_SIZE * (b-y+2- HALF_CHUNK_Y);
							// 2. Triangle
							pReal1[z+30] = TILE_SIZE * (a-x-   HALF_CHUNK_X);
							pReal1[z+31] = d;
							pReal1[z+32] = TILE_SIZE * (b-y+2- HALF_CHUNK_Y);
							pReal1[z+40] = TILE_SIZE * (a-x+2- HALF_CHUNK_X);
							pReal1[z+41] = h;
							pReal1[z+42] = TILE_SIZE * (b-y-   HALF_CHUNK_Y);
							pReal1[z+50] = TILE_SIZE * (a-x+2- HALF_CHUNK_X);
							pReal1[z+51] = f;
							pReal1[z+52] = TILE_SIZE * (b-y+2- HALF_CHUNK_Y);
							/////////////////////////////////////////////////////////////////////////
							// Normals.
							/////////////////////////////////////////////////////////////////////////
							// 1. Triangle
							pReal1[z+ 3] = 0;	pReal1[z+ 4] = 1;	pReal1[z+ 5] = 0;
							pReal1[z+13] = 0;	pReal1[z+14] = 1;	pReal1[z+15] = 0;
							pReal1[z+23] = 0;	pReal1[z+24] = 1;	pReal1[z+25] = 0;
							// 2. Triangle
							pReal1[z+33] = 0;	pReal1[z+34] = 1;	pReal1[z+35] = 0;
							pReal1[z+43] = 0;	pReal1[z+44] = 1;	pReal1[z+45] = 0;
							pReal1[z+53] = 0;	pReal1[z+54] = 1;	pReal1[z+55] = 0;
							/////////////////////////////////////////////////////////////////////////
							// Textures.
							/////////////////////////////////////////////////////////////////////////
							col = m_TileManagerPtr->Get_Map_Texture_Col(a,b);
							row = m_TileManagerPtr->Get_Map_Texture_Row(a,b);
							// 1. triangle
							pReal1[z+ 6] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
							pReal1[z+ 7] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
							pReal1[z+16] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
							pReal1[z+17] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
							pReal1[z+26] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
							pReal1[z+27] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
							// 2. Triangle
							pReal1[z+36] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
							pReal1[z+37] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
							pReal1[z+46] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
							pReal1[z+47] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
							pReal1[z+56] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
							pReal1[z+57] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
							/////////////////////////////////////////////////////////////////////////
							// Grid.
							/////////////////////////////////////////////////////////////////////////
							// 1. Triangle
							pReal1[z+ 8] = 0;
							pReal1[z+ 9] = 0;
							pReal1[z+18] = 1;
							pReal1[z+19] = 0;
							pReal1[z+28] = 0;
							pReal1[z+29] = 1;
							// 2. Triangle
							pReal1[z+38] = 0;
							pReal1[z+39] = 1;
							pReal1[z+48] = 1;
							pReal1[z+49] = 0;
							pReal1[z+58] = 1;
							pReal1[z+59] = 1;

							z += 60;
							} // if
					} // y
			} // x
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

	// Setzen des Sichtbarkeits-Quaders. Fällt dieser Quader außerhalb des Sichtbereits der
	// Kamera, so wird das Kartenstück nicht gerendert.
	m_Land_Mesh_low->_setBounds( *m_bounds );

	m_Land_subMesh_low->setMaterialName("Land_LowDetails");
	sprintf( TempName, "Land[%d,%d] Low Entity", m_posX, m_posY );
	m_Land_entity_low = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);

	m_IsAttached = false;

	//m_Land->attachObject( m_Land_entity_low );
	m_Land_Mesh_low->load();
	}

//=================================================================================================
// Create Land in high Quality. 1 Tile = 4 triangles. We need this for the filter.
// +------+
// |\  4 /|
// | \  / |
// |  \/  |
// |1 /\ 3|
// | /  \ |
// |/  2 \|
// +------+
//=================================================================================================
void CChunk::CreateLandHigh()
{
	int x = m_posX * CHUNK_SIZE_X;
	int y = m_posY * CHUNK_SIZE_Y;
	/////////////////////////////////////////////////////////////////////////
	// Create Mesh with a SubMesh.
	/////////////////////////////////////////////////////////////////////////
	sprintf(MeshName, "Land[%d,%d] High", x, y);
	m_Land_Mesh_high = MeshManager::getSingleton().createManual(MeshName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	sprintf(TempName, "SubLand[%d,%d] High", x, y);
	m_Land_subMesh_high = m_Land_Mesh_high->createSubMesh(TempName);
	m_Land_subMesh_high->setMaterialName("Land_HighDetails");

	CreateLandHigh_Buffers();

	m_Land_Mesh_high->_setBounds( *m_bounds ); // Rendering is only done when Camera looks into this quad.
	m_Land_Mesh_high->load();
	sprintf( TempName, "Land[%d,%d] High Entity", m_posX, m_posY );
	m_Land_entity_high = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);
	// m_Land->attachObject( m_Land_entity_high );
	m_IsAttached = false;
}

//=================================================================================================
// Change Land in high Quality
//=================================================================================================
void CChunk::ChangeLandHigh()
{
	delete m_Land_subMesh_high->vertexData;
	CreateLandHigh_Buffers();
}

//=================================================================================================
//
//=================================================================================================
void CChunk::CreateLandHigh_Buffers()
{
	int x = m_posX * CHUNK_SIZE_X;
	int y = m_posY * CHUNK_SIZE_Y;

	/////////////////////////////////////////////////////////////////////////
	// Count the Vertices in this chunk.
	/////////////////////////////////////////////////////////////////////////
	unsigned int numVertices = 0;
	for (int a = x; a < x + CHUNK_SIZE_X; ++a)
	{
		for (int b = y; b < y + CHUNK_SIZE_Y; ++b)
		{
			if (m_TileManagerPtr->Get_Map_Height(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b  ) > LEVEL_WATER_TOP ||
					m_TileManagerPtr->Get_Map_Height(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map_Height(a+1, b+1) > LEVEL_WATER_TOP)
			{
				numVertices += 12;
			}
		}
	}
	if (numVertices == 0) { Create_Dummy(m_Land_subMesh_high); return; }

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

	long z = 0;
	Real g, h, d, f, row, col;
	Real average, StretchZ = m_TileManagerPtr->Get_StretchZ();
	Real* pReal1 = static_cast<Real*>(vbuf0->lock(HardwareBuffer::HBL_NORMAL));
	for (short a = x; a < x+ CHUNK_SIZE_X; ++a)
	{
		for (short b = y; b < y +CHUNK_SIZE_Y; ++b)
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
				pReal1[z   ] = TILE_SIZE * (a-x   - HALF_CHUNK_X);
				pReal1[z+ 1] = d;
				pReal1[z+ 2] = TILE_SIZE * (b-y+1 - HALF_CHUNK_Y);

				pReal1[z+12] = TILE_SIZE * (a-x  - HALF_CHUNK_X);
				pReal1[z+13] = g;
				pReal1[z+14] = TILE_SIZE * (b-y  - HALF_CHUNK_Y);

				pReal1[z+24] = TILE_SIZE * (a-x+.5 - HALF_CHUNK_X);
				pReal1[z+25] = average;
				pReal1[z+26] = TILE_SIZE * (b-y+.5 - HALF_CHUNK_Y);

				// 2. Triangle
				pReal1[z+36] = TILE_SIZE * (a-x - HALF_CHUNK_X);
				pReal1[z+37] = g;
				pReal1[z+38] = TILE_SIZE * (b-y - HALF_CHUNK_Y);

				pReal1[z+48] = TILE_SIZE * (a-x+1- HALF_CHUNK_X);
				pReal1[z+49] = h;
				pReal1[z+50] = TILE_SIZE * (b-y  - HALF_CHUNK_Y);

				pReal1[z+60] = TILE_SIZE * (a-x +.5- HALF_CHUNK_X);
				pReal1[z+61] = average;
				pReal1[z+62] = TILE_SIZE * (b-y +.5- HALF_CHUNK_Y);

				// 3. Triangle
				pReal1[z+72] = TILE_SIZE * (a-x+1- HALF_CHUNK_X);
				pReal1[z+73] = h;
				pReal1[z+74] = TILE_SIZE * (b-y  - HALF_CHUNK_Y);

				pReal1[z+84] = TILE_SIZE * (a-x +1- HALF_CHUNK_X);
				pReal1[z+85] = f;
				pReal1[z+86] = TILE_SIZE * (b-y +1- HALF_CHUNK_Y);

				pReal1[z+96] = TILE_SIZE * (a-x+.5 - HALF_CHUNK_X);
				pReal1[z+97] = average;
				pReal1[z+98] = TILE_SIZE * (b-y+.5 - HALF_CHUNK_Y);

				// 4. Triangle
				pReal1[z+108] = TILE_SIZE * (a-x +1- HALF_CHUNK_X);
				pReal1[z+109] = f;
				pReal1[z+110] = TILE_SIZE * (b-y +1- HALF_CHUNK_Y);

				pReal1[z+120] = TILE_SIZE * (a-x   - HALF_CHUNK_X);
				pReal1[z+121] = d;
				pReal1[z+122] = TILE_SIZE * (b-y +1- HALF_CHUNK_Y);

				pReal1[z+132] = TILE_SIZE * (a-x+.5- HALF_CHUNK_X);
				pReal1[z+133] = average;
				pReal1[z+134] = TILE_SIZE * (b-y+.5- HALF_CHUNK_Y);

				/////////////////////////////////////////////////////////////////////////
				// Normalvektoren
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal1[z+  3] = 0.0;	pReal1[z+  4] = 1.0;	pReal1[z+  5] = 0.0;
				pReal1[z+ 15] = 0.0;	pReal1[z+ 16] = 1.0;	pReal1[z+ 17] = 0.0;
				pReal1[z+ 27] = 0.0;	pReal1[z+ 28] = 1.0;	pReal1[z+ 29] = 0.0;
				// 2. Triangle
				pReal1[z+ 39] = 0.0;	pReal1[z+ 40] = 1.0;	pReal1[z+ 41] = 0.0;
				pReal1[z+ 51] = 0.0;	pReal1[z+ 52] = 1.0;	pReal1[z+ 53] = 0.0;
				pReal1[z+ 63] = 0.0;	pReal1[z+ 64] = 1.0;	pReal1[z+ 65] = 0.0;
				// 3. Triangle
				pReal1[z+ 75] = 0.0;	pReal1[z+ 76] = 1.0;	pReal1[z+ 77] = 0.0;
				pReal1[z+ 87] = 0.0;	pReal1[z+ 88] = 1.0;	pReal1[z+ 89] = 0.0;
				pReal1[z+ 99] = 0.0;	pReal1[z+100] = 1.0;	pReal1[z+101] = 0.0;
				// 4. Triangle
				pReal1[z+111] = 0.0;	pReal1[z+112] = 1.0;	pReal1[z+113] = 0.0;
				pReal1[z+123] = 0.0;	pReal1[z+124] = 1.0;	pReal1[z+125] = 0.0;
				pReal1[z+135] = 0.0;	pReal1[z+136] = 1.0;	pReal1[z+137] = 0.0;
				/////////////////////////////////////////////////////////////////////////
				// Ground-Texture.
				/////////////////////////////////////////////////////////////////////////
				col = (1.0 / 8.0 + 2 * 1.0 / 128.0) * m_TileManagerPtr->Get_Map_Texture_Col(a,b)+ 1.0 / 128.0;
				row = (1.0 / 8.0 + 2 * 1.0 / 128.0) * m_TileManagerPtr->Get_Map_Texture_Row(a,b)+ 1.0 / 128.0;
								// 1. Triangle
				pReal1[z+  6] = col;
				pReal1[z+  7] = row;
				pReal1[z+ 18] = col;
				pReal1[z+ 19] = row + 1.0 / 8.0;
				pReal1[z+ 30] = col + 1.0 / 16.0;
				pReal1[z+ 31] = row + 1.0 / 16.0;
				// 2. Triangle
				pReal1[z+ 42] = col;
				pReal1[z+ 43] = row + 1.0 / 8.0;
				pReal1[z+ 54] = col + 1.0 / 8.0;
				pReal1[z+ 55] = row + 1.0 / 8.0;
				pReal1[z+ 66] = col + 1.0 / 16.0;
				pReal1[z+ 67] = row + 1.0 / 16.0;
				// 3. Triangle
				pReal1[z+ 78] = col + 1.0 / 8.0;
				pReal1[z+ 79] = row + 1.0 / 8.0;
				pReal1[z+ 90] = col + 1.0 / 8.0;
				pReal1[z+ 91] = row;
				pReal1[z+102] = col + 1.0 / 16.0;
				pReal1[z+103] = row + 1.0 / 16.0;
				// 4. Triangle
				pReal1[z+114] = col + 1.0 / 8.0;
				pReal1[z+115] = row;
				pReal1[z+126] = col;
				pReal1[z+127] = row;
				pReal1[z+138] = col + 1.0 / 16.0;
				pReal1[z+139] = row + 1.0 / 16.0;
				/////////////////////////////////////////////////////////////////////////
				// Filter-Texture.
				/////////////////////////////////////////////////////////////////////////
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
				pReal1[z+ 8] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
				pReal1[z+ 9] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
				pReal1[z+20] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
				pReal1[z+21] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
				pReal1[z+32] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 16.0;
				pReal1[z+33] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 16.0;
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
				pReal1[z+44] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
				pReal1[z+45] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
				pReal1[z+56] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
				pReal1[z+57] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
				pReal1[z+68] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 16.0;
				pReal1[z+69] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 16.0;
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
				pReal1[z+80] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
				pReal1[z+81] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 8.0;
				pReal1[z+92] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
				pReal1[z+93] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
				pReal1[z+104] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 16.0;
				pReal1[z+105] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 16.0;
				// 4. Triangle
				if ( b != TILES_SUM_Y -1)
				{
				col = m_TileManagerPtr->Get_Map_Texture_Col(a,b+1);
				row = m_TileManagerPtr->Get_Map_Texture_Row(a,b+1);
				}
				else
				{
					col = 0;
					row = 0;
				}
				pReal1[z+116] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 8.0;
				pReal1[z+117] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
				pReal1[z+128] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0;
				pReal1[z+129] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0;
				pReal1[z+140] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * col + 1.0 / 128.0 + 1.0 / 16.0;
				pReal1[z+141] = (1.0 / 8.0 + 2 * 1.0 / 128.0) * row + 1.0 / 128.0 + 1.0 / 16.0;
				/////////////////////////////////////////////////////////////////////////
				// Grid-Texture.
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal1[z+ 10] = 0.0;	pReal1[z+ 11] = 1.0;	pReal1[z+ 22] = 0.0;
				pReal1[z+ 23] = 1.0;	pReal1[z+ 34] = 0.5;	pReal1[z+ 35] = 0.5;
				// 2. Triangle
				pReal1[z+ 46] = 0.0;	pReal1[z+ 47] = 1.0;	pReal1[z+ 58] = 1.0;
				pReal1[z+ 59] = 1.0;	pReal1[z+ 70] = 0.5;	pReal1[z+ 71] = 0.5;
				// 3. Triangle
				pReal1[z+ 82] = 1.0;	pReal1[z+ 83] = 1.0;	pReal1[z+ 94] = 1.0;
				pReal1[z+ 95] = 0.0;	pReal1[z+106] = 0.5;	pReal1[z+107] = 0.5;
				// 4. Triangle
				pReal1[z+118] = 1.0;	pReal1[z+119] = 0.0;	pReal1[z+130] = 0.0;
				pReal1[z+131] = 0.0;	pReal1[z+142] = 0.5;	pReal1[z+143] = 0.5;

				z += 144;
			} // if
		} // y
	} // x

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

//=================================================================================================
// Creates a dummy submesh containing only 1 Triangle.
//=================================================================================================
void CChunk::Create_Dummy(SubMesh* submesh)
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
