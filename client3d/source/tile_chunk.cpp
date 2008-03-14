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

#define SUM_TILES 21


const char SRC_TEXTURE_NAME[]     = "terrain_group_00_1024.png";
const char TEXTURE_LAND_NAME[]    = "TileEngine/TexLand";
const char TEXTURE_WATER_NAME[]   = "TileEngine/TexWater";
const char MATERIAL_LAND_NAME[]   = "TileEngine/MatLand";
const char MATERIAL_WATER_NAME[]  = "TileEngine/MatWater";
const char MATERIAL_PAINTER_NAME[]= "TileEngine/MatPainter";
const int  GRID_SHADOW_MASK_NR    = 72; // Number of the shadow filter used to draw a grid.

//================================================================================================
// The map has its zero-position in the top-left corner:
// (x,z)
//  0,0 1,0 2,0 3,0
//  0,1 1,1 2,1 3,1
//  0,2 1,2 2,2 3,2
//  0,3 1,3 2,3 3,3
//================================================================================================

//================================================================================================
// Holds the vertex positions for mirroring on X and Z axis.
//================================================================================================
const char MIRROR_TABLE[4][6][2]=
{
    {{0,0}, {0,1}, {1,0}, {1,0}, {0,1}, {1,1}}, // No mirror
    {{1,0}, {1,1}, {0,0}, {0,0}, {1,1}, {0,1}}, // Mirror on x-axis.
    {{0,1}, {0,0}, {1,1}, {1,1}, {0,0}, {1,0}}, // Mirror on z-axis.
    {{1,1}, {1,0}, {0,1}, {0,1}, {1,0}, {0,0}}  // Mirror on x and z-axis.
};

//================================================================================================
// .
//================================================================================================
const int CHUNK_START_OFFSET[TileManager::MAP_SIZE/2+2]=
{
                //
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18
};

/*
const int CHUNK_START_OFFSET[MAP_SIZE/2+2]=
{
    {7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18}, //   0° camera rotation.
    {6, 6, 7, 7, 8, 8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17}, //  15° camera rotation.
    {5, 5, 6, 6, 7, 7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16}, //  30° camera rotation.
};
*/

//================================================================================================
// .
//================================================================================================
Real getTileTextureSize(int x, int z)
{
    if (z > 15 && x > 15 && x < 27) return 64.0f;
    if (z > 14 && x > 14 && x < 28) return 60.0f;
    if (z > 13 && x > 13 && x < 29) return 56.0f;
    if (z > 12 && x > 12 && x < 30) return 52.0f;
    if (z > 11 && x > 11 && x < 31) return 48.0f;
    if (z > 10 && x > 10 && x < 32) return 44.0f;
    if (z >  9 && x >  9 && x < 33) return 40.0f;
    if (z >  8 && x >  8 && x < 34) return 36.0f;
    if (z >  8 && x >  7 && x < 35) return 32.0f;
    if (z >  7 && x >  6 && x < 36) return 28.0f;
    if (z >  5 && x >  5 && x < 37) return 24.0f;
    if (z >  4 && x >  4 && x < 38) return 20.0f;
    if (z >  3 && x >  3 && x < 39) return 16.0f;
    if (z >  2 && x >  2 && x < 40) return 12.0f;
    return 8.0f;
}

