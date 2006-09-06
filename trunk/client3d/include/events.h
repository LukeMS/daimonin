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

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef EVENTS_H
#define EVENTS_H

#include <Ogre.h>
#include <OgreEventListeners.h>
#include "tile_manager.h"

using namespace Ogre;

const Real MIN_CAMERA_ZOOM =  20.0;
const Real MAX_CAMERA_ZOOM =  110.0;

//================================================================================================
// Class.
//================================================================================================
class CEvent: public FrameListener, public KeyListener, public MouseMotionListener, public MouseListener
{
public:
    enum
    {
        LIGHT_VOL, LIGHT_SPOT
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    CEvent(RenderWindow* win, SceneManager *mSceneMgr);
    ~CEvent();
    SceneManager *GetSceneManager()
    {
        return mSceneManager;
    }
    SceneNode *GetWorldNode()
    {
        return mWorld;
    }
    const Vector3 &getWorldPos()
    {
        return mWorld->getPosition();
    }
    void setWorldPos(int posX, int posZ);
    void setLightMember(Light *light, int nr)
    {
        mLight[nr] = light;
    }
    Camera *getCamera()
    {
        return mCamera;
    }
    Real getCamCornerX()
    {
        return mCamCornerX;
    }
    Real getCamCornerY()
    {
        return mCamCornerY;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    bool mQuitGame;
    int mDayTime;
    int mSceneDetailIndex;
    Real mIdleTime;
    Real mCameraZoom;
    Real mMouseX, mMouseY;
    Real mCamCornerX, mCamCornerY;
    Real mTimeUntilNextToggle; // just to stop toggles flipping too fast
    SceneNode *mWorld;
    SceneManager *mSceneManager;
    EventProcessor *mEventProcessor;
    InputReader* mInputDevice;
    RenderWindow* mWindow;
    Camera* mCamera;
    Light *mLight[2];
    Light *mSpotLight, *mVolLight;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    bool frameStarted(const FrameEvent& evt);
    bool frameEnded  (const FrameEvent& evt);
    void keyClicked (KeyEvent *e);
    void keyPressed (KeyEvent *e);
    void keyReleased(KeyEvent *e);
    void mouseMoved   (MouseEvent *e);
    void mouseDragged (MouseEvent *e);
    void mouseClicked (MouseEvent *e);
    void mouseEntered (MouseEvent *e);
    void mouseExited  (MouseEvent *e);
    void mousePressed (MouseEvent *e);
    void mouseReleased(MouseEvent *e);
    void mouseDragEntered(MouseEvent* )
    {}
    void mouseDragExited(MouseEvent* )
    {}
    void mouseDragDropped(MouseEvent* )
{}}
;

extern  CEvent *Event;

#endif
