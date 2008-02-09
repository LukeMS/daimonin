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


#ifndef NEWCLIENT_H
#define NEWCLIENT_H

/* max. socket input buffer we can read/fill when reading from socket.
 * This is raw data until we sort it out and put it in the player command queue.
 */
#define MAXSOCKBUF_IN (3*1024)

/* used for the OS socket bufs */
#define SOCKET_BUFSIZE_SEND (24*1024)
#define SOCKET_BUFSIZE_READ (8*1024)

#define CS_QUERY_HIDEINPUT 0x4  /* Hide input being entered */

/* These are multiplication values that should be used when changing
 * floats to ints, and vice version.  MULTI is integer representatin
 * (float to int), MULTF is float, for going from int to float.
 */
#define FLOAT_MULTI 100000
#define FLOAT_MULTF 100000.0

/* ID's for the various stats that get sent across. */

#define CS_STAT_HP   1
#define CS_STAT_MAXHP    2
#define CS_STAT_SP   3
#define CS_STAT_MAXSP    4
#define CS_STAT_STR  5
#define CS_STAT_INT  6
#define CS_STAT_WIS  7
#define CS_STAT_DEX  8
#define CS_STAT_CON  9
#define CS_STAT_CHA 10
#define CS_STAT_EXP 11
#define CS_STAT_LEVEL   12
#define CS_STAT_WC  13
#define CS_STAT_AC  14
#define CS_STAT_DAM 15
#define CS_STAT_ARMOUR  16
#define CS_STAT_SPEED   17
#define CS_STAT_FOOD    18
#define CS_STAT_WEAP_SP 19
#define CS_STAT_RANGE   20
#define CS_STAT_TITLE   21
#define CS_STAT_POW 22
#define CS_STAT_GRACE   23
#define CS_STAT_MAXGRACE    24
#define CS_STAT_FLAGS   25
#define CS_STAT_WEIGHT_LIM  26
#define CS_STAT_EXT_TITLE 27

/* 0.96 */
#define CS_STAT_REG_HP 28
#define CS_STAT_REG_MANA 29
#define CS_STAT_REG_GRACE 30
#define CS_STAT_TARGET_HP 31
#define CS_STAT_SPELL_FUMBLE 32

#define CS_STAT_DIST_WC 33
#define CS_STAT_DIST_DPS 34
#define CS_STAT_DIST_TIME 35

/* Start & end of skill experience + skill level, inclusive. */
#define CS_STAT_SKILLEXP_START 118
#define CS_STAT_SKILLEXP_END 129
#define CS_STAT_SKILLEXP_AGILITY 118
#define CS_STAT_SKILLEXP_AGLEVEL 119
#define CS_STAT_SKILLEXP_PERSONAL 120
#define CS_STAT_SKILLEXP_PELEVEL 121
#define CS_STAT_SKILLEXP_MENTAL 122
#define CS_STAT_SKILLEXP_MELEVEL 123
#define CS_STAT_SKILLEXP_PHYSIQUE 124
#define CS_STAT_SKILLEXP_PHLEVEL 125
#define CS_STAT_SKILLEXP_MAGIC 126
#define CS_STAT_SKILLEXP_MALEVEL 127
#define CS_STAT_SKILLEXP_WISDOM 128
#define CS_STAT_SKILLEXP_WILEVEL 129

#define CS_STAT_RES_START  130
#define CS_STAT_RES_END    (130+NROFATTACKS-1)

/* These are used with CS_STAT_FLAGS above to communicate S->C what the
 * server thinks the fireon & runon states are.
 */
#define SF_RUNON            2
#define SF_BLIND            4
#define SF_XRAYS            8
#define SF_INFRAVISION      16

/* The following are the color flags passed to new_draw_info.
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
#define F_ETHEREAL      0x0080
#define F_INVISIBLE     0x0100
#define F_UNPAID        0x0200
#define F_MAGIC         0x0400
#define F_CURSED        0x0800
#define F_DAMNED        0x1000
#define F_OPEN          0x2000
#define F_NOPICK        0x4000
#define F_LOCKED        0x8000
#define F_TRAPED        0x10000

#define CF_FACE_NONE    0
#define CF_FACE_BITMAP  1
#define CF_FACE_XPM 2
#define CF_FACE_PNG 3
#define CF_FACE_CACHE   0x10

/* Used in the new_face structure on the magicmap field.  Low bits
 * are color informatin.  For now, only high bit information we need
 * is floor information.
 */
