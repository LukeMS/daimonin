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

    The author can be reached via e-mail to daimonin@nord-com.net
*/
#if !defined(__EVENT_H)
#define __EVENT_H

#define MAX_KEYS 512
#define MAX_KEYMAP 512

typedef struct _keymap
{
    char text[256];/*the command text, submited to server when key pressed*/
    char keyname[256];
    int key;/*scancode of key*/
    int repeatflag;/*if true, key will be repeated when pressed*/
    int mode;/*the send mode OR the menu id*/
    int menu_mode;
}_keymap;
enum {
    KEYBIND_STATUS_NO,
        KEYBIND_STATUS_NEW,
        KEYBIND_STATUS_EDIT,
        KEYBIND_STATUS_NEWKEY,
        KEYBIND_STATUS_EDITKEY
};

typedef struct _keybind_key {
    char macro[256];            /* the text */
    char keyname[256];            /* the text */
    int entry;                  /* -1: new macro - 0-xx edit entry */
    int key;
    int repeat_flag;
}_keybind_key;

extern _keymap keymap[MAX_KEYMAP]; /* thats the one and only key bind table*/

extern int KeyScanFlag; /* for debug/alpha , remove later */
extern int keymap_count;	/* how much keys we have in the the keymap...*/

extern int Event_PollInputDevice(void);
extern void init_keys(void);
extern void reset_keys(void);
extern void read_keybind_file(char *fname);
extern void add_keybind_macro(struct _keybind_key *keybind_key);
extern void save_keybind_file(char *fname);

#endif
