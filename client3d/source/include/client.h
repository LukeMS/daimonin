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

#ifndef CLIENT_H
#define CLIENT_H


#include <Ogre.h>
#include <OgreConfigFile.h>
#include <OgreSceneManager.h>

#include "frame_Listener.h"

using namespace Ogre;

class DaimoninClient
{
  public:
    SceneNode *MouseCursor;
    SceneNode *World;

    DaimoninClient()
    {
        mFrameListener = 0;
        mRoot = 0;
    }

    ~DaimoninClient()
    {
        if (mFrameListener) { delete mFrameListener; }
        if (mRoot)          { delete mRoot; }
    }

    /// Start the example
    void go(void);

  private:
	Viewport       *mVP;
    Root           *mRoot;
    Camera         *mCamera;
    MouseMotionListener *mMouseMotionListener;
	MouseListener *mMouseListener;
    SceneManager   *mSceneMgr;
    Frame_Listener *mFrameListener;
	InputReader    *mInputReader;
    RenderWindow   *mWindow;

    // These internal methods package up the stages in the startup process
    // Sets up the application - returns false if the user chooses to abandon configuration.
    bool setup(void);

    // Configures the application - returns false if the user chooses to abandon configuration.
    bool configure(void);

    void chooseSceneManager(void)
    {
        // Get the SceneManager, in this case a generic one
        mSceneMgr = mRoot->getSceneManager(ST_GENERIC);
    }

    void createViewports(void);

    /// Method which will define the source of resources (other than current folder)
    void setupResources(void);

    /// Optional override method where you can perform resource group loading
    /// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    void loadResources(void)
	{
        // Initialise, parse scripts etc
        ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	}
   
    void createScene(void);
    void destroyScene(void);

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new Frame_Listener(mWindow, mCamera, mMouseMotionListener, mMouseListener);
        mRoot->addFrameListener(mFrameListener);
        mFrameListener->setResolution(mVP->getActualWidth(), mVP->getActualHeight());
    }

    void createCamera(void)
    {
        mCamera = mSceneMgr->createCamera("Camera");
        mCamera->setProjectionType(PT_ORTHOGRAPHIC);
        mCamera->setNearClipDistance(400);
        mCamera->setPosition(0, 400, 400);
        mCamera->lookAt(0, 0, 0);
	}
}; 

#endif
