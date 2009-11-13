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
#include <OgreSceneManager.h>
#include <OgreMeshManager.h>
#include "logger.h"
#include "tile_decal.h"
#include "tile_manager.h"

using namespace Ogre;

int TileDecal::mSumDecal = 0;
int TileDecal::mMaxDecal = 0;
Ogre::SceneManager *TileDecal::mSceneManager = 0;

//================================================================================================
// Constructor.
// Todo: Can be optimized by using OT_TRIANGLE_STRIP instead of OT_TRIANGLE_LIST.
//================================================================================================
TileDecal::TileDecal(unsigned int sizeInSubtiles, int posX, int posZ, const char *strMaterial)
{
    if (!mMaxDecal) mSceneManager = TileManager::getSingleton().getSceneManager();
    if (!mSceneManager)
    {
        Logger::log().error() << "TileManager must me initialized before you can use the TileDecal class!";
        return;
    }
    mSize = (sizeInSubtiles >= MAX_SIZE)?MAX_SIZE:sizeInSubtiles+1;
    ++mSumDecal;
    String strNumber = StringConverter::toString(++mMaxDecal);
    MeshPtr mesh = MeshManager::getSingleton().createManual("Mesh/Decal" + strNumber, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMesh = mesh->createSubMesh();
    mSubMesh->useSharedVertices = false; // Has its own vertex data.
    // Create the vertex data.
    VertexData *vData = new VertexData(); // The 'delete' will be done by Ogre::Submsh.
    vData->vertexCount = mSize * mSize * 4;
    VertexDeclaration *vdec = vData->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);               offset+= VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0); offset+= VertexElement::getTypeSize(VET_FLOAT2);
    HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(offset, vData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vData->vertexBufferBinding->setBinding(0, vbuf);
    mSubMesh->vertexData = vData;
    // Create the index data.
    mSubMesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
    IndexData *idata = mSubMesh->indexData;
    idata->indexCount = mSize * mSize * 6;
    idata->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, idata->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    idata->indexStart = 0;
    unsigned short *pIdx = static_cast<unsigned short*>(idata->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
    for (unsigned int tCount = 0; tCount < vData->vertexCount; tCount+=4)
    {
        *pIdx++ = tCount+2; *pIdx++ = tCount+3; *pIdx++ = tCount+0;
        *pIdx++ = tCount+3; *pIdx++ = tCount+1; *pIdx++ = tCount+0;
    }
    idata->indexBuffer->unlock();
    setPosition(posX, posZ);
    mesh->_setBounds(AxisAlignedBox(Math::NEG_INFINITY, Math::NEG_INFINITY, Math::NEG_INFINITY, Math::POS_INFINITY, Math::POS_INFINITY, Math::POS_INFINITY));
    //mesh->_setBoundingSphereRadius(Real radius);
    mesh->load();
    Entity *entity = mSceneManager->createEntity("Entity/Decal" + strNumber, mesh->getName());
    entity->setMaterialName(strMaterial);
    entity->setQueryFlags(0);
    entity->setRenderQueueGroup(RENDER_QUEUE_8);
    mNode= mSceneManager->getRootSceneNode()->createChildSceneNode("Node/Decal" + strNumber);
    mNode->attachObject(entity);
}

//================================================================================================
// Destructor.
//================================================================================================
TileDecal::~TileDecal()
{
    mNode->getParentSceneNode()->removeAndDestroyChild(mNode->getName());
    --mSumDecal;
}

//================================================================================================
// OT_TRIANGLE_LIST.
//================================================================================================
void TileDecal::setPosition(int posX, int posZ)
{
    const Real DELTA_HEIGHT = 0.5;
    int tileX = posX/TileManager::TILE_RENDER_SIZE;
    int tileZ = posZ/TileManager::TILE_RENDER_SIZE;
    HardwareVertexBufferSharedPtr vbuf = mSubMesh->vertexData->vertexBufferBinding->getBuffer(0);
    Real *pReal = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    Real deltaUV = 1.0/(mSize-1);
    Real dU = (Real)(posX-tileX*TileManager::TILE_RENDER_SIZE)/TileManager::TILE_RENDER_SIZE*deltaUV;
    Real dV = (Real)(posZ-tileZ*TileManager::TILE_RENDER_SIZE)/TileManager::TILE_RENDER_SIZE*deltaUV;
    Real startX, startZ = 0;
    for (int z = tileZ; z < tileZ+mSize; ++z)
    {
        startX = 0;
        for (int x = tileX; x < tileX+mSize; ++x)
        {
            if (((x+z)&1))
            {
                // 0: Bottom/Left
                *pReal++ = (x+0)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = TileManager::getSingleton().getMapHeight(x+0, z+1) + DELTA_HEIGHT;
                *pReal++ = (z+1)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = startX-dU;
                *pReal++ = startZ-dV+deltaUV;
                // 1: Top/Left
                *pReal++ = (x+0)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = TileManager::getSingleton().getMapHeight(x+0, z+0) + DELTA_HEIGHT;
                *pReal++ = (z+0)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = startX-dU;
                *pReal++ = startZ-dV;
                // 2: Bottom/Right
                *pReal++ = (x+1)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = TileManager::getSingleton().getMapHeight(x+1, z+1) + DELTA_HEIGHT;
                *pReal++ = (z+1)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = startX-dU+deltaUV;
                *pReal++ = startZ-dV+deltaUV;
                // 3: Top/Right
                *pReal++ = (x+1)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = TileManager::getSingleton().getMapHeight(x+1, z+0) + DELTA_HEIGHT;
                *pReal++ = (z+0)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = startX-dU+deltaUV;
                *pReal++ = startZ-dV;
            }

            else
            {
                // 2: Bottom/Right
                *pReal++ = (x+1)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = TileManager::getSingleton().getMapHeight(x+1, z+1) + DELTA_HEIGHT;
                *pReal++ = (z+1)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = startX-dU+deltaUV;
                *pReal++ = startZ-dV+deltaUV;
                // 0: Bottom/Left
                *pReal++ = (x+0)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = TileManager::getSingleton().getMapHeight(x+0, z+1) + DELTA_HEIGHT;
                *pReal++ = (z+1)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = startX-dU;
                *pReal++ = startZ-dV+deltaUV;
                // 3: Top/Right
                *pReal++ = (x+1)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = TileManager::getSingleton().getMapHeight(x+1, z+0) + DELTA_HEIGHT;
                *pReal++ = (z+0)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = startX-dU+deltaUV;
                *pReal++ = startZ-dV;
                // 1: Top/Left
                *pReal++ = (x+0)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = TileManager::getSingleton().getMapHeight(x+0, z+0) + DELTA_HEIGHT;
                *pReal++ = (z+0)*TileManager::TILE_RENDER_SIZE;
                *pReal++ = startX-dU;
                *pReal++ = startZ-dV;
            }
            startX+= deltaUV;
        }
        startZ+= deltaUV;
    }
    vbuf->unlock();
}
