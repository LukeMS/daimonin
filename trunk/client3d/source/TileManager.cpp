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

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <time.h>
#include "TileManager.h"
#include "tile.h"
#include "logfile.h"
#include "EnvironmentManager.h"

//#define LOW_QUALITY_RENDERING

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
	m_ibuf.setNull();
	m_vbuf0.setNull();
	m_Water_Mesh_high.setNull();
	m_Water_Mesh_low.setNull();
	m_Land_Mesh_high.setNull();
	m_Land_Mesh_low.setNull();
	m_Kartentextur.setNull();
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
	int x = m_posX * CHUNK_SUM_X;
	int y = m_posY * CHUNK_SUM_Y;
	float StretchZ = m_TileManagerPtr->Get_StretchZ();
	unsigned long numVertices = 0;

	/////////////////////////////////////////////////////////////////////////
	// Bestimmung der Anzahl der Geometriepunkte
	/////////////////////////////////////////////////////////////////////////
	for (int a = x; a < x + CHUNK_SUM_X; a += 2)
	{
		for (int b = y; b < y + CHUNK_SUM_Y; b += 2)
		{
			if (m_TileManagerPtr->Get_Map(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b  ) > LEVEL_WATER_TOP ||
					m_TileManagerPtr->Get_Map(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b+1) > LEVEL_WATER_TOP)
			{
				numVertices += 6;
			}
		}
	}
	if (numVertices == 0) { return; }

	m_vdata = new VertexData();
	sprintf( MeshName, "Land[%d,%d] Low", x, y );
	m_Land_Mesh_low = MeshManager::getSingleton().createManual( MeshName,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	sprintf( TempName, "SubLand[%d,%d] Low", x, y );
	m_Land_subMesh_low = m_Land_Mesh_low->createSubMesh(TempName);

	m_idata = m_Land_subMesh_low->indexData;
	m_vdata->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent

	VertexDeclaration* vdec = m_vdata->vertexDeclaration;
	size_t offset = 0;
	vdec->addElement( 0, offset, VET_FLOAT3, VES_POSITION );
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	vdec->addElement( 0, offset, VET_FLOAT3, VES_NORMAL );
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
	offset += VertexElement::getTypeSize(VET_FLOAT2);
	vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1);  // Gittertextur
	offset += VertexElement::getTypeSize(VET_FLOAT2);

	m_vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
							offset, // size of one whole vertex
							numVertices, // number of vertices
							HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
							false // no shadow buffer
						);

	VertexBufferBinding* vbbind = m_vdata->vertexBufferBinding;
	vbbind->setBinding(0, m_vbuf0);

	Real* pReal1 = static_cast<Real*>(m_vbuf0->lock(HardwareBuffer::HBL_NORMAL))
								 ;

	long z = -60;
	long g,h,d,f;
	short row, col;
	for (int a = x; a < x+ CHUNK_SUM_X; a = a + 2)
			{
			for (int b = y; b < y +CHUNK_SUM_Y; b = b + 2)
					{

					if (m_TileManagerPtr->Get_Map(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b  ) > LEVEL_WATER_TOP ||
									m_TileManagerPtr->Get_Map(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b+1) > LEVEL_WATER_TOP)


							{
							z += 60;
							g = m_TileManagerPtr->Get_Map(a  ,b  ); //if (g < LEVEL_WATER_TOP) g = LEVEL_WATER_TOP;
							h = m_TileManagerPtr->Get_Map(a+2,b  ); //if (h < LEVEL_WATER_TOP) h = LEVEL_WATER_TOP;
							d = m_TileManagerPtr->Get_Map(a  ,b+2); //if (d < LEVEL_WATER_TOP) d = LEVEL_WATER_TOP;
							f = m_TileManagerPtr->Get_Map(a+2,b+2); //if (f < LEVEL_WATER_TOP) f = LEVEL_WATER_TOP;
							/////////////////////////////////////////////////////////////////////////
							// Position.
							/////////////////////////////////////////////////////////////////////////
							// 1. Triangle
							pReal1[z   ] = TILE_SIZE * (a-x - HALF_CHUNK_SIZE);
							pReal1[z+ 1] = TILE_SIZE * (b-y - HALF_CHUNK_SIZE);
							pReal1[z+ 2] = g * StretchZ;
							pReal1[z+10] = TILE_SIZE * (a-x+2- HALF_CHUNK_SIZE);
							pReal1[z+11] = TILE_SIZE * (b-y  - HALF_CHUNK_SIZE);
							pReal1[z+12] = h * StretchZ;
							pReal1[z+20] = TILE_SIZE * (a-x  - HALF_CHUNK_SIZE);
							pReal1[z+21] = TILE_SIZE * (b-y+2- HALF_CHUNK_SIZE);
							pReal1[z+22] = d * StretchZ;
							// 2. Triangle
							pReal1[z+30] = TILE_SIZE * (a-x-   HALF_CHUNK_SIZE);
							pReal1[z+31] = TILE_SIZE * (b-y+2- HALF_CHUNK_SIZE);
							pReal1[z+32] = d * StretchZ;
							pReal1[z+40] = TILE_SIZE * (a-x+2- HALF_CHUNK_SIZE);
							pReal1[z+41] = TILE_SIZE * (b-y-   HALF_CHUNK_SIZE);
							pReal1[z+42] = h * StretchZ;
							pReal1[z+50] = TILE_SIZE * (a-x+2- HALF_CHUNK_SIZE);
							pReal1[z+51] = TILE_SIZE * (b-y+2- HALF_CHUNK_SIZE);
							pReal1[z+52] = f * StretchZ;
							/////////////////////////////////////////////////////////////////////////
							// Normals.
							/////////////////////////////////////////////////////////////////////////
							// 1. Triangle
							pReal1[z+ 3] = 0;
							pReal1[z+ 4] = 0;
							pReal1[z+ 5] = 1;
							pReal1[z+13] = 0;
							pReal1[z+14] = 0;
							pReal1[z+15] = 1;
							pReal1[z+23] = 0;
							pReal1[z+24] = 0;
							pReal1[z+25] = 1;
							// 2. Triangle
							pReal1[z+33] = 0;
							pReal1[z+34] = 0;
							pReal1[z+35] = 1;
							pReal1[z+43] = 0;
							pReal1[z+44] = 0;
							pReal1[z+45] = 1;
							pReal1[z+53] = 0;
							pReal1[z+54] = 0;
							pReal1[z+55] = 1;
							/////////////////////////////////////////////////////////////////////////
							// Textures.
							/////////////////////////////////////////////////////////////////////////
							col = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a,b)->Get_terrain();
							row = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a,b)->Get_terrain_textur();
							// 1. triangle
							pReal1[z+ 6] = (col  ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+ 7] = (row  ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+16] = (col+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+17] = (row  ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+26] = (col  ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+27] = (row+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							// 2. Triangle
							pReal1[z+36] = (col  ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+37] = (row+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+46] = (col+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+47] = (row  ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+56] = (col+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
							pReal1[z+57] = (row+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
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
							} // if
					} // y
			} // x
	m_vbuf0->unlock();

	m_ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
						 HardwareIndexBuffer::IT_16BIT, // type of index
						 numVertices, // number of indexes
						 HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
						 false // no shadow buffer
					 );
	m_idata->indexBuffer = m_ibuf;
	m_idata->indexStart = 0;
	m_idata->indexCount = numVertices;

	unsigned short* pIdx = static_cast<unsigned short*>(m_ibuf->lock(HardwareBuffer::HBL_DISCARD))
												 ;
	for (unsigned short a = 0; a < (unsigned short)numVertices; ++a)
			{
			pIdx[a] = a;
			}
	m_ibuf->unlock();

	m_Land_subMesh_low->operationType = RenderOperation::OT_TRIANGLE_LIST;
	m_Land_subMesh_low->useSharedVertices = false;
	m_Land_subMesh_low->vertexData = m_vdata;

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
	int x = m_posX * CHUNK_SUM_X;
	int y = m_posY * CHUNK_SUM_Y;
	/////////////////////////////////////////////////////////////////////////
	// Count the Vertices in this chunk.
	/////////////////////////////////////////////////////////////////////////
	unsigned long numVertices = 0;
	for (int a = x; a < x + CHUNK_SUM_X; ++a)
	{
		for (int b = y; b < y + CHUNK_SUM_Y; ++b)
		{
			if (m_TileManagerPtr->Get_Map(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b  ) > LEVEL_WATER_TOP ||
					m_TileManagerPtr->Get_Map(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b+1) > LEVEL_WATER_TOP)
			{
				numVertices += 12;
			}
		}
	}
	if (numVertices == 0) { return; }
	/////////////////////////////////////////////////////////////////////////
	// Create Mesh with a SubMesh.
	/////////////////////////////////////////////////////////////////////////
	sprintf(MeshName, "Land[%d,%d] High", x, y);
	m_Land_Mesh_high = MeshManager::getSingleton().createManual(MeshName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	sprintf(TempName, "SubLand[%d,%d] High", x, y);
	m_Land_subMesh_high = m_Land_Mesh_high->createSubMesh(TempName);
	/////////////////////////////////////////////////////////////////////////
	// Create VertexData.
	/////////////////////////////////////////////////////////////////////////
	m_vdata = new VertexData();
	m_vdata->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent
	VertexDeclaration* vdec = m_vdata->vertexDeclaration;
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

	m_vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
		offset, // size of one whole vertex
		numVertices, // number of vertices
		HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
		false); // no shadow buffer

	VertexBufferBinding* vbbind = m_vdata->vertexBufferBinding;
	vbbind->setBinding(0, m_vbuf0);

	long z = 0;
	short g, h, d, f, row, col;
	Real average, StretchZ = m_TileManagerPtr->Get_StretchZ();
	Real* pReal1 = static_cast<Real*>(m_vbuf0->lock(HardwareBuffer::HBL_NORMAL));
	for (short a = x; a < x+ CHUNK_SUM_X; ++a)
	{
		for (short b = y; b < y +CHUNK_SUM_Y; ++b)
		{
			if (m_TileManagerPtr->Get_Map(a, b  ) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b  ) > LEVEL_WATER_TOP ||
					m_TileManagerPtr->Get_Map(a, b+1) > LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b+1) > LEVEL_WATER_TOP)
			{
				g = m_TileManagerPtr->Get_Map(a  , b  );  //if (g < LEVEL_WATER_TOP) g = LEVEL_WATER_TOP;
				h = m_TileManagerPtr->Get_Map(a+1, b  );  //if (h < LEVEL_WATER_TOP) h = LEVEL_WATER_TOP;
				d = m_TileManagerPtr->Get_Map(a  , b+1);  //if (d < LEVEL_WATER_TOP) d = LEVEL_WATER_TOP;
				f = m_TileManagerPtr->Get_Map(a+1, b+1);  //if (f < LEVEL_WATER_TOP) f = LEVEL_WATER_TOP;
				average = ( g + h + d + f ) / 4.0;
				/////////////////////////////////////////////////////////////////////////
				// Position.
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal1[z   ] = TILE_SIZE * (a-x   - HALF_CHUNK_SIZE);
				pReal1[z+1 ] = TILE_SIZE * (b-y+1 - HALF_CHUNK_SIZE);
				pReal1[z+2 ] = d * StretchZ;
				pReal1[z+12] = TILE_SIZE * (a-x  - HALF_CHUNK_SIZE);
				pReal1[z+13] = TILE_SIZE * (b-y  - HALF_CHUNK_SIZE);
				pReal1[z+14] = g * StretchZ;
				pReal1[z+24] = TILE_SIZE * (a-x+.5 - HALF_CHUNK_SIZE);
				pReal1[z+25] = TILE_SIZE * (b-y+.5 - HALF_CHUNK_SIZE);
				pReal1[z+26] = average * StretchZ;
				// 2. Triangle
				pReal1[z+36] = TILE_SIZE * (a-x - HALF_CHUNK_SIZE);
				pReal1[z+37] = TILE_SIZE * (b-y - HALF_CHUNK_SIZE);
				pReal1[z+38] = g * StretchZ;
				pReal1[z+48] = TILE_SIZE * (a-x+1- HALF_CHUNK_SIZE);
				pReal1[z+49] = TILE_SIZE * (b-y  - HALF_CHUNK_SIZE);
				pReal1[z+50] = h * StretchZ;
				pReal1[z+60] = TILE_SIZE * (a-x +.5- HALF_CHUNK_SIZE);
				pReal1[z+61] = TILE_SIZE * (b-y +.5- HALF_CHUNK_SIZE);
				pReal1[z+62] = average * StretchZ;
				// 3. Triangle
				pReal1[z+72] = TILE_SIZE * (a-x+1- HALF_CHUNK_SIZE);
				pReal1[z+73] = TILE_SIZE * (b-y  - HALF_CHUNK_SIZE);
				pReal1[z+74] = h* StretchZ;
				pReal1[z+84] = TILE_SIZE * (a-x +1- HALF_CHUNK_SIZE);
				pReal1[z+85] = TILE_SIZE * (b-y +1- HALF_CHUNK_SIZE);
				pReal1[z+86] = f * StretchZ;
				pReal1[z+96] = TILE_SIZE * (a-x+.5 - HALF_CHUNK_SIZE);
				pReal1[z+97] = TILE_SIZE * (b-y+.5 - HALF_CHUNK_SIZE);
				pReal1[z+98] = average * StretchZ;
				// 4. Triangle
				pReal1[z+108] = TILE_SIZE * (a-x +1- HALF_CHUNK_SIZE);
				pReal1[z+109] = TILE_SIZE * (b-y +1- HALF_CHUNK_SIZE);
				pReal1[z+110] = f* StretchZ;
				pReal1[z+120] = TILE_SIZE * (a-x   - HALF_CHUNK_SIZE);
				pReal1[z+121] = TILE_SIZE * (b-y +1- HALF_CHUNK_SIZE);
				pReal1[z+122] = d * StretchZ;
				pReal1[z+132] = TILE_SIZE * (a-x+.5- HALF_CHUNK_SIZE);
				pReal1[z+133] = TILE_SIZE * (b-y+.5- HALF_CHUNK_SIZE);
				pReal1[z+134] = average * StretchZ;
				/////////////////////////////////////////////////////////////////////////
				// Normalvektoren
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal1[z+  3] = 0.0;	pReal1[z+  4] = 0.0;	pReal1[z+  5] = 1.0;
				pReal1[z+ 15] = 0.0;	pReal1[z+ 16] = 0.0;	pReal1[z+ 17] = 1.0;
				pReal1[z+ 27] = 0.0;	pReal1[z+ 28] = 0.0;	pReal1[z+ 29] = 1.0;
				// 2. Triangle
				pReal1[z+ 39] = 0.0;	pReal1[z+ 40] = 0.0;	pReal1[z+ 41] = 1.0;
				pReal1[z+ 51] = 0.0;	pReal1[z+ 52] = 0.0;	pReal1[z+ 53] = 1.0;
				pReal1[z+ 63] = 0.0;	pReal1[z+ 64] = 0.0;	pReal1[z+ 65] = 1.0;
				// 3. Triangle
				pReal1[z+ 75] = 0.0;	pReal1[z+ 76] = 0.0;	pReal1[z+ 77] = 1.0;
				pReal1[z+ 87] = 0.0;	pReal1[z+ 88] = 0.0;	pReal1[z+ 89] = 1.0;
				pReal1[z+ 99] = 0.0;	pReal1[z+100] = 0.0;	pReal1[z+101] = 1.0;
				// 4. Triangle
				pReal1[z+111] = 0.0;	pReal1[z+112] = 0.0;	pReal1[z+113] = 1.0;
				pReal1[z+123] = 0.0;	pReal1[z+124] = 0.0;	pReal1[z+125] = 1.0;
				pReal1[z+135] = 0.0;	pReal1[z+136] = 0.0;	pReal1[z+137] = 1.0;
				/////////////////////////////////////////////////////////////////////////
				// Ground-Texture.
				/////////////////////////////////////////////////////////////////////////
				col = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a,b)->Get_terrain();
				row = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a,b)->Get_terrain_textur();
				// 1. Triangle
				pReal1[z+  6] = (col    ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+  7] = (row    ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 18] = (col    ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 19] = (row + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 30] = (col +.5) / 7.0;
				pReal1[z+ 31] = (row +.5) / 7.0;
				// 2. Triangle
				pReal1[z+ 42] = (col    ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 43] = (row + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 54] = (col + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 55] = (row + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 66] = (col +.5) / 7.0;
				pReal1[z+ 67] = (row +.5) / 7.0;
				// 3. Triangle
				pReal1[z+ 78] = (col + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 79] = (row + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 90] = (col + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 91] = (row    ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+102] = (col +.5) / 7.0;
				pReal1[z+103] = (row +.5) / 7.0;
				// 4. Triangle
				pReal1[z+114] = (col + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+115] = (row    ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+126] = (col    ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+127] = (row    ) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+138] = (col +.5) / 7.0;
				pReal1[z+139] = (row +.5) / 7.0;
				/////////////////////////////////////////////////////////////////////////
				// Filter-Texture.
				/////////////////////////////////////////////////////////////////////////
				if (a)
				{
					col = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a-1,b)->Get_terrain();
					row = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a-1,b)->Get_terrain_textur();
				}
				else
				{
					col = 0;
					row = 0;
				}
				// 1. Triangle
				pReal1[z+ 8] = col / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+ 9] = row / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+20] = col / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+21] = (row + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+32] = (col + .5) / 7.0;
				pReal1[z+33] = (row + .5)/ 7.0;
				// 2. Triangle
				if (b)
				{
					col = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a,b-1)->Get_terrain();
					row = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a,b-1)->Get_terrain_textur();
				}
				else
				{
					col = 0;
					row = 0;
				}
				pReal1[z+44] = col / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+45] = (row+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+56] = (col + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+57] = (row + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+68] = (col + .5) / 7.0;
				pReal1[z+69] = (row + .5)/ 7.0;
				// 3. Triangle
				if ( a != TILES_MAX_X - 1)
				{
					col = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a+1,b)->Get_terrain();
					row = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a+1,b)->Get_terrain_textur();
				}
				else
				{
					col = 0;
					row = 0;
				}
				pReal1[z+80] = (col+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+81] = (row+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+92] = (col+1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+93] = (row) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+104] = (col + .5) / 7.0;
				pReal1[z+105] = (row + .5)/ 7.0;
				// 4. Triangle
				if ( b != TILES_MAX_Y -1)
				{
					col = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a,b+1)->Get_terrain();
					row = m_TileManagerPtr->Get_pworldmap()->Get_ptile(a,b+1)->Get_terrain_textur();
				}
				else
				{
					col = 0;
					row = 0;
				}
				pReal1[z+116] = (col + 1) / 7.0 - 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+117] = row / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+128] = col / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+129] = (row) / 7.0 + 9.0 / (PIXEL_PER_ROW - PIXEL_PER_TILE);
				pReal1[z+140] = (col + .5) / 7.0;
				pReal1[z+141] = (row + .5)/ 7.0;
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
	m_vbuf0->unlock();
	/////////////////////////////////////////////////////////////////////////
	// Create Index-buffer
	/////////////////////////////////////////////////////////////////////////
	m_ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(
		 HardwareIndexBuffer::IT_16BIT, // type of index
		 numVertices, // number of indexes
		 HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
		 false); // no shadow buffer
	m_idata = m_Land_subMesh_high->indexData;
	m_idata->indexBuffer = m_ibuf;
	m_idata->indexStart = 0;
	m_idata->indexCount = numVertices;
	unsigned short* pIdx = static_cast<unsigned short*>(m_ibuf->lock(HardwareBuffer::HBL_DISCARD));
	for (unsigned short a = 0; a < (unsigned short)numVertices; ++a) { pIdx[a] = a; }
	m_ibuf->unlock();

	m_Land_subMesh_high->operationType = RenderOperation::OT_TRIANGLE_LIST;
	m_Land_subMesh_high->useSharedVertices = false;
	m_Land_subMesh_high->vertexData = m_vdata;
	m_Land_subMesh_high->setMaterialName("Land_HighDetails");

	m_Land_Mesh_high->_setBounds( *m_bounds ); // Rendering is only done when Camera looks into this quad.
	m_Land_Mesh_high->load();

	sprintf( TempName, "Land[%d,%d] High Entity", m_posX, m_posY );
	m_Land_entity_high = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);
	// m_Land->attachObject( m_Land_entity_high );
	m_IsAttached = false;
	}