//================================================================================================
// v0  +--+ v2/3
//     | /|
//     |/ |
// v1/4+--+ v5
//================================================================================================
void TilePainter::updateVertexBuffer()
{
    VertexBufferBinding *vBind = mRenderOp.vertexData->vertexBufferBinding;
    HardwareVertexBufferSharedPtr tvbuf = vBind->getBuffer(1);
    float* pTex = static_cast<float*>(tvbuf->lock(HardwareBuffer::HBL_DISCARD));
    int tilesPerRow  = TileManager::TEXTURE_SIZE /TileManager::ATLAS_TILE_SIZE;
    int filtersPerRow= TileManager::TEXTURE_SIZE /TileManager::ATLAS_FILTER_SIZE;
    float sizeTile   = 1.0f/(float) tilesPerRow;
    float sizeFilter = 1.0f/(float) filtersPerRow;
    Real colLayer0, rowLayer0, colLayer1, rowLayer1, colFilter, rowFilter;
    Real offX, offZ, colShadow, rowShadow;
    int tLayer0, tLayer1, tFilter, tShadow, tLayerMirror, tShadowMirror, mapX, mapZ;

    //TileManager::getSingleton().calcMapShadows();

    int startX, stopX, startZ, stopZ;
    startX = 7;
    stopX = 7+15;
    startZ = 7;
    stopZ = startZ + 29;

    TileManager::getSingleton().getMapScroll(mapX, mapZ);

    for (int z = 0; z < TileManager::MAP_SIZE/2+2; ++z)
    {
        offZ = (Real)((mapZ-z)&1) * sizeTile/2;
        for (int x = CHUNK_START_OFFSET[z]; x < TileManager::MAP_SIZE - CHUNK_START_OFFSET[z]; ++x) // TODO::: add the offset to draw the correct tiles.
        {
            offX = (Real)((mapX-x)&1) * sizeTile/2;
            // Tile gfx.
            TileManager::getSingleton().getMapGfx(x, z, tLayer0, tLayer1, tFilter, tLayerMirror);


            tLayer0 = 4;
            tLayer1 = 4;
            tFilter = 0;

            colLayer0 = offX + (Real)(tLayer0&3) * sizeTile;
            rowLayer0 = offZ + ((Real)(tLayer0/tilesPerRow)) * sizeTile;
            colLayer1 = offX + (Real)(tLayer1&3) * sizeTile;
            rowLayer1 = offZ + ((Real)(tLayer1/tilesPerRow)) * sizeTile;
            colFilter = ((Real)(tFilter&15)) * sizeFilter;
            rowFilter = ((Real)(tFilter/filtersPerRow)) * sizeFilter;
            // Tile shadow.
            TileManager::getSingleton().getMapShadow(x, z, tShadow, tShadowMirror);



            tShadow = 72;
            //tShadow = 66;

            colShadow = ((Real)(tShadow&15)) * sizeFilter;
            rowShadow =(((Real)(tShadow/filtersPerRow))+ filtersPerRow/2) * sizeFilter;
            // Vertex 0
            *pTex++ = colLayer0;
            *pTex++ = rowLayer0;
            *pTex++ = colFilter;
            *pTex++ = rowFilter;
            *pTex++ = colLayer1;
            *pTex++ = rowLayer1;
            *pTex++ = colShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][0][0];
            *pTex++ = rowShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][0][1];
            // Vertex 1
            *pTex++ = colLayer0;
            *pTex++ = rowLayer0 + sizeTile/2;
            *pTex++ = colFilter;
            *pTex++ = rowFilter + sizeFilter;
            *pTex++ = colLayer1;
            *pTex++ = rowLayer1 + sizeTile/2;
            *pTex++ = colShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][1][0];
            *pTex++ = rowShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][1][1];
            // Vertex 2
            *pTex++ = colLayer0 + sizeTile/2;
            *pTex++ = rowLayer0;
            *pTex++ = colFilter + sizeFilter;
            *pTex++ = rowFilter;
            *pTex++ = colLayer1 + sizeTile/2;
            *pTex++ = rowLayer1;
            *pTex++ = colShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][2][0];
            *pTex++ = rowShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][2][1];
            // Vertex 3
            *pTex++ = colLayer0 + sizeTile/2;
            *pTex++ = rowLayer0;
            *pTex++ = colFilter + sizeFilter;
            *pTex++ = rowFilter;
            *pTex++ = colLayer1 + sizeTile/2;
            *pTex++ = rowLayer1;
            *pTex++ = colShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][3][0];
            *pTex++ = rowShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][3][1];
            // Vertex 4
            *pTex++ = colLayer0;
            *pTex++ = rowLayer0 + sizeTile/2;
            *pTex++ = colFilter;
            *pTex++ = rowFilter + sizeFilter;
            *pTex++ = colLayer1;
            *pTex++ = rowLayer1 + sizeTile/2;
            *pTex++ = colShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][4][0];
            *pTex++ = rowShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][4][1];
            // Vertex 5
            *pTex++ = colLayer0 + sizeTile/2;
            *pTex++ = rowLayer0 + sizeTile/2;
            *pTex++ = colFilter + sizeFilter;
            *pTex++ = rowFilter + sizeFilter;
            *pTex++ = colLayer1 + sizeTile/2;
            *pTex++ = rowLayer1 + sizeTile/2;
            *pTex++ = colShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][5][0];
            *pTex++ = rowShadow + sizeFilter * MIRROR_TABLE[tShadowMirror][5][1];
        }
    }
    tvbuf->unlock();
}

