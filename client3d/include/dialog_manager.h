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

#include "define.h"

const int INTERFACE_WINLEN_NPC = 353;
const int INTERFACE_MAX_LINE = 100;
const int INTERFACE_MAX_CHAR = 256;
const int INTERFACE_MAX_REWARD_LINE = 15;
const int MAX_INTERFACE_ICON = 15;
const int MAX_INTERFACE_LINKS = 25;


typedef struct gui_interface_head
{
    int face;
    char name[128];          /* face (picture name) */
    //    _Sprite *picture;      /* the real picture */
    char body_text[128]; /* head title */
}
_gui_interface_head;

typedef struct gui_interface_link
{
    char link[128];
    char cmd[128];
}
_gui_interface_link;

typedef struct gui_interface_who
{
    char body[128];
}
_gui_interface_who;

typedef struct gui_interface_textfield
{
    char text[128];
}
_gui_interface_textfield;

typedef struct gui_interface_message
{
    char title[128];
    char body_text[4096];
    int line_count;
    char lines[INTERFACE_MAX_LINE][INTERFACE_MAX_CHAR];
}
_gui_interface_message;

typedef struct gui_interface_xtended
{
    char title[128];
    char body_text[4096];
    int line_count;
}
_gui_interface_xtended;

typedef struct gui_interface_reward
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

typedef struct gui_interface_icon
{
    char mode;
    int num;                /* real bmap number of the incon */
    char title[128];
    char name[128];          /* face (picture name) */
    //    item element;
    //    _Sprite *picture;      /* the real picture */
    char *second_line;
    char body_text[128]; /* head title */
}
_gui_interface_icon;

typedef struct gui_interface_button
{
    char title[64];
    char title2[64];
    char command[128];
}
_gui_interface_button;


typedef struct gui_interface_struct
{
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
}
_gui_interface_struct;



class DialogInterface
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
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
        GUI_INTERFACE_STATUS_NORMAL = 0,
        GUI_INTERFACE_STATUS_WAIT   = 1
    };

    static DialogInterface &getSingleton()
    {
        static DialogInterface Singleton; return Singleton;
    }

    void reset_gui_interface();
    void load_gui_interface(int mode, char *data, int len, int pos);
    void gui_interface_send_command(int mode, char *cmd);
    int get_interface_line(int *element, int *index, char **keyword, int x, int y, int mx, int my);
    int precalc_interface_npc(void);
    void show_interface_npc(int mark);
    //void gui_interface_mouse(SDL_Event *e);

private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
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

    int mode;
    int status;
    unsigned int used_flag;
    int icon_count;
    int link_count;
    int win_length;
    int input_flag;
    int yoff;
    int startx;
    int starty;
    int icon_select;
    int selected;
    int link_selected;

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    DialogInterface()
    {}
    ~DialogInterface();
    DialogInterface(const DialogInterface&); // disable copy-constructor.
    int interface_cmd_head  (_gui_interface_head   *head, char *data, int *pos);
    int interface_cmd_link  (_gui_interface_link   *head, char *data, int *pos);
    int interface_cmd_who   (_gui_interface_who     *head, char *data, int *pos);
    int interface_cmd_reward(_gui_interface_reward  *head, char *data, int *pos);
    int interface_cmd_message(_gui_interface_message *msg, char *data, int *pos);
    int interface_cmd_xtended(_gui_interface_xtended *msg, char *data, int *pos);
    int interface_cmd_icon(_gui_interface_icon *head, char *data, int *pos);
    int interface_cmd_button(_gui_interface_button *head, char *data, int *pos);
    int interface_cmd_textfield(_gui_interface_textfield *textfield, char *data, int *pos);
    void format_gui_interface(_gui_interface_struct *gui_int);

};


#endif
