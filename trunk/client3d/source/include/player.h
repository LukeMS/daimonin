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

#include "ExampleFrameListener.h"

using namespace Ogre;

enum PlayerState
{
    IDLE,
    WALK_FORWARD,
    WALK_BACKWARD
};

class Player
{
    Real                                   _anim_speed;
    AnimationState*                        _anim_state;
    std::map<PlayerState, AnimationState*> _anim_states;
    SceneNode*                             _node;
    PlayerState                            _state;
    Real                                   _turn_speed;
    Real                                   _walk_speed;
	Radian   mFacing;

  public:

	Player(SceneManager* scene_manager):
        _anim_speed(1), _node(scene_manager->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 0)))
    {
		mFacing = Degree(90);
		Entity* entity = scene_manager->createEntity("player", "robot.mesh");
        this->_node->attachObject(entity);
		this->_node->setScale(0.5, 0.5, 0.5);
        this->_node->yaw(Radian(Degree(mFacing)));
		this->_anim_states[IDLE] = this->_anim_state = entity->getAnimationState("Idle");
        this->_anim_states[WALK_FORWARD] = this->_anim_states[WALK_BACKWARD] = entity->getAnimationState("Walk");
        this->_anim_state->setEnabled(true);
        this->_state = IDLE;
        this->_turn_speed = 2;
        this->_walk_speed = this->_anim_speed * 48;
    }

    void changeState(PlayerState state)
    {
        if (state != this->_state)
        {
            switch(state)
            {
                case IDLE:
                if (this->_anim_speed < 0)
                {
                    this->_anim_speed *= -1;
                    this->_walk_speed *= -1;
                }
                this->_anim_speed /= 2;
                break;

                case WALK_FORWARD:
                if (this->_anim_speed < 0)
                {
                    this->_anim_speed *= -1;
                    this->_walk_speed *= -1;
                }
                if (this->_state == IDLE)
                    this->_anim_speed *= 2;
                break;

                case WALK_BACKWARD:
                if (this->_anim_speed > 0)
                {
                    this->_anim_speed *= -1;
                    this->_walk_speed *= -1;
                }
                if (this->_state == IDLE)
                    this->_anim_speed *= 2;
                break;
            }
            this->_anim_state = this->_anim_states[state];
            this->_anim_state->setEnabled(true);
            this->_state = state;
        }
    }

    PlayerState getState()
    {
        return this->_state;
    }

    Real getTurnSpeed()
    {
        return this->_turn_speed;
    }

    void setFacing(Real deg)
    {
        mFacing += Degree(deg);
        this->_node->yaw(Radian(Degree(deg)));
    }

    Radian *getFacing()
    {
        return &mFacing;
    }

    Real getWalkSpeed()
    {
        return this->_walk_speed;
    }

    void nextFrame(const FrameEvent& event)
    {
        this->_anim_state->addTime(event.timeSinceLastFrame * this->_anim_speed);
    }
};

extern Player* player;


#endif