//=================================================================================================
// Create Water in low Quality
//=================================================================================================
void CChunk::CreateWaterLow()
	{
	long x = m_posX * CHUNK_SUM_X;
	long y = m_posY * CHUNK_SUM_Y;
	float StretchZ = m_TileManagerPtr->Get_StretchZ();
	unsigned long numVertices = 0;

	// Bestimmung der Anzahl der Geometriepunkte

	for (short a = x; a < x + CHUNK_SUM_X; ++a)
			{
			for (short b = y; b < y + CHUNK_SUM_Y; ++b)
					{
					if (m_TileManagerPtr->Get_Map(a, b  ) <= LEVEL_WATER_TOP && m_TileManagerPtr->Get_Map(a+1, b  ) <= LEVEL_WATER_TOP &&
							m_TileManagerPtr->Get_Map(a, b+1) <= LEVEL_WATER_TOP && m_TileManagerPtr->Get_Map(a+1, b+1) <= LEVEL_WATER_TOP)
							{
							numVertices += 6;
							} // endif
					} // b
			} // a

	if (numVertices == 0)
		return;

	VertexData* m_vdata = new VertexData();
	IndexData* m_idata;

	sprintf( MeshName, "Water[%d,%d] Low", x, y );
	m_Water_Mesh_low = MeshManager::getSingleton().createManual( MeshName,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	sprintf( TempName, "SubWater[%d,%d] Low", x, y );
	m_Water_subMesh_low = m_Water_Mesh_low->createSubMesh(TempName);
	m_idata = m_Water_subMesh_low->indexData;

	numVertices = 6;

	m_vdata->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent

	VertexDeclaration* vdec = m_vdata->vertexDeclaration;

	size_t offset = 0;
	vdec->addElement( 0, offset, VET_FLOAT3, VES_POSITION );
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	vdec->addElement( 0, offset, VET_FLOAT3, VES_NORMAL );
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
	offset += VertexElement::getTypeSize(VET_FLOAT2);
	vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1);
	offset += VertexElement::getTypeSize(VET_FLOAT2);

	m_vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
							offset, // size of one whole vertex
							numVertices, // number of vertices
							HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
							false); // no shadow buffer

	VertexBufferBinding* vbbind = m_vdata->vertexBufferBinding;
	vbbind->setBinding(0, m_vbuf0);

	Real* pReal;
	pReal = static_cast<Real*>(m_vbuf0->lock(HardwareBuffer::HBL_NORMAL))
					;
	long z = -60;
	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[0] = - (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[1] = - (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[2] =  LEVEL_WATER_TOP * StretchZ;

	pReal[3] = 0;
	pReal[4] = 0;
	pReal[5] = 1;

	pReal[6] =  0;
	pReal[7] =  0;

	pReal[8] = 0;
	pReal[9] = 0;

	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[10] = + (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[11] = - (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[12] = LEVEL_WATER_TOP * StretchZ;

	pReal[13] = 0;
	pReal[14] = 0;
	pReal[15] = 1;

	pReal[16] = TILES_PER_CHUNK /4;
	pReal[17] = 0;

	pReal[18] = 1;
	pReal[19] = 0;

	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[20] = - (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[21] = + (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[22] = LEVEL_WATER_TOP * StretchZ;

	pReal[23] = 0;
	pReal[24] = 0;
	pReal[25] = 1;

	pReal[26] = 0;
	pReal[27] = TILES_PER_CHUNK /4;

	pReal[28] = 0;
	pReal[29] = 1;


	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[30] = - (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[31] = + (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[32] = LEVEL_WATER_TOP * StretchZ;

	pReal[33] = 0;
	pReal[34] = 0;
	pReal[35] = 1;

	pReal[36] = 0;
	pReal[37] = TILES_PER_CHUNK /4;

	pReal[38] = 0;
	pReal[39] = 1;


	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[40] = + (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[41] = - (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[42] = LEVEL_WATER_TOP * StretchZ;

	pReal[43] = 0;
	pReal[44] = 0;
	pReal[45] = 1;

	pReal[46] = TILES_PER_CHUNK /4;
	pReal[47] = 0;

	pReal[48] = 1;
	pReal[49] = 0;


	/////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////
	pReal[50] = + (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[51] = + (TILE_SIZE * HALF_CHUNK_SIZE);
	pReal[52] = LEVEL_WATER_TOP * StretchZ;

	pReal[53] = 0;
	pReal[54] = 0;
	pReal[55] = 1;

	pReal[56] = TILES_PER_CHUNK /4;
	pReal[57] = TILES_PER_CHUNK /4;

	pReal[58] = 1;
	pReal[59] = 1;

	m_vbuf0->unlock();

	m_ibuf = HardwareBufferManager::getSingleton().
					 createIndexBuffer(
						 HardwareIndexBuffer::IT_16BIT, // type of index
						 numVertices, // number of indexes
						 HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
						 false); // no shadow buffer

	m_idata->indexBuffer = m_ibuf;
	m_idata->indexStart = 0;
	m_idata->indexCount = numVertices;

	unsigned short* pIdx = static_cast<unsigned short*>(m_ibuf->lock(HardwareBuffer::HBL_DISCARD))
												 ;

	for (unsigned short a = 0; a < (unsigned short)numVertices; ++a)
		pIdx[a] = a;

	m_ibuf->unlock();

	m_Water_subMesh_low->operationType = RenderOperation::OT_TRIANGLE_LIST;
	m_Water_subMesh_low->useSharedVertices = false;
	m_Water_subMesh_low->vertexData = m_vdata;

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
	int x = m_posX * CHUNK_SUM_X;
	int y = m_posY * CHUNK_SUM_Y;
	/////////////////////////////////////////////////////////////////////////
	// Count the Vertices in this chunk.
	/////////////////////////////////////////////////////////////////////////
	unsigned long numVertices = 0;
	for (short a = x; a < x + CHUNK_SUM_X; ++a)
	{
		for (short b = y; b < y + CHUNK_SUM_Y; ++b)
		{
			if (m_TileManagerPtr->Get_Map(a, b  ) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b  ) <= LEVEL_WATER_TOP ||
					m_TileManagerPtr->Get_Map(a, b+1) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b+1) <= LEVEL_WATER_TOP)
			{
				numVertices += 6;
			}
		}
	}
	if (numVertices == 0) { return; }
	/////////////////////////////////////////////////////////////////////////
	// Create Mesh with a SubMesh.
	/////////////////////////////////////////////////////////////////////////
	sprintf( MeshName, "Water[%d,%d] High", x, y );
	m_Water_Mesh_high = MeshManager::getSingleton().createManual( MeshName,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	sprintf( TempName, "SubWater[%d,%d] High", x, y );
	m_Water_subMesh_high = m_Water_Mesh_high->createSubMesh(TempName);
	/////////////////////////////////////////////////////////////////////////
	// Create VertexData.
	/////////////////////////////////////////////////////////////////////////
	m_vdata = new VertexData();
	m_vdata->vertexCount = numVertices; // Wichtig! Anzahl der Punkte muss angegeben werden, sonst Objekt nicht existent
	VertexDeclaration* vdec = m_vdata->vertexDeclaration;
	size_t offset = 0;
	vdec->addElement( 0, offset, VET_FLOAT3, VES_POSITION );
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	vdec->addElement( 0, offset, VET_FLOAT3, VES_NORMAL );
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
	offset += VertexElement::getTypeSize(VET_FLOAT2);
	vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1);
	offset += VertexElement::getTypeSize(VET_FLOAT2);

	m_vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(
		offset, // size of one whole vertex
		numVertices, // number of vertices
		HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
		false); // no shadow buffer
	VertexBufferBinding* vbbind = m_vdata->vertexBufferBinding;
	vbbind->setBinding(0, m_vbuf0);

	long z = 0;
	Real StretchZ = m_TileManagerPtr->Get_StretchZ();
	Real* pReal = static_cast<Real*>(m_vbuf0->lock(HardwareBuffer::HBL_NORMAL));
	for (short a = x; a < x+ CHUNK_SUM_X; ++a)
	{
		for (short b = y; b < y +CHUNK_SUM_Y; ++b)
		{
			if (m_TileManagerPtr->Get_Map(a, b  ) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b  ) <= LEVEL_WATER_TOP ||
					m_TileManagerPtr->Get_Map(a, b+1) <= LEVEL_WATER_TOP || m_TileManagerPtr->Get_Map(a+1, b+1) <= LEVEL_WATER_TOP)
			{
				/////////////////////////////////////////////////////////////////////////
				// Position.
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal[z  ] = TILE_SIZE * (a-x - HALF_CHUNK_SIZE);
				pReal[z+1] = TILE_SIZE * (b-y- HALF_CHUNK_SIZE);
				pReal[z+2] =  LEVEL_WATER_CLP * StretchZ;
				pReal[z+10] = TILE_SIZE * ((a-x)+1- HALF_CHUNK_SIZE);
				pReal[z+11] = TILE_SIZE * (b-y- HALF_CHUNK_SIZE);
				pReal[z+12] = LEVEL_WATER_CLP* StretchZ;
				pReal[z+20] = TILE_SIZE * (a-x- HALF_CHUNK_SIZE);
				pReal[z+21] = TILE_SIZE * ((b-y)+1- HALF_CHUNK_SIZE);
				pReal[z+22] = LEVEL_WATER_CLP* StretchZ;
				// 2. Triangle
				pReal[z+30] = TILE_SIZE * (a-x- HALF_CHUNK_SIZE);
				pReal[z+31] = TILE_SIZE * ((b-y)+1- HALF_CHUNK_SIZE);
				pReal[z+32] = LEVEL_WATER_CLP* StretchZ;
				pReal[z+40] = TILE_SIZE * ((a-x)+1- HALF_CHUNK_SIZE);
				pReal[z+41] = TILE_SIZE * (b-y- HALF_CHUNK_SIZE);
				pReal[z+42] = LEVEL_WATER_CLP * StretchZ;
				pReal[z+50] = TILE_SIZE * ((a-x)+1- HALF_CHUNK_SIZE);
				pReal[z+51] = TILE_SIZE * ((b-y)+1- HALF_CHUNK_SIZE);
				pReal[z+52] = LEVEL_WATER_CLP * StretchZ;
				/////////////////////////////////////////////////////////////////////////
				// Normalvektoren
				/////////////////////////////////////////////////////////////////////////
				// 1. Triangle
				pReal[z+ 3] = 0;	pReal[z+ 4] = 0;	pReal[z+ 5] = 1;
				pReal[z+13] = 0;	pReal[z+14] = 0;	pReal[z+15] = 1;
				pReal[z+23] = 0;	pReal[z+24] = 0;	pReal[z+25] = 1;
				// 2. Triangle
				pReal[z+33] = 0;	pReal[z+34] = 0;	pReal[z+35] = 1;
				pReal[z+43] = 0;	pReal[z+44] = 0;	pReal[z+45] = 1;
				pReal[z+53] = 0;	pReal[z+54] = 0;	pReal[z+55] = 1;
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
	m_vbuf0->unlock();
	/////////////////////////////////////////////////////////////////////////
	// Create Index-buffer
	/////////////////////////////////////////////////////////////////////////
	m_ibuf = HardwareBufferManager::getSingleton().
		createIndexBuffer(
		HardwareIndexBuffer::IT_16BIT, // type of index
		numVertices, // number of indexes
		HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
		false); // no shadow buffer
	m_idata = m_Water_subMesh_high->indexData;
	m_idata->indexBuffer = m_ibuf;
	m_idata->indexStart = 0;
	m_idata->indexCount = numVertices;
	unsigned short* pIdx = static_cast<unsigned short*>(m_ibuf->lock(HardwareBuffer::HBL_DISCARD));
	for (unsigned short a = 0; a < (unsigned short)numVertices; ++a) { pIdx[a] = a; }
	m_ibuf->unlock();

	m_Water_subMesh_high->operationType = RenderOperation::OT_TRIANGLE_LIST;
	m_Water_subMesh_high->useSharedVertices = false;
	m_Water_subMesh_high->vertexData = m_vdata;
	m_Water_subMesh_high->setMaterialName("Water_HighDetails");

	m_Water_Mesh_high->_setBounds( *m_bounds ); // Rendering is only done when Camera looks into this quad.
	m_Water_Mesh_high->load();

	sprintf( TempName, "Water[%d,%d] High Entity", m_posX, m_posY );
	m_Water_entity_high = m_TileManagerPtr->Get_pSceneManager()->createEntity(TempName, MeshName);
	m_IsAttached = false;
}

//=================================================================================================
// Create Sctene-Nodes.
//=================================================================================================
void CChunk::CreateSceneNode()
{
	m_Land  = m_TileManagerPtr->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
	m_Land ->setPosition(m_posX * TILE_SIZE * CHUNK_SUM_X + TILE_SIZE * CHUNK_SUM_X / 2,
											 m_posY * TILE_SIZE * CHUNK_SUM_Y + TILE_SIZE * CHUNK_SUM_Y / 2, 0);
	m_Water = m_TileManagerPtr->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
	m_Water->setPosition(m_posX * TILE_SIZE * CHUNK_SUM_X + TILE_SIZE * CHUNK_SUM_X / 2,
											 m_posY * TILE_SIZE * CHUNK_SUM_Y + TILE_SIZE * CHUNK_SUM_Y / 2, 0);
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
// Create Chunk itself
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
// Create EnvironmentManager
//=================================================================================================
void CChunk::CreateEnvironmentManager()
{
	m_EnvironmentManagerPtr = new CEnvironmentManager(m_TileManagerPtr, this);
	m_EnvironmentManagerPtr->UpdateEnvironment();
}

//=================================================================================================
// Constructor
//=================================================================================================
CTileManager::CTileManager() {}

//=================================================================================================
// Destructor
//=================================================================================================
CTileManager::~CTileManager()
{
	for(int x = 0; x < TILES_MAX_X + 1; ++x) { delete[] m_Map[x]; }
	delete[] m_Map;
}


//=================================================================================================
//
//=================================================================================================
void CTileManager::Init(Cworldmap* worldmap, SceneManager* SceneMgr)
{
	m_worldmap = worldmap;
	m_SceneManager = SceneMgr;
	m_StretchZ = 2;
	srand(1);
	m_Map = new short*[TILES_MAX_X+1];
	for (int x = 0; x < TILES_MAX_X + 1; ++x) { m_Map[x] = new short[TILES_MAX_Y + 1]; }
	Create_Map();
	CreateTextureGroup("terrain"); // only used to create a new texture group.
	CreateChunks();
}

//=================================================================================================
//
//=================================================================================================
void CTileManager::Create_Map()
{
	for (int y = 0; y < TILES_MAX_Y+1; ++y)
	{
		for (int x = 0; x < TILES_MAX_X+1; ++x)
		{
			if ( x && x != TILES_MAX_X && y && y != TILES_MAX_Y)
			{
				m_Map[x][y] = m_worldmap->Get_ptile(x,y)->Get_height();
			}
			else
			{
				m_Map[x][y] = 0;
			}
		}
	}
}

//=================================================================================================
//
//=================================================================================================
void CTileManager::CreateChunks()
{
	CChunk::m_TileManagerPtr = this;
	CChunk::m_bounds = new AxisAlignedBox(
		 - TILE_SIZE * HALF_CHUNK_SIZE, -TILE_SIZE * HALF_CHUNK_SIZE, 0,
			 TILE_SIZE * HALF_CHUNK_SIZE,  TILE_SIZE * HALF_CHUNK_SIZE, 100 * m_StretchZ);

	long time = clock();
	for (short x = 0; x < CHUNK_SUM_X; ++x)
	{
		for (short y = 0; y < CHUNK_SUM_Y; ++y)
		{
			m_mapchunk[x][y].Create(x, y);
		}
	}
	delete CChunk::m_bounds;
	LogFile::getSingleton().Info("time: %d\n", clock()-time);
}

//=================================================================================================
// +/- 5 Chunks around the camera are drawn in high quality.
//=================================================================================================
void CTileManager::ControlChunks(Vector3 vector)
{
	int x = (int)vector.x / (TILE_SIZE * CHUNK_SUM_X)+1;
	int y = (int)vector.y / (TILE_SIZE * CHUNK_SUM_Y)+1;

	if ( x < CHUNK_SUM_X && y < CHUNK_SUM_Y)
	{
		for(int cx = 0; cx < CHUNK_SUM_X; ++cx)
		{
			for (int cy = 0; cy < CHUNK_SUM_Y; ++cy)
			{
#ifndef LOW_QUALITY_RENDERING
				if (cx >= x - HIGH_QUALITY_RANGE && cx <= x + HIGH_QUALITY_RANGE
				&& cy >= y - HIGH_QUALITY_RANGE && cy <= y + HIGH_QUALITY_RANGE)
				{
					m_mapchunk[cx][cy].Attach(QUALITY_HIGH);
				}
				else
#endif
				{
					m_mapchunk[cx][cy].Attach(QUALITY_LOW);
				}
			}
		}
	}
SwitchMaterial(true, false); // Testing - DELETE ME!
}

//=================================================================================================
// Create the texture-file out of the single textures + filter texture.
//=================================================================================================
void CTileManager::CreateTextureGroup(const char *terrain_type)
{
	const int PIXEL = (int)PIXEL_PER_TILE;
	const int TEXTURES_PER_ROW = 7; // Texture is a quad, so we have (x^2)-1 textures in the main-texture.
	const int SIZE = (int) PIXEL_PER_ROW;
	const int BUFFER_SIZE =80;
	char filename[BUFFER_SIZE+1];

	/////////////////////////////////////////////////////////////////////////
	// Create grid texture.
	/////////////////////////////////////////////////////////////////////////
	Image grid;
	uchar* grid_data = new uchar[PIXEL * PIXEL * 4];
	grid.loadDynamicImage(grid_data, PIXEL, PIXEL, PF_R8G8B8A8);

	for (int x = 0; x != PIXEL; ++x)
	{
		for (int y = 0; y != PIXEL; ++y)
		{
			if ( x == 0 || y == 0 || x == PIXEL - 1 || y == PIXEL -1)
			{
				grid_data[4*(PIXEL*y + x) + 0] = 255;
				grid_data[4*(PIXEL*y + x) + 1] = 50;
				grid_data[4*(PIXEL*y + x) + 2] = 50;
				grid_data[4*(PIXEL*y + x) + 3] = 50;
			}
			else
			{
				grid_data[4*(PIXEL*y + x) + 0] = 0;
				grid_data[4*(PIXEL*y + x) + 1] = 0;
				grid_data[4*(PIXEL*y + x) + 2] = 0;
				grid_data[4*(PIXEL*y + x) + 3] = 0;
			}
		}
	}
	sprintf(filename,"%s%s%.3d%s",PATH_TILE_TEXTURES, "grid_", PIXEL, ".png");
	grid.save(filename);

	/////////////////////////////////////////////////////////////////////////
	// Create a ground texture.
	/////////////////////////////////////////////////////////////////////////
	Image TextureGroup; // Will be created.
	uchar* TextureGroup_data = new uchar[SIZE * SIZE *4];
	TextureGroup.loadDynamicImage(TextureGroup_data, SIZE, SIZE,PF_A8B8G8R8);

	Image Filter, Texture; // Needed Textures.
	snprintf(filename, BUFFER_SIZE, "filter_%.3d.png", PIXEL);
	Filter.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	uchar* Filter_data = Filter.getData();

	int i=-1, x=0, y = 0;
	while(1)
	{
		// Check if the texture-file exists.
		sprintf(filename, "%s%s_%.2d_%.3d.png", PATH_TILE_TEXTURES, terrain_type, ++i, PIXEL);
		if (access(filename, 0) == -1) { break; }
		// Load the texture-file.
		sprintf(filename, "%s_%.2d_%.3d.png", terrain_type, i, PIXEL);
		Texture.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		addToGroupTexture(TextureGroup_data, Filter_data, &Texture, PIXEL, TEXTURES_PER_ROW, x, y);
		if (++x == TEXTURES_PER_ROW)
		{
			if (++y == TEXTURES_PER_ROW) { break; }
			x = 0;
		}
	}
	sprintf(filename,"%s%s%s",PATH_TILE_TEXTURES, terrain_type, "_texture.png");
	TextureGroup.save(filename);
}

//=================================================================================================
// Add a texture to the group-texture.
//=================================================================================================
inline void CTileManager::addToGroupTexture(uchar* TextureGroup_data, uchar *Filter_data, Image* Texture, short pix, short size, short x, short y)
{
	unsigned long index1;
	unsigned long index2;

	uchar* Texture_data = Texture->getData();
	for (int i = 0; i < pix; ++i)
	{
		for (int j = 0; j < pix; ++j)
		{
			index1 = (4* (pix + 18) * size + 8)* (pix + 18) * y + (4* (pix + 18) * size + 8) * (i+9) + 4* x * (pix + 18) + 4* (j + 9);
			index2 = 3* pix * i + 3 * j;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data [index2] / 2;
		}
	}
	/////////////////////////////////////////////////////////////////////////
	// left border creation
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != pix; ++i)
	{
		for (int j = 0; j!= 9; ++j)
		{
			index1 = (4* (pix + 18) * size + 8)* (pix + 18) * y + (4* (pix + 18) * size + 8) * (i+9) + 4* x * (pix + 18) + 4* j;
			index2 = 3* pix * i + 3 * (pix - 9 + j);
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}
	/////////////////////////////////////////////////////////////////////////
	// right border creation
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != pix; ++i)
	{
		for (int j = 0; j!= 9; ++j)
		{
			index1 = (4* (pix + 18) * size + 8)* (pix + 18) * y + (4* (pix + 18) * size + 8) * (i+9) + 4* x * (pix + 18) + 4* (pix + 9) + 4* j;
			index2 = 3* pix * i + 3 * j;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}
	/////////////////////////////////////////////////////////////////////////
	// upper border creation
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != 9; ++i)
	{
		for (int j = 0; j!= pix; ++j)
		{
			index1 = (4* (pix + 18) * size + 8)* (pix + 18) * y + (4* (pix + 18) * size + 8) * i + 4* x * (pix + 18) + 4* (j+9);
			index2 = 3* pix * (pix - 9 + i) + 3 * j;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}
	/////////////////////////////////////////////////////////////////////////
	// lower border creation
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != 9; ++i)
	{
		for (int j = 0; j!= pix; ++j)
		{
			index1 = (4* (pix + 18) * size + 8)* (pix + 18) * y + (4* (pix + 18) * size + 8) * (pix + 9 + i) + 4* x * (pix + 18) + 4* (j+9);
			index2 = 3* pix * i + 3 * j;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}
	/////////////////////////////////////////////////////////////////////////
	// remaining 4 edges
	/////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != 9; ++i)
	{
		for (int j = 0; j!= 9; ++j)
		{
			index1 = (4* (pix + 18) * size + 8)* (pix + 18) * y + (4* (pix + 18) * size + 8) * i + 4* x * (pix + 18) + 4* j;
			index2 = 0;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}

	for (int i = 0; i != 9; ++i)
	{
		for (int j = pix + 9; j!= pix + 18; ++j)
		{
			index1 = (4* (pix + 18) * size + 8)* (pix + 18) * y + (4* (pix + 18) * size + 8) * i + 4* x * (pix + 18) + 4* j;
			index2 = 0;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}

	for (int i = pix + 9; i !=  pix + 18; ++i)
	{
		for (int j = 0; j!= 9; ++j)
		{
			index1 = (4* (pix + 18) * size + 8)* (pix + 18) * y + (4* (pix + 18) * size + 8) * i + 4* x * (pix + 18) + 4* j;
			index2 = 0;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}

	for (int i = pix + 9; i !=  pix + 18; ++i)
	{
		for (int j = pix + 9; j!= pix + 18; ++j)
		{
			index1 = (4* (pix + 18) * size + 8)* (pix + 18) * y + (4* (pix + 18) * size + 8) * i + 4* x * (pix + 18) + 4* j;
			index2 = 0;
			TextureGroup_data[  index1] = Texture_data[index2];
			TextureGroup_data[++index1] = Texture_data[index2 + 1];
			TextureGroup_data[++index1] = Texture_data[index2 + 2];
			TextureGroup_data[++index1] = Filter_data[index2] / 2;
		}
	}
}

//=================================================================================================
//
//=================================================================================================
void CTileManager::SwitchMaterial(bool grid, bool filter)
{
	Entity *entity;
	/////////////////////////////////////////////////////////////////////////
	// Display Grid.
	/////////////////////////////////////////////////////////////////////////
	if (grid)
	{
		if (filter == true)
		{
			for (int x = 0; x < CHUNK_SUM_X; ++x)
			{
				for (int y = 0; y < CHUNK_SUM_Y; ++y)
				{
					entity = m_mapchunk[x][y].Get_Land_entity();
					if (entity) { entity->setMaterialName("Land_HighDetails_Grid"); }
					entity = m_mapchunk[x][y].Get_Water_entity();
					if (entity) { entity->setMaterialName("Water_HighDetails_Grid"); }
				}
			}
		}
		else
		{
			for (int x = 0; x < CHUNK_SUM_X; ++x)
			{
				for (int y = 0; y < CHUNK_SUM_Y; ++y)
				{
					entity = m_mapchunk[x][y].Get_Land_entity();
					if (entity) { entity->setMaterialName("Land_LowDetails_Grid"); }
					entity = m_mapchunk[x][y].Get_Water_entity();
					if (entity) { entity->setMaterialName("Water_LowDetails_Grid"); }
				}
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////
	// Don't display grid.
	/////////////////////////////////////////////////////////////////////////
	else
	{
		if (filter == true)
		{
			for (int x = 0; x < CHUNK_SUM_X; ++x)
			{
				for (int y = 0; y < CHUNK_SUM_Y; ++y)
				{
					entity = m_mapchunk[x][y].Get_Land_entity();
					if (entity) { entity->setMaterialName("Land_HighDetails"); }
					entity = m_mapchunk[x][y].Get_Water_entity();
					if (entity) { entity->setMaterialName("Water_HighDetails"); }
				}
			}
		}
		else
		{
			for (int x = 0; x < CHUNK_SUM_X; ++x)
			{
				for (int y = 0; y < CHUNK_SUM_Y; ++y)
				{
					entity = m_mapchunk[x][y].Get_Land_entity();
					if (entity) { entity->setMaterialName("Land_LowDetails"); }
					entity = m_mapchunk[x][y].Get_Water_entity();
					if (entity) { entity->setMaterialName("Water_LowDetails"); }
				}
			}
		}
	}
}
