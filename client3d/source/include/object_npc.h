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

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef NPC_H
#define NPC_H

#include "animate.h"
#include "logger.h"

using namespace Ogre;

// Mob => (M)oveable (Ob)ject.

/// /////////////////////////////////////////////////////////
/// Defines.
///
/// Mob => (M)oveable (Ob)ject.
/// /////////////////////////////////////////////////////////
const int BONE_WEAPON_HAND = 0, BONE_SHIELD_HAND = 1, BONE_HEAD = 2, BONE_BODY = 3;

/// /////////////////////////////////////////////////////////
/// Class.
/// /////////////////////////////////////////////////////////
class NPC
{
public:
  /// /////////////////////////////////////////////////////////
  /// Functions.
  /// /////////////////////////////////////////////////////////
  NPC(SceneManager *SceneMgr, SceneNode  *Node, const char *filename, float Facing);
  ~NPC()
  {}
  void moveToTile(int x, int z);
  void faceToTile(int x, int z);
  void walking(Real walk)
  {
    mWalking = walk;
  }
  void turning(Real turn)
  {
    mTurning = turn;
  }
  const Vector3 &getPos()
  {
    return mNode->getPosition();
  }
  const Vector3 &getWorldPos()
  {
    return mTranslateVector;
  }
  const SceneNode *getNode()
  {
    return mNode;
  }
  void update(const FrameEvent& event);
  void castSpell(int spell);
  void toggleTexture(int pos, int textureNr);
  void toggleMesh   (int pos, int WeaponNr);
  void toggleAnimGroup()
  {
    mAnim->toggleAnimGroup();
  }
  void toggleAnimation(int animationNr)
  {
    mAnim->toggleAnimation(animationNr);
  }
  Real getFacing()
  {
    return mFacing.valueRadians();
  }

protected:
  /// /////////////////////////////////////////////////////////
  /// Variables.
  /// /////////////////////////////////////////////////////////
  static unsigned int mInstanceNr; // mInstanceNr = Player's Hero
  unsigned int thisNPC;
  Real mWalking, mTurning;
  Degree mFacing, mNewFacing;
  int mPosTileX, mPosTileZ;
  int mWalkToX, mWalkToZ;
  bool mAutoTurning, mAutoMoving;
  SceneNode *mNode;
  Entity *mEntityNPC, *mEntityWeapon, *mEntityShield, *mEntityHelmet, *mEntityArmor;
  Vector3 mTranslateVector, mWalkToPos, mBoundingBox, mDeltaPos;
  Animate *mAnim;
  std::string mDescFile;
  SceneManager *mSceneMgr;
  Real animOffset; // every npc gets a random animation offset. preventing of  synchronous "dancing"

  /// /////////////////////////////////////////////////////////
  /// Functions.
  /// /////////////////////////////////////////////////////////
  NPC(const NPC&); // disable copy-constructor.
};

#endif
