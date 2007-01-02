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

#ifndef TILEPOS_H
#define TILEPOS_H

//================================================================================================
//
//================================================================================================
typedef class TilePos
{
public:
    TilePos()
    {}
    TilePos(const int fX, const int fZ, const int fsubX, const int fsubZ)
            : x(fX), z(fZ), subX(fsubX), subZ(fsubZ)
    {}
    ~TilePos()
    {}
    bool operator == (const TilePos& tst) const
    {
        return (x == tst.x && z == tst.z && subX == tst.subX && subZ == tst.subZ);
    }
    bool operator != (const TilePos& tst) const
    {
        return (x != tst.x || z != tst.z || subX != tst.subX || subZ != tst.subZ);
    }
    void operator += (const TilePos& add)
    {
        x += add.x;
        z += add.z;
        subX += add.subX;
        subZ += add.subZ;
    }
    void operator -= (const TilePos& sub)
    {
        x -= sub.x;
        z -= sub.z;
        subX -= sub.subX;
        subZ -= sub.subZ;
    }
    TilePos operator + (const TilePos& add)
    {
        return TilePos(
                   x + add.x,
                   z + add.z,
                   subX + add.subX,
                   subZ + add.subZ);
    }
    TilePos operator - (const TilePos& sub)
    {
        return TilePos(
                   x - sub.x,
                   z - sub.z,
                   subX - sub.subX,
                   subZ - sub.subZ);
    }

    int x, z;       /**< Position of the tile. **/
    int subX, subZ; /**< Position within the tile. **/
}
TilePos;

#endif
