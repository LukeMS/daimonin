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
#include "item.h"
#include "gui_textout.h"
#include "gui_gadget_slot.h"
#include "gui_gadget_button.h"
#include "gui_gadget_combobox.h"
#include "gui_gadget_scrollbar.h"
#include "gui_table.h"
#include "gui_graphic.h"
#include "gui_listbox.h"
#include "gui_statusbar.h"

// Define: Gadget elements are small interactive objects.

/**
 ** This class provides a graphical window.
 *****************************************************************************/
class GuiWindow
{
public:
    enum { TIME_DOUBLECLICK = 200 };
    /** Mouse Events **/
    enum
    {
        MOUSE_MOVEMENT,
        MOUSE_RESIZING,
        BUTTON_PRESSED,
        BUTTON_CLICKED,
        BUTTON_RELEASED,
        DRAGGING,
        DRAG_ENTER,
        DRAG_EXIT,
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ~GuiWindow() {}
    GuiWindow()
    {
        mInit = false;
    }
    void loadDnDResources()
    {
        mvSlot[0]->loadResources();
    }
    void loadResources();
    void freeRecources();
    bool isVisible()
    {
        return mInit?mOverlay->isVisible():false;
    }
    void setVisible(bool visible);
    void Init(TiXmlElement *xmlElem, const char *resourceWin, const char *resourceDnD, int winNr, unsigned char defaultZPos);
    bool keyEvent(const char keyChar, const unsigned char key);
    void update(Ogre::Real timeSinceLastFrame);
    void getTexturseSize(int &w, int &h)
    {
        w = mWidth;
        h = mHeight;
    }
    int getWidth()
    {
        return mWidth;
    }
    int getHeight()
    {
        return mHeight;
    }
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
    const char *getTooltip()
    {
        return mStrTooltip.c_str();
    }
    Ogre::Texture *getTexture()
    {
        return mTexture.getPointer();
    }
    Ogre::PixelBox *getPixelBox()
    {
        return &mSrcPixelBox;
    }
    Ogre::uint32 *getLayerBG()
    {
        return mWinLayerBG;
    }
    void centerWindowOnMouse(int x, int y);

    void setVisible(int element, bool visible);
    // ////////////////////////////////////////////////////////////////////
    // GUI_Element stuff.
    // ////////////////////////////////////////////////////////////////////
    const Ogre::String &getElementText(int element);
    void setElementText(int element, const char *text);
    // ////////////////////////////////////////////////////////////////////
    // GUI_Statusbar stuff.
    // ////////////////////////////////////////////////////////////////////
    void setStatusbarValue(int element, Ogre::Real value);
    // ////////////////////////////////////////////////////////////////////
    // GUI_Table stuff.
    // ////////////////////////////////////////////////////////////////////
    int  getTableSelection(int element);
    int  getTableActivated(int element);
    bool getTableUserBreak(int element);
    void addTableRow(int element, const char *text);
    void clearTable(int element);
    // ////////////////////////////////////////////////////////////////////
    // GUI_Listbox stuff.
    // ////////////////////////////////////////////////////////////////////
    void clearListbox(int element);
    int  addTextline(int element, const char *text, Ogre::uint32 color);
    // ////////////////////////////////////////////////////////////////////
    // GUI_Button stuff.
    // ////////////////////////////////////////////////////////////////////
    class GuiGadgetButton *getButtonHandle(int element);
    class GuiGadgetSlot *getSlotHandle(int element);
    // ////////////////////////////////////////////////////////////////////
    // GUI_Gadget_Slot stuff.
    // ////////////////////////////////////////////////////////////////////
    void setSlotBusyTime(int element, Ogre::Real busyTime);
    void setSlotBusy(int element);
    void addItem(Item::sItem *item);
    void delItem(Item::sItem *item);
    void clrItem();
    int getDragSlot()
    {
        return mGadgetDrag;
    }
    void hideDragOverlay()
    {
        mvSlot[0]->hideDragOverlay();
    }
    void moveDragOverlay()
    {
        mvSlot[0]->moveDragOverlay();
    }
    int getMouseOverSlot(int x, int y);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static int mMouseDragging;
    static Ogre::String mStrTooltip;
    Ogre::String mResourceName;
    short mPosX, mPosY;
    int mWindowNr;
    int mMouseOver;
    int mWidth, mHeight;
    int mDragPosX1, mDragPosX2, mDragPosY1, mDragPosY2, mDragOffsetX, mDragOffsetY;
    int mMinimized, mDefaultHeight;
    int mGadgetDrag;
    unsigned int mSumUsedSlots;
    bool mInit;
    bool mSizeRelative;
    bool mLockSlots; /**< TODO: Lock all slots, so no item can accidental be removed. **/
    std::vector<class GuiTable*>mvTable;
    std::vector<class GuiGraphic*>mvGraphic;
    std::vector<class GuiListbox*>mvListbox;
    std::vector<class GuiStatusbar*>mvStatusbar;
    std::vector<class GuiGadgetSlot*>mvSlot;
    std::vector<class GuiGadgetButton *>mvGadgetButton;
    std::vector<class GuiGadgetCombobox*>mvGadgetCombobox;
    std::vector<class GuiGadgetScrollbar*>mvGadgetScrollbar;
    std::vector<GuiTextout::TextLine*>mvTextline;
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
    static void listboxPressed(GuiWindow *me, int index, int line);
    void checkForOverlappingElements();
    void setHeight(int h);
    void delGadget(int number);
    void parseWindowData(TiXmlElement *xmlElem, const char *resourceWin, unsigned char defaultZPos);
    void printParsedTextline(TiXmlElement *xmlElem);
};

#endif
