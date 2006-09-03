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
this program; if not, write to the Free Sftware Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef TILE_INTERFACE_H
#define TILE_INTERFACE_H

#include "define.h"
#include "Ogre.h"

using namespace Ogre;

class TileInterface
{
public:
    TileInterface(SceneManager* sceneManager);
    ~TileInterface();
    const Vector3 getSelectedPos();
    const SubPos2D getSelectedTile();
    Vector3 tileToWorldPos(SubPos2D tile);
    int calcTileDistance(const SubPos2D &pos1, const SubPos2D &pos2);
    void pickTile(float mMouseX, float mMouseY);

private:
    enum
    {
        QUADRANT_LEFT,  QUADRANT_TOP,
        QUADRANT_RIGHT, QUADRANT_BOTTOM,
    }
    quadrant;
    static const unsigned char mSubPosTable[][32];
    static const unsigned char mWorldPosTable[4][8];
    RaySceneQuery* mRaySceneQuery;
    SubPos2D mPos;
    Vector3 mPosVector;
    Vector3 mTris[4];

    bool getPickPos(Ray *mouseRay, int quadrant);
    void fillVectors(SubPos2D &tile, int quadrant);
};

#endif
