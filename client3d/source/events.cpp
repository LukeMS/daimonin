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

#include <OgreKeyEvent.h>
#include "events.h"
#include "sound.h"
#include "object_manager.h"
#include "option.h"
#include "logger.h"
#include "network.h"
#include "TileManager.h"
#include "gui_textout.h"
#include "gui_manager.h"
#include "gui_imageset.h"
#include "object_manager.h"
#include "particle_manager.h"
#include "spell_manager.h"

using namespace Ogre;

///================================================================================================
/// Global variables.
///================================================================================================
CEvent *Event=0;

///================================================================================================
/// Constructor.
///================================================================================================
CEvent::CEvent(RenderWindow* win, SceneManager *SceneMgr)
{
  /// ////////////////////////////////////////////////////////////////////
  /// Create unbuffered key & mouse input.
  /// ////////////////////////////////////////////////////////////////////
  mSceneManager = SceneMgr;
  mWindow = win;
  mEventProcessor = new EventProcessor();
  mEventProcessor->initialise(win);
  mEventProcessor->startProcessingEvents();
  mInputDevice =  mEventProcessor->getInputReader();
  mTileManager = NULL;
  mTimeUntilNextToggle = 0;
  mTranslateVector = Vector3(0,0,0);
  mIdleTime =0;
  mDayTime = 15;
  mCameraZoom = MAX_CAMERA_ZOOM;
  mMouseX = mMouseY =0;
  mQuitGame = false;
  Option::getSingleton().setGameStatus(GAME_STATUS_INIT_VIEWPORT);
}

///================================================================================================
/// Destructor.
///================================================================================================
CEvent::~CEvent()
{
  if (mEventProcessor)  delete mEventProcessor;
  if (mTileManager)     delete mTileManager;
  ObjectManager::getSingleton().freeRecources();
  GuiManager   ::getSingleton().freeRecources();
  Sound        ::getSingleton().freeRecources();
}

///================================================================================================
/// Player has moved, update the world position.
///================================================================================================
void CEvent::setWorldPos(Vector3 &pos)
{
  static Vector3 dPos = pos;
  bool ui = false;
  /// East
  if (pos.x + dPos.x > TILE_SIZE)
  {
    pos.x -= dPos.x;
    short tmp[ TILES_SUM_Z+1];
    for (short y = 0; y < TILES_SUM_Z+1; ++y) tmp[y] = mTileManager->Get_Map_Height(0, y);
    for (int x = 1; x < TILES_SUM_X+1; ++x)
    {
      for (int y = 0; y < TILES_SUM_Z+1; ++y)
      {
        unsigned short value = mTileManager->Get_Map_Height(x, y);
        mTileManager->Set_Map_Height(x-1, y, value);
      }
    }
    for (short y = 0; y < TILES_SUM_Z+1; ++y) mTileManager->Set_Map_Height(TILES_SUM_X, y, tmp[y] );
    ui = true;
  }

  /// West
  else if (pos.x + dPos.x < -TILE_SIZE)
  {
    pos.x -= dPos.x;
    short tmp[ TILES_SUM_Z+1];
    for (short z = 0; z < TILES_SUM_Z+1; ++z) tmp[z] = mTileManager->Get_Map_Height(TILES_SUM_X, z);
    for (int x = TILES_SUM_X-1; x >= 0; --x)
    {
      for (int y = 0; y < TILES_SUM_Z+1; ++y)
      {
        unsigned short value = mTileManager->Get_Map_Height(x, y);
        mTileManager->Set_Map_Height(x+1, y, value);
      }
    }
    for (short y = 0; y < TILES_SUM_Z+1; ++y) mTileManager->Set_Map_Height(0, y, tmp[y] );
    ui = true;
  }

  /// South
  else if (pos.z + dPos.z > TILE_SIZE)
  {
    pos.z -= dPos.z;
    short tmp[ TILES_SUM_X+1];
    for (short x = 0; x < TILES_SUM_X+1; ++x) tmp[x] = mTileManager->Get_Map_Height(x, 0);
    for (int z = 1; z < TILES_SUM_Z+1; ++z)
    {
      for (int x = 0; x < TILES_SUM_X+1; ++x)
      {
        unsigned short value = mTileManager->Get_Map_Height(x, z);
        mTileManager->Set_Map_Height(x, z-1, value);
      }
    }
    for (short x = 0; x < TILES_SUM_X+1; ++x) mTileManager->Set_Map_Height(x, TILES_SUM_Z, tmp[x] );
    ui = true;
  }

  /// North
  else if (pos.z + dPos.z < -TILE_SIZE)
  {
    pos.z -= dPos.z;
    short tmp[ TILES_SUM_X+1];
    for (short x = 0; x < TILES_SUM_X+1; ++x) tmp[x] = mTileManager->Get_Map_Height(x, TILES_SUM_Z);
    for (int z = TILES_SUM_Z-1; z >= 0; --z)
    {
      for (int x = 0; x < TILES_SUM_X+1; ++x)
      {
        unsigned short value = mTileManager->Get_Map_Height(x, z);
        mTileManager->Set_Map_Height(x, z+1, value);
      }
    }
    for (short x = 0; x < TILES_SUM_X+1; ++x) mTileManager->Set_Map_Height(x, 0, tmp[x] );
    ui = true;
  }
  dPos+=pos;
  mTileManager->ControlChunks(pos);
  if (ui == true)
  {
    mTileManager->ChangeChunks();
  }
  mCamera->move(pos);

  // mCamera->setPosition(mCamera->getPosition()+pos);
  // ParticleManager::getSingleton().synchToWorldPos(pos);
}

