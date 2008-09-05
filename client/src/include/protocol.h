/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2008 Michael Toennies

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

/* List of client to server (cs) binary command tags */
typedef enum client_cmd {

    CLIENT_CMD_PING, /* system command */

    CLIENT_CMD_SETUP,
    CLIENT_CMD_REQUESTFILE,
    CLIENT_CMD_ADDME,
    CLIENT_CMD_REPLY,
    CLIENT_CMD_NEWCHAR,
    CLIENT_CMD_FACE,
    CLIENT_CMD_MOVE,
    CLIENT_CMD_APPLY,
    CLIENT_CMD_EXAMINE,
    CLIENT_CMD_INVMOVE,
    CLIENT_CMD_GUITALK,
    CLIENT_CMD_LOCK,
    CLIENT_CMD_MARK,
    CLIENT_CMD_FIRE,
    CLIENT_CMD_GENERIC,

    CLIENT_CMD_MAX_NROF

} _client_cmd;


/* Important define: we don't allow bigger data tail blocks as this max! */
#define MAX_DATA_TAIL_LENGTH 255


/* helper defines for calculating the data tail size of binary commands */
#define PARM_SIZE_CHAR     1
#define PARM_SIZE_SHORT    2
#define PARM_SIZE_INT      4


/* List of server to client (sc) binary command tags */
typedef enum server_client_cmd {
    BINARY_CMD_PING, /* unused */
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
    BINARY_CMD_SKILLRDY,
    BINARY_CMD_PLAYER,
    BINARY_CMD_SPELL_LIST,
    BINARY_CMD_SKILL_LIST,
    BINARY_CMD_GOLEMCMD,
    BINARY_CMD_ADDME_SUC,
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
} _server_client_cmd;


enum
{
    SRV_CLIENT_SKILLS,
    SRV_CLIENT_SPELLS,
    SRV_CLIENT_SETTINGS,
    SRV_CLIENT_ANIMS,
    SRV_CLIENT_BMAPS,
    SRV_CLIENT_FILES /* last index */
};


/* list of requested files shared between server and client */
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


/* ID's for the various stats of the player char
 * Note: Not really a low level protocol part but
 * this must be 1:1 shared and changes will inflict
 * every time a protocol change too
 */
#define CS_STAT_HP          1
#define CS_STAT_MAXHP       2
#define CS_STAT_SP          3
#define CS_STAT_MAXSP       4
#define CS_STAT_STR         5
#define CS_STAT_INT         6
#define CS_STAT_WIS         7
#define CS_STAT_DEX         8
#define CS_STAT_CON         9
#define CS_STAT_CHA         10
#define CS_STAT_EXP         11
#define CS_STAT_LEVEL       12
#define CS_STAT_WC          13
#define CS_STAT_AC          14
#define CS_STAT_DAM         15
#define CS_STAT_ARMOUR      16
#define CS_STAT_SPEED       17
#define CS_STAT_FOOD        18
#define CS_STAT_WEAP_SP     19
#define CS_STAT_RANGE       20
#define CS_STAT_TITLE       21
#define CS_STAT_POW         22
#define CS_STAT_GRACE       23
#define CS_STAT_MAXGRACE    24
#define CS_STAT_FLAGS       25
#define CS_STAT_WEIGHT_LIM  26
#define CS_STAT_EXT_TITLE   27

#define CS_STAT_REG_HP      28
#define CS_STAT_REG_MANA    29
#define CS_STAT_REG_GRACE   30
#define CS_STAT_TARGET_HP   31
#define CS_STAT_SPELL_FUMBLE 32

#define CS_STAT_DIST_WC     33
#define CS_STAT_DIST_DPS    34
#define CS_STAT_DIST_TIME   35

/* Start & end of skill experience + skill level, inclusive. */
#define CS_STAT_SKILLEXP_START      118
#define CS_STAT_SKILLEXP_END        129
#define CS_STAT_SKILLEXP_AGILITY    118
#define CS_STAT_SKILLEXP_AGLEVEL    119
#define CS_STAT_SKILLEXP_PERSONAL   120
#define CS_STAT_SKILLEXP_PELEVEL    121
#define CS_STAT_SKILLEXP_MENTAL     122
#define CS_STAT_SKILLEXP_MELEVEL    123
#define CS_STAT_SKILLEXP_PHYSIQUE   124
#define CS_STAT_SKILLEXP_PHLEVEL    125
#define CS_STAT_SKILLEXP_MAGIC      126
#define CS_STAT_SKILLEXP_MALEVEL    127
#define CS_STAT_SKILLEXP_WISDOM     128
#define CS_STAT_SKILLEXP_WILEVEL    129

#define CS_STAT_RES_START           130
#define CS_STAT_RES_END             (130+NROFATTACKS-1)

#define UPD_LOCATION        0x01
#define UPD_FLAGS           0x02
#define UPD_WEIGHT          0x04
#define UPD_FACE            0x08
#define UPD_NAME            0x10
#define UPD_ANIM            0x20
#define UPD_ANIMSPEED       0x40
#define UPD_NROF            0x80
#define UPD_DIRECTION       0x100
#define UPD_QUALITY         0x200
#define UPD_ALL             0xffff
