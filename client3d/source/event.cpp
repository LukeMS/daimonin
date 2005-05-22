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

#include <OgreKeyEvent.h>
#include "event.h"
#include "dialog.h"
#include "sound.h"
#include "object_manager.h"
#include "option.h"
#include "logfile.h"
#include "textwindow.h"
#include "textinput.h"
#include "network.h"
#include "tile_map.h"

//#define SHOW_FREE_MEM

#ifdef SHOW_FREE_MEM
  #ifdef WIN32
    #include "windows.h"
    #include "winbase.h"
  #else
  
  #endif
#endif

using namespace Ogre;

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
//OverlayElement* mRowText;
//Overlay *mRowOverlay;
CEvent *Event=0;

//=================================================================================================
// Constructor.
//=================================================================================================
CEvent::CEvent(RenderWindow* win, Camera* cam, MouseMotionListener *mMotionListener, 
		MouseListener *mMListener, bool useBufferedInputKeys, bool)
{
    /////////////////////////////////////////////////////////////////////////////////////////
	// Create all Overlays.
	/////////////////////////////////////////////////////////////////////////////////////////
    mDebugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
    mDebugOverlay->show();
//	mRowOverlay= OverlayManager::getSingleton().getByName("RowOverlay");
//	mRowText = OverlayManager::getSingleton().getOverlayElement("RowOverlayText");
    mMouseCursor  = OverlayManager::getSingleton().getByName("CursorOverlay");    
	mMouseCursor->show();
    mMouseX = mMouseY =0;
	Dialog ::getSingleton().Init();
    TextWin = new CTextwindow("Message Window", -280, 300);
    ChatWin = new CTextwindow("Chat Window"   , -280, 300);
    TextWin->setChild(ChatWin);
	ChatWin->Print("Welcome to Daimonin 3D.  ", TXT_YELLOW);
	ChatWin->Print("-----------------------------------------", TXT_YELLOW);
	ChatWin->Print("  L            -> Lauch network");
//	ChatWin->Print("  C            -> Camera detail");
//	ChatWin->Print("  Page Up/Down  -> Camera view.");
	ChatWin->Print("  Print         -> Screenshot.");
	ChatWin->Print("Player commands (depends on model):", TXT_WHITE);
	ChatWin->Print("  Cursor-> Movement");
	ChatWin->Print("  1    -> Toggle Weapon");
	ChatWin->Print("  2    -> Toggle Shield");
	ChatWin->Print("  3-7  -> Toggle Material_XX_Texture");
	ChatWin->Print("  8    -> Toggle Helmet");
	ChatWin->Print("  9    -> Toggle Armor");
	ChatWin->Print("  A    -> Attack");
	ChatWin->Print("  C    -> Fireball");	
	ChatWin->Print("  B    -> Block");
	ChatWin->Print("  S    -> Slump");
	ChatWin->Print("  D    -> Death.");
	ChatWin->Print("  H    -> Hit.");
	ChatWin->Print("  F1   -> Toggle Anim Group.");
    ChatWin->Print(" ");
    ChatWin->Print("You have to wait for animation end,", TXT_WHITE);
    ChatWin->Print("before you can select the next one.", TXT_WHITE);
	TextWin->Print("Enemy commands:", TXT_WHITE);
	TextWin->Print("  J    -> Turn left");
	TextWin->Print("  K    -> Turn right");
	TextWin->Print("  I    -> Attack");	
	TextWin->Print("  P    -> Wound");
    TextWin->Print("  G    -> Move in circle");	
	TextWin->Print("  Q    -> Heal");
	TextWin->Print("Extras:", TXT_WHITE);
	TextWin->Print("  W    -> TimeWarp");
	TextWin->Print("Press +/- on numpad for zoom.");
	TextWin->Print(" ");
	TextWin->Print(" ");

	/////////////////////////////////////////////////////////////////////////////////////////
	// Create unbuffered key & mouse input.
	/////////////////////////////////////////////////////////////////////////////////////////
    mEventProcessor = new EventProcessor();
	mEventProcessor->initialise(win);
	mEventProcessor->startProcessingEvents();
	mEventProcessor->addKeyListener(this);
	mEventProcessor->addMouseMotionListener(this);
	mEventProcessor->addMouseListener(this);			
	mInputDevice =  mEventProcessor->getInputReader();
    mMouseMotionListener = mMotionListener;
    mMouseListener = mMListener;

	mQuitGame = false;
	mCamera = cam;
	mWindow = win;
	mTimeUntilNextToggle = 0;
	mSceneDetailIndex = 0;
    mTranslateVector = Vector3(0,0,0);
    mAniso = 1;
    mFiltering = TFO_BILINEAR;
	mIdleTime =0;
	mDayTime = 15;
	mCameraZoom = CAMERA_ZOOM;
}

