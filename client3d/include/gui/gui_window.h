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
#include "gui_element_slot.h"
#include "gui_element_statusbar.h"

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
        mInit = false;
    }
    void loadResources(bool dnd);
    void freeRecources();
    bool isVisible()
    {
        return mInit?mOverlay->isVisible():false;
    }
    void setVisible(bool visible);
    void Init(TiXmlElement *xmlElem, const char *resourceWin, const char *resourceDnD, int winNr, unsigned char defaultZPos);
    int keyEvent(const int keyChar, const unsigned int key);
    void update(Ogre::Real timeSinceLastFrame);
    void getSize(int &w, int &h)
    {
        w = mWidth;
        h = mHeight;
    }
    int getWidth()                 { return mWidth;    }
    int getHeight()                { return mHeight;   }
    bool isInit()                  { return mInit;     }
    int getID()                    { return mWindowNr; }
    int getSumElements()           { return (int)mvElement.size(); }
    Ogre::Texture *getTexture()    { return mTexture.getPointer(); }
    Ogre::PixelBox *getPixelBox()  { return &mSrcPixelBox;   }
    Ogre::uint32 *getLayerBG()     { return mWinLayerBG;     }
    unsigned char getZPos()
    {
        return mInit?mOverlay->getZOrder():0;
    }
    void setZPos(unsigned char zorder)
    {
        if (mInit) mOverlay->setZOrder(zorder);
    }
    int mouseEvent(int MouseAction, Ogre::Vector3 &mouse);
    bool mouseWithin(int x, int y)
    {
        return (!mInit || !isVisible() || x < mPosX || x > mPosX + mWidth || y < mPosY || y > mPosY + mHeight)?false:true;
    }
    void centerWindowOnMouse(int x, int y);
    int sendMsg(int elementNr, int message, void *parm1, void *parm2, void *parm3);
    // ////////////////////////////////////////////////////////////////////
    // GUI_Statusbar stuff.
    // ////////////////////////////////////////////////////////////////////
    void setStatusbarValue(int element, Ogre::Real value);
    // ////////////////////////////////////////////////////////////////////
    // GUI_Element_Slot stuff.
    // ////////////////////////////////////////////////////////////////////
    int getDragSlot()
    {
        return mElementDrag;
    }
    void hideDragOverlay()
    {
        if (mSlotReference) mSlotReference->hideDragOverlay();
    }
    void moveDragOverlay()
    {
        if (mSlotReference) mSlotReference->moveDragOverlay();
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static GuiElementSlot *mSlotReference;
    static int mMouseDragging;
    Ogre::String mResourceName;
    short mPosX, mPosY;
    int mWindowNr;
    int mMouseOver;
    int mWidth, mHeight;
    int mDragPosX1, mDragPosX2, mDragPosY1, mDragPosY2, mDragOffsetX, mDragOffsetY;
    int mMinimized, mDefaultHeight;
    int mElementDrag;
    unsigned int mSumUsedSlots;
    bool mInit;
    bool mSizeRelative;
    bool mLockSlots; /**< TODO: Lock all slots, so no item can accidental be removed. **/
    std::vector<class GuiElement*>mvElement;
    std::vector<class GuiStatusbar*>mvStatusbar; // Todo: change this to mvElement!
    Ogre::Overlay *mOverlay;
    Ogre::OverlayElement *mElement;
    Ogre::PixelBox mSrcPixelBox;
    Ogre::TexturePtr mTexture;
    Ogre::uint32 *mWinLayerBG; /**< Its a backup of the window background to avoid
                                    read access to the window texture and to restore
                                    the background after a dynamic part of the win
                                    has changed (e.g. button that changed to inisible)*/
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static void buttonPressed(GuiWindow *me, int index);
    void checkForOverlappingElements();
    void setHeight(int h);
    void delElement(int number);
    void parseWindowData(TiXmlElement *xmlElem, const char *resourceWin, unsigned char defaultZPos);
};

#endif
