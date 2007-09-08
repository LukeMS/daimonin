/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "tile_interface.h"
#include "tile_manager.h"
#include "object_manager.h"
#include "logger.h"
#include "events.h"

using namespace Ogre;

const unsigned char TileInterface::mSubPosTable[2][32]=
    {
        {   // Format: YX (The first nibble is the y, the second the x pos).
            // QUADRANT: LEFT / TOP.
            //00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15
            0x70, 0x33, 0x40, 0x30, 0x51, 0x21, 0x52, 0x11, 0x60, 0x32, 0x41, 0x20, 0x50, 0x31, 0x42, 0x10,
            //16    17    18    19    20    21    22    23    24    25    26    27    28    29    30    31
            0x60, 0x32, 0x41, 0x20, 0x50, 0x31, 0x42, 0x10, 0x61, 0x22, 0x51, 0x21, 0x40, 0x30, 0x43, 0x00,
        },
        {   // QUADRANT: RIGHT / BOTTOM.
            //00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15
            0x07, 0x44, 0x37, 0x47, 0x26, 0x56, 0x25, 0x66, 0x17, 0x45, 0x36, 0x57, 0x27, 0x46, 0x35, 0x67,
            //16    17    18    19    20    21    22    23    24    25    26    27    28    29    30    31
            0x17, 0x45, 0x36, 0x57, 0x27, 0x46, 0x35, 0x67, 0x16, 0x55, 0x26, 0x56, 0x37, 0x47, 0x34, 0x77,
        },
    };

const unsigned char TileInterface::mWorldPosTable[4][8]=
    {
        // bit 7 stands for the right quadrant.
        // bit 6 stands for the upper quadrant.
        { 31,  80,  76,  66,  67,  83,  79, 128},
        { 15,   7,  68,  74,  77,  91, 152, 144},
        { 19,  27,  25,  78,  73, 134, 154, 148},
        {  3,  13,   9,   1, 158, 150, 146, 156}
    };

//================================================================================================
// Returns the selected position of a tile (Including the subposition within the tile).
//================================================================================================
const TilePos TileInterface::getSelectedTile()
{
    return mPos;
}

//================================================================================================
// Returns the selected world-pos.
//================================================================================================
const Vector3 TileInterface::getSelectedPos()
{
    return tileToWorldPos(mPos);
}

//================================================================================================
// Constructor.
//================================================================================================
TileInterface::TileInterface(SceneManager* sceneManager)
{
    mRaySceneQuery = TileManager::getSingleton().getSceneManager()->createRayQuery(Ray());
}

//================================================================================================
// Destructor.
//================================================================================================
TileInterface::~TileInterface()
{
    TileManager::getSingleton().getSceneManager()->destroyQuery(mRaySceneQuery);
}

//================================================================================================
// Stores the clicked tile in mPos.
//================================================================================================
const TilePos TileInterface::pickTile(float mouseX, float mouseY)
{
    mPos.x = 0, mPos.subX =0;
    mPos.z = 0, mPos.subZ =0;
    Ray mouseRay = TileManager::getSingleton().getSceneManager()->getCamera("PlayerCam")->getCameraToViewportRay(mouseX, mouseY);
    mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setQueryMask(ObjectManager::QUERY_TILES_LAND_MASK);

    // Perform the scene query.
    RaySceneQueryResult &result = mRaySceneQuery->execute();
    if (result.size() >1)
    {
        Logger::log().error() << "BUG in TileInterface.cpp: RaySceneQuery returned more than 1 result.";
        Logger::log().error() << "(Did you create Entities without setting a setQueryFlags() on them?)";
        for (RaySceneQueryResult::iterator itr = result.begin(); itr!= result.end(); ++itr)
            Logger::log().error() << "Query Result: " << itr->movable->getName();;
        return mPos;
    }

    // ////////////////////////////////////////////////////////////////////
    // Find the selected tile.
    // We start with our 4 tringles (each tile = 4 triangles).
    // Then we divide each triangle several times to increase the accuracy.
    // ////////////////////////////////////////////////////////////////////
    for (mPos.x = 0; mPos.x < TileManager::CHUNK_SIZE_X; ++mPos.x)
    {
        for (mPos.z = 0; mPos.z < TileManager::CHUNK_SIZE_Z; ++mPos.z)
        {
            if (getPickPos(&mouseRay, QUADRANT_LEFT  )) return mPos; // Found the clicked pos.
            if (getPickPos(&mouseRay, QUADRANT_RIGHT )) return mPos; // Found the clicked pos.
            if (getPickPos(&mouseRay, QUADRANT_TOP   )) return mPos; // Found the clicked pos.
            if (getPickPos(&mouseRay, QUADRANT_BOTTOM)) return mPos; // Found the clicked pos.
        }
    }
    return mPos;
}

