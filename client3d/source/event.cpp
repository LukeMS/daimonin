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
#include "event.h"
#include "sound.h"
#include "object_manager.h"
#include "option.h"
#include "logger.h"
#include "gui_textout.h"
#include "network.h"
#include "TileManager.h"
#include "gui_manager.h"
#include "object_manager.h"
#include "particle_manager.h"
#include "spell_manager.h"
#include "TileChunk.h"

using namespace Ogre;

///================================================================================================
/// Global variables.
///================================================================================================
CEvent *Event=0;

void CEvent::Setup()
{
  /// ////////////////////////////////////////////////////////////////////
  /// Create one viewport, entire window
  /// ////////////////////////////////////////////////////////////////////
  mCamera = mSceneManager->createCamera("PlayerCam");
  Viewport *VP = mWindow->addViewport(mCamera);
  VP->setBackgroundColour(ColourValue(0,0,0));
  /// Alter the camera aspect ratio to match the viewport
  mCamera->setAspectRatio(Real(VP->getActualWidth()) / Real(VP->getActualHeight()));

  if (!(Option ::getSingleton().Init())) return;
  if (!(Sound  ::getSingleton().Init())) return;
  if (!(Network::getSingleton().Init())) return;
  Sound::getSingleton().playSong(FILE_MUSIC_001);

  /// Create the world.
  mWorld = mSceneManager->getRootSceneNode()->createChildSceneNode();

  // mWorld->setPosition(0,0,0);
  mSceneManager->setAmbientLight(ColourValue(0, 0, 0));
  mSceneManager->setAmbientLight(ColourValue(1.0, 1.0, 1.0));

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

  mTileManager = new TileManager();
  mTileManager->Init(mSceneManager, 128,1);

  SpellManager   ::getSingleton().init(mSceneManager);
  ParticleManager::getSingleton().init(mSceneManager);
  ObjectManager  ::getSingleton().init(mSceneManager);
}

///================================================================================================
/// Constructor.
///================================================================================================
CEvent::CEvent(RenderWindow* win, SceneManager *SceneMgr)
{
  /// ////////////////////////////////////////////////////////////////////
  /// Create unbuffered key & mouse input.
  /// ////////////////////////////////////////////////////////////////////
  mEventProcessor = new EventProcessor();
  mEventProcessor->initialise(win);
  mEventProcessor->startProcessingEvents();
  mEventProcessor->addKeyListener(this);
  mEventProcessor->addMouseMotionListener(this);
  mEventProcessor->addMouseListener(this);
  mInputDevice =  mEventProcessor->getInputReader();

  mSceneManager = SceneMgr;
  mWindow = win;

  mTimeUntilNextToggle = 0;
  mTranslateVector = Vector3(0,0,0);
  mAniso = 1;
  mFiltering = TFO_BILINEAR;
  mIdleTime =0;
  mDayTime = 15;
  mCameraZoom = MAX_CAMERA_ZOOM;

  mMouseX = mMouseY =0;
  /// ////////////////////////////////////////////////////////////////////
  /// Create all Overlays.
  /// ////////////////////////////////////////////////////////////////////
  GuiManager::getSingleton().Init(FILE_GUI_IMAGESET, FILE_GUI_WINDOWS, win->getWidth(), win->getHeight());

  GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_TEXTWIN  , (void*)"Welcome to ~Daimonin 3D~.");
  GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_TEXTWIN  , (void*)"~#001b1b88Text~ ~#00663310Text~ ~#0000ffffText~");
  GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_TEXTWIN  , (void*)"~#00ff0000Text~ ~#0000ff00Text~ ~#000000ffText~");
  GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_TEXTWIN  , (void*)"");

  GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_TEXTWIN  , (void*)"line a1 ");
  GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_TEXTWIN  , (void*)"line a2 ");
  GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_TEXTWIN  , (void*)"line b1 ");

  mQuitGame = false;
}

