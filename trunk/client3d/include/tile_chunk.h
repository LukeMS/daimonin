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

#ifndef TILE_CHUNK_H
#define TILE_CHUNK_H

#include "Ogre.h"

/**
 * Helper class for drawing blended tile layers to a rendertexture.
 *****************************************************************************/
class TilePainter : public Ogre::SimpleRenderable
{
public:
    TilePainter(int sumVertices, int size);
    ~TilePainter() { delete mRenderOp.vertexData; }
    void updateVertexBuffer(int rotation);

private:
    // Not used.
    Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const { return 0; }
    Ogre::Real getBoundingRadius(void) const { return 0; }
};

/**
 * RenderOperation for landtiles.
 *****************************************************************************/
class LandTiles : public Ogre::SimpleRenderable
{
public:
    LandTiles(int sumVertices);
    ~LandTiles() { delete mRenderOp.vertexData; }
    void updateVertexBuffer();

private:
    // Not used.
    Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const { return 0; }
    Ogre::Real getBoundingRadius(void) const { return 0; }
};

/**
 * TileEngine class which manages the tiles in a chunk.
 *****************************************************************************/
class TileChunk
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { MAX_TERRAIN_HEIGHT = 255 *10 };
    enum { WATERLEVEL         =  14 }; /**<  At this height the water clips the land-tiles. **/
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileChunk(){};
    ~TileChunk(){};
    void init(int textureSize);
    void change();
    void freeRecources();
    void loadAtlasTexture(int group);
    void updatePainter();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    TilePainter *mPainter;  // Make it static when using more TileChunks.
    LandTiles *mLandTiles;
    Ogre::TexturePtr mTexLand, mTexWater;
    Ogre::MeshPtr mMeshWater;
    Ogre::Entity *mEntityWater;
    int mSumVertices;
    int mTextureSize;
    int mSubTileSize;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /** The terrain must have a land- AND a waterSubmesh. If there are no datas for it, we create a dummy. **/
    void createDummySubMesh(Ogre::SubMesh *submesh);
    void createWater();
};

#endif
