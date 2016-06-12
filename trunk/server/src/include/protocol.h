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

    The author can be reached via e-mail to info@daimonin.org
*/

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#define DAI_VERSION_RELEASE 0
#define DAI_VERSION_MAJOR 10
#define DAI_VERSION_MINOR 8
#define PROTOCOL_VERSION 991031

/* min & max length of player & account names & password
* important - for a login server this must snyced up with it too
*/
#define MIN_PLAYER_NAME        3
#define MAX_PLAYER_NAME        16

#define MIN_ACCOUNT_NAME        3
#define MAX_ACCOUNT_NAME        24
#define MIN_ACCOUNT_PASSWORD    6
#define MAX_ACCOUNT_PASSWORD    16

/* When reclaiming a pre-account (=B4) player, the user must enter the correct
 * password. This is a default that will be used the first time (when the
 * sever is just checking to see if the player exists to be reclaimed at all).
 *
 * By sending this dummy password we avoid inidicating password guessing.
 *
 * If a client does not send exactly this string, password guessing will be
 * indicated, although (ATM) this has no effect other than a false positive in
 * the server logs. */
#define RECLAIM_NOPASS "NOPASS" // must be >= 6 characters

/* The embedded character codes are used to markup text, mainly in the textwindows
 * and GUIs. */
/* 0x00-0x1f are internal. Some are genuine remaps, some are simply meaningful
 * names for a number, and some are both. */
#define ECC_CHAROFFSET    0x00
#define ECC_LINEHEIGHT    0x01
#define ECC_NEWCOLR       0x02
#define ECC_DEFCOLR       0x03
#define ECC_TAB           0x09
#define ECC_NEWLINE       0x0a
#define ECC_ARROWUP       0x1c
#define ECC_ARROWDOWN     0x1d
#define ECC_ARROWLEFT     0x1e
#define ECC_ARROWRIGHT    0x1f
/* 0x80-0x95 are smileys. */
#define ECC_SMILE         0x80
#define ECC_FROWN         0x81
#define ECC_GRIN          0x82
#define ECC_POKERFACE     0x83
#define ECC_OOH           0x84
#define ECC_RASPBERRY     0x85
#define ECC_WINK          0x86
#define ECC_COOL          0x87
/* No idea what 0x88 is meant to be and it's unused anyway. */
#define ECC_WINKRASPBERRY 0x89
#define ECC_CRY           0x8a
#define ECC_PERPLEXITY    0x8b
#define ECC_SUCKALEMON    0x8c
#define ECC_NAUGHTY       0x8d
#define ECC_WICKED        0x8e
/* The rest are 'normal'. These are easily typeable, but seldom used,
 * characters so players/scripters can use them.
 *
 * Underline has extra meaning in the NPC GUI (and eventually also the book
 * GUI). Here it will render a so-called intertitle, basically a sub-title.
 *
 * Hypertext is only available in the NPC GUI (and eventually probably the book
 * GUI too). Otherwise it will just print the character.
 *
 * Emphasis, strong, and underline can be combined to apply any combination of
 * two or three markups to the text. Hypertext negates any existing strong,
 * emphasis, or underline. */
#define ECC_STRONG              '|'
#define ECC_EMPHASIS            '~'
#define ECC_UNDERLINE           '`'
#define ECC_HYPERTEXT           '^'

/* List of client to server (cs) binary command tags */
typedef enum client_cmd {
    /* start of pre-processed cmds */
    CLIENT_CMD_PING,
    /* Ns_Login mode only commands */
    CLIENT_CMD_SETUP,
    CLIENT_CMD_REQUESTFILE,
    CLIENT_CMD_CHECKNAME,
    CLIENT_CMD_LOGIN,
    /* Ns_Account mode only commands */
    CLIENT_CMD_NEWCHAR,
    CLIENT_CMD_DELCHAR,
    CLIENT_CMD_ADDME,
    /* Ns_Playing mode only commands */
    CLIENT_CMD_FACE, /* special case: Allowed since Ns_Login for face sending servers */
    CLIENT_CMD_MOVE,
    /* end of pre-processed cmds */
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
    SERVER_CMD_PING,
    SERVER_CMD_DRAWINFO,
    SERVER_CMD_ADDME_FAIL,
    SERVER_CMD_MAP2,
    SERVER_CMD_DRAWINFO2,
    SERVER_CMD_ITEMX,
    SERVER_CMD_SOUND,
    SERVER_CMD_TARGET,
    SERVER_CMD_UPITEM,
    SERVER_CMD_DELITEM,
    SERVER_CMD_STATS,
    SERVER_CMD_IMAGE,
    SERVER_CMD_FACE1,
    SERVER_CMD_SKILLRDY,
    SERVER_CMD_PLAYER,
    SERVER_CMD_SPELL_LIST,
    SERVER_CMD_SKILL_LIST,
    SERVER_CMD_GOLEMCMD,
    SERVER_CMD_ACCNAME_SUC,
    SERVER_CMD_SETUP,
    SERVER_CMD_DATA,
    SERVER_CMD_ITEMY,
    SERVER_CMD_GROUP,
    SERVER_CMD_INVITE,
    SERVER_CMD_GROUP_UPDATE,
    SERVER_CMD_INTERFACE,
    SERVER_CMD_BOOK,
    SERVER_CMD_MARK,
    SERVER_CMD_ACCOUNT,
#ifdef USE_CHANNELS
    SERVER_CMD_CHANNELMSG,
#endif

    BINAR_CMD /* last entry */
} _server_client_cmd;

/* number of chars one account can control */
#define ACCOUNT_MAX_PLAYER 6

