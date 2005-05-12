/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
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
-----------------------------------------------------------------------------
*/

#ifndef NPC_H
#define NPC_H

#include "animate.h"

using namespace Ogre;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////
const int WEAPON_HAND = 0, SHIELD_HAND = 1;

////////////////////////////////////////////////////////////
// Class.
////////////////////////////////////////////////////////////
class NPC
{
  protected:
    ////////////////////////////////////////////////////////////
	// Variables.
    ////////////////////////////////////////////////////////////
	static int mInstanceNr;
	Real mWalking, mTurning;
	Radian mFacing;
	Real mFacingOffset;
    SceneNode  *mNode;
    Entity *mEntityNPC, *mEntityWeapon, *mEntityShield;
    Vector3 mTranslateVector;    
    Animate *mAnim;
    std::string mDescFile;
    SceneManager *mSceneMgr;
    Real animOffset; // every npc gets a random animation offset. preventing of  synchronous "dancing"
    
    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////
    NPC(const NPC&); // disable copy-constructor.

  public:
  
    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////
	 NPC(SceneManager *SceneMgr, SceneNode  *Node, const char *filename);
	~NPC() {;}
	void walking(Real walk)  { mWalking = walk; }
	void turning(Real turn)  { mTurning = turn; }
	const Vector3& getPos() { return mTranslateVector; }
    void update(const FrameEvent& event);
    void toggleTexture(int pos, int textureNr);
    void toggleWeapon(int Hand, int WeaponNr);
    void toggleAnimGroup() { mAnim->toggleAnimGroup(); }
	void toggleAnimation(int animationNr) {mAnim->toggleAnimation(animationNr); }
};

#endif
