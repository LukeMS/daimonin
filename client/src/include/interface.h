/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to info@daimonin.net
*/

#if !defined(__INTERFACE_H)
#define __INTERFACE_H

#define INTERFACE_WINLEN_NPC 353

#define INTERFACE_MAX_LINE 50
#define INTERFACE_MAX_CHAR 128

#define INTERFACE_MAX_REWARD_LINE 15

typedef struct gui_interface_head
{
    int face;
    char name[128];          /* face (picture name) */
    _Sprite *picture;      /* the real picture */
    char body_text[128]; /* head title */
}_gui_interface_head;

typedef struct gui_interface_link
{
    char link[128];
    char cmd[128];
}_gui_interface_link;

typedef struct gui_interface_message
{
    char title[128];
    char body_text[4096];
    int line_count;
    char lines[INTERFACE_MAX_LINE][INTERFACE_MAX_CHAR];
}_gui_interface_message;

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
}_gui_interface_reward;

typedef struct gui_interface_icon
{
    char mode;
    int num;                /* real bmap number of the incon */
    char title[128];
    char name[128];          /* face (picture name) */
    item element;
    _Sprite *picture;      /* the real picture */
    char body_text[128]; /* head title */
}_gui_interface_icon;

typedef struct gui_interface_button
{
    char title[64];
    char title2[64];
    char command[128];
}_gui_interface_button;

#define MAX_INTERFACE_ICON 10
#define MAX_INTERFACE_LINKS 10

/* which area of the static gui_interface struct is used* */
#define GUI_INTERFACE_HEAD 0x1
#define GUI_INTERFACE_MESSAGE 0x2
#define GUI_INTERFACE_REWARD 0x4

#define GUI_INTERFACE_ACCEPT    0x8
#define GUI_INTERFACE_DECLINE    0x20

/* don't use, internal */
#define GUI_INTERFACE_ICON    0x40
#define GUI_INTERFACE_LINK    0x80

#define GUI_INTERFACE_STATUS_NORMAL 0
#define GUI_INTERFACE_STATUS_WAIT 1

typedef struct gui_interface_struct {
    int mode;
    int status;
    uint32 used_flag;
    int icon_count;
    int link_count;
    int win_length;
    int input_flag;
    int yoff;
    int startx;
    int starty;
    int icon_select;
    int selected;
    _gui_interface_head head;
    _gui_interface_link link[MAX_INTERFACE_ICON];
    _gui_interface_message message;
    _gui_interface_reward reward;
    _gui_interface_icon icon[MAX_INTERFACE_ICON];
    _gui_interface_button ok;
    _gui_interface_button accept;
    _gui_interface_button decline;
} _gui_interface_struct;

extern void reset_gui_interface(void);
extern _gui_interface_struct *load_gui_interface(int mode, char *data, int len, int pos);
extern void gui_interface_send_command(int mode, char *cmd);

#endif
