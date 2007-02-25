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

#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <vector>
#include <Ogre.h>
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

    enum
    {
        GUI_ACTION_NONE,
        GUI_ACTION_START_TEXT_INPUT,
        GUI_ACTION_SUM
    };
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
    static const char *OVERLAY_ELEMENT_TYPE;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ~GuiWindow()
    {}
    GuiWindow();
    void freeRecources();
    bool isVisible()
    {
        return mOverlay->isVisible();
    }
    void setVisible(bool visible);
    void Init(TiXmlElement *xmlElem, int zOrder);
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
    const char *Message(int message, int element, void *value1, void *value2);
    bool mouseEvent(int MouseAction, Ogre::Vector3 &mouse);
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
    void centerWindowOnMouse(int x, int y);

    // ////////////////////////////////////////////////////////////////////
    // GUI_Table stuff.
    // ////////////////////////////////////////////////////////////////////
    int  getTableSelection(int element);
    int  getTableActivated(int element);
    bool getTableUserBreak(int element);
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
    void updateItemSlot(int element, int slotNr, int state);
    void setItemReference(int element, std::list<Item::sItem*> *itemContainer);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static int msInstanceNr, mMouseDragging;
    static Ogre::String mStrTooltip;
    int mWindowNr;
    int mMousePressed, mMouseOver;
    int mPosX, mPosY, mPosZ, mWidth, mHeight;
    int mHeadPosX, mHeadPosY;
    int mDragPosX1, mDragPosX2, mDragPosY1, mDragPosY2, mDragOldMousePosX, mDragOldMousePosY;
    int mMinimized, mDefaultHeight;
    int mGadgetDrag;
    bool isInit;
    bool mSizeRelative;
    Ogre::SceneNode *mSceneNode;
    std::vector<class GuiTable*>mvTable;
    std::vector<class GuiGraphic*>mvGraphic;
    std::vector<class GuiListbox*>mvListbox;
    std::vector<class GuiStatusbar*>mvStatusbar;
    std::vector<class GuiGadgetSlot*>mvSlot;
    std::vector<class GuiGadgetButton *>mvGadgetButton;
    std::vector<class GuiGadgetCombobox*>mvGadgetCombobox;
    std::vector<class GuiGadgetScrollbar*>mvGadgetScrollbar;
    std::vector<GuiTextout::TextLine*>mvTextline;
    Ogre::Overlay *mOverlay, *mNPC_HeadOverlay;
    Ogre::OverlayElement *mElement;
    Ogre::AnimationState *mSpeakAnimState, *mManualAnimState;
    Ogre::PixelBox mSrcPixelBox;
    Ogre::MaterialPtr mMaterial;
    Ogre::TexturePtr mTexture;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static void buttonPressed(GuiWindow *me, int index);
    static void listboxPressed(GuiWindow *me, int index, int line);
    void createWindow(int zOrder);
    void setHeight(int h);
    void delGadget(int number);
    void parseWindowData(TiXmlElement *xmlElem, int zOrder);
    void printParsedTextline(TiXmlElement *xmlElem);
};

#endif
