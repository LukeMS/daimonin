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

/* Maximum size of any packet we expect.  Using this makes it so we don't need to
 * allocated and deallocated teh same buffer over and over again and the price
 * of using a bit of extra memory.  IT also makes the code simpler.
 */
#define MAXSOCKBUF 20480
/*#define MAXSOCKBUF 10240 */


#define CS_QUERY_YESNO	0x1	/* Yes/no question */
#define CS_QUERY_SINGLECHAR 0x2	/* Single character response expected */
#define CS_QUERY_HIDEINPUT 0x4	/* Hide input being entered */

#define CS_SAY_NORMAL	0x1	/* Normal say command */
#define CS_SAY_SHOUT	0x2	/* Text is shouted. */
#define CS_SAY_GSAY	0x4	/* Text is group say command */

/* These are multiplication values that should be used when changing 
 * floats to ints, and vice version.  MULTI is integer representatin
 * (float to int), MULTF is float, for going from int to float.
 */
#define FLOAT_MULTI	100000
#define FLOAT_MULTF	100000.0

/* ID's for the various stats that get sent across. */

#define CS_STAT_HP	 1
#define CS_STAT_MAXHP	 2
#define CS_STAT_SP	 3
#define CS_STAT_MAXSP	 4
#define CS_STAT_STR	 5
#define CS_STAT_INT	 6
#define CS_STAT_WIS	 7
#define CS_STAT_DEX	 8
#define CS_STAT_CON	 9
#define CS_STAT_CHA	10
#define CS_STAT_EXP	11
#define CS_STAT_LEVEL	12
#define CS_STAT_WC	13
#define CS_STAT_AC	14
#define CS_STAT_DAM	15
#define CS_STAT_ARMOUR	16
#define CS_STAT_SPEED	17
#define CS_STAT_FOOD	18
#define CS_STAT_WEAP_SP 19
#define CS_STAT_RANGE	20
#define CS_STAT_TITLE	21
#define CS_STAT_POW	22
#define CS_STAT_GRACE	23
#define CS_STAT_MAXGRACE	24
#define CS_STAT_FLAGS	25
#define CS_STAT_WEIGHT_LIM	26
#define CS_STAT_EXT_TITLE 27

/* Start & end of resistances, inclusive. */
#define CS_STAT_RESIST_START	100
#define CS_STAT_RESIST_END	117
#define CS_STAT_RES_PHYS	100
#define CS_STAT_RES_MAG		101
#define CS_STAT_RES_FIRE	102
#define CS_STAT_RES_ELEC	103
#define CS_STAT_RES_COLD	104
#define CS_STAT_RES_CONF	105
#define CS_STAT_RES_ACID	106
#define CS_STAT_RES_DRAIN	107
#define CS_STAT_RES_GHOSTHIT	108
#define CS_STAT_RES_POISON	109
#define CS_STAT_RES_SLOW	110
#define CS_STAT_RES_PARA	111
#define CS_STAT_TIME		112
#define CS_STAT_RES_FEAR	113
#define CS_STAT_RES_DEPLETE	114
#define CS_STAT_RES_DEATH	115
#define CS_STAT_RES_HOLYWORD	116
#define CS_STAT_RES_BLIND	117

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

#define CS_STAT_PROT_START	130
#define CS_STAT_PROT_END	149

#define CS_STAT_PROT_HIT	130
#define CS_STAT_PROT_SLASH	131
#define CS_STAT_PROT_CLEAVE	132
#define CS_STAT_PROT_PIERCE	133
#define CS_STAT_PROT_WMAGIC	134

#define CS_STAT_PROT_FIRE	135
#define CS_STAT_PROT_COLD	136
#define CS_STAT_PROT_ELEC	137
#define CS_STAT_PROT_POISON	138
#define CS_STAT_PROT_ACID	139

#define CS_STAT_PROT_MAGIC	140
#define CS_STAT_PROT_MIND	141
#define CS_STAT_PROT_BODY	142
#define CS_STAT_PROT_PSIONIC	143
#define CS_STAT_PROT_ENERGY	144

#define CS_STAT_PROT_NETHER	145
#define CS_STAT_PROT_CHAOS	146
#define CS_STAT_PROT_DEATH	147
#define CS_STAT_PROT_HOLY	148
#define CS_STAT_PROT_CORRUPT	149

/* These are used with CS_STAT_FLAGS above to communicate S->C what the
 * server thinks the fireon & runon states are.
 */
#define SF_FIREON           1
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
#define NDI_WHITE	0
#define NDI_ORANGE	1
#define NDI_NAVY	2
#define NDI_RED		3
#define NDI_GREEN	4
#define NDI_BLUE	5		
#define NDI_GREY	6
#define NDI_BROWN	7		/* open door, close/open containers... */
#define NDI_PURPLE	8
#define NDI_FLESH  9
#define NDI_YELLOW  10
#define NDI_DK_NAVY  11

