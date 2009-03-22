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
 ** TileEngine class which handles the tiles in a chunk.
 *****************************************************************************/
class TileChunk
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileChunk()  {}
    ~TileChunk() {}
    void init(int textureSize, int queryMaskLand, int queryMaskWater);
    void update();
    void rotate(Ogre::Real cameraAngle);
    void freeRecources();
    void loadAtlasTexture(int landGroup, int waterGroup);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { X = 0, Z = 1 };                 /**< Only used for readability. **/
    enum { MAX_TERRAIN_HEIGHT = 255 *10 }; /**< Height of the terrain is limited. **/
    enum { WATERLEVEL         =  14 };     /**< At this height the water clips the land-tiles. **/
    int mTextureSize;
    unsigned int mCameraRotation;
    Ogre::MeshPtr mMeshLand, mMeshWater;
    Ogre::Entity *mEntityLand, *mEntityWater;
    Ogre::Vector3 mNormal, mVec1, mVec2, mVec3;
    Ogre::Real *mPosVBuf;
    Ogre::Real mTexPosInAtlas[8];
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /** The terrain must have a land- AND a waterSubmesh. If there is no data for it, we create a dummy. **/
    void createDummySubMesh(Ogre::SubMesh *submesh);
    void createIndexData(Ogre::SubMesh *submesh, int sumVertices);
    void calcNormal(Ogre::Real x1, Ogre::Real z1, Ogre::Real x2, Ogre::Real z2, Ogre::Real x3, Ogre::Real z3);
    void setVertex(Ogre::Vector3 &pos, Ogre::Real posTexX, Ogre::Real posTexZ, Ogre::Real posShadowX, Ogre::Real posShadowZ, int offset);
    void changeLand();
    void changeWater();
    int  calcTextureUnitSorting(int l0, int l1, int l2);
};

#endif
