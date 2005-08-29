/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html
 
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.
 
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "define.h"
#include "event.h"
#include "client.h"
#include "network.h"
#include "logger.h"
#include "textwindow.h"
#include "dialog.h"
#include "option.h"
#include "sound.h"
#include "object_manager.h"
#include "particle_manager.h"
#include "spell_manager.h"
#include "TileChunk.h"

using namespace Ogre;

const int SUM_MIPMAPS = 0;
Camera *mCamera = 0;

/// ========================================================================
/// Start the example
/// ========================================================================
void DaimoninClient::go(void)
{
  mRoot = 0;
  if (!setup()) return;
  if (!(Option ::getSingleton().Init())) return;
  if (!(Sound  ::getSingleton().Init())) return;
  if (!(Network::getSingleton().Init())) return;
  Sound::getSingleton().playSong(FILE_MUSIC_001);

  // Create the world.
  Event->World = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 0), Quaternion(1.0,0.0,0.0,0.0));
  Event->SetSceneManager(mSceneMgr);
  // Event->World->setPosition(0,0,0);
  mSceneMgr->setAmbientLight(ColourValue(0, 0, 0));
  mSceneMgr->setAmbientLight(ColourValue(1.0, 1.0, 1.0));

  Light *light;
  light = mSceneMgr->createLight("Light_Vol");
  light->setType(Light::LT_POINT );
  light->setPosition(-100, 200, 800);
  ///    light->setDiffuseColour(1.0, 1.0, 1.0);
  light->setSpecularColour(1.0, 1.0, 1.0);
  Event->World->attachObject(light);
  Event->setLightMember(light, 0);

  light = mSceneMgr->createLight("Light_Spot");
  light->setType(Light::LT_SPOTLIGHT);
  light->setDirection(0, -1, -1);
  light->setPosition (-125, 200, 100);
  light->setDiffuseColour(1.0, 1.0, 1.0);
  /// light->setSpotlightRange(Radian(.2) , Radian(.6), 5.5);
  /// light->setAttenuation(1000,1,0.005,0);

  Event->World->attachObject(light);
  Event->setLightMember(light, 1);
  light->setVisible(false);

  mTileManager = new TileManager();
  mTileManager->Init(mSceneMgr);
  Event->Set_pgraphics(mTileManager);

  SpellManager::getSingleton().init(mSceneMgr, Event->World);
  ParticleManager::getSingleton().init(mSceneMgr, Event->World);
  ObjectManager::getSingleton().init(mSceneMgr, Event->World);

  mRoot->startRendering();
  /////////////////////////////////////////////////////////////////////////
  /// Clean up.
  /////////////////////////////////////////////////////////////////////////
  if (mTileManager) delete mTileManager;
  // Network::getSingleton().freeRecources();
  // Option ::getSingleton().freeRecources();
  Sound::getSingleton().freeRecources();
  if (Event)        delete Event;
  if (mRoot)        delete mRoot;
}

/// ========================================================================
/// These internal methods package up the stages in the startup process
/// Sets up the application - returns false if user abandon configuration.
/// ========================================================================
bool DaimoninClient::setup(void)
{
  mRoot = new Root();
  setupResources();
  /////////////////////////////////////////////////////////////////////////
  /// Show the configuration dialog and initialise the system
  /// You can skip this and use root.restoreConfig() to load configuration
  /// settings if you were sure there are valid ones saved in ogre.cfg
  /////////////////////////////////////////////////////////////////////////
  if(!mRoot->showConfigDialog()) return false;
  mWindow = mRoot->initialise(true);
  /////////////////////////////////////////////////////////////////////////
  /// Get the SceneManager, in this case a generic one
  /////////////////////////////////////////////////////////////////////////
  mSceneMgr = mRoot->getSceneManager(ST_GENERIC);
  /////////////////////////////////////////////////////////////////////////
  /// Create a camera
  /////////////////////////////////////////////////////////////////////////
  mCamera = mSceneMgr->createCamera("Camera");
  /////////////////////////////////////////////////////////////////////////
  /// Create one viewport, entire window
  /////////////////////////////////////////////////////////////////////////
  mVP = mWindow->addViewport(mCamera);
  mVP->setBackgroundColour(ColourValue(0,0,0));
  /// Alter the camera aspect ratio to match the viewport
  mCamera->setAspectRatio(Real(mVP->getActualWidth()) / Real(mVP->getActualHeight()));
  /////////////////////////////////////////////////////////////////////////
  /// Set default mipmap level (NB some APIs ignore this)
  /////////////////////////////////////////////////////////////////////////
  // TextureManager::getSingleton().setDefaultNumMipmaps(SUM_MIPMAPS);
  /////////////////////////////////////////////////////////////////////////
  /// Optional override method where you can perform resource group loading
  /// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
  /// Initialise, parse scripts etc
  /////////////////////////////////////////////////////////////////////////
  ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
  Event= new CEvent(mWindow, mCamera, mMouseMotionListener, mMouseListener);
  mRoot->addFrameListener(Event);
  return true;
}

/// ========================================================================
/// Define the source of resources (other than current folder)
/// ========================================================================
void DaimoninClient::setupResources(void)
{
  /// Load resource paths from config file
  ConfigFile cf;
  cf.load("resources.cfg");
  /// Go through all sections & settings in the file
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
