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
    enum { MAX_OVERLAY_ZPOS = 650 }; /**< According to Ogre documentation. **/
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    /** Window id's. **/
    enum
    {
        WIN_LOGIN,
        WIN_SERVERSELECT,
        // WIN_CREATION,
        WIN_EQUIPMENT,
        WIN_INVENTORY,
        WIN_TRADE,
        WIN_SHOP,
        WIN_CONTAINER,
        WIN_TILEGROUND,

        WIN_PLAYERINFO,
        WIN_PLAYERCONSOLE,
        WIN_NPCDIALOG,
        WIN_TEXTWINDOW,
        WIN_CHATWINDOW,
        WIN_STATISTICS,
        WIN_SUM
    };
    static const int SUM_WIN_DIGITS; /**< Numbers of digits (For string format) **/
    enum
    {
        EVENT_CHECK_DONE,
        EVENT_CHECK_NEXT,
        EVENT_USER_ACTION,
        EVENT_DRAG_STRT,
        EVENT_DRAG_DONE,
        EVENT_SUM
    };

    enum
    {
        MSG_CLEAR,
        MSG_UPDATE,
        MSG_ADD_ROW,
        MSG_GET_USERBREAK,
        MSG_GET_SELECTION,
        MSG_GET_ACTIVATED,
        MSG_GET_KEY_EVENT,
        MSG_GET_MOUSE_EVENT,
        MSG_SET_VISIBLE,
        MSG_SUM
    };

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
    Ogre::Overlay *loadResources(int w, int h, Ogre::String name);
    void windowToFront(int window);
    void loadResources(Ogre::Resource *res);
    void freeRecources();
    void Init(int w, int h);
    void reloadTexture(Ogre::String &name);
    void parseImageset(const char *XML_imageset_file);
    void parseWindows (const char *XML_windows_file);
    void update(Ogre::Real);
    bool mouseEvent(int MouseAction, Ogre::Vector3 &mouse);
    bool keyEvent(const int keyChar, const unsigned int key);
    void setTooltip(const char *text);
    void displaySystemMessage(const char*text);
    void centerWindowOnMouse(int window);
    void showWindow(int window, bool visible);
    int getScreenWidth()     { return mScreenWidth; }
    int getScreenHeight()    { return mScreenHeight;}
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
    const char *getTextInput() { return mStrTextInput.c_str(); }

    int sendMsg(int window, int element, int message, void *parm1 =0, void *parm2 =0, void *parm3 =0)
    {
        return guiWindow[window].sendMsg(element, message, parm1, parm2, parm3);
    }

    // ////////////////////////////////////////////////////////////////////
    // GUI_Element stuff.
    // ////////////////////////////////////////////////////////////////////
    const Ogre::String &getElementText(int window, int element)
    {
        return guiWindow[window].getElementText(element);
    }
    void setElementText(int window, int element, const char *text)
    {
        guiWindow[window].setElementText(element, text);
    }
    // ////////////////////////////////////////////////////////////////////
    // GUI_Statusbar stuff.
    // ////////////////////////////////////////////////////////////////////
    void setStatusbarValue(int window, int element, Ogre::Real value)
    {
        guiWindow[window].setStatusbarValue(element, value);
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { NO_ACTIVE_WINDOW = -1  };
    typedef struct
    {
        const char *name;
        unsigned int index;
    }
    WindowID;
    static WindowID mWindowID[WIN_SUM];
    static class GuiWindow guiWindow[WIN_SUM];
    static unsigned char guiWindowZPos[WIN_SUM]; /**< The window-numbers are sorted here on the z-pos */
    int mDragSrcWin, mDragSrcSlot;
    int mDragDstWin, mDragDstSlot;
    int mDragSrcContainer, mDragDestContainer;
    int mDragSrcItemPosx, mDragSrcItemPosy; // Set on dragStart for moving back on false drag&drop.
    int mTextInputWindow;  /**< The window where a text input is in progress.  */
    int mTextInputElement; /**< The windows-element where a text input is in progress. */
    int mActiveWindow;     /**< The window which was selected by the user.     */

    int mHotSpotX, mHotSpotY;
    unsigned int mScreenWidth, mScreenHeight;
    unsigned long mTooltipDelay;
    bool mIsDragging;
    bool mMouseInside;       /**< Mouse is used for gui related stuff at the moment. **/
    bool mTooltipRefresh;
    bool mTextInputUserAction;
    Ogre::Vector3 mMouse;
    Ogre::Overlay *mOverlay;
    Ogre::OverlayElement *mElement;
    Ogre::TexturePtr mTexture;
    Ogre::String mStrTooltip;
    Ogre::String mStrTextInput, mBackupStrTextInput;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiManager()  {}
    ~GuiManager() {}
    GuiManager(const GuiManager&); // disable copy-constructor.
    void clearTooltip();
    void loadResources();
};

#endif
