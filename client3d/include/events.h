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

//================================================================================================
// Class.
//================================================================================================
class Events: public Ogre::FrameListener, public Ogre::KeyListener, public Ogre::MouseMotionListener, public Ogre::MouseListener
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        LIGHT_VOL, LIGHT_SPOT
    };
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static Events &getSingleton()
    {
        static Events Singleton; return Singleton;
    }
    void Init(Ogre::RenderWindow* win, Ogre::SceneManager *mSceneMgr);
    void freeRecources();
    Ogre::SceneManager *GetSceneManager()
    {
        return mSceneManager;
    }
    Ogre::SceneNode *GetWorldNode()
    {
        return mWorld;
    }
    const Ogre::Vector3 &getWorldPos()
    {
        return mWorld->getPosition();
    }
    void setWorldPos(int posX, int posZ);
    void setLightMember(Ogre::Light *light, int nr)
    {
        mLight[nr] = light;
    }
    Ogre::Camera *getCamera()
    {
        return mCamera;
    }
    Ogre::Real getCamCornerX()
    {
        return mCamCornerX;
    }
    Ogre::Real getCamCornerY()
    {
        return mCamCornerY;
    }
    bool isShiftDown()
    {
        return mShiftDown;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        MIN_CAMERA_ZOOM =  10,
        STD_CAMERA_ZOOM =  30,
        MAX_CAMERA_ZOOM = 100
    };
    enum
    {
        NONE,
        DEFAULT,
        POSITIVE,
        NEGATIVE,
    } mCameraRotating;
    bool mQuitGame;
    bool mShiftDown;
    int mSceneDetailIndex;
    Ogre::Real mIdleTime;
    Ogre::Real mCameraZoom;
    Ogre::Vector3 mMouse;
    Ogre::Real mCamCornerX, mCamCornerY;
    Ogre::Real mTimeUntilNextToggle; // just to stop toggles flipping too fast
    Ogre::SceneNode *mWorld;
    Ogre::SceneManager *mSceneManager;
    Ogre::EventProcessor *mEventProcessor;
    Ogre::InputReader* mInputDevice;
    Ogre::RenderWindow* mWindow;
    Ogre::Camera* mCamera;
    Ogre::Light *mLight[2];
    Ogre::Light *mSpotLight, *mVolLight;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    Events() {};
    ~Events(){};
    bool frameStarted(const Ogre::FrameEvent& evt);
    bool frameEnded  (const Ogre::FrameEvent& evt);
    bool checkUsername(const char *name);
    void keyClicked (Ogre::KeyEvent *e);
    void keyPressed (Ogre::KeyEvent *e);
    void keyReleased(Ogre::KeyEvent *e);
    void mouseDragEntered(Ogre::MouseEvent *) {}
    void mouseDragExited (Ogre::MouseEvent *) {}
    void mouseDragDropped(Ogre::MouseEvent *) {}
    void mouseMoved   (Ogre::MouseEvent *e);
    void mouseDragged (Ogre::MouseEvent *e);
    void mouseClicked (Ogre::MouseEvent *e);
    void mouseEntered (Ogre::MouseEvent *e);
    void mouseExited  (Ogre::MouseEvent *e);
    void mousePressed (Ogre::MouseEvent *e);
    void mouseReleased(Ogre::MouseEvent *e);
};

#endif
