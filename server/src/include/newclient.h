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

    The author can be reached via e-mail to info@daimonin.org
*/
/* This file defines various flags that both the new client and
 * newserver uses.  These should never be changed, only expanded.
 * Changing them will likely cause all old clients to not work properly.
 * While called newclient, it is really used by both the client and
 * server to keep some values the same.
 *
 * Name format is CS_(command)_(flag)
 * CS = Client/Server.
 * (command) is protocol command, ie ITEM
 * (flag) is the flag name
 */

/* It is trivial to keep a link of copy of this file in the client
 * or server area.  But keeping one common file should make things
 * more reliable, as both the client and server will definately be
 * talking about the same values.
 */


#ifndef __NEWCLIENT_H
#define __NEWCLIENT_H

/* max. socket input buffer we can read/fill when reading from socket.
 * This is raw data until we sort it out and put it in the player command queue.
 */
#define MAXSOCKBUF_IN (3*1024)

/* used for the OS socket bufs */
#define SOCKET_BUFSIZE_SEND (24*1024)
#define SOCKET_BUFSIZE_READ (8*1024)

/* These are multiplication values that should be used when changing
 * floats to ints, and vice version.  MULTI is integer representatin
 * (float to int), MULTF is float, for going from int to float.
 */
#define FLOAT_MULTI 100000
#define FLOAT_MULTF 100000.0

/* These are used with CS_STAT_FLAGS above to communicate S->C what the
 * server thinks the fireon & runon states are.
 */
#define SF_RUNON            2
#define SF_BLIND            4
#define SF_XRAYS            8
#define SF_INFRAVISION      16

/* The following are the color flags passed to ndi.
 *
 * We also set up some control flags
 *
 * NDI = New Draw Info
 */

/* default color 0 = WHITE - if no color is selected */
#define NDI_WHITE   0
#define NDI_ORANGE  1
#define NDI_NAVY    2
#define NDI_RED     3
#define NDI_GREEN   4
#define NDI_BLUE    5
#define NDI_GREY    6
#define NDI_BROWN   7       /* open door, close/open containers... */
#define NDI_PURPLE  8
#define NDI_FLESH  9
#define NDI_YELLOW  10
#define NDI_DK_NAVY  11

#define NDI_MAX_COLOR   11  /* Last value in */
#define NDI_COLOR_MASK  0xff   /* colors are first 8 bit - o bit digit */
#define NDI_FLAG_MASK   0xffff  /* 2nd 8 bit are flags to define draw_info string */

/* implicit rule: if not NDI_PLAYER or NDI_SYSTEM is defined,
 * message comes from NPC.
 */
#define NDI_SAY     0x0100  /* its a say command */
#define NDI_SHOUT   0x0200
#define NDI_TELL    0x0400
#define NDI_GSAY    0x0800
#define NDI_EMOTE   0x01000
#define NDI_GM      0x02000 /* Its from a staff member */
#define NDI_PLAYER  0x04000 /* this comes from a player */
#define NDI_VIM     0x08000 /* VIM-Message */


#define NDI_UNIQUE  0x10000 /* Print this out immediately, don't buffer */
#define NDI_ALL     0x20000 /* Inform all players of this message */

#define CF_FACE_NONE    0
#define CF_FACE_PNG 3
#define CF_FACE_CACHE   0x10

typedef struct _srv_client_files
{
	sockbuf_struct *sockbuf;    /* static sockbuf with file data */
	int             len;        /* if -1, the file is not compressed */
	int             len_ucomp;  /* original uncompressed file length */
	unsigned int    crc;        /* crc adler32 */
} _srv_client_files;

/* helper functions to grap paramter from binary commands.
 * WARNING: This macros increase _data_ directly. Be sure you
 * only use them on real and local buffer pointers.
 * They also will fail in if(..) GetShort_Buffer().
 * There must be explicit set {}!
 */
#define GetChar_Buffer(_data_)			*((uint8*) (_data_));(_data_)++ 
#define GetShort_Buffer(_data_)			*((uint16*) (_data_));(_data_)+=2
#define GetInt_Buffer(_data_)			*((uint32*) (_data_));(_data_)+=4

/* thats a bit hard coded but well... */
#define AddIf_SOCKBUF_PTR _sockbufptr

/* Sends the stats to the client - only sends them if they have changed */

#define AddIfInt(Old,New,Type) if (Old != New) {\
	Old = New; \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (Type)); \
	SockBuf_AddInt(AddIf_SOCKBUF_PTR, (New)); \
	}

#define AddIfShort(Old,New,Type) if (Old != New) {\
	Old = New; \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (Type)); \
	SockBuf_AddShort(AddIf_SOCKBUF_PTR, (New)); \
	}

#define AddIfChar(Old,New,Type) if (Old != New) {\
	Old = New; \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (Type)); \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (New)); \
	}

#define AddIfIntFlag(Old,New, Flag,Value, Type) if (Old != New) {\
	Old = New; \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (Type)); \
	SockBuf_AddInt(AddIf_SOCKBUF_PTR, (New)); \
	Flag |= Value; \
	}

#define AddIfShortFlag(Old,New, Flag,Value,Type) if (Old != New) {\
	Old = New; \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (Type)); \
	SockBuf_AddShort(AddIf_SOCKBUF_PTR, (New)); \
	Flag |= Value; \
	}

#define AddIfCharFlag(Old,New, Flag,Value, Type) if (Old != New) {\
	Old = New; \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (Type)); \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (New)); \
	Flag |= Value; \
	}

#define AddIfFloat(Old,New,Type) if ((Old) != (New)) {\
	(Old) = (New); \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (Type)); \
	SockBuf_AddInt(AddIf_SOCKBUF_PTR,((New)*FLOAT_MULTI));\
	}

#define AddIfString(Old,New,Type) if (Old == NULL || strcmp(Old,New)) {\
	int _len;\
	if (Old) free(Old);\
	Old = strdup_local(New);\
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (Type)); \
	SockBuf_AddChar(AddIf_SOCKBUF_PTR, (_len = strlen(New))); \
	SockBuf_AddString(AddIf_SOCKBUF_PTR, (New), len); \
	}


#endif /* ifndef __NEWCLIENT_H */
