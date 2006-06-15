/*-----------------------------------------------------------------------------
This source file is part of Code-Black (http://www.code-black.org)
Copyright (c) 2005 by the Code-Black Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------*/

#include "tile_interface.h"
#include "tile_manager.h"
#include "logger.h"

///================================================================================================
///
///================================================================================================
const Vector3 TileInterface::get_Selection()
{
    Vector3 tmp;
    if (mX <0 || mZ <0)
    {
        tmp.x = 0;
        tmp.z = 0;
        tmp.y = 20;
    }
    else
    {
        tmp.x = mX;
        tmp.z = mZ;
        tmp.y = (Real) (TileManager::getSingleton().getAvgMapHeight(mX, mZ));
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
void TileInterface::pick_Tile(float mouseX, float mouseY)
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
    /// we start with a given tile chunk (coordinates a and b) and a ray
    /// and try to find the tile that was selected. we have to check every
    /// tile if it was hit and return the nearest one
    /// ////////////////////////////////////////////////////////////////////
    Real height[4], avgHeight;
    Real offsetX, offsetY;
    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int y = 0; y < CHUNK_SIZE_Z; ++y)
        {
            /// ////////////////////////////////////////////////////////////////////
            /// we have to build a bounding box for each tile and check if the ray
            /// intersects this box.
            /// To do this, we need the height of the tile  vertices.
            /// ////////////////////////////////////////////////////////////////////
            height[0] = TileManager::getSingleton().getMapHeight(x    , y    );
            height[1] = TileManager::getSingleton().getMapHeight(x + 1, y    );
            height[2] = TileManager::getSingleton().getMapHeight(x    , y + 1);
            height[3] = TileManager::getSingleton().getMapHeight(x + 1, y + 1);
            avgHeight = (height[0]+height[1]+height[2]+height[3]) /4.0;
            /// ////////////////////////////////////////////////////////////////////
            /// now we build 4 bounding boxes per tile to increase picking accuracy
            /// Note: Ogre only allows bounding boxes with the first vector having
            /// got the lower value in every(!) component. so we have to check
            /// which height value is greater
            /// ////////////////////////////////////////////////////////////////////
            std::pair<bool, Real> Test;
            offsetX =0, offsetY=0;
            for (int edge=0; edge < 4; ++edge)
            {
                if (height[edge] > avgHeight)
                    Test = mouseRay.intersects(
                               AxisAlignedBox((x + offsetX      )* TILE_SIZE_X, avgHeight,
                                              (y + offsetY      )* TILE_SIZE_Z,
                                              (x + offsetX + 0.5)* TILE_SIZE_X, height[edge],
                                              (y + offsetY + 0.5)* TILE_SIZE_Z));
                else
                    Test = mouseRay.intersects(
                               AxisAlignedBox((x + offsetX      )* TILE_SIZE_X, height[edge],
                                              (y + offsetY      )* TILE_SIZE_Z,
                                              (x + offsetX + 0.5)* TILE_SIZE_X, avgHeight,
                                              (y + offsetY + 0.5)* TILE_SIZE_Z));
                offsetX+= 0.5;
                if (offsetX > 0.5)
                {
                    offsetX = 0.0;
                    offsetY+= 0.5;
                }
                if (Test.first == true)
                { /// intersection! Find the closest intersection to the camera.
                    if (Test.second < mDistance)
                    {
                        mDistance = Test.second;
                        mX  = x;
                        mZ  = y;
                    }
                }
            }
        }
    }
}

