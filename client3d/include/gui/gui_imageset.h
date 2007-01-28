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

#ifndef GUI_IMAGESET_H
#define GUI_IMAGESET_H

#include <tinyxml.h>
#include <Ogre.h>
#include <vector>

/**
 ** This singleton class stores the graphic positions of all gui elements.
 ** All graphics are stored in a single gfx-file.
 ** Each graphic can have serveral states (like: pressed, mouse over, etc).
 *****************************************************************************/
class GuiImageset
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {
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
        // TextValues.
        GUI_TEXTVALUE_STAT_CUR_FPS,
        GUI_TEXTVALUE_STAT_BEST_FPS,
        GUI_TEXTVALUE_STAT_WORST_FPS,
        GUI_TEXTVALUE_STAT_SUM_TRIS,
        GUI_TEXTBOX_SERVER_INFO1,
        GUI_TEXTBOX_SERVER_INFO2,
        GUI_TEXTBOX_SERVER_INFO3,
        GUI_TEXTBOX_LOGIN_INFO1,
        GUI_TEXTBOX_LOGIN_INFO2,
        GUI_TEXTBOX_LOGIN_INFO3,
        GUI_TEXTBOX_LOGIN_WARN,
        GUI_TEXTBOX_NPC_HEADLINE,
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
        GUI_SLOT_CONTAINER,
        // Sum of all entries.
        GUI_ELEMENTS_SUM
    };

    typedef struct
    {
        const char *name;
        unsigned int index;
    }
    GuiElementNames;

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
        STATE_MOUSE_SUM
    };
    static GuiElementNames mMouseState[STATE_MOUSE_SUM];

    /** Actual state of an GuiElement: **/
    enum
    {
        STATE_ELEMENT_DEFAULT,
        STATE_ELEMENT_PUSHED,
        STATE_ELEMENT_M_OVER,  /**< Mouse over. **/
        STATE_ELEMENT_PASSIVE, /**< Disabled. **/
        STATE_ELEMENT_SUM
    };
    static GuiElementNames mElementState[STATE_ELEMENT_SUM];

    typedef struct
    {
        short x, y;
    }
    gfxPos;

    typedef struct
    {
        Ogre::String name;
        int width, height;
        bool alpha;
        gfxPos state[STATE_ELEMENT_SUM];
    }
    GuiSrcEntry;

    typedef struct
    {
        int width, height;
        bool alpha;
        gfxPos state[STATE_MOUSE_SUM];
    }
    GuiSrcEntryMouse;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiImageset &getSingleton()
    {
        static GuiImageset singleton; return singleton;
    }
    void parseXML(const char *XML_imageset_file);
    GuiSrcEntry *getStateGfxPositions(const char* guiImage);
    void deleteStateGfxPositions(const char* guiImage);
    GuiSrcEntryMouse *getStateGfxPosMouse();
    Ogre::PixelBox &getPixelBox()
    {
        return mSrcPixelBox;
    }
    const char *getElementName(int i);
    int getElementIndex(int i);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    GuiSrcEntryMouse *mSrcEntryMouse;
    std::vector<GuiSrcEntry*>mvSrcEntry;
    Ogre::String mStrImageSetGfxFile;
    Ogre::Image mImageSetImg;
    Ogre::PixelBox mSrcPixelBox;
    static GuiElementNames mGuiElementNames[GUI_ELEMENTS_SUM];

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiImageset();
    ~GuiImageset();
    GuiImageset(const GuiImageset&); // disable copy-constructor
    bool parseStates(TiXmlElement *xmlElem, gfxPos *Entry, int sum_state, bool mouseStates);
};

#endif
