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

#ifndef TILE_PATH_H
#define TILE_PATH_H

#include "tile_manager.h"

const int mapWidth  = CHUNK_SIZE_X*8;
const int mapHeight = CHUNK_SIZE_Z*8;

class TilePath
{
public:
    TilePath();
    ~TilePath();
    /**< Find a path (A* is used for this). **/
    bool FindPath (TilePos posStart, TilePos posDest, int precision =0);
    /**< Read a step from the path. **/
    bool ReadPath();

    int xPath;
    int yPath;

private:
    int openList[mapWidth*mapHeight+2]; //1d array holding ID# of open list items
    int openX   [mapWidth*mapHeight+2]; //1d array stores the x location of an item on the open list
    int openY   [mapWidth*mapHeight+2]; //1d array stores the y location of an item on the open list
    int Fcost   [mapWidth*mapHeight+2]; //1d array to store F cost of a cell on the open list
    int Hcost   [mapWidth*mapHeight+2]; //1d array to store H cost of a cell on the open list

    char walkability[mapWidth][mapHeight];

    int whichList[mapWidth+1][mapHeight+1];
    int parentX  [mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (x)
    int parentY  [mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (y)
    int Gcost    [mapWidth+1][mapHeight+1]; //2d array to store G cost for each cell.

    int onClosedList;
    int mPrecision;
    int pathLength;
    int pathLocation;
    int *pathBank;
    bool pathStatus;
};

#endif
