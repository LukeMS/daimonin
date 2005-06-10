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

#if !defined(__DIALOG_H)
#define __DIALOG_H

#define OPTWIN_MAX_TAB 20
#define OPTWIN_MAX_OPT 26
#define OPTWIN_MAX_KEYS 100

typedef struct _option
{
    char           *name;
    char           *info1;  /* info text row 1 */
    char           *info2;  /* info text row 2 */
    char           *val_text; /* text-replacement for number values */
    int             sel_type;
    int             minRange, maxRange, deltaRange;
    int             default_val;
    void           *value;
    int             value_type;
}_option;
extern _option  opt[];

extern enum
{
    VAL_BOOL,
    VAL_TEXT,
    VAL_CHAR,
    VAL_INT,
    VAL_U32
}               value_type;
extern char    *opt_tab[];
extern int      dialog_new_char_warn;
extern int      active_button;
extern int dialog_login_warning_level;
extern char        *spell_tab[];


#define DIALOG_LOGIN_WARNING_NONE       0
#define DIALOG_LOGIN_WARNING_WRONGNAME  1
#define DIALOG_LOGIN_WARNING_WRONGPASS  2
#define DIALOG_LOGIN_WARNING_VERIFY_FAILED  3

int add_button(int x, int y, int xo, int yo, int id, int gfxNr, char *text, char *text_h);
extern void show_optwin(void);
extern void show_newplayer_server(void);
extern void show_login_server(void);
extern void show_meta_server(_server *node, int metaserver_start, int metaserver_sel);
extern void accept_char();
extern int get_interface_line(int *element, int *index, char **keyword, int x, int y, int mx, int my);
extern int precalc_interface_npc(void);
extern void show_interface_npc(int mark);
extern void gui_interface_mouse(SDL_Event *e);

#endif

