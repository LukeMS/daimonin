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

#define ROUND_TAG global_round_tag /* put this here because the DIFF */
extern uint32 global_round_tag; /* global round ticker ! this is real a global */

#define NUM_LOOK_OBJECTS 15

#ifdef WIN32
#pragma pack(push,1)
#endif

struct MapCell {
    int		count;
    short	faces[MAP_LAYERS];
    uint8	fflag[MAP_LAYERS];  
    uint8	ff_probe[MAP_LAYERS];  
    char	quick_pos[MAP_LAYERS];
}MapCell;

struct Map {
  struct MapCell cells[MAP_CLIENT_X][MAP_CLIENT_Y];
};

/* True max is 16383 given current map compaction method */
#define MAXFACENUM  5000
#define MAXANIMNUM  2000

struct statsinfo {
    char *range, *title, *ext_title;
};


/* This contains basic information on the socket structure.  status is its
 * current state.  we set up our on buffers for sending/receiving, so we can
 * handle some higher level functions.  fd is the actual file descriptor we
 * are using.
 */

enum Sock_Status {Ns_Avail, Ns_Add, Ns_Dead};


/* The following is the setup for a ring buffer for storing outbut
 * data that the OS can't handle right away.
 */

typedef struct Buffer {
    int	    start;
    int	    len;
    char    data[SOCKETBUFSIZE];
} Buffer;


#ifdef WIN32
#pragma pack(pop)
#endif

typedef struct NewSocket {
    int fd;
    int   mapx, mapy;	    /* How large a map the client wants */
    int   mapx_2, mapy_2;	/* same like above but /2 */
    char    *host;	    /* Which host it is connected from (ip address)*/
    uint32  cs_version, sc_version; /* versions of the client */
	uint32 update_tile;		/* marker to see we must update the below windows of the tile the player is */
    sint16 look_position;  /* start of drawing of look window */
    uint8   faceset;	    /* Set the client is using, default 0 */
    enum Sock_Status status;
    struct Map lastmap;
    struct statsinfo stats;
    /* If we get an incomplete packet, this is used to hold the data. */
    SockList	inbuf;
    Buffer  outputbuffer;   /* For undeliverable data */
    uint32  facecache:1;    /* If true, client is caching images */
    uint32  sent_scroll:1;
    uint32  sound:1;	    /* does the client want sound */
    uint32  map2cmd:1;	    /* Always use map2 protocol command */
	uint32  ext_title_flag:1; /* send ext title to client */
    uint32  darkness:1;	    /* True if client wants darkness information */
    uint32  image2:1;	    /* Client wants image2/face2 commands */
    uint32  can_write:1;    /* Can we write to this socket? */
	/* these blocks are simple flags to ensure
	 * that the client don't hammer startup commands
	 * again & again to abuse the server.
	 */
	uint32 version:1;		
	uint32 setup:1;		
	uint32 rf_settings:1;
	uint32 rf_skills:1;
	uint32 rf_spells:1;
	uint32 rf_anims:1;
	uint32 rf_bmaps:1;

    /* Below here is information only relevant for old sockets */
    /*char    *comment;*/	    /* name or listen comment */
} NewSocket;



#define FACE_TYPES  1

#define PNG_FACE_INDEX	0

typedef struct Socket_Info {
    struct timeval timeout;	/* Timeout for select */
    int	    max_filedescriptor;	/* max filedescriptor on the system */
    int	    nconns;		/* Number of connections */
    int	    allocated_sockets;	/* number of allocated in init_sockets */
} Socket_Info;

extern Socket_Info socket_info;

#endif
