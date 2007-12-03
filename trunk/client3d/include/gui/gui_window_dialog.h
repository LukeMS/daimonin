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

#ifndef DIALOG_INTERFACE_H
#define DIALOG_INTERFACE_H

#include "define.h"
#include "gui_gadget_button.h"

/**
 ** This class provides a graphical dialog window.
 *****************************************************************************/
class GuiDialog
{
public:

    enum
    {
        MAX_MSG_LINE = 100,
        MAX_ELEMENT = 15,
        MAX_LINKS = 25,
    };

    /** Which area of the interface is used. **/
    enum
    {
        GUI_INTERFACE_HEAD     = 1 << 0,
        GUI_INTERFACE_MESSAGE  = 1 << 1,
        GUI_INTERFACE_REWARD   = 1 << 2,
        GUI_INTERFACE_ACCEPT   = 1 << 3,
        GUI_INTERFACE_TEXTFIELD= 1 << 4,
        GUI_INTERFACE_DECLINE  = 1 << 5,
        GUI_INTERFACE_BUTTON   = 1 << 6,
        GUI_INTERFACE_WHO      = 1 << 7,
        // internal, don't use.
        GUI_INTERFACE_XTENDED  = 1 << 8,
        GUI_INTERFACE_ICON     = 1 << 9,
        GUI_INTERFACE_LINK     = 1 <<10,
    };

    enum
    {
        INTERFACE_MODE_NO,
        INTERFACE_MODE_NPC,
        INTERFACE_MODE_QLIST
    };

    enum
    {
        GUI_INTERFACE_STATUS_NORMAL,
        GUI_INTERFACE_STATUS_WAIT
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiDialog &getSingleton()
    {
        static GuiDialog Singleton;
        return Singleton;
    }
    void show();
    void reset();
    void buttonEvent(int index);
    void mouseEvent(int index);
    bool keyEvent(const char keyChar, const unsigned char key);
    bool load(int mode, char *data, int len, int pos);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        INTERFACE_CMD_NO       = 1 << 0,
        INTERFACE_CMD_HEAD     = 1 << 1,
        INTERFACE_CMD_MESSAGE  = 1 << 2,
        INTERFACE_CMD_REWARD   = 1 << 3,
        INTERFACE_CMD_ICON     = 1 << 4,
        INTERFACE_CMD_ACCEPT   = 1 << 5,
        INTERFACE_CMD_DECLINE  = 1 << 6,
        INTERFACE_CMD_LINK     = 1 << 7,
        INTERFACE_CMD_TEXTFIELD= 1 << 8,
        INTERFACE_CMD_BUTTON   = 1 << 9,
        INTERFACE_CMD_WHO      = 1 <<10,
        INTERFACE_CMD_XTENDED  = 1 <<11,
    };

    typedef struct
    {
        Ogre::String label;   /**< Text displayed on the button. **/
        Ogre::String command; /**< After button was pressed, this command will be send to server. **/
    }
    Button;
    Button butAccept, butDecline;

    struct _mHead
    {
        int face;           /**< ID of the picture.       **/
        Ogre::String name;        /**< Name of the picture.     **/
        //_Sprite *picture; /**< Pointer to the gfx data. **/
        Ogre::String body_text;   /**< Title-text of the head.  **/
    }
    mHead;

    struct _mLink
    {
        Ogre::String link;
        Ogre::String cmd;
    }
    mLink[MAX_LINKS];

    struct _mWho
    {
        Ogre::String body;
    }
    mWho;

    struct _mTextfield
    {
        Ogre::String text;
    }
    mTextfield;

    struct _mMessage
    {
        Ogre::String title;
        Ogre::String body_text;
        Ogre::String lines[MAX_MSG_LINE];
        int line_count;
    }
    mMessage;

    struct _mXtended
    {
        Ogre::String title;
        Ogre::String body_text;
        int line_count;
    }
    mXtended;

    struct _mReward
    {
        int copper;
        int silver;
        int gold;
        int mithril;
        int line_count;
        Ogre::String title;
        Ogre::String body_text;
        Ogre::String lines;
    }
    mReward;

    struct _mIcon
    {
        char mode;
        int num;           /**< ID of the icon. **/
        Ogre::String title;
        Ogre::String name;       /**< Picture name. **/
        //item element;
        //_Sprite *picture;/**< Pointer to the gfx data. **/
        char *second_line;
        Ogre::String body_text;  /**< Head title. **/
    }
    mIcon[MAX_ELEMENT];

    bool mVisible;
    bool mIcon_select;
    bool mLink_selected;
    int mMode;
    int mStatus;
    int mIcon_count;
    int mLink_count;
    int mInput_flag;
    int mSelected;
    int mUsed_flag;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiDialog();
    ~GuiDialog();
    GuiDialog(const GuiDialog&); // disable copy-constructor.
    bool cmd_head     (char *data, int &pos);
    bool cmd_link     (char *data, int &pos);
    bool cmd_who      (char *data, int &pos);
    bool cmd_reward   (char *data, int &pos);
    bool cmd_icon     (char *data, int &pos);
    bool cmd_message  (char *data, int &pos);
    bool cmd_xtended  (char *data, int &pos);
    bool cmd_textfield(char *data, int &pos);
    bool cmd_button   (Button &button, char *data, int &pos);
    bool getElement(int line, int *element, int *index, Ogre::String *keyword);
    char parseParameter(char *data, int &pos);
    char *get_parameter_string(char *data, int &pos);
    void format_gui_interface();
    void sendCommand(int mode, char *cmd);
};

#endif
