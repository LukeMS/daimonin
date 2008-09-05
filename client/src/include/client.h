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
#include "include.h"

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

/* ClientSocket could probably hold more of the global values - it could
* probably hold most all socket/communication related values instead
* of globals.
*/
/* with the binary client protocol, ClientSocket only hold ATM the socket identifier.
* we can safely add here more features like latency counters, pings or statistics.
* MT/2008
*/
typedef struct ClientSocket
{
    SOCKET  fd;
}
ClientSocket;

extern ClientSocket csocket;

extern char         *server;
extern char         *client_libdir;
extern char         *image_file;

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

#define NDI_SAY     0x0100  /* its a say command */
#define NDI_SHOUT   0x0200
#define NDI_TELL    0x0400
#define NDI_GSAY    0x0800
#define NDI_EMOTE   0x01000
#define NDI_GM  0x02000 /* Its from a staff member */
#define NDI_PLAYER  0x04000 /* this comes from a player */
#define NDI_VIM     0x08000 /* VIM-Message */


#define NDI_UNIQUE  0x10000 /* Print this out immediately, don't buffer */
#define NDI_ALL     0x20000 /* Inform all players of this message */

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

#define COLOR_DEFAULT 0 /* white */
#define COLOR_WHITE  0
#define COLOR_ORANGE 1
#define COLOR_LBLUE  2 /* navy... */
#define COLOR_RED   3
#define COLOR_GREEN 4
#define COLOR_BLUE  5
#define COLOR_GREY  6
#define COLOR_YELLOW  7
#define COLOR_DK_NAVY  8

#define COLOR_HGOLD 64 /* client only colors */
#define COLOR_DGOLD 65
#define COLOR_DBROWN  44

#define COLOR_TURQUOISE 210

#define COLOR_BLACK 255

/* Contains the base information we use to make up a packet we want to send. */
typedef struct SockList
{
    int             cmd; /* the binary command tag we want send */
    int             flags; /* the flags for send function */ 
    int             len; /**< How much data in buf */
    int             pos; /**< Start of data in buf */
    unsigned char  *buf;
    unsigned char  defbuf[MAX_DATA_TAIL_LENGTH];
}
SockList;

/* help define for a clean socklist init */
#define         SockList_INIT(_sl_, _buf_) {memset( (_sl_), 0, sizeof(SockList) );(_sl_)->buf=(_buf_);}
#define         SockList_COMMAND(__sl, __cmd, __flags) {(__sl)->cmd=(__cmd);(__sl)->flags=(__flags);}
#define         SockList_AddChar(__sl, __c) (__sl)->buf?*((__sl)->buf+(__sl)->len++):(((__sl)->defbuf[(__sl)->len++])= (unsigned char)(__c))

extern void     send_game_command(const char *command);

extern void     finish_face_cmd(int pnum, uint32 checksum, char *face);
extern int      request_face(int num, int mode);

extern void     SendSetupCmd(void);
extern void     RequestFile(ClientSocket csock, int index);
extern void     SendAddMe(void);
extern void     send_reply(char *text);
extern void     send_new_char(struct _server_char *nc);
extern void     client_send_apply(int tag);
extern void     send_move_command(int dir, int mode);
extern void     client_send_examine(int tag);
extern void     send_inv_move(int loc, int tag, int nrof);
extern void     client_send_tell_extended(char* body, char *tail);
extern void     send_lock_command(int mode, int tag);
extern void     send_mark_command(int tag);
extern void     send_fire_command(int num, int mode, char *tmp_name);