#define NDI_MAX_COLOR	11	/* Last value in */
#define NDI_COLOR_MASK	0xff	/* Gives lots of room for expansion - we are */
				/* using an int anyways, so we have the space */
				/* to still do all the flags */


#define NDI_UNIQUE	0x100	/* Print this out immediately, don't buffer */
#define NDI_ALL		0x200	/* Inform all players of this message */

/* Flags for the item command */
enum {a_none, a_readied, a_wielded, a_worn, a_active, a_applied};
#define F_APPLIED       0x000F
#define F_ETHEREAL		0x0080
#define F_INVISIBLE		0x0100
#define F_UNPAID        0x0200
#define F_MAGIC         0x0400
#define F_CURSED        0x0800
#define F_DAMNED        0x1000
#define F_OPEN          0x2000
#define F_NOPICK        0x4000
#define F_LOCKED        0x8000

#define CF_FACE_NONE	0
#define CF_FACE_BITMAP	1
#define CF_FACE_XPM	2
#define CF_FACE_PNG	3
#define CF_FACE_CACHE	0x10

/* Used in the new_face structure on the magicmap field.  Low bits
 * are color informatin.  For now, only high bit information we need
 * is floor information.
 */
#define FACE_FLOOR	0x80
#define FACE_WALL	0x40	/* Or'd into the color value by the server
				 * right before sending.
				 */
#define FACE_COLOR_MASK	0xf

#define UPD_LOCATION	0x01
#define UPD_FLAGS	0x02
#define UPD_WEIGHT	0x04
#define UPD_FACE	0x08
#define UPD_NAME	0x10
#define UPD_ANIM	0x20
#define UPD_ANIMSPEED	0x40
#define UPD_NROF	0x80
#define UPD_DIRECTION	0x100

/* Contains the base information we use to make up a packet we want to send. */
typedef struct SockList {
    int len;
    unsigned char *buf;
} SockList;

typedef struct CS_Stats {
    int	    ibytes;	/* ibytes, obytes are bytes in, out */
    int	    obytes;
    short   max_conn;	/* Maximum connections received */
    time_t  time_start;	/* When we started logging this */
} CS_Stats;

typedef struct _srv_client_files {
	char *file;		/* file data, compressed or not */
	int len;		/* if -1, the file is not compressed */
	int len_ucomp;	/* original uncompressed file length */
	unsigned int crc;		/* crc adler32 */
} _srv_client_files;

enum {
	SRV_CLIENT_SKILLS,
	SRV_CLIENT_SPELLS,
	SRV_CLIENT_SETTINGS,
	SRV_CLIENT_ANIMS,
	SRV_CLIENT_BMAPS,
	SRV_CLIENT_FILES /* last index */
};

extern _srv_client_files SrvClientFiles[SRV_CLIENT_FILES];

extern CS_Stats cst_tot, cst_lst;

#define DATA_PACKED_CMD 0x80

enum {
	DATA_CMD_NO,
	DATA_CMD_SKILL_LIST,
	DATA_CMD_SPELL_LIST,
	DATA_CMD_SETTINGS_LIST,
	DATA_CMD_ANIM_LIST,
	DATA_CMD_BMAP_LIST
};

#define SOCKET_SET_BINARY_CMD(__s__, __bc__) (__s__)->buf[0]=__bc__;(__s__)->len=1

enum {
	BINARY_CMD_COMC=1,
	BINARY_CMD_MAP2,
	BINARY_CMD_DRAWINFO,
	BINARY_CMD_MAP_SCROLL,
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
	BINARY_CMD_MAPSTATS,
	BINARY_CMD_SPELL_LIST,
	BINARY_CMD_SKILL_LIST,
	BINARY_CMD_GOLEMCMD,
	BINARY_CMD_ADDME_SUC,
	BINARY_CMD_ADDME_FAIL,
	BINARY_CMD_VERSION,
	BINARY_CMD_BYE,
	BINARY_CMD_SETUP,
	BINARY_CMD_QUERY,
	BINARY_CMD_DATA,
	
	/* old, unused or outdated crossfire cmds! */
	BINARY_CMD_DELINV,
	BINARY_CMD_MAGICMAP,
	BINARY_CMD_REPLYINFO,
	BINARY_CMD_IMAGE2,
	BINARY_CMD_FACE,
	BINARY_CMD_FACE2,
	BINARY_CMD_ITEM1,/*not used in server */

	BINAR_CMD /* last entry */
};

#endif