//================================================================================================
// Increase the pick precision by dividing the tile tris.
//================================================================================================
bool TileInterface::getPickPos(Ray *mouseRay, int quadrant)
{
    int deep, upper = 0;
    std::pair<bool, Real> Test;

    fillVectors(mPos, quadrant);
    Test = Math::intersects(*mouseRay, mTris[0], mTris[1], mTris[2]);
    if (!Test.first) return false;  // This tile quadrant was not clicked.
    // A deep of 5 gives us a 8x8 Matrix for the sub positions.
    for (deep =0; deep < 5; ++deep)
    {
        // We divide the triangle (at the hypotenuse) into 2 triangles of the same size.
        mTris[3]= (mTris[0] + mTris[1]) /2;
        Test = Math::intersects(*mouseRay, mTris[0], mTris[2], mTris[3]);
        if (Test.first)
        {
            // We test always against mTris[0], mTris[2], mTris[3].
            // so he have to copy the new test vectors into this set.
            mTris[1] = mTris[2];
            mTris[2] = mTris[3];
            // If the click was on the upper half of the triangle, set the upper bit.
            // That way we have a unique number for each possible position.
            // We use the mSubPosTable to convert this number into a subX and subZ pos.
            upper+= (1 << deep);
        }
        else
        {
            mTris[0] = mTris[2];
            mTris[2] = mTris[3];
        }
    }
    if (quadrant&1)
    {   // Top / Bottom.
        mPos.subX = (mSubPosTable[quadrant>>1][31-upper] >> 4) & 0x0f;
        mPos.subZ = (mSubPosTable[quadrant>>1][31-upper]     ) & 0x0f;
        mPos.subX = (mSubPosTable[quadrant>>1][31-upper] >> 4) & 0x0f;
        mPos.subZ = (mSubPosTable[quadrant>>1][31-upper]     ) & 0x0f;
    }
    else
    {   // Left / Right.
        mPos.subZ = (mSubPosTable[quadrant>>1][upper] >> 4) & 0x0f;
        mPos.subX = (mSubPosTable[quadrant>>1][upper]     ) & 0x0f;
        mPos.subZ = (mSubPosTable[quadrant>>1][upper] >> 4) & 0x0f;
        mPos.subX = (mSubPosTable[quadrant>>1][upper]     ) & 0x0f;
    }
    return true;
}

//================================================================================================
// Converts a tile pos into the world pos.
//================================================================================================
Vector3 TileInterface::tileToWorldPos(TilePos tile)
{
    int upper;
    if (tile.subZ <=3)
    {
        upper = mWorldPosTable[tile.subZ][tile.subX];
        if      (upper & (1 << 7)) fillVectors(tile, QUADRANT_RIGHT);
        else if (upper & (1 << 6)) fillVectors(tile, QUADRANT_TOP);
        else                       fillVectors(tile, QUADRANT_LEFT);
    }
    else
    {
        upper = mWorldPosTable[3-(tile.subZ-4)][7-tile.subX];
        if      (upper & (1 << 7)) fillVectors(tile, QUADRANT_LEFT);
        else if (upper & (1 << 6)) fillVectors(tile, QUADRANT_BOTTOM);
        else                       fillVectors(tile, QUADRANT_RIGHT);
    }
    for (int deep =0; deep < 5; ++deep)
    {
        mTris[3]= (mTris[0] + mTris[1]) /2;
        if (upper & (1 << deep))
        {
            mTris[1] = mTris[2];
            mTris[2] = mTris[3];
        }
        else
        {
            mTris[0] = mTris[2];
            mTris[2] = mTris[3];
        }
    }
    mTris[3]= (mTris[0] + mTris[1]) /2;
    return mTris[3];
}

