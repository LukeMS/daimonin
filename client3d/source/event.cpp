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

#include "event.h"
#include "dialog.h"
#include "option.h"
#include "logfile.h"
#include "textwindow.h"
#include "network.h"

using namespace Ogre;

CTextwindow *ChatWin=0, *TextWin=0;

//=================================================================================================
// Constructor.
//=================================================================================================
Event::Event(RenderWindow* win, Camera* cam, MouseMotionListener *mMMotionListener, 
		MouseListener *mMListener, bool useBufferedInputKeys, bool useBufferedInputMouse)
{
    /////////////////////////////////////////////////////////////////////////////////////////
	// Create all Overlays.
	/////////////////////////////////////////////////////////////////////////////////////////
    mDebugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
    showDebugOverlay(true);
    mMouseCursor  = OverlayManager::getSingleton().getByName("CursorOverlay");    
    mMouseX = mMouseY =0;
	mMouseCursor->show();
    Dialog::getSingelton().Init();
    TextWin = new CTextwindow("Message Window", -280, 300);
    ChatWin = new CTextwindow("Chat Window"   , -280, 300);
    TextWin->setChild(ChatWin);
	ChatWin->Print("Welcome to Daimonin 3D.  ", ColourValue::Black); 
	ChatWin->Print("-----------------------  ", ColourValue::Black);
    ChatWin->Print("Checking your sytem...   ", ColourValue::White);
    ChatWin->Print("...              ", ColourValue::White);
    ChatWin->Print("Harddisk space is ok.    ", ColourValue::Green);
    ChatWin->Print("Memory space is ok.      ", ColourValue::Green);
    ChatWin->Print("Video card test skipped. ", ColourValue::White);
    ChatWin->Print("Processor test faild!!   ", ColourValue::Red);
    ChatWin->Print(" Whats this, a toaster?  ", ColourValue::White);
    ChatWin->Print(" Sorry, you need at least", ColourValue::White);
    ChatWin->Print(" 4.5 more GHz.       ", ColourValue::White);
	TextWin->Print("Press 'L' for localhost login");
	TextWin->Print("Page Up/Down for Camera view.");
	TextWin->Print("");
	TextWin->Print("Keep coding...", ColourValue::White);
	TextWin->Print("<polyveg>", ColourValue::White);



	/////////////////////////////////////////////////////////////////////////////////////////
	// .
	/////////////////////////////////////////////////////////////////////////////////////////
    mUseBufferedInputKeys  = useBufferedInputKeys;
	mUseBufferedInputMouse = useBufferedInputMouse;
	mInputTypeSwitchingOn  = mUseBufferedInputKeys || mUseBufferedInputMouse;
    mRotateSpeed = 36;
    mMoveSpeed   = 100;
	if (mInputTypeSwitchingOn)
	{
        mEventProcessor = new EventProcessor();
		mEventProcessor->initialise(win);
		mEventProcessor->startProcessingEvents();
		mEventProcessor->addKeyListener(this);
		mEventProcessor->addMouseMotionListener(this);
		mEventProcessor->addMouseListener(this);			
		mInputDevice =   mEventProcessor->getInputReader();
	}
    else
    {
        mInputDevice = PlatformManager::getSingleton().createInputReader();
        mInputDevice->initialise(win,true, true);

    }
    mMouseMotionListener = mMMotionListener;
    mMouseListener = mMListener;
    mCamera = cam;
    mWindow = win;
    mStatsOn = true;
	mNumScreenShots = 0;
	mTimeUntilNextToggle = 0;
    mSceneDetailIndex = 0;
    mMoveScale = 0.0f;
    mRotScale = 0.0f;
    mTranslateVector = Vector3::ZERO;
    mAniso = 1;
    mFiltering = TFO_BILINEAR;
}

//=================================================================================================
// Destructor.
//=================================================================================================
Event::~Event()
{
	if (mInputTypeSwitchingOn) { delete mEventProcessor; }
    else               { PlatformManager::getSingleton().destroyInputReader( mInputDevice ); }
    if (TextWin) delete TextWin;
    if (ChatWin) delete ChatWin;
}


