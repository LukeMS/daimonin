/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#ifndef EVENTS_H
#define EVENTS_H

#include <Ogre.h>
#include <OISMouse.h>
#include <OISKeyboard.h>
#include <OISInputManager.h>

/**
 ** This singleton class handles all engine and user events.
 *****************************************************************************/
class Events: public Ogre::FrameListener, public OIS::KeyListener, OIS::MouseListener
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static Events &getSingleton()
    {
        static Events Singleton; return Singleton;
    }
    void Init(Ogre::RenderWindow *win, Ogre::SceneManager *mSceneMgr);
    void freeRecources();
    void setWorldPos(int posX, int posZ);
    Ogre::SceneManager *getSceneManager() const
    {
        return mSceneManager;
    }
    Ogre::Camera *getCamera() const
    {
        return mCamera;
    }
    bool isShiftDown() const
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
        STD_CAMERA_ZOOM =  21,
        MAX_CAMERA_ZOOM = 400
    };
    enum
    {
        NONE,
        FREEZE,
        TURNBACK,
        POSITIVE,
        NEGATIVE,
    } mCameraRotating;

    bool mQuitGame;
    bool mShiftDown;
    OIS::InputManager *mInputManager;
    OIS::Keyboard     *mInputKeyboard;
    OIS::Mouse        *mInputMouse;
    Ogre::Real mIdleTime;                  /**< Time without user interaction. **/
    Ogre::Vector3 mMouse;
    Ogre::Camera* mCamera;
    Ogre::Real mCameraZoom;
    Ogre::SceneNode *mWorld;
    Ogre::SceneManager *mSceneManager;
    Ogre::RenderWindow* mWindow;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    Events()  {}
    ~Events() {}
    Events(const Events&); // disable copy-constructor.
    void elementClicked(int element);
    bool checkUsername(const char *name);
    bool frameStarted (const Ogre::FrameEvent &evt);
    bool frameEnded   (const Ogre::FrameEvent &evt);
    bool keyPressed   (const OIS::KeyEvent &e);
    bool keyReleased  (const OIS::KeyEvent &e);
    bool mouseMoved   (const OIS::MouseEvent &e);
    bool mousePressed (const OIS::MouseEvent &e, OIS::MouseButtonID id);
    bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);
};

#endif
