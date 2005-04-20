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

#include "define.h"
#include "event.h"
#include "player.h"
#include "npc.h"
#include "client.h"
#include "network.h"
#include "logfile.h"
#include "textwindow.h"
#include "dialog.h"
#include "option.h"
#include "sound.h"
#include "tile_gfx.h"
#include "tile_map.h"

using namespace Ogre;

// ========================================================================
// Start the example
// ========================================================================
DaimoninClient::DaimoninClient()
{
	mRoot  = 0;
}

// ========================================================================
// Start the example
// ========================================================================
DaimoninClient::~DaimoninClient()
{
	if (Event)       { delete Event; }
	if (mRoot)       { delete mRoot;  }
	if (NPC_Enemy1)  { delete NPC_Enemy1; }
}


// ========================================================================
// Start the example
// ========================================================================
void DaimoninClient::go(void)
{
    if (!setup()) { return; }
 	Sound::getSingleton().playSong(FILE_MUSIC_001);
    mRoot->startRendering();
    // Clean up all Ogre-Ptr.
	TileMap::getSingleton().freeRecources();     
}

// ========================================================================
// These internal methods package up the stages in the startup process
// Sets up the application - returns false if user abandon configuration.
// ========================================================================
bool DaimoninClient::setup(void)
{
	LogFile::getSingleton().Init();
    if (TileGfx::getSingleton().read_bmaps_p0() <0) return false; 
    TileGfx::getSingleton().read_bmap_tmp(); // only testing.NORMALLY started from netword.cpp.

	Option ::getSingleton().Init();
	Sound  ::getSingleton().Init();
	Network::getSingleton().Init();

	mRoot = new Root();

	setupResources();
		
	/////////////////////////////////////////////////////////////////////////
	// Show the configuration dialog and initialise the system
	// You can skip this and use root.restoreConfig() to load configuration
	// settings if you were sure there are valid ones saved in ogre.cfg
	/////////////////////////////////////////////////////////////////////////
	if(mRoot->showConfigDialog()) { mWindow = mRoot->initialise(true); }
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
    TextureManager::getSingleton().setDefaultNumMipmaps(3);
    /////////////////////////////////////////////////////////////////////////
    // Optional override method where you can perform resource group loading
    // Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    // Initialise, parse scripts etc
    /////////////////////////////////////////////////////////////////////////
    ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    Event= new CEvent(mWindow, mCamera, mMouseMotionListener, mMouseListener);
    mRoot->addFrameListener(Event);
    Event->setResolutionMember(mVP->getActualWidth(), mVP->getActualHeight());

    createScene();
	return true;
}

// ========================================================================
// Define the source of resources (other than current folder)
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
// Create all Elements of the Scene.
// ========================================================================
void DaimoninClient::createScene(void)
{
	// Create the world.
	Event->World = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 0), Quaternion(1.0,0.0,0.0,0.0));
//	Event->World->setPosition(0,0,0);
    mSceneMgr->setAmbientLight(ColourValue(0, 0, 0));
//    mSceneMgr->setAmbientLight(ColourValue(1.0, 1.0, 1.0));

    Light *light;
    light = mSceneMgr->createLight("Light_Vol");
    light->setType(Light::LT_POINT );
    light->setPosition(-100, 100, 500);
//    light->setDiffuseColour(1.0, 1.0, 1.0);
	light->setSpecularColour(1.0, 1.0, 1.0);
    Event->World->attachObject(light);
	Event->setLightMember(light, 0);

    light = mSceneMgr->createLight("Light_Spot");
    light->setType(Light::LT_SPOTLIGHT);
	light->setDirection(0, -1, -1);
    light->setPosition (-125, 200, 100);
    light->setDiffuseColour(1.0, 1.0, 1.0);
//	light->setSpotlightRange(Radian(.2) , Radian(.6), 5.5);
//	light->setAttenuation(1000,1,0.005,0);

	Event->World->attachObject(light);
	Event->setLightMember(light, 1);
	light->setVisible(false);

    // Setup animation default
    Animation::setDefaultInterpolationMode(Animation::IM_LINEAR);
    Animation::setDefaultRotationInterpolationMode(Animation::RIM_LINEAR);

    Player::getSingleton().Init(mSceneMgr);
	NPC_Enemy1->Init(mSceneMgr, Event->World);
	TileMap::getSingleton().Init(mSceneMgr, mSceneMgr->getRootSceneNode());
	
	// Make sure the camera track this node

/*
	// Create the camera node & attach camera
//	SceneNode* camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
//	camNode->attachObject(mCamera);
//	mCamera->setAutoTracking(true, Player::getSingleton().getNode());
	mCamera->setPosition(Vector3(0,CAMERA_ZOOM+50, CAMERA_ZOOM+50));
    mCamera->setNearClipDistance(CAMERA_ZOOM);
//    mCamera->setFarClipDistance(600);
	mCamera->lookAt(Vector3(0,0,0));
*/
/*
	// Create the camera node & attach camera
    mCamera->setPosition(Vector3(0, 0, 500));
    // Look back along -Z
    mCamera->lookAt(Vector3(0,0,0));
    mCamera->setNearClipDistance(500);
//    mCamera->setAutoTracking(true, Event->World);
//    SceneNode* camNode = Player::getSingleton().getNode()->createChildSceneNode();
//    camNode->attachObject(mCamera);
*/
	mCamera->setPosition(Vector3(0,CAMERA_ZOOM+50, CAMERA_ZOOM+50));
    mCamera->setNearClipDistance(CAMERA_ZOOM);
	mCamera->lookAt(Vector3(0,0,0));
}
