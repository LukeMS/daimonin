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

#ifndef PLAYER_H
#define PLAYER_H

#include <Ogre.h>
#include <OgreConfigFile.h>
#include <OgreSceneManager.h>

using namespace Ogre;

enum AnimState
{
    STATE_IDLE1,	STATE_IDLE2,	STATE_IDLE3,
    STATE_WALK1,	STATE_WALK2,	STATE_WALK3,
    STATE_RUN1,		STATE_RUN2,		STATE_RUN3,
    STATE_ATTACK1,	STATE_ATTACK2,	STATE_ATTACK3,
    STATE_BLOCK1,	STATE_BLOCK2,	STATE_BLOCK3,
    STATE_SLUMP1,	STATE_SLUMP2,	STATE_SLUMP3,
    STATE_DEATH1,	STATE_DEATH2,	STATE_DEATH3,
    STATE_HIT1,		STATE_HIT2,		STATE_HIT3,
	STATE_SUM
};
/*
enum Weapon
{
	WEAPON_NONE,
	WEAPON_01, WEAPON_02,
	WEAPON_SUM
};

enum Shield
{
	SHIELD_NONE,
	SHIELD_01, SHIELD_02,
	SHIELD_SUM
};
*/
const Real PLAYER_ANIM_SPEED = 0.5f;
const Real PLAYER_TURN_SPEED = 2.0f;
const Real PLAYER_WALK_SPEED = 2.0f;
const Real RAD = 3.14159265/180.0;
const int WEAPON_HAND = 0, SHIELD_HAND = 1;

class Player
{
  private:
	int mAnimType;
	int mAnimGroup;
	Real mWalking, mTurning;
    Real _anim_speed;
	Real mFacingOffset;
	Radian mFacing;
    SceneNode  *mNode;
    SceneManager *mSceneMgr;
    Entity     *mEntityPlayer, *mEntityWeapon, *mEntityShield;
    Vector3 mTranslateVector;
	AnimationState *mAnimState;    
	AnimationState *mAnimStates[STATE_SUM];
	std::string mStrTemp;
    Player(const Player&); // disable copy-constructor.
  public:
    static Player &getSingleton() { static Player Singleton; return Singleton; }
	 Player() {;}
	~Player() {;}

	SceneNode *getNode() { return mNode; }
	bool Init(SceneManager *mSceneMgr);
	void walking(Real walk)  { mWalking = walk; }
	void turning(Real turn)  { mTurning = turn; }
	void toggleAnimGroup();
	void toggleWeapon(int Hand, int Weapon);
	const Vector3& getPos() { return mTranslateVector; }
    void updateAnim(const FrameEvent& event);
	void toggleAnimation(int animationNr);
};

#endif
