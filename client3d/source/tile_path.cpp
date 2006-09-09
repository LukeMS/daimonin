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



#include "define.h"
#include "tile_path.h"
#include "logger.h"

/*
const int MAX_RECURSION = 100;  // Prevent deadlocks.

static const int travCost[]=
    {
        DIAGONAL, STRAIGHT, DIAGONAL, STRAIGHT, STRAIGHT, DIAGONAL, STRAIGHT, DIAGONAL
    };
static const int xOffset[] =
    {
        0, 1, 1, 1, 0,-1,-1,-1
    };
static const int yOffset[] =
    {
        -1,-1, 0, 1, 1, 1, 0,-1
    };
*/


// This code is based on the A* tutorial from Patrick Lester.
// It will be completely rewritten as soon as find the time for it.

// TODO:
// - facing value must be included.
// - terrain type value must be included.
// - terrain height value must be included.



const int STRAIGHT = 392; // 100000/255.
const int DIAGONAL = 554; // sqrt(2) * 100000/255.

//================================================================================================
// Find a path (using A*).
//================================================================================================
TilePath::TilePath()
{
    pathBank = 0;
}

//================================================================================================
// Find a path (using A*).
//================================================================================================
TilePath::~TilePath()
{
    delete[] pathBank;
}

//================================================================================================
// Find a path (using A*).
//================================================================================================
bool TilePath::FindPath(SubPos2D posStart, SubPos2D posDest, int precision)
{
    mPrecision = precision;
    posStart.subX+= posStart.x* SUM_SUBTILES;
    posStart.subZ+= posStart.z* SUM_SUBTILES;
    posDest.subX += posDest.x * SUM_SUBTILES;
    posDest.subZ += posDest.z * SUM_SUBTILES;

    int onOpenList=0, parentXval=0, parentYval=0;
    int a=0, b=0, m=0, u=0, v=0, temp=0, numberOfOpenListItems=0;
    int tempGcost = 0;
    int tempx, pathX, pathY, cellPosition, newOpenListItemID=0;

    // Under the some circumstances no path needs to be generated.
    if (posStart == posDest)
    {
        return false;
    }

    // Is target subtile unwalkable?
    if (!TileManager::getSingleton().getWalkablePos(posDest.subX, posDest.subZ)) return false;

    memset(whichList, 0, sizeof(whichList));
    onClosedList= 1;
    onOpenList  = 2;
    pathLocation= 0;
    Gcost[posStart.subX][posStart.subZ] = 0; //reset starting square's G value to 0

    //4.Add the starting location to the open list of squares to be checked.
    numberOfOpenListItems = 1;
    openList[1] = 1;//assign it as the top (and currently only) item in the open list, which is maintained as a binary heap (explained below)
    openX[1] = posStart.subX ;
    openY[1] = posStart.subZ;
    int iterations =0;
    const int MAX_ITERATIONS = 10000;
    while (++iterations < MAX_ITERATIONS)
    {
        if (!numberOfOpenListItems)
        {
            pathStatus = false;
            break;
        }

        //7. Pop the first item off the open list.
        parentXval = openX[openList[1]];
        parentYval = openY[openList[1]]; //record cell coordinates of the item
        whichList[parentXval][parentYval] = onClosedList;//add the item to the closed list

        // Open List = Binary Heap: Delete this item from the open list, which
        //  is maintained as a binary heap. For more information on binary heaps, see:
        // http://www.policyalmanac.org/games/binaryHeaps.htm
        --numberOfOpenListItems;

        // Delete the top item in binary heap and reorder the heap, with the lowest F cost item rising to the top.
        openList[1] = openList[numberOfOpenListItems+1];//move the last item in the heap up to slot #1
        v = 1;
        // Repeat the following until the new item in slot #1 sinks to its proper spot in the heap.
        while (1)
        {
            u = v;
            if (2*u+1 <= numberOfOpenListItems) //if both children exist
            {
                //Check if the F cost of the parent is greater than each child.
                //Select the lowest of the two children.
                if (Fcost[openList[u]] >= Fcost[openList[2*u]])
                    v = 2*u;
                if (Fcost[openList[v]] >= Fcost[openList[2*u+1]])
                    v = 2*u+1;
            }
            else
            {
                if (2*u <= numberOfOpenListItems) //if only child #1 exists
                {
                    //Check if the F cost of the parent is greater than child #1
                    if (Fcost[openList[u]] >= Fcost[openList[2*u]])
                        v = 2*u;
                }
            }

            if (u != v) //if parent's F is > one of its children, swap them
            {
                temp = openList[u];
                openList[u] = openList[v];
                openList[v] = temp;
            }
            else
                break; //otherwise, exit loop

        }

        //7.Check the adjacent squares. (Its "children" -- these path children
        // are similar, conceptually, to the binary heap children mentioned
        // above, but don't confuse them. They are different. Path children
        // are portrayed in Demo 1 with grey pointers pointing toward
        // their parents.) Add these adjacent child squares to the open list
        // for later consideration if appropriate (see various if statements
        // below).
        for (b = parentYval-1; b <= parentYval+1; b++)
        {
            for (a = parentXval-1; a <= parentXval+1; a++)
            {
                // If not off the map (do this first to avoid array out-of-bounds errors)
                if ((a < 0 || b < 0 || a >= mapWidth || b >= mapHeight)
                        // Is on close list - can be ignored.
                        || (whichList[a][b] == onClosedList)
                        // Is not walkable - can be ignored.
                        || (!TileManager::getSingleton().getWalkablePos(a, b)))
                    continue;

                // Don't cut across corners
                if (a == parentXval-1)
                {
                    if (b == parentYval-1)
                    {
                        if (!TileManager::getSingleton().getWalkablePos(parentXval-1,parentYval)
                                || !TileManager::getSingleton().getWalkablePos(parentXval, parentYval-1))
                            continue;
                    }
                    else if (b == parentYval+1)
                    {
                        if (!TileManager::getSingleton().getWalkablePos(parentXval,parentYval+1)
                                || !TileManager::getSingleton().getWalkablePos(parentXval-1,parentYval))
                            continue;
                    }
                }
                else if (a == parentXval+1)
                {
                    if (b == parentYval-1)
                    {
                        if (!TileManager::getSingleton().getWalkablePos(parentXval, parentYval-1)
                                || !TileManager::getSingleton().getWalkablePos(parentXval+1, parentYval))
                            continue;
                    }
                    else if (b == parentYval+1)
                    {
                        if (!TileManager::getSingleton().getWalkablePos(parentXval+1, parentYval)
                                || !TileManager::getSingleton().getWalkablePos(parentXval, parentYval+1))
                            continue;
                    }
                }



                // If not already on the open list, add it to the open list.
                if (whichList[a][b] != onOpenList)
                {
                    //Create a new open list item in the binary heap.
                    newOpenListItemID = newOpenListItemID + 1; //each new item has a unique ID #
                    m = numberOfOpenListItems+1;
                    openList[m] = newOpenListItemID;//place the new open list item (actually, its ID#) at the bottom of the heap
                    openX[newOpenListItemID] = a;
                    openY[newOpenListItemID] = b;//record the x and y coordinates of the new item

                    //Figure out its G cost
                    if (abs(a-parentXval) == 1 && abs(b-parentYval) == 1)
                        Gcost[a][b] = Gcost[parentXval][parentYval] + DIAGONAL;
                    else
                        Gcost[a][b] = Gcost[parentXval][parentYval] + STRAIGHT;


                    //Figure out its H and F costs and parent
                    Hcost[openList[m]] = 10*(abs(a - posDest.subX) + abs(b - posDest.subZ));
                    Fcost[openList[m]] = Gcost[a][b] + Hcost[openList[m]];
                    parentX[a][b] = parentXval ;
                    parentY[a][b] = parentYval;

                    //Move the new open list item to the proper place in the binary heap.
                    //Starting at the bottom, successively compare to parent items,
                    //swapping as needed until the item finds its place in the heap
                    //or bubbles all the way to the top (if it has the lowest F cost).
                    while (m != 1) //While item hasn't bubbled to the top (m=1)
                    {
                        //Check if child's F cost is < parent's F cost. If so, swap them.
                        if (Fcost[openList[m]] <= Fcost[openList[m/2]])
                        {
                            temp = openList[m/2];
                            openList[m/2] = openList[m];
                            openList[m] = temp;
                            m = m/2;
                        }
                        else
                            break;
                    }
                    ++numberOfOpenListItems;

                    //Change whichList to show that the new item is on the open list.
                    whichList[a][b] = onOpenList;
                }

                //8.If adjacent cell is already on the open list, check to see if this
                // path to that cell from the starting location is a better one.
                // If so, change the parent of the cell and its G and F costs.
                else //If whichList(a,b) = onOpenList
                {
                    //Figure out the G cost of this possible new path
                    if (abs(a-parentXval) == 1 && abs(b-parentYval) == 1)
                        tempGcost = Gcost[parentXval][parentYval] + DIAGONAL;
                    else
                        tempGcost = Gcost[parentXval][parentYval] + STRAIGHT;

                    //If this path is shorter (G cost is lower) then change
                    //the parent cell, G cost and F cost.
                    if (tempGcost < Gcost[a][b]) //if G cost is less,
                    {
                        parentX[a][b] = parentXval; //change the square's parent
                        parentY[a][b] = parentYval;
                        Gcost[a][b] = tempGcost;//change the G cost

                        //Because changing the G cost also changes the F cost, if
                        //the item is on the open list we need to change the item's
                        //recorded F cost and its position on the open list to make
                        //sure that we maintain a properly ordered open list.
                        for (int x = 1; x <= numberOfOpenListItems; x++) //look for the item in the heap
                        {
                            if (openX[openList[x]] == a && openY[openList[x]] == b) //item found
                            {
                                Fcost[openList[x]] = Gcost[a][b] + Hcost[openList[x]];//change the F cost

                                //See if changing the F score bubbles the item up from it's current location in the heap
                                m = x;
                                while (m != 1) //While item hasn't bubbled to the top (m=1)
                                {
                                    //Check if child is < parent. If so, swap them.
                                    if (Fcost[openList[m]] < Fcost[openList[m/2]])
                                    {
                                        temp = openList[m/2];
                                        openList[m/2] = openList[m];
                                        openList[m] = temp;
                                        m = m/2;
                                    }
                                    else
                                        break;
                                }
                                break; //exit for x = loop
                            } //If openX(openList(x)) = a
                        } //For x = 1 To numberOfOpenListItems

                    }//If tempGcost < Gcost(a,b)
                }//else If whichList(a,b) = onOpenList
            }//for (a = parentXval-1; a <= parentXval+1; a++){
        }//for (b = parentYval-1; b <= parentYval+1; b++){

        //If target is added to open list then path has been found.
        if (whichList[posDest.subX][posDest.subZ] == onOpenList)
        {
            pathStatus = true;
            break;
        }
    }
    if (iterations == MAX_ITERATIONS)
    {
        Logger::log().error() << "Could not find a path";
        pathStatus = false;
        return pathStatus;
    }

    //10.Save the path if it exists.
	pathLength  = 0;
    if (!pathStatus) return false;
        //a.Working backwards from the target to the starting location by checking
        // each cell's parent, figure out the length of the path.
        pathX = posDest.subX;
        pathY = posDest.subZ;
        do
        {
            //Look up the parent of the current cell.
            tempx = parentX[pathX][pathY];
            pathY = parentY[pathX][pathY];
            pathX = tempx;

            //Figure out the path length
            ++pathLength;
        }
        while (pathX != posStart.subX || pathY != posStart.subZ);
		//b.Resize the data bank to the right size in bytes
        delete[] pathBank;
        pathBank = new int[pathLength*8];
        //c. Now copy the path information over to the databank. Since we are
        // working backwards from the target to the start location, we copy
        // the information to the data bank in reverse order. The result is
        // a properly ordered set of path data, from the first step to the last.

        pathX = posDest.subX ;
        pathY = posDest.subZ;
        cellPosition = pathLength*2;//start at the end
        do
        {
            cellPosition = cellPosition - 2;//work backwards 2 integers
            pathBank[cellPosition  ] = pathX;
            pathBank[cellPosition+1] = pathY;
            //d.Look up the parent of the current cell.
            tempx = parentX[pathX][pathY];
            pathY = parentY[pathX][pathY];
            pathX = tempx;
        }
        while (pathX != posStart.subX || pathY != posStart.subZ);
        //11.Read the first path step into xPath/yPath arrays
        ReadPath();
    return pathStatus;
}

//================================================================================================
// Read the next path step.
//================================================================================================
bool TilePath::ReadPath()
{
    if (pathStatus == true)
    {
        if (++pathLocation >= pathLength - mPrecision)
        {
            pathStatus = false;
        }
        else
        {
            xPath = pathBank[pathLocation*2-2];
            yPath = pathBank[pathLocation*2-1];
        }
    }
    return pathStatus;
}
