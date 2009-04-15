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
 ** This class manages the gui functionality.
 ** ONLY this class is needed to be included from the outside.
 **
 ** Dependencies:
 ** <Ogre.h>
 ** <tinyxml.h>
 ** <OISKeyboard.h>
 ** "logger.h"
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
        /** User action on these elements will be handled inside the gui only. **/
        GUI_BUTTON_CLOSE,
        GUI_BUTTON_MINIMIZE,
        GUI_BUTTON_MAXIMIZE,
        GUI_BUTTON_RESIZE,
        /** User action on these elements will be send to the world outside. **/
        // Buttons.
        GUI_BUTTON_NPC_ACCEPT,
        GUI_BUTTON_NPC_DECLINE,
        GUI_BUTTON_TEST,
        // Listboxes.
        GUI_LIST_MSGWIN,
        GUI_LIST_CHATWIN,
        GUI_LIST_NPC,
        // StatusBars.
        GUI_STATUSBAR_NPC_MANA,
        GUI_STATUSBAR_NPC_GRACE,
        GUI_STATUSBAR_NPC_HEALTH,
        GUI_STATUSBAR_PLAYER_MANA,
        GUI_STATUSBAR_PLAYER_GRACE,
        GUI_STATUSBAR_PLAYER_HEALTH,
        // TextValues.
        GUI_TEXTBOX_STAT_CUR_FPS,
        GUI_TEXTBOX_STAT_BEST_FPS,
        GUI_TEXTBOX_STAT_WORST_FPS,
        GUI_TEXTBOX_STAT_SUM_TRIS,
        GUI_TEXTBOX_SERVER_INFO1,
        GUI_TEXTBOX_SERVER_INFO2,
        GUI_TEXTBOX_SERVER_INFO3,
        GUI_TEXTBOX_LOGIN_WARN,
        GUI_TEXTBOX_LOGIN_PSWDVERIFY,
        GUI_TEXTBOX_LOGIN_INFO1,
        GUI_TEXTBOX_LOGIN_INFO2,
        GUI_TEXTBOX_LOGIN_INFO3,
        GUI_TEXTBOX_NPC_HEADLINE,
        GUI_TEXTBOX_INV_EQUIP,
        GUI_TEXTBOX_INV_EQUIP_WEIGHT,
        // TextInput
        GUI_TEXTINPUT_LOGIN_NAME,
        GUI_TEXTINPUT_LOGIN_PASSWD,
        GUI_TEXTINPUT_LOGIN_VERIFY,
        GUI_TEXTINPUT_NPC_DIALOG,
        // Table
        GUI_TABLE,
        // Combobox
        GUI_COMBOBOX_TEST,
        // Slots
        GUI_SLOT_QUICKSLOT,
        GUI_SLOT_EQUIPMENT,
        GUI_SLOT_INVENTORY,
        GUI_SLOTGROUP_INVENTORY,
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
        WIN_PLAYERTARGET,
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
        MSG_CLOSE_PARENT,
        MSG_CLEAR,
        MSG_ADD_ROW,
        MSG_ADD_ITEM,
        MSG_DEL_ITEM,
        MSG_GET_USERBREAK,
        MSG_GET_SELECTION,
        MSG_GET_ACTIVATED,
        MSG_SET_TEXT,
        MSG_SET_VALUE,
        MSG_SET_VISIBLE,
        MSG_GET_KEYWORD
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
    static const char *FILE_ITEM_ATLAS;
    static const char *FILE_SYSTEM_FONT;
    static const char *FILE_ITEM_UNKNOWN;
    /** Read only description files **/
    static const char *FILE_TXT_WINDOWS;
    static const char *FILE_TXT_IMAGESET;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiManager &getSingleton()
    {
        static GuiManager singleton;
        return singleton;
    }
    void Init(int w, int h, bool createMedia, bool printInfo, const char *soundActionFailed, const char *pathTxt, const char *pathGfx, const char *pathFonts, const char *pathItems);

    Ogre::uint32 *getBuildBuffer()    { return mBuildBuffer;}
    void resizeBuildBuffer(size_t size);

    static Ogre::Overlay *loadResources(int w, int h, Ogre::String name);
    static void loadResources(Ogre::Resource *res);
    void freeRecources();

    void windowToFront(int window);
    void centerWindowOnMouse(int window);
    void showWindow(int window, bool visible);
    void parseWindows ();

    void update(Ogre::Real);  /**< Returns the clicked element or -1 when nothing was clicked. **/
    void setTooltip(const char *text, bool systemMessage = false);
    void displaySystemMessage(const char *text) { setTooltip(text, true); }

    /** The gui doesn't play sounds but stores the filesnames in a string-vector.
        This vector will be accessed via getNextSound() from outside the gui. **/
    void playSound(const char *filename); /**< Stores the filename of a sound in a string-vector. **/
    const char *getNextSound();           /**< Returns the first sound and delete it from the string-vector. **/

    void setMouseState(int action);
    int getScreenWidth()    { return mScreenWidth; }
    int getScreenHeight()   { return mScreenHeight;}
    int getElementIndex(const char *name, int windowID = -1, int getElementIndex = -1);
    const char *getElementName(int index);

    int mouseEvent(int MouseAction, Ogre::Vector3 &mouse);
    bool keyEvent(const int keyChar, const unsigned int key);

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
    int getElementPressed();
    const char *getTextInput() { return mStrTextInput.c_str(); }
    void printText(int width, int height, Ogre::uint32 *dst, int dstLineSkip,
                   Ogre::uint32 *bak, int bakLineSkip, const char *txt, unsigned int fontNr,
                   Ogre::uint32 color = 0xffffffff, bool hideText = false);
    void drawDragElement(const Ogre::PixelBox &src);
    bool getMediaCreation()
    {
        return mCreateMedia;
    }
    bool getPrintInfo()
    {
        return mPrintInfo;
    }
    const Ogre::String &getPathDescription()
    {
        return mPathDescription;
    }
    const Ogre::String &getPathTextures()
    {
        return mPathTextures;
    }
    const Ogre::String &getPathItems()
    {
        return mPathTexturesItems;
    }
    const Ogre::String &getPathFonts()
    {
        return mPathTexturesFonts;
    }

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
    int mDragSrcWin, mDragSrcSlot, mDragSrcContainer;
    int mDragDstWin, mDragDstSlot, mDragDstContainer;
    int mDragSrcItemPosx, mDragSrcItemPosy; // Set on dragStart for moving back on false drag&drop.
    int mTextInputWindow;   /**< The window where a text input is in progress. */
    int mTextInputElement;  /**< The windows-element where a text input is in progress. */
    int mActiveWindow;      /**< The window which was selected by the user. */
    bool mCreateMedia;      /**< Create all media (e.g. atlastextures, raw-fonts). */
    bool mPrintInfo;        /**< Print gui information into the logfile. */

    int mHotSpotX, mHotSpotY;
    unsigned int mScreenWidth, mScreenHeight;
    unsigned long mTooltipDelay;
    bool mIsDragging;
    bool mTextInputUserAction;
    std::vector<Ogre::String> mvSound;
    Ogre::uint32 *mBuildBuffer;  /**< Buffer to draw all graphics before blitting them into the texture. **/
    Ogre::Vector3 mMouse;
    static Ogre::Overlay *mOverlay;
    static Ogre::OverlayElement *mElement;
    Ogre::TexturePtr mTexture;
    Ogre::String mSoundWrongInput;
    Ogre::String mStrTooltip;
    Ogre::String mStrTextInput, mBackupStrTextInput;
    Ogre::String mPathDescription, mPathTextures, mPathTexturesFonts, mPathTexturesItems;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiManager()  {}
    ~GuiManager() {}
    GuiManager(const GuiManager&); // disable copy-constructor.
    void drawTooltip();
    int sendMsg(int element, int message, const char *text, Ogre::uint32 param = COLOR_WHITE);
    const char *sendMsg(int elementNr, int info);
    class ResourceLoader : public Ogre::ManualResourceLoader
    {
    public:
        ResourceLoader()  {}
        ~ResourceLoader() {}
        /** Called by Ogre when a manual created resource needs to be reloaded. **/
        void loadResource(Ogre::Resource *resource) { loadResources(resource); }
    };
    static class ResourceLoader mLoader;

