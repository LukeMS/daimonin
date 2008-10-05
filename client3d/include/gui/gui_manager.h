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

#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <Ogre.h>
#include "gui_window.h"

/**
 ** This is the interface to the world outside.
 ** Alawys use this class to access the gui from outside.
 *****************************************************************************/
class GuiManager
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    /** Window ids (sorted by zOrder). **/
    enum
    {
        GUI_WIN_LOGIN,
        GUI_WIN_SERVERSELECT,
        // GUI_WIN_CREATION,
        GUI_WIN_EQUIPMENT,
        GUI_WIN_INVENTORY,
        GUI_WIN_TRADE,
        GUI_WIN_SHOP,
        GUI_WIN_CONTAINER,
        GUI_WIN_TILEGROUND,

        GUI_WIN_PLAYERINFO,
        GUI_WIN_PLAYERCONSOLE,
        GUI_WIN_NPCDIALOG,
        GUI_WIN_TEXTWINDOW,
        GUI_WIN_CHATWINDOW,
        GUI_WIN_STATISTICS,
        GUI_WIN_SUM
    };
    static const int SUM_WIN_DIGITS; /**< Numbers of digits (For string format) **/
    enum
    {
        GUI_MSG_TXT_GET,
        GUI_MSG_TXT_CHANGED,
        GUI_MSG_BAR_CHANGED,
        GUI_MSG_ADD_TABLEROW,
        GUI_MSG_BUT_PRESSED,
        GUI_MSG_GET_SEL_KEY,  /**< Returns the selected keyword **/
        GUI_MSG_SLOT_REDRAW,
        GUI_MSG_SUM
    };
    enum
    {
        EVENT_DRAG_STRT,
        EVENT_DRAG_DONE,
        EVENT_CHECK_NEXT,
        EVENT_CHECK_DONE,
        EVENT_SUM
    };
    enum
    {
        MSG_CHANGE_TEXT, MSG_BUTTON_PRESSED, MSG_SUM
    };
    typedef struct
    {
        const char *name;
        unsigned int index;
    }
    GuiWinNam;

    static const char *GUI_MATERIAL_NAME;
    static const char *OVERLAY_ELEMENT_TYPE;
    static const char *OVERLAY_RESOURCE_NAME;
    static const char *ELEMENT_RESOURCE_NAME;
    static const char *TEXTURE_RESOURCE_NAME;
    static const char *MATERIAL_RESOURCE_NAME;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiManager &getSingleton()
    {
        static GuiManager singleton; return singleton;
    }
    bool mouseInsideGui()
    {
        return mMouseInside;
    }
    Ogre::Overlay *loadResources(int w, int h, Ogre::String name, int posZ);
    void loadResources(Ogre::Resource *res);
    void freeRecources();
    void Init(int w, int h);
    void reloadTexture(Ogre::String &name);
    void parseImageset(const char *XML_imageset_file);
    void parseWindows (const char *XML_windows_file);
    void update(Ogre::Real);
    bool mouseEvent(int MouseAction, Ogre::Vector3 &mouse);
    bool keyEvent(const int keyChar, const unsigned int key);
    const char *sendMessage(int window, int message, int element, void *value1 = 0, void *value2 = 0);
    void setTooltip(const char *text);
    void displaySystemMessage(const char*text);
    void centerWindowOnMouse(int window);
    void showWindow(int window, bool visible);
    int getScreenWidth()  { return mScreenWidth; }
    int getScreenHeight() { return mScreenHeight;}

    void startTextInput(int window, int winElement, int maxChars, bool blockNumbers=false, bool blockWhitespaces=false);
    void resetTextInput();
    bool brokenTextInput();
    bool finishedTextInput();
    bool getUserAction()
    {
        if (!mTextInputUserAction)
            return false;
        mTextInputUserAction = false;
        return true;
    }
    void cancelTextInput();
    const char *getTextInput();
    // ////////////////////////////////////////////////////////////////////
    // GUI_Table stuff.
    // ////////////////////////////////////////////////////////////////////
    int getTableUserBreak(int window, int element)
    {
        return guiWindow[window].getTableUserBreak(element);
    }
    int getTableSelection(int window, int element)
    {
        return guiWindow[window].getTableSelection(element);
    }
    int getTableActivated(int window, int element)
    {
        return guiWindow[window].getTableActivated(element);
    }
    void clearTable(int window, int element)
    {
        guiWindow[window].clearTable(element);
    }

    // ////////////////////////////////////////////////////////////////////
    // GUI_Lisbox stuff.
    // ////////////////////////////////////////////////////////////////////
    void clearListbox(int window, int element)
    {
        guiWindow[window].clearListbox(element);
    }
    int addTextline(int window, int element, const char *text, Ogre::uint32 color = 0x00ffffff);

    // ////////////////////////////////////////////////////////////////////
    // GUI_Button stuff.
    // ////////////////////////////////////////////////////////////////////
    class GuiGadgetButton *getButtonHandle(int window, int element)
    {
        return guiWindow[window].getButtonHandle(element);
    }

    // ////////////////////////////////////////////////////////////////////
    // GUI_gadget_Slot stuff.
    // ////////////////////////////////////////////////////////////////////
    void setSlotBusyTime(int window, int element, Ogre::Real busyTime)
    {
        guiWindow[window].setSlotBusyTime(element, busyTime);
    }
    void setSlotBusy(int window, int element)
    {
        guiWindow[window].setSlotBusy(element);
    }
    void addItem(int window, Item::sItem *item)
    {
        guiWindow[window].addItem(item);
    }
    void delItem(int window, Item::sItem *item)
    {
        guiWindow[window].delItem(item);
    }
    void clrItem(int window)
    {
        guiWindow[window].clrItem();
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static GuiWinNam mGuiWindowNames[GUI_WIN_SUM];
    static class GuiWindow guiWindow[GUI_WIN_SUM];
    int mDragSrcWin, mDragSrcSlot;
    int mDragDstWin, mDragDstSlot;
    int mDragSrcContainer, mDragDestContainer;
    int mDragSrcItemPosx, mDragSrcItemPosy; // Set on dragStart for moving back on false drag&drop.
    int mActiveWindow, mActiveElement;
    int mHotSpotX, mHotSpotY;
    unsigned int mScreenWidth, mScreenHeight;
    unsigned long mTooltipDelay;
    bool mIsDragging;
    bool mMouseInside;       /**< Mouse is used for gui related stuff at the moment. **/
    bool mTooltipRefresh;
    bool mTextInputActive;
    bool mTextInputUserAction;
    Ogre::Vector3 mMouse;
    Ogre::Overlay *mOverlay;
    Ogre::OverlayElement *mElement;
    Ogre::TexturePtr mTexture;
    Ogre::String mStrTooltip, mBackupTextInputString;
    Ogre::String mStrTextInput;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiManager()  {}
    ~GuiManager() {}
    GuiManager(const GuiManager&); // disable copy-constructor.
    void clearTooltip();
    void loadResources(int posZ);
};

#endif