//================================================================================================
// Create the painter used to draw the blended tiles from alpha-texture to a render-texture.
//================================================================================================
TilePainter::TilePainter(int sumVertices)
{
    mUseIdentityProjection = true;
    mUseIdentityView = true;
    mRenderOp.useIndexes = false;
    mRenderOp.vertexData = new VertexData();
    mRenderOp.vertexData->vertexCount = sumVertices;
    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST ;
    VertexDeclaration   *decl = mRenderOp.vertexData->vertexDeclaration;
    VertexBufferBinding *bind = mRenderOp.vertexData->vertexBufferBinding;
    size_t offset = 0;
    decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(VertexElement::getTypeSize(VET_FLOAT3), mRenderOp.vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    bind->setBinding(0, vbuf);
    float* pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    Real left, top, size;
    top = 1.0f;
    for (int z = 0; z < TileManager::MAP_SIZE/2+2; ++z)
    {
        left=-1.0f;
        for (int x = CHUNK_START_OFFSET[z]; x < TileManager::MAP_SIZE - CHUNK_START_OFFSET[z]; ++x)
        {
            size = getTileTextureSize(x, z)/512.0f;
            *pFloat++ = left;
            *pFloat++ = top;
            *pFloat++ = -1;
            *pFloat++ = left;
            *pFloat++ = top - size;
            *pFloat++ = -1;
            *pFloat++ = left+ size;
            *pFloat++ = top;
            *pFloat++ = -1;
            *pFloat++ = left + size;
            *pFloat++ = top;
            *pFloat++ = -1;
            *pFloat++ = left;
            *pFloat++ = top - size;
            *pFloat++ = -1;
            *pFloat++ = left+ size;
            *pFloat++ = top - size;
            *pFloat++ = -1;
            left+= size;
        }
        top-=getTileTextureSize(TileManager::MAP_SIZE/2, z)/512.0f; // Biggest size is in the middle.
    }
    vbuf->unlock();
    const int SUM_TEXTURE_UNITS = 4;
    offset = 0;
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,0);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,1);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,2);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES,3);
    offset += VertexElement::getTypeSize(VET_FLOAT2);

    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(VertexElement::getTypeSize(VET_FLOAT2)*SUM_TEXTURE_UNITS, sumVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    bind->setBinding(1, vbuf);
    updateVertexBuffer();

    MaterialPtr tmpMaterial;
    tmpMaterial= MaterialManager::getSingleton().getByName(MATERIAL_PAINTER_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(SRC_TEXTURE_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName(SRC_TEXTURE_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName(SRC_TEXTURE_NAME);
    //tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(3)->setTextureName("terrain_shadow.png");
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(3)->setTextureName(SRC_TEXTURE_NAME);
    setMaterial(MATERIAL_PAINTER_NAME);

    AxisAlignedBox aabInf;
    aabInf.setInfinite();
    setBoundingBox(aabInf);
    /*
            mRenderOp.useIndexes = true;
            mRenderOp.indexData = new IndexData();
            mRenderOp.indexData->indexCount = sumVertices;
            mRenderOp.indexData->indexStart = 0;
            mRenderOp.indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, mRenderOp.indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY,false);
            unsigned short* pIdx = static_cast<unsigned short*>(mRenderOp.indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
            unsigned short numVertices = mRenderOp.indexData->indexCount;
            for (unsigned short p=0; p < numVertices; ++p)
                *pIdx++ = p;
            mRenderOp.indexData->indexBuffer->unlock();
    */
}



//================================================================================================
// Constructor.
//================================================================================================
void TileChunk::init()
{
    MaterialPtr tmpMaterial;
    mSumVertices = 0;
    for (int i=0; i < TileManager::MAP_SIZE/2+2; ++i)
        mSumVertices+= TileManager::MAP_SIZE - 2*CHUNK_START_OFFSET[i];
    mSumVertices *= 6;
    // ////////////////////////////////////////////////////////////////////
    // Build the land-tiles.
    // ////////////////////////////////////////////////////////////////////
    mMeshLand = MeshManager::getSingleton().createManual("Mesh_Land", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
    mSubMeshLand = mMeshLand->createSubMesh();
    createLand();
    mMeshLand->_setBounds(AxisAlignedBox(0,0,0,TileManager::TILE_SIZE * TileManager::MAP_SIZE,MAX_TERRAIN_HEIGHT,TileManager::TILE_SIZE * TileManager::MAP_SIZE));
    mMeshLand->load();
    mEntityLand = TileManager::getSingleton().getSceneManager()->createEntity("Entity_Land", "Mesh_Land");
    mEntityLand->setQueryFlags(ObjectManager::QUERY_TILES_LAND_MASK);
    mEntityLand->setRenderQueueGroup(RENDER_QUEUE_1);
    mTexLand = TextureManager::getSingleton().createManual(TEXTURE_LAND_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 1024, 1024, 0, PF_R8G8B8, TU_RENDERTARGET);
    tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_LAND_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_LAND_NAME);
    mEntityLand->setMaterialName(MATERIAL_LAND_NAME);

    // ////////////////////////////////////////////////////////////////////
    // Build the water-tiles.
    // ////////////////////////////////////////////////////////////////////
    mMeshWater = MeshManager::getSingleton().createManual("Mesh_Water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSubMeshWater = mMeshWater->createSubMesh();
    createDummySubMesh(mSubMeshWater); // We don't have the tile datas yet, so no need to waste time in creating the tiles.
    mMeshWater->_setBounds(AxisAlignedBox(0,0,0,TileManager::TILE_SIZE * TileManager::MAP_SIZE,MAX_TERRAIN_HEIGHT,TileManager::TILE_SIZE * TileManager::MAP_SIZE));
    mMeshWater->load();
    mEntityWater = TileManager::getSingleton().getSceneManager()->createEntity("Entity_Water", "Mesh_Water");
    mEntityWater->setQueryFlags(ObjectManager::QUERY_TILES_WATER_MASK);
    mEntityWater->setRenderQueueGroup(RENDER_QUEUE_8);
    mTexWater = TextureManager::getSingleton().createManual(TEXTURE_WATER_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, TileManager::ATLAS_TILE_SIZE, TileManager::ATLAS_TILE_SIZE, 0, PF_A8R8G8B8, TU_STATIC);
    tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_WATER_NAME);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_WATER_NAME);
    mEntityWater->setMaterialName(MATERIAL_WATER_NAME);
    // ////////////////////////////////////////////////////////////////////
    // Attach the tiles to a scenenode.
    // ////////////////////////////////////////////////////////////////////
    mNode= TileManager::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();
    mNode->setPosition(0, 0, 0);
    mNode->attachObject(mEntityLand);
    mNode->attachObject(mEntityWater);

    mPainter= new TilePainter(mSumVertices);
    loadAtlasTexture("terrain_group_00_1024.png");
    updatePainter();
}

//================================================================================================
// Set a new material.
//================================================================================================
void TileChunk::loadAtlasTexture(String newAtlasTexture)
{
    Image image;
    image.load(newAtlasTexture, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    uint32 *src = (uint32 *) image.getData();
    // ////////////////////////////////////////////////////////////////////
    // Copy the water tile to a separate texture
    // (Water needs own texture for the scroll animation).
    // ////////////////////////////////////////////////////////////////////
    PixelBox pb;
    uint32 *dst;
    int size = (int)image.getWidth();
    int sizeTile = size/4;

    pb = mTexWater->getBuffer()->lock(Box(0, 0, TileManager::ATLAS_TILE_SIZE, TileManager::ATLAS_TILE_SIZE), HardwareBuffer::HBL_DISCARD);
    dst = (uint32 *)pb.data;
    for (int y = 0; y<sizeTile; ++y)
    {
        for (int x = 0; x<sizeTile; ++x)
            *dst++ = *src++ | 0xff000000;
        src+=(size - sizeTile);
    }
    mTexWater->getBuffer()->unlock();
    // ////////////////////////////////////////////////////////////////////
    // Copy the new atlastexture over the old one.
    // ////////////////////////////////////////////////////////////////////
    MaterialPtr tmpMaterial = mPainter->getMaterial();
    TexturePtr texPainter = TextureManager::getSingleton().getByName(tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName());
    pb = texPainter->getBuffer()->lock(Box(0, 0, TileManager::ATLAS_TILE_SIZE, TileManager::ATLAS_TILE_SIZE), HardwareBuffer::HBL_DISCARD);
    src = (uint32 *) image.getData();
    dst = (uint32 *)pb.data;
    for (int i=0; i< size*size; ++i)
        *dst++ = *src++;
    texPainter->getBuffer()->unlock();
    updatePainter();
}

//================================================================================================
// Update the rendertexture (e.g after a mapscroll).
//================================================================================================
void TileChunk::updatePainter()
{
    mPainter->updateVertexBuffer();
    SceneNode *node = TileManager::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode("nodeTilePainter");
    node->attachObject(mPainter);
    RenderTexture *renderTarget = mTexLand->getBuffer()->getRenderTarget();
    renderTarget->setAutoUpdated(false);
    Camera *camera = TileManager::getSingleton().getSceneManager()->createCamera("tmpCamera");
    camera->setUseIdentityView(true);
    camera->setUseIdentityProjection(true);
    Viewport *viewport = renderTarget->addViewport(camera);
    viewport->setOverlaysEnabled(false);
    viewport->setClearEveryFrame(false);
    viewport->setDimensions(0.0f, 0.0f, 1.0f, 1.0f);
    // Render only the queues in the special case list.
    TileManager::getSingleton().getSceneManager()->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_INCLUDE);
    TileManager::getSingleton().getSceneManager()->addSpecialCaseRenderQueue(RENDER_QUEUE_MAIN);
    mPainter->setRenderQueueGroup(RENDER_QUEUE_MAIN);
    renderTarget->update();
//renderTarget->writeContentsToFile("Painter.png");
    TileManager::getSingleton().getSceneManager()->removeSpecialCaseRenderQueue(RENDER_QUEUE_MAIN);
    //Render all except the queues in the special case list.
    TileManager::getSingleton().getSceneManager()->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
    renderTarget->removeViewport(0);
    node->detachAllObjects();
    TileManager::getSingleton().getSceneManager()->destroyCamera(camera);
    TileManager::getSingleton().getSceneManager()->destroySceneNode("nodeTilePainter");
}

//================================================================================================
// Free all resources.
//================================================================================================
void TileChunk::freeRecources()
{
    mMeshLand.setNull();
    mMeshWater.setNull();
    mTexWater.setNull();
    mTexLand.setNull();
    mMeshPainter.setNull();
    delete mPainter;
}

//================================================================================================
// Change a chunk.
//================================================================================================
void TileChunk::change()
{
    long time = Root::getSingleton().getTimer()->getMicroseconds();
    changeLand();
    createWater();
    updatePainter();
    Logger::log().error() << "Time to change terrain: " << (double)(Root::getSingleton().getTimer()->getMicroseconds() - time)/1000 << " ms";
}

//================================================================================================
// Create hardware buffers for water.
//================================================================================================
void TileChunk::createWater()
{



    createDummySubMesh(mSubMeshWater);
    return;





    // ////////////////////////////////////////////////////////////////////
    // Count the Vertices in this chunk.
    // ////////////////////////////////////////////////////////////////////
    unsigned int numVertices = 0;
    for (int x = 0; x < TileManager::CHUNK_SIZE; ++x)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE; ++z)
        {
            if (TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_TL) < WATERLEVEL || TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_TR) < WATERLEVEL ||
                    TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_BL) < WATERLEVEL || TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_BR) < WATERLEVEL)
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
        createDummySubMesh(mSubMeshWater);
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Create VertexData.
    // ////////////////////////////////////////////////////////////////////
    int mapX, mapZ;
    delete mSubMeshWater->vertexData;
    VertexData* vdata = new VertexData();
    vdata->vertexCount = numVertices;
    VertexDeclaration* vdec = vdata->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement( 0, offset, VET_FLOAT3, VES_POSITION );
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement( 0, offset, VET_FLOAT3, VES_NORMAL   );
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset += VertexElement::getTypeSize(VET_FLOAT2);

    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(offset, numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vdata->vertexBufferBinding->setBinding(0, vbuf0);

    static Real offsetWave = 0.03;
    static Real WaveHigh = 0;
    WaveHigh+= offsetWave;
    if (WaveHigh >1.7 || WaveHigh < -1.7) offsetWave*=-1;

    Real* pReal = static_cast<Real*>(vbuf0->lock (HardwareBuffer::HBL_DISCARD));
    Real q1, q2, offX, offZ;
    for (int x = 0; x < TileManager::CHUNK_SIZE; ++x)
    {
        for (int z = 0; z < TileManager::CHUNK_SIZE; ++z)
        {
            if (TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_TL) < WATERLEVEL || TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_TR) < WATERLEVEL ||
                    TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_BL) < WATERLEVEL || TileManager::getSingleton().getMapHeight(x, z, TileManager::VERTEX_BR) < WATERLEVEL)
            {
                // ////////////////////////////////////////////////////////////////////
                // Position.
                // ////////////////////////////////////////////////////////////////////
                TileManager::getSingleton().getMapScroll(mapX, mapZ);
                mapX-=x;
                mapZ-=z;
                if ((mapX&1) != (mapZ&1))
                {
                    q1 = WATERLEVEL + WaveHigh;
                    q2 = WATERLEVEL - WaveHigh;
                }
                else
                {
                    q1 = WATERLEVEL - WaveHigh;
                    q2 = WATERLEVEL + WaveHigh;
                }
                offX = 1-(Real)(mapX&3) * 0.25;
                offZ = 1-(Real)(mapZ&3) * 0.25;
                // 1. Triangle
                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = q1;
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX;
                *pReal++ = offZ;

                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX;
                *pReal++ = offZ + 0.25;

                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX + 0.25;
                *pReal++ = offZ;
                // 2. Triangle
                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * z;
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX + 0.25;
                *pReal++ = offZ;

                *pReal++ = TileManager::TILE_SIZE * x;
                *pReal++ = q2;
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX;
                *pReal++ = offZ + 0.25;

                *pReal++ = TileManager::TILE_SIZE * (x+1);
                *pReal++= q1;
                *pReal++ = TileManager::TILE_SIZE * (z+1);
                *pReal++ = 0;
                *pReal++ = 1;
                *pReal++ = 0;
                *pReal++ = offX + 0.25;
                *pReal++ = offZ + 0.25;
            }
        }
    }
    vbuf0->unlock();
    // ////////////////////////////////////////////////////////////////////
    // Create Index-buffer
    // (SubMeshes always use indexes)
    // ////////////////////////////////////////////////////////////////////
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    IndexData* idata = mSubMeshWater->indexData;
    idata->indexBuffer = ibuf;
    idata->indexStart = 0;
    idata->indexCount = numVertices;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock (HardwareBuffer::HBL_DISCARD));
    for (unsigned short p=0; numVertices; --numVertices) *pIdx++ = p++;
    ibuf->unlock();

    mSubMeshWater->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshWater->useSharedVertices = false;
    mSubMeshWater->vertexData = vdata;
}
















