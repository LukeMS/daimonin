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
#if !defined(__EVENT_H)
#define __EVENT_H

#define MAX_KEYS 512
#define MAX_KEYMAP 512

typedef struct _button_status
{
    int mx,my;   /* current mouse postion */
    int mx_down,my_down; /* position as button was pressed. -1,-1 means button is not pressed */
    int mx_up,my_up; /* if NOT -1,-1, we have a button up */
    int down;   /* if != -1, we have a button down event */
    int click;   /* if != -1, we have a down/up button event */
    int valid;   /* if != -1 = if this is 0, no event manager has used this data for a click, >0 means used */
}
button_status;

typedef struct _key_macro
{
    char    macro[64]; /* the macro*/
    char    cmd[64]; /* our cmd string*/
    int     internal;/*intern: Use this function to generate the cmd*/
    int     value;/* a default value for cmds*/
    int     mode;/* the default send mode*/
    int     menu_mode;
}
_key_macro;

enum
{
    KEYBIND_STATUS_NO,
    KEYBIND_STATUS_EDIT,
    KEYBIND_STATUS_EDITKEY
};

enum
{
    KEYFUNC_NO,
    KEYFUNC_RUN,
    KEYFUNC_MOVE,
    KEYFUNC_CONSOLE,
    KEYFUNC_CURSOR,
    KEYFUNC_RANGE,
    KEYFUNC_RANGE_BACK,
    KEYFUNC_RANGE_SELECT,
    KEYFUNC_APPLY,
    KEYFUNC_DROP,
    KEYFUNC_GET,
    KEYFUNC_LOCK,
    KEYFUNC_MARK,
    KEYFUNC_EXAMINE,
    KEYFUNC_PAGEUP,
    KEYFUNC_PAGEDOWN,
    KEYFUNC_PAGEUP_TOP,
    KEYFUNC_PAGEDOWN_TOP,
    KEYFUNC_STATUS,
    KEYFUNC_HELP,
    KEYFUNC_OPTION,
    KEYFUNC_SPELL,
    KEYFUNC_KEYBIND,
    KEYFUNC_SKILL,
    KEYFUNC_LAYER0,
    KEYFUNC_LAYER1,
    KEYFUNC_LAYER2,
    KEYFUNC_LAYER3,
    KEYFUNC_TARGET_ENEMY,
    KEYFUNC_TARGET_FRIEND,
    KEYFUNC_TARGET_SELF,
    KEYFUNC_COMBAT,
    KEYFUNC_SCREENTOGGLE,
};

typedef struct _keybind_key
{
    char    macro[256];            /* the text */
    char    keyname[256];            /* the text */
    int     entry;                  /* -1: new macro - 0-xx edit entry */
    int     key;
    int     repeat_flag;
}
_keybind_key;

enum
{
    DRAG_GET_STATUS             = -1,
    DRAG_NONE,
    DRAG_IWIN_BELOW,
    DRAG_IWIN_INV,
    DRAG_QUICKSLOT,
    DRAG_QUICKSLOT_SPELL,
    DRAG_PDOLL
};

/* use these constants to determine the state of mouse and its events */
enum {
	IDLE=1, /* don't change this to 0 (enum default is 0, so leave this as is) */
	LB_DN,
	LB_UP,
	RB_DN,
	RB_UP,
	MB_UP,
	MB_DN,
};

extern uint32 MouseState; /* state of the buttons */
extern int MouseEvent; /* whether there is an event (removed at end of main loop) */

extern int old_mouse_y;

extern int itemExamined;

extern button_status global_buttons;

extern int          KeyScanFlag; /* for debug/alpha , remove later */
extern int          cursor_type;
extern _key_macro   defkey_macro[];
extern const int    DEFAULT_KEYMAP_MACROS;
extern int  draggingInvItem(int src);
extern int  Event_PollInputDevice(void);
extern void init_keys(void);
extern void reset_keys(void);
extern void read_keybind_file(char *fname);
extern void save_keybind_file(char *fname);
extern void check_menu_keys(int menu, int value);
extern void quickslot_key(SDL_KeyboardEvent *key, int slot);
extern Boolean  check_menu_macros(char *text);

Boolean     process_macro_keys(int id, int value);
extern void     check_keys(int key);
#endif

