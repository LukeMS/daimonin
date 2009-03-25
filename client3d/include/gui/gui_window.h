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

#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <vector>
#include <Ogre.h>
#include <tinyxml.h>

/**
 ** This class provides a graphical window.
 *****************************************************************************/
class GuiWindow
{
public:
    enum { TIME_DOUBLECLICK = 200 };
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ~GuiWindow() {}
    GuiWindow()
    {
        mOverlay = 0;
    }
    void loadResources();
    void freeRecources();
    bool isVisible()
    {
        return mOverlay?mOverlay->isVisible():false;
    }
    void setVisible(bool visible);
    void Init(TiXmlElement *xmlElem, const char *resourceWin, int winNr, unsigned char defaultZPos);
    int keyEvent(const int keyChar, const unsigned int key);
    void update(Ogre::Real timeSinceLastFrame);
    void getSize(int &w, int &h)
    {
        w = mWidth;
        h = mHeight;
    }
    int getWidth()                 { return mWidth;    }
    int getHeight()                { return mHeight;   }
    int getID()                    { return mWindowNr; }
    int getSumElements()           { return (int)mvElement.size(); }
    Ogre::uint32  *getLayerBG()    { return mWinLayerBG; }
    Ogre::Texture *getTexture()    { return mTexture.getPointer(); }
    unsigned char getZPos()
    {
        return mOverlay?mOverlay->getZOrder():0;
    }
    void setZPos(unsigned char zorder)
    {
        if (mOverlay) mOverlay->setZOrder(zorder);
    }
    int mouseEvent(int MouseAction, Ogre::Vector3 &mouse);
    bool mouseWithin(int x, int y)
    {
        return (isVisible() && x > mPosX && x < mPosX + mWidth && y > mPosY && y < mPosY + mHeight)?true:false;
    }
    void centerWindowOnMouse(int x, int y);
    int sendMsg(int elementNr, int message, const char *text, Ogre::uint32 param);
    const char *sendMsg(int elementNr, int info);
    int getDragSlot()
    {
        return mDragElement;
    }
    int getElementPressed()
    {
        int ret = mElementClicked;
        mElementClicked = -1;
        return ret;
    }
    void checkForOverlappingElements();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static int mDragWindowNr;   /**< Number of the window  which is currently moved by the user.    -1 for none. **/
    static int mDragElement;    /**< Number of the element which is currently moved by the user.    -1 for none. **/
    static int mElementClicked; /**< Number of the element which was clicked with left mousebutton. -1 for none. **/
    static int mDragOffsetX, mDragOffsetY;
    short mPosX, mPosY;
    int mWindowNr;
    int mWidth, mHeight;
    int mDragPosX1, mDragPosX2, mDragPosY1, mDragPosY2;
    bool mLockSlots;            /**< Lock all slots, so no item can accidental be removed. **/
    std::vector<class GuiElement*>mvElement;
    Ogre::String mResourceName;
    Ogre::Overlay *mOverlay;
    Ogre::OverlayElement *mElement;
    Ogre::TexturePtr mTexture;
    Ogre::uint32 *mWinLayerBG;  /**< Backup of the window texture to restore the background after a dynamic
                                     element has changed (e.g. button that changed to invisible) */
};

#endif
