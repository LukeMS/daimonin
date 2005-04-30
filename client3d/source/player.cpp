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

#include "sound.h"
#include "player.h"
#include "option.h"
#include "logfile.h"
#include "textwindow.h"

Player *player = 0;

//=================================================================================================
// Init stuff.
//=================================================================================================
Player::Player(SceneManager *SceneMgr, SceneNode  *Node, const char *filename):NPC(SceneMgr, Node, filename)
{
}

//=================================================================================================
// Update player.
//=================================================================================================
void Player::update(const FrameEvent& event)
{
    mAnim->update(event);
	mTranslateVector = Vector3(0,0,0);
	if (mAnim->isMovement())
	{
		if (mTurning)
		{
		    mFacing += Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mTurning);
			mNode->yaw(Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mTurning));
		}
		if (mWalking)
		{
			mAnim->toggleAnimation(STATE_WALK1);
	        mTranslateVector.y = -sin(mFacing.valueRadians()+mFacingOffset)* mAnim->getAnimSpeed() * mWalking;
		    mTranslateVector.x = -cos(mFacing.valueRadians()+mFacingOffset)* mAnim->getAnimSpeed() * mWalking;
		}
		else 
		{
            mAnim->toggleAnimation(STATE_IDLE1);
        }
	}
}