public:
    static ResourceLoader *getLoader() { return &mLoader; }
    void clear(int element)                                   { sendMsg(element, MSG_CLEAR,       0); }
    void closeParentWin(int element)                          { sendMsg(element, MSG_CLOSE_PARENT,0); }
    void setText(int element, const char *text)               { sendMsg(element, MSG_SET_TEXT, text); }
    void addLine(int element, const char *text)               { sendMsg(element, MSG_ADD_ROW,  text); }
    void addItem(int element, const char *itemGfx, int param) { sendMsg(element, MSG_ADD_ITEM,itemGfx, param); }
    void delItem(int element, const char *itemGfx)            { sendMsg(element, MSG_DEL_ITEM,    0); }
    void setValue(int element, int value)                     { sendMsg(element, MSG_SET_VALUE,   0, value); }
    void setVisible(int element, bool value)                  { sendMsg(element, MSG_SET_VISIBLE, 0, value==true?1:0); }
    bool getUserBreak(int element)                            { return sendMsg(element, MSG_GET_USERBREAK, 0)?true:false; }
    int getSelection(int element)                             { return sendMsg(element, MSG_GET_SELECTION, 0); }
    int getActivated(int element)                             { return sendMsg(element, MSG_GET_ACTIVATED, 0); }
    int print(int element, const char *text,
              Ogre::uint32 color = COLOR_WHITE)               { return sendMsg(element, MSG_ADD_ROW,  text, color); }
    const char *getKeyword(int element)                       { return sendMsg(element, (int)MSG_GET_KEYWORD); }
};

#endif
