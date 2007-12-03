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
 * TileEngine class which manages the tiles in a chunk.
 *****************************************************************************/
class TileChunk
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////

    /** Height levels for the tiles. **/
    enum {
        // Mountains
        LEVEL_MOUNTAIN_TOP = 70,
        LEVEL_MOUNTAIN_MID = 60,
        LEVEL_MOUNTAIN_DWN = 40,
        // Plains
        LEVEL_PLAINS_TOP = 18,
        LEVEL_PLAINS_MID = 16,
        LEVEL_PLAINS_DWN = 14,
        LEVEL_PLAINS_SUB = 12,
        // Water
        LEVEL_WATER_CLP = LEVEL_PLAINS_SUB +2, // At this point the water clips the land-tiles.
        LEVEL_WATER_TOP = LEVEL_WATER_CLP -1,
    };

    static Ogre::AxisAlignedBox *mBounds;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileChunk();
    ~TileChunk();
    void create(int tileTextureSize);
    void change();
    void setMaterial(Ogre::String matLand, Ogre::String matWater);
    void freeRecources();
    /** Every chunk must have a land- AND a waterSubmesh,
    if there is no Water, we make a dummy submesh. **/
    void createDummy(Ogre::SubMesh* submesh);
    /** Create the land chunk. **/
    void createLand(int tileTextureSize);
    /** Change the land chunk. **/
    void changeLand();
    /** Create HW buffers for land chunk. **/
    void createLand_Buffers();
    /** Create the water chunk. **/
    void createWater();
    /** Change the water chunk. **/
    void changeWater();
    /** Create HW buffers for the water chunk. **/
    void createWater_Buffers();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::MeshPtr mMeshWater, mMeshLand;
    Ogre::SceneNode *mNodeWater,*mNodeLand;
    Ogre::SubMesh *mSubMeshWater, *mSubMeshLand;
    Ogre::Entity *mEntityWater, *mEntityLand;
};

#endif
