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
#include "sound.h"

#include "logfile.h"

using namespace Ogre;

enum PlayerState
{
    IDLE,
    WALK
};

const Real PLAYER_ANIM_SPEED = 0.5f;
const Real PLAYER_TURN_SPEED = 2.0f;
const Real PLAYER_WALK_SPEED = 2.0f;

class Player
{
  private:
	Real mWalking;
	Real mTurning;
    Real     _anim_speed;
	Radian   mFacing;
	Real mIdleTime;
    SceneNode  *mNode;
    Entity     *mEntity;
    Vector3 mTranslateVector;    
	AnimationState* mAnimState;    
	std::map<PlayerState, AnimationState*> mAnimStates;

    Player(const Player&); // disable copy-constructor.
  public:

	 Player() {;}
	~Player() {;}

	void Init(Entity* entity, SceneNode *node) 
    {
		mIdleTime = 0;
		mFacing = Degree(90);		
		mEntity = entity;
		mNode   = node;
        mNode->attachObject(mEntity);
		mNode->setScale(0.5, 0.5, 0.5);
        mNode->yaw(Radian(Degree(mFacing)));
		mAnimStates[IDLE]  = entity->getAnimationState("Idle");
		mAnimStates[WALK]  = entity->getAnimationState("Walk");
	}

	void walking(Real walk)  { mWalking = walk; }
	void turning(Real turn)  { mTurning = turn; }	
	const Vector3& getPos() { return mTranslateVector; }
    static Player &getSingelton() { static Player singelton; return singelton; }

	
    void updateAnim(const FrameEvent& event)
    {
		if (mTurning)
		{
	        mFacing += Degree(mTurning);
		    mNode->yaw(Radian(Degree(mTurning)));
		}

		if (mWalking)
		{
            mTranslateVector.z =  sin(mFacing.valueRadians())* mWalking;
	        mTranslateVector.x = -cos(mFacing.valueRadians())* mWalking;
			mAnimState= mAnimStates[WALK];
			mAnimState->addTime(event.timeSinceLastFrame * mWalking);
		}
		else 
		{
			mTranslateVector = Vector3::ZERO;
			mAnimState= mAnimStates[IDLE];
			mAnimState->addTime(event.timeSinceLastFrame * PLAYER_ANIM_SPEED);
		}
        mAnimState->setEnabled(true);
		mIdleTime += event.timeSinceLastFrame;
		if (mIdleTime > 10.0)
		{ 
			Sound::getSingelton().PlaySample(SAMPLE_PLAYER_IDLE); 
			mIdleTime = -120;
		}
	}
};

#endif
