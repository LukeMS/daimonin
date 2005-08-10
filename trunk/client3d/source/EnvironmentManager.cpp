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

#include "Ogre.h"
#include "TileChunk.h"
#include "TileManager.h"
#include "EnvironmentManager.h"

CEnvironmentManager::CEnvironmentManager(CTileManager* TileManagerPointer, CChunk* ChunkPointer)
{
	m_ChunkPtr = ChunkPointer;
//	m_WorldmapPtr = TileManagerPointer->Get_pworldmap();
	m_TileManagerPtr = TileManagerPointer;
}

CEnvironmentManager::~CEnvironmentManager()
{
	// m_TilePointer = NULL;
}

void CEnvironmentManager::UpdateEnvironment()
{
	m_Environment_SceneNode = m_TileManagerPtr->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();

// testing with meshes is more fun...
	static int INr=0;

if (INr > 1000) return;

	SceneNode *Node = m_Environment_SceneNode->createChildSceneNode(Vector3(850+INr, 850+INr, 80), Quaternion(1.0,0.0,0.0,0.0));
	Entity *mEntityNPC = Node->getCreator()->createEntity("OBJ_"+StringConverter::toString(++INr), "tree1.mesh" );
	Node->attachObject(mEntityNPC);
	Node->setScale(Vector3(.4, .4, .4));
//	Node->pitch(Radian(90));

	INr+= 50;

/*
	long x = m_ChunkPtr->m_posX;
	long y = m_ChunkPtr->m_posY;
	float StretchZ = m_TileManagerPtr->Get_StretchZ();
	unsigned long numVertices = 0; 

	// Bestimmung der Anzahl der Geometriepunkte
	numVertices = 3;
	if (numVertices == 0) return;
	VertexData* m_vdata = new VertexData();
	IndexData* m_idata;
	char name[50];
	char name2[50];
	sprintf( name, "Environment[%d,%d]", x, y );
	m_Environment_Mesh = MeshManager::getSingleton().createManual( name,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME ); 
	sprintf( name2, "SubEnvironment[%d,%d]", x, y );
	m_Environment_SubMesh= m_Environment_Mesh->createSubMesh(name2); 
	m_idata = m_Environment_SubMesh->indexData;
	
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

	m_vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer( 
		offset, // size of one whole vertex 
		numVertices, // number of vertices 
        HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage 
        false); // no shadow buffer 

	VertexBufferBinding* vbbind = m_vdata->vertexBufferBinding; 
	vbbind->setBinding(0, m_vbuf0); 

	Real* pReal;
	pReal = static_cast<Real*>(m_vbuf0->lock(HardwareBuffer::HBL_NORMAL)); 
				///////////////////////////////////////////////////////////////////////// 
				// .
				///////////////////////////////////////////////////////////////////////// 

		int height = (int) (m_WorldmapPtr->Get_heightdata(m_ChunkPtr->m_posX * CHUNK_SUM_X + 1,m_ChunkPtr->m_posY * CHUNK_SUM_Y + 1) * StretchZ);
		
		// 1st triangle
			pReal[ 0] =   0; pReal[ 1] =   0; pReal[2] = 30;
			pReal[ 3] =   0; pReal[ 4] =   0; pReal[5] = 1;
			pReal[ 6] =   0; pReal[ 7] =   1;

			pReal[ 8] = TILE_SIZE*16; pReal[ 9] =   0; pReal[10] = 30;
			pReal[11] =   0; pReal[12] =   0; pReal[13] = 1;
			pReal[14] = 0.5; pReal[15] =   1; 

			pReal[16] = TILE_SIZE*16; pReal[17] = TILE_SIZE*16; pReal[18] = 30;
			pReal[19] =   0; pReal[20] =   0; pReal[21] = 1;
			pReal[22] = 0.5; pReal[23] =   0; 

		// 2nd triangle
			pReal[24] =   0; pReal[25] =   0; pReal[26] = 30;
			pReal[27] =   0; pReal[28] =   0; pReal[29] = 1;
			pReal[30] =   0; pReal[31] =   1;

			pReal[32] = TILE_SIZE*16; pReal[33] = TILE_SIZE*16; pReal[34] = 30;
			pReal[35] =   0; pReal[36] =   0; pReal[37] = 1;
			pReal[38] = 0.5; pReal[39] =   0; 

			pReal[40] =   0; pReal[41] = TILE_SIZE*16; pReal[42] = 30;
			pReal[43] =   0; pReal[44] =   0; pReal[45] = 1;
			pReal[46] =   0; pReal[47] =   0; 

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

	unsigned short* pIdx = static_cast<unsigned short*>(m_ibuf->lock(HardwareBuffer::HBL_DISCARD)); 
   
	for (unsigned short a = 0; a < (unsigned short)numVertices; a++)
		pIdx[a] = a; 
   
	m_ibuf->unlock(); 

	m_Environment_SubMesh->operationType = RenderOperation::OT_TRIANGLE_LIST; 
	m_Environment_SubMesh->useSharedVertices = false; 
	m_Environment_SubMesh->vertexData = m_vdata; 

	// Setzen des Sichtbarkeits-Quaders. Fällt dieser Quader außerhalb des Sichtbereits der
	// Kamera, so wird das Kartenstück nicht gerendert.
	
	AxisAlignedBox* bounds = new AxisAlignedBox( 
		- TILE_SIZE *16* HALF_CHUNK_SIZE, -TILE_SIZE *16* HALF_CHUNK_SIZE, 0, 
		  TILE_SIZE *16* HALF_CHUNK_SIZE,  TILE_SIZE *16* HALF_CHUNK_SIZE, 100 * StretchZ); 
	m_Environment_Mesh->_setBounds( *bounds );
	
	delete bounds;

	m_Environment_SubMesh->setMaterialName("Water_HighDetails"); 
	sprintf( name2, "Environment[%d,%d] Entity", x, y);
	m_Environment_Entity = m_TileManagerPtr->Get_pSceneManager()->createEntity(name2, name); 

	m_Environment_SceneNode = m_TileManagerPtr->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode();
	m_Environment_SceneNode->attachObject( m_Environment_Entity );
	m_Environment_SceneNode->setPosition(x * TILE_SIZE * CHUNK_SUM_X,y * + TILE_SIZE * CHUNK_SUM_Y,0);
*/
}