#define FACE_FLOOR  0x80
#define FACE_WALL   0x40    /* Or'd into the color value by the server
                            * right before sending.
                            */
#define FACE_COLOR_MASK 0xf

#define UPD_LOCATION    0x01
#define UPD_FLAGS   0x02
#define UPD_WEIGHT  0x04
#define UPD_FACE    0x08
#define UPD_NAME    0x10
#define UPD_ANIM    0x20
#define UPD_ANIMSPEED   0x40
#define UPD_NROF    0x80
#define UPD_DIRECTION   0x100
#define UPD_QUALITY   0x200
#define UPD_ALL     0xffff

enum
{
    SRV_CLIENT_SKILLS,
    SRV_CLIENT_SPELLS,
    SRV_CLIENT_SETTINGS,
    SRV_CLIENT_ANIMS,
    SRV_CLIENT_BMAPS,
    SRV_CLIENT_FILES /* last index */
};

#define DATA_PACKED_CMD 0x80

enum
{
    DATA_CMD_NO,
    DATA_CMD_SKILL_LIST,
    DATA_CMD_SPELL_LIST,
    DATA_CMD_SETTINGS_LIST,
    DATA_CMD_ANIM_LIST,
    DATA_CMD_BMAP_LIST
};

typedef enum client_cmd {
    CLIENT_CMD_GENERIC,
    CLIENT_CMD_STOP,

    CLIENT_CMD_MAX_NROF
} _client_cmd;

enum
{
	BINARY_CMD_PING, /* unused */
	BINARY_CMD_COMC,
    BINARY_CMD_VERSION ,
    BINARY_CMD_DRAWINFO,
    BINARY_CMD_ADDME_FAIL,
    BINARY_CMD_MAP2,
    BINARY_CMD_DRAWINFO2,
    BINARY_CMD_ITEMX,
    BINARY_CMD_SOUND,
    BINARY_CMD_TARGET,
    BINARY_CMD_UPITEM,
    BINARY_CMD_DELITEM,
    BINARY_CMD_STATS,
    BINARY_CMD_IMAGE,
    BINARY_CMD_FACE1,
    BINARY_CMD_ANIM,
    BINARY_CMD_SKILLRDY,
    BINARY_CMD_PLAYER,
    BINARY_CMD_SPELL_LIST,
    BINARY_CMD_SKILL_LIST,
    BINARY_CMD_GOLEMCMD,
    BINARY_CMD_ADDME_SUC,
    BINARY_CMD_BYE,
    BINARY_CMD_SETUP,
    BINARY_CMD_QUERY,
    BINARY_CMD_DATA,
    BINARY_CMD_NEW_CHAR,
    BINARY_CMD_ITEMY,
    BINARY_CMD_GROUP,
    BINARY_CMD_INVITE,
    BINARY_CMD_GROUP_UPDATE,
    BINARY_CMD_INTERFACE,
    BINARY_CMD_BOOK,
    BINARY_CMD_MARK,
#ifdef USE_CHANNELS
    BINARY_CMD_CHANNELMSG,
#endif
    /* old, unused or outdated crossfire cmds! */
    BINARY_CMD_IMAGE2,
    BINARY_CMD_FACE,
    BINARY_CMD_FACE2,
    BINAR_CMD /* last entry */
};

typedef struct _srv_client_files
{
	sockbuf_struct *sockbuf;    /* static sockbuf with file data */
	int             len;        /* if -1, the file is not compressed */
	int             len_ucomp;  /* original uncompressed file length */
	unsigned int    crc;        /* crc adler32 */
} _srv_client_files;

#define GetInt_String(_data_)			( *(uint32*)(_data_) )
#define GetShort_String(_data_)			( *(uint16*)(_data_) )

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


#endif