///================================================================================================
/// Destructor.
///================================================================================================
CEvent::~CEvent()
{
  if (mEventProcessor)  delete mEventProcessor;
  GuiManager::getSingleton().freeRecources();
  if (mTileManager) delete mTileManager;
  // Network::getSingleton().freeRecources();
  // Option ::getSingleton().freeRecources();
  Sound::getSingleton().freeRecources();
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
  if (mWindow->isClosed())
  {
    return false;
  }
  ObjectManager::getSingleton().update(OBJECT_NPC, evt);
  ParticleManager::getSingleton().moveNodeObject(evt);
  mIdleTime += evt.timeSinceLastFrame;
  if (mIdleTime > 30.0)
  {
    Sound::getSingleton().playSample(SAMPLE_PLAYER_IDLE);
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

  GuiManager::getSingleton().update();
  if (Option::getSingleton().mStartNetwork)  Network::getSingleton().Update();
  if (mQuitGame)  return false;

  return true;
}

///================================================================================================
/// Frame End event.
///================================================================================================
bool CEvent::frameEnded(const FrameEvent& )
{
  const RenderTarget::FrameStats& stats = mWindow->getStatistics();
  static int skipFrames = 0;
  if (--skipFrames <= 0)
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

///================================================================================================
/// Buffered Key Events.
///================================================================================================
void CEvent::keyPressed(KeyEvent *e)
{
  mIdleTime =0;
  static Real g_pitch = 0.2;
  if (GuiManager::getSingleton().hasFocus())
  {
    GuiManager::getSingleton().keyEvent(e->getKeyChar(), e->getKey());
    e->consume();
    return;
  }
  switch (e->getKey())
  {
      /////////////////////////////////////////////////////////////////////////
      /// Player Movemment.
      /////////////////////////////////////////////////////////////////////////
    case KC_UP:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_WALK, 1);
      //mCamera->  moveRelative (Vector3(0,100,0));
      break;
    case KC_DOWN:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_WALK, -1);
      //mCamera->  moveRelative (Vector3(0,-100,0));
      break;
    case KC_RIGHT:
      //mCamera->  moveRelative (Vector3(100,0,0));
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TURN, -1);
      break;
    case KC_LEFT:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TURN,  1);
      //mCamera->  moveRelative (Vector3(-100,0,0));
      break;


    case KC_F1:
      ObjectManager::getSingleton().toggleAnimGroup(OBJECT_PLAYER);
      break;
    case KC_A:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_ANIMATION, STATE_ATTACK1);
      break;
    case KC_B:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_ANIMATION, STATE_BLOCK1);
      break;
    case KC_C:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_ANIMATION, STATE_CAST1);
      break;
    case KC_S:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_ANIMATION, STATE_SLUMP1);
      break;
    case KC_D:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_ANIMATION, STATE_DEATH1);
      break;
    case KC_H:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_ANIMATION, STATE_HIT1);
      break;
    case KC_1:
      ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_WEAPON_HAND, 1);
      break;
    case KC_2:
      ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_SHIELD_HAND, 1);
      break;
    case KC_3:
      //ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,0, -1);
      break;
    case KC_4:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,1, -1);
      break;
    case KC_5:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,2, -1);
      break;
    case KC_6:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,3, -1);
      break;
    case KC_7:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,4, -1);
      break;
    case KC_8:
      ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_HEAD, 1);
      break;
    case KC_9:
      ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_BODY, 1);
      break;

    case KC_J:
      // ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TURN,  1);
      mCamera->yaw(Degree(10));
      break;
    case KC_K:
      //ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TURN, -1);
      mCamera->yaw(Degree(-10));
      break;
    case KC_G:
      mTileManager->ToggleGrid();
      break;
    case KC_I:
      ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_ANIMATION, STATE_ATTACK1);
      break;
    case KC_P:
      ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TEXTURE, 0, 0);
      break;
    case KC_Q:
      ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TEXTURE, 0, 1);
      break;

      /// ////////////////////////////////////////////////////////////////////
      /// Engine settings.
      /// ////////////////////////////////////////////////////////////////////
      /*
        case KC_C:
         mSceneDetailIndex = (mSceneDetailIndex+1)%3 ;
         switch(mSceneDetailIndex)
         {
          case 0 : mCamera->setDetailLevel(SDL_SOLID) ;     break ;
          case 1 : mCamera->setDetailLevel(SDL_WIREFRAME) ; break ;
          case 2 : mCamera->setDetailLevel(SDL_POINTS) ;    break ;
         }
         break;
      */
    case KC_X:
      {
        static int pixels =128;
        //change pixel size of terrain textures
        pixels /= 2; // shrink pixel value
        if (pixels < 8) pixels = 128; // if value is too low resize to maximum
        mTileManager->SetTextureSize(pixels);
        mTimeUntilNextToggle = .5;
      }
      break;

    case KC_F:
      switch(mFiltering)
      {
        case TFO_BILINEAR:
          mFiltering = TFO_TRILINEAR;
          mAniso = 1;
          break;
        case TFO_TRILINEAR:
          mFiltering = TFO_ANISOTROPIC;
          mAniso = 8;
          break;
        case TFO_ANISOTROPIC:
          mFiltering = TFO_BILINEAR;
          mAniso = 1;
          break;
        default:
          break;
      }
      MaterialManager::getSingleton().setDefaultTextureFiltering(mFiltering);
      MaterialManager::getSingleton().setDefaultAnisotropy(mAniso);
      break;
    case KC_L:
      Option::getSingleton().mStartNetwork = true;
      break;
    case KC_W:
      if (mDayTime)
      {
        mDayTime =0;
        mLight[LIGHT_VOL ]->setVisible(false);
        mLight[LIGHT_SPOT]->setVisible(true);
      }
      else
      {
        mDayTime =15;
        mLight[LIGHT_VOL ]->setVisible(true);
        mLight[LIGHT_SPOT]->setVisible(false);
      }
      break;

    case KC_PGUP:
      {
        mCamera->pitch(Radian(-0.1));
        g_pitch -= 0.1;
        Vector3 pos = mCamera->getPosition();
        pos.y += 30;
        mCamera->setPosition(pos);
      }
      break;
    case KC_PGDOWN:
      {
        mCamera->pitch(Radian(+0.1));
        g_pitch += 0.1;
        Vector3 pos = mCamera->getPosition();
        pos.y -= 30;
        mCamera->setPosition(pos);
      }
      break;

    case KC_SUBTRACT:
      {
        if (mCameraZoom < MAX_CAMERA_ZOOM) mCameraZoom += 5;
        mCamera->setFOVy(Degree(mCameraZoom));
      }
      break;

    case KC_ADD:
      {
        if (mCameraZoom > MIN_CAMERA_ZOOM) mCameraZoom -= 5;
        mCamera->setFOVy(Degree(mCameraZoom));
      }
      break;

      /// ////////////////////////////////////////////////////////////////////
      /// Screenshot.
      /// ////////////////////////////////////////////////////////////////////
    case KC_SYSRQ:
      {
        static int mNumScreenShots=0;
        char tmp[20];
        sprintf(tmp, "screenshot_%d.png", ++mNumScreenShots);
        mWindow->writeContentsToFile(tmp);
        mTimeUntilNextToggle = 0.5;
      }
      break;

      /// ////////////////////////////////////////////////////////////////////
      /// Exit game.
      /// ////////////////////////////////////////////////////////////////////
    case KC_ESCAPE:
      mQuitGame = true;
      break;
    default:
      break;
  }
  // e->consume();
}

