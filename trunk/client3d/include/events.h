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
#include <OISMouse.h>
#include <OISKeyboard.h>
#include <OISInputManager.h>

//================================================================================================
// Class.
//================================================================================================
class Events: public Ogre::FrameListener, public OIS::KeyListener, OIS::MouseListener
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
        STD_CAMERA_ZOOM =  35,
        MAX_CAMERA_ZOOM = 100
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
    int mSceneDetailIndex;
    OIS::InputManager *mInputManager;
    OIS::Keyboard     *mInputKeyboard;
    OIS::Mouse        *mInputMouse;
    Ogre::Real mIdleTime;
    Ogre::Real mCameraZoom;
    Ogre::Vector3 mMouse;
    Ogre::Real mCamCornerX, mCamCornerY;
    Ogre::SceneNode *mWorld;
    Ogre::SceneManager *mSceneManager;
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
    bool keyPressed(const OIS::KeyEvent &e);
    bool keyReleased(const OIS::KeyEvent &e);
    bool mouseMoved(const OIS::MouseEvent &e);
    bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
    bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);
};

#endif
