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

    The author can be reached via e-mail to info@daimonin.org
*/

#ifndef __INTERFACE_H
#define __INTERFACE_H

#define GUI_NPC_WIDTH          296 /* Width of the scrollable window */
#define GUI_NPC_HEIGHT         353 /* Height of the scrollable window */
#define GUI_NPC_LEFTMARGIN     35  /* Width of the left margin */
#define GUI_NPC_TOPMARGIN      79  /* Height of the top margin */
#define GUI_NPC_SCROLL         20  /* Distance to scroll */
#define GUI_NPC_ICONSIZE       36  /* Width/height (icons are always square) */
#define GUI_NPC_ICONSHOP       7   /* Max number of icons per row in a shop */
#define GUI_NPC_BUTTONWIDTH    62  /* Width of button frame */
#define GUI_NPC_BUTTONTEXT     55  /* Width of button */
#define GUI_NPC_TEXTFIELDWIDTH \
    GUI_NPC_WIDTH - (GUI_NPC_BUTTONWIDTH + 4) * 2

#define GUI_NPC_MESSAGE_MAX_LINE 100
#define GUI_NPC_REWARD_MAX_LINE  100
#define GUI_NPC_ICON_MAX_LINE    3
#define GUI_NPC_UPDATE_MAX_LINE  10

typedef enum _gui_npc_type
{
    GUI_NPC_NO,
    GUI_NPC_HEAD,
    GUI_NPC_HYPERTEXT,
    GUI_NPC_MESSAGE,
    GUI_NPC_REWARD,
    GUI_NPC_ICON,
    GUI_NPC_LINK,
    GUI_NPC_UPDATE,
    GUI_NPC_BUTTON,
    GUI_NPC_TEXTFIELD
}
_gui_npc_type;

typedef enum _gui_npc_status
{
    GUI_NPC_STATUS_NORMAL,
    GUI_NPC_STATUS_WAIT
}
_gui_npc_status;

typedef struct gui_internal_image
{
    char    *name;
    int      face;
    _Sprite *sprite;
}
_gui_internal_image;

typedef struct gui_internal_body
{
    char  *text;
    uint8  line_count;
    char  *line[GUI_NPC_MESSAGE_MAX_LINE];
}
_gui_internal_body;

typedef struct _gui_npc_element
{
    struct _gui_npc_element *prev;
    struct _gui_npc_element *next;
    struct _gui_npc_element *last;
    uint8                    type;
    SDL_Rect                 box;
    char                     mode;
    _gui_internal_image      image;
    char                    *keyword;
    char                    *title;
    char                    *title2;
    char                    *command;
    _gui_internal_body       body;
    int                      quantity;
    int                      copper;
    int                      silver;
    int                      gold;
    int                      mithril;
}
_gui_npc_element;

typedef struct _gui_npc
{
    uint16            startx;
    uint16            starty;
    uint16            yoff;
    uint16            height;
    int               status;
    int               mode;
    uint8             sound : 1;
    uint8             shop : 1;
    uint8             input_flag;
    unsigned long     total_coins;
    _gui_npc_element *first_selectable;
    _gui_npc_element *keyword_selected;
    _gui_npc_element *icon_selected;
    _gui_npc_element *link_selected;
    _gui_npc_element *button_selected;
    _gui_npc_element *hypertext;
    _gui_npc_element *head;
    _gui_npc_element *message;
    _gui_npc_element *reward;
    _gui_npc_element *icon;
    _gui_npc_element *link;
    _gui_npc_element *update;
    _gui_npc_element *lhsbutton;
    _gui_npc_element *rhsbutton;
    _gui_npc_element *textfield;
}
_gui_npc;

extern _gui_npc *gui_npc;

extern void gui_npc_reset(void);
extern _gui_npc *gui_npc_create(int mode, char *data, int len, int pos);
extern void gui_npc_show(void);
extern void gui_npc_mousemove(uint16 x, uint16 y);
extern void gui_npc_mouseclick(SDL_Event *e);
extern void gui_npc_keypress(int key);

#endif /* ifndef __INTERFACE_H */
