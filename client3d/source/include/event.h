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

#ifndef EVENT_H
#define EVENT_H
#include <Ogre.h>
#include <OgreEventListeners.h>
#include "particle_manager.h"
#include "TileManager.h"
using namespace Ogre;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////

enum { LIGHT_VOL, LIGHT_SPOT };

const Real MIN_CAMERA_ZOOM =  20.0;
const Real MAX_CAMERA_ZOOM = 115.0;

////////////////////////////////////////////////////////////
// Class.
////////////////////////////////////////////////////////////
class CEvent: public FrameListener, public KeyListener, public MouseMotionListener, public MouseListener
{
public:
	////////////////////////////////////////////////////////////
	// Variables.
	////////////////////////////////////////////////////////////
	SceneNode *World;

	////////////////////////////////////////////////////////////
	// Functions.
	////////////////////////////////////////////////////////////
	// Constructor takes a RenderWindow because it uses that to determine input context
		CEvent(RenderWindow* win, Camera* cam, MouseMotionListener *mMMotionListener,
			MouseListener *mMListener, bool useBufferedInputKeys = false, bool useBufferedInputMouse = true);
	~CEvent() ;

	void SetSceneManager(SceneManager *SManager) { mSceneManager = SManager; }
	const Vector3 &getWorldPos()  {  return World->getPosition(); }
	void setWorldPos(Vector3 &pos);
	void setLightMember(Light *light, int nr) { mLight[nr] = light;}
	void setResolutionMember(int SreenWidth, int SreenHeight)
	{
		mSreenHeight = SreenHeight;
		mSreenWidth  = SreenWidth;
	}
	Camera *getCamera() { return mCamera; }

	void Set_pgraphics(TileManager* mTileManager)
	{
		pgTileManager = mTileManager;
	}
	TileManager *pgTileManager;

private:
	////////////////////////////////////////////////////////////
	// Variables.
	////////////////////////////////////////////////////////////
	bool mQuitGame;
	int mSceneDetailIndex;
	SceneManager  *mSceneManager;
	RaySceneQuery *mRaySceneQuery;
	Real mIdleTime;
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
	Light *mLight[2];
	int mDayTime;
	Real mCameraZoom;

	////////////////////////////////////////////////////////////
	// Functions.
	////////////////////////////////////////////////////////////
	bool frameStarted(const FrameEvent& evt);
	bool frameEnded  (const FrameEvent& evt);
	void drawTile(int gfx_nr, int offX, int offY);
	void keyClicked (KeyEvent *e);
	void keyPressed (KeyEvent *e);
	void keyReleased(KeyEvent *e);
	void keyEventDialog(KeyEvent *e);
	void mouseMoved   (MouseEvent *e);
	void mouseDragged (MouseEvent *e);
	void mouseClicked (MouseEvent *e);
	void mouseEntered (MouseEvent *e);
	void mouseExited  (MouseEvent *e);
	void mousePressed (MouseEvent *e);
	void mouseReleased(MouseEvent *e);
};
extern  CEvent *Event;

#endif