//================================================================================================
// v0  +--+ v2/3
//     | /|
//     |/ |
// v1/4+--+ v5
//================================================================================================
void TileChunk::changeLand()
{
    VertexData *vData = mSubMeshLand->vertexData;
    VertexBufferBinding *vBind = vData->vertexBufferBinding;
    HardwareVertexBufferSharedPtr vbuf = vBind->getBuffer(0);
    Real *pReal = static_cast<Real*>(vbuf->lock (HardwareBuffer::HBL_DISCARD));
    int mapX, mapZ;
    TileManager::getSingleton().getMapScroll(mapX, mapZ);
    mapX += (mapZ&1);
    Real offX = 13 * TileManager::TILE_SIZE;
    Real offZ =  2 * TileManager::TILE_SIZE;
    Real left, top, size;
    top = 0.0f;
    for (int z = 0; z < TileManager::MAP_SIZE/2+2; ++z)
    {
        left= 0.0f;
        for (int x = CHUNK_START_OFFSET[z]; x < TileManager::MAP_SIZE - CHUNK_START_OFFSET[z]; ++x)
        {
            // 1. Triangle
            size = getTileTextureSize(x, z)/1024.0f;
            *pReal++ = offX + TileManager::TILE_SIZE * x;
            *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TL);
            *pReal++ = offZ + TileManager::TILE_SIZE * z;
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = left;
            *pReal++ = top;

            *pReal++  = offX + TileManager::TILE_SIZE * x;
            *pReal++  = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BL);
            *pReal++  = offZ + TileManager::TILE_SIZE * (z+1);
            *pReal++  = 0.0;
            *pReal++  = 1.0;
            *pReal++  = 0.0;
            *pReal++ = left;
            *pReal++ = top + size;

            *pReal++  = offX + TileManager::TILE_SIZE * (x+1);
            *pReal++  = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TR);
            *pReal++  = offZ + TileManager::TILE_SIZE * z;
            *pReal++  = 0.0;
            *pReal++  = 1.0;
            *pReal++  = 0.0;
            *pReal++ = left + size;
            *pReal++ = top;

            // 2. Triangle
            *pReal++ = offX + TileManager::TILE_SIZE * (x+1);
            *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_TR);
            *pReal++ = offZ + TileManager::TILE_SIZE * z;
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = left + size;
            *pReal++ = top;

            *pReal++ = offX + TileManager::TILE_SIZE * x;
            *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BL);
            *pReal++ = offZ + TileManager::TILE_SIZE * (z+1);
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = left;
            *pReal++ = top + size;

            *pReal++ = offX + TileManager::TILE_SIZE * (x+1);
            *pReal++ = TileManager::getSingleton().getMapHeight(x,z,TileManager::VERTEX_BR);
            *pReal++ = offZ + TileManager::TILE_SIZE * (z+1);
            *pReal++ = 0.0;
            *pReal++ = 1.0;
            *pReal++ = 0.0;
            *pReal++ = left + size;
            *pReal++ = top + size;
            left+= size;
            --mapX;
        }
        top+=getTileTextureSize(TileManager::MAP_SIZE/2, z)/1024.0f; // Biggest size is in the middle.
    }
    vbuf->unlock();
}

