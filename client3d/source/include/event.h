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

#ifndef EVENT_H
#define EVENT_H

#include <Ogre.h>
#include <OgreKeyEvent.h>
#include <OgreEventListeners.h>
#include <OgreStringConverter.h>
#include <OgreException.h>

#include "player.h"

using namespace Ogre;

enum { LIGHT_VOL, LIGHT_SPOT };

class Event: public FrameListener, public KeyListener, public MouseMotionListener, public MouseListener
{
  public:
    SceneNode *World;

	void setLightMember(Light *light, int nr) { mLight[nr] = light;}
	void setResolutionMember(int SreenWidth, int SreenHeight)
	{ 
	    mSreenHeight = SreenHeight;
		mSreenWidth  = SreenWidth;
	}

	// Constructor takes a RenderWindow because it uses that to determine input context
    Event(RenderWindow* win, Camera* cam, MouseMotionListener *mMMotionListener, 
		MouseListener *mMListener, bool useBufferedInputKeys = false, bool useBufferedInputMouse = true);
    ~Event();

  private:
	int mSceneDetailIndex;
    Real mMoveSpeed;
    Real mMoveScale;
	Real mIdleTime;
    Degree mRotateSpeed;
    Degree mRotScale;
    Overlay *mDebugOverlay;
	Overlay *mMouseCursor; 
    EventProcessor* mEventProcessor;
    InputReader* mInputDevice;
    MouseMotionListener *mMouseMotionListener;
	MouseListener *mMouseListener;
    Vector3 mTranslateVector;
    RenderWindow* mWindow;
    Camera* mCamera;
    Light *mSpotLight, *mVolLight;
    int mSreenHeight, mSreenWidth;
    Real mMouseX, mMouseY;
    Real mTimeUntilNextToggle; // just to stop toggles flipping too fast
    TextureFilterOptions mFiltering;
    int mAniso;
    bool mQuitGame;
    Light *mLight[2];
    int mDayTime;

	/////////////////////////////////////////////////////////////////////////
    // Frame Events.
	/////////////////////////////////////////////////////////////////////////
    bool frameStarted(const FrameEvent& evt);
    bool frameEnded  (const FrameEvent& evt);

    ///////////////////////////////////////////////////////////////////////// 
    // Key Events.
	/////////////////////////////////////////////////////////////////////////
	void keyClicked (KeyEvent *e);
	void keyPressed (KeyEvent *e);
	void keyReleased(KeyEvent *e);
	void keyEventDialog(KeyEvent *e);

    ///////////////////////////////////////////////////////////////////////// 
    // Mouse Events.
	/////////////////////////////////////////////////////////////////////////
    void mouseMoved   (MouseEvent *e);
    void mouseDragged (MouseEvent *e);
    void mouseClicked (MouseEvent *e);
    void mouseEntered (MouseEvent *e);
    void mouseExited  (MouseEvent *e);
	void mousePressed (MouseEvent *e);
	void mouseReleased(MouseEvent *e);
};

#endif

