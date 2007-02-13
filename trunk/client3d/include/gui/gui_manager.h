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

#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <vector>
#include <tinyxml.h>
#include <Ogre.h>
#include "gui_window.h"
#include "gui_cursor.h"
#include "gui_imageset.h"

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
    enum
    {
        GUI_WIN_STATISTICS,
        GUI_WIN_PLAYERINFO,
        GUI_WIN_PLAYERCONSOLE,
        GUI_WIN_TEXTWINDOW,
        GUI_WIN_CHATWINDOW,
        GUI_WIN_SERVERSELECT,
        GUI_WIN_LOGIN,
        //  GUI_WIN_CREATION,
        GUI_WIN_NPCDIALOG,
        GUI_WIN_ITEM_CONTAINER,
        GUI_WIN_SUM
    };
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
        MSG_CHANGE_TEXT, MSG_BUTTON_PRESSED, MSG_SUM
    };
    typedef struct
    {
        const char *name;
        unsigned int index;
    }
    GuiWinNam;

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
    void freeRecources();
    void Init(int w, int h);
    void parseImageset(const char *XML_imageset_file);
    void parseWindows (const char *XML_windows_file);
    void update(Ogre::Real);
    bool mouseEvent(int MouseAction, Ogre::Vector3 &mouse);
    bool keyEvent(const char keyChar, const unsigned char key);
    const char *sendMessage(int window, int message, int element, void *value1 = 0, void *value2 = 0);
    void setTooltip(const char *text);
    void displaySystemMessage(const char*text);
    void startTextInput(int window, int winElement, int maxChars, bool blockNumbers=false, bool blockWhitespaces=false);
    bool brokenTextInput();
    void resetTextInput();
    bool finishedTextInput();
    void cancelTextInput();
    const char *getTextInput();
    void centerWindowOnMouse(int window);
    void showWindow(int window, bool visible);
    int getScreenWidth()
    {
        return mScreenWidth;
    }
    int getScreenHeight()
    {
        return mScreenHeight;
    }

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
    void setItemReference(int window, int element, std::list<Item::sItem*> *itemContainer)
    {
        guiWindow[window].setItemReference(element, itemContainer);
    }

    void updateItemSlot(int window, int element, int slotNr, int state = GuiImageset::STATE_ELEMENT_DEFAULT)
    {
        guiWindow[window].updateItemSlot(element, slotNr, state);
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static GuiWinNam mGuiWindowNames[GUI_WIN_SUM];
    static class GuiWindow guiWindow[GUI_WIN_SUM];
    int mDragSrcWin, mDragDestWin;
    int mDragSrcContainer, mDragDestContainer;
    int mDragSrcItemPosx, mDragSrcItemPosy; // Set on dragStart for moving back on false drag&drop.
    int mActiveWindow, mActiveElement;
    int mHotSpotX, mHotSpotY;
    Ogre::Vector3 mMouse;
    bool mMouseInside;       /**< Mouse is used for gui related stuff at the moment. **/
    bool mTooltipRefresh;
    bool mIsDragging;
    bool mProcessingTextInput;
    unsigned int mScreenWidth, mScreenHeight;
    unsigned long mTooltipDelay;
    Ogre::Overlay *mOverlay;
    Ogre::OverlayElement *mElement;
    Ogre::MaterialPtr mMaterial;
    Ogre::TexturePtr mTexture;
    Ogre::String mStrTooltip, mBackupTextInputString;
    Ogre::String mStrTextInput;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiManager()
    {}
    ~GuiManager()
    {}
    GuiManager(const GuiManager&); // disable copy-constructor.
    //GuiManager& operator=(GuiManager const&);
    bool parseWindowsData (const char *file);
    void clearTooltip();
};

#endif