//================================================================================================
//  Create hardware buffers for land.
//================================================================================================
void TileChunk::createLand()
{
    // ////////////////////////////////////////////////////////////////////
    // Create VertexData.
    // ////////////////////////////////////////////////////////////////////
    VertexData* vdata = new VertexData();
    vdata->vertexCount = mSumVertices;
    VertexDeclaration* vdec = vdata->vertexDeclaration;
    size_t offset = 0;
    vdec->addElement(0, offset, VET_FLOAT3, VES_POSITION);
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    vdec->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(offset, mSumVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vdata->vertexBufferBinding->setBinding(0, vbuf0);
    // ////////////////////////////////////////////////////////////////////
    // Create Index-buffer (SubMeshes always use indexes)
    // ////////////////////////////////////////////////////////////////////
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, mSumVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    IndexData* idata = mSubMeshLand->indexData;
    idata->indexBuffer = ibuf;
    idata->indexStart = 0;
    idata->indexCount = mSumVertices;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock (HardwareBuffer::HBL_DISCARD));
    for (unsigned short p=0; p < mSumVertices; ++p) *pIdx++ = p;
    ibuf->unlock();

    mSubMeshLand->operationType = RenderOperation::OT_TRIANGLE_LIST;
    mSubMeshLand->useSharedVertices = false;
    mSubMeshLand->vertexData = vdata;
}