bool Event::processUnbufferedKeyInput(const FrameEvent& evt)
{
    if (player->getState() != IDLE 
	&& !mInputDevice->isKeyDown(KC_UP) && !mInputDevice->isKeyDown(KC_DOWN))
        player->changeState(IDLE);

    if (mInputDevice->isKeyDown(KC_UP))
    {
        player->changeState(WALK_FORWARD);
        mTranslateVector.z =  sin(player->getFacing()->valueRadians());
        mTranslateVector.x = -cos(player->getFacing()->valueRadians());
    }


    if (mInputDevice->isKeyDown(KC_DOWN))
    {
        player->changeState(WALK_BACKWARD);
        mTranslateVector.z = -sin(player->getFacing()->valueRadians());
        mTranslateVector.x =  cos(player->getFacing()->valueRadians());
    }

    if (mInputDevice->isKeyDown(KC_RIGHT))
    {
        player->setFacing(-player->getTurnSpeed());
	}

    if (mInputDevice->isKeyDown(KC_LEFT))
    {
        player->setFacing(player->getTurnSpeed());
    }

    if (mInputDevice->isKeyDown(KC_PGUP))
    {
        mRot-= Degree(0.13);
        mCamera->pitch(mRot);
    }


    if (mInputDevice->isKeyDown(KC_PGDOWN))
    {
        mRot+= Degree(0.13);
        mCamera->pitch(mRot);
	}

    if (mInputDevice->isKeyDown(KC_L) && mTimeUntilNextToggle <= 0)
    {
		mTimeUntilNextToggle = 0.5;
        Option::getSingelton().toggleLogin();
	    if (Option::getSingelton().getLoginActive())
		    Dialog::getSingelton().visible(true);
		else 
            Dialog::getSingelton().visible(false);
		}


    if( mInputDevice->isKeyDown( KC_ESCAPE) )
    {        
        return false;
    }

    if (mInputTypeSwitchingOn && mInputDevice->isKeyDown(KC_K) && mTimeUntilNextToggle <= 0)
    {
			// must be going from immediate keyboard to buffered keyboard
			switchKeyMode();
        mTimeUntilNextToggle = 1;
    }
    if (mInputDevice->isKeyDown(KC_F) && mTimeUntilNextToggle <= 0)
    {
        mStatsOn = !mStatsOn;
        showDebugOverlay(mStatsOn);

        mTimeUntilNextToggle = 1;
    }
    if (mInputDevice->isKeyDown(KC_T) && mTimeUntilNextToggle <= 0)
    {
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

        showDebugOverlay(mStatsOn);

        mTimeUntilNextToggle = 1;
    }

    if (mInputDevice->isKeyDown(KC_SYSRQ) && mTimeUntilNextToggle <= 0)
    {
		char tmp[20];
		sprintf(tmp, "screenshot_%d.png", ++mNumScreenShots);
        mWindow->writeContentsToFile(tmp);
        mTimeUntilNextToggle = 0.5;
		mWindow->setDebugText(String("Wrote ") + tmp);
    }
		
	if (mInputDevice->isKeyDown(KC_R) && mTimeUntilNextToggle <=0)
	{
		mSceneDetailIndex = (mSceneDetailIndex+1)%3 ;
		switch(mSceneDetailIndex)
		{
			case 0 : mCamera->setDetailLevel(SDL_SOLID) ;     break ;
			case 1 : mCamera->setDetailLevel(SDL_WIREFRAME) ; break ;
			case 2 : mCamera->setDetailLevel(SDL_POINTS) ;    break ;
		}
		mTimeUntilNextToggle = 0.5;
	}

    static bool displayCameraDetails = false;
    if (mInputDevice->isKeyDown(KC_P) && mTimeUntilNextToggle <= 0)
    {
        displayCameraDetails = !displayCameraDetails;
        mTimeUntilNextToggle = 0.5;
        if (!displayCameraDetails)
        mWindow->setDebugText("");
    }

    if (displayCameraDetails)
    {
        // Print camera details
        mWindow->setDebugText("P: " + StringConverter::toString(mCamera->getDerivedPosition()) + " " + 
        "O: " + StringConverter::toString(mCamera->getDerivedOrientation()));
    }

    // Return true to continue rendering
    return true;
}


void Event::showDebugOverlay(bool show)
{
    if (mDebugOverlay)
    {
        if (show) { mDebugOverlay->show(); }
        else      { mDebugOverlay->hide(); }
    }
}