//=================================================================================================
// Destructor.
//=================================================================================================
CEvent::~CEvent()
{
	if (mEventProcessor) { delete mEventProcessor; }
    if (TextWin)         { delete TextWin; }
    if (ChatWin)         { delete ChatWin; }
}

//=================================================================================================
// Player has moved, update the world position.
//=================================================================================================
void CEvent::setWorldPos(Vector3 &pos)
{ 
    World->translate(-pos);
    ParticleManager::getSingleton().synchToWorldPos(pos);
}

//=================================================================================================
// Frame Start event.
//=================================================================================================
bool CEvent::frameStarted(const FrameEvent& evt)
{
    if(mWindow->isClosed()) { return false; }
    ObjectManager::getSingleton().update(OBJECT_NPC, evt);
    ParticleManager::getSingleton().moveNodeObject(evt);    
	mIdleTime += evt.timeSinceLastFrame;
	if (mIdleTime > 30.0)
	{ 
		Sound::getSingleton().playSample(SAMPLE_PLAYER_IDLE);
		mIdleTime = 0;
	}

	TileMap::getSingleton().draw();

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
	if (Option::getSingleton().mStartNetwork) { Network::getSingleton().Update(); }
    if (mQuitGame) { return false; }
	return true;
}

//=================================================================================================
// Frame End event.
//=================================================================================================
bool CEvent::frameEnded(const FrameEvent& evt)
{
    static String currFps  = "Current FPS: ";
    static String avgFps   = "Average FPS: ";
    static String bestFps  = "Best FPS: ";
    static String worstFps = "Worst FPS: ";
    static String tris     = "Triangle Count: ";
    OverlayElement* guiAvg   = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
    OverlayElement* guiCurr  = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
    OverlayElement* guiBest  = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
    OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

    const RenderTarget::FrameStats& stats = mWindow->getStatistics();

    guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
    guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
    #ifdef SHOW_FREE_MEM
	  #ifdef WIN32
		MEMORYSTATUS ms;
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatus(&ms);
		if((long)ms.dwAvailPageFile<0) { ms.dwAvailPageFile = 0; }
		if((long)ms.dwAvailPhys    <0) { ms.dwAvailPhys     = 0; }
		long usedPhys = ((long)ms.dwTotalPhys     - (long)ms.dwAvailPhys) / 1024;
		long usedPage = ((long)ms.dwTotalPageFile - (long)ms.dwAvailPageFile)/ 1024;
	  #else
	  #endif // WIN32
		guiBest ->setCaption("Phys Mem used:" + StringConverter::toString(usedPhys)+ " kb");
		guiWorst->setCaption("Page Mem used:" + StringConverter::toString(usedPage)+ " kb");
	#else
		guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)+" "+StringConverter::toString(stats.bestFrameTime)+" ms");
		guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS) +" "+StringConverter::toString(stats.worstFrameTime)+" ms");
	#endif
    OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
    guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

    OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
    guiDbg->setCaption(mWindow->getDebugText());
    TextWin->Update();
	ChatWin->Update();

	///////////////////////////////////////////////////////////////////////// 
	// Print camera details
	/////////////////////////////////////////////////////////////////////////
	mWindow->setDebugText("Camera zoom: " + StringConverter::toString(mCameraZoom)+ " pos: " 
        + StringConverter::toString(mCamera->getDerivedPosition())
        + " orientation: " + StringConverter::toString(mCamera->getDerivedOrientation()));

    Vector3 pPos = World->getPosition();
    mWindow->setDebugText(  " x: "+ StringConverter::toString(pPos.x)+
                            " y: "+ StringConverter::toString(pPos.y)+
                            " z: "+ StringConverter::toString(pPos.z));
	return true;
}

//=================================================================================================
// Buffered Key Events - Dialog.
//=================================================================================================
void CEvent::keyEventDialog(KeyEvent *e)
{
	switch (e->getKey())
	{
		case KC_RETURN:
		case KC_TAB:
			TextInput::getSingleton().finished();
			break;
		case KC_DELETE:
		case KC_BACK:
			TextInput::getSingleton().backspace();
			break;
		case KC_ESCAPE:
			TextInput::getSingleton().canceled();
			break;
		default:
/*
			if (e->getKeyChar()) 
				TextInput::getSingleton().addChar(e->getKeyChar());
*/
			TextInput::getSingleton().keyEvent(e->getKeyChar(), e->getKey());
			break;
	}
}

