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
#include <OgreImage.h>
#include <OgreConfigFile.h>
#include <OgreSceneManager.h>

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
	mCamera->setPosition(Vector3(0,400, 400));
	mCamera->lookAt(Vector3(0,0,0));
    mCamera->setNearClipDistance(400);
    mCamera->setFarClipDistance(800);

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

	mEvent->World = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 0), Quaternion(1.0,0.0,0.0,0.0));
    Player::getSingleton().Init(mSceneMgr);
	NPC_Enemy1->Init(mSceneMgr, mEvent->World);
/*
	Entity     *mEntity = mSceneMgr->createEntity("enemy", "scorpion.mesh");
	SceneNode  *mNode;
	mNode   = mEvent->World->createChildSceneNode(Vector3(0, 10, 0), Quaternion(1.0,0.0,0.0,0.0));
//	mFacing = Degree(0);

//	Real faceing = atof(strTemp.c_str());
//	mFacingOffset = faceing * RAD;
    mNode->attachObject(mEntity);

//	Option::getSingleton().getDescStr("MeshSize", strTemp);
//	Real size = atof(strTemp.c_str());
	Real size = 0.25f;
	mNode->setScale(size, size, size);
//    mNode->yaw(mFacing);
*/



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
	const int startX = -190;
	const int startZ = - 60;
	
//	uchar *pImage = new uchar[512*512*4];
//	mImage.loadDynamicImage(pImage, 100,100,PF_B8G8R8);

/*
	MaterialPtr mMaterial = MaterialManager::getSingleton().getByName("dynamic");
	string texName = "testMat";
	Image mImage;
	mImage.load("grass.101.png", "General");
	TexturePtr mTexture = TextureManager::getSingleton().loadImage(texName, "Tiles", mImage, TEX_TYPE_2D, 3,1.0f);
	mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texName);
	mMaterial->load();
*/

// Lets play a little with this mateial stuff....

	int gfxNr =251;
	TileGfx::getSingleton().load_picture_from_pack(gfxNr);
	MaterialPtr mMaterial = MaterialManager::getSingleton().getByName("dynamic");
	string texName = "testMat"+ StringConverter::toString(gfxNr);
	TexturePtr mTexture = TextureManager::getSingleton().loadImage(texName, "Tiles", TileGfx::getSingleton().getSprite(gfxNr), TEX_TYPE_2D, 3,1.0f);
	mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texName);
	mMaterial->load();


	// create floor-tile.
	string name = "row 1 tile ";
	for (int x1 =0; x1 < 5; ++x1)
	{
		ent = mSceneMgr->createEntity(name+StringConverter::toString(x1), SceneManager::PT_PLANE);
//		ent->setMaterialName("grass1");
		ent->setMaterialName(mMaterial->getName());
		floor_node = mEvent->World->createChildSceneNode(Vector3(startX+x1*50, -20, startZ+25), Quaternion(1.0,0.0,0.0,0.0));
		floor_node->attachObject(ent);
		floor_node->setScale(0.25, 0.25, 0.25);
	}

	name = "row 2 tile ";
	for (int x2 =0; x2 < 5; ++x2)
	{
		ent = mSceneMgr->createEntity(name+StringConverter::toString(x2), SceneManager::PT_PLANE);
//		ent->setMaterialName("grass2");
		ent->setMaterialName(mMaterial->getName());
		floor_node = mEvent->World->createChildSceneNode(Vector3(startX+25+x2*50, -20, startZ+50), Quaternion(1.0,0.0,0.0,0.0));
		floor_node->attachObject(ent);
		floor_node->setScale(0.25, 0.25, 0.25);
	}

	name = "row 3 tile ";
	for (int x3 =0; x3 < 5; ++x3)
	{
		ent = mSceneMgr->createEntity(name+StringConverter::toString(x3), SceneManager::PT_PLANE);
//		ent->setMaterialName("grass3");
		ent->setMaterialName(mMaterial->getName());
		floor_node = mEvent->World->createChildSceneNode(Vector3(startX+x3*50,-20, startZ+75), Quaternion(1.0,0.0,0.0,0.0));
		floor_node->attachObject(ent);
		floor_node->setScale(0.25, 0.25, 0.25);
	}

}

// ========================================================================
// 
// ========================================================================
void DaimoninClient::destroyScene(void)
{
}
