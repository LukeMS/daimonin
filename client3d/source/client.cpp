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

#include <Ogre.h>
#include <OgreConfigFile.h>
#include <OgreSceneManager.h>

#include "event.h"
#include "player.h"
#include "client.h"
#include "network.h"
#include "logfile.h"
#include "textwindow.h"
#include "dialog.h"
#include "option.h"
#include "sound.h"

using namespace Ogre;

// ========================================================================
// Start the example
// ========================================================================
void DaimoninClient::go(void)
{
    if (!setup()) { return; }
    mRoot->startRendering();
    // clean up
    destroyScene();
}

// ========================================================================
// These internal methods package up the stages in the startup process
// Sets up the application - returns false if user abandon configuration.
// ========================================================================

bool DaimoninClient::setup(void)
{
	LogFile::getSingelton().Init("client_log.html");
	Option ::getSingelton().Init("options.dat");
	Sound  ::getSingelton().Init();	
	Network::getSingelton().Init();
	mRoot = new Root();
	setupResources();

	///////////////////////////////////////////////////////////////////////// 
	// Show the configuration dialog and initialise the system
	// You can skip this and use root.restoreConfig() to load configuration
	// settings if you were sure there are valid ones saved in ogre.cfg
	///////////////////////////////////////////////////////////////////////// 
	if(mRoot->showConfigDialog())
	{
		mWindow = mRoot->initialise(true);
	}
	else return false;


    ///////////////////////////////////////////////////////////////////////// 
    // Get the SceneManager, in this case a generic one
	/////////////////////////////////////////////////////////////////////////
    mSceneMgr = mRoot->getSceneManager(ST_GENERIC);
    ///////////////////////////////////////////////////////////////////////// 
    // Create a camera
	/////////////////////////////////////////////////////////////////////////
    mCamera = mSceneMgr->createCamera("Camera");
    mCamera->setProjectionType(PT_ORTHOGRAPHIC);
    mCamera->setNearClipDistance(400);
    mCamera->setPosition(0, 400, 400);
    mCamera->lookAt(0, 0, 0);

	/////////////////////////////////////////////////////////////////////////
    // Create one viewport, entire window
	/////////////////////////////////////////////////////////////////////////
    mVP = mWindow->addViewport(mCamera);
    mVP->setBackgroundColour(ColourValue(0,0,0));
    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(Real(mVP->getActualWidth()) / Real(mVP->getActualHeight()));

    ///////////////////////////////////////////////////////////////////////// 
    // Set default mipmap level (NB some APIs ignore this)
    ///////////////////////////////////////////////////////////////////////// 
    TextureManager::getSingleton().setDefaultNumMipmaps(5);

    ///////////////////////////////////////////////////////////////////////// 
    // Optional override method where you can perform resource group loading
    // Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    // Initialise, parse scripts etc
    ///////////////////////////////////////////////////////////////////////// 
    ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    mEvent= new Event(mWindow, mCamera, mMouseMotionListener, mMouseListener);
    mRoot->addFrameListener(mEvent);
    mEvent->setResolution(mVP->getActualWidth(), mVP->getActualHeight());
    createScene();            // Create the scene
	return true;
}

// ========================================================================
// Method which will define the source of resources (other than current folder)
// ========================================================================
void DaimoninClient::setupResources(void)
{
    // Load resource paths from config file
    ConfigFile cf;
    cf.load("resources.cfg");

    // Go through all sections & settings in the file
    ConfigFile::SectionIterator seci = cf.getSectionIterator();

    String secName, typeName, archName;
    while (seci.hasMoreElements())
	{
        secName = seci.peekNextKey();
        ConfigFile::SettingsMultiMap *settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
		{
            typeName = i->first;
             archName = i->second;
             ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
		}
	}
}

// ========================================================================
// 
// ========================================================================
void DaimoninClient::createScene(void)
{
    // Setup animation default
    Animation::setDefaultInterpolationMode(Animation::IM_LINEAR);
    Animation::setDefaultRotationInterpolationMode(Animation::RIM_LINEAR);

    mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

	mEvent->World = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 0));
    Player::getSingelton().Init(
		mSceneMgr->createEntity("player", "robot.mesh"),
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 0))
		);


    Light* l;
    l = mSceneMgr->createLight("BlueLight");
    l->setPosition(-200,-80,-100);
    l->setDiffuseColour(0.5, 0.5, 1.0);
	mEvent->World->attachObject(l);

    l = mSceneMgr->createLight("GreenLight");
    l->setPosition(0,0,-100);
    l->setDiffuseColour(0.5, 1.0, 0.5);
    mEvent->World->attachObject(l);


    Entity* ent;
    SceneNode* floor_node;

		// create floor-tile.
        ent = mSceneMgr->createEntity("dfgd", SceneManager::PT_PLANE);
        ent->setMaterialName("grass1");
        floor_node = mEvent->World->createChildSceneNode(Vector3(0, 0, 0));
        floor_node->attachObject(ent);
        floor_node->pitch(Radian(Degree(-90)));
        floor_node->setScale(0.25, 0.25, 0.25);

        ent = mSceneMgr->createEntity("234", SceneManager::PT_PLANE);
        ent->setMaterialName("grass2");
        floor_node = mEvent->World->createChildSceneNode(Vector3(50, 0, 0));
        floor_node->attachObject(ent);
        floor_node->pitch(Radian(Degree(-90)));
        floor_node->setScale(0.25, 0.25, 0.25);

        ent = mSceneMgr->createEntity("s3", SceneManager::PT_PLANE);
        ent->setMaterialName("grass3");
        floor_node = mEvent->World->createChildSceneNode(Vector3(25, 1, 25));
        floor_node->attachObject(ent);
        floor_node->pitch(Radian(Degree(-90)));
        floor_node->setScale(0.25, 0.25, 0.25);

    
		
    ent = mSceneMgr->createEntity("hier", SceneManager::PT_PLANE);
    ent->setMaterialName("stone1");
    floor_node = mEvent->World->createChildSceneNode(Vector3(60, 0, 60));
    floor_node->attachObject(ent);
    floor_node->pitch(Radian(Degree(-90)));
    floor_node->setScale(0.2, 0.2, 0.2);
    floor_node->roll(Radian(Degree(-45))); // rotate around z-axis.
	

    Entity* ent1 = ent->clone("test");
    ent1->setMaterialName("stone1");
    SceneNode* floor_node1;
    floor_node1 = mEvent->World->createChildSceneNode(Vector3(88, 0, 88));
    floor_node1->attachObject(ent1);
    floor_node1->pitch(Radian(Degree(-90)));
    floor_node1->setScale(0.2, 0.2, 0.2);
    floor_node1->roll(Radian(Degree(-45))); // rotate around z-axis.


    Entity* ent2 = ent->clone("test2");
    ent2->setMaterialName("stone1");
    SceneNode* floor_node2;
    floor_node2 = mEvent->World->createChildSceneNode(Vector3(130, 0, 130));
    floor_node2->attachObject(ent2);
    floor_node2->pitch(Radian(Degree(-90)));
    floor_node2->setScale(0.2, 0.2, 0.2);
    floor_node2->roll(Radian(Degree(-45))); // rotate around z-axis.

}

// ========================================================================
// 
// ========================================================================
void DaimoninClient::destroyScene(void)
{
}