//================================================================================================
// Converts a tile pos into the special pos for walls.
//================================================================================================
Vector3 TileInterface::tileToWallPos(TilePos tile)
{
   mTris[0].x = (tile.x + tile.subX)*TileManager::TILE_SIZE_X;
   mTris[0].y = TileManager::getSingleton().getMapHeight(tile.x,tile.z);
   mTris[0].z = (tile.z + tile.subZ)*TileManager::TILE_SIZE_Z;
   return mTris[0];
}

//================================================================================================
// Fill the (original) tris positions of the given tile in the given quadrant.
// (see TileChunk class for more infos)
// +------+
// |\  2 /|
// | \  / |   1 = QUADRANT_LEFT
// |  \/  |   2 = QUADRANT_TOP
// |1 /\ 3|   3 = QUADRANT_RIGHT
// | /  \ |   4 = QUADRANT_BOTTOM
// |/  4 \|
// +------+
//================================================================================================
void TileInterface::fillVectors(TilePos &tile, int quad)
{
    if (quad == QUADRANT_LEFT)
    {
        mTris[0].x = (tile.x+0.0)*TileManager::TILE_SIZE_X;
        mTris[0].y = TileManager::getSingleton().getMapHeight(tile.x,tile.z);
        mTris[0].z = (tile.z+0.0)*TileManager::TILE_SIZE_Z;
        mTris[1].x = (tile.x+0.0)*TileManager::TILE_SIZE_X;
        mTris[1].y = TileManager::getSingleton().getMapHeight(tile.x, tile.z+1);
        mTris[1].z = (tile.z+1.0)*TileManager::TILE_SIZE_Z;
    }
    else if (quad == QUADRANT_RIGHT)
    {
        mTris[0].x = (tile.x+1.0)*TileManager::TILE_SIZE_X;
        mTris[0].y = TileManager::getSingleton().getMapHeight(tile.x+1, tile.z+1);
        mTris[0].z = (tile.z+1.0)*TileManager::TILE_SIZE_Z;
        mTris[1].x = (tile.x+1.0)*TileManager::TILE_SIZE_X;
        mTris[1].y = TileManager::getSingleton().getMapHeight(tile.x+1, tile.z);
        mTris[1].z = (tile.z+0.0)*TileManager::TILE_SIZE_Z;
    }
    else if (quad == QUADRANT_TOP)
    {
        mTris[0].x = (tile.x+1.0)*TileManager::TILE_SIZE_X;
        mTris[0].y = TileManager::getSingleton().getMapHeight(tile.x+1, tile.z);
        mTris[0].z = (tile.z+0.0)*TileManager::TILE_SIZE_Z;
        mTris[1].x = (tile.x+0.0)*TileManager::TILE_SIZE_X;
        mTris[1].y = TileManager::getSingleton().getMapHeight(tile.x, tile.z);
        mTris[1].z = (tile.z+0.0)*TileManager::TILE_SIZE_Z;
    }
    else // QUADRANT_BOTTOM
    {
        mTris[0].x = (tile.x+0.0)*TileManager::TILE_SIZE_X;
        mTris[0].y = TileManager::getSingleton().getMapHeight(tile.x, tile.z+1);
        mTris[0].z = (tile.z+1.0)*TileManager::TILE_SIZE_Z;
        mTris[1].x = (tile.x+1.0)*TileManager::TILE_SIZE_X;
        mTris[1].y = TileManager::getSingleton().getMapHeight(tile.x+1, tile.z+1);
        mTris[1].z = (tile.z+1.0)*TileManager::TILE_SIZE_Z;
    }
    mTris[2].x = (tile.x+0.5)*TileManager::TILE_SIZE_X;
    mTris[2].y = TileManager::getSingleton().getAvgMapHeight(tile.x, tile.z);
    mTris[2].z = (tile.z+0.5)*TileManager::TILE_SIZE_Z;
}

//================================================================================================
// Returns the distance between 2 subtile positions.
//================================================================================================
int TileInterface::calcTileDistance(const TilePos &pos1, const TilePos &pos2)
{
    int deltaZ = Math::IAbs((pos1.z * TileManager::SUM_SUBTILES + pos1.subZ) - (pos2.z * TileManager::SUM_SUBTILES + pos2.subZ));
    int deltaX = Math::IAbs((pos1.x * TileManager::SUM_SUBTILES + pos1.subX) - (pos2.x * TileManager::SUM_SUBTILES + pos2.subX));
    return (deltaZ > deltaX)?deltaZ:deltaX;
}