///================================================================================================
/// Frame Start event.
///================================================================================================
bool CEvent::frameStarted(const FrameEvent& evt)
{
  static Overlay *mOverlay;
  if (mWindow->isClosed() || mQuitGame)
    return false;

  switch (Option::getSingleton().getGameStatus())
  {
      case GAME_STATUS_INIT_VIEWPORT:
      {
        /// ////////////////////////////////////////////////////////////////////
        /// Create one viewport, entire window
        /// ////////////////////////////////////////////////////////////////////
        mCamera = mSceneManager->createCamera("PlayerCam");
        Viewport *VP = mWindow->addViewport(mCamera);
        VP->setBackgroundColour(ColourValue(0,0,0));
        /// Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(Real(VP->getActualWidth()) / Real(VP->getActualHeight()));
        mCamera->setProjectionType(PT_ORTHOGRAPHIC);
        mCamera->setFOVy(Degree(MAX_CAMERA_ZOOM));
        mCamera->setPosition(Vector3(CHUNK_SIZE_X *TILE_SIZE/2 , 450, CHUNK_SIZE_Z * TILE_SIZE/2 + 942));
        mCamera->pitch(Degree(-25));
        const Vector3 *corner = mCamera->getWorldSpaceCorners();
        mCamCornerX =  (corner[0].x - corner[1].x)/2;
        mCamCornerY = -(corner[0].y - corner[2].y)/2 -13.57;  // Todo: clean up.
        /// ////////////////////////////////////////////////////////////////////
        /// Create the world.
        /// ////////////////////////////////////////////////////////////////////
        mWorld = mSceneManager->getRootSceneNode()->createChildSceneNode();
        /// ////////////////////////////////////////////////////////////////////
        /// Create a minimal gui for some loading infos..
        /// ////////////////////////////////////////////////////////////////////
        GuiManager::getSingleton().Init(mWindow->getWidth(), mWindow->getHeight());
        GuiTextout::getSingleton().loadRawFont(FILE_SYSTEM_FONT);
        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_INIT_SOUND);
        /// Show the loading-gfx.
        mOverlay = OverlayManager::getSingleton().getByName ("Overlay/Loading");
        mOverlay->show();
        if (Root::getSingleton().getTimer()->getMilliseconds() & 1)
          mOverlay->getChild("OverlayElement/Screen1")->hide();
        else
          mOverlay->getChild("OverlayElement/Screen2")->hide();

      /*
                mCamera->setFOVy(Degree(55));
                mCamera->setPosition(Vector3(0 , 60, 80));
                mCamera->lookAt(0, 0, 0);
                SceneNode *n = mSceneManager->getRootSceneNode()->createChildSceneNode("node_loading");
                BillboardSet* set = mSceneManager->createBillboardSet("loading", 1);
                set->setMaterialName("LoadScreen");
                set->setVisible(true);
                n->attachObject(set);
                Billboard* a = set->createBillboard(0, 0, 0);
        */

        GuiManager::getSingleton().displaySystemMessage("* Welcome to Daimonin *");
        GuiManager::getSingleton().displaySystemMessage("Starting the sound-system...");
      }
      break;

      case GAME_STATUS_INIT_SOUND:
      {
        Sound::getSingleton().Init();
        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_INIT_LIGHT);
        GuiManager::getSingleton().displaySystemMessage("Starting lights...");
      }
      break;

      case GAME_STATUS_INIT_LIGHT:
      {
        /*
        /// Todo: lightmanager
        Light *light;
        light = mSceneManager->createLight("Light_Vol");
        light->setType(Light::LT_POINT );
        light->setPosition(-100, 200, 800);
        //    light->setDiffuseColour(1.0, 1.0, 1.0);
        light->setSpecularColour(1.0, 1.0, 1.0);
        mWorld->attachObject(light);
        setLightMember(light, 0);

        light = mSceneManager->createLight("Light_Spot");
        light->setType(Light::LT_SPOTLIGHT);
        light->setDirection(0, -1, -1);
        light->setPosition (-125, 200, 100);
        light->setDiffuseColour(1.0, 1.0, 1.0);
        // light->setSpotlightRange(Radian(.2) , Radian(.6), 5.5);
        // light->setAttenuation(1000,1,0.005,0);

        mWorld->attachObject(light);
        setLightMember(light, 1);
        light->setVisible(false);
        mSceneManager->setAmbientLight(ColourValue(1.0, 1.0, 1.0));

        // mSceneMgr->setFog(FOG_LINEAR , ColourValue(.7,.7,.7), 0.005, 450, 800);
        // mSceneMgr->setFog(FOG_LINEAR , ColourValue(1,1,1), 0.005, 450, 800);

        */
        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_INIT_SPELL);
        GuiManager::getSingleton().displaySystemMessage("Starting the spells...");
      }
      break;

      case GAME_STATUS_INIT_SPELL:
      {
        SpellManager::getSingleton().init(mSceneManager);
        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_INIT_PARTICLE);
        GuiManager::getSingleton().displaySystemMessage("Starting the particles...");
      }
      break;

      case GAME_STATUS_INIT_PARTICLE:
      {
        ParticleManager::getSingleton().init(mSceneManager);
        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_INIT_GUI_IMAGESET);
        GuiManager::getSingleton().displaySystemMessage("Starting the gui...");
        GuiManager::getSingleton().displaySystemMessage(" - Parsing Imageset");
      }
      break;

      case GAME_STATUS_INIT_GUI_IMAGESET:
      {
        GuiImageset::getSingleton().parseXML(FILE_GUI_IMAGESET);
        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_INIT_GUI_WINDOWS);
        GuiManager::getSingleton().displaySystemMessage(" - Parsing windows");
      }
      break;

      case GAME_STATUS_INIT_GUI_WINDOWS:
      {
        Logger::log().headline("Starting GUI");
        GuiManager::getSingleton().parseWindows(FILE_GUI_WINDOWS);
        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_INIT_TILE);
        GuiManager::getSingleton().displaySystemMessage("Starting the tile-engine...");
      }
      break;

      case GAME_STATUS_INIT_TILE:
      {
        /// As events are handled in the gui.
        /// The Listeners must be added after gui was init.
        mEventProcessor->addKeyListener(this);
        mEventProcessor->addMouseMotionListener(this);
        mEventProcessor->addMouseListener(this);
        mTileManager = new TileManager();
        if (Option::getSingleton().getIntValue(Option::HIGH_TEXTURE_DETAILS))
          mTileManager->Init(mSceneManager, 128,1);
        else
          mTileManager->Init(mSceneManager, 16,1);
        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_INIT_OBJECT);
        GuiManager::getSingleton().displaySystemMessage("Starting the objects...");
        Vector3 pos = Vector3::ZERO;
        setWorldPos(pos);
      }
      break;

      case GAME_STATUS_INIT_OBJECT:
      {
        ObjectManager::getSingleton().init();
        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_INIT_NET);
        GuiManager::getSingleton().displaySystemMessage("Starting the network...");
      }
      break;

      case GAME_STATUS_INIT_NET:
      {
        Network::getSingleton().Init();
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Welcome to ~Daimonin 3D~.");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Press ~right~ MB on ground to move.");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"~#00880000Remember:~");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"~#001b1b88Server~ will send us~#00880088 one Step~ at");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"once, so the code won't work for a");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"bigger distance.");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Press ~4~ to change t-shirt.");

        /// Set next state.
        Option::getSingleton().setGameStatus(GAME_STATUS_META);
        GuiManager::getSingleton().displaySystemMessage("");
        OverlayManager::getSingleton().destroy(mOverlay);
      }
      break;

      default:
      {
        ObjectManager::getSingleton().update(OBJECT_NPC, evt);
        ParticleManager::getSingleton().moveNodeObject(evt);
        mIdleTime += evt.timeSinceLastFrame;
        if (mIdleTime > 30.0)
        {
          Sound::getSingleton().playStream(Sound::PLAYER_IDLE);
          mIdleTime = 0;
        }
        /*
         if (!mUseBufferedInputKeys)
         {
          // one of the input modes is immediate, so setup what is needed for immediate mouse/key movement
          if (mTimeUntilNextToggle >= 0)
           mTimeUntilNextToggle -= evt.timeSinceLastFrame;
          // If this is the first frame, pick a speed
          if (evt.timeSinceLastFrame == 0)
          {
           mMoveScale = 1;
           mRotScale = 0.1;
          }
          // Otherwise scale movement units by time passed since last frame
          else
          {
           // Move about 100 units per second,
           mMoveScale = mMoveSpeed * evt.timeSinceLastFrame;
           // Take about 10 seconds for full rotation
           mRotScale = mRotateSpeed * evt.timeSinceLastFrame;
          }
             mTranslateVector = Vector3(0,0,0);
                if (processUnbufferedKeyInput(evt) == false) { return false; }
         }
        */
        GuiManager::getSingleton().update(evt.timeSinceLastFrame);
        ParticleManager::getSingleton().update(evt.timeSinceLastFrame);
        if (Option::getSingleton().getIntValue(Option::UPDATE_NETWORK))
          Network::getSingleton().Update();
      }
      break;
  }
  return true;
}

