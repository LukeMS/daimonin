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

using namespace Ogre;

const int INTERFACE_WINLEN_NPC = 353;
const int INTERFACE_MAX_LINE = 100;
const int INTERFACE_MAX_CHAR = 256;
const int INTERFACE_MAX_REWARD_LINE = 15;
const int MAX_INTERFACE_ICON = 15;
const int MAX_INTERFACE_LINKS = 25;


typedef struct
{
    int face;
    char name[128];          // face (picture name)
    //    _Sprite *picture;      // the real picture
    char body_text[128]; // head title
}
_gui_interface_head;

typedef struct
{
    char link[128];
    char cmd[128];
}
_gui_interface_link;

typedef struct
{
    char body[128];
}
_gui_interface_who;

typedef struct
{
    char text[128];
}
_gui_interface_textfield;

typedef struct
{
    char title[128];
    char body_text[4096];
    int line_count;
    char lines[INTERFACE_MAX_LINE][INTERFACE_MAX_CHAR];
}
_gui_interface_message;

typedef struct
{
    char title[128];
    char body_text[4096];
    int line_count;
}
_gui_interface_xtended;

typedef struct
{
    int copper;
    int silver;
    int gold;
    int mithril;
    int line_count;
    char title[128];
    char body_text[4096];
    char lines[INTERFACE_MAX_REWARD_LINE][INTERFACE_MAX_CHAR];
}
_gui_interface_reward;

typedef struct
{
    char mode;
    int num;                // real bmap number of the incon
    char title[128];
    char name[128];          // face (picture name)
    //    item element;
    //    _Sprite *picture;      // the real picture
    char *second_line;
    char body_text[128]; // head title
}
_gui_interface_icon;

typedef struct
{
    char title[64];
    char title2[64];
    char command[128];
}
_gui_interface_button;



/**
 ** This class provides a graphical dialog window.
 *****************************************************************************/
class GuiDialog
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    // which area of the interface used
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

    static GuiDialog &getSingleton()
    {
        static GuiDialog Singleton; return Singleton;
    }

    void reset_gui_interface();
    bool load_gui_interface(int mode, char *data, int len, int pos);
    void gui_interface_send_command(int mode, char *cmd);
    bool get_interface_line(int line, int *element, int *index, char **keyword);
    void show_interface_npc(int mark);
    char *get_parameter_string(char *data, int *pos);

    bool mouseEvent(int MouseAction, int x, int y);
    bool keyEvent(const char keyChar, const unsigned char key);

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
    };

    bool mVisible;
    int mMode;
    int mStatus;
    uint32 mUsed_flag;
    int mIcon_count;
    int mLink_count;
    int mWin_length;
    int mInput_flag;
    int mYoff;
    int mStartx, mStarty;
    int mSelected;
    bool mIcon_select;
    int mLink_selected;
    _gui_interface_head head;
    _gui_interface_link link[MAX_INTERFACE_LINKS];
    _gui_interface_message message;
    _gui_interface_xtended xtended;
    _gui_interface_reward reward;
    _gui_interface_who who;
    _gui_interface_icon icon[MAX_INTERFACE_ICON];
    _gui_interface_button ok;
    _gui_interface_button accept;
    _gui_interface_button decline;
    _gui_interface_textfield textfield;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiDialog();
    ~GuiDialog();
    GuiDialog(const GuiDialog&); // disable copy-constructor.
    int interface_cmd_head  (_gui_interface_head   *head, char *data, int *pos);
    int interface_cmd_link  (_gui_interface_link   *head, char *data, int *pos);
    int interface_cmd_who   (_gui_interface_who    *head, char *data, int *pos);
    int interface_cmd_reward(_gui_interface_reward *head, char *data, int *pos);
    int interface_cmd_icon  (_gui_interface_icon   *head, char *data, int *pos);
    int interface_cmd_button(_gui_interface_button *head, char *data, int *pos);
    int interface_cmd_message(_gui_interface_message *msg, char *data, int *pos);
    int interface_cmd_xtended(_gui_interface_xtended *msg, char *data, int *pos);
    int interface_cmd_textfield(_gui_interface_textfield *textfield, char *data, int *pos);
    void format_gui_interface();
};


#endif
