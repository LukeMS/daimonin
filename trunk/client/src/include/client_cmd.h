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

#ifndef __CLIENT_H
#define __CLIENT_H

#define MAP_UPDATE_CMD_SAME 0
#define MAP_UPDATE_CMD_NEW 1
#define MAP_UPDATE_CMD_CONNECTED 2

#define STRINGCOMMAND 0
#define MAX_BUF 256
#define BIG_BUF 1024

/* How many skill types server supports/client will get sent to it.
 * If more skills are added to server, this needs to get increased.
 */
#define MAX_SKILL   6

/* defines for the ext map command */
#define FFLAG_SLEEP     0x01        /* object sleeps */
#define FFLAG_CONFUSED  0x02        /* object is confused */
#define FFLAG_PARALYZED 0x04        /* object is paralyzed */
#define FFLAG_SCARED    0x08        /* object is scared    */
#define FFLAG_EATING    0x10        /* object is eating */
#define FFLAG_INVISIBLE 0x20        /* object is invisible (but when send, player can see it) */
#define FFLAG_ETHEREAL  0x40        /* object is etheral - but when send, object can be seen */
#define FFLAG_PROBE     0x80        /* object is target of player */

#define INPUT_MODE_NO      0
#define INPUT_MODE_CONSOLE 1
#define INPUT_MODE_NUMBER  4
#define INPUT_MODE_GETKEY  8
#define INPUT_MODE_NPCDIALOG  16

#define NUM_MODE_GET  1
#define NUM_MODE_DROP 2

typedef struct _server_char
{
    struct _server_char    *next;
    struct _server_char    *prev;
    char                   *name; /* race name: human, elf */
    char                   *desc[4]; /* 4 description strings */
    int                     bar[3];
    int                     bar_add[3];
    int                     gender[4]; /* male, female, neutrum, herm. */
    int                     gender_selected;
    int                     skill_selected;
    char                   *char_arch[4]; /* 4 description strings */
    sint32                  face[4];
    int                     stats[7];
}
_server_char;

typedef struct _screensize
{
    int x;
    int y;
    int xoff;
    int yoff;
}
_screensize;

extern _screensize Screensize;

extern _screensize Screendefs[16];

extern char         *server;
extern char         *client_libdir;
extern char         *face_file;

typedef enum rangetype
{
    range_bottom        = -1,
    range_none          = 0,
    range_bow           = 1,
    range_magic         = 2,
    range_wand          = 3,
    range_rod           = 4,
    range_scroll        = 5,
    range_horn          = 6,
    range_steal         = 7,
    range_size          = 8
}    rangetype;

/* These are used with CS_STAT_FLAGS above to communicate S->C what the
 * server thinks the fireon & runon states are.
 */
#define SF_FIREON           1
#define SF_RUNON            2
#define SF_BLIND            4
#define SF_XRAYS            8
#define SF_INFRAVISION      16

/* Flags for the item command */
enum
{
    a_none,
    a_readied,
    a_wielded,
    a_worn,
    a_active,
    a_applied
};
#define F_APPLIED       0x000F
#define F_LOCATION      0x00F0
#define F_UNPAID        0x0200
#define F_MAGIC         0x0400
#define F_CURSED        0x0800
#define F_DAMNED        0x1000
#define F_OPEN          0x2000
#define F_NOPICK        0x4000
#define F_LOCKED        0x8000
#define F_TRAPED        0x10000

#define SOUND_NORMAL    0
#define SOUND_SPELL 1

/* Contains the base information we use to make up a packet we want to send. */
typedef struct SockList
{
    int             cmd; /* the binary command tag we want send */
    int             flags; /* the flags for send function */ 
    int             len; /**< How much data in buf */
    int             pos; /**< Start of data in buf */
    char            *buf;
    char            defbuf[MAX_DATA_TAIL_LENGTH];
}
SockList;

extern void client_cmd_ping(uint32 ping);
extern void client_cmd_setup(void);
extern void client_cmd_requestfile(uint8 index);
extern void client_cmd_checkname(char *name);
extern void client_cmd_login(int mode, char *name, char *pass);
extern void client_cmd_newchar(struct _server_char *nc);
extern void client_cmd_delchar(char *name);
extern void client_cmd_addme(char *name);
extern void client_cmd_face(uint16 num);
extern void client_cmd_move(uint8 dir, uint8 mode);
extern void client_cmd_apply(int tag);
extern void client_cmd_examine(int tag);
extern void client_cmd_invmove(int loc, int tag, int nrof);
extern void client_cmd_guitalk(sint8 mode, char *topic);
extern void client_cmd_lock(int mode, int tag);
extern void client_cmd_mark(int tag);
extern void client_cmd_fire(int num, int mode, char *name);
extern void client_cmd_generic(const char *command);

#endif /* ifndef __CLIENT_H */
