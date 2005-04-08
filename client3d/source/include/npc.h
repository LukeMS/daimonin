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

#include <Ogre.h>
#include <OgreSceneManager.h>
#include "player.h"

using namespace Ogre;

class NPC
{
  private:
	static int mInstanceNr;
	Real mWalking, mTurning;
	int mAnimType;
    Real _anim_speed;
	Radian mFacing;
	Real mFacingOffset;
	int mAnimGroup;
    SceneNode  *mNode;
    Entity     *mEntity;
    Vector3 mTranslateVector;    
	AnimationState *mAnimState;    
	AnimationState *mAnimStates[STATE_SUM];
    NPC(const NPC&); // disable copy-constructor.
		  
  public:
	 NPC() {;}
	~NPC() {;}
	bool Init(SceneManager *SceneMgr, SceneNode  *Node);
	void walking(Real walk)  { mWalking = walk; }
	void turning(Real turn)  { mTurning = turn; }
	void playAnimation(int type) {if (mAnimType <0) mAnimType = type; }
	void toggleAnimaGroup(); 
	const Vector3& getPos() { return mTranslateVector; }
    void updateAnim(const FrameEvent& event);
    void updateTexture(int textureNr);
};

extern NPC *NPC_Enemy1;

#endif
