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
#if !defined(__MENU_H)
#define __MENU_H

#define MENU_NO       0x01
#define MENU_KEYBIND  0x02
#define MENU_STATUS   0x03
#define MENU_SPELL    0x08
#define MENU_SKILL    0x10
#define MENU_OPTION   0x20
#define MENU_CREATE   0x40
/* in future, we need a different system - with up to 2 open
 * interfaces... menus like keybind & stuff should be a layer above that. MT-2005
 */
#define MENU_NPC  0x80
#define MENU_BOOK    0x100
#define MENU_QUEST   0x200

#define MENU_ALL (MENU_NO & MENU_KEYBIND & MENU_SPELL & MENU_STATUS & MENU_OPTION & MENU_NPC & MENU_BOOK & MENU_QUEST)

#define MENU_SOUND_VOL 40
struct _skill_list      skill_list[SKILL_LIST_MAX];
extern _dialog_list_set skill_list_set;

struct _spell_list      spell_list[SPELL_LIST_MAX]; /* skill list entries */
extern _dialog_list_set spell_list_set;

extern _dialog_list_set option_list_set;

struct _bindkey_list    bindkey_list[BINDKEY_LIST_MAX];
extern _dialog_list_set bindkey_list_set;

extern _dialog_list_set create_list_set;
extern int              keybind_status;

#define MAX_QUICK_SLOTS 8
typedef union _quickslot
{
    struct
    {
        Boolean         is_spell; /* do we have an item or a spell in quickslot */
        int             tag;
    }
    shared;
    struct
    {
        Boolean         is_spell;
        int             tag;     /* what item/spellNr in quickslot */
        int             invSlot;
        int             nr;
    }
    item;
    struct
    {
        Boolean         is_spell;
        int             tag;
        int             spellNr;     /* */
        int             groupNr; /* spellgroup */
        int             classNr; /* spellclass */
    }
    spell;
    struct
    {
        Boolean         is_spell;
        int             tag;
        char           *name;
    }
    name;
}
_quickslot;
extern _quickslot   quick_slots[MAX_QUICK_SLOTS];

typedef struct _media_file
{
    char    name[256];      /* file name */
    void   *data;           /* data buffer */
    int     type;           /* what is this? (what loaded in buffer) */
    int     p1;             /* parameter 1 */
    int     p2;
}
_media_file;


typedef enum _media_type
{
    MEDIA_TYPE_NO,
    MEDIA_TYPE_PNG
}                   _media_type;

#define MEDIA_MAX 10
#define MEDIA_SHOW_NO -1

extern _media_file  media_file[MEDIA_MAX];

extern int          media_count;    /* buffered media files */
extern int          media_show;
extern int          media_show_update ;

extern void     do_console(int x, int y);
extern void     do_number(int x, int y);
extern void     widget_show_number(int x, int y);
extern void     widget_number_event(int x, int y, SDL_Event event);
extern void     widget_show_console(int x, int y);
extern void     widget_show_resist(int x, int y);
extern void     show_keybind(void);
extern void     show_status(void);
extern void     show_spelllist(void);
extern void     show_skilllist(void);

extern void     widget_show_mapname(int x, int y);
extern void     show_menu(void);
extern void     show_media(int x, int y);
extern void     widget_show_range(int x, int y);
extern void     widget_range_event(int x, int y, SDL_Event event, int MEvent);
extern int      init_media_tag(char *tag);
extern void     blt_inventory_face_from_tag(int tag, int x, int y);
extern int      blt_window_slider(_Sprite *slider, int max_win, int winlen, int off, int len, int x, int y);
extern void     do_keybind_input(void);
extern void     do_npcdialog_input(void);


extern int      get_facenum_from_name(char * name);
extern int      read_anim_tmp(void);
extern int      read_bmap_tmp(void);
extern void     read_anims(void);
extern void     read_bmaps_p0(void);
extern void     delete_bmap_tmp(void);
extern void     read_bmaps(void);
extern void     delete_server_chars(void);
extern void     load_settings(void);
extern void     read_settings(void);
extern void     read_spells(void);
extern void     read_skills(void);
extern Boolean  blt_face_centered(int face, int x, int y);
extern int      get_quickslot(int x, int y);
extern void     show_quickslots(int x, int y);
extern void     widget_quickslots(int x, int y);
extern void     widget_quickslots_mouse_event(int x, int y, int MEvent);
extern void     update_quickslots(int del_item);
extern void     load_quickslots_entrys();
extern void     save_quickslots_entrys();

extern int      client_command_check(char *cmd);
extern void     widget_event_target(int x, int y, SDL_Event event);
extern void     widget_show_target(int x, int y);
extern void     reload_icons(void);

extern int get_bmap_id(char *name);
extern void reset_menu_status(void);

#endif