void CEvent::keyClicked(KeyEvent* )
{
}

void CEvent::keyReleased(KeyEvent* e)
{
  switch (e->getKey())
  {
      /// ////////////////////////////////////////////////////////////////////
      /// Player Movemment.
      /// ////////////////////////////////////////////////////////////////////
    case KC_UP:
    case KC_DOWN:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_WALK, 0);
      break;
    case KC_RIGHT:
    case KC_LEFT:
      ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TURN, 0);
      break;

    case KC_J:
    case KC_K:
      ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TURN,  0);
      break;
    default:
      break;

    case KC_G:
      ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_WALK,  0);
      break;
  }
}

///================================================================================================
/// Buffered Mouse Events.
///================================================================================================
void CEvent::mouseMoved (MouseEvent *e)
{
  mMouseX = e->getX();
  mMouseY = e->getY();
  if (mMouseX > 0.995) mMouseX = 0.995;
  if (mMouseY > 0.990) mMouseY = 0.990;
  GuiManager::getSingleton().mouseEvent(M_MOVED, mMouseX, mMouseY);

}

void CEvent::mousePressed (MouseEvent *e)
{
  mMouseX = e->getX();
  mMouseY = e->getY();

  int button = e->getButtonID();
  if (button & InputEvent::BUTTON0_MASK ) // LeftButton.
  {
    if (GuiManager::getSingleton().mouseEvent(M_PRESSED, mMouseX, mMouseY))
    { // Button was pressed in a gui_window.
    }
    else
    {
      //pgTileManager->get_TileInterface()->pick_Tile(mMouseX, mMouseY);
    }
  }
  else if (button & InputEvent::BUTTON1_MASK ) // RightButton.
  {
    // activate mouse picking of tiles
    #ifdef WIN32
    mTileManager->get_TileInterface()->pick_Tile(mMouseX, mMouseY);
    #endif
  }
  else if (button & InputEvent::BUTTON2_MASK ) // MidButton.
  {
    #ifndef WIN32
    mTileManager->get_TileInterface()->pick_Tile(mMouseX, mMouseY);
    #endif
  }
  e->consume();
}

void CEvent::mouseDragged(MouseEvent *e)
{
  mouseMoved(e);
  e->consume();
}

void CEvent::mouseClicked (MouseEvent *e)
{
  mouseMoved(e);
  e->consume();
}

void CEvent::mouseEntered (MouseEvent *e)
{
  mouseMoved(e);
  e->consume();
}

void CEvent::mouseExited  (MouseEvent *e)
{
  mouseMoved(e);
  e->consume();
}

void CEvent::mouseReleased(MouseEvent *e)
{
  GuiManager::getSingleton().mouseEvent(M_RELEASED, mMouseX, mMouseY);
  //  mouseMoved(e);
  e->consume();
}

