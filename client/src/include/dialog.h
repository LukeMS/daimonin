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
    char           *info3;  /* info text row 3 */
    char           *val_text; /* text-replacement for number values */
    int             sel_type;
    int             minRange, maxRange, deltaRange;
    int             default_val;
    void           *value;
    int             value_type;
}
_option;
extern _option  opt[];

extern enum
    {
        VAL_BOOL,
        VAL_TEXT,
        VAL_CHAR,
        VAL_INT,
        VAL_U32
    }
    value_type;
extern char    *opt_tab[];
extern int      dialog_new_char_warn;
extern int dialog_login_warning_level;
extern char        *spell_tab[];

enum
{
    DIALOG_LOGIN_WARNING_NONE,
    DIALOG_LOGIN_WARNING_NAME_NO,
    DIALOG_LOGIN_WARNING_NAME_BLOCKED,
    DIALOG_LOGIN_WARNING_NAME_PLAYING,
    DIALOG_LOGIN_WARNING_NAME_TAKEN,
    DIALOG_LOGIN_WARNING_NAME_BANNED,
    DIALOG_LOGIN_WARNING_NAME_WRONG,
    DIALOG_LOGIN_WARNING_PWD_WRONG,
    DIALOG_LOGIN_WARNING_PWD_SHORT,
    DIALOG_LOGIN_WARNING_PWD_NAME,
    DIALOG_LOGIN_WARNING_ACCOUNT_UNKNOWN
};

void add_close_button(int x, int y, int menu, Boolean newstyle);
int add_button(int x, int y, int id, int gfxNr, char *text, char *text_h);
int add_rangebox(int x, int y, int id, int text_w, int text_x, char *text, int color);
extern void show_optwin(void);
extern void show_newplayer_server(void);
extern void show_login_server(void);
extern void show_meta_server(_server *node, int metaserver_start, int metaserver_sel);
extern void show_account(void);
#endif