/* define how much gender we have - needed to check client_settings gender entries */
#define MAX_RACE_GENDER 4

/* TODO: add start skill ids to client_settings including this define */
#define MAX_START_SKILLS 4

/* return values for account login */
typedef enum account_status_enum
{
    ACCOUNT_STATUS_OK,
    ACCOUNT_STATUS_UNKNOWN,
    ACCOUNT_STATUS_EXISTS,
    ACCOUNT_STATUS_CORRUPT,
    ACCOUNT_STATUS_NOSAVE,
    ACCOUNT_STATUS_BANNED,
    ACCOUNT_STATUS_DISCONNECT,
    ACCOUNT_STATUS_WRONGPWD
} account_status;

/* tell the main account login function what to do */
typedef enum account_mode_enum
{
    ACCOUNT_MODE_LOGIN,
    ACCOUNT_MODE_CREATE
} account_mode;

/* return values for addme commands */
typedef enum addme_login_msg_struct {
    ADDME_MSG_OK,
    ADDME_MSG_UNKNOWN,
    ADDME_MSG_CORRUPT,
    ADDME_MSG_ACCOUNT,
    ADDME_MSG_TAKEN,
    ADDME_MSG_INTERNAL,
    ADDME_MSG_BANNED,
    ADDME_MSG_DISCONNECT,
    ADDME_MSG_WRONGPWD
} addme_login_msg;

enum
{
    SRV_CLIENT_SOUNDS,
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
    DATA_CMD_SOUND_LIST,
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

#define CS_STAT_ACTION_TIME   36

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

#define UPD_LOCATION    (1 << 0)
#define UPD_FLAGS       (1 << 1)
#define UPD_WEIGHT      (1 << 2)
#define UPD_FACE        (1 << 3)
#define UPD_NAME        (1 << 4)
#define UPD_ANIM        (1 << 5)
#define UPD_ANIMSPEED   (1 << 6)
#define UPD_NROF        (1 << 7)
#define UPD_DIRECTION   (1 << 8)
#define UPD_QUALITY     (1 << 9)
#define UPD_SERVERFLAGS (1 << 15)
//#define UPD_ALL         0xffff

/* Item apply values -- only set if F_APPLIED */
#define A_READIED 1
#define A_WIELDED 2
#define A_WORN    3
#define A_ACTIVE  4
#define A_APPLIED 5

/* Item flags. */
/* TODO: Why the huge gap between F_APPLIED and F_ETHEREAL? F_ETHEREAL could be
 * 1<<5 or 1<<6. Then we could fit these flags into a uint16 whereas now we
 * need uint32.
 *
 * -- Smacky 20130305 */
#define F_APPLIED   ((1 << 4) - 1)
#define F_ETHEREAL  (1 <<  7)
#define F_INVISIBLE (1 <<  8)
#define F_UNPAID    (1 <<  9)
#define F_MAGIC     (1 << 10)
#define F_CURSED    (1 << 11)
#define F_DAMNED    (1 << 12)
#define F_OPEN      (1 << 13)
#define F_NOPICK    (1 << 14)
#define F_LOCKED    (1 << 15)
#define F_TRAPED    (1 << 16)

/* Map flags for thing over and thing under. */
#define FFLAG_SLEEP     (1 << 0) // sleeping
#define FFLAG_CONFUSED  (1 << 1) // confused
#define FFLAG_PARALYZED (1 << 2) // paralyzed
#define FFLAG_SCARED    (1 << 3) // scared - it will run away
#define FFLAG_EATING    (1 << 4) // eating
#define FFLAG_INVISIBLE (1 << 5) // invisible (normal or gmaster)
#define FFLAG_ETHEREAL  (1 << 6) // ethereal
/* TODO: Unfortunately EATING was added in place of BLINDED at some point in
 * the past. As SERVER_CMD_MAP2 only allows a uint8 here, there is not room to
 * now readd BLINDED -- so we need to tweak the protocol to send/receive extra
 * data (probably a whole byte). This requires a Y update. Still, the rest of
 * the server has been tweaked in this commit (r7297) so it should be a simple
 * case of removing this #if 0 9and updating the client).
 *
 * -- Smacky 20130305 */
#if 0
#define FFLAG_BLINDED   (1 << 7) // blinded
#define FFLAG_PROBE     (1 << 8) // probed !Flag is set by map2 cmd!
#else
#define FFLAG_PROBE     (1 << 7) // probed !Flag is set by map2 cmd!
#endif

/* maximum reachable level */
#define MAXLEVEL 110

typedef enum _gui_npc_mode
{
    GUI_NPC_MODE_NO,
    GUI_NPC_MODE_NPC,
    GUI_NPC_MODE_RHETORICAL,
    GUI_NPC_MODE_QUEST,
    GUI_NPC_MODE_END
}
_gui_npc_mode;

typedef enum altact_mode_t
{
    ALTACT_MODE_ARCHERY,
    ALTACT_MODE_SPELL,
    ALTACT_MODE_SKILL,
    ALTACT_MODE_PRAYER,
    ALTACT_MODE_DEVICE,
    ALTACT_MODE_THROWING,

    ALTACT_NROF // must be last entry
}
altact_mode_t;

extern int account_name_valid(char *cp);
extern int account_char_valid(char c);
extern int password_valid(char *cp);
extern int password_char_valid(char c);
extern int player_name_valid(char *cp);
extern int player_char_valid(char c);
extern int transform_account_name_string(char *name);
extern int transform_player_name_string(char *name);

#endif /* ifndef __PROTOCOL_H */
