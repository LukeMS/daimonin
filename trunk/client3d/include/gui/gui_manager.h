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
    enum
    {
        /** Element id's. **/
        // TextValues.
        GUI_TEXTBOX_STAT_CUR_FPS,
        GUI_TEXTBOX_STAT_BEST_FPS,
        GUI_TEXTBOX_STAT_WORST_FPS,
        GUI_TEXTBOX_STAT_SUM_TRIS,
        GUI_TEXTBOX_SERVER_INFO1,
        GUI_TEXTBOX_SERVER_INFO2,
        GUI_TEXTBOX_SERVER_INFO3,
        GUI_TEXTBOX_LOGIN_INFO1,
        GUI_TEXTBOX_LOGIN_INFO2,
        GUI_TEXTBOX_LOGIN_INFO3,
        GUI_TEXTBOX_LOGIN_WARN,
        GUI_TEXTBOX_LOGIN_PSWDVERIFY,
        GUI_TEXTBOX_NPC_HEADLINE,
        GUI_TEXTBOX_INV_EQUIP,
        GUI_TEXTBOX_INV_EQUIP_WEIGHT,
        // TextInput
        GUI_TEXTINPUT_LOGIN_NAME,
        GUI_TEXTINPUT_LOGIN_PASSWD,
        GUI_TEXTINPUT_LOGIN_VERIFY,
        GUI_TEXTINPUT_NPC_DIALOG,
        // Standard Buttons (Handled inside of gui_windows).
        GUI_BUTTON_CLOSE,
        GUI_BUTTON_OK,
        GUI_BUTTON_CANCEL,
        GUI_BUTTON_MINIMIZE,
        GUI_BUTTON_MAXIMIZE,
        GUI_BUTTON_RESIZE,
        // Unique Buttons (Handled outside of gui_windows).
        GUI_BUTTON_NPC_ACCEPT,
        GUI_BUTTON_NPC_DECLINE,
        GUI_BUTTON_TEST,
        // Listboxes.
        GUI_LIST_MSGWIN,
        GUI_LIST_CHATWIN,
        GUI_LIST_NPC,
        GUI_LIST_UP,
        GUI_LIST_DOWN,
        GUI_LIST_LEFT,
        GUI_LIST_RIGHT,
        // StatusBars.
        GUI_STATUSBAR_NPC_HEALTH,
        GUI_STATUSBAR_NPC_MANA,
        GUI_STATUSBAR_NPC_GRACE,

        GUI_STATUSBAR_PLAYER_MANA,
        GUI_STATUSBAR_PLAYER_GRACE,
        GUI_STATUSBAR_PLAYER_HEALTH,
        // Table
        GUI_TABLE,
        // Combobox
        GUI_COMBOBOX_TEST,
        // Slots
        GUI_SLOT_QUICKSLOT,
        GUI_SLOT_EQUIPMENT,
        GUI_SLOT_INVENTORY,
        GUI_SLOT_CONTAINER,
        GUI_SLOT_TRADE_OFFER,
        GUI_SLOT_TRADE_RETURN,
        GUI_SLOT_SHOP,
        // Sum of all entries.
        GUI_ELEMENTS_SUM
    };
    /** Window id's. **/
    enum
    {
        /** The first window in this list MUST be initialized.
            Because guiWindow[0].xyz() will be used to access some static stuff. **/
        WIN_LOGIN,
        WIN_SERVERSELECT,
        WIN_EQUIPMENT,
        WIN_INVENTORY,
        WIN_TRADE,
        WIN_SHOP,
        WIN_CONTAINER,
        WIN_TILEGROUND,
        WIN_NPCDIALOG,
        WIN_PLAYERINFO,
        WIN_TEXTWINDOW,
        WIN_CHATWINDOW,
        WIN_STATISTICS,
        WIN_PLAYERCONSOLE,
        //WIN_CREATION,
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

    enum
    {
        MSG_CLEAR,
        MSG_UPDATE,
        MSG_ADD_ROW,
        MSG_ADD_ITEM,
        MSG_DEL_ITEM,
        MSG_GET_USERBREAK,
        MSG_GET_SELECTION,
        MSG_GET_ACTIVATED,
        MSG_SET_TEXT,
        MSG_SET_VISIBLE,
        MSG_SUM
    };

    /** Actual state of the mouse cursor: **/
    enum
    {
        STATE_MOUSE_DEFAULT,             /**< Default. **/
        STATE_MOUSE_PUSHED,              /**< Any button down. **/
        STATE_MOUSE_TALK,
        STATE_MOUSE_SHORT_RANGE_ATTACK,
        STATE_MOUSE_LONG_RANGE_ATTACK,
        STATE_MOUSE_OPEN,
        STATE_MOUSE_CAST,
        STATE_MOUSE_DRAGGING,            /**< Dragging in action. **/
        STATE_MOUSE_RESIZING,            /**< Resizing a window. **/
        STATE_MOUSE_PICKUP,
        STATE_MOUSE_STOP,
        STATE_MOUSE_SUM
    };

    static const Ogre::uint32 COLOR_BLACK;
    static const Ogre::uint32 COLOR_BLUE;
    static const Ogre::uint32 COLOR_GREEN;
    static const Ogre::uint32 COLOR_LBLUE;
    static const Ogre::uint32 COLOR_RED;
    static const Ogre::uint32 COLOR_PINK;
    static const Ogre::uint32 COLOR_YELLOW;
    static const Ogre::uint32 COLOR_WHITE;

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
        static GuiManager singleton;
        return singleton;
    }
    bool mouseInsideGui() { return mMouseInside; }
    Ogre::Overlay *loadResources(int w, int h, Ogre::String name);
    void windowToFront(int window);
    void loadResources(Ogre::Resource *res);
    void freeRecources();
    void Init(int w, int h);
    void reloadTexture(Ogre::String &name);
    void parseImageset(const char *XML_imageset_file);
    void parseWindows (const char *XML_windows_file);
    int update(Ogre::Real);  /**< Returns the clicked element or -1 when nothing was clicked. **/
    int getElementIndex(const char *name, int windowID = -1, int getElementIndex = -1);
    bool mouseEvent(int MouseAction, Ogre::Vector3 &mouse);
    bool keyEvent(const int keyChar, const unsigned int key);
    void setTooltip(const char *text, bool systemMessage = false);
    void displaySystemMessage(const char *text) { setTooltip(text, true); }
    void centerWindowOnMouse(int window);
    void showWindow(int window, bool visible);
    Ogre::uint32 *getBuildBuffer()    { return mBuildBuffer;}
    int getScreenWidth()    { return mScreenWidth; }
    int getScreenHeight()   { return mScreenHeight;}
    void startTextInput(int window, int winElement, int maxChars, bool blockNumbers=false, bool blockWhitespaces=false);
    void resetTextInput();
    bool brokenTextInput();
    bool finishedTextInput();
    void setMouseState(int action);
    void loadRawFont(const char *filename);
    int calcTextWidth(const char *text, int fontNr);
    void resizeBuildBuffer(size_t size);
    bool getUserAction()
    {
        if (!mTextInputUserAction)
            return false;
        mTextInputUserAction = false;
        return true;
    }
    void cancelTextInput();
    const char *getTextInput() { return mStrTextInput.c_str(); }
    int sendMsg(int element, int message, const char *text = 0, Ogre::uint32 param = 0x00ffffff);
    void setStatusbarValue(int window, int element, Ogre::Real value);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { NO_ACTIVE_WINDOW =  -1 };
    enum { CURSOR_FREQUENCY = 500 };
    typedef struct
    {
        const char *name;
        unsigned int index;
    }
    WindowID;
    typedef struct
    {
        short windowNr;
        short winElementNr;
        const char *name;
        short index;
    }
    ElementID;

    static WindowID mWindowID[WIN_SUM];
    static ElementID mStateStruct[GUI_ELEMENTS_SUM];
    static short mWindowZPos[WIN_SUM]; /**< The window-numbers are sorted here on the z-pos */
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
    bool mMouseInside;           /**< Mouse is used for gui related stuff at the moment. **/
    bool mTextInputUserAction;
    Ogre::uint32 *mBuildBuffer;  /**< Buffer to draw all graphics before blitting them into the texture. **/
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
    void drawTooltip();
    void loadResources();
};

#endif
