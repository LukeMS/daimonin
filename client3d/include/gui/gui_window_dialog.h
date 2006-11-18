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
#ifndef DIALOG_INTERFACE_H
#define DIALOG_INTERFACE_H

#include <Ogre.h>
#include "define.h"
#include "gui_gadget_button.h"

using namespace Ogre;

/**
 ** This class provides a graphical dialog window.
 *****************************************************************************/
class GuiDialog
{
public:

    enum
    {
        MAX_TEXTAREA_CHAR = 4096,
        MAX_TEXTLINE_CHAR = 128,
        MAX_LABEL_CHAR = 64,
        MAX_LINE_CHAR = 256,
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
    bool mouseEvent(int index);
    bool keyEvent(const char keyChar, const unsigned char key);
    bool load(int mode, char *data, int len, int pos);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables.
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
    }
    ;

    typedef struct
    {
        char label[MAX_LABEL_CHAR];      /**< Text displayed on the button. **/
        char command[MAX_TEXTLINE_CHAR]; /**< After button was pressed, this command will be send to server. **/
    }
    Button;
    Button butAccept, butDecline;

    typedef struct
    {
        int face;                          /**< ID of the picture.       **/
        char name[MAX_TEXTLINE_CHAR];      /**< Name of the picture.     **/
        //_Sprite *picture;                /**< Pointer to the gfx data. **/
        char body_text[MAX_TEXTLINE_CHAR]; /**< Title-text of the head.  **/
    }
    Head;
    Head head;

    typedef struct
    {
        char link[MAX_TEXTLINE_CHAR];
        char cmd[MAX_TEXTLINE_CHAR];
    }
    Link;
    Link link[MAX_LINKS];

    typedef struct
    {
        char body[MAX_TEXTLINE_CHAR];
    }
    Who;
    Who who;

    typedef struct
    {
        char text[MAX_TEXTLINE_CHAR];
    }
    TextInput;
    TextInput textfield;

    typedef struct
    {
        char title[MAX_TEXTLINE_CHAR];
        char body_text[MAX_TEXTAREA_CHAR];
        int line_count;
        char lines[MAX_MSG_LINE][MAX_LINE_CHAR];
    }
    Message;
    Message message;

    typedef struct
    {
        char title[MAX_TEXTLINE_CHAR];
        char body_text[MAX_TEXTAREA_CHAR];
        int line_count;
    }
    Extended;
    Extended xtended;

    typedef struct
    {
        int copper;
        int silver;
        int gold;
        int mithril;
        int line_count;
        char title[MAX_TEXTLINE_CHAR];
        char body_text[MAX_TEXTAREA_CHAR];
        char lines[MAX_ELEMENT][MAX_LINE_CHAR];
    }
    Reward;
    Reward reward;

    typedef struct
    {
        char mode;
        int num;                           /**< ID of the icon. **/
        char title[MAX_TEXTLINE_CHAR];
        char name [MAX_TEXTLINE_CHAR];     /**< Picture name. **/
        //item element;
        //_Sprite *picture;                /**< Pointer to the gfx data. **/
        char *second_line;
        char body_text[MAX_TEXTLINE_CHAR]; /**< Head title. **/
    }
    Icon;
    Icon icon[MAX_ELEMENT];

    bool mVisible;
    bool mIcon_select;
    bool mLink_selected;
    int mMode;
    int mStatus;
    int mIcon_count;
    int mLink_count;
    int mInput_flag;
    int mSelected;
    uint32 mUsed_flag;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiDialog();
    ~GuiDialog();
    GuiDialog(const GuiDialog&); // disable copy-constructor.
    bool cmd_head     (Head     *head, char *data, int *pos);
    bool cmd_link     (Link     *head, char *data, int *pos);
    bool cmd_who      (Who      *head, char *data, int *pos);
    bool cmd_reward   (Reward   *head, char *data, int *pos);
    bool cmd_icon     (Icon     *head, char *data, int *pos);
    bool cmd_message  (Message   *msg, char *data, int *pos);
    bool cmd_xtended  (Extended  *msg, char *data, int *pos);
    bool cmd_textfield(TextInput *inp, char *data, int *pos);
    bool cmd_button   (Button *button, char *data, int *pos);
    bool getElement(int line, int *element, int *index, char **keyword);
    char *get_parameter_string(char *data, int *pos);
    void format_gui_interface();
    void sendCommand(int mode, char *cmd);
};

#endif
