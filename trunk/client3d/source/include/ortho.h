/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    SkeletalAnimation.h
Description: Specialisation of OGRE's framework application to show the
             skeletal animation feature, including spline animation.
-----------------------------------------------------------------------------
*/

#include <OgreSceneManager.h>

#include "ExampleApplication.h"

enum PlayerState
{
    IDLE,
    WALK_FORWARD,
    WALK_BACKWARD
};

class Player
{
	Entity* entity;
    Real                                   _anim_speed;
    AnimationState*                        _anim_state;
    std::map<PlayerState, AnimationState*> _anim_states;
    SceneNode*                             _node;
    PlayerState                            _state;
    Real                                   _turn_speed;
    Real                                   _walk_speed;

    public:
    Player(SceneManager* scene_manager):
        _anim_speed(1), _node(scene_manager->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 0)))
    {
        entity = scene_manager->createEntity("player", "robot.mesh");
        this->_node->attachObject(entity);
        this->_node->yaw(Radian(Degree(230)));
        this->_anim_states[IDLE] = this->_anim_state = entity->getAnimationState("Idle");
        this->_anim_states[WALK_FORWARD] = this->_anim_states[WALK_BACKWARD] = entity->getAnimationState("Walk");
        this->_anim_state->setEnabled(true);
        this->_state = IDLE;
        this->_turn_speed = 2;
        this->_walk_speed = this->_anim_speed * 48;
    }

	Entity* returnEntitiy(void)
	{
		return entity;
	}

	void turnPlayer(int deg)
	{
        this->_node->yaw(Radian(Degree(deg)));
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

    Real getWalkSpeed()
    {
        return this->_walk_speed;
    }

    void nextFrame(const FrameEvent& event)
    {
        this->_anim_state->addTime(event.timeSinceLastFrame * this->_anim_speed);
    }
}* player;

SceneNode* center;
SceneNode* world;

// Event handler to animate
class OrthoTestFrameListener : public ExampleFrameListener
{
protected:
public:
    OrthoTestFrameListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }

    bool frameStarted(const FrameEvent& evt)
    {
        player->nextFrame(evt);
        mTranslateVector = center->getLocalAxes() * mTranslateVector;
        mTranslateVector.x *= -1;
        world->translate(mTranslateVector);

        // Call default
        return ExampleFrameListener::frameStarted(evt);

    }

    bool processUnbufferedKeyInput(const FrameEvent& evt)
    {
        bool proceed = true;

        if (player->getState() != IDLE && !mInputDevice->isKeyDown(KC_UP) && !mInputDevice->isKeyDown(KC_W) && !mInputDevice->isKeyDown(KC_DOWN) && !mInputDevice->isKeyDown(KC_S))
            player->changeState(IDLE);

        if (mInputDevice->isKeyDown(KC_UP) || mInputDevice->isKeyDown(KC_W) )
        {
            proceed = false;
            player->changeState(WALK_FORWARD);
            mTranslateVector.z = player->getWalkSpeed() * evt.timeSinceLastFrame;
        }

        if (mInputDevice->isKeyDown(KC_DOWN) || mInputDevice->isKeyDown(KC_S) )
        {
            proceed = false;
            player->changeState(WALK_BACKWARD);
            mTranslateVector.z = player->getWalkSpeed() * evt.timeSinceLastFrame;
        }

        if (mInputDevice->isKeyDown(KC_A) || mInputDevice->isKeyDown(KC_D) || mInputDevice->isKeyDown(KC_PGUP) || mInputDevice->isKeyDown(KC_PGDOWN))
        {
            proceed = false;
        }

        if (mInputDevice->isKeyDown(KC_RIGHT))
        {
            proceed = false;
			// center->yaw(Radian(player->getTurnSpeed() * evt.timeSinceLastFrame));
			player->turnPlayer(-1);
        }

        if (mInputDevice->isKeyDown(KC_LEFT))
        {
            proceed = false;
            //center->yaw(Radian(-player->getTurnSpeed() * evt.timeSinceLastFrame));
			player->turnPlayer(1);
        }

        if (proceed)
            return ExampleFrameListener::processUnbufferedKeyInput(evt);
        return true;
    }

    void moveCamera() {}
};



class OrthoTestApplication : public ExampleApplication
{
public:
    OrthoTestApplication() {}

protected:
    // Just override the mandatory create scene method
    void createScene(void)
    {
        center = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 0));
        world = center->createChildSceneNode(Vector3(0, 0, 0));

        // Setup animation default
        Animation::setDefaultInterpolationMode(Animation::IM_LINEAR);
        Animation::setDefaultRotationInterpolationMode(Animation::RIM_LINEAR);

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        player = new Player(mSceneMgr);

        Entity* ent = mSceneMgr->createEntity("floor", SceneManager::PT_PLANE);
        ent->setMaterialName("OrthoTest/floor");
        SceneNode* floor_node = world->createChildSceneNode(Vector3(0, 0, 0));
        floor_node->attachObject(ent);
        floor_node->pitch(Radian(Degree(-90)));
        floor_node->setScale(2, 2, 2);

        // Give it a little ambience with lights
        Light* l;
        l = mSceneMgr->createLight("BlueLight");
        l->setPosition(-200,-80,-100);
        l->setDiffuseColour(0.5, 0.5, 1.0);
        world->attachObject(l);

        l = mSceneMgr->createLight("GreenLight");
        l->setPosition(0,0,-100);
        l->setDiffuseColour(0.5, 1.0, 0.5);
        world->attachObject(l);

        // Report whether hardware skinning is enabled or not
        Technique* t =  player->returnEntitiy()->getSubEntity(0)->getMaterial()->getBestTechnique();
        Pass* p = t->getPass(0);
        if (p->hasVertexProgram() &&
            p->getVertexProgram()->isSkeletalAnimationIncluded())
        {
            mWindow->setDebugText("Hardware skinning is enabled");
            mWindow->setDebugText("2. Zeile");
        }
        else
        {
            mWindow->setDebugText("Software skinning is enabled");
        }




    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new OrthoTestFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

    void createCamera(void)
    {
        mCamera = mSceneMgr->createCamera("Camera");
        mCamera->setProjectionType(PT_ORTHOGRAPHIC);
        mCamera->setNearClipDistance(600);
        mCamera->setPosition(0, 600, 600);
        mCamera->lookAt(0, 0, 0);
    }
};