///================================================================================================
/// Frame End event.
///================================================================================================
bool CEvent::frameEnded(const FrameEvent& )
{
  if (Option::getSingleton().getGameStatus() <= GAME_STATUS_INIT_NET)
    return true;

  const RenderTarget::FrameStats& stats = mWindow->getStatistics();
  static int skipFrames = 0;
  if (--skipFrames <= 0 )
  {
    static char buffer[16];
    skipFrames = 10;
    sprintf(buffer, "%.1f", stats.lastFPS);
    GuiManager::getSingleton().sendMessage(GUI_WIN_STATISTICS, GUI_MSG_TXT_CHANGED, GUI_TEXTVALUE_STAT_CUR_FPS  , (void*)buffer);
    sprintf(buffer, "%.1f", stats.bestFPS);
    GuiManager::getSingleton().sendMessage(GUI_WIN_STATISTICS, GUI_MSG_TXT_CHANGED, GUI_TEXTVALUE_STAT_BEST_FPS , (void*)buffer);
    sprintf(buffer, "%.1f", stats.worstFPS);
    GuiManager::getSingleton().sendMessage(GUI_WIN_STATISTICS, GUI_MSG_TXT_CHANGED, GUI_TEXTVALUE_STAT_WORST_FPS, (void*)buffer);
    sprintf(buffer, "%d", stats.triangleCount);
    GuiManager::getSingleton().sendMessage(GUI_WIN_STATISTICS, GUI_MSG_TXT_CHANGED, GUI_TEXTVALUE_STAT_SUM_TRIS , (void*)buffer);
  }
  return true;
}