//=================================================================================================
// Buffered Key Events.
//=================================================================================================
void CEvent::keyPressed(KeyEvent *e) 
{
	mIdleTime =0;
	if (Dialog::getSingleton().isVisible()) 
	{
		keyEventDialog(e);
		e->consume();
		return;
	}

	switch (e->getKey())
	{
		///////////////////////////////////////////////////////////////////////// 
		// Player Movemment.
		/////////////////////////////////////////////////////////////////////////
		
		case KC_UP:
	        ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_WALK, -1);
			break;
		case KC_DOWN:
	        ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_WALK,  1);
			break;
		case KC_RIGHT:
	        ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TURN, -1);
			break;
		case KC_LEFT:
	        ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TURN,  1);
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
		case KC_8:
	        ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_HEAD, 1);
			break;
		case KC_9:
	        ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_BODY, 1);
			break;

		case KC_3:
	        ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,0, -1);
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

		case KC_J:
            ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TURN,  1);
			break;
		case KC_K:
            ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TURN, -1);
			break;
		case KC_G:
            ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_WALK, -1);
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

		///////////////////////////////////////////////////////////////////////// 
		// Engine settings.
		/////////////////////////////////////////////////////////////////////////
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
		    mCamera->pitch(Radian(-0.1));
			break;
		case KC_PGDOWN:
		    mCamera->pitch(Radian(+0.1));
			break;

		case KC_ADD:
			mCameraZoom -= 10;
			mCamera->setPosition(Vector3(0,mCameraZoom, mCameraZoom));
		    mCamera->setNearClipDistance(mCameraZoom);
			break;
            
		case KC_SUBTRACT:
			mCameraZoom += 10;
			mCamera->setPosition(Vector3(0,mCameraZoom, mCameraZoom));
		    mCamera->setNearClipDistance(mCameraZoom);
			break;

		///////////////////////////////////////////////////////////////////////// 
		// Screenshot.
		/////////////////////////////////////////////////////////////////////////
		case KC_SYSRQ:
		    {
				static int mNumScreenShots=0;
				char tmp[20];
				sprintf(tmp, "screenshot_%d.png", ++mNumScreenShots);
		        mWindow->writeContentsToFile(tmp);
		        mTimeUntilNextToggle = 0.5;
				mWindow->setDebugText(String("Wrote ") + tmp);
			}
			break;

		///////////////////////////////////////////////////////////////////////// 
		// Exit game.
		/////////////////////////////////////////////////////////////////////////
		case KC_ESCAPE:
			mQuitGame = true;
			break;
		default:
			break;
	}
//	e->consume();
}

void CEvent::keyClicked(KeyEvent* e) 
{
}

void CEvent::keyReleased(KeyEvent* e) 
{
	switch (e->getKey())
	{
		///////////////////////////////////////////////////////////////////////// 
		// Player Movemment.
		/////////////////////////////////////////////////////////////////////////
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

//=================================================================================================
// Buffered Mouse Events.
//=================================================================================================
void CEvent::mouseMoved (MouseEvent *e)
{
	mMouseX += e->getRelX();
	mMouseY += e->getRelY();
	if (mMouseX <0.000) mMouseX =0.0;
	if (mMouseY <0.000) mMouseY =0.0;
	if (mMouseX >0.995) mMouseX =0.995;
	if (mMouseY >0.985) mMouseY =0.985;
	mMouseCursor->setScroll(mMouseX*2 , -mMouseY*2);
	e->consume();
}

void CEvent::mouseDragged(MouseEvent *e)
{
	if (!TextWin->MouseAction(M_DRAGGED, mMouseX, mMouseY, e->getRelY()*mSreenHeight)) { return; }
	if (!ChatWin->MouseAction(M_DRAGGED, mMouseX, mMouseY, e->getRelY()*mSreenHeight)) { return; }
	mouseMoved(e);
	e->consume();
}

void CEvent::mouseClicked (MouseEvent *e)
{
	if (!TextWin->MouseAction(M_CLICKED, mMouseX, mMouseY)) { return; }
	if (!ChatWin->MouseAction(M_CLICKED, mMouseX, mMouseY)) { return; }
	mouseMoved(e);
	e->consume();
}

void CEvent::mouseEntered (MouseEvent *e)
{
	TextWin->MouseAction(M_ENTERED, mMouseX, mMouseY);
	ChatWin->MouseAction(M_ENTERED, mMouseX, mMouseY);
	mouseMoved(e);
	e->consume();
}

void CEvent::mouseExited  (MouseEvent *e)
{
	TextWin->MouseAction(M_EXITED,mMouseX, mMouseY);
	ChatWin->MouseAction(M_EXITED,mMouseX, mMouseY);
	mouseMoved(e);
	e->consume();
}

void CEvent::mousePressed (MouseEvent *e)
{
	TextWin->MouseAction(M_PRESSED, mMouseX, mMouseY);
	ChatWin->MouseAction(M_PRESSED, mMouseX, mMouseY);
	mouseMoved(e);
	e->consume();
}

void CEvent::mouseReleased(MouseEvent *e)
{
	TextWin->MouseAction(M_RELEASED, mMouseX, mMouseY);
	ChatWin->MouseAction(M_RELEASED, mMouseX, mMouseY);
	mouseMoved(e);
	e->consume();
}
