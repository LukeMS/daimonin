/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

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
/*
    newserver.h defines various structures and values that are use for the
    new client server communication method.  Values defined here are only
    used on the server side code.  For shared client/server values, see
    newclient.h
*/

#ifndef NEWSERVER_H
#define NEWSERVER_H

#define NUM_LOOK_OBJECTS 15

/* True max is 16383 given current map compaction method */
#define MAXFACENUM  5000
#define MAXANIMNUM  2000

#define FACE_TYPES  1
#define PNG_FACE_INDEX  0

/* This contains basic information on the socket structure.  status is its
 * current state.  we set up our on buffers for sending/receiving, so we can
 * handle some higher level functions.  fd is the actual file descriptor we
 * are using.
 */
enum Sock_Status
{
    Ns_Avail,
    Ns_Wait,
    Ns_Add,
    Ns_Login,
    Ns_Dead
};

/* The following is the setup for a ring buffer for storing outbut
 * data that the OS can't handle right away.
 */

typedef struct Buffer_struct
{
    int     start;
    int     len;
    char    data[MAXSOCKBUF];
} Buffer;

/* Contains the base information we use to make up a packet we want to send. */
typedef struct SockList_struct
{
    int             len;
    unsigned char  *buf;
} SockList;

typedef struct NewSocket_struct
{
    int                 fd;
    struct player      *pl;             /* if != NULL this socket is part of a player struct */
    struct Map          lastmap;        /* Thats the VISIBLE map area of the player, used to send to client */
    uint32              ip;
    uint32              login_count;        /* if someone is too long idle in the login, we kick him here! */
    int                 mapx, mapy;         /* How large a map the client wants */
    int                 mapx_2, mapy_2;     /* same like above but /2 */
    char               *host;               /* Which host it is connected from (ip address)*/
    uint32              cs_version;         /*client/server versions */
    uint32              sc_version;
    uint32              update_tile;        /* marker to see we must update the below windows of the tile the player is */
    enum Sock_Status    status;
    SockList            inbuf;          /* This holds *one* command we try to handle */
    SockList            readbuf;        /* Raw data read in from the socket  */
    SockList            cmdbuf;         /* buffer for the *real* player/char commands */

    Buffer              outputbuffer;       /* For undeliverable data */
    uint32              idle_flag       : 1;        /* idle warning was given and we count for disconnect */
    uint32              addme           : 1;        /* important: when set, a "connect" was initizialised as "player" */
    uint32              facecache       : 1;        /* If true, client is caching images */
    uint32              sent_scroll     : 1;
    uint32              sound           : 1;        /* does the client want sound */
    uint32              map2cmd         : 1;        /* Always use map2 protocol command */
    uint32              ext_title_flag  : 1;        /* send ext title to client */
    uint32              darkness        : 1;        /* True if client wants darkness information */
    uint32              image2          : 1;        /* Client wants image2/face2 commands */
    uint32              can_write       : 1;        /* Can we write to this socket? */
    uint32              version         : 1;
    uint32              write_overflow  : 1;
    uint32              setup           : 1;
    uint32              rf_settings     : 1;
    uint32              rf_skills       : 1;
    uint32              rf_spells       : 1;
    uint32              rf_anims        : 1;
    uint32              rf_bmaps        : 1;

    sint16              look_position;  /* start of drawing of look window */
    sint16              look_position_container;  /* start of drawing of look window for a container */
    uint8               faceset;        /* Set the client is using, default 0 */
} NewSocket;


typedef struct Socket_Info_struct
{
    struct timeval  timeout;    /* Timeout for select */
    int             max_filedescriptor; /* max filedescriptor on the system */
    int             nconns;     /* Number of connections */
    int             allocated_sockets;  /* number of allocated in init_sockets */
} Socket_Info;

#endif