//================================================================================================
// Creates a dummy submesh containing only 1 Triangle.
// Used when there is no water in the scene.
//================================================================================================
void TileChunk::createDummySubMesh(SubMesh* submesh)
{
    delete submesh->vertexData;
    VertexData* vdata = new VertexData();
    vdata->vertexCount = 3;
    vdata->vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
    HardwareVertexBufferSharedPtr vbuf0;
    vbuf0 = HardwareBufferManager::getSingleton().createVertexBuffer(VertexElement::getTypeSize(VET_FLOAT3), vdata->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    vdata->vertexBufferBinding->setBinding(0, vbuf0);
    Real *pReal = static_cast<Real*>(vbuf0->lock (HardwareBuffer::HBL_DISCARD));
    for (unsigned int i =0; i < vdata->vertexCount * 3; ++i) *pReal++= 0;
    vbuf0->unlock();
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, vdata->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
    IndexData* idata = submesh->indexData;
    idata->indexBuffer= ibuf;
    idata->indexStart = 0;
    idata->indexCount = vdata->vertexCount;
    unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock (HardwareBuffer::HBL_DISCARD));
    *pIdx++ = 2;
    *pIdx++ = 1;
    *pIdx++ = 0;
    ibuf->unlock();
    submesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
    submesh->useSharedVertices = false;
    submesh->vertexData = vdata;
}
