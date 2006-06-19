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
#include "logger.h"

///================================================================================================
/// Returns the selected position of a tile (Including the subposition within the tile).
///================================================================================================
const Vector3 TileInterface::getSelectedTile()
{
    Vector3 tmp;
    if (mX <0 || mZ <0)
    {
        tmp.x = 0;
        tmp.z = 0;
        tmp.y = 0;
    }
    else
    {
        tmp.x = mX;
        tmp.z = mZ;
        tmp.y = mSubtile;
    }
    return tmp;
}

///================================================================================================
/// Returns the selected map pos. (Including the subposition within the map).
///================================================================================================
const Vector3 TileInterface::getSelectedPos()
{
    Vector3 tmp;
    if (mX <0 || mZ <0)
    {
        tmp.x = 0;
        tmp.z = 0;
        tmp.y = 0;
    }
    else
    {
        tmp.y = (Real) (TileManager::getSingleton().getAvgMapHeight(mX, mZ));
        if (mSubtile <0)
        {
            tmp.x = (mX +0.5) * TILE_SIZE_X;
            tmp.z = (mZ +0.5) * TILE_SIZE_Z;
              Logger::log().error() << "hewre";
        }
        else
        {
            if (mSubtile ==1 || mSubtile==3)
                tmp.x = (mX +0.75) * TILE_SIZE_X;
            else
                tmp.x = (mX +0.25) * TILE_SIZE_X;
            if (mSubtile >=2)
                tmp.z = (mZ +0.75) * TILE_SIZE_Z;
            else
                tmp.z = (mZ +0.25) * TILE_SIZE_Z;
        }
    }
    return tmp;
}

///================================================================================================
/// Constructor.
///================================================================================================
TileInterface::TileInterface(SceneManager* sceneManager)
{
    mSceneManager = sceneManager;
    mRaySceneQuery = mSceneManager->createRayQuery(Ray());
}

///================================================================================================
/// Destructor.
///================================================================================================
TileInterface::~TileInterface()
{
    mSceneManager->destroyQuery(mRaySceneQuery);
}

///================================================================================================
/// Mouse picking.
///================================================================================================
void TileInterface::pickTile(float mouseX, float mouseY)
{
    /// save old selection to compare to new selection later
    mDistance = 1000000; // something big
    mX = -1;
    mZ = -1;

    Ray mouseRay = TileManager::getSingleton().getSceneManager()->getCamera("PlayerCam")->getCameraToViewportRay(mouseX, mouseY);
    mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setQueryMask(QUERY_TILES_LAND_MASK);

    /// Perform the scene query.
    RaySceneQueryResult &result = mRaySceneQuery->execute();
    if (result.size() >1)
    {
        Logger::log().error() << "BUG in TileInterface.cpp: RaySceneQuery returned more than 1 result.";
        Logger::log().error() << "(You created Entities without setting a setQueryFlags(...) on them)";
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Find the tile that was selected.
    /// ////////////////////////////////////////////////////////////////////
    Real height[4], avgHeight;
    std::pair<bool, Real> Test;
    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int y = 0; y < CHUNK_SIZE_Z; ++y)
        {
            /// ////////////////////////////////////////////////////////////////////
            /// we have to build a bounding box for each tile and check if the ray
            /// intersects this box.
            /// To do this, we need the height of the tile vertices.
            /// ////////////////////////////////////////////////////////////////////
            height[0] = TileManager::getSingleton().getMapHeight(x    , y    );
            height[1] = TileManager::getSingleton().getMapHeight(x + 1, y    );
            height[2] = TileManager::getSingleton().getMapHeight(x    , y + 1);
            height[3] = TileManager::getSingleton().getMapHeight(x + 1, y + 1);
            avgHeight = (height[0]+height[1]+height[2]+height[3]) /4.0;
            Test = mouseRay.intersects(
                       AxisAlignedBox((x       )* TILE_SIZE_X, avgHeight,
                                      (y       )* TILE_SIZE_Z,
                                      (x  + 1.0)* TILE_SIZE_X, avgHeight,
                                      (y  + 1.0)* TILE_SIZE_Z));
            if (!Test.first || Test.second > mDistance)
                continue;
            mDistance = Test.second;
            mX  = x;
            mZ  = y;
        }
    }

    /// ////////////////////////////////////////////////////////////////////
    /// now we build 5 bounding boxes for the tile to increase picking accuracy
    /// Note: Ogre only allows bounding boxes with the first vector having
    /// got the lower value in every(!) component. so we have to check
    /// which height value is greater
    /// ////////////////////////////////////////////////////////////////////
    Real offsetX =0, offsetY=0;

    mDistance = 100000;
    height[0] = TileManager::getSingleton().getMapHeight(mX    , mZ    );
    height[1] = TileManager::getSingleton().getMapHeight(mX + 1, mZ    );
    height[2] = TileManager::getSingleton().getMapHeight(mX    , mZ + 1);
    height[3] = TileManager::getSingleton().getMapHeight(mX + 1, mZ + 1);
    avgHeight = (height[0]+height[1]+height[2]+height[3]) /4.0;
    // 4 edges.
    for (int edge=0; edge < 4; ++edge)
    {
        if (height[edge] > avgHeight)
            Test = mouseRay.intersects(
                       AxisAlignedBox((mX + offsetX      )* TILE_SIZE_X, avgHeight,
                                      (mZ + offsetY      )* TILE_SIZE_Z,
                                      (mX + offsetX + 0.5)* TILE_SIZE_X, height[edge],
                                      (mZ + offsetY + 0.5)* TILE_SIZE_Z));
        else
            Test = mouseRay.intersects(
                       AxisAlignedBox((mX + offsetX      )* TILE_SIZE_X, height[edge],
                                      (mZ + offsetY      )* TILE_SIZE_Z,
                                      (mX + offsetX + 0.5)* TILE_SIZE_X, avgHeight,
                                      (mZ + offsetY + 0.5)* TILE_SIZE_Z));
        offsetX+= 0.5;
        if (offsetX > 0.5)
        {
            offsetX = 0.0;
            offsetY+= 0.5;
        }
        if (!Test.first || Test.second > mDistance)
            continue;
        mDistance = Test.second;
        mSubtile = edge;
    }
/*
    // middle of the tile.
    Test = mouseRay.intersects(
               AxisAlignedBox((mX  + 0.1)* TILE_SIZE_X, avgHeight,
                              (mZ  + 0.1)* TILE_SIZE_Z,
                              (mX  + 0.9)* TILE_SIZE_X, avgHeight,
                              (mZ  + 0.9)* TILE_SIZE_Z));
    if (Test.first && Test.second < mDistance)
    {
        mDistance = Test.second;
        mSubtile  = -1;
Logger::log().error() << "hier2";
    }
*/
}