bool Event::frameStarted(const FrameEvent& evt)
{
    if(mWindow->isClosed()) { return false; }

    player->nextFrame(evt);
    World->translate(mTranslateVector);

    if (!mInputTypeSwitchingOn)
   	{
        mInputDevice->capture();
    }


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
		mRot = 0;
	    mTranslateVector = Vector3::ZERO;
	}

    if (mUseBufferedInputKeys)
    {
        // no need to do any processing here, it is handled by event processor and 
			// you get the results as KeyEvents
    }
    else
    {
        if (processUnbufferedKeyInput(evt) == false)
		{
			return false;
		}
    }
	if (Option::getSingelton().getLoginActive()) { Network::getSingelton().Update(); }
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////
bool Event::frameEnded(const FrameEvent& evt)
{
    static String currFps  = "Current FPS: ";
    static String avgFps   = "Average FPS: ";
    static String bestFps  = "Best FPS: ";
    static String worstFps = "Worst FPS: ";
    static String tris     = "Triangle Count: ";

    // update stats when necessary
    try {
        OverlayElement* guiAvg   = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
        OverlayElement* guiCurr  = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
        OverlayElement* guiBest  = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
        OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

        const RenderTarget::FrameStats& stats = mWindow->getStatistics();

        guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
        guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
        guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)
            +" "+StringConverter::toString(stats.bestFrameTime)+" ms");
        guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS)
            +" "+StringConverter::toString(stats.worstFrameTime)+" ms");

        OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
        guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

        OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
        guiDbg->setCaption(mWindow->getDebugText());
		TextWin->Update();
		ChatWin->Update();
	}
	catch(...)
	{
		// ignore
	}
	return true;
}

///////////////////////////////////////////////////////////////////////// 
// Key Events.
////////////////////////////////////////////////////////////////////////
void Event::switchKeyMode() 
{
	mUseBufferedInputKeys = !mUseBufferedInputKeys;
	mInputDevice->setBufferedInput(mUseBufferedInputKeys, mUseBufferedInputMouse);
}

void Event::keyClicked(KeyEvent* e) 
{
	if (e->getKeyChar() == 'k')
	{
		switchKeyMode();
	}
}


///////////////////////////////////////////////////////////////////////// 
// Mouse Events.
/////////////////////////////////////////////////////////////////////////
void Event::mouseMoved (MouseEvent *e)
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

void Event::mouseDragged(MouseEvent *e)
{
	if (!TextWin->MouseAction(M_DRAGGED, mMouseX, mMouseY, e->getRelY()*mSreenHeight)) { return; }
	if (!ChatWin->MouseAction(M_DRAGGED, mMouseX, mMouseY, e->getRelY()*mSreenHeight)) { return; }
	mouseMoved(e);
	e->consume();
}

void Event::mouseClicked (MouseEvent *e)
{
	if (!TextWin->MouseAction(M_CLICKED, mMouseX, mMouseY)) { return; }
	if (!ChatWin->MouseAction(M_CLICKED, mMouseX, mMouseY)) { return; }
	mouseMoved(e);
	e->consume();
}

void Event::mouseEntered (MouseEvent *e)
{
	TextWin->MouseAction(M_ENTERED, mMouseX, mMouseY);
	ChatWin->MouseAction(M_ENTERED, mMouseX, mMouseY);
	mouseMoved(e);
	e->consume();
}

void Event::mouseExited  (MouseEvent *e)
{
	TextWin->MouseAction(M_EXITED,mMouseX, mMouseY);
	ChatWin->MouseAction(M_EXITED,mMouseX, mMouseY);
	mouseMoved(e);
	e->consume();
}

void Event::mousePressed (MouseEvent *e)
{
	TextWin->MouseAction(M_PRESSED, mMouseX, mMouseY);
	ChatWin->MouseAction(M_PRESSED, mMouseX, mMouseY);
	mouseMoved(e);
	e->consume();
}

void Event::mouseReleased(MouseEvent *e)
{
	TextWin->MouseAction(M_RELEASED, mMouseX, mMouseY);
	ChatWin->MouseAction(M_RELEASED, mMouseX, mMouseY);
	mouseMoved(e);
	e->consume();
}
